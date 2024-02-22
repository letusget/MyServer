#include "log.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <functional>
#include <map>

#include "config.h"

namespace mylog {

// 使用宏来减少重复, 输出日志级别
const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
        // 每次使用 XX(name) 都会展开为一个 case 语句。
#define XX(name)         \
    case LogLevel::name: \
        return #name;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(name)               \
    if (str == "") {           \
        return LogLevel::name; \
    }
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
    return LogLevel::UNKNOWN;
#undef XX
}
LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {}
LogEventWrap::~LogEventWrap() { m_event->getLogger()->log(m_event->getLevel(), m_event); }

void LogEvent::format(const char* fmt, ...) {
    // 可变参数列表
    va_list al;
    va_start(al, fmt);  // 初始化
    format(fmt, al);
    va_end(al);  // 清理遍历参数列表
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    // 动态分配足够的内存来存放格式化后的字符串
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        // vasprintf格式化成功，添加到 m_ss
        m_ss << std::string(buf, len);
        free(buf);
    }
}
std::stringstream& LogEventWrap::getSS() { return m_event->getSS(); }

// 线程ID
class ThreadIdFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadIdFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

// 日志内容
class MessageFormatItem : public LogFormatter::FormatItem {
   public:
    MessageFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                        LogEvent::ptr event) override {
        os << event->getContent();
    }
};

// 协程ID
class FiberIdFormatItem : public LogFormatter::FormatItem {
   public:
    FiberIdFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

// 日志时间
class DateTimeFormatItem : public LogFormatter::FormatItem {
   public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
        // 没有指定格式时使用的默认日期格式
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                        LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);  // 线程安全方法，而不是localtime
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

   private:
    std::string m_format;
};

// 日志级别
class LevelFormatItem : public LogFormatter::FormatItem {
   public:
    LevelFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

// 时间长度
class ElapseFormatItem : public LogFormatter::FormatItem {
   public:
    ElapseFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

// 日志名称
class NameFormatItem : public LogFormatter::FormatItem {
   public:
    NameFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        // os << logger->getName();
        os << event->getLogger()->getName();
    }
};

// 文件名称
class FilenameFormatItem : public LogFormatter::FormatItem {
   public:
    FilenameFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

// 行号
class LineFormatItem : public LogFormatter::FormatItem {
   public:
    LineFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

// 文本
class StringFormatItem : public LogFormatter::FormatItem {
   public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }

   private:
    std::string m_string;
};

// 换行符
class NewLineFormatItem : public LogFormatter::FormatItem {
   public:
    NewLineFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

// 文本
class TabFormatItem : public LogFormatter::FormatItem {
   public:
    TabFormatItem(const std::string& str = "") : m_string(str) {}
    virtual void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }

   private:
    std::string m_string;
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line,
                   uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    : m_logger(logger),
      m_level(level),
      m_file(file),
      m_line(line),
      m_elapse(elapse),
      m_threadId(thread_id),
      m_fiberId(fiber_id),
      m_time(time) {}

Logger::Logger(const std::string& name) : m_name(name), m_level(LogLevel::DEBUG) {
    // 定义常见日志格式
    // m_formatter.reset(new LogFormatter("%d  [%p]  < %f : %l >    %m  %n"));
    // m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T %t %T %F %T[%p]%T[%c]%T %f:%l %T %m%n"));
    m_formatter.reset(new LogFormatter("%d%T%t %T%F%T[%p]%T[%c]%T%f:%l %T%m%n"));
}

// 虚函数必须要提供定义
// TODO
LogAppender::~LogAppender() {}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() { m_appenders.clear(); }
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    // 日志等级覆盖
    if (level >= m_level) {
        // 获得指向自己的指针
        auto self = shared_from_this();
        if (!m_appenders.empty()) {
            for (auto& i : m_appenders) {
                i->log(self, level, event);
            }
        } else if (m_root) {
            m_root->log(level, event);
        }
    }
}

void Logger::setFormatter(LogFormatter::ptr val) { m_formatter = val; }
void Logger::setFormatter(std::string& val) {
    mylog::LogFormatter::ptr new_val(new mylog::LogFormatter(val));
    if (new_val->isError()) {
        std::cout << "Logger setFormatter name = " << m_name << "value = " << val << " invalid formatter "
                  << "\n";
        return;
    }
    m_formatter = new_val;
}
LogFormatter::ptr Logger::getFormatter() { return m_formatter; }

//  Logger::志级别方法
void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

// TODO reopen() 优化
FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) { reopen(); }

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen() {
    // 重复打开的文件就先打开再关闭
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    // !! 可以将非0转1， 0保持
    return !!m_filestream;
}
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern), m_error(false) { init(); }

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

// 解析 %xx 或 %xx{xx} 同时还需要能够输出 %
void LogFormatter::init() {
    // string, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string normal_str;
    // 遍历整个pattern, 分别解析 %d形式的 和 %d{ }形式的
    for (size_t i = 0; i < m_pattern.size(); i++) {
        // 去除 %
        if (m_pattern[i] != '%') {
            // 放入 % 之后的内容
            normal_str.append(1, m_pattern[i]);
            continue;
        }

        // 非转义字符，真正的 % 的情况
        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                normal_str.append(1, '%');
                continue;
            }
        }

        // % 之后的内容
        size_t n = i + 1;
        // 记录解析状态： 1解析到{ ; 2解析到}
        int format_status   = 0;
        size_t format_begin = 0;

        std::string fstr;
        std::string fmt;
        // 遍历 % 之后的内容,找到空格结束符，匹配{}
        while (n < m_pattern.size()) {
            // if (isspace(m_pattern[n])) {
            if (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}') {
                break;
            }
            if (format_status == 0) {
                if (m_pattern[n] == '{') {
                    fstr = m_pattern.substr(i + 1, n - (i + 1));  // 跳过%
                    // 解析格式
                    format_status = 1;
                    format_begin  = n;
                    ++n;
                    continue;
                }
            }
            if (format_status == 1) {
                if (m_pattern[n] == '}') {
                    // 获取 { } 之间的子串
                    fmt           = m_pattern.substr(format_begin + 1, n - (format_begin + 1));  // 跳过%
                    format_status = 2;
                    break;
                }
            }

            ++n;
        }

        if (format_status == 0) {
            if (!normal_str.empty()) {
                // 放入 非% 的内容
                vec.push_back(std::make_tuple(normal_str, std::string(), 0));
                normal_str.clear();
            }

            // 没有解析到 {} 的情况
            fstr = m_pattern.substr(i + 1, n - (i + 1));
            vec.push_back(std::make_tuple(fstr, fmt, 1));
            i = n - 1;
        } else if (format_status == 1) {
            // 只解析到{的情况，输出异常日志
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << " \n";
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } else if (format_status == 2) {
            if (!normal_str.empty()) {
                // 放入 非% 的内容
                vec.push_back(std::make_tuple(normal_str, "", 0));
                normal_str.clear();
            }
            // 正常解析完 {}
            vec.push_back(std::make_tuple(fstr, fmt, 1));
            i = n - 1;
        }
    }

    if (!normal_str.empty()) {
        vec.push_back(std::make_tuple(normal_str, "", 0));
    }

    /**
     * 日志格式举例：%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    格式解析：
        %d{%Y-%m-%d %H:%M:%S} : %d 标识输出的是时间 {%Y-%m-%d %H:%M:%S}为时间格式，可选 DateTimeFormatItem
        %T : Tab[\t]            TabFormatItem
        %t : 线程id             ThreadIdFormatItem
        %N : 线程名称           ThreadNameFormatItem
        %F : 协程id             FiberIdFormatItem
        %p : 日志级别           LevelFormatItem
        %c : 日志名称           NameFormatItem
        %f : 文件名             FilenameFormatItem
        %l : 行号               LineFormatItem
        %m : 日志内容           MessageFormatItem
        %n : 换行符[\r\n]       NewLineFormatItem
    */

    // 初始化map：接受一个 std::string 类型的参数并返回一个指向特定 FormatItem 子类的智能指针
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C)                                                              \
    {                                                                           \
        str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

        XX("m", MessageFormatItem),  XX("p", LevelFormatItem),    XX("r", ElapseFormatItem),
        XX("c", NameFormatItem),     XX("t", ThreadIdFormatItem), XX("n", NewLineFormatItem),
        XX("d", DateTimeFormatItem), XX("f", FilenameFormatItem), XX("l", LineFormatItem),
        XX("T", TabFormatItem),      XX("F", FiberIdFormatItem),
#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            // 直接构造一个新类(可以比emplace_back少一次复制操作)
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format % " + std::get<0>(i) + " >>")));
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);

    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    init();
}
Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    logger->m_root  = m_root;
    m_loggers[name] = logger;
    return logger;
}

struct LogAppenderDefine {
    // 1 File; 2 Stdout
    int type              = 0;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file;
    }
};
struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& oth) const {
        return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == oth.appenders;
    }

    bool operator<(const LogDefine& oth) const { return name < oth.name; }
};

// 偏特化 反序列化
template <class T>
class LexicalCast<std::string, std::set<LogDefine>> {
   public:
    std::set<LogDefine> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<LogDefine> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// 偏特化 序列化
template <class T>
class LexicalCast<std::set<LogDefine>, std::string> {
   public:
    std::string operator()(const std::set<LogDefine>& v) {
        // YAML::Node node;
        // for (auto& i : v) {
        //     node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        // }
        // std::stringstream ss;
        // ss << node;
        // return ss.str();
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> vec;
        for (size_t i = 0; i < node.size(); ++i) {
            auto n = node[i];
            if (!n["name"].IsDefined()) {
                std::cout << "log config error: name is NULL - " << n << "\n";
                continue;
            }
        }

        LogDefine ld;
        ld.name  = n["name"].as<std::string>();
        ld.level = n["level"].IsDefined ? n["level"].as<std::string>() : "";
    }
};

mylog::ConfigVar<std::set<LogDefine>> g_log_defines =
    mylog::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_defines->addListener(0xF1E231,
                                   [](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value) {
                                       MYLOG_LOG_NAME(MYLOG_LOG_ROOT()) << "on_logger_conf_changed";
                                       // 只有三种情况：新增、修改、删除
                                       // 新增
                                       for (auto& i : new_value) {
                                           auto it = old_value.find(i);
                                           mylog::Logger::ptr logger;
                                           if (it == old_value.end()) {
                                               // 新增logger
                                               logger.reset(new mylog::Logger(i.name));

                                           } else {
                                               // 是否更新(发生变化)
                                               if (!(i == *it)) {
                                                   // 修改旧的logger
                                                   logger = MYLOG_LOG_NAME(i.name);
                                               }
                                           }
                                           logger->setLevel(i.level);
                                           if (!i.formatter.empty()) {
                                               logger->setFormatter(std::string(i.formatter));
                                           }

                                           logger->clearAppenders();
                                           for (auto& a : i.appenders) {
                                               mylog::LogAppender::ptr ap;
                                               if (a.type == 1) {
                                                   ap.reset(new FileLogAppender(a.file));
                                               } else if (a.type == 2) {
                                                   ap.reset(new StdoutLogAppender);
                                               }
                                               ap->setLevel(a.level);
                                               logger->addAppender(ap);
                                           }
                                       }

                                       // 删除
                                       for (auto& i : old_value) {
                                           auto it = new_value.find(i);
                                           if (it == new_value.end()) {
                                               // 删除旧的
                                               auto logger = MYLOG_LOG_NAME(i.name);
                                               // 这里使用高等级(不会用到的等级)来表示删除
                                               logger->setLevel((LogLevel::Level)100);
                                               // 清空，相当于删除，下次默认使用root打印
                                               logger->clearAppenders();
                                           }
                                       }
                                   });
    }
};
// 全局对象在main之前构造执行
static LogIniter __log_init;

void LoggerManager::init() {}

}  // namespace mylog
