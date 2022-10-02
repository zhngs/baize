#include "webrtc/pc/webrtc_settings.h"

#include "log/logger.h"
#include "webrtc/pc/peer_connection.h"

namespace baize
{

namespace net
{

thread_local SdpMessage gt_sdp;
thread_local DtlsConfig::Uptr gt_dtls_config;
thread_local string gt_local_sdp;

void WebRTCSettings::Initialize()
{
    gt_sdp.net_.ip_ = "101.43.183.201";
    gt_sdp.net_.port_ = 6061;
    gt_sdp.net_.ice_ufrag_ = "2eaP";
    gt_sdp.net_.ice_pwd_ = "0HK9scLUJ8kv2TiuDAPHccjb";
    gt_sdp.net_.ice_option_ = "ice-lite";
    gt_sdp.net_.finger_print_ =
        "3E:7B:7D:D0:03:75:8A:E1:D6:69:32:D3:A3:EE:57:D2:4B:11:13:F3:5F:F6:"
        "3F:91:5D:DF:F6:E8:3A:0A:D2:09";

    gt_local_sdp = gt_sdp.local_sdp();

    gt_dtls_config = DtlsConfig::New("./cert.crt", "./cert.key");
    if (!gt_dtls_config) {
        LOG_FATAL << "dtls config init failed";
    }
}

string WebRTCSettings::ice_password() { return gt_sdp.net_.ice_pwd_; }

string WebRTCSettings::local_sdp() { return gt_local_sdp; }

SSL_CTX* WebRTCSettings::dtls_ctx() { return gt_dtls_config->ssl_ctx(); }

}  // namespace net

}  // namespace baize
