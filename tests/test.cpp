#include <iostream>

// #include "src/log.h"
#include <iostream>
#include <thread>

#include "log.h"
#include "util.h"

int main(int argc, char** argv) {
    myserver::Logger::ptr logger(new myserver::Logger);
    logger->addAppender(myserver::LogAppender::ptr(new myserver::StdoutLogAppender));

    // myserver::LogEvent::ptr event(
    //     new myserver::LogEvent(__FILE__, __LINE__, 0, myserver::GetThreadId(), myserver::GetFiberId(), time(0)));
    // event->getSS() << "hello myserver log";
    // logger->log(myserver::LogLevel::DEBUG, event);

    std::cout << "hello myserver log!"
              << "\n";

    MYSERVER_LOG_INFO(logger) << "test info log";
    MYSERVER_LOG_DEBUG(logger) << "test debug log";
    return 0;
}