#ifndef BAIZE_TIMER_H_
#define BAIZE_TIMER_H_

#include "time/time_stamp.h"

namespace baize
{

namespace time
{

using TimerCallback = std::function<void()>;
using TimerId = uint64_t;

class Timer  // noncopyable
{
public:  // types
    friend class TimeWheel;

public:
    Timer() = default;
    Timer(int64_t timer_ms, bool repeat, TimerCallback cb);
    Timer(TimerCallback cb, Timestamp when, double interval = 0);
    ~Timer();
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Run() { cb_(); }

    void Start();
    void Stop();
    void Restart();
    void Restart(Timestamp now);

    // getter
    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    TimerId timerid() const { return id_; }

    // setter
    void set_repeat(bool repeat) { repeat_ = repeat; }
    void set_timer_ms(int64_t ms) { timer_ms_ = ms; }

private:
    Timer* prev_ = nullptr;
    Timer* next_ = nullptr;

    TimerCallback cb_;
    Timestamp expiration_;
    int64_t timer_ms_ = 0;
    bool repeat_ = false;
    double interval_ = 0;
    uint64_t id_ = 0;
};

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMER_H
