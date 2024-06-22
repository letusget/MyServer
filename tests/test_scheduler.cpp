#include "myserver.h"

static mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

int main() {
    myserver::Scheduler sc;
    sc.start(); 
    
    sc.stop(); 

    return 0;
}