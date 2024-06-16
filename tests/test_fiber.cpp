#include "myserver.h"

static mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

void run_in_fiber() {
    MYLOG_LOG_INFO(g_logger) << "run_in_fiber begin\n";
    // 这里如果调用swapOut() 会断言导致异常
    myserver::Fiber::YieldToHold();

    MYLOG_LOG_INFO(g_logger) << "run_in_fiber end\n";
}

void test_fiber() {
    MYLOG_LOG_INFO(g_logger) << "main begin ...\n";
    {
        myserver::Fiber::GetThis();
        MYLOG_LOG_INFO(g_logger) << "main begin\n";
        myserver::Fiber::ptr fiber(new myserver::Fiber(run_in_fiber));

        fiber->swapIn();

        MYLOG_LOG_INFO(g_logger) << "main after swapIn\n";
        fiber->swapIn();

        MYLOG_LOG_INFO(g_logger) << "main end\n";
    }
    MYLOG_LOG_INFO(g_logger) << "main end ...\n";
}

int main(int argc, char* argv[]) {
    myserver::Thread::SetName("main");

    std::vector<myserver::Thread::ptr> threads;
    for (int i = 0; i < 3; ++i) {
        threads.push_back(myserver::Thread::ptr(new myserver::Thread(&test_fiber, "name_" + std::to_string(i))));
    }

    MYLOG_LOG_INFO(g_logger) << "\nmain before join\n";
    for (auto& t : threads) {
        t->join();
    }

    return 0;
}