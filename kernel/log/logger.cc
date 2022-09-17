#include "log/logger.h"

#include "runtime/event_loop.h"
#include "thread/thread.h"
#include "time/time_stamp.h"

namespace baize
{

namespace log
{

thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_last_second;

const char* log_level_name[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};
Logger::LogLevel Logger::loglevel_ = Logger::INFO;

// default output and flush
void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}
void defaultFlush() { fflush(stdout); }
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::Logger(const char* filename,
               int line,
               const char* func,
               LogLevel level,
               int savedErrno)
  : filename_(filename), line_(line), func_(func), level_(level)
{
    FormatTime();
    stream_ << log_level_name[level_];
    if (savedErrno != 0) {
        stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

Logger::~Logger()
{
    stream_ << " - " << filename_ << ":" << line_ << ":" << func_ << " - "
            << "routine" << runtime::current_routineid() << " - "
            << thread::threadname() << ":" << thread::tidstring() << "\n";
    g_output(stream_.buffer(), stream_.length());
    if (level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::FormatTime()
{
    time::Timestamp now = time::Timestamp::Now();
    int64_t now_us = now.us();
    time_t seconds = static_cast<time_t>(now_us / time::Timestamp::kUsPerSec);
    int us = static_cast<int>(now_us % time::Timestamp::kUsPerSec);
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
    stream_ << StringPiece(t_time, 17) << StringPiece(fmt_us.data(), 8);
}

void Logger::set_output(OutputFunc func) { g_output = func; }

void Logger::set_flush(FlushFunc func) { g_flush = func; }

}  // namespace log

}  // namespace baize