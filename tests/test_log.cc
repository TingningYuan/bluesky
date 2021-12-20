#include "bluesky/log.h"
#include "bluesky/util.h"

//std::shared_ptr<bluesky::Logger> logger = BLUESKY_LOG_ROOT();

int main(int argc, char *argv[])
{
    std::shared_ptr<bluesky::Logger> logger(new bluesky::Logger);
    logger->add_appender(std::shared_ptr<bluesky::LogAppender>(new bluesky::StdoutLogAppender));

    std::shared_ptr<bluesky::FileLogAppender> file_appender(new bluesky::FileLogAppender("test.log"));
    std::shared_ptr<bluesky::LogFormatter> fmt(new bluesky::LogFormatter("%d%T%m%n%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    file_appender->set_formatter(fmt);
    file_appender->set_level(bluesky::LogLevel::WARN);
    logger->add_appender(file_appender);


    BLUESKY_LOG_ERROR(logger) << "logger";
    BLUESKY_LOG_ERROR(logger) << "哈哈哈哈，终于解决啦";
    BLUESKY_LOG_WARN(BLUESKY_LOG_ROOT()) << "test";
    BLUESKY_LOG_FMT_ERROR(BLUESKY_LOG_ROOT(), "test macro fmt error %s", "aa");

    BLUESKY_LOG_DEBUG(BLUESKY_LOG_ROOT()) << "log root";
    BLUESKY_LOG_DEBUG(BLUESKY_LOG_ROOT()) << "SECOND log root";
    return 0;

}
