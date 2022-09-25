#include "http/file_reader.h"

#include <fcntl.h>
#include <unistd.h>

#include "log/logger.h"

namespace baize
{

namespace net
{

FileReader::FileReader(const char* filename)
{
    fd_ = ::open(filename, O_RDONLY);
    if (fd_ < 0) {
        LOG_SYSERR << "file reader open failed";
    }
}

FileReader::~FileReader()
{
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

StringPiece FileReader::ReadAll()
{
    if (fd_ < 0) {
        return StringPiece();
    }

    if (!buf_.slice().empty()) {
        return buf_.slice();
    }

    while (1) {
        int rn = buf_.ReadFd(fd_);
        if (rn == 0) {
            break;
        } else if (rn < 0) {
            LOG_ERROR << "FileReader ReadAll failed";
            break;
        }
    }

    return buf_.slice();
}

}  // namespace net

}  // namespace baize
