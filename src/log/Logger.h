#ifndef BAIZE_LOGGER_H
#define BAIZE_LOGGER_H

#include "util/noncopyable.h"

namespace baize
{

namespace log
{

class Logger: util::noncopyable
{
public:
    Logger(const char* filename, int line, const char* func);
    ~Logger();
private:
    const char* filename_;
    int line_;
    const char* func_;
};

#define LOG_INFO Logger(__FILE__, __LINE__, __func__)

} //namespace log

} //namespace baize

#endif //BAIZE_LOGGER_H