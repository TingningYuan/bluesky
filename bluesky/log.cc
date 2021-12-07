#include "log.h"

namespace bluesky
{
    /*-------------LoggerManager---------------*/
    LoggerManager::LoggerManager()
    {
        root_.reset(new Logger);
        root_->add_appender(std::shared_ptr<bluesky::LogAppender>(new bluesky::StdoutLogAppender));
    }

    std::shared_ptr<Logger> LoggerManager::get_logger(const std::string &name)
    {
        for (auto iter = loggers_.begin(); iter != loggers_.end();iter++)
        {
            if(iter->first.compare(name))
            {
                return iter->second;
            }
        }
        return nullptr;
    }

    void LoggerManager::init()
    {
        loggers_["root_"] = get_root();
    }
    /*-----------LoggerManager End-------------*/


    /*--------------Logger Level--------------*/
    std::string LogLevel::to_string(LogLevel::Level level)
    {
        std::string str = "Unknown";
        switch (level)
        {
        case 1:
            str = "debug";
            break;
        case 2:
            str = "info";
            break;
        case 3:
            str = "warnning";
            break;
        case 4:
            str = "error";
            break;
        case 5:
            str = "fatal";
            break;
        }
        return str.c_str();
    }

    LogLevel::Level from_string(const std::string &str_level)
    {
        if (str_level.compare("debug") || str_level.compare("DEBUG"))
        {
            return LogLevel::Level::DEBUG;
        }
        if (str_level.compare("info") || str_level.compare("INFO"))
        {
            return LogLevel::Level::INFO;
        }
        if (str_level.compare("warnning") || str_level.compare("WARNNING"))
        {
            return LogLevel::Level::WARNNING;
        }
        if (str_level.compare("error") || str_level.compare("ERROR"))
        {
            return LogLevel::Level::ERROR;
        }
        if (str_level.compare("fatal") || str_level.compare("FATAL"))
        {
            return LogLevel::Level::FATAL;
        }
        return LogLevel::Level::DEBUG;
    }
    /*------------Logger Level End------------*/

    /*--------------Logger Event--------------*/
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const std::string& filename,
                       int32_t line, uint64_t elapse, uint32_t threadID, uint32_t fiberID, uint64_t time)
        : logger_(logger), level_(level), filename_(filename),
          line_(line), threadID_(threadID), fiberID_(fiberID), time_(time),
          elapse_(elapse)
    {}

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            ss_ << std::string(buf, len);
            free(buf);
        }
    }

    LogEventWrap::LogEventWrap(std::shared_ptr<LogEvent> event)
        :event_(event)
    {}

    LogEventWrap::~LogEventWrap()
    {
        event_->get_logger()->log(event_->get_loglevel(), event_);
    }

    std::stringstream& LogEventWrap::get_ss()
    {
        return event_->get_ss();
    }


    /*------------Logger Event End------------*/

    /*-----------------Logger-----------------*/
    Logger::Logger(const std::string &name, LogLevel::Level level)
        : name_(name), level_(level)
    {
        formater_.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::log(LogLevel::Level level, const std::shared_ptr<LogEvent> event)
    {
        if (level >= level_)
        {
            auto self = shared_from_this();
            for (auto &app : appenders_)
            {
               app->log(self, level, event);
            }
        }
    }

    /*---------------------日志级别输入--------------*/
    void Logger::debug(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::INFO, event);
    }

    void Logger::warnning(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::WARNNING, event);
    }

    void Logger::error(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::FATAL, event);
    }

    /*---------------------增删appender-------------*/
    void Logger::add_appender(std::shared_ptr<LogAppender> appender)
    {
        if(!appender->get_formatter()){
            appender->set_formatter(formater_);
        }
        appenders_.push_back(appender);
    }

    void Logger::del_appender(std::shared_ptr<LogAppender> appender)
    {
        for (auto iter = appenders_.begin();
             iter != appenders_.end(); iter++)
        {
            if (*iter == appender)
            {
                appenders_.erase(iter);
                break;
            }
        }
    }
    
    void Logger::clear_appender(std::shared_ptr<LogAppender> appender)
    {
        appenders_.clear();
    }

    /*------------------Logger End--------------*/

    /*------------------Appender----------------*/
    void LogAppender::set_formatter(std::shared_ptr<LogFormatter> formatter)
    {
        formatter_ = formatter;
        if(formatter_){
            has_formatter_ = true;
        }
    }

    void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        if (level >= get_level())
        {
            std::cout << get_formatter()->format(logger,level,event);
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename)
        : filename_(filename)
    {
        if (!filestream_.is_open())
        {
            filestream_.open(filename_);
        }
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        if (level >= get_level())
        {
            filestream_ << get_formatter()->format(logger, level, event);
        }
    }

    bool FileLogAppender::reopen()
    {
        if (filestream_.is_open())
        {
            filestream_.close();
        }
        filestream_.open(filename_);
        return filestream_.is_open();
    }

    /*----------------Appender End---------------*/

    /*----------------Formatter------------------*/
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_content();
        }
    };

    class LevelFormatItem:public LogFormatter::FormatItem
    {
        public:
        LevelFormatItem(const std::string &str="") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << LogLevel::to_string(event->get_loglevel());
        }
    };

    class ElapseFormatItem:public LogFormatter::FormatItem
    {
        public:
        ElapseFormatItem(const std::string& str="") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_elapse();
        }

    };

    class NameFormatItem:public LogFormatter::FormatItem
    {
        public:
        NameFormatItem(const std::string& str="") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_logger()->get_name();
        }
    };

    class ThreadIdFormatItem:public LogFormatter::FormatItem
    {
        public:
        ThreadIdFormatItem(const std::string& str=""){}
        
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_threadID();
        }
    };

    class FiberIdFormatItem:public LogFormatter::FormatItem
    {
        public:
        FiberIdFormatItem(const std::string& str=""){}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_fiberID();
        }
    };

    class ThreadNameFormatItem:public LogFormatter::FormatItem
    {
        public:
        ThreadNameFormatItem(const std::string& str="") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_threadname();
        }
    }; 

    class DateTimeFormatItem:public LogFormatter::FormatItem
    {
        public:
        DateTimeFormatItem(const std::string &format="%Y-%m-%d %H:%M:%S")
        :format_(format)
        {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            struct tm tm;
            time_t time = event->get_time();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), format_.c_str(), &tm);
            os << buf;
        }

        private:
            std::string format_;
    };

    class FilenameFormatItem:public LogFormatter::FormatItem
    {
        public:
        FilenameFormatItem(const std::string& str=""){}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_filename();
        }
    };
    
    class LineFormatItem:public LogFormatter::FormatItem
    {
        public:
        LineFormatItem(const std::string& str="") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->get_line();
        }
    };

    class NewLineFormatItem:public LogFormatter::FormatItem
    {
        public:
        NewLineFormatItem(const std::string& str=""){}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem:public LogFormatter::FormatItem
    {
        public:
        StringFormatItem(const std::string& str=""):str_(str)
        {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << str_;
        }

        private:
            std::string str_;
    };

    class TabFormatItem: public LogFormatter::FormatItem
    {
        public:
            TabFormatItem(const std::string &str = ""):str_(str){};
            void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
            {
                os << "\t";
            }

        private:
            std::string str_;
    };

    LogFormatter::LogFormatter(const std::string &pattern) : pattern_(pattern)
    {
        init();
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        std::stringstream ss;
        for (auto &item : items_)
        {
            item->format(ss, logger, level, event);
        }
        return ss.str();
    }

    std::ostream &LogFormatter::format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        for (auto &item : items_)
        {
            item->format(os, logger, level, event);
        }
        return os;
    }

    void LogFormatter::init()
    {
        //str,format,type
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < pattern_.size(); i++)
        {
            if (pattern_[i] != '%')
            {
                nstr.append(1, pattern_[i]);
                continue;
            }
            if ((i + 1) < pattern_.size() && pattern_[i + 1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while (n < pattern_.size())
            {
                //解析{xxxxx}
                if (fmt_status == 0 && (!isalpha(pattern_[n])) && pattern_[n] != '{' && pattern_[n] != '}')
                {
                    str = pattern_.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0 && pattern_[n] == '{')
                {
                    str = pattern_.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    n++;
                    continue;
                }
                if (fmt_status == 1 && pattern_[n] == '}')
                {
                    fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    n++;
                    break;
                }
                n++;
                if (n == pattern_.size() && str.empty())
                {
                    str = pattern_.substr(i + 1);
                }
            }
            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << pattern_ << " - " << pattern_.substr(i) << std::endl;
                error_ = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }

        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        static std::map<std::string, std::function<FormatItem::Ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::Ptr(new C(fmt));}}

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem),
#undef XX
    };

        for (auto &v : vec)
        {
            if (std::get<2>(v) == 0)
            {
                items_.push_back(FormatItem::Ptr(new StringFormatItem(std::get<0>(v))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(v));
                if (it == s_format_items.end())
                {
                    items_.push_back(FormatItem::Ptr(new StringFormatItem("<<error_format %" + std::get<0>(v) + ">>")));
                    error_ = true;
                }
                else
                {
                    items_.push_back(it->second(std::get<1>(v)));
                }
            }
        }
    }

    /*---------------Formatter End------*/
} //end of namespace
