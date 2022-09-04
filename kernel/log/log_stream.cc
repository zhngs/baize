#include "log/log_stream.h"

#include "stdarg.h"

namespace baize
{

namespace log
{

const int klogBufferLen = 65536;
thread_local char g_logbuffer[klogBufferLen];

char g_extbuf[klogBufferLen];

void StringAppend(string& s, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_extbuf, sizeof(g_extbuf), fmt, args);
    va_end(args);
    s += g_extbuf;
    s += '\n';
}

string DumpHexFormat(StringPiece s)
{
    char* pos = g_extbuf;
    const uint8_t* data = s.data_uint8();
    for (int i = 0; i < s.size(); ++i) {
        if (i % 16 == 0) {
            *pos = '\n';
            pos++;
        } else if (i % 8 == 0) {
            *pos = ' ';
            pos++;
        }
        snprintf(pos, 4, "%.2x ", data[i]);
        pos += 3;
    }
    *pos = 0;
    return g_extbuf;
}

string DumpHex(StringPiece s)
{
    const uint8_t* data = s.data_uint8();
    for (int i = 0; i < s.size(); ++i) {
        // NOTE: n must be 3 because snprintf adds a \0 after printed chars.
        snprintf(g_extbuf + (i * 2), 3, "%.2x", data[i]);
    }
    return g_extbuf;
}

log::LogStream::LogStream() : cur_(g_logbuffer) {}

void log::LogStream::Append(const void* buf, int len)
{
    if (cur_ - g_logbuffer + len <= klogBufferLen) {
        memcpy(cur_, buf, len);
        cur_ += len;
    }
}

const char* log::LogStream::buffer() { return g_logbuffer; }

int log::LogStream::length() { return static_cast<int>(cur_ - g_logbuffer); }

log::LogStream& log::LogStream::operator<<(bool v)
{
    if (v) {
        Append("true", 4);
    } else {
        Append("false", 5);
    }
    return *this;
}

log::LogStream& log::LogStream::operator<<(char v)
{
    Append(&v, 1);
    return *this;
}

log::LogStream& log::LogStream::operator<<(short v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%hd", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned short v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%hu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(int v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%d", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned int v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%u", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(long v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%ld", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned long v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%lu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(long long v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%lld", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(unsigned long long v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%llu", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(float v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%g", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(double v)
{
    int len = snprintf(cur_, klogBufferLen - length(), "%g", v);
    cur_ += len;
    return *this;
}

log::LogStream& log::LogStream::operator<<(const char* v)
{
    if (v) {
        Append(v, static_cast<int>(strlen(v)));
    } else {
        Append("(null)", 6);
    }
    return *this;
}

log::LogStream& log::LogStream::operator<<(const string& v)
{
    Append(v.data(), static_cast<int>(v.length()));
    return *this;
}

log::LogStream& log::LogStream::operator<<(const StringPiece& v)
{
    Append(v.data(), v.size());
    return *this;
}

template <typename T>
log::Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic<T>::value == true,
                  "Must be arithmetic type");

    length_ = snprintf(buf_, sizeof(buf_), fmt, val);
    assert(static_cast<size_t>(length_) < sizeof(buf_));
}

// Explicit instantiations
template log::Fmt::Fmt(const char* fmt, char);
template log::Fmt::Fmt(const char* fmt, short);
template log::Fmt::Fmt(const char* fmt, unsigned short);
template log::Fmt::Fmt(const char* fmt, int);
template log::Fmt::Fmt(const char* fmt, unsigned int);
template log::Fmt::Fmt(const char* fmt, long);
template log::Fmt::Fmt(const char* fmt, unsigned long);
template log::Fmt::Fmt(const char* fmt, long long);
template log::Fmt::Fmt(const char* fmt, unsigned long long);
template log::Fmt::Fmt(const char* fmt, float);
template log::Fmt::Fmt(const char* fmt, double);

}  // namespace log

}  // namespace baize