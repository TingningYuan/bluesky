#ifndef __BLUESKY_LOG_H__
#define __BLUESKY_LOG_H__

#include "singleton.h"
#include "util.h"
#include "locker.h"
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <stdarg.h>



/*----------------------流式日志------------------*/
//使用logger写入日志级别为level的日志
#define BLUESKY_LOG_LEVEL(logger, level)                                                                              \
    if (logger->get_level() <= level)                                                                                 \
    bluesky::LogEventWrap(logger, std::shared_ptr<bluesky::LogEvent>(new bluesky::LogEvent(logger->get_name(), level, \
                                                                                           __FILE__,                  \
                                                                                           __LINE__, 0,               \
                                                                                           bluesky::get_threadID(),   \
                                                                                           bluesky::get_fiberID(),    \
                                                                                           time(0))))                 \
        .get_ss()

//使用logger写入日志级别为debug的日志
#define BLUESKY_LOG_DEBUG(logger) BLUESKY_LOG_LEVEL(logger, bluesky::LogLevel::DEBUG)
//使用logger写入日志级别为info的日志
#define BLUESKY_LOG_INFO(logger) BLUESKY_LOG_LEVEL(logger, bluesky::LogLevel::INFO)
//使用logger写入日志级别为warnning的日志
#define BLUESKY_LOG_WARN(logger) BLUESKY_LOG_LEVEL(logger, bluesky::LogLevel::WARN)
//使用logger写入日志级别为error的日志
#define BLUESKY_LOG_ERROR(logger) BLUESKY_LOG_LEVEL(logger, bluesky::LogLevel::ERROR)
//使用logger写入日志级别为fatal的日志
#define BLUESKY_LOG_FATAL(logger) BLUESKY_LOG_LEVEL(logger, bluesky::LogLevel::FATAL)

/*-----------------格式化 printf日志-----------------*/
#define BLUESKY_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                                \
    if (logger->get_level() <= level)                                                                                 \
    bluesky::LogEventWrap(logger, std::shared_ptr<bluesky::LogEvent>(new bluesky::LogEvent(logger->get_name(), level, \
                                                                                           __FILE__,                  \
                                                                                           __LINE__, 0,               \
                                                                                           bluesky::get_threadID(),   \
                                                                                           bluesky::get_fiberID(),    \
                                                                                           time(0))))                 \
        .get_event()                                                                                                  \
        ->format(fmt, __VA_ARGS__)

//使用logger写入日志级别为debug的日志(格式化,printf)
#define BLUESKY_LOG_FMT_DEBUG(logger, fmt, ...) BLUESKY_LOG_FMT_LEVEL(logger, bluesky::LogLevel::DEBUG, fmt, __VA_ARGS__)
//使用logger写入日志级别为info的日志(格式化,printf)
#define BLUESKY_LOG_FMT_INFO(logger, fmt, ...) BLUESKY_LOG_FMT_LEVEL(logger, bluesky::LogLevel::INFO, fmt, __VA_ARGS__)
//使用logger写入日志级别为warnning的日志(格式化,printf)
#define BLUESKY_LOG_FMT_WARN(logger, fmt, ...) BLUESKY_LOG_FMT_LEVEL(logger, bluesky::LogLevel::WARN, fmt, __VA_ARGS__)
//使用logger写入日志级别为error的日志(格式化,printf)
#define BLUESKY_LOG_FMT_ERROR(logger, fmt, ...) BLUESKY_LOG_FMT_LEVEL(logger, bluesky::LogLevel::ERROR, fmt, __VA_ARGS__)
//使用logger写入日志级别为fatal的日志(格式化,printf)
#define BLUESKY_LOG_FMT_FATAL(logger, fmt, ...) BLUESKY_LOG_FMT_LEVEL(logger, bluesky::LogLevel::FATAL, fmt, __VA_ARGS__)

//获取主日志器
#define BLUESKY_LOG_ROOT() bluesky::Singleton<bluesky::LoggerManager>::get_instance().get_root()

//获取指定名称的日志器，不存在则创建
#define BLUESKY_LOG_NAME(name) bluesky::Singleton<bluesky::LoggerManager>::get_instance().get_logger(name)


namespace bluesky
{
    class Logger;
    class LogAppender;
    class LogFormatter;
    //定义日志级别
    class LogLevel
    {
    public:
        //定义日志级别
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        //将日志级别转成文本输出
        static std::string to_string(LogLevel::Level level);

        //将文本转换成日志级别
        static Level from_string(std::string &str_level);
    };

    class LoggerManager
    {
    public:
        typedef std::shared_ptr<LoggerManager> Ptr;
        typedef Mutex MutexType;

        LoggerManager();
        std::shared_ptr<Logger> get_logger(const std::string &name);

        void init();
        std::shared_ptr<Logger> get_root() const { return root_; }
        std::string toYamlString();

    private:
        std::map<std::string, std::shared_ptr<Logger>> loggers_;
        std::shared_ptr<Logger> root_;
        MutexType mutex_;
    };

    typedef bluesky::Singleton<LoggerManager> LoggerMgr;

    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> Ptr;

        LogEvent(std::string logger_name,
                 LogLevel::Level level,
                 const std::string &filename,
                 int32_t line,
                 uint64_t elapse,
                 uint32_t threadID,
                 uint32_t fiberID,
                 uint64_t time);

        const std::string get_filename() const { return filename_; }
        const int32_t get_line() const { return line_; }
        const uint32_t get_threadID() const { return threadID_; }
        const uint32_t get_fiberID() const { return fiberID_; }
        const uint64_t get_time() const { return time_; }
        const uint64_t get_elapse() const { return elapse_; }
        const std::string get_threadname() const { return thread_name_; }
        const std::string get_content() const { return ss_.str(); }
        const std::string get_loggername() const { return logger_name_; }
        //std::shared_ptr<Logger> get_logger() const { return logger_; }
        LogLevel::Level get_loglevel() const { return level_; }
        std::stringstream &get_ss() { return ss_; }

        //格式化写入日志内容
        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);

    private:
        std::string filename_;    //文件名
        int32_t line_ = 0;        //行号
        uint32_t threadID_ = 0;   //线程名
        uint32_t fiberID_ = 0;    //协程ID
        uint64_t time_;           //时间戳
        uint64_t elapse_;         //程序启动到现在的时间ms
        std::string thread_name_; //线程名
        std::stringstream ss_;    //日志内容流
        //std::shared_ptr<Logger> logger_; //日志器
        std::string logger_name_; //日志器名称
        LogLevel::Level level_;   //日志等级
    };

    class LogEventWrap
    {
    public:
        LogEventWrap(const std::shared_ptr<Logger>& logger, const std::shared_ptr<LogEvent>& event);
        ~LogEventWrap();

        std::shared_ptr<LogEvent> get_event() const { return event_; }
        std::shared_ptr<Logger> get_logger() const { return logger_; }
        std::stringstream &get_ss();

    private:
        std::shared_ptr<LogEvent> event_;
        std::shared_ptr<Logger> logger_;
    };

    //日志器定义
    class Logger : public std::enable_shared_from_this<Logger>
    {
        friend LoggerManager;

    public:
        typedef std::shared_ptr<Logger> Ptr;
        typedef Mutex MutexType;

        Logger(const std::string &name = "root", LogLevel::Level level = LogLevel::DEBUG);

        //写入日志，指定日志的级别
        void log(LogLevel::Level level, const LogEvent::Ptr event);

        //日志级别输出
        void debug(std::shared_ptr<LogEvent> event);
        void info(std::shared_ptr<LogEvent> event);
        void warn(std::shared_ptr<LogEvent> event);
        void error(std::shared_ptr<LogEvent> event);
        void fatal(std::shared_ptr<LogEvent> event);

        //增删appender
        void add_appender(std::shared_ptr<LogAppender> appender);
        void del_appender(std::shared_ptr<LogAppender> appender);
        void clear_appender();

        const std::string &get_name() const { return name_; }
        const LogLevel::Level get_level() const { return level_; }
        void set_level(LogLevel::Level level) { level_ = level; }

        //设置formatter
        void set_formatter(std::shared_ptr<LogFormatter> &formatter);
        void set_formatter(const std::string &value);
        std::shared_ptr<LogFormatter> get_formatter();

        std::string toYamlString();

    private:
        std::string name_;
        LogLevel::Level level_;
        std::list<std::shared_ptr<LogAppender>> appenders_;
        std::shared_ptr<LogFormatter> formatter_;
        Logger::Ptr root_;
        MutexType mutex_;
    };

    //日志格式
    class LogFormatter
    {
    public:
        /**
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
        typedef std::shared_ptr<LogFormatter> Ptr;
        LogFormatter(const std::string &pattern);
        ~LogFormatter() {}
        //将LogEvent格式化字符串
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);
        std::ostream &format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);

    public:
        //具体日志格式项
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> Ptr;
            virtual ~FormatItem() {}

            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) = 0;
        };

        void init();
        bool is_error() const { return error_; }
        std::string get_pattern() { return pattern_; }

    private:
        std::string pattern_;                //日志格式
        std::vector<FormatItem::Ptr> items_; //通过日志格式解析出来的具体格式
        bool error_ = false;
    };

    //日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> Ptr;
        typedef Mutex MutexType;
        virtual ~LogAppender() {}

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) = 0;
        virtual std::string toYamlString() = 0;

    public:
        void set_formatter(std::shared_ptr<LogFormatter> formatter);
        std::shared_ptr<LogFormatter> get_formatter();
        LogLevel::Level get_level() const { return level_; }
        void set_level(LogLevel::Level level) { level_ = level; }

    public:
        LogLevel::Level level_;
        std::shared_ptr<LogFormatter> formatter_;
        bool hasFormatter_ = false;
        MutexType mutex_;
    };

    //输出到控制台
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> Ptr;
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override;
        virtual std::string toYamlString();
    };

    //输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> Ptr;

        FileLogAppender(const std::string &filename);
        virtual std::string toYamlString();
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override;

        //重新打开文件
        bool reopen();

    private:
        std::string filename_;
        std::ofstream filestream_;
        uint64_t lastTime_=0;
    };

    
} //end of namespace

#endif
