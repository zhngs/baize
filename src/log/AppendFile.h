#ifndef BAIZE_APPENDFILE_H
#define BAIZE_APPENDFILE_H

#include <stdio.h>

namespace baize
{

namespace log
{

class AppendFile //noncopyable
{
public:
    static const int kBufferSize = 64 * 1024;

    explicit AppendFile(const char* filename);
    ~AppendFile();
    AppendFile(const AppendFile&) = delete;
    AppendFile& operator=(const AppendFile&) = delete;

    void append(const char* logline, size_t len);
    void flush();
    off_t getWrittenBytes() const { return writtenBytes_; }
private:
    size_t write(const char* logline, size_t len);

    FILE* file_;
    char buffer_[kBufferSize];
    off_t writtenBytes_;
};
    
} // namespace log
    
} // namespace baize


#endif //BAIZE_APPENDFILE_H