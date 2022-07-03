#ifndef BAIZE_LOGSTREAM_H
#define BAIZE_LOGSTREAM_H

#include "util/types.h"

namespace baize
{
    
namespace log
{
    
class LogStream //noncopy
{
public:
    LogStream();
    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    LogStream& operator<<(bool v);
    LogStream& operator<<(char v);
    LogStream& operator<<(short v);
    LogStream& operator<<(unsigned short v);
    LogStream& operator<<(int v);
    LogStream& operator<<(unsigned int v);
    LogStream& operator<<(long v);
    LogStream& operator<<(unsigned long v);
    LogStream& operator<<(long long v);
    LogStream& operator<<(unsigned long long v);
    LogStream& operator<<(float v);
    LogStream& operator<<(double v);

    LogStream& operator<<(const char* v);
    LogStream& operator<<(const string& v);


    void append(const void* buf, int len);
    const char* getLogBuffer();
    int getContentLength();
private:
    char* cur_;
};

} // namespace log

} // namespace baize


#endif //BAIZE_LOGSTREAM_H