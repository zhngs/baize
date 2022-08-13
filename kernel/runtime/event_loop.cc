#include "runtime/event_loop.h"

#include <signal.h>

#include "log/logger.h"
#include "process/signal.h"
#include "runtime/routine.h"

namespace baize
{

namespace runtime
{

thread_local EventLoop* tg_loop = nullptr;
const int kEpollTimeout = 10000;
const int kEpollEventSize = 16;

EventLoop* current_loop()
{
    assert(tg_loop != nullptr);
    return tg_loop;
}

EventLoop::EventLoop(int routinesize)
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    routine_pool_(std::make_unique<RoutinePool>(routinesize)),
    events_(kEpollEventSize),
    timerqueue_(std::make_unique<time::TimerQueue>())
{
    if (tg_loop) {
        LOG_FATAL << "another loop exists";
    }
    tg_loop = this;

    process::TakeOverSignal();
    timerqueue_->Start();
}

EventLoop::~EventLoop() {}

void EventLoop::Start()
{
    std::vector<FunctionCallBack> functions;

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
                int fd = events_[i].data.fd;
                int events = events_[i].events;
                LOG_TRACE << "fd " << fd << " epoll event is ["
                          << epoll_event_string(events) << "]";
                if (events &
                    (EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, kWaitReadable);
                    ScheduleRoutine(req, events);
                }
                if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, kWaitWritable);
                    ScheduleRoutine(req, events);
                }
            }
        } else if (num_events == 0) {
            LOG_TRACE << "nothing happened";
        } else {
            LOG_TRACE << "epoll error happened";
            // error happens, log uncommon ones
            if (saved_errno != EINTR) {
                errno = saved_errno;
                LOG_SYSERR << "epoll_wait()";
            }
        }

        routine_pool_->Refresh();
    }
}

void EventLoop::Do(RoutineCallBack func)
{
    RunInLoop([this, func] { routine_pool_->Start(func); });
}

void EventLoop::Call(RoutineId id)
{
    RunInLoop([this, id] { routine_pool_->Call(id); });
}

WaitRequest EventLoop::WaitReadable(int fd, ScheduleInfo* info)
{
    assert(!is_main_routine());
    WaitRequest req(fd, kWaitReadable);
    LOG_DEBUG << "add WaitReadable " << req.debug_string() << ":"
              << info->debug_string();

    assert(wait_requests_.find(req) == wait_requests_.end());
    wait_requests_[req] = info;

    return req;
}

WaitRequest EventLoop::WaitWritable(int fd, ScheduleInfo* info)
{
    assert(!is_main_routine());
    WaitRequest req(fd, kWaitWritable);
    LOG_DEBUG << "add WaitWritable " << req.debug_string() << ":"
              << info->debug_string();

    assert(wait_requests_.find(req) == wait_requests_.end());
    wait_requests_[req] = info;

    return req;
}

void EventLoop::CancelWaiting(WaitRequest request)
{
    if (wait_requests_.find(request) != wait_requests_.end()) {
        wait_requests_.erase(request);
    } else {
        LOG_ERROR << "there is no request " << request.debug_string();
    }
}

void EventLoop::CheckTicks()
{
    assert(!is_main_routine());
    RoutineId id = current_routineid();
    auto& routine = routine_pool_->routine(id);
    if (routine.is_ticks_end()) {
        routine.set_ticks(kRoutineTicks);
        Call(id);
        Return();
    } else {
        routine.set_ticks_down();
    }
}

void EventLoop::EnablePoll(int fd)
{
    epoll_event ev;
    ev.events = (EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    ev.data.fd = fd;
    EpollControl(EPOLL_CTL_ADD, fd, &ev);
}

void EventLoop::DisablePoll(int fd)
{
    epoll_event ev;
    MemZero(&ev, sizeof(ev));
    EpollControl(EPOLL_CTL_DEL, fd, &ev);
}

time::TimerId EventLoop::RunAt(time::Timestamp time, time::TimerCallback cb)
{
    return timerqueue_->AddTimer(cb, time, 0);
}

time::TimerId EventLoop::RunAfter(double delay, time::TimerCallback cb)
{
    time::Timestamp when(time::AddTime(time::Timestamp::Now(), delay));
    return RunAt(when, cb);
}

time::TimerId EventLoop::RunEvery(double interval, time::TimerCallback cb)
{
    time::Timestamp when(time::AddTime(time::Timestamp::Now(), interval));
    return timerqueue_->AddTimer(cb, when, interval);
}

void EventLoop::CancelTimer(time::TimerId timerid)
{
    timerqueue_->RemoveTimer(timerid);
}

void EventLoop::ScheduleRoutine(WaitRequest req, int events)
{
    if (wait_requests_.find(req) != wait_requests_.end()) {
        ScheduleInfo* info = wait_requests_[req];
        info->epoll_revents_ = events;
        info->selected_ = 1;

        routine_pool_->Call(info->routineid_);
    }
}

void EventLoop::RunInLoop(FunctionCallBack func)
{
    if (is_main_routine()) {
        func();
    } else {
        functions_.push_back(func);
    }
}

void EventLoop::EpollControl(int op, int fd, epoll_event* ev)
{
    if (::epoll_ctl(epollfd_, op, fd, ev) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op=" << op << " fd=" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op=" << op << " fd=" << fd;
        }
    }
}

string epoll_event_string(int events)
{
    string s;
    if (events & EPOLLIN) s += " EPOLLIN";
    if (events & EPOLLOUT) s += " EPOLLOUT";
    if (events & EPOLLPRI) s += " EPOLLPRI";
    if (events & EPOLLRDHUP) s += " EPOLLRDHUP";
    if (events & EPOLLHUP) s += " EPOLLHUP";
    if (events & EPOLLERR) s += " EPOLLERR";
    if (s.empty()) {
        s = " EMPTY ";
    } else {
        s += " ";
    }
    return s;
}

}  // namespace runtime

}  // namespace baize