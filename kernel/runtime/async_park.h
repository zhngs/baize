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

    // getter
    int fd() { return fd_; }
    Routine* read_routine() { return read_routine_; }
    Routine* write_routine() { return write_routine_; }
    uint32_t epoll_events() { return events_; }

    // setter
    void set_read_routine(Routine* routine) { read_routine_ = routine; }
    void set_write_routine(Routine* routine) { write_routine_ = routine; }
    void set_epoll_events(uint32_t events) { events_ = events; }

private:
    int fd_;
    uint32_t events_ = 0;
    Routine* read_routine_ = nullptr;
    Routine* write_routine_ = nullptr;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ASYNC_PARK_H_