#include "myserver.h"

static mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

void run_in_fiber() {
    MYLOG_LOG_INFO(g_logger) << "run_in_fiber begin\n";
    // 这里如果调用swapOut() 会断言导致异常
    myserver::Fiber::YieldToHold();

    MYLOG_LOG_INFO(g_logger) << "run_in_fiber end\n";
}

int main(int argc, char* argv[]) {
    myserver::Fiber::GetThis();

    MYLOG_LOG_INFO(g_logger) << "main begin\n";
    myserver::Fiber::ptr fiber(new myserver::Fiber(run_in_fiber));

    fiber->swapIn();

    MYLOG_LOG_INFO(g_logger) << "main after swapIn\n";
    fiber->swapIn();

    MYLOG_LOG_INFO(g_logger) << "main end\n";

    return 0;
}