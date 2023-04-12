#ifndef BAIZE_IO_H
#define BAIZE_IO_H

#include "type.h"

namespace baize {

class IReader {
 public:
  virtual ~IReader(){};
  virtual result<int> Read(slice<byte> p) = 0;
};
using Reader = shared_ptr<IReader>;

class IWriter {
 public:
  virtual ~IWriter(){};
  virtual result<int> Write(slice<byte> p) = 0;
};
using Writer = shared_ptr<IWriter>;

class ISeeker {
 public:
  virtual ~ISeeker(){};
  virtual result<int64> Seek(int64 offset, int whence) = 0;
};

class DevZero : public IReader {
 public:
  result<int> Read(slice<byte> p) override {
    memset(p.data<byte>(), 0, p.len());
    return result<int>(static_cast<int>(p.len()));
  }
};

class DevNull : public IWriter {
 public:
  result<int> Write(slice<byte> p) override {
    return static_cast<int>(p.len());
  }
};

result<slice<byte>> ReadAll(Reader r);

}  // namespace baize

#endif  // BAIZE_IO_H