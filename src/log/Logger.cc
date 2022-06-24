#include "log/Logger.h"
#include "time/Timestamp.h"

using namespace baize;

log::Logger::LogLevel log::Logger::logLevel_ = []{
    return log::Logger::LogLevel::TRACE;
}();

const char* logLevelName[log::Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO ",
    "WARN ",
    "ERROR ",
    "FATAL ",
};

log::Logger::Logger(const char* filename, int line, const char* func, LogLevel level)
  : filename_(filename),
    line_(line),
    func_(func),
    logStream_(std::cout)
{
    logStream_ << logLevelName[level];
    logStream_ << time::Timestamp::now().toFormatString() << " [ ";
}

log::Logger::~Logger()
{
    logStream_ << " ] " << filename_ << " : " << func_ << " : " << line_ << "\n";
}