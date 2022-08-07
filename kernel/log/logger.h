#ifndef BAIZE_LOGGER_H_
#define BAIZE_LOGGER_H_
// copy from muduo and make some small changes

#include "log/log_stream.h"

namespace baize
{

namespace log
{

const char* strerror_tl(int savedErrno);

class Logger  // noncopyable
{
public:
    using OutputFunc = std::function<void(const char*, int)>;
    using FlushFunc = std::function<void()>;
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(const char* filename,
           int line,
           const char* func,
           LogLevel level,
           int savedErrno);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // getter
    LogStream& stream() { return stream_; }
    static LogLevel loglevel() { return loglevel_; };

    // setter
    static void set_loglevel(LogLevel level) { loglevel_ = level; };
    static void set_output(OutputFunc);
    static void set_flush(FlushFunc);

private:
    void FormatTime();

    static LogLevel loglevel_;

    const char* filename_;
    int line_;
    const char* func_;
    LogLevel level_;
    LogStream stream_;
};

}  // namespace log

#define LOG_TRACE                                      \
    if (log::Logger::TRACE >= log::Logger::loglevel()) \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::TRACE, 0).stream()
#define LOG_DEBUG                                      \
    if (log::Logger::DEBUG >= log::Logger::loglevel()) \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::DEBUG, 0).stream()
#define LOG_INFO                                      \
    if (log::Logger::INFO >= log::Logger::loglevel()) \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::INFO, 0).stream()
#define LOG_WARN \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::WARN, 0).stream()
#define LOG_ERROR \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::ERROR, 0).stream()
#define LOG_FATAL \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::FATAL, 0).stream()
#define LOG_SYSERR                                                       \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::ERROR, errno) \
        .stream()
#define LOG_SYSFATAL                                                     \
    log::Logger(__FILE__, __LINE__, __func__, log::Logger::FATAL, errno) \
        .stream()

}  // namespace baize

#endif  // BAIZE_LOGGER_H