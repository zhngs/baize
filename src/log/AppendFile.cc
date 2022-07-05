#include "log/AppendFile.h"

#include "log/Logger.h"

#include <assert.h>

using namespace baize;

log::AppendFile::AppendFile(const char* filename)
  : file_(::fopen(filename, "ae")),
    writtenBytes_(0)
{
    assert(file_);
    ::setbuffer(file_, buffer_, kBufferSize);
}

log::AppendFile::~AppendFile()
{
    ::fclose(file_);
}

size_t log::AppendFile::write(const char* logline, size_t len)
{
    return ::fwrite_unlocked(logline, 1, len, file_);
}

void log::AppendFile::flush()
{
    // ::fflush_unlocked(file_);
    ::fflush(file_);
}

void log::AppendFile::append(const char* logline, size_t len)
{
    size_t written = 0;
    while (written != len) {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain) {
            int err = ferror(file_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }
    writtenBytes_ += written;
}