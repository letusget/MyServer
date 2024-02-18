#include <iostream>

// #include "src/log.h"
#include <iostream>
#include <thread>

#include "log.h"
#include "util.h"

int main(int argc, char** argv) {
    mylog::Logger::ptr logger(new mylog::Logger);
    logger->addAppender(mylog::LogAppender::ptr(new mylog::StdoutLogAppender));

    mylog::FileLogAppender::ptr file_appender(new mylog::FileLogAppender("./log.txt"));

    mylog::LogFormatter::ptr fmt(new mylog::LogFormatter("%d %T %p %T %m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(mylog::LogLevel::ERROR);
    logger->addAppender(file_appender);

    // mylog::LogEvent::ptr event(
    //     new mylog::LogEvent(__FILE__, __LINE__, 0, mylog::GetThreadId(), mylog::GetFiberId(), time(0)));
    // event->getSS() << "hello mylog log";
    // logger->log(mylog::LogLevel::DEBUG, event);

    std::cout << "hello mylog log!"
              << "\n";

    MYLOG_LOG_INFO(logger) << "test info log";
    MYLOG_LOG_DEBUG(logger) << "test debug log";

    MYLOG_LOG_FMT_ERROR(logger, "test error fmt log %s", "hello");

    auto tt = mylog::LoggerMgr::GetInstance()->getLogger("xx");
    MYLOG_LOG_INFO(tt) << "xxxx";

    return 0;
}