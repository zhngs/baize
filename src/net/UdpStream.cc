#include "net/UdpStream.h"

#include "log/Logger.h"
#include "net/Socket.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::UdpStream::UdpStream()
  : loop_(runtime::getCurrentLoop()),
    conn_(std::make_unique<Socket>(creatUdpSocket(AF_INET)))
{
    loop_->registerPollEvent(conn_->getSockfd());
}

net::UdpStream::UdpStream(uint16_t port)
  : loop_(runtime::getCurrentLoop()),
    bindaddr_(port),
    conn_(std::make_unique<Socket>(creatUdpSocket(bindaddr_.getFamily())))
{
    loop_->registerPollEvent(conn_->getSockfd());
    conn_->bindAddress(bindaddr_);
}

net::UdpStream::~UdpStream()
{
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

int net::UdpStream::asyncSendto(const void* buf, int len, const InetAddress& addr)
{
    while (1) {
        ssize_t wn = conn_->sendto(buf, len, addr);
        if (wn < 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop_->addWaitRequest(conn_->getSockfd(), WAIT_WRITE_REQUEST, runtime::getCurrentRoutineId());
                loop_->backToMainRoutine();
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
    memZero(addr, sizeof(InetAddress));
    while (1) {
        ssize_t rn = conn_->recvfrom(buf, len, addr);
        if (rn < 0) {
            int saveErrno = errno;
            if (errno == EAGAIN) {
                loop_->addWaitRequest(conn_->getSockfd(), WAIT_READ_REQUEST, runtime::getCurrentRoutineId());
                loop_->backToMainRoutine();
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