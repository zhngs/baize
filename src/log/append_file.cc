#include "log/append_file.h"

#include <assert.h>

#include "log/logger.h"

namespace baize
{

namespace log
{

AppendFile::AppendFile(const char* filename)
  : file_(::fopen(filename, "ae")), written_(0)
{
    assert(file_);
    ::setbuffer(file_, buffer_, kBufferSize);
}

AppendFile::~AppendFile() { ::fclose(file_); }

size_t AppendFile::write(const char* logline, size_t len)
{
    return ::fwrite_unlocked(logline, 1, len, file_);
}

void AppendFile::Flush()
{
    // ::fflush_unlocked(file_);
    ::fflush(file_);
}

void AppendFile::Append(const char* logline, size_t len)
{
    size_t wn = 0;
    while (wn != len) {
        size_t remain = len - wn;
        size_t n = write(logline + wn, remain);
        if (n != remain) {
            int err = ferror(file_);
            if (err) {
                fprintf(stderr,
                        "AppendFile::append() failed %s\n",
                        strerror_tl(err));
                break;
            }
        }
        wn += n;
    }
    written_ += wn;
}

}  // namespace log

}  // namespace baize