#include "bluesky/log.h"
#include "bluesky/util.h"

//std::shared_ptr<bluesky::Logger> logger = BLUESKY_LOG_ROOT();

int main(int argc, char *argv[])
{

    std::shared_ptr<bluesky::Logger> logger(new bluesky::Logger);
    logger->add_appender(std::shared_ptr<bluesky::LogAppender>(new bluesky::StdoutLogAppender));
    std::shared_ptr<bluesky::FileLogAppender> file_appender(new bluesky::FileLogAppender("test.log"));
    std::shared_ptr<bluesky::LogFormatter> fmt(new bluesky::LogFormatter("%d%T%p%T%m%n"));
    file_appender->set_formatter(fmt);
    file_appender->set_level(bluesky::LogLevel::WARN);
    logger->add_appender(file_appender);

    BLUESKY_LOG_WARN(logger) << "test";
    BLUESKY_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    BLUESKY_LOG_DEBUG(BLUESKY_LOG_ROOT()) << "log root";
    BLUESKY_LOG_DEBUG(BLUESKY_LOG_ROOT()) << "SECOND log root";
    return 0;

}
