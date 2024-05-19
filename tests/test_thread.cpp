#include "myserver.h"
#include <time.h>

mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

void fun1() {
    // do something
    MYLOG_LOG_INFO(g_logger) << "name: " << myserver::Thread::GetName()
                             << " this.name: " << myserver::Thread::GetThis()->getName()
                             << " id: " << mylog::GetThreadId()
                             << " this.id: " << myserver::Thread::GetThis()->getId()
                             << " this.id: " << myserver::Thread::GetThis()->getId() << std::endl;
}

void fun2() {
    // do something
}
int main() {
    MYLOG_LOG_INFO(g_logger) << "main thread test start";
    std::vector<myserver::Thread::ptr> threads;
    for (int i = 0; i < 10; i++) {
        myserver::Thread::ptr t(new myserver::Thread(&fun1, "name_ " + std::to_string(i)));
        threads.push_back(t);
    }

    sleep(20);

    for (int i = 0; i < 10; i++) {
        threads[i]->join();
    }

    MYLOG_LOG_INFO(g_logger) << "main thread test end";
    return 0;
}