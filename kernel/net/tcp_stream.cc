#include "net/tcp_stream.h"

#include "log/logger.h"
#include "net/inet_address.h"

namespace baize
{

namespace net
{

TcpStream::TcpStream(int fd, InetAddress peeraddr)
  : conn_(std::make_unique<Socket>(fd)),
    peeraddr_(peeraddr),
    async_park_(conn_->sockfd())
{
}

TcpStream::~TcpStream() {}

void TcpStream::ShutdownWrite() { return conn_->ShutdownWrite(); }

void TcpStream::set_tcp_nodelay() { conn_->set_tcp_nodelay(true); }

int TcpStream::AsyncRead(void* buf, int count)
{
    while (1) {
        async_park_.CheckTicks();
        int rn = conn_->Read(buf, count);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WaitRead();
                continue;
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return rn;
    }
}

int TcpStream::AsyncRead(void* buf, int count, int ms, bool& timeout)
{
    while (1) {
        async_park_.CheckTicks();
        int rn = conn_->Read(buf, count);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WaitRead(ms, timeout);
                if (!timeout) {
                    continue;
                }
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return rn;
    }
}

int TcpStream::AsyncRead(Buffer& buf)
{
    while (1) {
        async_park_.CheckTicks();
        int rn = buf.ReadFd(sockfd());
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WaitRead();
                continue;
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return rn;
    }
}

int TcpStream::AsyncRead(Buffer& buf, int ms, bool& timeout)
{
    while (1) {
        async_park_.CheckTicks();
        int rn = buf.ReadFd(sockfd());
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WaitRead(ms, timeout);
                if (!timeout) {
                    continue;
                }
            } else {
                LOG_SYSERR << "async read failed";
            }

            errno = saveErrno;
        }
        return rn;
    }
}

int TcpStream::AsyncWrite(const void* buf, int count)
{
    int ret = 0;
    while (1) {
        async_park_.CheckTicks();
        errno = 0;
        int wn = conn_->Write(buf, count);
        if (wn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WatiWrite();
                continue;
            } else {
                LOG_SYSERR << "async write failed";
            }
            errno = saveErrno;
        } else if (wn == 0 && errno != 0) {
            LOG_SYSERR << "async write failed";
        } else if (wn < count) {
            buf = static_cast<const char*>(buf) + wn;
            count -= wn;

            ret += wn;
            continue;
        } else {
            ret += wn;
        }
        return ret;
    }
}

int TcpStream::sockfd() { return conn_->sockfd(); }
string TcpStream::peer_ip_port() { return peeraddr_.ip_port(); }

TcpStreamSptr TcpStream::AsyncConnect(const char* ip, uint16_t port)
{
    InetAddress serveraddr(ip, port);
    int fd = CreateTcpSocket(serveraddr.family());
    TcpStreamSptr stream(std::make_shared<TcpStream>(fd, serveraddr));
    int ret = stream->conn_->Connect(serveraddr);
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            stream->async_park_.WatiWrite();
            int err = stream->conn_->socket_error();
            if (err) {
                LOG_ERROR << "AsyncConnect SO_ERROR = " << err << " "
                          << log::strerror_tl(err);
                return TcpStreamSptr();
            } else if (stream->conn_->is_self_connect()) {
                LOG_ERROR << "AsyncConnect self connect";
                return TcpStreamSptr();
            } else {
                return stream;
            }
        } else {
            return TcpStreamSptr();
        }
    }
    return stream;
}

}  // namespace net

}  // namespace baize