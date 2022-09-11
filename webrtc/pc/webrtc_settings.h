#ifndef BAIZE_WEBRTC_SETTINGS_H_
#define BAIZE_WEBRTC_SETTINGS_H_

#include "openssl/ssl.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class WebRTCSettings  // noncopyable
{
public:
    static void Initialize();

    // getter
    static string ice_password();
    static string local_sdp();
    static SSL_CTX* dtls_ctx();
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_PC_SETTINGS_H_