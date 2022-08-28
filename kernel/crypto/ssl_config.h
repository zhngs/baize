#ifndef BAIZE_SSL_CONFIG_H_
#define BAIZE_SSL_CONFIG_H_

#include "openssl/ssl.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class SslConfig  // noncopyable
{
public:
    SslConfig();
    ~SslConfig();
    SslConfig(const SslConfig&) = delete;
    SslConfig& operator=(const SslConfig&) = delete;

    // setter
    int set_tls_server(string cert, string key);
    int set_tls_client();

    // getter
    SSL_CTX* ssl_ctx() { return ctx_; }

private:
    SSL_CTX* ctx_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SSL_CONFIG_H_