#include "net/TcpStream.h"

#include "log/Logger.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::TcpStream::TcpStream(int fd, InetAddress peeraddr)
    : loop_(runtime::getCurrentLoop()), conn_(std::make_unique<Socket>(fd)),
      peeraddr_(peeraddr) {
  loop_->registerPollEvent(conn_->getSockfd());
  // conn_->setTcpNoDelay(true);
}

net::TcpStream::~TcpStream() { loop_->unregisterPollEvent(conn_->getSockfd()); }

ssize_t net::TcpStream::read(void *buf, size_t count) {
  return conn_->read(buf, count);
}

ssize_t net::TcpStream::write(const void *buf, size_t count) {
  return conn_->write(buf, count);
}

void net::TcpStream::shutdownWrite() { return conn_->shutdownWrite(); }

void net::TcpStream::setTcpNoDelay() { conn_->setTcpNoDelay(true); }

int net::TcpStream::asyncRead(void *buf, size_t count) {
  while (1) {
    loop_->checkRoutineTimeout();
    ssize_t rn = conn_->read(buf, count);
    if (rn < 0) {
      int saveErrno = errno;
      if (errno == EAGAIN) {
        loop_->addWaitRequest(conn_->getSockfd(), WAIT_READ_REQUEST,
                              runtime::getCurrentRoutineId());
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

int net::TcpStream::asyncWrite(const void *buf, size_t count) {
  while (1) {
    loop_->checkRoutineTimeout();
    ssize_t wn = conn_->write(buf, count);
    if (wn <= 0) {
      int saveErrno = errno;
      if (errno == EAGAIN) {
        loop_->addWaitRequest(conn_->getSockfd(), WAIT_WRITE_REQUEST,
                              runtime::getCurrentRoutineId());
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

int net::TcpStream::asyncReadOrDie(void *buf, size_t count) {
  int rn = asyncRead(buf, count);
  assert(rn >= 0);
  return rn;
}

int net::TcpStream::asyncWriteOrDie(const void *buf, size_t count) {
  int wn = asyncWrite(buf, count);
  assert(wn > 0);
  return wn;
}

int net::TcpStream::getSockfd() { return conn_->getSockfd(); }

string net::TcpStream::getPeerIpPort() { return peeraddr_.getIpPort(); }

net::TcpStreamSptr net::TcpStream::asyncConnect(const char *ip, uint16_t port) {
  runtime::EventLoop *loop = runtime::getCurrentLoop();
  InetAddress serveraddr(ip, port);
  int fd = creatTcpSocket(serveraddr.getFamily());
  TcpStreamSptr stream(std::make_shared<TcpStream>(fd, serveraddr));
  int ret = stream->conn_->connect(serveraddr);
  if (ret < 0) {
    if (errno == EINPROGRESS) {
      loop->addWaitRequest(fd, WAIT_WRITE_REQUEST,
                           runtime::getCurrentRoutineId());
      loop->backToMainRoutine();

      int err = stream->conn_->getSocketError();
      if (err) {
        LOG_ERROR << "asyncConnect SO_ERROR = " << err << " "
                  << log::strerror_tl(err);
        return TcpStreamSptr();
      } else if (stream->conn_->isSelfConnect()) {
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