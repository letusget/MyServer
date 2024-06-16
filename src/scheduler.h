#ifndef __MYSERVER_SCHEDULER_H__
#define __MYSERVER_SCHEDULER_H__

#include <list>
#include <memory>
#include <vector>

#include "fiber.h"
#include "thread.h"

namespace myserver {
class Scheduler {
   public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef myserver::Mutex MutexType;

    /**
     * @brief 有参构造函数
     * @param threads 线程数 默认为1
     * @param use_caller 是否使用当前协程调度器 默认为true
     * @param name 协程调度器名 默认为"Scheduler"
     */
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "Scheduler");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }
    /**
     * @brief 获取当前协程调度器对象
     * @return 无
     */
    static Scheduler* GetThis();

    /**
     * @brief 获取主协程对象
     * @return 主协程对象
     */
    static Fiber* GetMainFiber();

    /**
     * @brief 启动协程调度器
     * @return 无
     */
    void start();

    /**
     * @brief 停止协程调度器
     * @return 无
     */
    void stop();

    /**
     * @brief 有锁调度器调度函数
     * @param fc 协程对象或回调函数
     * @param thread_id 线程id 默认为-1
     * @return 无
     */
    template <class FiberOrCb>
    void schedule(FiberOrCb fc, int thread_id = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread_id);
        }

        if (need_tickle) {
            tickle();
        }
    }

    /**
     * @brief 批量调度函数
     * @param begin 开始迭代器
     * @param end 结束迭代器
     * @return 无
     * @note
     * 批量调度函数，将迭代器范围内的协程对象或回调函数放入待执行队列，并唤醒协程调度器，可以确保在同一个消息队列中连续执行
     */
    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin) || need_tickle;
                ++begin;
            }
        }
        if (need_tickle) {
            tickle();
        }
    }

   protected:
    /**
     * @brief 唤醒协程调度器
     * @return 无
     */
    virtual void tickle();

   private:
    /**
     * @brief 无锁调度器调度函数
     * @param fc 协程对象或回调函数
     * @param thread_id 线程id 默认为-1
     * @return bool 是否需要唤醒协程调度器
     * @note 无锁调度器调度函数，将协程对象或回调函数放入待执行队列，并唤醒协程调度器
     */
    template <class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread_id = -1) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread_id);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }

   private:
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread_id;
        FiberAndThread(Fiber::ptr f, int id) : fiber(f), thread_id(id) {}
        // 这里接受后可以将智能指针引用计数减一，避免内存泄漏
        FiberAndThread(Fiber::ptr* f, int id) : thread_id(id) { fiber.swap(*f); }
        FiberAndThread(std::function<void()> c, int id) : cb(c), thread_id(id) {}
        // 这里接受后可以将智能指针引用计数减一，避免内存泄漏
        FiberAndThread(std::function<void()>* c, int id) : thread_id(id) { cb.swap(*c); }
        // 防止放入空对象(放入stl容器时无法初始化)
        FiberAndThread() : thread_id(-1) {}

        void reset() {
            fiber     = nullptr;
            cb        = nullptr;
            thread_id = -1;
        }
    };

   private:
    MutexType m_mutex;
    // 线程池
    std::vector<Thread::ptr> m_threads;
    // 计划执行的协程对象
    std::list<FiberAndThread> m_fibers;
    std::string m_name;
};
}  // namespace myserver
#endif