#include "webrtc/ice/ice_server.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

IceServer::Uptr IceServer::New(string password,
                               UdpStreamSptr stream,
                               InetAddress addr)
{
    return std::make_unique<IceServer>(password, stream, addr);
}

IceServer::~IceServer() {}

void IceServer::ProcessStunPacket(StringPiece stun_packet)
{
    StunPacket packet;
    int err = packet.Parse(stun_packet);
    if (!err) {
        packet.dump();
    } else {
        LOG_ERROR << "stun packet parse failed";
        return;
    }

    if (packet.method() != StunPacket::Method::BINDING) {
        return;
    }

    if (!packet.has_fingerprint() &&
        packet.klass() != StunPacket::Class::INDICATION) {
        return;
    }

    switch (packet.klass()) {
        case StunPacket::Class::REQUEST: {
            // USERNAME, MESSAGE-INTEGRITY and PRIORITY are required.
            if (!packet.has_message_integrity() || (packet.priority() == 0) ||
                packet.username().empty()) {
                break;
            }

            // todo: check username and pwd

            // The remote peer must be ICE controlling.
            if (packet.ice_controlled()) {
                break;
            }

            StunPacket rsp;
            rsp.set_class(StunPacket::Class::SUCCESS_RESPONSE);
            rsp.set_method(StunPacket::Method::BINDING);
            rsp.set_transaction_id(packet.transaction_id());
            rsp.set_xor_mapped_address(dest_addr_);
            rsp.set_password(ice_password_);

            send_buf_.TakeAll();
            rsp.Pack(send_buf_);

            stream_->AsyncSendto(
                send_buf_.read_index(), send_buf_.readable_bytes(), dest_addr_);

            if (packet.has_use_candidate()) state_ = IceState::CONNECTED;

            break;
        }

        case StunPacket::Class::INDICATION: {
            LOG_INFO << "STUN Binding Indication processed";
            break;
        }

        case StunPacket::Class::SUCCESS_RESPONSE: {
            LOG_INFO << "STUN Binding Success Response processed";
            break;
        }

        case StunPacket::Class::ERROR_RESPONSE: {
            LOG_INFO << "STUN Binding Error Response processed";
            break;
        }

        default:
            break;
    }
}

}  // namespace net

}  // namespace baize
