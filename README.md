# MyServer
Linux下完备的服务器框架

## mylog
日志系统基本使用：
本日志系统使用 [yaml-cpp](https://github.com/jbeder/yaml-cpp) 解析配置，需要提前安装：
```sh
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build
cd build
cmake ..
make

# 全局安装
sudo make install
```
测试程序需要的头文件和动态库：
```sh
cp lib/libmylog.so src/log.h src/singleton.h src/util.h /path/to/test/file/
```

简单的测试程序如下：
```cpp
#include <iostream>
#include <log.h>

int main() {
        std::cout<<"Hello world!\n";
        mylog::Logger::ptr test_log = MYLOG_LOG_NAME("test_log");
        MYLOG_LOG_DEBUG(test_log) << "DEBUG log";
        MYLOG_LOG_INFO(test_log) << "INFO log";
        MYLOG_LOG_WARN(test_log) << "WARN log";
        MYLOG_LOG_ERROR(test_log) << "ERROR log";
        MYLOG_LOG_FATAL(test_log) << "FATAL log";
        return 0;
}
```
可以使用如下命令编译并执行：
```sh
# 目录结构
$ ls
build  CMakeLists.txt  libmylog.so  log.h  singleton.h  t.cpp  util.h
# 编译
$ g++ -o t t.cpp -L. -L/usr/local/lib -lmylog -lyaml-cpp -I. -Wl,-rpath,./
# 执行
$ ./t
Hello mylog!
2024-02-25 19:34:51     357858  0       [DEBUG] [test_log]      t.cpp:7         DEBUG log
2024-02-25 19:34:51     357858  0       [INFO]  [test_log]      t.cpp:8         INFO log
2024-02-25 19:34:51     357858  0       [WARN]  [test_log]      t.cpp:9         WARN log
2024-02-25 19:34:51     357858  0       [ERROR] [test_log]      t.cpp:10        ERROR log
2024-02-25 19:34:51     357858  0       [FATAL] [test_log]      t.cpp:11        FATAL log
```
**其他日志测试、配置系统使用等，参见tests目录中给出的测试程序**