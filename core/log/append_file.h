#ifndef BAIZE_APPENDFILE_H_
#define BAIZE_APPENDFILE_H_

#include <stdio.h>

namespace baize
{

namespace log
{

class AppendFile  // noncopyable
{
public:
    static const int kBufferSize = 64 * 1024;

    explicit AppendFile(const char* filename);
    ~AppendFile();
    AppendFile(const AppendFile&) = delete;
    AppendFile& operator=(const AppendFile&) = delete;

    void Append(const char* logline, size_t len);
    void Flush();

    // getter
    off_t written() const { return written_; }

private:
    size_t write(const char* logline, size_t len);

    FILE* file_;
    char buffer_[kBufferSize];
    off_t written_;
};

}  // namespace log

}  // namespace baize

#endif  // BAIZE_APPENDFILE_H