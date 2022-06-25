#ifndef BAIZE_TYPES_H 
#define BAIZE_TYPES_H 

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

} // namespace baize


#endif //BAIZE_TYPES_H 