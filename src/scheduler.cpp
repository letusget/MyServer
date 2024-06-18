#include "scheduler.h"

#include "log.h"
#include "macro.h"

namespace myserver {
static mylog::Logger::ptr g_logger = MYLOG_LOG_NAME("system");

// 线程局部变量
static thread_local Scheduler* t_scheduler = nullptr;
// 当前线程的主协程
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) : m_name(name) {
    MYSERVER_ASSERT_MSG(threads > 0, "threads must be greater than 0");

    // 如果线程没有指定协程，则使用分配的主协程, 否则由GetThis获取主协程进行调度，由reset获取新协程来执行
    if (use_caller) {
        myserver::Fiber::GetThis();
        --threads;

        // 确保只有一个协程调度器
        MYSERVER_ASSERT_MSG(GetThis() == nullptr, "only one scheduler can be used in a thread");
        t_scheduler = this;

        // 工作协程
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
        // 确保线程名称已经设置
        myserver::Thread::SetName(m_name);

        // 这个地方的关键点在于，是否把创建协程调度器的线程放到协程调度器管理的线程池中。
        // 如果不放入，那这个线程专职协程调度；
        // 如果放的话，那就要把协程调度器封装到一个协程中，称之为主协程或协程调度器协程。
        t_fiber        = m_rootFiber.get();
        m_rootThreadId = myserver::GetThreadId();
        m_threadIds.push_back(m_rootThreadId);
    } else {
        // 复用主协程的线程池
        m_rootThreadId = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    MYSERVER_ASSERT_MSG(m_stopping, "scheduler is not stopped");
    // 只有当且主协程调度器存在
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() { return t_scheduler; }

Fiber* Scheduler::GetMainFiber() { return t_fiber; }

void Scheduler::start() {
    // 启动线程
    MutexType::Lock lock(m_mutex);
    // 已经启动的就直接返回
    if (!m_stopping) {
        return;
    }

    m_stopping = false;
    MYSERVER_ASSERT_MSG(m_threads.empty(), "threads are already started");

    // 创建线程
    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
}

void Scheduler::stop() {
    m_autoStop = true;
    // 创建此调度器的线程创建的run的协程
    if (m_rootFiber && m_threadCount == 0 &&
        (m_rootFiber->GetFiberState() == Fiber::TERM || m_rootFiber->GetFiberState() == Fiber::INIT)) {
        MYLOG_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    // bool exit_on_this_fiber = false;
    if (m_rootThreadId == -1) {
        MYSERVER_ASSERT_MSG(GetThis() == this, "only one scheduler can be used in a thread");
    } else {
        MYSERVER_ASSERT_MSG(GetThis() != this, "only one scheduler can be used in a thread");
    }

    m_stopping = true;
    // 等待线程退出
    for (size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if (m_rootFiber) {
        tickle();
    }

    if (stopping()) {
        return;
    }

    // if(exit_on_this_fiber) {

    // }
}

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::run() {
    Fiber::GetThis();
    setThis();
    if (myserver::GetThreadId() != m_rootThreadId) {
        t_fiber = Fiber::GetThis().get();
    }
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));

    // 回调
    Fiber::ptr cb_fiber;
    FiberAndThread ft;
    while (true) {
        ft.reset();
        bool tickle_me = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                // 已经分配好的线程不用调度
                if (it->thread_id != -1 && it->thread_id != myserver::GetThreadId()) {
                    ++it;
                    // 通知其他线程执行
                    tickle_me = true;
                    continue;
                }

                MYSERVER_ASSERT_MSG(it->fiber || it->cb, "fiber or cb is null");
                // 跳过正在执行的协程
                if (it->fiber && it->fiber->GetFiberState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                // 需要处理的协程
                ft = *it;
                m_fibers.erase(it);
            }
        }

        if (tickle_me) {
            tickle();
        }

        // 处理协程
        if (ft.fiber && (ft.fiber->GetFiberState() != Fiber::TERM || ft.fiber->GetFiberState() != Fiber::EXCEPT)) {
            ++m_activeThreadCount;
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if (ft.fiber->GetFiberState() == Fiber::READY) {
                // 协程准备就绪，放入线程池
                schedule(ft.fiber);
            } else if (ft.fiber->GetFiberState() != Fiber::TERM && ft.fiber->GetFiberState() != Fiber::EXCEPT) {
                // 让出执行权
                ft.fiber->SetFiberState(Fiber::HOLD);
            }
            ft.reset();
        } else if (ft.cb) {
            if (cb_fiber) {
                // cb_fiber->reset(&ft.cb);
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            ++m_activeThreadCount;
            cb_fiber->swapIn();
            --m_activeThreadCount;

            if (cb_fiber->GetFiberState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if (cb_fiber->GetFiberState() == Fiber::EXCEPT || cb_fiber->GetFiberState() == Fiber::TERM) {
                // 协程异常或终止，放弃处理
                cb_fiber.reset();
            } else {
                // 协程未结束
                cb_fiber->SetFiberState(Fiber::HOLD);
                cb_fiber.reset();
            }
        } else {
            // 当前有任务执行的情况
            if (idle_fiber->GetFiberState() == Fiber::TERM) {
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if (idle_fiber->GetFiberState() == Fiber::TERM || idle_fiber->GetFiberState() != Fiber::EXCEPT) {
                idle_fiber->SetFiberState(Fiber::HOLD);
            }
        }
    }
}
}  // namespace myserver