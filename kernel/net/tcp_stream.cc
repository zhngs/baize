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
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitReadable(conn_->sockfd(), &info);

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
        return static_cast<int>(rn);
    }
}

int TcpStream::AsyncRead(void* buf, size_t count, double ms, bool& timeout)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        ssize_t rn = conn_->Read(buf, count);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo read_info;
                runtime::WaitRequest item =
                    loop->WaitReadable(conn_->sockfd(), &read_info);

                runtime::ScheduleInfo timer_info;
                time::TimerId timer_id =
                    loop->RunAfter(ms / 1000, [&timer_info, &item] {
                        timer_info.selected_ = 1;
                        runtime::current_loop()->CancelWaiting(item);
                        runtime::current_loop()->Call(timer_info.routineid_);
                    });

                runtime::Return();

                LOG_DEBUG << "AsyncRead timer scheduleinfo = "
                          << timer_info.debug_string()
                          << ", read info = " << read_info.debug_string();

                if (timer_info.selected_) {
                    timeout = true;
                } else if (read_info.selected_) {
                    loop->CancelTimer(timer_id);
                    loop->CancelWaiting(item);
                    continue;
                } else {
                    LOG_FATAL << "can't happen";
                }
            } else {
                LOG_SYSERR << "async read failed";
            }

            errno = saveErrno;
        }
        return static_cast<int>(rn);
    }
}

int TcpStream::AsyncRead()
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        int rn = read_buffer_.ReadFd(sockfd());
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitReadable(conn_->sockfd(), &info);

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

int TcpStream::AsyncRead(double ms, bool& timeout)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        int rn = read_buffer_.ReadFd(sockfd());
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo read_info;
                runtime::WaitRequest item =
                    loop->WaitReadable(conn_->sockfd(), &read_info);

                runtime::ScheduleInfo timer_info;
                time::TimerId timer_id =
                    loop->RunAfter(ms / 1000, [&timer_info, &item] {
                        timer_info.selected_ = 1;
                        runtime::current_loop()->CancelWaiting(item);
                        runtime::current_loop()->Call(timer_info.routineid_);
                    });

                runtime::Return();

                LOG_DEBUG << "AsyncRead timer scheduleinfo = "
                          << timer_info.debug_string()
                          << ", read info = " << read_info.debug_string();

                if (timer_info.selected_) {
                    timeout = true;
                } else if (read_info.selected_) {
                    loop->CancelTimer(timer_id);
                    loop->CancelWaiting(item);
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

int TcpStream::AsyncWrite(const void* buf, size_t count)
{
    runtime::EventLoop* loop = runtime::current_loop();
    ssize_t ret = 0;
    while (1) {
        loop->CheckTicks();
        errno = 0;
        ssize_t wn = conn_->Write(buf, count);
        if (wn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitWritable(conn_->sockfd(), &info);

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
        } else if (wn == 0 && errno != 0) {
            LOG_SYSERR << "async write failed";
        } else if (static_cast<size_t>(wn) < count) {
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

int TcpStream::sockfd() { return conn_->sockfd(); }
string TcpStream::peer_ip_port() { return peeraddr_.ip_port(); }
Buffer* TcpStream::read_buffer() { return &read_buffer_; }

TcpStreamSptr TcpStream::AsyncConnect(const char* ip, uint16_t port)
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress serveraddr(ip, port);
    int fd = CreatTcpSocket(serveraddr.family());
    TcpStreamSptr stream(std::make_shared<TcpStream>(fd, serveraddr));
    int ret = stream->conn_->Connect(serveraddr);
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            runtime::ScheduleInfo info;
            auto req = loop->WaitWritable(fd, &info);

            runtime::Return();

            LOG_DEBUG << "AsyncConnect scheduleinfo = " << info.debug_string();

            if (info.selected_) {
                loop->CancelWaiting(req);
            } else {
                LOG_FATAL << "can't happen";
            }

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