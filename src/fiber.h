/**
 * @file fiber.h
 * @author William <<EMAIL>>
 * @date 2024-06-10
 * @version 0.1
 * @brief 协程类
 */
#ifndef __MYSERVER_FIBER_H__
#define __MYSERVER_FIBER_H__

#include <ucontext.h>

#include <functional>
#include <memory>

#include "thread.h"

namespace myserver {

// 拥有智能指针的 Fiber 类，不会在栈上分配内存
class Fiber : public std::enable_shared_from_this<Fiber> {
   public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        INIT,   // 刚刚创建，尚未执行
        HOLD,   // 挂起，等待被唤醒
        EXEC,   // 正在执行
        TERM,   // 已经终止
        READY,  // 准备就绪，可以被调度
        EXCEPT  // 发生异常，无法继续执行
    };

   private:
    Fiber();

   public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    /**
     * @brief 重置协程函数并重置状态(初始或结束)
     * @param cb 要执行的回调函数
     * @param stacksize 栈大小，默认 0 表示使用默认大小
     * @return 启动成功返回 true，否则返回 false
     * @note 一个协程执行完毕后，可以由其他协程继续使用这段栈空间，提高执行效率
     */
    bool reset(std::function<void()> cb);

    /**
     * @brief 进入 fiber，切换到当前协程执行
     */
    void swapIn();

    /**
     * 离开 fiber，切换到后台执行
     */
    void swapOut();

    uint64_t getId() const { return m_id; }

   public:
    /**
     * @brief 获取当前 fiber
     * @return 当前 fiber 的智能指针
     * @note 这个方法只能在 fiber 内调用
     */
    static Fiber::ptr GetThis();
    /**
     * @brief 设置当前 fiber
     * @param ptr 要设置的 fiber 的智能指针
     * @return 设置成功返回 true，否则返回 false
     */
    static void SetThis(Fiber* ptr);

    /**
     * @brief 协程切换到后台，并且设置为 ready 状态
     * @return 当前 fiber 的状态
     */
    static void YieldToReady();

    /**
     * @brief 协程切换到后台，并且设置为 hold 状态
     * @return 当前 fiber 的状态
     */
    static void YieldToHold();

    /**
     * @brief 获取当前 fiber 总数
     * @return 当前 fiber 总数
     */
    static uint64_t TotalFibers();

    /**
     * @brief 执行主 fiber
     */
    static void MainFunc();

    static uint64_t GetFiberId();
    State GetFiberState() const { return m_state; }
    void SetFiberState(State state) { m_state = state; }

   private:
    uint64_t m_id        = 0;        // 协程 ID
    uint32_t m_stacksize = 0;        // 栈大小
    State m_state        = INIT;     // 协程状态
    void* m_stack        = nullptr;  // 栈空间
    ucontext_t m_ctx;                // 协程上下文
    std::function<void()> m_cb;      // 协程函数
};
}  // namespace myserver

#endif  // __MYSERVER_FIBER_H__