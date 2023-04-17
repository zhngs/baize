#ifndef BAIZE_BYTES_H
#define BAIZE_BYTES_H

#include <limits.h>
#include <stdlib.h>

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

template <typename T>
inline result<T> NumberCast(const string& s) {
  int base;
  char* endptr;
  long val;

  const char* str = s.c_str();
  base = 10;

  errno = 0;
  val = strtol(str, &endptr, base);

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
      (errno != 0 && val == 0)) {
    return {errno, strerror(errno)};
  }
  if (endptr == str) {
    return {-1, "No digit found"};
  }

  return {static_cast<T>(val)};
}

}  // namespace baize

#endif  // BAIZE_BYTES_H