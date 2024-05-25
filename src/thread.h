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

// 封装互斥量对象 lockguard
// 使用类模板LockGuard，封装互斥锁: 构造加锁 析构解锁(RAII)
template <class T>
class ScopedLockImpl {
    /**
     * @brief 构造函数，加锁
     * @param mutex 互斥锁对象
     * 通用互斥锁类模板
     * 封装互斥锁对象，提供加锁和解锁接口
     */
   public:
    // 加锁
    ScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.lock();  // 加锁
        m_locked = true;
    }
    ~ScopedLockImpl() {
        // 解锁
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();  // 加锁
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();  // 解锁
            m_locked = false;
        }
    }

   private:
    // 禁用拷贝构造函数和赋值运算符，防止互斥锁对象被复制
    ScopedLockImpl(const ScopedLockImpl&)  = delete;
    ScopedLockImpl(const ScopedLockImpl&&) = delete;
    ScopedLockImpl& operator=(const ScopedLockImpl&) = delete;
    ScopedLockImpl& operator=(const ScopedLockImpl&&) = delete;

   private:
    // 互斥锁对象
    T& m_mutex;
    // 互斥锁状态
    bool m_locked = false;
};

// // 互斥锁对象 不分读写锁
// class Mutex {
//    public:
//     Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
//     ~Mutex() { pthread_mutex_destroy(&m_mutex); }

//     void lock() { pthread_mutex_lock(&m_mutex); }

//     void unlock() { pthread_mutex_unlock(&m_mutex); }

//    private:
//     // 互斥锁对象
//     pthread_mutex_t m_mutex;
// };

// 封装读锁对象 lockguard
// 使用类模板LockGuard，封装读写锁: 构造加锁 析构解锁(RAII)
template <class T>
class ReadScopedLockImpl {
    /**
     * @brief 构造函数，加锁
     * @param mutex 互斥锁对象
     * 通用互斥锁类模板
     * 封装互斥锁对象，提供加锁和解锁接口
     */
   public:
    // 加锁
    ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.rdlock();  // 加锁
        m_locked = true;
    }
    ~ReadScopedLockImpl() {
        // 解锁
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();  // 加锁
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();  // 解锁
            m_locked = false;
        }
    }

   private:
    // 禁用拷贝构造函数和赋值运算符，防止互斥锁对象被复制
    ReadScopedLockImpl(const ReadScopedLockImpl&)  = delete;
    ReadScopedLockImpl(const ReadScopedLockImpl&&) = delete;
    ReadScopedLockImpl& operator=(const ReadScopedLockImpl&) = delete;
    ReadScopedLockImpl& operator=(const ReadScopedLockImpl&&) = delete;

   private:
    // 互斥锁对象
    T& m_mutex;
    // 互斥锁状态
    bool m_locked = false;
};

// 封装写锁对象 lockguard
// 使用类模板LockGuard，封装读写锁: 构造加锁 析构解锁(RAII)
template <class T>
class WriteScopedLockImpl {
    /**
     * @brief 构造函数，加锁
     * @param mutex 互斥锁对象
     * 通用互斥锁类模板
     * 封装互斥锁对象，提供加锁和解锁接口
     */
   public:
    // 加锁
    WriteScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.wrlock();  // 加锁
        m_locked = true;
    }
    ~WriteScopedLockImpl() {
        // 解锁
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();  // 加锁
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();  // 解锁
            m_locked = false;
        }
    }

   private:
    // 禁用拷贝构造函数和赋值运算符，防止互斥锁对象被复制
    WriteScopedLockImpl(const WriteScopedLockImpl&)  = delete;
    WriteScopedLockImpl(const WriteScopedLockImpl&&) = delete;
    WriteScopedLockImpl& operator=(const WriteScopedLockImpl&) = delete;
    WriteScopedLockImpl& operator=(const WriteScopedLockImpl&&) = delete;

   private:
    // 互斥锁对象
    T& m_mutex;
    // 互斥锁状态
    bool m_locked = false;
};

// 互斥锁对象 分读写锁
class RWMutex {
   public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    
    RWMutex() { pthread_rwlock_init(&m_rwlock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_rwlock); }

    // 读锁
    void rdlock() { pthread_rwlock_rdlock(&m_rwlock); }

    // 写锁
    void wrlock() { pthread_rwlock_wrlock(&m_rwlock); }

    // 解锁
    void unlock() { pthread_rwlock_unlock(&m_rwlock); }

   private:
    // 读写锁对象
    pthread_rwlock_t m_rwlock;
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