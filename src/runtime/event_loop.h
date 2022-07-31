#ifndef BAIZE_EVENTLOOP_H_
#define BAIZE_EVENTLOOP_H_

#include <sys/epoll.h>

#include <map>
#include <memory>
#include <vector>

#include "runtime/routine.h"
#include "time/timer_queue.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{

using FunctionCallBack = std::function<void()>;
enum class WaitMode {
    kWaitReadable,
    kWaitWritable,
};
using WaitRequest = std::pair<int, WaitMode>;

class EventLoop;
EventLoop* current_loop();

class EventLoop  // noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void Start();

    void Do(RoutineCallBack func);

    void EnablePoll(int fd);
    void DisablePoll(int fd);

    void WaitReadable(int fd);
    void WaitWritable(int fd);

    void CheckTicks();

    // timers
    time::TimerId RunAt(time::Timestamp time, time::TimerCallback cb);
    time::TimerId RunAfter(double delay, time::TimerCallback cb);
    time::TimerId RunEvery(double interval, time::TimerCallback cb);
    void CancelTimer(time::TimerId timerId);

    // getter
    int epollfd() { return epollfd_; }

private:
    void SpawnRoutine(RoutineCallBack func);
    void ScheduleRoutine(WaitRequest req);
    void MonitorRoutine();

    void RunInLoop(FunctionCallBack func);
    void EpollControl(int op, int fd, epoll_event* ev);

    // getter
    string epoll_event_string(int events);

    int epollfd_;
    std::map<RoutineId, std::unique_ptr<Routine>> routines_;
    std::map<WaitRequest, RoutineId> wait_requests_;
    std::vector<RoutineId> ticks_end_routines_;
    std::vector<FunctionCallBack> functions_;
    std::vector<epoll_event> events_;
    std::unique_ptr<time::TimerQueue> timerqueue_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_EVENTLOOP_H