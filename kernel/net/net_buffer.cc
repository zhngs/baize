#include "net/net_buffer.h"

#include <sys/uio.h>

#include <algorithm>

#include "log/logger.h"

namespace baize
{

namespace net
{

char g_extrabuf[65536];

Buffer::Buffer(int initialSize)
  : buffer_(kCheapPrepend + initialSize),
    reader_index_(kCheapPrepend),
    writer_index_(kCheapPrepend)
{
    assert(readable_bytes() == 0);
    assert(writable_bytes() == initialSize);
    assert(prependable_bytes() == kCheapPrepend);
}

int Buffer::ReadFd(int fd)
{
    // saved an ioctl()/FIONREAD call to tell how much to read
    struct iovec vec[2];
    const int writable = writable_bytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = g_extrabuf;
    vec[1].iov_len = sizeof(g_extrabuf);
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt =
        (writable < static_cast<int>(sizeof(g_extrabuf))) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    int rn = static_cast<int>(n);
    if (rn < 0) {
        // do nothing
    } else if (rn <= writable) {
        writer_index_ += rn;
    } else {
        writer_index_ = static_cast<int>(buffer_.size());
        Append(g_extrabuf, rn - writable);
    }
    return static_cast<int>(n);
}

void Buffer::Append(const StringPiece& str) { Append(str.data(), str.size()); }
void Buffer::Append(const void* data, int len)
{
    if (len > 0) {
        EnsureWritableBytes(len);
        memcpy(write_index(), data, len);
        writer_index_ += len;
    } else if (len < 0) {
        LOG_ERROR << "len less than zero";
    }
}

void Buffer::AddReadableLength(int len)
{
    if (len > 0) {
        EnsureWritableBytes(len);
        writer_index_ += len;
    } else if (len < 0) {
        LOG_ERROR << "len less than zero";
    }
}

void Buffer::Take(int len)
{
    if (len > 0) {
        assert(len <= readable_bytes());
        if (len < readable_bytes()) {
            reader_index_ += len;
        } else {
            TakeAll();
        }
    } else if (len < 0) {
        LOG_ERROR << "len less than zero";
    }
}

void Buffer::TakeUntil(const char* pos)
{
    assert(read_index() <= pos);
    assert(pos <= write_index());
    Take(static_cast<int>(pos - read_index()));
}

void Buffer::TakeAll()
{
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
}

StringPiece Buffer::slice()
{
    return StringPiece(read_index(), readable_bytes());
}

void Buffer::EnsureWritableBytes(int len)
{
    if (writable_bytes() < len) {
        MakeSpace(len);
    }
    assert(writable_bytes() >= len);
}

void Buffer::MakeSpace(int len)
{
    if (writable_bytes() + prependable_bytes() < len + kCheapPrepend) {
        buffer_.resize(writer_index_ + len);
    } else {
        assert(kCheapPrepend < reader_index_);
        int readable = readable_bytes();
        std::copy(begin() + reader_index_,
                  begin() + writer_index_,
                  begin() + kCheapPrepend);
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
        assert(readable == readable_bytes());
    }
}

}  // namespace net

}  // namespace baize
