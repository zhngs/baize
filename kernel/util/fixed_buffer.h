#ifndef BAIZE_FIXEDBUFFER_H_
#define BAIZE_FIXEDBUFFER_H_

#include "util/types.h"

namespace baize
{

template <int SIZE>
class FixedBuffer  // noncopyable
{
public:
    FixedBuffer() : data_(), cur_(data_) { bzero(); }
    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer& operator=(const FixedBuffer&) = delete;

    void append(const char* buf, size_t len)
    {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    char* data() { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }
    int capacity() { return static_cast<int>(sizeof(data_)); }

    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { MemZero(data_, sizeof(data_)); }

    string AsString() const { return string(data_, length()); }

private:
    const char* end() const { return data_ + sizeof(data_); }

    char data_[SIZE];
    char* cur_;
};

}  // namespace baize

#endif  // BAIZE_FIXEDBUFFER_H