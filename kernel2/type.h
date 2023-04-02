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
  slice() {}

  slice(int len, int cap) {
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

  T& operator[](int i) { return (*ptr_)[begin_ + i]; }

  void swap(slice& s) noexcept {
    s.ptr_.swap(ptr_);
    std::swap(begin_, s.begin_);
    std::swap(end_, s.end_);
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  int len() const { return end_ - begin_; }
  int cap() const { return ptr_->capacity(); }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<T>& vec() { return *ptr_; }
  T* data() { return ptr_->data() + begin_; }

  slice as_slice(int l, int r = -1) {
    if (r < 0 || r > this->len()) r = this->len();
    if (l < 0) l = 0;
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
  void ensure(int expect) {
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
  int begin_;
  int end_;
};

template <>
class slice<byte> {
 public:
  using iterator = typename vector<byte>::iterator;

 public:
  slice() {}

  slice(int len, int cap) {
    ptr_ = std::make_shared<vector<byte>>(len, 0);
    ptr_->reserve(cap);
    begin_ = 0;
    end_ = len;
  }
  slice(string s) {
    ptr_ = std::make_shared<vector<byte>>(s.begin(), s.end());
    begin_ = 0;
    end_ = s.length();
  }
  slice(void* b, int l) {
    byte* tmp = (byte*)b;
    ptr_ = std::make_shared<vector<byte>>(tmp, tmp + l);
    begin_ = 0;
    end_ = l;
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

  byte& operator[](int i) { return (*ptr_)[begin_ + i]; }

  void swap(slice& s) noexcept {
    s.ptr_.swap(ptr_);
    std::swap(begin_, s.begin_);
    std::swap(end_, s.end_);
  }

  void reset() {
    slice tmp;
    this->swap(tmp);
  }

  int len() const { return end_ - begin_; }
  int cap() const { return ptr_->capacity(); }
  iterator begin() { return ptr_->begin() + begin_; }
  iterator end() { return ptr_->begin() + end_; }

  vector<byte>& vec() { return *ptr_; }
  template <typename Y>
  Y* data() {
    return ptr_->data() + begin_;
  }

  void append(byte b) {
    ensure(end_ + 1);

    ptr_->push_back(b);
    end_++;
  }

  int copy(const slice& s) {
    int n = std::min(this->len(), s.len());
    auto pos = s.ptr_->cbegin() + begin_;
    std::copy(pos, pos + n, this->begin());
    return n;
  }

  slice as_slice(int l, int r = -1) {
    if (r < 0 || r > this->len()) r = this->len();
    if (l < 0) l = 0;
    if (l > this->len()) l = this->len();
    if (r < l) r = l;

    slice tmp(*this);
    tmp.end_ = tmp.begin_ + r;
    tmp.begin_ = tmp.begin_ + l;
    return tmp;
  }

  string as_string() { return string(this->begin(), this->end()); }

  string dump(string prefix, int linebytes = 8) {
    string s;
    char buf[128] = "";
    snprintf(buf, sizeof(buf),
             "[%s] slice stat: ptr=%d len=%d cap=%d begin=%d end=%d\n",
             prefix.c_str(), ptr_->data(), this->len(), this->cap(), begin_,
             end_);
    s += buf;

    auto printchar = [](byte c) {
      if (isprint(c))
        return c;
      else
        return byte('.');
    };

    int end_pos = this->len() - 1;
    slice& self = *this;
    for (int i = 0; i <= end_pos;) {
      memset(buf, 0, sizeof(buf));

      int cur_pos_in_line = 0;
      for (int j = 0; j < linebytes; j++) {
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

      for (int j = 0; j < linebytes; j++) {
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
  void ensure(int expect) {
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
  int begin_;
  int end_;
};

}  // namespace baize

#endif
