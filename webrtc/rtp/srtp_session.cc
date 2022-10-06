#include "webrtc/rtp/srtp_session.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

SrtpSessionUptr SrtpSession::New(Type type,
                                 StringPiece use_srtp,
                                 StringPiece key)
{
    SrtpSessionUptr srtp_session = std::make_unique<SrtpSession>();

    LOG_INFO << "use_srtp:" << use_srtp << ", key: " << log::DumpHexFormat(key);

    srtp_policy_t policy;
    MemZero(&policy, sizeof(policy));

    if (use_srtp == "SRTP_AEAD_AES_128_GCM") {
        srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtp);
        srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtcp);
    } else if (use_srtp == "SRTP_AES128_CM_SHA1_80") {
        srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
        srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
    } else {
        LOG_ERROR << "use_srtp err: " << use_srtp;
        return SrtpSessionUptr();
    }

    switch (type) {
        case Type::kInBound:
            policy.ssrc.type = ssrc_any_inbound;
            break;
        case Type::kOutBound:
            policy.ssrc.type = ssrc_any_outbound;
            break;
        default:
            return SrtpSessionUptr();
    }

    if (key.size() != policy.rtp.cipher_key_len) {
        LOG_ERROR << "cipher key len err";
        return SrtpSessionUptr();
    }

    policy.ssrc.value = 0;
    policy.key = key.data_uint8();
    policy.allow_repeat_tx = 1;
    policy.window_size = 1024;
    policy.next = nullptr;

    // Set the SRTP session.
    srtp_err_status_t err = srtp_create(&srtp_session->session_, &policy);
    if (err != srtp_err_status_ok) {
        LOG_ERROR << "srtp create err: " << err;
        return SrtpSessionUptr();
    }

    return srtp_session;
}

SrtpSession::SrtpSession() {}

SrtpSession::~SrtpSession() {}

bool SrtpSession::EncryptRtp(StringPiece& packet)
{
    uint8_t* data = packet.data_uint8();
    int size = packet.size();

    srtp_err_status_t err = srtp_protect(session_, data, &size);
    if (err != srtp_err_status_ok || size <= 0) {
        return false;
    }
    packet.set(data, size);

    return true;
}

bool SrtpSession::DecryptRtp(StringPiece& packet)
{
    uint8_t* data = packet.data_uint8();
    int size = packet.size();
    srtp_err_status_t err = srtp_unprotect(session_, data, &size);
    if (err != srtp_err_status_ok || size <= 0) {
        return false;
    }
    packet.set(data, size);

    return true;
}

bool SrtpSession::EncryptRtcp(StringPiece& packet)
{
    uint8_t* data = packet.data_uint8();
    int size = packet.size();
    srtp_err_status_t err = srtp_protect_rtcp(session_, data, &size);
    if (err != srtp_err_status_ok || size <= 0) {
        return false;
    }
    packet.set(data, size);

    return true;
}

bool SrtpSession::DecryptRtcp(StringPiece& packet)
{
    uint8_t* data = packet.data_uint8();
    int size = packet.size();
    srtp_err_status_t err = srtp_unprotect_rtcp(session_, data, &size);
    if (err != srtp_err_status_ok || size <= 0) {
        return false;
    }
    packet.set(data, size);

    return true;
}

}  // namespace net

}  // namespace baize
