#include "log/Logger.h"

#include "runtime/EventLoop.h"
#include "thread/Thread.h"
#include "time/Timestamp.h"

using namespace baize;

thread_local char errnobuf[512];

void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void defaultFlush() { fflush(stdout); }

log::Logger::OutputFunc g_output = defaultOutput;
log::Logger::FlushFunc g_flush = defaultFlush;

log::Logger::LogLevel log::Logger::logLevel_ = [] {
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

log::Logger::Logger(const char* filename,
                    int line,
                    const char* func,
                    LogLevel level,
                    int savedErrno)
  : filename_(filename), line_(line), func_(func), level_(level)
{
    logStream_ << logLevelName[level_];
    logStream_ << time::Timestamp::now().toFormatString();

    if (savedErrno != 0) {
        logStream_ << " " << strerror_tl(savedErrno) << " (errno=" << savedErrno
                   << ")";
    }
    logStream_ << " [ ";
}

log::Logger::~Logger()
{
    logStream_ << " ] " << filename_ << ":" << line_ << ":" << func_ << " -> "
               << "routine" << runtime::getCurrentRoutineId() << " -> "
               << thread::getCurrentThreadName() << ":"
               << thread::getCurrentTidString() << "\n";
    g_output(logStream_.getLogBuffer(), logStream_.getContentLength());
    if (level_ == LogLevel::FATAL) {
        g_flush();
        abort();
    }
}

void log::Logger::setOutput(OutputFunc func) { g_output = func; }

void log::Logger::setFlush(FlushFunc func) { g_flush = func; }