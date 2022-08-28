#include "crypto/tls_stream.h"

#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

TlsStream::TlsStream(TcpStreamSptr stream)
  : stream_(stream), ctx_(nullptr), ssl_(nullptr)
{
}

TlsStream::~TlsStream()
{
    if (ssl_ != nullptr) {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
    }
}

TlsStreamSptr TlsStream::AsServer(SslConfig& config, TcpStreamSptr stream)
{
    TlsStreamSptr tls_stream(std::make_shared<TlsStream>(stream));
    tls_stream->ctx_ = config.ssl_ctx();
    tls_stream->ssl_ = SSL_new(tls_stream->ctx_);
    if (!tls_stream->ctx_ || !tls_stream->ssl_) {
        return TlsStreamSptr();
    }
    SSL_set_fd(tls_stream->ssl_, stream->sockfd());

    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        errno = 0;
        int err = SSL_accept(tls_stream->ssl_);
        if (err <= 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                runtime::WaitRequest item =
                    loop->WaitReadable(stream->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "tls AsServer scheduleinfo = "
                          << info.debug_string();

                loop->CancelWaiting(item);
                continue;
            } else {
                LOG_SYSERR << "tls AsServer failed, SSL_get_errno="
                           << SSL_get_error(tls_stream->ssl_, err);
                return TlsStreamSptr();
            }
        } else {
            break;
        }
    }

    return tls_stream;
}

TlsStreamSptr TlsStream::AsClient(SslConfig& config, TcpStreamSptr stream)
{
    TlsStreamSptr tls_stream(std::make_shared<TlsStream>(stream));
    tls_stream->ctx_ = config.ssl_ctx();
    tls_stream->ssl_ = SSL_new(tls_stream->ctx_);
    if (!tls_stream->ctx_ || !tls_stream->ssl_) {
        return TlsStreamSptr();
    }
    SSL_set_fd(tls_stream->ssl_, stream->sockfd());

    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        errno = 0;
        int err = SSL_connect(tls_stream->ssl_);
        if (err <= 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                runtime::WaitRequest item =
                    loop->WaitReadable(stream->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "tls AsClient scheduleinfo = "
                          << info.debug_string();

                loop->CancelWaiting(item);
                continue;
            } else {
                LOG_SYSERR << "tls AsClient failed, SSL_get_errno="
                           << SSL_get_error(tls_stream->ssl_, err);
                return TlsStreamSptr();
            }
        } else {
            break;
        }
    }

    return tls_stream;
}

int TlsStream::AsyncRead(void* buf, int count)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        int rn = SSL_read(ssl_, buf, count);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitReadable(stream_->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "AsyncRead scheduleinfo = " << info.debug_string();

                if (info.selected_) {
                    loop->CancelWaiting(req);
                    continue;
                } else {
                    LOG_FATAL << "can't happen";
                }
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return rn;
    }
}

int TlsStream::AsyncWrite(const void* buf, int count)
{
    runtime::EventLoop* loop = runtime::current_loop();
    ssize_t ret = 0;
    while (1) {
        loop->CheckTicks();
        errno = 0;
        int wn = SSL_write(ssl_, buf, count);
        if (wn <= 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitWritable(stream_->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "AsyncWrite scheduleinfo = "
                          << info.debug_string();

                if (info.selected_) {
                    loop->CancelWaiting(req);
                    continue;
                } else {
                    LOG_FATAL << "can't happen";
                }
            } else {
                LOG_SYSERR << "async write failed";
            }
            errno = saveErrno;
        } else if (wn < count) {
            buf = static_cast<const char*>(buf) + wn;
            count -= wn;

            ret += wn;
            continue;
        } else {
            ret += wn;
        }
        return static_cast<int>(ret);
    }
}

}  // namespace net

}  // namespace baize
