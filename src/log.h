#ifndef __MYSERVER_LOG_H_
#define __MYSERVER_LOG_H_

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
namespace myserver {

// 日志事件
class LogEvent {
   public:
    LogEvent();
    ~LogEvent();

    typedef std::shared_ptr<LogEvent> ptr;

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
};

// 方便其他类确定日志等级
class LogLevel {
   public:
    enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };
};

// 日志格式器
class LogFormatter {
   public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter();
    ~LogFormatter();

    std::string format(LogEvent::ptr event);

   private:
};

// 日志输出地
class LogAppender {
   public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender();
    virtual ~LogAppender();
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }

   protected:
    // 针对哪些日志的等级
    LogLevel::Level m_level;
    // 处理不同日志格式
    LogFormatter::ptr m_formatter;
};

// 日志输出器
class Logger {
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
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

   private:
    // Appender集合
    std::list<LogAppender::ptr> m_appenders;
    // 日志名称
    std::string m_name;
    // 日志器的级别
    LogLevel::Level m_level;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
   public:
    StdoutLogAppender();
    ~StdoutLogAppender();

    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

   private:
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
   public:
    FileLogAppender();
    ~FileLogAppender();

    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
    // 文件的重复打开, 打开成功返回true
    bool reopen();

   private:
    std::string m_filename;
    std::ofstream m_filestream;
};

}  // namespace myserver

#endif