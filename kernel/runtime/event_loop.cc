#include "runtime/event_loop.h"

#include "log/logger.h"
#include "process/signal.h"
#include "runtime/routine.h"

namespace baize
{

namespace runtime
{

/**
 * function declaration
 */
void EpollControl(int epollfd, int op, int fd, epoll_event* ev);

/**
 * global variable
 */
thread_local EventLoop* tg_loop = nullptr;

/**
 * class function
 */
EventLoop::EventLoop(int routinesize)
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    routine_pool_(std::make_unique<RoutinePool>(routinesize)),
    events_(kEpollEventSize),
    time_wheel_(time::TimeWheel::New())
{
    if (tg_loop) {
        LOG_FATAL << "another loop exists";
    }
    tg_loop = this;

    process::TakeOverSignal();
}

EventLoop::~EventLoop() {}

void EventLoop::Start()
{
    std::vector<FunctionCallBack> functions;

    const int kEpollTimeout = 1;
    while (1) {
        int epolltime = kEpollTimeout;

        // call function in loop
        if (!functions_.empty()) {
            functions = std::move(functions_);
            for (auto& func : functions) {
                func();
            }
            functions.clear();
        }

        // set epolltime for quick reaction
        if (!functions_.empty()) {
            epolltime = std::min(0, epolltime);
        }

        // do epoll
        int num_events = ::epoll_wait(epollfd_,
                                      &*events_.begin(),
                                      static_cast<int>(events_.size()),
                                      epolltime);
        int saved_errno = errno;
        if (num_events > 0) {
            LOG_TRACE << num_events << " events happen";
            if (static_cast<size_t>(num_events) == events_.size()) {
                events_.resize(events_.size() * 2);
            }
            for (int i = 0; i < num_events; i++) {
                AsyncPark* park = static_cast<AsyncPark*>(events_[i].data.ptr);
                park->Schedule(events_[i].events);
            }
        } else if (num_events == 0) {
            // LOG_TRACE << "nothing happened";
        } else {
            LOG_TRACE << "epoll error happened";
            // error happens, log uncommon ones
            if (saved_errno != EINTR) {
                errno = saved_errno;
                LOG_SYSERR << "epoll_wait()";
            }
        }

        // todo: this may be called many times in 1ms
        time_wheel_->TurnWheel();
    }
}

void EventLoop::Do(RoutineCallBack func, string routine_name)
{
    RunInLoop([this, func, routine_name] {
        routine_pool_->Start(func, routine_name);
    });
}

void EventLoop::RunRoutineInLoop(Routine* routine)
{
    if (routine == nullptr) return;
    RunInLoop([routine] { routine->Call(); });
}

void EventLoop::EnablePoll(AsyncPark* park)
{
    epoll_event ev;
    ev.events = (EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    ev.data.ptr = park;
    EpollControl(epollfd_, EPOLL_CTL_ADD, park->fd(), &ev);
}

void EventLoop::DisablePoll(AsyncPark* park)
{
    epoll_event ev;
    MemoryZero(&ev, sizeof(ev));
    EpollControl(epollfd_, EPOLL_CTL_DEL, park->fd(), &ev);
}

void EventLoop::RunInLoop(FunctionCallBack func)
{
    if (is_main_routine()) {
        func();
    } else {
        functions_.push_back(func);
    }
}

/**
 * free function
 */
EventLoop* current_loop()
{
    assert(tg_loop != nullptr);
    return tg_loop;
}

void EpollControl(int epollfd, int op, int fd, epoll_event* ev)
{
    if (::epoll_ctl(epollfd, op, fd, ev) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op=" << op << " fd=" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op=" << op << " fd=" << fd;
        }
    }
}

}  // namespace runtime

}  // namespace baize