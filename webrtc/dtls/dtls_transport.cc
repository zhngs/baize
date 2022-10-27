#include "webrtc/dtls/dtls_transport.h"

#include "log/logger.h"
#include "webrtc/pc/peer_connection.h"

namespace baize
{

namespace net
{

thread_local uint8_t gt_ssl_read_buf[65536];

unsigned int OnSslDtlsTimer(SSL* ssl, unsigned int timer_us)
{
    if (timer_us == 0) {
        return 100000;  // 100ms
    } else if (timer_us >= 10000000) {
        return 10000000;  // 10s
    } else {
        return 2 * timer_us;
    }
}

bool DtlsTransport::IsDtls(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();
    return ((len >= 13) && (data[0] > 19 && data[0] < 64));
}

DtlsTransport::Uptr DtlsTransport::New(PeerConnection* pc, SSL_CTX* ctx)
{
    Uptr dtls_trans = std::make_unique<DtlsTransport>(pc, ctx);

    dtls_trans->ssl_ = SSL_new(ctx);
    if (dtls_trans->ssl_ == nullptr) {
        return Uptr();
    }

    SSL_set_ex_data(dtls_trans->ssl_, 0, static_cast<void*>(dtls_trans.get()));

    dtls_trans->bio_read_ = BIO_new(BIO_s_mem());
    if (dtls_trans->bio_read_ == nullptr) {
        SSL_free(dtls_trans->ssl_);
        LOG_ERROR << "bio new failed";
        return Uptr();
    }

    dtls_trans->bio_write_ = BIO_new(BIO_s_mem());
    if (dtls_trans->bio_write_ == nullptr) {
        SSL_free(dtls_trans->ssl_);
        BIO_free(dtls_trans->bio_read_);
        LOG_ERROR << "bio new failed";
        return Uptr();
    }

    SSL_set_bio(
        dtls_trans->ssl_, dtls_trans->bio_read_, dtls_trans->bio_write_);

    SSL_set_mtu(dtls_trans->ssl_, 1350);
    DTLS_set_link_mtu(dtls_trans->ssl_, 1350);

    DTLS_set_timer_cb(dtls_trans->ssl_, OnSslDtlsTimer);

    return dtls_trans;
}

void DtlsTransport::OnSslInfo(int where, int ret)
{
    int w = where & -SSL_ST_MASK;
    const char* role;

    if ((w & SSL_ST_CONNECT) != 0) {
        role = "client";
    } else if ((w & SSL_ST_ACCEPT) != 0) {
        role = "server";
    } else {
        role = "undefined";
    }

    if ((where & SSL_CB_LOOP) != 0) {
        LOG_INFO << log::TempFmt(
            "[role:%s, action:'%s']", role, SSL_state_string_long(ssl_));
    } else if ((where & SSL_CB_ALERT) != 0) {
        const char* alert_type;
        switch (*SSL_alert_type_string(ret)) {
            case 'W':
                alert_type = "warning";
                break;
            case 'F':
                alert_type = "fatal";
                break;
            default:
                alert_type = "undefined";
        }

        if ((where & SSL_CB_READ) != 0) {
            LOG_INFO << log::TempFmt("received DTLS %s alert: %s",
                                     alert_type,
                                     SSL_alert_desc_string_long(ret));
        } else if ((where & SSL_CB_WRITE) != 0) {
            LOG_INFO << log::TempFmt("sending DTLS %s alert: %s",
                                     alert_type,
                                     SSL_alert_desc_string_long(ret));
        } else {
            LOG_INFO << log::TempFmt("DTLS %s alert: %s",
                                     alert_type,
                                     SSL_alert_desc_string_long(ret));
        }
    } else if ((where & SSL_CB_EXIT) != 0) {
        if (ret == 0) {
            LOG_INFO << log::TempFmt(
                "[role:%s, failed:'%s']", role, SSL_state_string_long(ssl_));
        } else if (ret < 0) {
            LOG_INFO << log::TempFmt(
                "[role: %s, waiting:'%s']", role, SSL_state_string_long(ssl_));
        }
    } else if ((where & SSL_CB_HANDSHAKE_START) != 0) {
        LOG_INFO << "DTLS handshake start";
    } else if ((where & SSL_CB_HANDSHAKE_DONE) != 0) {
        LOG_INFO << "DTLS handshake done";
        HandshakeDone_ = true;
    }
}

DtlsTransport::DtlsTransport(PeerConnection* pc, SSL_CTX* ctx)
  : pc_(pc), ctx_(ctx), timer_([this] { return HandleTimeout(); })
{
}

DtlsTransport::~DtlsTransport()
{
    if (ssl_) {
        SSL_free(ssl_);
        ssl_ = nullptr;
        bio_read_ = nullptr;
        bio_write_ = nullptr;
    }
}

void DtlsTransport::Initialize(Role role)
{
    state_ = DtlsState::CONNECTING;
    role_ = role;
    switch (role) {
        case Role::CLIENT: {
            SSL_set_connect_state(ssl_);
            SSL_do_handshake(ssl_);
            SendBioData();
            UpdateTimeout();
            break;
        }

        case Role::SERVER: {
            SSL_set_accept_state(ssl_);
            SSL_do_handshake(ssl_);
            break;
        }

        default: {
            LOG_ERROR << "role error";
        }
    }
}

void DtlsTransport::ProcessDtlsPacket(StringPiece packet)
{
    const char* data = packet.data();
    int len = packet.size();
    if (!is_running()) return;

    int written = BIO_write(bio_read_, data, len);
    if (written != len) {
        LOG_WARN << "OpenSSL BIO_write() wrote less bytes";
    }
    int read = SSL_read(ssl_, gt_ssl_read_buf, sizeof(gt_ssl_read_buf));
    SendBioData();

    // Check SSL status and return if it is bad/closed.
    if (!CheckStatus(read)) return;

    // Set/update the DTLS timeout.
    UpdateTimeout();
}

bool DtlsTransport::CheckStatus(int ret)
{
    int err = SSL_get_error(ssl_, ret);
    switch (err) {
        case SSL_ERROR_NONE:
            break;
        case SSL_ERROR_WANT_READ:
            break;
        default:
            LOG_ERROR << "dtls error:" << err;
    }

    if (HandshakeDone_ && !ExtractSrtpKey_) {
        ExtractSrtpKey_ = true;
        timer_.Stop();
        // todo: checkout fingerprint
        bool err_key = ExtractSrtpKey();
        if (!err_key) {
            LOG_ERROR << "extract srtp key err";
        }
        return err_key;
    } else if (((SSL_get_shutdown(ssl_) & SSL_RECEIVED_SHUTDOWN) != 0) ||
               err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL) {
        if (state_ == DtlsState::CONNECTED) {
            state_ = DtlsState::CLOSED;
        } else {
            state_ = DtlsState::FAILED;
        }
        return false;
    } else {
        return true;
    }
}

bool DtlsTransport::ExtractSrtpKey()
{
    SRTP_PROTECTION_PROFILE* use_srtp = SSL_get_selected_srtp_profile(ssl_);
    if (!use_srtp) {
        LOG_ERROR << "use srtp is null";
        return false;
    }
    int srtp_key_len = 0;
    int srtp_salt_len = 0;
    use_srtp_ = use_srtp->name;
    if (use_srtp_ == "SRTP_AEAD_AES_128_GCM") {
        srtp_key_len = 16;
        srtp_salt_len = 12;
        srtp_master_len_ = srtp_key_len + srtp_salt_len;
    } else if (use_srtp_ == "SRTP_AES128_CM_SHA1_80") {
        srtp_key_len = 16;
        srtp_salt_len = 14;
        srtp_master_len_ = srtp_key_len + srtp_salt_len;
    } else {
        LOG_ERROR << "unkown use_srtp";
        return false;
    }

    uint8_t srtp_material[64] = {};
    int ret = SSL_export_keying_material(ssl_,
                                         srtp_material,
                                         srtp_master_len_ * 2,
                                         "EXTRACTOR-dtls_srtp",
                                         19,
                                         nullptr,
                                         0,
                                         0);
    if (ret != 1) {
        LOG_ERROR << "SSL_export_keying_material err";
        return false;
    }

    uint8_t* srtp_local_key = nullptr;
    uint8_t* srtp_local_salt = nullptr;
    uint8_t* srtp_remote_key = nullptr;
    uint8_t* srtp_remote_salt = nullptr;
    switch (role_) {
        case Role::SERVER: {
            srtp_remote_key = srtp_material;
            srtp_local_key = srtp_remote_key + srtp_key_len;
            srtp_remote_salt = srtp_local_key + srtp_key_len;
            srtp_local_salt = srtp_remote_salt + srtp_salt_len;
            break;
        }
        case Role::CLIENT: {
            srtp_local_key = srtp_material;
            srtp_remote_key = srtp_local_key + srtp_key_len;
            srtp_local_salt = srtp_remote_key + srtp_key_len;
            srtp_remote_salt = srtp_local_salt + srtp_salt_len;
            break;
        }
        default: {
            LOG_ERROR << "unkown role";
            return false;
        }
    }

    // Create the SRTP local master key.
    memcpy(srtp_local_master_, srtp_local_key, srtp_key_len);
    memcpy(srtp_local_master_ + srtp_key_len, srtp_local_salt, srtp_salt_len);
    // Create the SRTP remote master key.
    memcpy(srtp_remote_master_, srtp_remote_key, srtp_key_len);
    memcpy(srtp_remote_master_ + srtp_key_len, srtp_remote_salt, srtp_salt_len);

    state_ = DtlsState::CONNECTED;
    return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
void DtlsTransport::SendBioData()
{
    if (BIO_eof(bio_write_)) return;

    int64_t read;
    char* data = nullptr;
    read = BIO_get_mem_data(bio_write_, &data);
    if (read <= 0) return;

    pc_->AsyncSend(StringPiece(data, static_cast<int>(read)));
    LOG_INFO << read << " bytes of DTLS data ready to sent to the peer";

    (void)BIO_reset(bio_write_);
}
#pragma GCC diagnostic pop

bool DtlsTransport::UpdateTimeout()
{
    if (HandshakeDone_) return true;

    timeval dtls_timeout = {0, 0};
    uint64_t timeout_ms;

// DTLSv1_get_timeout queries the next DTLS handshake timeout. If there is
// a timeout in progress, it sets *out to the time remaining and returns
// one. Otherwise, it returns zero.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    DTLSv1_get_timeout(ssl_, &dtls_timeout);
#pragma GCC diagnostic pop

    timeout_ms = (dtls_timeout.tv_sec * 1000) + (dtls_timeout.tv_usec / 1000);
    if (timeout_ms == 0) {
        HandleTimeout();
        return true;
    } else if (timeout_ms < 5000) {
        timer_.Start(timeout_ms);
        return true;
    } else {
        LOG_WARN << "DTLS timeout too high, " << timeout_ms << " ms";
        state_ = DtlsState::FAILED;
        return false;
    }
}

int DtlsTransport::HandleTimeout()
{
    if (HandshakeDone_) return time::kTimerStop;

    // DTLSv1_handle_timeout is called when a DTLS handshake timeout expires.
    // If no timeout had expired, it returns 0. Otherwise, it retransmits the
    // previous flight of handshake messages and returns 1. If too many timeouts
    // had expired without progress or an error occurs, it returns -1.
    auto ret = DTLSv1_handle_timeout(ssl_);
    if (ret == 1) {
        SendBioData();
        UpdateTimeout();
    } else if (ret == -1) {
        LOG_ERROR << "DTLSv1_handle_timeout() failed";
        state_ = DtlsState::FAILED;
    }

    return time::kTimerStop;
}

bool DtlsTransport::is_running()
{
    switch (state_) {
        case DtlsState::NEW:
            return false;
        case DtlsState::CONNECTING:
        case DtlsState::CONNECTED:
            return true;
        case DtlsState::FAILED:
        case DtlsState::CLOSED:
            return false;
        default:
            return false;
    }
}

bool DtlsTransport::is_connected() { return state_ == DtlsState::CONNECTED; }

StringPiece DtlsTransport::srtp_local_master()
{
    return StringPiece(srtp_local_master_, srtp_master_len_);
}

StringPiece DtlsTransport::srtp_remote_master()
{
    return StringPiece(srtp_remote_master_, srtp_master_len_);
}

}  // namespace net

}  // namespace baize
