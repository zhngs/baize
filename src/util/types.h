#ifndef BAIZE_TYPES_H 
#define BAIZE_TYPES_H 
// copy from muduo and make some small changes

#include <string>
#include <stdint.h>
#include <string.h>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace baize
{

using std::string;

inline void memZero(void *p, size_t n)
{
    memset(p, 0, n);
}

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
    return f;
}

} // namespace baize


#endif //BAIZE_TYPES_H 