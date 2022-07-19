#ifndef BAIZE_TIMER_H
#define BAIZE_TIMER_H

#include "time/TimeType.h"
#include "time/Timestamp.h"

namespace baize {

namespace time {

class Timer // noncopyable
{
public:
  Timer(TimerCallback cb, Timestamp when, double interval);

  void run() { cb_(); }

  Timestamp getExpiration() { return expiration_; }
  bool isRepeat() { return repeat_; }
  TimerId getTimerId() { return TimerId(expiration_, id_); }

  void restart(Timestamp now);

  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;

private:
  const TimerCallback cb_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  const uint64_t id_;
};

} // namespace time

} // namespace baize

#endif // BAIZE_TIMER_H
