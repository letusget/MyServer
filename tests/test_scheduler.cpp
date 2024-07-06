#include "myserver.h"

static mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

void test_fiber() { MYLOG_LOG_INFO(g_logger) << "test in fiber"; }

int main() {
    MYLOG_LOG_INFO(g_logger) << "Starting scheduler";

    myserver::Scheduler sc;

    sc.schedule(test_fiber);

    sc.start();
    sc.stop();

    MYLOG_LOG_INFO(g_logger) << "Scheduler stopped";

    return 0;
}