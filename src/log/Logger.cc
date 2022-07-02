#include "log/Logger.h"
#include "time/Timestamp.h"

using namespace baize;

thread_local char errnobuf[512];

log::Logger::LogLevel log::Logger::logLevel_ = []{
    return log::Logger::LogLevel::TRACE;
}();

const char* log::strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, errnobuf, sizeof(errnobuf));
}

const char* logLevelName[log::Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO ",
    "WARN ",
    "ERROR ",
    "FATAL ",
};

log::Logger::Logger(const char* filename, int line, const char* func, LogLevel level, int savedErrno)
  : filename_(filename),
    line_(line),
    func_(func),
    level_(level),
    logStream_(std::cout)
{
    logStream_ << logLevelName[level_];
    logStream_ << time::Timestamp::now().toFormatString();

    if (savedErrno != 0) {
        logStream_ << " " << strerror_tl(savedErrno) << " (errno=" << savedErrno << ")";
    }
    logStream_ << " [ ";
}

log::Logger::~Logger()
{
    logStream_ << " ] " << filename_ << ":" << line_ << " -> " << func_ << "\n";
    if (level_ == LogLevel::FATAL) abort();
}