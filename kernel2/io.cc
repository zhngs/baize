#include "io.h"

namespace baize {

result<slice<byte>> ReadAll(Reader r) {
  slice<byte> b(0, 1500);
  while (1) {
    if (b.len() == b.cap()) {
      b = b.append(0).as_slice(0, b.len());
    }
    auto res = r->Read(b.as_slice(b.len(), b.cap()));
    b = b.as_slice(0, b.len() + res.v);
    if (res.err) {
      if (res.errstring == ErrEOF) {
        res.clear_err();
      }
      return {b};
    }
  }
}

}  // namespace baize
