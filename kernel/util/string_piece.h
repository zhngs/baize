#ifndef BAIZE_STRINGPIECE_H_
#define BAIZE_STRINGPIECE_H_

#include <algorithm>
#include <vector>

#include "util/types.h"

namespace baize
{

class StringPiece  // copyable
{
public:
    StringPiece();
    StringPiece(const char* str);
    StringPiece(const unsigned char* str);
    StringPiece(const string& str);
    StringPiece(const void* offset, int len);
    StringPiece(const void* begin, const void* end);

    const char* Find(char ch) const;
    const char* Find(StringPiece slice) const;

    void TrimSpace();
    std::vector<StringPiece> Split(char ch) const;
    std::vector<StringPiece> Split(StringPiece slice) const;
    StringPiece SliceFragment(StringPiece first, StringPiece last);

    // 若返回值与begin()相等，说明解析失败
    const char* ParseInt(int& num) const;

    bool StartsWith(const StringPiece& x) const;

    string AsString() const { return string(data(), size()); }
    void CopyToString(string* target) const { target->assign(ptr_, length_); }

    void RemoveSuffix(int n) { length_ -= n; }
    void RemovePrefix(int n)
    {
        ptr_ += n;
        length_ -= n;
    }
    void RemovePrefixUntil(const char* pos);
    void RemoveSuffixUntil(const char* pos);

    // data() may return a pointer to a buffer with embedded NULs, and the
    // returned buffer may or may not be null terminated.  Therefore it is
    // typically a mistake to pass data() to a routine that expects a NUL
    // terminated string.  Use "as_string().c_str()" if you really need to
    // do this.  Or better yet, change your routine so it does not rely on
    // NUL termination.

    // getter
    const char* data() const { return ptr_; }
    const uint8_t* data_uint8() const
    {
        return reinterpret_cast<const uint8_t*>(ptr_);
    }
    int size() const { return length_; }
    bool empty() const { return length_ == 0; }
    const char* begin() const { return ptr_; }
    const char* end() const { return ptr_ + length_; }

    // setter
    void clear();
    void set(const char* buffer, int len);
    void set(const char* str);
    void set(const void* buffer, int len);
    void set(const char* begin, const char* end);

    // operator
    char operator[](int i) const { return ptr_[i]; }
    bool operator==(const StringPiece& x) const
    {
        return ((length_ == x.length_) && (memcmp(ptr_, x.ptr_, length_) == 0));
    }
    bool operator!=(const StringPiece& x) const { return !(*this == x); }
#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                            \
    bool operator cmp(const StringPiece& x) const                            \
    {                                                                        \
        int r =                                                              \
            memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
        return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));      \
    }
    STRINGPIECE_BINARY_PREDICATE(<, <);
    STRINGPIECE_BINARY_PREDICATE(<=, <);
    STRINGPIECE_BINARY_PREDICATE(>=, >);
    STRINGPIECE_BINARY_PREDICATE(>, >);
#undef STRINGPIECE_BINARY_PREDICATE

    void swap(StringPiece& rhs)
    {
        std::swap(ptr_, rhs.ptr_);
        std::swap(length_, rhs.length_);
    }

private:
    const char* ptr_;
    int length_;
};

}  // namespace baize

#endif  // BAIZE_STRINGPIECE_H