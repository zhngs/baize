#ifndef BAIZE_DTLS_CONFIG_H_
#define BAIZE_DTLS_CONFIG_H_

#include <memory>

#include "crypto/ssl_config.h"

namespace baize
{

namespace net
{

class DtlsConfig  // noncopyable
{
public:
    using Uptr = std::unique_ptr<DtlsConfig>;

    // factory
    static Uptr New(string cert_path, string key_path);

    DtlsConfig();
    ~DtlsConfig();
    DtlsConfig(const DtlsConfig&) = delete;
    DtlsConfig& operator=(const DtlsConfig&) = delete;

    // getter
    SSL_CTX* ssl_ctx() { return ssl_config_.ssl_ctx(); }

private:
    SslConfig ssl_config_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_DTLS_CONFIG_H_