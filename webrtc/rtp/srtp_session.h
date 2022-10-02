#ifndef BAIZE_SRTP_SESSION_H_
#define BAIZE_SRTP_SESSION_H_

#include <memory>

#include "srtp.h"
#include "util/string_piece.h"

namespace baize
{

namespace net
{

class SrtpSession;
using SrtpSessionUptr = std::unique_ptr<SrtpSession>;

class SrtpSession  // noncopyable
{
public:  // types
    enum class Type { kInBound, kOutBound };

public:  // factory function
    static SrtpSessionUptr New(Type type,
                               StringPiece use_srtp,
                               StringPiece key);

public:
    SrtpSession();
    ~SrtpSession();

    bool EncryptRtp(StringPiece packet);
    bool DecryptRtp(StringPiece packet);
    bool EncryptRtcp(StringPiece packet);
    bool DecryptRtcp(StringPiece packet);

private:
    srtp_t session_ = nullptr;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SRTP_SESSION_H_
