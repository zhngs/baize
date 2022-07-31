#include "net/tcp_stream.h"

#include "log/logger.h"
#include "net/inet_address.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

TcpStream::TcpStream(int fd, InetAddress peeraddr)
  : conn_(std::make_unique<Socket>(fd)), peeraddr_(peeraddr)
{
    runtime::EventLoop* loop = runtime::current_loop();
    loop->EnablePoll(conn_->sockfd());
}

TcpStream::~TcpStream()
{
    runtime::EventLoop* loop = runtime::current_loop();
    loop->DisablePoll(conn_->sockfd());
}

ssize_t TcpStream::Read(void* buf, size_t count)
{
    return conn_->Read(buf, count);
}

ssize_t TcpStream::Write(const void* buf, size_t count)
{
    return conn_->Write(buf, count);
}

void TcpStream::ShutdownWrite() { return conn_->ShutdownWrite(); }

void TcpStream::set_tcp_nodelay() { conn_->set_tcp_nodelay(true); }

int TcpStream::AsyncRead(void* buf, size_t count)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        ssize_t rn = conn_->Read(buf, count);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop->WaitReadable(conn_->sockfd());
                runtime::Return();
                continue;
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return static_cast<int>(rn);
    }
}

int TcpStream::AsyncWrite(const void* buf, size_t count)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        ssize_t wn = conn_->Write(buf, count);
        if (wn <= 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop->WaitWritable(conn_->sockfd());
                runtime::Return();
                continue;
            } else {
                LOG_SYSERR << "async write failed";
            }
            errno = saveErrno;
        }
        return static_cast<int>(wn);
    }
}

int TcpStream::AsyncReadOrDie(void* buf, size_t count)
{
    int rn = AsyncRead(buf, count);
    assert(rn >= 0);
    return rn;
}

int TcpStream::AsyncWriteOrDie(const void* buf, size_t count)
{
    int wn = AsyncWrite(buf, count);
    assert(wn > 0);
    return wn;
}

int TcpStream::sockfd() { return conn_->sockfd(); }

string TcpStream::peer_ip_port() { return peeraddr_.ip_port(); }

TcpStreamSptr TcpStream::asyncConnect(const char* ip, uint16_t port)
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress serveraddr(ip, port);
    int fd = CreatTcpSocket(serveraddr.family());
    TcpStreamSptr stream(std::make_shared<TcpStream>(fd, serveraddr));
    int ret = stream->conn_->Connect(serveraddr);
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            loop->WaitWritable(fd);
            runtime::Return();

            int err = stream->conn_->socket_error();
            if (err) {
                LOG_ERROR << "asyncConnect SO_ERROR = " << err << " "
                          << log::strerror_tl(err);
                return TcpStreamSptr();
            } else if (stream->conn_->is_self_connect()) {
                LOG_ERROR << "async Connect self connect";
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