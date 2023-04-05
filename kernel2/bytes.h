#ifndef BAIZE_BYTES_H
#define BAIZE_BYTES_H

#include "type.h"
#include <algorithm>

namespace baize
{

int Index(slice<byte> s, byte c) {
    auto pos = std::find(s.begin(), s.end(), c);
    if (pos == s.end()) return -1;
    else return static_cast<int>(pos - s.begin());
}

int Index(slice<byte> s, slice<byte> sep) {
    auto pos = std::search(s.begin(), s.end(), sep.begin(), sep.end());
    if (pos == s.end()) return -1;
    else return static_cast<int>(pos - s.begin());
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
	while(1) {
		int i = Index(s, sep);
		if (i == -1) {
			return n;
		}
		n++;
        s = s.as_slice(i + sep.len(), s.len());
	}
}

slice<slice<byte>> genSplit(slice<byte> s, slice<byte> sep, int sepSave, int n) {
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

slice<slice<byte>> Split(slice<byte> s, slice<byte> sep) { return genSplit(s, sep, 0, -1); }
    
} // namespace baize


#endif // BAIZE_BYTES_H