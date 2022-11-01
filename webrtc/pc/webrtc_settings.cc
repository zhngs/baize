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

    gt_dtls_config = DtlsConfig::New("./cert.crt", "./cert.key");
    if (!gt_dtls_config) {
        LOG_FATAL << "dtls config init failed";
    }

    srtp_init();
}

const SdpMessage& WebRTCSettings::local_sdp() { return gt_sdp; }

string WebRTCSettings::local_sdp_string()
{
    char local_sdp_buf[8192];
    int ssrc = 1234;
    snprintf(local_sdp_buf,
             sizeof(local_sdp_buf),
             "v=0\r\n"
             "o=- 1495799811084970 1495799811084970 IN IP4 0.0.0.0\r\n"
             "s=baize webrtc sfu server\r\n"
             "c=IN IP4 0.0.0.0\r\n"
             "t=0 0\r\n"
             "a=ice-options:%s\r\n"
             "a=msid-semantic: WMS baize\r\n"
             "a=group:BUNDLE 0\r\n"
             "m=video 9 RTP/SAVPF 127\r\n"
             "a=rtpmap:127 H264/90000\r\n"
             "a=setup:passive\r\n"
             "a=mid:0\r\n"
             "a=sendrecv\r\n"
             "a=ice-ufrag:%s\r\n"
             "a=ice-pwd:%s\r\n"
             "a=fingerprint:sha-256 %s\r\n"
             "a=msid:baize baizev0\r\n"
             "a=ssrc:%d cname:baizevideo\r\n"
             "a=ssrc:%d msid:baize baizev0\r\n"
             "a=candidate:1 1 udp 1 %s %d typ host\r\n"
             "a=rtcp-mux\r\n"
             "a=rtcp-rsize\r\n",
             gt_sdp.net_.ice_option_.c_str(),
             gt_sdp.net_.ice_ufrag_.c_str(),
             gt_sdp.net_.ice_pwd_.c_str(),
             gt_sdp.net_.finger_print_.c_str(),
             ssrc,
             ssrc,
             gt_sdp.net_.ip_.c_str(),
             gt_sdp.net_.port_);
    return local_sdp_buf;
}

SSL_CTX* WebRTCSettings::dtls_ctx() { return gt_dtls_config->ssl_ctx(); }

}  // namespace net

}  // namespace baize
