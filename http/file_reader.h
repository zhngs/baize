#ifndef BAIZE_FILE_READER_H_
#define BAIZE_FILE_READER_H_

#include "net/net_buffer.h"

namespace baize
{

namespace net
{

class FileReader  // noncopyable
{
public:
    FileReader(const char* filename);
    ~FileReader();
    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;

    StringPiece ReadAll();

private:
    int fd_;
    Buffer buf_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_FILE_READER_H_