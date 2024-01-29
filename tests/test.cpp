#include <iostream>

// #include "src/log.h"
#include <iostream>
#include <thread>

#include "log.h"
#include "util.h"

int main(int argc, char** argv) {
    myserver::Logger::ptr logger(new myserver::Logger);
    logger->addAppender(myserver::LogAppender::ptr(new myserver::StdoutLogAppender));

    myserver::FileLogAppender::ptr file_appender(new myserver::FileLogAppender("./log.txt"));

    myserver::LogFormatter::ptr fmt(new myserver::LogFormatter("%d %T %p %T %m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(myserver::LogLevel::ERROR);
    logger->addAppender(file_appender);

    // myserver::LogEvent::ptr event(
    //     new myserver::LogEvent(__FILE__, __LINE__, 0, myserver::GetThreadId(), myserver::GetFiberId(), time(0)));
    // event->getSS() << "hello myserver log";
    // logger->log(myserver::LogLevel::DEBUG, event);

    std::cout << "hello myserver log!"
              << "\n";

    MYSERVER_LOG_INFO(logger) << "test info log";
    MYSERVER_LOG_DEBUG(logger) << "test debug log";

    MYSERVER_LOG_FMT_ERROR(logger, "test error fmt log %s", "hello");

    auto tt = myserver::LoggerMgr::GetInstance()->getLogger("xx");
    MYSERVER_LOG_INFO(tt) << "xxxx";

    return 0;
}