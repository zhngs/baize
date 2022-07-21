#ifndef BAIZE_TIMERQUEUE_H
#define BAIZE_TIMERQUEUE_H

#include <map>
#include <memory>

#include "time/TimeType.h"
#include "time/Timer.h"

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

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void removeTimer(TimerId id);

    void start();

private:
    void handleActiveTimer();
    Timestamp asyncReadTimerfd();

    const int timerfd_;
    std::map<TimerId, std::unique_ptr<Timer>> timers_;
};

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMERQUEUE_H