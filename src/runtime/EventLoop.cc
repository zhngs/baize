#include "runtime/EventLoop.h"

#include "log/Logger.h"
#include "runtime/Routine.h"


using namespace baize;

thread_local runtime::EventLoop* loopInThisThread = nullptr;
const int kepollTimeout = 10000;

runtime::EventLoop::EventLoop()
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(16)
{
    if (loopInThisThread) {
        LOG_FATAL << "another loop exists";
    }
    loopInThisThread = this;    
}

runtime::EventLoop* runtime::EventLoop::getCurrentLoop()
{
    assert(loopInThisThread != nullptr);
    return loopInThisThread;
}

void runtime::EventLoop::start()
{
    for (auto& item : routines_) {
        item.second->call();
    }

    while (1) {
        for (auto& item : functions_) {
            item();
        }

        int numEvents = ::epoll_wait(epollfd_,
                                    &*events_.begin(),
                                    static_cast<int>(events_.size()),
                                    kepollTimeout);
        int savedErrno = errno;
        if (numEvents > 0) {
            LOG_TRACE << numEvents << " events happened";
            if (static_cast<size_t>(numEvents) == events_.size())
            {
                events_.resize(events_.size() * 2);
            }
            for (int i = 0; i < numEvents; i++) {
                int fd = events_[i].data.fd;
                int events = events_[i].events;
                if (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    WaitRequest key(fd, WAIT_READ_REQUEST);
                    if (waitRequests_.find(key) != waitRequests_.end()) {
                        RoutineId id = waitRequests_[key];
                        auto& routinePtr = routines_[id];
                        waitRequests_.erase(key);
                        routinePtr->call();
                    }
                }
                if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    WaitRequest key(fd, WAIT_WRITE_REQUEST);
                    if (waitRequests_.find(key) != waitRequests_.end()) {
                        RoutineId id = waitRequests_[key];
                        auto& routinePtr = routines_[id];
                        waitRequests_.erase(key);
                        routinePtr->call();
                    }
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
    }
}

void runtime::EventLoop::addRoutine(RoutineCallBack func)
{
    auto routine = std::make_unique<Routine>(func);
    routine->call();
    routines_[routine->getRoutineId()] = std::move(routine);
}

void runtime::EventLoop::addWaitRequest(int fd, int mode, uint64_t routineid)
{
    WaitRequest key(fd, mode);
    if (waitRequests_.find(key) != waitRequests_.end()) {
        LOG_FATAL << "This WaitRequest exists";
    }
    waitRequests_[key] = routineid;
}

void runtime::EventLoop::runInLoop(FunctionCallBack func)
{
    functions_.push_back(func);
}