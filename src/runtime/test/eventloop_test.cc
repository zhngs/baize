#include "runtime/EventLoop.h"
#include "runtime/Routine.h"

#include "log/Logger.h"
#include "net/TcpListener.h"
#include "net/TcpStream.h"

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;

void echo_server()
{
    TcpListener listener(6060);
    listener.start();
    int listenfd = listener.getSockfd();

    int efd = EventLoop::getCurrentLoop()->getEpollfd();
    epoll_event ev;
    ev.events = (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
    ev.data.fd = listenfd;
    ::epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &ev);

    while (1) {
        TcpStreamSptr stream = listener.accept();
        if (!stream) {
            EventLoop::getCurrentLoop()->addWaitRequest(listenfd, WAIT_READ_REQUEST, Routine::getCurrentRoutineId());
            Routine::hangup();
            continue;
        }
        LOG_INFO << "accept connection " << stream->getPeerIpPort();
    }
}

void echo_server2()
{
    TcpListener listener(6061);
    listener.start();
    int listenfd = listener.getSockfd();

    int efd = EventLoop::getCurrentLoop()->getEpollfd();
    epoll_event ev;
    ev.events = (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
    ev.data.fd = listenfd;
    ::epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &ev);

    while (1) {
        TcpStreamSptr stream = listener.accept();
        if (!stream) {
            EventLoop::getCurrentLoop()->addWaitRequest(listenfd, WAIT_READ_REQUEST, Routine::getCurrentRoutineId());
            Routine::hangup();
            continue;
        }
        LOG_INFO << "accept connection " << stream->getPeerIpPort();
    }
}

int main()
{
    EventLoop loop;
    loop.addRoutine(echo_server);
    loop.addRoutine(echo_server2);
    loop.start();
}