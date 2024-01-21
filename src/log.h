#ifndef __MYSERVER_LOG_H_
#define __MYSERVER_LOG_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <list>
namespace myserver {

// 日志事件
class LogEvent {
   private:
    // 日志文件名
    const char* m_file = nullptr;
    // 日志行号
    int32_t m_line = 0;
    // 程序启动到现在的时间长度(毫秒数)
    uint32_t m_elapse = 0;
    // 线程ID
    uint32_t m_threadId = 0;
    // 协程ID
    uint32_t m_fiberId = 0;
    // 时间戳
    uint64_t m_time = 0;
    // 日志内容
    std::string m_content;

   public:
    LogEvent();
    ~LogEvent();

    typedef std::shared_ptr<LogEvent> ptr;
};

// 方便其他类确定日志等级
class LogLevel {
   public:
    enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };
};

// 日志格式器
class LogFormatter {
   private:
   public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter();
    ~LogFormatter();

    std::string format(LogEvent::ptr event);
};

// 日志输出地
class LogAppender {
   private:
    // 针对哪些日志的等级
    LogLevel::Level m_level;

   public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender();
    virtual ~LogAppender();
    void log(LogLevel::Level level, LogEvent::ptr event);
};

// 日志输出器
class Logger {
   private:
    // Appender集合
    std::list<LogAppender::ptr> m_appenders;
    // 日志名称
    std::string m_name;
    // 日志器的级别
    LogLevel::Level m_level;

   public:
    Logger(const std::string& name = "root");
    ~Logger();

    typedef std::shared_ptr<Logger> ptr;

    void log(LogLevel::Level level, LogEvent::ptr event);

    // 日志级别方法
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const { return m_level;}
    void setLevel(LogLevel::Level val) { m_level = val;}
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
   private:
   public:
    StdoutLogAppender();
    ~StdoutLogAppender();
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
   private:
   public:
    FileLogAppender();
    ~FileLogAppender();
};

}  // namespace myserver

#endif