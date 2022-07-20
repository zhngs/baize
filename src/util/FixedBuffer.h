#ifndef BAIZE_FIXEDBUFFER_H
#define BAIZE_FIXEDBUFFER_H

#include "util/types.h"

namespace baize
{

template <int SIZE>
class FixedBuffer  // noncopyable
{
public:
    FixedBuffer() : data_(), cur_(data_) {}
    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer& operator=(const FixedBuffer&) = delete;

    void append(const char* buf, size_t len)
    {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memZero(data_, sizeof(data_)); }

    string toString() const { return string(data_, length()); }

private:
    const char* end() const { return data_ + sizeof(data_); }

    char data_[SIZE];
    char* cur_;
};

}  // namespace baize

#endif  // BAIZE_FIXEDBUFFER_H