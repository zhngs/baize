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

template <typename T>
class slice {
 public:
  using iterator = typename vector<T>::iterator;

 public:
  slice() {
    ptr_ = std::make_shared<vector<T>>();
    begin_ = 0;
    end_ = 0;
  }

  slice(size len, size cap) {
    if (len > cap) cap = len;
    ptr_ = std::make_shared<vector<T>>(len);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = len;
  }

  slice(const slice& s) : ptr_(s.ptr_), begin_(s.begin_), end_(s.end_) {}
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
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  size len() const { return end_ - begin_; }
  size cap() const { return ptr_->capacity(); }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<T>& vec() { return *ptr_; }
  T* data() { return ptr_->data() + begin_; }

  slice as_slice(size l, size r) {
    if (r > this->len()) r = this->len();
    if (l > this->len()) l = this->len();
    if (r < l) r = l;

    slice tmp(*this);
    tmp.end_ = tmp.begin_ + r;
    tmp.begin_ = tmp.begin_ + l;
    return tmp;
  }

  void append(T b) {
    ensure(end_ + 1);

    ptr_->push_back(b);
    end_++;
  }

 private:
  void ensure(size expect) {
    if (expect > this->cap()) {
      slice tmp(this->len(), expect * 2);
      *tmp.ptr_ = *ptr_;
      tmp.begin_ = begin_;
      tmp.end_ = end_;
      tmp.swap(*this);
    }
  }

 private:
  shared_ptr<vector<T>> ptr_;
  size begin_;
  size end_;
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
  }

  slice(size len, size cap) {
    if (len > cap) cap = len;
    ptr_ = std::make_shared<vector<byte>>(len, 0);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = len;
  }
  slice(void* b, size l, size cap) {
    byte* tmp = static_cast<byte*>(b);
    ptr_ = std::make_shared<vector<byte>>(tmp, tmp + l);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = l;
  }
  slice(string s) {
    ptr_ = std::make_shared<vector<byte>>(s.begin(), s.end());
    begin_ = 0;
    end_ = s.length();
  }

  slice(const slice& s) : ptr_(s.ptr_), begin_(s.begin_), end_(s.end_) {}
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
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  size len() const { return end_ - begin_; }
  size cap() const { return ptr_->capacity(); }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<byte>& vec() { return *ptr_; }
  template <typename Y>
  Y* data() {
    return reinterpret_cast<Y*>(ptr_->data() + begin_);
  }

  void append(byte b) {
    ensure(end_ + 1);
    ptr_->push_back(b);
    end_++;
  }

  size copy(const slice& s) {
    size n = std::min(this->len(), s.len());
    auto pos = s.ptr_->cbegin() + begin_;
    std::copy(pos, pos + n, this->begin());
    return n;
  }

  slice as_slice(size l, size r) {
    if (r > this->len()) r = this->len();
    if (l > this->len()) l = this->len();
    if (r < l) r = l;

    slice tmp(*this);
    tmp.end_ = tmp.begin_ + r;
    tmp.begin_ = tmp.begin_ + l;
    return tmp;
  }

  string as_string() { return string(this->begin(), this->end()); }

  string dump(string prefix, size linebytes = 8) {
    string s;
    char buf[128] = "";
    snprintf(buf, sizeof(buf),
             "[%s] slice stat: ptr=%#lx len=%lu cap=%lu begin=%lu end=%lu\n",
             prefix.c_str(), reinterpret_cast<size>(ptr_->data()), this->len(), this->cap(), begin_,
             end_);
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

 private:
  void ensure(size expect) {
    if (expect > this->cap()) {
      slice tmp(this->len(), expect * 2);
      *tmp.ptr_ = *ptr_;
      tmp.begin_ = begin_;
      tmp.end_ = end_;
      tmp.swap(*this);
    }
  }

 private:
  shared_ptr<vector<byte>> ptr_;
  size begin_;
  size end_;
};

}  // namespace baize

#endif
