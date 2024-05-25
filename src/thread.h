#ifndef __MYSERVER_THREAD_H__
#define __MYSERVER_THREAD_H__

/**
 * 使用C++11支持的std::thread
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

// typedef pthread_t tid_t;
namespace myserver {

// 封装信号量对象
class Semaphore {
   public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    // 等待信号量
    void wait();
    // 提醒信号量
    void notify();
    // // 释放信号量
    // void post();
    // // 获取信号量计数
    // uint32_t getValue() const;
    // // 设置信号量计数
    // void setValue(uint32_t count);
    // // 重置信号量计数
    // void reset(uint32_t count = 0);
    // // 等待信号量，直到超时或计数为0    
    // bool timedWait(uint32_t timeout_ms);

   private:
    // 禁用拷贝构造函数和赋值运算符，防止信号量对象被复制
    Semaphore(const Semaphore&)  = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&&) = delete;

   private:
    sem_t m_semaphore;
};

// 封装线程对象
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

    // 信号量对象
    Semaphore m_semaphore;
};

}  // namespace myserver

#endif