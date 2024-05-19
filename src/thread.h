#ifndef __MYSERVER_THREAD_H__
#define __MYSERVER_THREAD_H__

/**
 * 使用C++11支持的std::thread
 */
#include <pthread.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

// typedef pthread_t tid_t;
namespace myserver {

class Thread {
   public:
    typedef std::shared_ptr<Thread> ptr;
    // 封装线程ID，与实际ps命令执行得到的线程ID相同
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void join();
    // 获取当前线程对象指针
    static Thread* GetThis();
    static const std::string& GetName();
    // 获取当前线程名称
    static const std::string& GetCurrentThreadName();
    // 设置当前线程名称
    static void SetCurrentThreadName(const std::string& name);
   private:
    // 禁用拷贝构造函数和赋值运算符，防止线程对象被复制
    Thread(const Thread&)  = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(const Thread&&) = delete;

    static void* run(void* arg);

   private:
    // 线程标识符
    pid_t m_id = -1;
    // 线程ID
    pthread_t m_pthread_id = 0;
    // 线程回调函数
    std::function<void()> m_cb = nullptr;
    // 线程名称
    std::string m_name = "";
};

}  // namespace myserver

#endif