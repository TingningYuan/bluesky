#ifndef __LOG_CONFIG_H__
#define __LOG_CONFIG_H__

#include "log.h"
#include "config.h"

namespace bluesky
{
    struct LogAppenderDefine
    {
        int type = 0; //1 FILE, 2 STDOUT
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine &appender) const
        {
            return type == appender.type && level == appender.level && formatter == appender.formatter && file == appender.file;
        }
    };


    
    struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &lgd) const
        {
            return name == lgd.name && level == lgd.level && formatter == lgd.formatter && appenders == lgd.appenders;
        }
        bool operator<(const LogDefine &lgd) const
        {
            return name < lgd.name;
        }
    };

    struct LogIniter
    {
    public:
        LogIniter();
    };

} //end of namespace

#endif