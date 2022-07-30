#ifndef BAIZE_TIMERQUEUE_H_
#define BAIZE_TIMERQUEUE_H_

#include <map>
#include <memory>
#include <set>

#include "time/timer.h"

namespace baize
{

namespace time
{

class TimerQueue  // noncopyable
{
public:
    TimerQueue();
    ~TimerQueue();
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    void Start();
    TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);
    void RemoveTimer(TimerId id);

private:
    void HandleActiveTimer();
    Timestamp AsyncReadTimerfd();

    const int timerfd_;
    using TimerOrder = std::pair<Timestamp, TimerId>;
    std::set<TimerOrder> ordered_timers_;
    std::map<TimerId, std::unique_ptr<Timer>> timers_;
};

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMERQUEUE_H