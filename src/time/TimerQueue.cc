#include "time/TimerQueue.h"

#include "log/Logger.h"
#include "runtime/EventLoop.h"
#include "time/Timer.h"
#include "time/Timestamp.h"

#include <sys/timerfd.h>
#include <unistd.h>

using namespace baize;

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(time::Timestamp when)
{
    int64_t us = when.getUs() - time::Timestamp::now().getUs();
    if (us < 100) {
        us = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(us / time::Timestamp::kusPerSec);
    ts.tv_nsec = static_cast<long>((us % time::Timestamp::kusPerSec) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, time::Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memZero(&newValue, sizeof(newValue));
    memZero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_SYSERR << "timerfd_settime()";
    }
}

time::TimerQueue::TimerQueue(runtime::EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd())
{
}

time::TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
}

time::TimerId time::TimerQueue::addTimer(TimerCallback cb,
                                         Timestamp when,
                                         double interval)
{
    std::unique_ptr<Timer> timer(std::make_unique<Timer>(cb, when, interval));
    TimerId timerid = timer->getTimerId();
    bool shouldreset = false;
    if (timers_.empty() || when < timers_.begin()->first.first) {
        shouldreset = true;
    }
    timers_[timerid] = std::move(timer);
    if (shouldreset) {
        resetTimerfd(timerfd_, when);
    }
    return timerid;
}

void time::TimerQueue::removeTimer(time::TimerId id)
{
    // can be O(logn)
    for (auto it = timers_.begin(); it != timers_.end();) {
        if (it->first.second == id.second) {
            timers_.erase(it++);
        } else {
            it++;
        }
    }
}

void time::TimerQueue::handleActiveTimer()
{
    LOG_TRACE << "TimerQueue handleActiveTimer";
    while (1) {
        Timestamp when = asyncReadTimerfd();

        // get expired timers
        std::vector<std::unique_ptr<Timer>> expired;
        TimerId key(when, UINT64_MAX);
        auto end = timers_.lower_bound(key);
        for (auto begin = timers_.begin(); begin != end; begin++) {
            expired.push_back(std::move(begin->second));
        }
        if (!expired.empty()) {
            timers_.erase(timers_.begin(), end);
        }

        // run timer callback and handle repeat timer
        for (auto& item : expired) {
            item->run();
            if (item->isRepeat()) {
                item->restart(when);
                timers_[item->getTimerId()] = std::move(item);
            }
        }

        // reset timers
        if (!timers_.empty()) {
            resetTimerfd(timerfd_, timers_.begin()->first.first);
        }
    }
}

time::Timestamp time::TimerQueue::asyncReadTimerfd()
{
    while (1) {
        loop_->checkRoutineTimeout();
        uint64_t howmany;
        Timestamp now(Timestamp::now());
        ssize_t n = ::read(timerfd_, &howmany, sizeof(howmany));
        if (n < 0) {
            if (errno == EAGAIN) {
                loop_->addWaitRequest(timerfd_, WAIT_READ_REQUEST, runtime::getCurrentRoutineId());
                loop_->backToMainRoutine();
                continue;
            } else {
                LOG_SYSERR << "async read timer failed";
            }
        }
        LOG_TRACE << "readTimerfd " << howmany;
        if (n != sizeof(howmany)) {
            LOG_ERROR << "readTimerfd reads " << n << " bytes instead of 8";
        }
        return now;
    }
}

void time::TimerQueue::start()
{
    LOG_TRACE << "TimerQueue start";
    loop_->registerPollEvent(timerfd_);
    loop_->addAndExecRoutine([=]{ handleActiveTimer(); });
}