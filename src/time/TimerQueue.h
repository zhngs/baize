#ifndef BAIZE_TIMERQUEUE_H
#define BAIZE_TIMERQUEUE_H

#include "time/TimeType.h"

#include <map>
#include <memory>

namespace baize
{

namespace runtime
{
class EventLoop;    
} // namespace runtime


namespace time
{

class Timer;

class TimerQueue //noncopyable
{
public:
    TimerQueue(runtime::EventLoop* loop);
    ~TimerQueue();
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void removeTimer(TimerId id);

    void start();
private:
    void handleActiveTimer();
    Timestamp asyncReadTimerfd();

    runtime::EventLoop* loop_;
    const int timerfd_;
    std::map<TimerId, std::unique_ptr<Timer>> timers_;
};
    
} // namespace time
    
} // namespace baize


#endif //BAIZE_TIMERQUEUE_H