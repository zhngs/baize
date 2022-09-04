#include "webrtc/ice/ice_server.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

void ProcessStunPacket(StunPacket& packet,
                       UdpStreamSptr stream,
                       InetAddress& addr,
                       string password)
{
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
            Buffer send_buf;
            rsp.set_class(StunPacket::Class::SUCCESS_RESPONSE);
            rsp.set_method(StunPacket::Method::BINDING);
            rsp.set_transaction_id(packet.transaction_id());
            rsp.set_xor_mapped_address(addr);
            rsp.set_password(password);
            rsp.Pack(send_buf);

            stream->AsyncSendto(
                send_buf.read_index(), send_buf.readable_bytes(), addr);
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
