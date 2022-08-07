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
}

EventLoop::~EventLoop() {}

void EventLoop::Start()
{
    std::vector<FunctionCallBack> functions;

    timerqueue_->Start();

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
                    WaitRequest req(fd, WaitMode::kWaitReadable);
                    ScheduleRoutine(req);
                }
                if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, WaitMode::kWaitWritable);
                    ScheduleRoutine(req);
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

WaitRequest EventLoop::WaitReadable(int fd)
{
    assert(!is_main_routine());
    WaitRequest req(fd, WaitMode::kWaitReadable);
    RoutineId id = current_routineid();
    LOG_TRACE << "add wait request { {fd:" << fd << ", mode:"
              << "READ"
              << "} : routine" << id << " }";

    assert(wait_requests_.find(req) == wait_requests_.end());
    ScheduleInfo info(id, 0);
    wait_requests_[req] = info;

    return req;
}

WaitRequest EventLoop::WaitReadable(int fd, double ms, bool& timeout)
{
    WaitRequest req = WaitReadable(fd);
    time::TimerId timer_id = RunAfter(ms / 1000, [this, &timeout, req] {
        RoutineId routine_id = wait_requests_[req].first;
        LOG_TRACE << "routine" << routine_id << " timeout";
        timeout = true;
        wait_requests_.erase(req);
        Call(routine_id);
    });
    wait_requests_[req].second = timer_id;

    return req;
}

WaitRequest EventLoop::WaitWritable(int fd)
{
    assert(!is_main_routine());
    WaitRequest req(fd, WaitMode::kWaitWritable);
    RoutineId id = current_routineid();
    LOG_TRACE << "add wait request { {fd:" << fd << ", mode:"
              << "Write"
              << "} : routine" << id << " }";
    assert(wait_requests_.find(req) == wait_requests_.end());
    ScheduleInfo info(id, 0);
    wait_requests_[req] = info;

    return req;
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

void EventLoop::ScheduleRoutine(WaitRequest req)
{
    if (wait_requests_.find(req) != wait_requests_.end()) {
        RoutineId routine_id = wait_requests_[req].first;
        time::TimerId timer_id = wait_requests_[req].second;
        LOG_TRACE << "erase wait request { {fd:" << req.first << ", mode:"
                  << (req.second == WaitMode::kWaitReadable ? "READ" : "WRITE")
                  << "} : routine" << routine_id << " }";
        wait_requests_.erase(req);

        if (timer_id != 0) {
            CancelTimer(timer_id);
        }

        routine_pool_->Call(routine_id);
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