#include <iostream>
#include <thread>

#include "log.h"

void basic_use_mylog() {
    mylog::Logger::ptr test_log = MYLOG_LOG_NAME("test_log");
    MYLOG_LOG_DEBUG(test_log) << "DEBUG log";
    MYLOG_LOG_INFO(test_log) << "INFO log";
    MYLOG_LOG_WARN(test_log) << "WARN log";
    MYLOG_LOG_ERROR(test_log) << "ERROR log";
    MYLOG_LOG_FATAL(test_log) << "FATAL log";
}
int main(int argc, char** argv) {
    mylog::Logger::ptr logger(new mylog::Logger);
    logger->addAppender(mylog::LogAppender::ptr(new mylog::StdoutLogAppender));
    mylog::FileLogAppender::ptr file_appender(new mylog::FileLogAppender("./log.txt"));
    mylog::LogFormatter::ptr fmt(new mylog::LogFormatter("%d %T %p %T %m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(mylog::LogLevel::ERROR);
    logger->addAppender(file_appender);

    std::cout << "hello mylog !"
              << "\n";

    MYLOG_LOG_INFO(logger) << "test info log";
    MYLOG_LOG_DEBUG(logger) << "test debug log";
    MYLOG_LOG_FMT_ERROR(logger, "test error fmt log %s", "hello");

    auto tt = mylog::LoggerMgr::GetInstance()->getLogger("xx");
    MYLOG_LOG_INFO(tt) << "xxxx";

    std::cout << "\n================================================\n\n";
    basic_use_mylog();

    return 0;
}