#include "runtime/async_park.h"

#include "log/logger.h"
#include "runtime/event_loop.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{

/**
 * function declaration
 */
string epoll_event_string(int events);

/**
 * class function
 */
AsyncPark::AsyncPark(int fd) : fd_(fd)
{
    if (fd_ >= 0) current_loop()->EnablePoll(this);
}

AsyncPark::~AsyncPark()
{
    if (fd_ >= 0) current_loop()->DisablePoll(this);
}

void AsyncPark::WaitRead()
{
    read_routine_ = current_routine();
    current_routine()->Return();
    read_routine_ = nullptr;
}

void AsyncPark::WatiWrite()
{
    write_routine_ = current_routine();
    current_routine()->Return();
    write_routine_ = nullptr;
}

void AsyncPark::WaitRead(int ms, bool& timeout)
{
    read_routine_ = current_routine();
    current_routine()->Return(ms, timeout);
    read_routine_ = nullptr;
}

void AsyncPark::WatiWrite(int ms, bool& timeout)
{
    write_routine_ = current_routine();
    current_routine()->Return(ms, timeout);
    write_routine_ = nullptr;
}

void AsyncPark::CheckTicks()
{
    assert(!is_main_routine());
    if (current_routine()->Tick()) {
        Routine* routine = current_routine();
        current_loop()->RunInLoop([routine] { routine->Call(); });
        current_routine()->Return();
    }
}

void AsyncPark::Schedule(uint32_t events)
{
    events_ = events;

    LOG_TRACE << "fd " << fd_ << " epoll event is ["
              << epoll_event_string(events) << "]";

    if (events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        ScheduleRead();
    }
    if (events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
        ScheduleWrite();
    }
}

void AsyncPark::ScheduleRead()
{
    if (read_routine_ != nullptr)
        current_loop()->RunInLoop([this] { read_routine_->Call(); });
}

void AsyncPark::ScheduleWrite()
{
    if (write_routine_ != nullptr)
        current_loop()->RunInLoop([this] { write_routine_->Call(); });
}

/**
 * free function
 */
string epoll_event_string(int events)
{
    string s;
    if (events & EPOLLIN) s += " EPOLLIN";
    if (events & EPOLLOUT) s += " EPOLLOUT";
    if (events & EPOLLPRI) s += " EPOLLPRI";
    if (events & EPOLLRDHUP) s += " EPOLLRDHUP";
    if (events & EPOLLHUP) s += " EPOLLHUP";
    if (events & EPOLLERR) s += " EPOLLERR";
    if (s.empty()) {
        s = " EMPTY ";
    } else {
        s += " ";
    }
    return s;
}

}  // namespace runtime

}  // namespace baize
