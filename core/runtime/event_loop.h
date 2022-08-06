#ifndef BAIZE_EVENTLOOP_H_
#define BAIZE_EVENTLOOP_H_

#include <sys/epoll.h>

#include <map>
#include <memory>
#include <vector>

#include "runtime/routine_pool.h"
#include "time/timer_queue.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{
const int kRoutinePoolSize = 10000;

using FunctionCallBack = std::function<void()>;
enum class WaitMode {
    kWaitReadable,
    kWaitWritable,
};
using WaitRequest = std::pair<int, WaitMode>;
using ScheduleInfo = std::pair<RoutineId, time::TimerId>;

class EventLoop;
EventLoop* current_loop();

class EventLoop  // noncopyable
{
public:
    EventLoop(int routine_num = kRoutinePoolSize);
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void Start();

    void Do(RoutineCallBack func);

    void EnablePoll(int fd);
    void DisablePoll(int fd);

    WaitRequest WaitReadable(int fd);
    WaitRequest WaitReadable(int fd, double ms, bool& timeout);
    WaitRequest WaitWritable(int fd);
    WaitRequest WaitWritable(int fd, double ms, bool& timeout);

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
    void Call(RoutineId id);

    void RunInLoop(FunctionCallBack func);
    void EpollControl(int op, int fd, epoll_event* ev);

    // getter
    string epoll_event_string(int events);

    int epollfd_;
    // routine pool
    std::unique_ptr<RoutinePool> routine_pool_;

    // WaitRequests
    std::map<WaitRequest, ScheduleInfo> wait_requests_;

    // call function in loop
    std::vector<FunctionCallBack> functions_;

    // epoll events
    std::vector<epoll_event> events_;

    // timerqueue
    std::unique_ptr<time::TimerQueue> timerqueue_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_EVENTLOOP_H