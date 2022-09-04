#ifndef BAIZE_STUN_PACKET_H_
#define BAIZE_STUN_PACKET_H_

#include "net/inet_address.h"
#include "net/net_buffer.h"
#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class StunPacket  // copyable
{
public:
    // STUN message class.
    enum class Class : uint16_t {
        REQUEST = 0,
        INDICATION = 1,
        SUCCESS_RESPONSE = 2,
        ERROR_RESPONSE = 3
    };

    // STUN message method.
    enum class Method : uint16_t { BINDING = 1 };

    // Attribute type.
    enum class Attribute : uint16_t {
        MAPPED_ADDRESS = 0x0001,
        USERNAME = 0x0006,
        MESSAGE_INTEGRITY = 0x0008,
        ERROR_CODE = 0x0009,
        UNKNOWN_ATTRIBUTES = 0x000A,
        REALM = 0x0014,
        NONCE = 0x0015,
        XOR_MAPPED_ADDRESS = 0x0020,
        PRIORITY = 0x0024,
        USE_CANDIDATE = 0x0025,
        SOFTWARE = 0x8022,
        ALTERNATE_SERVER = 0x8023,
        FINGERPRINT = 0x8028,
        ICE_CONTROLLED = 0x8029,
        ICE_CONTROLLING = 0x802A,
        NOMINATION = 0xC001
    };

    // Authentication result.
    enum class Authentication { OK = 0, UNAUTHORIZED = 1, BAD_REQUEST = 2 };

    static bool IsStun(StringPiece packet);

    int Parse(StringPiece packet);
    void Pack(Buffer& buf);
    // Authentication CheckAuthentication(const std::string& localUsername,
    //                                    const std::string& localPassword);
    // void Authenticate(const std::string& password);
    // void Serialize(uint8_t* buffer);

    // getter
    void dump() const;
    Class klass() const { return this->klass_; }
    Method method() const { return this->method_; }
    StringPiece transaction_id() { return this->transaction_id_; }
    const uint8_t* data() const { return this->packet_.data_uint8(); }
    size_t size() const { return this->packet_.size(); }
    StringPiece username() const { return this->username_; }
    uint32_t priority() const { return this->priority_; }
    uint64_t ice_controlling() const { return this->ice_controlling_; }
    uint64_t ice_controlled() const { return this->ice_controlled_; }
    bool has_use_candidate() const { return this->has_use_candidate_; }
    bool HasNomination() const { return this->has_nomination_; }
    uint32_t nomination() const { return this->nomination_; }
    uint16_t error_code() const { return this->error_code_; }
    bool has_message_integrity() const
    {
        return (this->message_integrity_.empty() ? false : true);
    }
    bool has_fingerprint() const { return this->has_fingerprint_; }

    // setter
    void set_class(Class klass) { this->klass_ = klass; }
    void set_method(Method method) { this->method_ = method; }
    void set_transaction_id(StringPiece id) { this->transaction_id_ = id; }
    void set_username(StringPiece username) { this->username_ = username; }
    void set_priority(uint32_t priority) { this->priority_ = priority; }
    void set_ice_controlling(uint64_t ing) { this->ice_controlling_ = ing; }
    void set_ice_controlled(uint64_t ed) { this->ice_controlled_ = ed; }
    void set_use_candidate() { this->has_use_candidate_ = true; }
    void set_nomination(uint32_t nomination) { this->nomination_ = nomination; }
    void set_xor_mapped_address(const InetAddress& addr)
    {
        xor_mapped_address_ = addr;
    }
    void set_error_code(uint16_t errorCode) { this->error_code_ = errorCode; }
    void set_message_integrity(StringPiece message_integrity)
    {
        this->message_integrity_ = message_integrity;
    }
    void set_fingerprint(uint32_t fingerprint)
    {
        this->has_fingerprint_ = true;
        this->fingerprint_ = fingerprint;
    }
    void set_has_nomination() { this->has_nomination_ = true; }
    void set_password(StringPiece password) { this->password_ = password; }

private:
    static const uint8_t magic_cookie_[];

    StringPiece packet_;          // all data
    Class klass_;                 // 2 bytes.
    Method method_;               // 2 bytes.
    StringPiece transaction_id_;  // 12 bytes.

    // STUN attributes.
    StringPiece username_;          // Less than 513 bytes.
    uint32_t priority_ = 0;         // 4 bytes unsigned integer.
    uint64_t ice_controlling_ = 0;  // 8 bytes unsigned integer.
    uint64_t ice_controlled_ = 0;   // 8 bytes unsigned integer.
    bool has_nomination_ = false;
    uint32_t nomination_ = 0;         // 4 bytes unsigned integer.
    bool has_use_candidate_ = false;  // 0 bytes.
    StringPiece message_integrity_;   // 20 bytes.
    bool has_fingerprint_ = false;    // 4 bytes.
    uint32_t fingerprint_ = 0;        // 4 bytes.
    InetAddress xor_mapped_address_;  // 8 or 20 bytes.
    uint16_t error_code_ = 0;         // 4 bytes (no reason phrase).
    StringPiece password_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_STUN_PACKET_H_