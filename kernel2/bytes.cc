#include "bytes.h"

namespace baize {

int Index(slice<byte> s, byte c) {
  auto pos = std::find(s.begin(), s.end(), c);
  if (pos == s.end())
    return -1;
  else
    return static_cast<int>(pos - s.begin());
}

int Index(slice<byte> s, slice<byte> sep) {
  auto pos = std::search(s.begin(), s.end(), sep.begin(), sep.end());
  if (pos == s.end())
    return -1;
  else
    return static_cast<int>(pos - s.begin());
}

int Count(slice<byte> s, byte c) {
  return static_cast<int>(std::count(s.begin(), s.end(), c));
}

int Count(slice<byte> s, slice<byte> sep) {
  if (sep.len() == 0) {
    return 0;
  }
  if (sep.len() == 1) {
    return Count(s, sep[0]);
  }

  int n = 0;
  while (1) {
    int i = Index(s, sep);
    if (i == -1) {
      return n;
    }
    n++;
    s = s.as_slice(i + sep.len(), s.len());
  }
}

slice<slice<byte>> genSplit(slice<byte> s, slice<byte> sep, int sepSave,
                            int n) {
  if (n == 0) {
    return slice<slice<byte>>();
  }
  if (sep.len() == 0) {
    return slice<slice<byte>>();
  }
  if (n < 0) {
    n = Count(s, sep) + 1;
  }

  slice<slice<byte>> a(n, n);
  n--;
  int i = 0;
  while (i < n) {
    int m = Index(s, sep);
    if (m < 0) {
      break;
    }
    a[i] = s.as_slice(0, m + sepSave, m + sepSave);
    s = s.as_slice(m + sep.len(), s.len());
    i++;
  }
  a[i] = s;
  return a.as_slice(0, i + 1);
}

slice<slice<byte>> Split(slice<byte> s, slice<byte> sep) {
  return genSplit(s, sep, 0, -1);
}

slice<byte> Join(slice<slice<byte>> s, slice<byte> sep) {
  if (s.len() == 0) {
    return slice<byte>();
  }
  if (s.len() == 1) {
    return s[0].clone();
  }

  size n = sep.len() * (s.len() - 1);
  for (size i = 0; i < s.len(); i++) {
    n += s[i].len();
  }

  slice<byte> b(n, n);
  size bp = b.copy(s[0]);
  for (size i = 1; i < s.len(); i++) {
    bp += b.as_slice(bp, b.len()).copy(sep);
    bp += b.as_slice(bp, b.len()).copy(s[i]);
  }
  return b;
}

slice<byte> Replace(slice<byte> s, slice<byte> from, slice<byte> to, int n) {
  int m = 0;
  if (from.len() == 0) {
    return s.clone();
  }
  if (n != 0) {
    m = Count(s, from);
  }
  if (m == 0) {
    return s.clone();
  }
  if (n < 0 || m < n) {
    n = m;
  }

  size len = s.len() + n * to.len() - n * from.len();
  slice<byte> t(len, len);
  size w = 0;
  size start = 0;
  for (int i = 0; i < n; i++) {
    size j = start;
    j += Index(s.as_slice(start, s.len()), from);
    w += t.as_slice(w, t.len()).copy(s.as_slice(start, j));
    w += t.as_slice(w, t.len()).copy(to);
    start = j + from.len();
  }
  w += t.as_slice(w, t.len()).copy(s.as_slice(start, s.len()));
  return t.as_slice(0, w);
}

slice<byte> ReplaceAll(slice<byte> s, slice<byte> from, slice<byte> to) {
  Replace(s, from, to, -1);
}
}  // namespace baize
