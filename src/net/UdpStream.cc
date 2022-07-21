#include "net/UdpStream.h"

#include "log/Logger.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::UdpStream::UdpStream()
  : conn_(std::make_unique<Socket>(creatUdpSocket(AF_INET)))
{
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    loop->registerPollEvent(conn_->getSockfd());
}

net::UdpStream::UdpStream(uint16_t port)
  : bindaddr_(port),
    conn_(std::make_unique<Socket>(creatUdpSocket(bindaddr_.getFamily())))
{
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    loop->registerPollEvent(conn_->getSockfd());
    conn_->bindAddress(bindaddr_);
}

net::UdpStream::~UdpStream()
{
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    loop->unregisterPollEvent(conn_->getSockfd());
}

net::UdpStreamSptr net::UdpStream::asServer(uint16_t port)
{
    return std::make_shared<UdpStream>(port);
}

net::UdpStreamSptr net::UdpStream::asClient()
{
    return std::make_shared<UdpStream>();
}

int net::UdpStream::sendto(const void* buf, int len, const InetAddress& addr)
{
    ssize_t wn = conn_->sendto(buf, len, addr);
    return static_cast<int>(wn);
}

int net::UdpStream::recvfrom(void* buf, int len, InetAddress* addr)
{
    memZero(addr, sizeof(InetAddress));
    ssize_t rn = conn_->recvfrom(buf, len, addr);
    return static_cast<int>(rn);
}

int net::UdpStream::asyncSendto(const void* buf,
                                int len,
                                const InetAddress& addr)
{
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    while (1) {
        loop->checkRoutineTimeout();
        ssize_t wn = conn_->sendto(buf, len, addr);
        if (wn < 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop->addWaitRequest(conn_->getSockfd(),
                                     WAIT_WRITE_REQUEST,
                                     runtime::getCurrentRoutineId());
                loop->backToMainRoutine();
                continue;
            } else {
                LOG_SYSERR << "async write failed";
            }
            errno = saveErrno;
        }
        return static_cast<int>(wn);
    }
}

int net::UdpStream::asyncRecvfrom(void* buf, int len, InetAddress* addr)
{
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    memZero(addr, sizeof(InetAddress));
    while (1) {
        loop->checkRoutineTimeout();
        ssize_t rn = conn_->recvfrom(buf, len, addr);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop->addWaitRequest(conn_->getSockfd(),
                                     WAIT_READ_REQUEST,
                                     runtime::getCurrentRoutineId());
                loop->backToMainRoutine();
                continue;
            } else {
                LOG_SYSERR << "async read failed";
            }
            errno = saveErrno;
        }
        return static_cast<int>(rn);
    }
}

net::InetAddress net::UdpStream::getLocalAddr()
{
    return conn_->getLocalAddr();
}