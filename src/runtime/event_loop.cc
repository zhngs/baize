#include "runtime/event_loop.h"

#include <signal.h>

#include "log/logger.h"
#include "runtime/routine.h"

namespace baize
{

namespace runtime
{

thread_local EventLoop* tg_loop = nullptr;
const int kEpollTimeout = 10000;
const int kEpollEventSize = 16;
const int kRoutineTicks = 3;

EventLoop* current_loop()
{
    assert(tg_loop != nullptr);
    return tg_loop;
}

class IgnoreSigPipe
{
public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
} ignore_sigpipe;

EventLoop::EventLoop()
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kEpollEventSize),
    timerqueue_(std::make_unique<time::TimerQueue>())
{
    if (tg_loop) {
        LOG_FATAL << "another loop exists";
    }
    tg_loop = this;
}

EventLoop::~EventLoop() {}

void EventLoop::Start()
{
    int epolltime = kEpollTimeout;
    std::vector<RoutineId> ticks_end;

    timerqueue_->Start();
    RunEvery(3, [=] { MonitorRoutine(); });

    while (1) {
        for (auto& func : functions_) {
            func();
        }
        functions_.clear();

        int numEvents = ::epoll_wait(epollfd_,
                                     &*events_.begin(),
                                     static_cast<int>(events_.size()),
                                     epolltime);
        int savedErrno = errno;
        if (numEvents > 0) {
            if (static_cast<size_t>(numEvents) == events_.size()) {
                events_.resize(events_.size() * 2);
            }
            for (int i = 0; i < numEvents; i++) {
                int fd = events_[i].data.fd;
                int events = events_[i].events;
                LOG_TRACE << "fd " << fd << " epoll event is ["
                          << epoll_event_string(events) << "]";
                if (events &
                    (EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, WaitMode::kWaitReadable);
                    ScheduleRoutine(req);
                }
                if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, WaitMode::kWaitWritable);
                    ScheduleRoutine(req);
                }
            }
        } else if (numEvents == 0) {
            LOG_TRACE << "nothing happened";
        } else {
            LOG_TRACE << "epoll error happened";
            // error happens, log uncommon ones
            if (savedErrno != EINTR) {
                errno = savedErrno;
                LOG_SYSERR << "epoll_wait()";
            }
        }

        ticks_end = std::move(ticks_end_routines_);
        assert(ticks_end_routines_.size() == 0);
        for (RoutineId id : ticks_end) {
            routines_[id]->Call();
        }
        ticks_end.clear();

        if (!ticks_end_routines_.empty()) {
            epolltime = 0;
        } else {
            epolltime = kEpollTimeout;
        }
    }
}

void EventLoop::Do(RoutineCallBack func)
{
    RunInLoop([=] { SpawnRoutine(func); });
}

void EventLoop::WaitReadable(int fd)
{
    assert(!is_main_routine());
    WaitRequest req(fd, WaitMode::kWaitReadable);
    RoutineId id = current_routineid();
    assert(wait_requests_.find(req) == wait_requests_.end());
    LOG_TRACE << "add wait request { {fd:" << fd << ", mode:"
              << "READ"
              << "} : routine" << id << " }";
    wait_requests_[req] = id;
}

void EventLoop::WaitWritable(int fd)
{
    assert(!is_main_routine());
    WaitRequest req(fd, WaitMode::kWaitWritable);
    RoutineId id = current_routineid();
    assert(wait_requests_.find(req) == wait_requests_.end());
    LOG_TRACE << "add wait request { {fd:" << fd << ", mode:"
              << "Write"
              << "} : routine" << id << " }";
    wait_requests_[req] = id;
}

void EventLoop::CheckTicks()
{
    assert(!is_main_routine());
    RoutineId id = current_routineid();
    auto& routine = routines_[id];
    if (routine->is_ticks_end()) {
        routine->set_ticks(kRoutineTicks);
        ticks_end_routines_.push_back(id);
        Return();
    } else {
        routine->set_ticks_down();
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
    memZero(&ev, sizeof(ev));
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

string EventLoop::epoll_event_string(int events)
{
    string s;
    if (events & EPOLLIN) s += " EPOLLIN";
    if (events & EPOLLOUT) s += " EPOLLOUT";
    if (events & EPOLLPRI) s += " EPOLLPRI";
    if (events & EPOLLRDHUP) s += " EPOLLRDHUP";
    if (events & EPOLLHUP) s += " EPOLLHUP";
    if (events & EPOLLERR) s += " EPOLLERR";
    s += " ";
    return s;
}

void EventLoop::SpawnRoutine(RoutineCallBack func)
{
    assert(is_main_routine());
    auto routine = std::make_unique<Routine>(func);
    RoutineId id = routine->routineid();
    routines_[id] = std::move(routine);
    routines_[id]->Call();
}

void EventLoop::ScheduleRoutine(WaitRequest req)
{
    if (wait_requests_.find(req) != wait_requests_.end()) {
        RoutineId id = wait_requests_[req];
        auto& routine = routines_[id];
        LOG_TRACE << "erase wait request { {fd:" << req.first << ", mode:"
                  << (req.second == WaitMode::kWaitReadable ? "READ" : "WRITE")
                  << "} : routine" << id << " }";
        wait_requests_.erase(req);
        routine->Call();
    }
}

void EventLoop::MonitorRoutine()
{
    for (auto it = routines_.begin(); it != routines_.end();) {
        if (it->second->is_routine_end()) {
            LOG_TRACE << "erase routine" << it->first;
            routines_.erase(it++);
        } else {
            it++;
        }
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

}  // namespace runtime

}  // namespace baize