#include "config.h"

myserver::ConfigVar<int>::ptr g_int_value_config = myserver::Config::Lookup("system.port", (int)8080, "system port");
myserver::ConfigVar<float>::ptr g_float_value_config =
    myserver::Config::Lookup("system.value", (float)6.16f, "system value");

int main(int argc, char** argv) {
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << g_int_value_config->getValue();
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << g_float_value_config->getValue();

    return 0;
}