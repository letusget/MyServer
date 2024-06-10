#include "fiber.h"

#include <atomic>

#include "config.h"
#include "log.h"
#include "macro.h"
#include "util.h"

namespace myserver {
// 本系统的日志对象为system
static mylog::Logger::ptr g_logger = MYLOG_LOG_NAME("system");

// 记录fiber id
static std::atomic<uint64_t> s_fiber_id = {0};
// 记录总的fiber数量
static std::atomic<uint64_t> s_fiber_count = {0};
// 当前线程的主fiber
static thread_local Fiber* t_main_fiber = nullptr;
// 管理当前线程的fiber
static thread_local Fiber::ptr t_current_fiber = nullptr;
// 协程的栈大小分配器
static mylog::ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    mylog::Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size in bytes");

// 内存分配器
class MallocStackAllocator {
   public:
    static void* Alloc(size_t size) { return malloc(size); }

    static void Dealloc(void* vp, size_t size) { return free(vp); }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
    m_state = State::EXEC;
    // 设置线程上下文
    SetThis(this);

    if (getcontext(&m_ctx)) {
        // 获取上下文失败，使用断言异常来改变程序状态
        MYSERVER_ASSERT_MSG(false, "getcontext failed");
    }

    ++s_fiber_count;

    MYLOG_LOG_DEBUG(g_logger) << "Fiber::Fiber ";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize) : m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    // 设置协程的栈大小，支持配置
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (!m_stack) {
        MYSERVER_ASSERT_MSG(false, "malloc failed");
    }

    // 设置线程上下文
    if (getcontext(&m_ctx)) {
        MYSERVER_ASSERT_MSG(false, "getcontext failed");
    }

    // 关联上下文
    m_ctx.uc_link          = &(t_current_fiber->m_ctx);  // 关联到当前线程的主协程
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    MYLOG_LOG_DEBUG(g_logger) << "Fiber::Fiber id: " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        MYSERVER_ASSERT(m_state == State::TERM || m_state == State::EXCEPT || m_state == State::INIT);

        // 释放栈空间
        StackAllocator::Dealloc(m_stack, m_stacksize);
        m_stack = nullptr;
    } else {
        // 栈空间没有分配，说明是主协程，不用释放
        MYSERVER_ASSERT(!m_cb);
        MYSERVER_ASSERT(m_state == State::EXEC);

        Fiber* cur = t_main_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }

    MYLOG_LOG_DEBUG(g_logger) << "Fiber::~Fiber id: " << m_id;
}

// 重置协程函数并重置状态(初始或结束)
bool Fiber::reset(std::function<void()> cb) {
    // 协程状态检查
    MYSERVER_ASSERT(m_stack);
    MYSERVER_ASSERT(m_state == State::TERM || m_state == State::EXCEPT || m_state == State::INIT);

    // 重新设置上下文
    m_cb = cb;
    if (getcontext(&m_ctx)) {
        MYSERVER_ASSERT_MSG(false, "getcontext failed");
        return false;
    }

    // m_ctx.uc_link          = nullptr;
    // 设置返回主协程
    m_ctx.uc_link          = &(t_current_fiber->m_ctx);
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    // 切换到运行态
    m_state = State::INIT;
    return true;
}

// 进入 fiber，切换到当前协程执行
void Fiber::swapIn() {
    SetThis(this);
    MYSERVER_ASSERT(m_state != State::EXEC);
    m_state = State::EXEC;

    if (swapcontext(&(t_current_fiber->m_ctx), &m_ctx)) {
        MYSERVER_ASSERT_MSG(false, "swapcontext failed");
    }
}

// 离开 fiber，切换到后台执行
void Fiber::swapOut() {
    SetThis(t_current_fiber.get());

    if (swapcontext(&m_ctx, &(t_current_fiber->m_ctx))) {
        MYSERVER_ASSERT_MSG(false, "swapcontext failed");
    }
}

// 获取当前 fiber
void Fiber::SetThis(Fiber* ptr) { t_main_fiber = ptr; }

// 设置当前 fiber
Fiber::ptr Fiber::GetThis() {
    if (t_main_fiber) {
        return t_main_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    MYSERVER_ASSERT(t_main_fiber == main_fiber.get());
    t_current_fiber = main_fiber;
    return t_main_fiber->shared_from_this();
}

// 协程切换到后台，并且设置为 ready 状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state   = State::READY;
    cur->swapOut();
}

// 协程切换到后台，并且设置为 hold 状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state   = State::HOLD;
    cur->swapOut();
}

// 获取当前 fiber 总数
uint64_t Fiber::TotalFibers() { return s_fiber_count; }

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    MYSERVER_ASSERT(cur);

    // 设置当前 fiber
    try {
        cur->m_cb();
        cur->m_cb    = nullptr;
        cur->m_state = State::TERM;  // 协程结束
    } catch (const std::exception& e) {
        cur->m_state = State::EXCEPT;  // 协程异常
        MYLOG_LOG_ERROR(g_logger) << "exception caught in fiber: " << e.what();
    } catch (...) {
        cur->m_state = State::EXCEPT;  // 协程异常
        MYLOG_LOG_ERROR(g_logger) << "unknown exception caught in fiber";
    }

    // 设置返回上下文
    // auto main_fiber = t_main_fiber;
    // if (main_fiber) {
    //     main_fiber->swapIn();
    // }
}

uint64_t Fiber::GetFiberId() {
    if (t_main_fiber) {
        return t_main_fiber->getId();
    }

    return 0;
}

}  // namespace myserver