#include "time/Timer.h"

using namespace baize;

thread_local uint64_t g_timerId = 0;

time::Timer::Timer(TimerCallback cb, Timestamp when, double interval)
  : cb_(cb),
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0),
    id_(g_timerId++)
{
}

void time::Timer::restart(Timestamp now)
{
  if (repeat_) {
      expiration_ = addTime(now, interval_);
  } else {
      expiration_ = Timestamp::invalid();
  }
}