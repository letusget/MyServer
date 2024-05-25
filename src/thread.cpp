#include "thread.h"

#include "log.h"
#include "util.h"
namespace myserver {

// 线程局部存储变量，指向当前线程对象
static thread_local Thread* t_thread = nullptr;
// 线程局部存储变量，指向当前线程名称
static thread_local std::string t_current_thread_name = "UNKNOWN";

// 日志设置
// TODO static初始化问题
static mylog::Logger::ptr g_logger = MYLOG_LOG_NAME("system");

Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem_init failed");
        MYLOG_LOG_ERROR(g_logger) << "sem_init failed, error code: " << errno;
    }
}

Semaphore::~Semaphore() {}

// 等待信号量
void Semaphore::wait() {
    // while (true)
    // {
    //     // sem_wait返回0表示获取成功，返回-1表示获取失败，errno=EINTR表示被信号打断，需要重试
    //     if(!sem_wait(&m_semaphore)) {
    //         return;
    //     }
    //     // 重试
    //     if(errno!= EINTR) {
    //         MYLOG_LOG_WARN(g_logger) << "sem_wait failed, retrying..., error code: " << errno;
    //         throw std::logic_error("sem_wait failed");
    //     }
    // }

    // sem_wait返回0表示获取成功，返回-1表示获取失败，
    while (sem_wait(&m_semaphore)) {
        // errno=EINTR表示被信号打断，需要重试
        if (errno != EINTR) {
            MYLOG_LOG_WARN(g_logger) << "sem_wait failed, retrying..., error code: " << errno;
            throw std::logic_error("sem_wait failed");
        }  
    }
}

// 提醒信号量
void Semaphore::notify() {
    // if (sem_post(&m_semaphore)) {
    //     MYLOG_LOG_ERROR(g_logger) << "sem_post failed, error code: " << errno;
    //     throw std::logic_error("sem_post failed");
    // }

    // sem_post 返回0表示获取成功，返回-1表示获取失败，
    while (sem_post(&m_semaphore))
    {
        // errno=EINTR表示被信号打断，需要重试
        if (errno != EINTR) {
            MYLOG_LOG_WARN(g_logger) << "sem_post failed, retrying..., error code: " << errno;
            throw std::logic_error("sem_post failed");
        }
    }
    
}

// 获取当前线程对象指针
Thread* Thread::GetThis() { return t_thread; }

const std::string& Thread::GetName() { return t_current_thread_name; }

// 获取当前线程名称
const std::string& Thread::GetCurrentThreadName() { return t_current_thread_name; }

void Thread::SetCurrentThreadName(const std::string& name) {
    if (t_thread) {
        t_thread->m_name = name;
    }
    t_current_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name) {
    if (name.empty()) {
        m_name = "UNKNOWN";
    } else {
        m_name = name;
    }
    m_cb = std::move(cb);

    int rt = pthread_create(&m_pthread_id, nullptr, &Thread::run, this);
    if (rt != 0) {
        MYLOG_LOG_ERROR(g_logger) << "pthread_create failed, error code: " << rt << " name: " << name;
        throw std::logic_error("pthread_create failed");
    }

    // 等待线程启动
    m_semaphore.wait();
}

Thread::~Thread() {
    if (m_pthread_id) {
        // 这里暂时不做join，因为join会导致当前线程阻塞，导致无法退出
        pthread_detach(m_pthread_id);
    }
}

void Thread::join() {
    if (m_pthread_id) {
        int rt = pthread_join(m_pthread_id, nullptr);
        if (rt != 0) {
            MYLOG_LOG_ERROR(g_logger) << "pthread_join failed, error code: " << rt << " name: " << m_name;
            throw std::logic_error("pthread_join failed");
        }
        m_pthread_id = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    // 设置线程局部存储变量
    t_thread              = thread;
    t_current_thread_name = thread->m_name;
    thread->m_id          = mylog::GetThreadId();
    // 设置线程名称 pthread_setname_np最大支持16个字符
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    // 防止智能指针没有释放
    cb.swap(thread->m_cb);

    // 通知线程启动完成
    thread->m_semaphore.notify();

    // 执行回调函数
    cb();
    return nullptr;
}

}  // namespace myserver