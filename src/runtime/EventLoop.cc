#include "runtime/EventLoop.h"

#include "log/Logger.h"
#include "runtime/Routine.h"
#include "time/TimerQueue.h"

using namespace baize;

thread_local runtime::EventLoop* loopInThisThread = nullptr;
const int kepollTimeout = 10000;
const int kepollEventSize = 16;

runtime::EventLoop::EventLoop()
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kepollEventSize),
    timerqueue_(std::make_unique<time::TimerQueue>(this))
{
    if (loopInThisThread) {
        LOG_FATAL << "another loop exists";
    }
    loopInThisThread = this;
}

runtime::EventLoop::~EventLoop() {}

runtime::EventLoop* runtime::getCurrentLoop()
{
    assert(loopInThisThread != nullptr);
    return loopInThisThread;
}

void runtime::EventLoop::start()
{
    int epolltime = kepollTimeout;
    std::vector<RoutineId> timeout;

    timerqueue_->start();
    while (1) {
        for (auto& item : functions_) {
            item();
        }
        functions_.clear();

        int numEvents = ::epoll_wait(epollfd_,
                                     &*events_.begin(),
                                     static_cast<int>(events_.size()),
                                     epolltime);
        int savedErrno = errno;
        if (numEvents > 0) {
            LOG_TRACE << numEvents << " events happened";
            if (static_cast<size_t>(numEvents) == events_.size()) {
                events_.resize(events_.size() * 2);
            }
            for (int i = 0; i < numEvents; i++) {
                int fd = events_[i].data.fd;
                int events = events_[i].events;
                LOG_TRACE << "fd " << fd << " epoll event is ["
                          << getEpollEventString(events) << "]";
                if (events &
                    (EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, WAIT_READ_REQUEST);
                    schedule(req);
                }
                if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    WaitRequest req(fd, WAIT_WRITE_REQUEST);
                    schedule(req);
                }
            }
        } else if (numEvents == 0) {
            LOG_TRACE << "nothing happened";
        } else {
            // error happens, log uncommon ones
            if (savedErrno != EINTR) {
                errno = savedErrno;
                LOG_SYSERR << "epoll_wait()";
            }
        }

        timeout = std::move(timeoutRoutines_);
        assert(timeoutRoutines_.size() == 0);
        for (RoutineId id : timeout) {
            routines_[id]->call();
        }
        timeout.clear();

        if (!timeoutRoutines_.empty()) {
            epolltime = 0;
        } else {
            epolltime = kepollTimeout;
        }
    }
}

void runtime::EventLoop::schedule(WaitRequest req)
{
    if (waitRequests_.find(req) != waitRequests_.end()) {
        RoutineId id = waitRequests_[req];
        auto& routinePtr = routines_[id];
        LOG_TRACE << "erase wait request { {fd:" << req.first << ", mode:"
                  << (req.second == WAIT_READ_REQUEST ? "WAIT_READ_REQUEST"
                                                      : "WAIT_WRITE_REQUEST")
                  << "} : routine" << id << " }";
        waitRequests_.erase(req);
        routinePtr->call();
    }
}

void runtime::EventLoop::addAndExecRoutine(RoutineCallBack func)
{
    auto routine = std::make_unique<Routine>(func);
    RoutineId id = routine->getRoutineId();
    routines_[id] = std::move(routine);
    routines_[id]->call();
}

void runtime::EventLoop::addRoutine(RoutineCallBack func)
{
    runInMainRoutine([=] { addAndExecRoutine(func); });
}

void runtime::EventLoop::removeRoutine(RoutineId id)
{
    if (hasRoutine(id)) {
        routines_.erase(id);
    } else {
        LOG_FATAL << "This is no routine" << id;
    }
}

bool runtime::EventLoop::hasRoutine(RoutineId id)
{
    return routines_.find(id) != routines_.end();
}

void runtime::EventLoop::backToMainRoutine() { Routine::hangup(); }

void runtime::EventLoop::addWaitRequest(int fd, int mode, uint64_t routineid)
{
    if (Routine::isMainRoutine()) {
        LOG_FATAL << "addWaitRequest can't be called by main routine";
    }
    WaitRequest key(fd, mode);
    if (waitRequests_.find(key) != waitRequests_.end()) {
        LOG_FATAL << "This WaitRequest exists";
    }
    LOG_TRACE << "add wait request { {fd:" << fd << ", mode:"
              << (mode == WAIT_READ_REQUEST ? "WAIT_READ_REQUEST"
                                            : "WAIT_WRITE_REQUEST")
              << "} : routine" << routineid << " }";
    waitRequests_[key] = routineid;
}

void runtime::EventLoop::checkRoutineTimeout()
{
    if (Routine::isMainRoutine()) {
        LOG_FATAL << "isRoutineTimeout can't be called by main routine";
    }
    RoutineId id = Routine::getCurrentRoutineId();
    auto& routine = routines_[id];
    if (routine->isTimeout()) {
        routine->setTimeout(10);
        timeoutRoutines_.push_back(id);
        backToMainRoutine();
    } else {
        routine->timeDecrease();
    }
}

void runtime::EventLoop::runInMainRoutine(FunctionCallBack func)
{
    if (Routine::isMainRoutine()) {
        LOG_FATAL << "runInMainRoutine can't be called by main routine";
    }
    functions_.push_back(func);
}

void runtime::EventLoop::registerPollEvent(int fd)
{
    epoll_event ev;
    ev.events = (EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    ev.data.fd = fd;
    epollControl(EPOLL_CTL_ADD, fd, &ev);
}

void runtime::EventLoop::unregisterPollEvent(int fd)
{
    epoll_event ev;
    memZero(&ev, sizeof(ev));
    epollControl(EPOLL_CTL_DEL, fd, &ev);
}

void runtime::EventLoop::epollControl(int op, int fd, epoll_event* ev)
{
    if (::epoll_ctl(epollfd_, op, fd, ev) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op=" << op << " fd=" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op=" << op << " fd=" << fd;
        }
    }
}

time::TimerId runtime::EventLoop::runAt(time::Timestamp time,
                                        time::TimerCallback cb)
{
    return timerqueue_->addTimer(cb, time, 0);
}

time::TimerId runtime::EventLoop::runAfter(double delay, time::TimerCallback cb)
{
    time::Timestamp when(time::addTime(time::Timestamp::now(), delay));
    return runAt(when, cb);
}

time::TimerId runtime::EventLoop::runEvery(double interval,
                                           time::TimerCallback cb)
{
    time::Timestamp when(time::addTime(time::Timestamp::now(), interval));
    return timerqueue_->addTimer(cb, when, interval);
}

void runtime::EventLoop::cancelTimer(time::TimerId timerid)
{
    timerqueue_->removeTimer(timerid);
}

uint64_t runtime::getCurrentRoutineId()
{
    return Routine::getCurrentRoutineId();
}

string runtime::EventLoop::getEpollEventString(int events)
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