#include "log.h"

#include <string.h>
#include <time.h>

#include <functional>
#include <map>

namespace myserver {

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

LogEventWrap::LogEventWrap(LogEvent::ptr e) :m_event(e) {
    
}
LogEventWrap::~LogEventWrap() {
    m_event -> getLogger() -> log(m_event->getLevel(), m_event);
}
std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

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
        os << logger->getName();
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

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id,
                   uint64_t time)
    : m_logger(logger), m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time) {}

Logger::Logger(const std::string& name) : m_name(name), m_level(LogLevel::DEBUG) {
    // 定义常见日志格式
    // m_formatter.reset(new LogFormatter("%d  [%p]  < %f : %l >    %m  %n"));
    // m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T %t %T %F %T[%p]%T[%c]%T %f:%l %T %m%n"));
    m_formatter.reset(new LogFormatter("%d%T%t %T%F%T[%p]%T[%c]%T%f:%l %T%m%n"));
}

// TODO
LogAppender::~LogAppender() {
    // 实现可以为空
}

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

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    // 日志等级覆盖
    if (level >= m_level) {
        // 获得指向自己的指针
        auto self = shared_from_this();
        for (auto& i : m_appenders) {
            i->log(self, level, event);
        }
    }
}

//  Logger::志级别方法
void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) {}

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

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) { init(); }

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
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ") \n";
    }

    // std::cout << m_items.size() << " ##\n";
}

}  // namespace myserver
