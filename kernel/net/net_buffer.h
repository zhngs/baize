#ifndef BAIZE_BUFFER_H
#define BAIZE_BUFFER_H
// copy from muduo and make some small changes

#include <vector>

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer  // copyable
{
public:
    static const int kCheapPrepend = 8;
    static const int kInitialSize = 1024;

    explicit Buffer(int initialSize = kInitialSize);

    int ReadFd(int fd);

    // find
    const char* FindCRLF() const;
    const char* Find(char ch) const;
    const char* Find(StringPiece slice) const;

    // append
    void Append(const StringPiece& str);
    void Append(const void* data, int len);

    // take
    void Take(int len);
    void TakeAll();
    void TakeUntil(const char* pos);
    void TakeCrossSpace();
    char TakeChar();

    // getter
    int readable_bytes() const { return writer_index_ - reader_index_; }
    int writable_bytes() const
    {
        return static_cast<int>(buffer_.size() - writer_index_);
    }
    int prependable_bytes() const { return reader_index_; }

    // getter
    const char* read_index() const { return begin() + reader_index_; }
    const char* write_index() const { return begin() + writer_index_; }
    char* read_index() { return begin() + reader_index_; }
    char* write_index() { return begin() + writer_index_; }

    // getter
    StringPiece debug_string_piece();

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

private:
    // getter
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    void EnsureWritableBytes(int len);
    void MakeSpace(int len);

    std::vector<char> buffer_;
    int reader_index_;
    int writer_index_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_BUFFER_H