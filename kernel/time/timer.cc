#include "time/timer.h"

#include "runtime/event_loop.h"

namespace baize
{

namespace time
{

thread_local uint64_t g_timerid = 1;

Timer::Timer(int64_t timer_ms, bool repeat, TimerCallback cb)
  : cb_(cb),
    expiration_(Timestamp::Now().AddMs(timer_ms)),
    timer_ms_(timer_ms),
    repeat_(repeat)
{
}

Timer::Timer(TimerCallback cb, Timestamp when, double interval)
  : cb_(std::move(cb)),
    expiration_(when),
    repeat_(interval > 0),
    interval_(interval),
    id_(g_timerid++)
{
}

Timer::~Timer() {}

void Timer::Start()
{
    if (!expiration_.valid()) return;
    runtime::current_loop()->AddTimer(this);
}

void Timer::Stop() { runtime::current_loop()->DelTimer(this); }

void Timer::Restart()
{
    Stop();
    if (repeat_) {
        expiration_.AddMs(timer_ms_);
    } else {
        expiration_ = Timestamp::Now().AddMs(timer_ms_);
    }
    Start();
}

void Timer::Restart(Timestamp now)
{
    if (repeat()) {
        expiration_ = AddTime(now, interval_);
    } else {
        expiration_ = Timestamp();
    }
}

}  // namespace time

}  // namespace baize