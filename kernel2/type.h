#ifndef BAIZE_TYPE_H
#define BAIZE_TYPE_H

#include <stdio.h>
#include <string.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace baize {

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using float32 = float;
using float64 = double;

using byte = uint8_t;
using size = size_t;
using ssize = ssize_t;

using std::map;
using std::string;
using std::vector;

using std::shared_ptr;
using std::unique_ptr;

class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

class copyable {
 protected:
  copyable() = default;
  ~copyable() = default;
};

#define ErrEOF "EOF"

template <typename T>
class result {
 public:
  result(T r) : v(r), err(0), errstring() {}
  result(int e, string s) : v(), err(e), errstring(s) {}
  result(T r, int e, string s) : v(r), err(e), errstring(s) {}
  void clear_err() {
    err = 0;
    errstring.clear();
  }

  T value_or(T d) { err == 0 ? v : d; }

 public:
  T v;
  int err;
  string errstring;
};

#define __RESULT(line, val, e, s, f) \
  auto r##line = f;                  \
  auto& val = r##line.v;             \
  int& e = r##line.err;              \
  string& s = r##line.errstring;     \
  (void)val;                         \
  (void)e;                           \
  (void)s
#define _RESULT(line, val, e, s, f) __RESULT(line, val, e, s, f)
#define RESULT(val, e, s, f) _RESULT(__LINE__, val, e, s, f)

template <typename T>
class slice {
 public:
  using iterator = typename vector<T>::iterator;

 public:
  slice() {
    ptr_ = std::make_shared<vector<T>>();
    begin_ = 0;
    end_ = 0;
    cap_ = ptr_->capacity();
  }

  slice(size len, size cap) {
    if (len > cap) cap = len;
    ptr_ = std::make_shared<vector<T>>(len);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = len;
    cap_ = ptr_->capacity();
  }

  slice(const slice& s)
      : ptr_(s.ptr_), begin_(s.begin_), end_(s.end_), cap_(s.cap_) {}
  slice& operator=(const slice& s) {
    slice tmp(s);
    this->swap(tmp);
    return *this;
  }

  slice(slice&& s) {
    slice tmp;
    tmp.swap(s);
    this->swap(tmp);
  }
  slice& operator=(slice&& s) {
    slice tmp(std::move(s));
    this->swap(tmp);
    return *this;
  }
  ~slice() {}

  T& operator[](size i) { return (*ptr_)[begin_ + i]; }

  void swap(slice& s) noexcept {
    s.ptr_.swap(ptr_);
    std::swap(begin_, s.begin_);
    std::swap(end_, s.end_);
    std::swap(cap_, s.cap_);
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  size len() const { return end_ - begin_; }
  size cap() const { return cap_ - begin_; }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<T>& vec() { return *ptr_; }
  T* data() { return ptr_->data() + begin_; }

  slice as_slice(size l, size r) {
    if (r > this->cap()) r = this->cap();
    if (l > this->cap()) l = this->cap();
    if (r < l) r = l;

    slice tmp(*this);
    tmp.end_ = tmp.begin_ + r;
    tmp.begin_ = tmp.begin_ + l;
    return tmp;
  }

  slice as_slice(size l, size r, size c) {
    if (r > this->cap()) r = this->cap();
    if (l > this->cap()) l = this->cap();
    if (r < l) r = l;
    if (c < r) c = r;

    auto s = this->as_slice(l, r);
    s.cap_ = c + s.begin_;
    return s;
  }

  slice& append(T b) {
    ensure(this->len() + 1);

    ptr_->push_back(b);
    end_++;
    return *this;
  }

 private:
  void ensure(size expect) {
    if (expect > this->cap()) {
      slice tmp(this->len(), expect * 2);
      std::copy(this->begin(), this->end(), tmp.begin());
      tmp.swap(*this);
    }
  }

 private:
  shared_ptr<vector<T>> ptr_;
  size begin_;
  size end_;
  size cap_;
};

template <>
class slice<byte> {
 public:
  using iterator = typename vector<byte>::iterator;

 public:
  slice() {
    ptr_ = std::make_shared<vector<byte>>();
    begin_ = 0;
    end_ = 0;
    cap_ = ptr_->capacity();
  }

  slice(size len, size cap) {
    if (len > cap) cap = len;
    ptr_ = std::make_shared<vector<byte>>(len, 0);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = len;
    cap_ = ptr_->capacity();
  }
  slice(void* b, size l, size cap) {
    byte* tmp = static_cast<byte*>(b);
    ptr_ = std::make_shared<vector<byte>>(tmp, tmp + l);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = l;
    cap_ = ptr_->capacity();
  }
  slice(string s) {
    ptr_ = std::make_shared<vector<byte>>(s.begin(), s.end());
    begin_ = 0;
    end_ = s.length();
    cap_ = ptr_->capacity();
  }
  slice(const char* s) {
    size len = strlen(s);
    ptr_ = std::make_shared<vector<byte>>(s, s + len);
    begin_ = 0;
    end_ = len;
    cap_ = ptr_->capacity();
  }

  slice(const slice& s)
      : ptr_(s.ptr_), begin_(s.begin_), end_(s.end_), cap_(s.cap_) {}
  slice& operator=(const slice& s) {
    slice tmp(s);
    this->swap(tmp);
    return *this;
  }

  slice(slice&& s) {
    slice tmp;
    tmp.swap(s);
    this->swap(tmp);
  }
  slice& operator=(slice&& s) {
    slice tmp(std::move(s));
    this->swap(tmp);
    return *this;
  }
  ~slice() {}

  byte& operator[](size i) { return (*ptr_)[begin_ + i]; }

  void swap(slice& s) noexcept {
    s.ptr_.swap(ptr_);
    std::swap(begin_, s.begin_);
    std::swap(end_, s.end_);
    std::swap(cap_, s.cap_);
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  size len() const { return end_ - begin_; }
  size cap() const { return cap_ - begin_; }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<byte>& vec() { return *ptr_; }

  template <typename Y>
  Y* data() {
    return reinterpret_cast<Y*>(ptr_->data() + begin_);
  }
  template <typename Y>
  Y* data() const {
    return reinterpret_cast<Y*>(ptr_->data() + begin_);
  }

  slice& append(byte b) {
    ensure(this->len() + 1);
    ptr_->push_back(b);
    end_++;
    return *this;
  }

  size copy(const slice& s) {
    size n = std::min(this->len(), s.len());
    auto pos = s.ptr_->cbegin() + s.begin_;
    std::copy(pos, pos + n, this->begin());
    return n;
  }

  slice clone() {
    slice<byte> tmp(this->len(), this->cap());
    std::copy(this->begin(), this->end(), tmp.begin());
    return tmp;
  }

  slice as_slice(size l, size r) {
    if (r > this->cap()) r = this->cap();
    if (l > this->cap()) l = this->cap();
    if (r < l) r = l;

    slice tmp(*this);
    tmp.end_ = tmp.begin_ + r;
    tmp.begin_ = tmp.begin_ + l;
    return tmp;
  }

  slice as_slice(size l, size r, size c) {
    if (r > this->cap()) r = this->cap();
    if (l > this->cap()) l = this->cap();
    if (r < l) r = l;
    if (c < r) c = r;

    auto s = this->as_slice(l, r);
    s.cap_ = c + s.begin_;
    return s;
  }

  string as_string() { return string(this->begin(), this->end()); }

  string dump(string prefix, size linebytes = 8) {
    string s;
    char buf[128] = "";
    snprintf(buf, sizeof(buf),
             "[%s] slice stat: ptr=%#lx len()=%lu cap()=%lu begin=%lu end=%lu "
             "cap=%lu vector->capacity()=%lu\n",
             prefix.c_str(), reinterpret_cast<size>(ptr_->data()), this->len(),
             this->cap(), begin_, end_, cap_, ptr_->capacity());
    s += buf;

    auto printchar = [](byte c) {
      if (isprint(c))
        return c;
      else
        return byte('.');
    };

    if (this->len() == 0) return s;

    size end_pos = this->len() - 1;
    slice& self = *this;
    for (size i = 0; i <= end_pos;) {
      memset(buf, 0, sizeof(buf));

      int cur_pos_in_line = 0;
      for (size j = 0; j < linebytes; j++) {
        if (i + j <= end_pos) {
          if ((j + 1) % 8 == 0) {
            snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line,
                     "%02x  ", self[i + j]);
            cur_pos_in_line += 4;
          } else {
            snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line,
                     "%02x ", self[i + j]);
            cur_pos_in_line += 3;
          }
        } else {
          if ((j + 1) % 8 == 0) {
            snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line,
                     "..  ");
            cur_pos_in_line += 4;
          } else {
            snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line,
                     ".. ");
            cur_pos_in_line += 3;
          }
        }
      }

      snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line, "  ");
      cur_pos_in_line += 2;

      for (size j = 0; j < linebytes; j++) {
        if (i + j <= end_pos) {
          snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line, "%c",
                   printchar(self[i + j]));
        } else {
          snprintf(buf + cur_pos_in_line, sizeof(buf) - cur_pos_in_line, ".");
        }
        cur_pos_in_line++;
      }

      s += buf;
      s += '\n';
      i += linebytes;
    }

    return s;
  }

  bool operator==(const slice& s) const {
    return ((this->len() == s.len()) &&
            (memcmp(this->data<byte>(), s.data<byte>(), this->len()) == 0));
  }
  bool operator!=(const slice& x) const { return !(*this == x); }

#define SLICE_BINARY_PREDICATE(cmp, auxcmp)                           \
  bool operator cmp(const slice& s) const {                           \
    int r = memcmp(this->data<byte>(), s.data<byte>(),                \
                   this->len() < s.len() ? this->len() : s.len());    \
    return ((r auxcmp 0) || ((r == 0) && (this->len() cmp s.len()))); \
  }
  SLICE_BINARY_PREDICATE(<, <);
  SLICE_BINARY_PREDICATE(<=, <);
  SLICE_BINARY_PREDICATE(>=, >);
  SLICE_BINARY_PREDICATE(>, >);
#undef SLICE_BINARY_PREDICATE

  int compare(const slice& s) const {
    int r = memcmp(this->data<byte>(), s.data<byte>(),
                   this->len() < s.len() ? this->len() : s.len());
    if (r == 0) {
      if (this->len() < s.len())
        r = -1;
      else if (this->len() > s.len())
        r = +1;
    }
    return r;
  }

 private:
  void ensure(size expect) {
    if (expect > this->cap()) {
      slice tmp(this->len(), expect * 2);
      std::copy(this->begin(), this->end(), tmp.begin());
      tmp.swap(*this);
    }
  }

 private:
  shared_ptr<vector<byte>> ptr_;
  size begin_;
  size end_;
  size cap_;
};

}  // namespace baize

#endif
