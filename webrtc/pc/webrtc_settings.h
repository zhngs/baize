#ifndef BAIZE_WEBRTC_SETTINGS_H_
#define BAIZE_WEBRTC_SETTINGS_H_

#include "openssl/ssl.h"
#include "util/types.h"
#include "webrtc/sdp/sdp_message.h"

namespace baize
{

namespace net
{

class WebRTCSettings  // noncopyable
{
public:
    static void Initialize();

    // getter
    static string local_sdp_string();
    static const SdpMessage& local_sdp();
    static SSL_CTX* dtls_ctx();
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_PC_SETTINGS_H_