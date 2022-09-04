#ifndef BAIZE_LOGSTREAM_H_
#define BAIZE_LOGSTREAM_H_

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace log
{
class Fmt  // : noncopyable
{
public:
    template <typename T>
    Fmt(const char* fmt, T val);

    const char* data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

class LogStream  // noncopy
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
    LogStream& operator<<(const StringPiece& v);

    void Append(const void* buf, int len);

    // getter
    const char* buffer();
    int length();

private:
    char* cur_;
};

void StringAppend(string& s, const char* fmt, ...);
string DumpHexFormat(StringPiece s);
string DumpHex(StringPiece s);

}  // namespace log

}  // namespace baize

#endif  // BAIZE_LOGSTREAM_H