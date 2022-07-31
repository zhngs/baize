#include "time/timer.h"

namespace baize
{

namespace time
{

thread_local uint64_t g_timerid = 1;

Timer::Timer(TimerCallback cb, Timestamp when, double interval)
  : cb_(std::move(cb)),
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0),
    id_(g_timerid++)
{
}

void Timer::Restart(Timestamp now)
{
    if (repeat_) {
        expiration_ = AddTime(now, interval_);
    } else {
        expiration_ = Timestamp();
    }
}

}  // namespace time

}  // namespace baize