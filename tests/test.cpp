#include <iostream>

// #include "src/log.h"
#include "log.h"

int main(int argc, char** argv) {
    myserver::Logger::ptr logger(new myserver::Logger);
    logger->addAppender(myserver::LogAppender::ptr(new myserver::StdoutLogAppender));

    myserver::LogEvent::ptr event(new myserver::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
    event->getSS() << "ss hello myserver log";
    logger->log(myserver::LogLevel::DEBUG, event);

    std::cout << "hello myserver log!"
              << "\n";

    return 0;
}