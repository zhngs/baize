#include "webrtc/pc/peer_connection.h"

#include "log/logger.h"
#include "webrtc/pc/webrtc_settings.h"

namespace baize
{

namespace net
{

PeerConnection::Sptr PeerConnection::New(UdpStreamSptr stream,
                                         InetAddress& addr)
{
    Sptr pc = std::make_shared<PeerConnection>(stream, addr);
    pc->ice_ = IceServer::New(WebRTCSettings::ice_password(), stream, addr);

    return pc;
}

PeerConnection::PeerConnection(UdpStreamSptr stream, InetAddress addr)
  : stream_(stream), addr_(addr)
{
}

PeerConnection::~PeerConnection() {}

int PeerConnection::ProcessPacket(StringPiece packet)
{
    /**
     * handle ice
     */
    if (StunPacket::IsStun(packet)) {
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
    if (!dtls_) {
        dtls_ = DtlsTransport::New(WebRTCSettings::dtls_ctx(), stream_, addr_);
        dtls_->Initialize(DtlsTransport::Role::SERVER);
    }

    if (DtlsTransport::IsDtls(packet)) {
        LOG_INFO << "process dtls";
        dtls_->ProcessDtlsPacket(packet);
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