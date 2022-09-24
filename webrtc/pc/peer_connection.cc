#include "webrtc/pc/peer_connection.h"

#include "log/logger.h"
#include "webrtc/pc/webrtc_settings.h"

namespace baize
{

namespace net
{

PeerConnection::PeerConnection(UdpStreamSptr stream, InetAddress addr)
  : stream_(stream), addr_(addr)
{
}

PeerConnection::~PeerConnection() {}

PeerConnection::Packet PeerConnection::AsyncRead()
{
    while (1) {
        if (packets_.empty()) {
            async_park_.WaitRead();
            continue;
        }
        Packet packet = std::move(packets_.back());
        packets_.pop_back();
        return std::move(packet);
    }
}

int PeerConnection::ProcessPacket(StringPiece packet)
{
    /**
     * handle ice
     */
    if (StunPacket::IsStun(packet)) {
        if (!ice_) {
            ice_ =
                IceServer::New(WebRTCSettings::ice_password(), stream_, addr_);
        }

        LOG_INFO << "process stun";
        ice_->ProcessStunPacket(packet);
        return 0;
    }

    if (!ice_->is_connected()) {
        return 0;
    }

    /**
     * handle dtls
     */

    if (DtlsTransport::IsDtls(packet)) {
        if (!dtls_) {
            dtls_ =
                DtlsTransport::New(WebRTCSettings::dtls_ctx(), stream_, addr_);
            dtls_->Initialize(DtlsTransport::Role::SERVER);
        }

        LOG_INFO << "process dtls";
        dtls_->ProcessDtlsPacket(packet);
        return 0;
    }

    if (!dtls_->is_running()) {
        return -1;
    } else if (!dtls_->is_connected()) {
        LOG_INFO << "dtls connected";
        return 0;
    }

    /**
     * handle rtp
     */
    return 0;
}

}  // namespace net

}  // namespace baize