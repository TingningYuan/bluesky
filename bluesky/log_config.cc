#include "log_config.h"

namespace bluesky
{

    template <>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> st;
            for (auto i = 0; i < node.size(); i++)
            {

                auto n = node[i];
                if (!n["name"].IsDefined())
                {

                    std::cout << "log config error: name is null, " << n << std::endl;
                    continue;
                }
                LogDefine lgd;
                lgd.name = n["name"].as<std::string>();
                std::string level;
                if (n["level"].IsDefined())
                {
                    level = n["level"].as<std::string>();
                }
                else
                {
                    level = "UNKNOW";
                }
                lgd.level = LogLevel::from_string(level);
                if (n["formatter"].IsDefined())
                {

                    lgd.formatter = n["formatter"].as<std::string>();
                }
                if (n["appenders"].IsDefined())
                {
                    std::cout << std::endl
                              << "--------Logger Appender's size: " << lgd.name << " = " << n["appenders"].size() << std::endl;
                    for (auto j = 0; j < n["appenders"].size(); j++)
                    {
                        auto app = n["appenders"][j];
                        if (!app["type"].IsDefined())
                        {
                            std::cout << "log config error: appender type is null " << app << std::endl;
                            continue;
                        }
                        std::string type = app["type"].as<std::string>();
                        LogAppenderDefine new_app;
                        if (type == "FileLogAppender")
                        {
                            new_app.type = 1;
                            if (!app["file"].IsDefined())
                            {

                                std::cout << "log config error: fileappender file is null" << app << std::endl;
                                continue;
                            }
                            new_app.file = app["file"].as<std::string>();
                            if (app["formatter"].IsDefined())
                            {
                                new_app.formatter = app["formatter"].as<std::string>();
                            }
                        }
                        else if (type == "StdoutLogAppender")
                        {
                            new_app.type = 2;
                            if (app["formatter"].IsDefined())
                            {

                                new_app.formatter = app["formatter"].as<std::string>();
                            }
                        }
                        else
                        {

                            std::cout << "log config error: appender type error" << std::endl;
                            continue;
                        }
                        lgd.appenders.push_back(new_app);
                    }
                }
                //std::cout << "$$$$$" << lgd.name << " = " << lgd.appenders.size() << std::endl;
                st.insert(lgd);
            }
            return st;
        }
    };
    template <>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine> &v)
        {
            YAML::Node node;
            std::stringstream ss;
            for (auto &lgd : v)
            {
                YAML::Node n;
                n["name"] = lgd.name;
                if (lgd.level != LogLevel::UNKNOW)
                {

                    n["level"] = LogLevel::to_string(lgd.level);
                }
                if (!lgd.formatter.empty())
                {
                    n["formatter"] = lgd.formatter;
                }
                if (!lgd.appenders.empty())
                {

                    for (auto app : lgd.appenders)
                    {
                        YAML::Node app_node;
                        if (app.type == 1)
                        {
                            app_node["type"] = "FileLogAppender";
                            app_node["file"] = app.file;
                            /*
                            if (!app.file.empty())
                            {

                                app_node["file"] = app.file;
                            }
                            else
                            {

                                app_node["file"] = " ";
                            }
                            */
                        }
                        else if (app.type == 2)
                        {

                            app_node["type"] = "StdoutLogAppender";
                        }
                        if (app.level != LogLevel::UNKNOW)
                        {
                            app_node["level"] = LogLevel::to_string(app.level);
                        }
                        if (!app.formatter.empty())
                        {
                            app_node["formatter"] = app.formatter;
                        }
                        n["appenders"].push_back(app_node);
                    }
                }
                node.push_back(n);
            }
            ss << node;
            return ss.str();
        }
    };

    std::shared_ptr<bluesky::ConfigVar<std::set<bluesky::LogDefine>>> g_log_defines =
        bluesky::Config::lookup("logs", std::set<bluesky::LogDefine>(), "logs default config");

    LogIniter::LogIniter()
    {
        g_log_defines->add_listener([](const std::set<LogDefine> &old_value,
                                       const std::set<LogDefine> &new_value)
                                    {
                                        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "log config changed";
                                        for (auto &i : new_value)
                                        {
                                            auto iter = old_value.find(i);
                                            std::shared_ptr<Logger> logger = BLUESKY_LOG_NAME(i.name);
                                            /*
                                            if (iter == old_value.end())
                                            {
                                                logger = BLUESKY_LOG_NAME(i.name);
                                            }
                                            else
                                            {
                                                if (!(i == *iter))
                                                {
                                                    logger = BLUESKY_LOG_NAME(i.name);
                                                }
                                            }
                                            */

                                            logger->set_level(i.level);
                                            if (!i.formatter.empty())
                                            {
                                                logger->set_formatter(i.formatter);
                                            }
                                            logger->clear_appender();
                                            for (auto &app : i.appenders)
                                            {
                                                std::shared_ptr<LogAppender> new_app;
                                                if (app.type == 1)
                                                {
                                                    new_app.reset(new FileLogAppender(app.file));
                                                }
                                                else if (app.type == 2)
                                                {
                                                    new_app.reset(new StdoutLogAppender);
                                                }
                                                new_app->set_level(app.level);
                                                if(!app.formatter.empty()){
                                                    LogFormatter::Ptr fmt(new LogFormatter(app.formatter));
                                                    if(!fmt->is_error())
                                                    {
                                                        new_app->set_formatter(fmt);
                                                    }else{
                                                        std::cout << "log.name = " << i.name << "  appender type = " << app.type
                                                                  << "  formatter = " << app.formatter << "  is invalid" << std::endl;
                                                    }
                                                }
                                                logger->add_appender(new_app);
                                            }
                                        }
                                        for (auto &i : old_value)
                                        {
                                            auto iter = new_value.find(i);
                                            if (iter == new_value.end())
                                            {
                                                auto logger = BLUESKY_LOG_NAME(i.name);
                                                logger->set_level(LogLevel::UNKNOW);
                                                logger->clear_appender();
                                            }
                                        }
                                    });
    }
    static LogIniter __log_initer;
} //end of namespace
