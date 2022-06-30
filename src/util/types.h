#ifndef BAIZE_TYPES_H 
#define BAIZE_TYPES_H 
// copy from muduo and make some small changes

#include <endian.h>
#include <functional>
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

inline uint64_t hostToNetwork64(uint64_t host64)
{
    return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
    return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
    return htobe16(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
    return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
    return be32toh(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
    return be16toh(net16);
}

} // namespace baize


#endif //BAIZE_TYPES_H 