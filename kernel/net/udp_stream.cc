#include "net/udp_stream.h"

#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

UdpStream::UdpStream()
  : conn_(std::make_unique<Socket>(CreatUdpSocket(AF_INET)))
{
    runtime::EventLoop* loop = runtime::current_loop();
    loop->EnablePoll(conn_->sockfd());
}

UdpStream::UdpStream(uint16_t port)
  : bindaddr_(port),
    conn_(std::make_unique<Socket>(CreatUdpSocket(bindaddr_.family())))
{
    runtime::EventLoop* loop = runtime::current_loop();
    loop->EnablePoll(conn_->sockfd());
    conn_->BindAddress(bindaddr_);
}

UdpStream::~UdpStream()
{
    runtime::EventLoop* loop = runtime::current_loop();
    loop->DisablePoll(conn_->sockfd());
}

UdpStreamSptr UdpStream::AsServer(uint16_t port)
{
    return std::make_shared<UdpStream>(port);
}

UdpStreamSptr UdpStream::AsClient() { return std::make_shared<UdpStream>(); }

int UdpStream::SendTo(const void* buf, int len, const InetAddress& addr)
{
    ssize_t wn = conn_->SendTo(buf, len, addr);
    return static_cast<int>(wn);
}

int UdpStream::RecvFrom(void* buf, int len, InetAddress* addr)
{
    MemZero(addr, sizeof(InetAddress));
    ssize_t rn = conn_->RecvFrom(buf, len, addr);
    return static_cast<int>(rn);
}

int UdpStream::AsyncSendto(const void* buf, int len, const InetAddress& addr)
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        ssize_t wn = conn_->SendTo(buf, len, addr);
        if (wn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitWritable(conn_->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "AsyncSendto scheduleinfo = "
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
        }
        return static_cast<int>(wn);
    }
}

int UdpStream::AsyncRecvFrom(void* buf, int len, InetAddress* addr)
{
    runtime::EventLoop* loop = runtime::current_loop();
    MemZero(addr, sizeof(InetAddress));
    while (1) {
        loop->CheckTicks();
        ssize_t rn = conn_->RecvFrom(buf, len, addr);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitReadable(conn_->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "AsyncRecvFrom scheduleinfo = "
                          << info.debug_string();

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

int UdpStream::AsyncRecvFrom(
    void* buf, int len, InetAddress* addr, double ms, bool& timeout)
{
    runtime::EventLoop* loop = runtime::current_loop();
    MemZero(addr, sizeof(InetAddress));
    while (1) {
        loop->CheckTicks();
        ssize_t rn = conn_->RecvFrom(buf, len, addr);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo read_info;
                auto req = loop->WaitReadable(conn_->sockfd(), &read_info);

                runtime::ScheduleInfo timer_info;
                auto timer_id = loop->RunAfter(ms / 1000, [&timer_info, &req] {
                    timer_info.selected_ = 1;
                    runtime::current_loop()->CancelWaiting(req);
                    runtime::current_loop()->Call(timer_info.routineid_);
                });

                runtime::Return();

                LOG_DEBUG << "AsyncRecvFrom timer info = "
                          << timer_info.debug_string()
                          << ", read info = " << read_info.debug_string();

                if (timer_info.selected_) {
                    timeout = true;
                } else if (read_info.selected_) {
                    loop->CancelTimer(timer_id);
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

InetAddress UdpStream::localaddr() { return conn_->localaddr(); }

}  // namespace net

}  // namespace baize
