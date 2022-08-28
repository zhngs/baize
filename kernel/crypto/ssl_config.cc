#include "crypto/ssl_config.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

SslConfig::SslConfig() : ctx_(nullptr) {}

SslConfig::~SslConfig()
{
    if (ctx_ != nullptr) {
        SSL_CTX_free(ctx_);
    }
}

int SslConfig::set_tls_server(string cert, string key)
{
    ctx_ = SSL_CTX_new(TLS_server_method());
    if (!ctx_) {
        LOG_SYSERR << "Unable to create SSL server context";
        return -1;
    }

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx_, cert.c_str(), SSL_FILETYPE_PEM) <=
        0) {
        LOG_SYSERR << "SSL set cert failed";
        return -1;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx_, key.c_str(), SSL_FILETYPE_PEM) <= 0) {
        LOG_SYSERR << "SSL set key failed";
        return -1;
    }

    return 0;
}

int SslConfig::set_tls_client()
{
    ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (!ctx_) {
        LOG_SYSERR << "Unable to create SSL client context";
        return -1;
    }
    return 0;
}

}  // namespace net

}  // namespace baize
