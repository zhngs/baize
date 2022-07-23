#include "log/Logger.h"

#include "runtime/EventLoop.h"
#include "thread/Thread.h"
#include "time/Timestamp.h"

using namespace baize;

thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_last_second;

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
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

const char* logLevelName[log::Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
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
    formatTime();
    logStream_ << logLevelName[level_];
    if (savedErrno != 0) {
        logStream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno
                   << ") ";
    }
}

log::Logger::~Logger()
{
    logStream_ << " - " << filename_ << ":" << line_ << ":" << func_ << " - "
               << "routine" << runtime::getCurrentRoutineId() << " - "
               << thread::getCurrentThreadName() << ":"
               << thread::getCurrentTidString() << "\n";
    g_output(logStream_.getLogBuffer(), logStream_.getContentLength());
    if (level_ == LogLevel::FATAL) {
        g_flush();
        abort();
    }
}

void log::Logger::formatTime()
{
    time::Timestamp now = time::Timestamp::now();
    int64_t now_us = now.getUs();
    time_t seconds = static_cast<time_t>(now_us / time::Timestamp::kusPerSec);
    int us = static_cast<int>(now_us % time::Timestamp::kusPerSec);
    if (seconds != t_last_second) {
        t_last_second = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time);
        int len = snprintf(t_time,
                           sizeof(t_time),
                           "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900,
                           tm_time.tm_mon + 1,
                           tm_time.tm_mday,
                           tm_time.tm_hour,
                           tm_time.tm_min,
                           tm_time.tm_sec);
        assert(len == 17);
        (void)len;
    }

    Fmt fmt_us(".%06d ", us);
    assert(fmt_us.length() == 8);
    logStream_ << StringPiece(t_time, 17) << StringPiece(fmt_us.data(), 8);
}

void log::Logger::setOutput(OutputFunc func) { g_output = func; }

void log::Logger::setFlush(FlushFunc func) { g_flush = func; }