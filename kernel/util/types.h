#ifndef BAIZE_TYPES_H
#define BAIZE_TYPES_H

#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <string.h>

#include <functional>
#include <string>

namespace baize
{

using std::string;

inline void MemZero(void* p, size_t n) { memset(p, 0, n); }

/**
 * Host Byte Order to Net Byte order
 */
inline uint64_t HostToNetwork64(uint64_t host64) { return htobe64(host64); }
inline uint32_t HostToNetwork32(uint32_t host32) { return htobe32(host32); }
inline uint16_t HostToNetwork16(uint16_t host16) { return htobe16(host16); }
/**
 * Net Byte Order to Host Byte order
 */
inline uint64_t NetworkToHost64(uint64_t net64) { return be64toh(net64); }
inline uint32_t NetworkToHost32(uint32_t net32) { return be32toh(net32); }
inline uint16_t NetworkToHost16(uint16_t net16) { return be16toh(net16); }

/**
 * get value in Host Byte Order.
 */
inline uint8_t byte1(const void* p, size_t i)
{
    const uint8_t* data = static_cast<const uint8_t*>(p);
    return data[i];
}
inline uint16_t byte2(const void* p, size_t i)
{
    const uint8_t* data = static_cast<const uint8_t*>(p);
    return static_cast<uint16_t>(static_cast<uint16_t>(data[i + 1]) |
                                 static_cast<uint16_t>(data[i]) << 8);
}
inline uint32_t byte3(const void* p, size_t i)
{
    const uint8_t* data = static_cast<const uint8_t*>(p);
    return static_cast<uint32_t>(data[i + 2]) |
           static_cast<uint32_t>(data[i + 1]) << 8 |
           static_cast<uint32_t>(data[i]) << 16;
}
inline uint32_t byte4(const void* p, size_t i)
{
    const uint8_t* data = static_cast<const uint8_t*>(p);
    return static_cast<uint32_t>(data[i + 3]) |
           static_cast<uint32_t>(data[i + 2]) << 8 |
           static_cast<uint32_t>(data[i + 1]) << 16 |
           static_cast<uint32_t>(data[i]) << 24;
}
inline uint64_t byte8(const void* p, size_t i)
{
    const uint8_t* data = static_cast<const uint8_t*>(p);
    return static_cast<uint64_t>(byte4(data, i)) << 32 |
           static_cast<uint64_t>(byte4(data, i + 4));
}

/**
 * set value in Network Byte Order.
 */
inline void set_byte1(void* p, size_t i, uint8_t value)
{
    uint8_t* data = static_cast<uint8_t*>(p);
    data[i] = value;
}

inline void set_byte2(void* p, size_t i, uint16_t value)
{
    uint8_t* data = static_cast<uint8_t*>(p);
    data[i + 1] = static_cast<uint8_t>(value);
    data[i] = static_cast<uint8_t>(value >> 8);
}

inline void set_byte3(void* p, size_t i, uint32_t value)
{
    uint8_t* data = static_cast<uint8_t*>(p);
    data[i + 2] = static_cast<uint8_t>(value);
    data[i + 1] = static_cast<uint8_t>(value >> 8);
    data[i] = static_cast<uint8_t>(value >> 16);
}

inline void set_byte4(void* p, size_t i, uint32_t value)
{
    uint8_t* data = static_cast<uint8_t*>(p);
    data[i + 3] = static_cast<uint8_t>(value);
    data[i + 2] = static_cast<uint8_t>(value >> 8);
    data[i + 1] = static_cast<uint8_t>(value >> 16);
    data[i] = static_cast<uint8_t>(value >> 24);
}

inline void set_byte8(void* p, size_t i, uint64_t value)
{
    uint8_t* data = static_cast<uint8_t*>(p);
    data[i + 7] = static_cast<uint8_t>(value);
    data[i + 6] = static_cast<uint8_t>(value >> 8);
    data[i + 5] = static_cast<uint8_t>(value >> 16);
    data[i + 4] = static_cast<uint8_t>(value >> 24);
    data[i + 3] = static_cast<uint8_t>(value >> 32);
    data[i + 2] = static_cast<uint8_t>(value >> 40);
    data[i + 1] = static_cast<uint8_t>(value >> 48);
    data[i] = static_cast<uint8_t>(value >> 56);
}

/**
 * alignment function
 */
inline uint16_t Align4(uint16_t size)
{
    // If size is not multiple of 32 bits then pad it.
    if (size & 0x03) {
        return static_cast<uint16_t>((size & 0xFFFC) + 4);
    } else {
        return size;
    }
}
inline uint32_t Align4(uint32_t size)
{
    // If size is not multiple of 32 bits then pad it.
    if (size & 0x03) {
        return (size & 0xFFFFFFFC) + 4;
    } else {
        return size;
    }
}

}  // namespace baize

#endif  // BAIZE_TYPES_H