#include "util/string_piece.h"

namespace baize
{

StringPiece::StringPiece() : ptr_(nullptr), length_(0) {}

StringPiece::StringPiece(const char* str)
  : ptr_(str), length_(static_cast<int>(strlen(ptr_)))
{
}

StringPiece::StringPiece(const unsigned char* str)
  : ptr_(reinterpret_cast<const char*>(str)),
    length_(static_cast<int>(strlen(ptr_)))
{
}

StringPiece::StringPiece(const string& str)
  : ptr_(str.data()), length_(static_cast<int>(str.size()))
{
}

StringPiece::StringPiece(const char* offset, int len)
  : ptr_(offset), length_(len)
{
}

StringPiece::StringPiece(const char* begin, const char* end)
  : ptr_(begin), length_(static_cast<int>(end - begin))
{
    assert(length_ >= 0);
}

const char* StringPiece::Find(char ch) const
{
    const char* pos = std::find(begin(), end(), ch);
    return pos;
}

const char* StringPiece::Find(StringPiece slice) const
{
    const char* pos = std::search(begin(), end(), slice.begin(), slice.end());
    return pos;
}

void StringPiece::TrimSpace()
{
    const char* pos = begin();
    while (pos < end()) {
        if (isspace(*pos)) {
            RemovePrefix(1);
        } else {
            break;
        }
        pos++;
    }
    pos = end() - 1;
    while (pos >= begin()) {
        if (isspace(*pos)) {
            RemoveSuffix(1);
        } else {
            break;
        }
        pos--;
    }
}

std::vector<StringPiece> StringPiece::Split(char ch) const
{
    std::vector<StringPiece> split;
    StringPiece tmp(begin(), end());
    while (!tmp.empty()) {
        const char* pos = tmp.Find(ch);
        StringPiece piece(tmp.begin(), pos);
        if (!piece.empty()) {
            split.push_back(piece);
        }
        if (pos == end()) {
            tmp.RemovePrefix(static_cast<int>(pos - tmp.begin()));
        } else {
            tmp.RemovePrefix(static_cast<int>(pos - tmp.begin() + 1));
        }
    }
    return std::move(split);
}

std::vector<StringPiece> StringPiece::Split(StringPiece slice) const
{
    std::vector<StringPiece> split;
    StringPiece tmp(begin(), end());
    while (!tmp.empty()) {
        const char* pos = tmp.Find(slice);
        StringPiece piece(tmp.begin(), pos);
        if (!piece.empty()) {
            split.push_back(piece);
        }
        if (pos == end()) {
            tmp.RemovePrefix(static_cast<int>(pos - tmp.begin()));
        } else {
            tmp.RemovePrefix(
                static_cast<int>(pos - tmp.begin() + slice.size()));
        }
    }
    return std::move(split);
}

StringPiece StringPiece::SliceFragment(StringPiece first, StringPiece last)
{
    const char* pos_begin = Find(first);
    if (pos_begin == end()) return StringPiece();
    StringPiece fragment(pos_begin + first.size(), end());
    const char* pos_end = fragment.Find(last);
    if (pos_end == fragment.end()) return StringPiece();
    fragment.RemoveSuffixUntil(pos_end);
    return fragment;
}

const char* StringPiece::ParseInt(int& num) const
{
    int number = 0;
    const char* pos = begin();
    while (pos != end()) {
        if (isdigit(*pos)) {
            int tmp = *pos - '0';
            number = number * 10 + tmp;
            pos++;
        } else {
            break;
        }
    }
    if (pos == begin()) {
        return begin();
    } else {
        num = number;
        return pos;
    }
}

bool StringPiece::StartsWith(const StringPiece& x) const
{
    return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
}

void StringPiece::RemovePrefixUntil(const char* pos)
{
    assert(begin() <= pos);
    assert(pos <= end());
    RemovePrefix(static_cast<int>(pos - begin()));
}

void StringPiece::RemoveSuffixUntil(const char* pos)
{
    assert(begin() <= pos);
    assert(pos <= end());
    RemoveSuffix(static_cast<int>(end() - pos));
}

void StringPiece::clear()
{
    ptr_ = NULL;
    length_ = 0;
}

void StringPiece::set(const char* buffer, int len)
{
    ptr_ = buffer;
    length_ = len;
}

void StringPiece::set(const char* str)
{
    ptr_ = str;
    length_ = static_cast<int>(strlen(str));
}

void StringPiece::set(const void* buffer, int len)
{
    ptr_ = reinterpret_cast<const char*>(buffer);
    length_ = len;
}

void StringPiece::set(const char* begin, const char* end)
{
    ptr_ = begin;
    length_ = static_cast<int>(end - begin);
    assert(length_ >= 0);
}

}  // namespace baize
