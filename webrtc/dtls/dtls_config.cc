#include "webrtc/dtls/dtls_config.h"

#include "log/logger.h"
#include "webrtc/dtls/dtls_transport.h"

namespace baize
{

namespace net
{

int OnSslCertVerify(int preverify, X509_STORE_CTX* ctx)
{
    // Always valid since DTLS certificates are self-signed.
    return 1;
}

void OnSslInfo(const SSL* ssl, int where, int ret)
{
    static_cast<DtlsTransport*>(SSL_get_ex_data(ssl, 0))->OnSslInfo(where, ret);
}

DtlsConfig::DtlsConfig() {}

DtlsConfig::~DtlsConfig() {}

DtlsConfig::Uptr DtlsConfig::New(string cert_path, string key_path)
{
    Uptr config_uptr = std::make_unique<DtlsConfig>();
    config_uptr->ssl_config_.set_dtls(cert_path, key_path);
    SSL_CTX* ctx = config_uptr->ssl_ctx();

    SSL_CTX_set_options(ctx,
                        SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET |
                            SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_QUERY_MTU);
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);

    SSL_CTX_set_read_ahead(ctx, 1);

    SSL_CTX_set_verify_depth(ctx, 4);

    SSL_CTX_set_verify(ctx,
                       SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       OnSslCertVerify);
    SSL_CTX_set_info_callback(ctx, OnSslInfo);

    int err = SSL_CTX_set_cipher_list(
        ctx,
        "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK");
    if (err == 0) {
        LOG_ERROR << "SSL_CTX_set_cipher_list() failed";
        return Uptr();
    }

    SSL_CTX_set_ecdh_auto(DtlsTransport::sslCtx, 1);

    err = SSL_CTX_set_tlsext_use_srtp(
        ctx, "SRTP_AEAD_AES_128_GCM:SRTP_AES128_CM_SHA1_80");
    if (err != 0) {
        LOG_ERROR << "SSL_CTX_set_tlsext_use_srtp() failed";
        return Uptr();
    }

    return config_uptr;
}

}  // namespace net

}  // namespace baize
