#ifndef BAIZE_DTLS_TRANSPORT_H_
#define BAIZE_DTLS_TRANSPORT_H_

#include <memory>

#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "time/timer.h"
#include "util/string_piece.h"
#include "webrtc/dtls/dtls_config.h"

namespace baize
{

namespace net
{

class DtlsTransport  // noncopyable
{
public:
    using Uptr = std::unique_ptr<DtlsTransport>;
    enum class DtlsState { NEW = 1, CONNECTING, CONNECTED, FAILED, CLOSED };
    enum class Role { CLIENT = 1, SERVER };

    // factory
    static Uptr New(SSL_CTX* ctx, UdpStreamSptr stream, InetAddress& addr);
    static bool IsDtls(StringPiece packet);

    DtlsTransport(SSL_CTX* ctx, UdpStreamSptr stream);
    ~DtlsTransport();
    DtlsTransport(const DtlsTransport&) = delete;
    DtlsTransport& operator=(const DtlsTransport&) = delete;

    void OnSslInfo(int where, int ret);
    void Initialize(Role role);
    void ProcessDtlsPacket(StringPiece packet);

    // getter
    bool is_running();
    bool is_connected();
    StringPiece srtp_loacl_master();
    StringPiece srtp_remote_master();

private:
    void SendBioData();
    bool UpdateTimeout();
    int HandleTimeout();
    bool CheckStatus(int ret);
    bool ExtractSrtpKey();

    SSL_CTX* ctx_ = nullptr;
    SSL* ssl_ = nullptr;
    BIO* bio_read_ = nullptr;   // The BIO from which ssl reads.
    BIO* bio_write_ = nullptr;  // The BIO in which ssl writes.

    UdpStreamSptr stream_;
    InetAddress* addr_;

    DtlsState state_ = DtlsState::NEW;
    Role role_;
    bool HandshakeDone_ = false;
    bool ExtractSrtpKey_ = false;

    uint8_t srtp_local_master_[64] = {};
    uint8_t srtp_remote_master_[64] = {};
    int srtp_master_len_ = 0;

    time::Timer timer_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_DTLS_TRANSPORT_H_