#include "net/tcp_listener.h"

#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

TcpListener::TcpListener(uint16_t port)
  : started_(false),
    listenaddr_(port),
    sock_(std::make_unique<Socket>(CreatTcpSocket((listenaddr_.family()))))
{
    sock_->set_reuse_addr(true);
}

TcpListener::~TcpListener()
{
    if (started_) {
        runtime::EventLoop* loop = runtime::current_loop();
        loop->DisablePoll(sock_->sockfd());
    }
}

void TcpListener::Start()
{
    started_ = true;
    sock_->BindAddress(listenaddr_);
    sock_->Listen();
    runtime::EventLoop* loop = runtime::current_loop();
    loop->EnablePoll(sock_->sockfd());
}

TcpStreamSptr TcpListener::Accept()
{
    InetAddress peeraddr;
    int connfd = sock_->Accept(&peeraddr);
    if (connfd < 0) {
        return TcpStreamSptr();
    } else {
        return std::make_shared<TcpStream>(connfd, peeraddr);
    }
}

TcpStreamSptr TcpListener::AsyncAccept()
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress peeraddr;
    while (1) {
        loop->CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                runtime::WaitRequest item =
                    loop->WaitReadable(sock_->sockfd(), &info);

                runtime::Return();

                LOG_DEBUG << "AsyncAccept scheduleinfo = "
                          << info.debug_string();

                loop->CancelWaiting(item);
                continue;
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

TcpStreamSptr TcpListener::AsyncAccept(double ms)
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress peeraddr;
    while (1) {
        loop->CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo accept_info;
                runtime::WaitRequest item =
                    loop->WaitReadable(sock_->sockfd(), &accept_info);

                runtime::ScheduleInfo timer_info;
                time::TimerId timer_id =
                    loop->RunAfter(ms / 1000, [&timer_info, &item] {
                        timer_info.selected_ = 1;
                        runtime::current_loop()->CancelWaiting(item);
                        runtime::current_loop()->Call(timer_info.routineid_);
                    });

                runtime::Return();

                LOG_DEBUG << "AsyncAccept timer scheduleinfo = "
                          << timer_info.debug_string()
                          << ", accept info = " << accept_info.debug_string();

                if (timer_info.selected_) {
                    return TcpStreamSptr();
                } else if (accept_info.selected_) {
                    loop->CancelTimer(timer_id);
                    loop->CancelWaiting(item);
                    continue;
                } else {
                    LOG_FATAL << "can't happend";
                }
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

int TcpListener::sockfd() { return sock_->sockfd(); }

}  // namespace net

}  // namespace baize