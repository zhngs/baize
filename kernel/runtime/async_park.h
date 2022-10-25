#ifndef BAIZE_ASYNC_PARK_H_
#define BAIZE_ASYNC_PARK_H_

#include "runtime/routine.h"

namespace baize
{

namespace runtime
{

class AsyncPark  // noncopyable
{
public:  // special function
    AsyncPark() = default;
    explicit AsyncPark(int fd);
    ~AsyncPark();
    AsyncPark(const AsyncPark&) = delete;
    AsyncPark& operator=(const AsyncPark&) = delete;

public:  // normal function
    void WaitRead();
    void WatiWrite();
    void WaitRead(int ms, bool& timeout);
    void WatiWrite(int ms, bool& timeout);

    void CheckTicks();

    void Schedule(uint32_t events);
    void ScheduleRead();
    void ScheduleWrite();

    // getter
    int fd() { return fd_; }
    uint32_t epoll_events() { return events_; }

private:
    int fd_ = -1;
    uint32_t events_ = 0;
    Routine* read_routine_ = nullptr;
    Routine* write_routine_ = nullptr;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ASYNC_PARK_H_