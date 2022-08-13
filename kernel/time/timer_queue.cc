#include "time/timer_queue.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace time
{

int CreateTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec HowMuchTimeFromNow(Timestamp when)
{
    int64_t us = when.us() - Timestamp::Now().us();
    // 可定时的最短时间是距离现在100ns
    if (us <= 100) {
        us = 100;
    }
    timespec ts;
    ts.tv_sec = static_cast<time_t>(us / Timestamp::kUsPerSec);
    ts.tv_nsec = static_cast<long>((us % Timestamp::kUsPerSec) * 1000);
    return ts;
}

void ResetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec new_value;
    struct itimerspec old_value;
    MemZero(&new_value, sizeof(new_value));
    MemZero(&old_value, sizeof(old_value));
    if (expiration.valid()) {
        new_value.it_value = HowMuchTimeFromNow(expiration);
    }
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret) {
        LOG_SYSERR << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue() : timerfd_(CreateTimerfd()), calling_timers(false) {}

TimerQueue::~TimerQueue() { ::close(timerfd_); }

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval)
{
    std::unique_ptr<Timer> timer(
        std::make_unique<Timer>(std::move(cb), when, interval));
    LOG_TRACE << "add timer {timerid:" << timer->timerid()
              << ", expiration:" << timer->expiration().date()
              << ", repeat:" << timer->repeat() << "}";
    bool shouldreset = false;
    if (timers_.empty() || when < timers_.begin()->second->expiration()) {
        shouldreset = true;
    }

    TimerId timerid = timer->timerid();
    assert(timers_.find(timerid) == timers_.end());
    TimerOrder order(timer->expiration(), timerid);
    assert(ordered_timers_.find(order) == ordered_timers_.end());

    ordered_timers_.insert(order);
    timers_[timerid] = std::move(timer);
    if (shouldreset && !calling_timers) {
        ResetTimerfd(timerfd_, when);
    }
    return timerid;
}

void TimerQueue::RemoveTimer(TimerId id)
{
    assert(timers_.find(id) != timers_.end());
    TimerOrder order(timers_[id]->expiration(), id);
    assert(ordered_timers_.find(order) != ordered_timers_.end());

    if (*ordered_timers_.begin() == order) {
        ordered_timers_.erase(order);
        if (!ordered_timers_.empty()) {
            ResetTimerfd(timerfd_, ordered_timers_.begin()->first);
        } else {
            ResetTimerfd(timerfd_, Timestamp());
        }
    } else {
        ordered_timers_.erase(order);
    }
    LOG_TRACE << "remove Timer" << id << "";
    timers_.erase(id);
}

void TimerQueue::HandleActiveTimer()
{
    LOG_TRACE << "TimerQueue Routine start";
    while (1) {
        Timestamp when = AsyncReadTimerfd();

        // get expired timers
        std::vector<TimerId> expired;
        TimerOrder order(when, UINT64_MAX);
        auto end = ordered_timers_.lower_bound(order);
        for (auto begin = ordered_timers_.begin(); begin != end; begin++) {
            expired.push_back(begin->second);
        }
        if (!expired.empty()) {
            ordered_timers_.erase(ordered_timers_.begin(), end);
        }

        // run timer callback and handle repeat timer
        calling_timers = true;
        for (TimerId id : expired) {
            auto& timer = timers_[id];
            timer->Run();
            if (timer->repeat()) {
                timer->Restart(when);
                ordered_timers_.insert(TimerOrder(timer->expiration(), id));
            } else {
                timers_.erase(id);
            }
        }
        calling_timers = false;

        // reset timers
        if (!ordered_timers_.empty()) {
            ResetTimerfd(timerfd_, ordered_timers_.begin()->first);
        }
    }
}

Timestamp TimerQueue::AsyncReadTimerfd()
{
    runtime::EventLoop* loop = runtime::current_loop();
    while (1) {
        loop->CheckTicks();
        uint64_t howmany;
        Timestamp now(Timestamp::Now());
        ssize_t n = ::read(timerfd_, &howmany, sizeof(howmany));
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                runtime::ScheduleInfo info;
                auto req = loop->WaitReadable(timerfd_, &info);

                runtime::Return();

                LOG_DEBUG << "AsyncReadTimerfd scheduleinfo = "
                          << info.debug_string();

                if (info.selected_) {
                    loop->CancelWaiting(req);
                    continue;
                } else {
                    LOG_FATAL << "can't happen";
                }
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

void TimerQueue::Start()
{
    runtime::EventLoop* loop = runtime::current_loop();

    loop->EnablePoll(timerfd_);
    loop->Do([=] { HandleActiveTimer(); });
}

}  // namespace time

}  // namespace baize