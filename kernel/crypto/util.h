#ifndef BAIZE_CRYPTO_UTIL_H_
#define BAIZE_CRYPTO_UTIL_H_

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace crypto
{

uint32_t CRC32(StringPiece s);
StringPiece HmacSha1(StringPiece key, StringPiece data);

}  // namespace crypto

}  // namespace baize

#endif  // BAIZE_CRYPTO_UTIL_H_