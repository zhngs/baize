#include "webrtc/ice/stun_packet.h"

#include "crypto/util.h"
#include "log/logger.h"

namespace baize
{

namespace net
{

const uint8_t StunPacket::magic_cookie_[] = {0x21, 0x12, 0xA4, 0x42};

bool StunPacket::IsStun(StringPiece packet)
{
    int len = packet.size();
    const uint8_t* data = packet.data_uint8();
    return (
        // STUN headers are 20 bytes.
        (len >= 20) &&
        // DOC:
        // https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
        (data[0] < 3) &&
        // Magic cookie must match.
        (data[4] == StunPacket::magic_cookie_[0]) &&
        (data[5] == StunPacket::magic_cookie_[1]) &&
        (data[6] == StunPacket::magic_cookie_[2]) &&
        (data[7] == StunPacket::magic_cookie_[3]));
}

int StunPacket::Parse(StringPiece packet)
{
    if (!StunPacket::IsStun(packet)) return -1;

    packet_ = packet;

    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    uint16_t msg_type = byte2(data, 0);
    uint16_t msg_length = byte2(data, 2);

    // length field must be total size minus header's 20 bytes, and must be
    // multiple of 4 Bytes.
    if ((msg_length != len - 20) || ((msg_length & 0x03) != 0)) {
        LOG_ERROR << "msg_length error";
        return -1;
    }

    method_ =
        static_cast<Method>((msg_type & 0x000f) | ((msg_type & 0x00e0) >> 1) |
                            ((msg_type & 0x3E00) >> 2));

    klass_ =
        static_cast<Class>(((data[0] & 0x01) << 1) | ((data[1] & 0x10) >> 4));

    transaction_id_ = StringPiece(data + 8, 12);

    // Start looking for attributes after STUN header (Byte #20).
    int pos = 20;
    // Flags (positions) for special MESSAGE-INTEGRITY and FINGERPRINT
    // attributes.
    bool has_message_integrity = false;
    bool has_fingerprint = false;
    int fingerprint_attr_pos;  // point to the beginning of the attribute.
    uint32_t fingerprint;      // Holds the value of the FINGERPRINT attribute.

    // Ensure there are at least 4 remaining bytes (attribute with 0 length).
    while (pos + 4 <= len) {
        auto attr_type = static_cast<Attribute>(byte2(data, pos));
        uint16_t attr_length = byte2(data, pos + 2);

        // Ensure the attribute length is not greater than the remaining size.
        if ((pos + 4 + attr_length) > len) {
            LOG_ERROR << "the attribute length exceeds the remaining size, "
                         "packet discarded";
            return -1;
        }

        // FINGERPRINT must be the last attribute.
        if (has_fingerprint) {
            LOG_ERROR << "attribute after FINGERPRINT is not allowed, packet "
                         "discarded";
            return -1;
        }

        // After a MESSAGE-INTEGRITY attribute just FINGERPRINT is allowed.
        if (has_message_integrity && attr_type != Attribute::FINGERPRINT) {
            LOG_ERROR << "attribute after MESSAGE-INTEGRITY other than "
                         "FINGERPRINT is not allowed, "
                         "packet discarded";
            return -1;
        }

        const uint8_t* attr_value_pos = data + pos + 4;
        switch (attr_type) {
            case Attribute::USERNAME: {
                set_username(StringPiece(attr_value_pos, attr_length));
                break;
            }

            case Attribute::PRIORITY: {
                // Ensure attribute length is 4 bytes.
                if (attr_length != 4) {
                    LOG_ERROR << "attribute PRIORITY must be 4 bytes length, "
                                 "packet discarded";
                    return -1;
                }
                set_priority(byte4(attr_value_pos, 0));
                break;
            }

            case Attribute::ICE_CONTROLLING: {
                // Ensure attribute length is 8 bytes.
                if (attr_length != 8) {
                    LOG_ERROR << "attribute ICE-CONTROLLING must be 8 bytes "
                                 "length, packet discarded";
                    return -1;
                }
                set_ice_controlling(byte8(attr_value_pos, 0));
                break;
            }

            case Attribute::ICE_CONTROLLED: {
                // Ensure attribute length is 8 bytes.
                if (attr_length != 8) {
                    LOG_ERROR << "attribute ICE-CONTROLLED must be 8 bytes "
                                 "length, packet discarded";
                    return -1;
                }
                set_ice_controlled(byte8(attr_value_pos, 0));
                break;
            }

            case Attribute::USE_CANDIDATE: {
                // Ensure attribute length is 0 bytes.
                if (attr_length != 0) {
                    LOG_ERROR << "attribute USE-CANDIDATE must be 0 bytes "
                                 "length, packet discarded";
                    return -1;
                }
                set_use_candidate();
                break;
            }

            case Attribute::NOMINATION: {
                // Ensure attribute length is 4 bytes.
                if (attr_length != 4) {
                    LOG_ERROR << "attribute NOMINATION must be 4 bytes length, "
                                 "packet discarded";
                    return -1;
                }
                set_has_nomination();
                set_nomination(byte4(attr_value_pos, 0));
                break;
            }

            case Attribute::MESSAGE_INTEGRITY: {
                // Ensure attribute length is 20 bytes.
                if (attr_length != 20) {
                    LOG_ERROR << "attribute MESSAGE-INTEGRITY must be 20 bytes "
                                 "length, packet discarded";
                    return -1;
                }
                has_message_integrity = true;
                set_message_integrity(StringPiece(attr_value_pos, 20));
                break;
            }

            case Attribute::FINGERPRINT: {
                // Ensure attribute length is 4 bytes.
                if (attr_length != 4) {
                    LOG_ERROR << "attribute FINGERPRINT must be 4 bytes "
                                 "length, packet discarded";
                    return -1;
                }
                has_fingerprint = true;
                fingerprint_attr_pos = pos;
                fingerprint = byte4(attr_value_pos, 0);
                set_fingerprint(fingerprint);
                break;
            }

            case Attribute::ERROR_CODE: {
                // Ensure attribute length >= 4bytes.
                if (attr_length < 4) {
                    LOG_ERROR << "attribute ERROR-CODE must be >= 4bytes "
                                 "length, packet discarded";
                    return -1;
                }
                uint8_t error_class = byte1(attr_value_pos, 2);
                uint8_t error_number = byte1(attr_value_pos, 3);
                uint16_t error_code =
                    static_cast<uint16_t>(error_class * 100 + error_number);
                set_error_code(error_code);
                break;
            }

            default:
                break;
        }

        // Set next attribute position.
        pos = Align4(static_cast<uint16_t>(pos + 4 + attr_length));
    }

    // Ensure current position matches the total length.
    if (pos != len) {
        LOG_ERROR << "computed packet size does not match total size, packet "
                     "discarded";
        return -1;
    }

    // If it has FINGERPRINT attribute then verify it.
    if (has_fingerprint) {
        // Compute the CRC32 of the received packet up to(but excluding)
        //     the FINGERPRINT attribute and XOR it with 0x5354554e.

        uint32_t computed_fingerprint =
            crypto::CRC32(StringPiece(data, fingerprint_attr_pos)) ^ 0x5354554e;

        // Compare with the FINGERPRINT value in the packet.
        if (fingerprint != computed_fingerprint) {
            LOG_ERROR << "computed FINGERPRINT value does not match the value";
            return -1;
        }
    }

    return 0;
}

void StunPacket::dump() const
{
    string dump;
    dump += "\n<StunPacket>\n";

    std::string klass;
    switch (this->klass_) {
        case Class::REQUEST:
            klass = "Request";
            break;
        case Class::INDICATION:
            klass = "Indication";
            break;
        case Class::SUCCESS_RESPONSE:
            klass = "SuccessResponse";
            break;
        case Class::ERROR_RESPONSE:
            klass = "ErrorResponse";
            break;
        default:
            klass = "None";
            break;
    }
    if (this->method_ == Method::BINDING) {
        log::StringAppend(dump, "  Binding %s", klass.c_str());
    } else {
        // This prints the unknown method number. Example: TURN Allocate =>
        // 0x003.
        log::StringAppend(dump,
                          "  %s with unknown method %#.3x",
                          klass.c_str(),
                          static_cast<uint16_t>(this->method_));
    }
    log::StringAppend(dump, "  size: %d bytes", this->packet_.size());

    log::StringAppend(
        dump, "  transaction_id: %s", log::DumpHex(transaction_id_).c_str());

    if (this->error_code_ != 0u)
        log::StringAppend(dump, "  error_code: %d", this->error_code_);
    if (!this->username_.empty())
        log::StringAppend(
            dump, "  username: %s", this->username_.AsString().c_str());
    if (this->priority_ != 0u)
        log::StringAppend(dump, "  priority: %d", this->priority_);
    if (this->ice_controlling_ != 0u)
        log::StringAppend(
            dump, "  ice_controlling: %#lx", this->ice_controlling_);
    if (this->ice_controlled_ != 0u)
        log::StringAppend(
            dump, "  ice_controlled: %#lx", this->ice_controlled_);
    if (this->has_use_candidate_) log::StringAppend(dump, "  use_candidate");
    // if (this->xorMappedAddress != nullptr) {
    //     int family;
    //     uint16_t port;
    //     std::string ip;

    //     Utils::IP::GetAddressInfo(this->xorMappedAddress, family, ip, port);

    //     MS_DUMP("  xorMappedAddress: %s : %" PRIu16, ip.c_str(), port);
    // }

    if (!this->message_integrity_.empty()) {
        log::StringAppend(dump,
                          "  message_integrity: %s",
                          log::DumpHex(message_integrity_).c_str());
    }

    if (this->has_fingerprint_)
        log::StringAppend(dump, "  has fingerprint: %#x", this->fingerprint_);

    dump += "</StunPacket>\n";
    LOG_INFO << dump;
}

// StunPacket::Authentication StunPacket::CheckAuthentication(
//     const std::string& localUsername, const std::string& localPassword)
// {
//     MS_TRACE();

//     switch (this->klass) {
//         case Class::REQUEST:
//         case Class::INDICATION: {
//             // Both USERNAME and MESSAGE-INTEGRITY must be present.
//             if (!this->messageIntegrity || this->username.empty())
//                 return Authentication::BAD_REQUEST;

//             // Check that USERNAME attribute begins with our local username
//             plus
//             // ":".
//             size_t localUsernameLen = localUsername.length();

//             if (this->username.length() <= localUsernameLen ||
//                 this->username.at(localUsernameLen) != ':' ||
//                 (this->username.compare(0, localUsernameLen, localUsername)
//                 !=
//                  0)) {
//                 return Authentication::UNAUTHORIZED;
//             }

//             break;
//         }
//         // This method cannot check authentication in received responses (as
//         we
//         // are ICE-Lite and don't generate requests).
//         case Class::SUCCESS_RESPONSE:
//         case Class::ERROR_RESPONSE: {
//             MS_ERROR("cannot check authentication for a STUN response");

//             return Authentication::BAD_REQUEST;
//         }
//     }

//     // If there is FINGERPRINT it must be discarded for MESSAGE-INTEGRITY
//     // calculation, so the header length field must be modified (and later
//     // restored).
//     if (this->hasFingerprint)
//         // Set the header length field: full size - header length (20) -
//         // FINGERPRINT length (8).
//         Utils::Byte::Set2Bytes(
//             this->data, 2, static_cast<uint16_t>(this->size - 20 - 8));

//     // Calculate the HMAC-SHA1 of the message according to MESSAGE-INTEGRITY
//     // rules.
//     const uint8_t* computedMessageIntegrity = Utils::Crypto::GetHmacSha1(
//         localPassword, this->data, (this->messageIntegrity - 4) -
//         this->data);

//     Authentication result;

//     // Compare the computed HMAC-SHA1 with the MESSAGE-INTEGRITY in the
//     packet. if (std::memcmp(this->messageIntegrity, computedMessageIntegrity,
//     20) == 0)
//         result = Authentication::OK;
//     else
//         result = Authentication::UNAUTHORIZED;

//     // Restore the header length field.
//     if (this->hasFingerprint)
//         Utils::Byte::Set2Bytes(
//             this->data, 2, static_cast<uint16_t>(this->size - 20));

//     return result;
// }

// StunPacket* StunPacket::CreateSuccessResponse()
// {
//     MS_TRACE();

//     MS_ASSERT(
//         this->klass == Class::REQUEST,
//         "attempt to create a success response for a non Request STUN
//         packet");

//     return new StunPacket(
//         Class::SUCCESS_RESPONSE, this->method, this->transaction_id_,
//         nullptr, 0);
// }

// StunPacket* StunPacket::CreateErrorResponse(uint16_t errorCode)
// {
//     MS_TRACE();

//     MS_ASSERT(
//         this->klass == Class::REQUEST,
//         "attempt to create an error response for a non Request STUN packet");

//     auto* response = new StunPacket(
//         Class::ERROR_RESPONSE, this->method, this->transaction_id_, nullptr,
//         0);

//     response->SetErrorCode(errorCode);

//     return response;
// }

// void StunPacket::Authenticate(const std::string& password)
// {
//     // Just for Request, Indication and SuccessResponse messages.
//     if (this->klass == Class::ERROR_RESPONSE) {
//         MS_ERROR("cannot set password for ErrorResponse messages");

//         return;
//     }

//     this->password = password;
// }

void StunPacket::Pack(Buffer& buf)
{
    // Some useful variables.
    uint16_t username_padded_len = 0;
    uint16_t xor_mapped_address_padded_len = 0;
    bool add_xor_mapped_address = ((xor_mapped_address_.family() != 0) &&
                                   method_ == StunPacket::Method::BINDING &&
                                   klass_ == Class::SUCCESS_RESPONSE);
    bool add_error_code =
        ((error_code_ != 0) && klass_ == Class::ERROR_RESPONSE);
    bool add_message_integrity =
        (klass_ != Class::ERROR_RESPONSE && !password_.empty());
    bool add_fingerprint = true;  // Do always.

    // First calculate the total required size for the entire packet.
    int size = 20;  // Header.
    if (!this->username_.empty()) {
        username_padded_len =
            Align4(static_cast<uint16_t>(this->username_.size()));
        size += 4 + username_padded_len;
    }
    if (this->priority_ != 0) size += 4 + 4;
    if (this->ice_controlling_ != 0) size += 4 + 8;
    if (this->ice_controlled_ != 0) size += 4 + 8;
    if (this->has_use_candidate_) size += 4;
    if (add_xor_mapped_address) {
        switch (xor_mapped_address_.family()) {
            case AF_INET: {
                xor_mapped_address_padded_len = 8;
                size += 4 + 8;
                break;
            }
            case AF_INET6: {
                xor_mapped_address_padded_len = 20;
                size += 4 + 20;
                break;
            }
            default: {
                LOG_ERROR
                    << "invalid inet family in XOR-MAPPED-ADDRESS attribute";
                add_xor_mapped_address = false;
            }
        }
    }
    if (add_error_code) size += 4 + 4;
    if (add_message_integrity) size += 4 + 20;
    if (add_fingerprint) size += 4 + 4;

    // Merge class and method fields into type.
    uint16_t type_field = static_cast<uint16_t>(
        (static_cast<uint16_t>(this->method_) & 0x0f80) << 2);
    type_field |= static_cast<uint16_t>(
        ((static_cast<uint16_t>(this->method_) & 0x0070) << 1));
    type_field |= (static_cast<uint16_t>(this->method_) & 0x000f);
    type_field |= static_cast<uint16_t>(
        (static_cast<uint16_t>(this->klass_) & 0x02) << 7);
    type_field |= static_cast<uint16_t>(
        (static_cast<uint16_t>(this->klass_) & 0x01) << 4);

    // Set type field.
    buf.Append(HostToNetwork16(type_field));
    // Set length field.
    buf.Append(HostToNetwork16(static_cast<uint16_t>(size - 20)));
    // Set magic cookie.
    buf.Append(StunPacket::magic_cookie_, 4);
    // Set TransactionId field.
    buf.Append(transaction_id_);

    // Add atributes.
    // Add USERNAME.
    if (username_padded_len != 0) {
        buf.Append(HostToNetwork16(static_cast<uint16_t>(Attribute::USERNAME)));
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(this->username_.size())));
        buf.Append(username_);
    }
    // Add PRIORITY.
    if (priority_ != 0) {
        buf.Append(HostToNetwork16(static_cast<uint16_t>(Attribute::PRIORITY)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(4)));
        buf.Append(HostToNetwork32(priority_));
    }
    // Add ICE-CONTROLLING.
    if (ice_controlling_ != 0) {
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(Attribute::ICE_CONTROLLING)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(8)));
        buf.Append(HostToNetwork64(ice_controlling_));
    }

    // Add ICE-CONTROLLED.
    if (ice_controlled_ != 0) {
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(Attribute::ICE_CONTROLLED)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(8)));
        buf.Append(HostToNetwork64(ice_controlled_));
    }

    // Add USE-CANDIDATE.
    if (has_use_candidate_) {
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(Attribute::USE_CANDIDATE)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(0)));
    }

    // Add XOR-MAPPED-ADDRESS
    if (add_xor_mapped_address) {
        buf.Append(HostToNetwork16(
            static_cast<uint16_t>(Attribute::XOR_MAPPED_ADDRESS)));
        buf.Append(HostToNetwork16(xor_mapped_address_padded_len));

        switch (xor_mapped_address_.family()) {
            case AF_INET: {
                uint8_t attr_value[8];
                // Set first byte to 0.
                attr_value[0] = 0;
                // Set inet family.
                attr_value[1] = 0x01;
                // Set port and XOR it.
                memcpy(attr_value + 2,
                       &(xor_mapped_address_.sockaddr_in()->sin_port),
                       2);
                attr_value[2] ^= StunPacket::magic_cookie_[0];
                attr_value[3] ^= StunPacket::magic_cookie_[1];
                // Set address and XOR it.
                memcpy(attr_value + 4,
                       &(xor_mapped_address_.sockaddr_in()->sin_addr.s_addr),
                       4);
                attr_value[4] ^= StunPacket::magic_cookie_[0];
                attr_value[5] ^= StunPacket::magic_cookie_[1];
                attr_value[6] ^= StunPacket::magic_cookie_[2];
                attr_value[7] ^= StunPacket::magic_cookie_[3];

                buf.Append(attr_value, 8);
                break;
            }

            case AF_INET6: {
                uint8_t attr_value[20];
                // Set first byte to 0.
                attr_value[0] = 0;
                // Set inet family.
                attr_value[1] = 0x02;
                // Set port and XOR it.
                memcpy(attr_value + 2,
                       &(xor_mapped_address_.sockaddr_in6()->sin6_port),
                       2);
                attr_value[2] ^= StunPacket::magic_cookie_[0];
                attr_value[3] ^= StunPacket::magic_cookie_[1];
                // Set address and XOR it.
                memcpy(attr_value + 4,
                       xor_mapped_address_.sockaddr_in6()->sin6_addr.s6_addr,
                       16);
                attr_value[4] ^= StunPacket::magic_cookie_[0];
                attr_value[5] ^= StunPacket::magic_cookie_[1];
                attr_value[6] ^= StunPacket::magic_cookie_[2];
                attr_value[7] ^= StunPacket::magic_cookie_[3];

                const uint8_t* transaction_id = transaction_id_.data_uint8();
                attr_value[8] ^= transaction_id[0];
                attr_value[9] ^= transaction_id[1];
                attr_value[10] ^= transaction_id[2];
                attr_value[11] ^= transaction_id[3];
                attr_value[12] ^= transaction_id[4];
                attr_value[13] ^= transaction_id[5];
                attr_value[14] ^= transaction_id[6];
                attr_value[15] ^= transaction_id[7];
                attr_value[16] ^= transaction_id[8];
                attr_value[17] ^= transaction_id[9];
                attr_value[18] ^= transaction_id[10];
                attr_value[19] ^= transaction_id[11];

                buf.Append(attr_value, 20);
                break;
            }
        }
    }

    // Add ERROR-CODE.
    if (add_error_code) {
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(Attribute::ERROR_CODE)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(4)));

        auto code_class = static_cast<uint8_t>(error_code_ / 100);
        uint8_t code_number =
            static_cast<uint8_t>(error_code_ - (code_class * 100));

        buf.Append(HostToNetwork16(static_cast<uint16_t>(0)));
        buf.Append(code_class);
        buf.Append(code_number);
    }
    // Add MESSAGE-INTEGRITY.
    if (add_message_integrity) {
        // Ignore FINGERPRINT.
        if (add_fingerprint) {
            set_byte2(
                buf.read_index(), 2, static_cast<uint16_t>(size - 20 - 8));
        }

        // Calculate the HMAC-SHA1 of the packet according to MESSAGE -
        // INTEGRITY rules.
        StringPiece computed_message_integrity =
            crypto::HmacSha1(password_, buf.slice());

        buf.Append(HostToNetwork16(
            static_cast<uint16_t>(Attribute::MESSAGE_INTEGRITY)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(20)));
        buf.Append(computed_message_integrity);

        // Restore length field.
        if (add_fingerprint) {
            set_byte2(buf.read_index(), 2, static_cast<uint16_t>(size - 20));
        }
    }

    // Add FINGERPRINT.
    if (add_fingerprint) {
        // Compute the CRC32 of the packet up to (but excluding) the FINGERPRINT
        // attribute and XOR it with 0x5354554e.
        uint32_t computed_fingerprint = crypto::CRC32(buf.slice()) ^ 0x5354554e;
        buf.Append(
            HostToNetwork16(static_cast<uint16_t>(Attribute::FINGERPRINT)));
        buf.Append(HostToNetwork16(static_cast<uint16_t>(4)));
        buf.Append(HostToNetwork32(computed_fingerprint));

        // Set flag.
        this->has_fingerprint_ = true;
    }

    if (size != buf.readable_bytes()) {
        LOG_ERROR << "stun pack failed, should is " << size << " but is "
                  << buf.readable_bytes();
    }
}

}  // namespace net

}  // namespace baize
