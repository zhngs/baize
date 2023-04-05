#ifndef BAIZE_BYTES_H
#define BAIZE_BYTES_H

#include <algorithm>

#include "type.h"

namespace baize {

int Index(slice<byte> s, byte c);
int Index(slice<byte> s, slice<byte> sep);

int Count(slice<byte> s, byte c);
int Count(slice<byte> s, slice<byte> sep);

slice<slice<byte>> Split(slice<byte> s, slice<byte> sep);

slice<byte> Join(slice<slice<byte>> s, slice<byte> sep);

slice<byte> Replace(slice<byte> s, slice<byte> from, slice<byte> to, int n);
slice<byte> ReplaceAll(slice<byte> s, slice<byte> from, slice<byte> to);

}  // namespace baize

#endif  // BAIZE_BYTES_H