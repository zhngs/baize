#include "log/LogStream.h"

using namespace baize;

const int klogBufferLen = 4096;
thread_local char g_logBuffer[klogBufferLen];

log::LogStream::LogStream() : cur_(g_logBuffer) {}

void log::LogStream::append(const void* buf, int len)
{
    if (cur_ - g_logBuffer + len <= klogBufferLen) {
        memcpy(cur_, buf, len);
        cur_ += len;
    }
}

const char* log::LogStream::getLogBuffer() { return g_logBuffer; }

int log::LogStream::getContentLength()
{
    return static_cast<int>(cur_ - g_logBuffer);
}

log::LogStream& log::LogStream::operator<<(bool v)
{
    append(v ? "1" : "0", 1);
    return *this;
}

log::LogStream& log::LogStream::operator<<(char v)
{
    append(&v, 1);
    return *this;
}

log::LogStream& log::LogStream::operator<<(short v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%hd", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned short v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%hu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(int v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%d", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned int v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%u", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(long v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%ld", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned long v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%lu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(long long v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%lld", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned long long v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%llu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(float v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%g", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(double v)
{
    int len = snprintf(cur_, klogBufferLen - getContentLength(), "%g", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(const char* v)
{
    if (v) {
        append(v, static_cast<int>(strlen(v)));
    } else {
        append("(null)", 6);
    }
    return *this;
}

log::LogStream& log::LogStream::operator<<(const string& v)
{
    append(v.data(), static_cast<int>(v.length()));
    return *this;
}
