#include <assert.h>

#include "myserver.h"

static mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

void test_assert() {
    MYLOG_LOG_INFO(g_logger) << myserver::BacktraceToString(10, 2, "\t");

    MYLOG_LOG_INFO(g_logger) << "test_assert()";
    // MYSERVER_ASSERT(false);
    MYSERVER_ASSERT_MSG(false, "This is a test message");
}

int main() {
    // assert(true);

    // test_util: tests/test_util.cpp:7: int main(): Assertion `false' failed.
    // assert(false); // This line will cause a runtime error.

    test_assert();

    return 0;
}