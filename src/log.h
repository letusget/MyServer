#ifndef __MYSERVER_LOG_H_
#define __MYSERVER_LOG_H_

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "singleton.h"

// 写入level级别的流式日志
#define MYSERVER_LOG_LEVEL(logger, level)                                                                             \
    if (logger->getLevel() <= level)                                                                                  \
    myserver::LogEventWrap(                                                                                           \
        myserver::LogEvent::ptr(new myserver::LogEvent(logger, level, __FILE__, __LINE__, 0, myserver::GetThreadId(), \
                                                       myserver::GetFiberId(), time(0))))                             \
        .getSS()

// 使用logger写入debug级别的流式日志
#define MYSERVER_LOG_DEBUG(logger) MYSERVER_LOG_LEVEL(logger, myserver::LogLevel::DEBUG)
// 使用logger写入info级别的流式日志
#define MYSERVER_LOG_INFO(logger) MYSERVER_LOG_LEVEL(logger, myserver::LogLevel::INFO)
// 使用logger写入warn级别的流式日志
#define MYSERVER_LOG_WARN(logger) MYSERVER_LOG_LEVEL(logger, myserver::LogLevel::WARN)
// 使用logger写入error级别的流式日志
#define MYSERVER_LOG_ERROR(logger) MYSERVER_LOG_LEVEL(logger, myserver::LogLevel::ERROR)
// 使用logger写入fatal级别的流式日志
#define MYSERVER_LOG_FATAL(logger) MYSERVER_LOG_LEVEL(logger, myserver::LogLevel::FATAL)

// 使用logger写入level级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                               \
    if (logger->getLevel() <= level)                                                                                  \
    myserver::LogEventWrap(                                                                                           \
        myserver::LogEvent::ptr(new myserver::LogEvent(logger, level, __FILE__, __LINE__, 0, myserver::GetThreadId(), \
                                                       myserver::GetFiberId(), time(0))))                             \
        .getEvent()                                                                                                   \
        ->format(fmt, __VA_ARGS__)

// 使用logger写入debug级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_DEBUG(logger, fmt, ...) \
    MYSERVER_LOG_FMT_LEVEL(logger, myserver::LogLevel::DEBUG, fmt, __VA_ARGS__)
// 使用logger写入info级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_INFO(logger, fmt, ...) \
    MYSERVER_LOG_FMT_LEVEL(logger, myserver::LogLevel::INFO, fmt, __VA_ARGS__)
// 使用logger写入warn级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_WARN(logger, fmt, ...) \
    MYSERVER_LOG_FMT_LEVEL(logger, myserver::LogLevel::WARN, fmt, __VA_ARGS__)
// 使用logger写入error级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_ERROR(logger, fmt, ...) \
    MYSERVER_LOG_FMT_LEVEL(logger, myserver::LogLevel::ERROR, fmt, __VA_ARGS__)
// 使用logger写入fatal级别的日志 (格式化, printf)
#define MYSERVER_LOG_FMT_FATAL(logger, fmt, ...) \
    MYSERVER_LOG_FMT_LEVEL(logger, myserver::LogLevel::FATAL, fmt, __VA_ARGS__)

namespace myserver {

// 提前声明(否则在LogAppender中拿不到该类)
class Logger;

// 方便其他类确定日志等级
class LogLevel {
   public:
    enum Level { UNKNOWN = 0, DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };
    static const char* ToString(LogLevel::Level level);
};

// 日志事件
class LogEvent {
   public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time);
    //  ~LogEvent();

    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    const std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    //  使用可变参数 ...
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

   private:
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    // 日志文件名
    const char* m_file = nullptr;
    // 日志行号
    int32_t m_line = 0;
    // 累计耗时：程序启动到现在的时间长度(毫秒数)
    uint32_t m_elapse = 0;
    // 线程ID
    uint32_t m_threadId = 0;
    // 协程ID
    uint32_t m_fiberId = 0;
    // 时间戳
    uint64_t m_time = 0;
    // 日志内容
    std::stringstream m_ss;
};

// 实现LogEvent可以将自己析构时写入logger
class LogEventWrap {
   public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream& getSS();
    LogEvent::ptr getEvent() const { return m_event; }

   private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
   public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);
    //  ~LogFormatter();

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    // 解析日志格式
    void init();

   public:
    // 虚子类 用于定义各种日志格式
    class FormatItem {
       public:
        typedef std::shared_ptr<FormatItem> ptr;
        //   FormatItem(const std::string& fmt = "") {};
        virtual ~FormatItem(){};
        // 直接解析到 输出流中
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                            LogEvent::ptr event) = 0;
    };

    // 具体的子类
   private:
    // 当前日志格式
    std::string m_pattern;
    // 各种日志格式
    std::vector<FormatItem::ptr> m_items;
};

// 日志输出地
class LogAppender {
   public:
    typedef std::shared_ptr<LogAppender> ptr;
    //  LogAppender();
    virtual ~LogAppender();
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

   protected:
    // 针对哪些日志的等级
    LogLevel::Level m_level = LogLevel::DEBUG;  // 需要初始化
    // 处理不同日志格式
    LogFormatter::ptr m_formatter;
};

// 日志输出器
class Logger : public std::enable_shared_from_this<Logger> {
   public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");
    //  ~Logger();

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
    const std::string getName() const { return m_name; }

   private:
    // Appender集合
    std::list<LogAppender::ptr> m_appenders;
    // 日志名称
    std::string m_name;
    // 日志器的级别
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
   public:
    //  StdoutLogAppender();
    //  ~StdoutLogAppender();

    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

   private:
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
   public:
    //  FileLogAppender();
    //  ~FileLogAppender();

    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    // 文件的重复打开, 打开成功返回true
    bool reopen();

   private:
    std::string m_filename;
    std::ofstream m_filestream;
};

// 日志管理器
class LoggerManager {
   public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);
    void init();

   private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef myserver::Singleton<LoggerManager> LoggerMgr;

}  // namespace myserver

#endif