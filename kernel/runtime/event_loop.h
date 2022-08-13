#ifndef BAIZE_EVENTLOOP_H_
#define BAIZE_EVENTLOOP_H_

#include <sys/epoll.h>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "runtime/routine_pool.h"
#include "runtime/wait_request.h"
#include "time/timer_queue.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{

// C10K
const int kRoutinePoolSize = 10000;

using FunctionCallBack = std::function<void()>;

class EventLoop;
EventLoop* current_loop();

string epoll_event_string(int events);

class EventLoop  // noncopyable
{
public:
    EventLoop(int routine_num = kRoutinePoolSize);
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void Start();

    // 添加并启动一个协程
    void Do(RoutineCallBack func);
    // 启动已有的协程
    void Call(RoutineId id);

    void EnablePoll(int fd);
    void DisablePoll(int fd);

    WaitRequest WaitReadable(int fd, ScheduleInfo* info);
    WaitRequest WaitWritable(int fd, ScheduleInfo* info);
    void CancelWaiting(WaitRequest request);

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
    void ScheduleRoutine(WaitRequest req, int events);
    void MonitorRoutine();

    void RunInLoop(FunctionCallBack func);
    void EpollControl(int op, int fd, epoll_event* ev);

    int epollfd_;
    // routine pool
    std::unique_ptr<RoutinePool> routine_pool_;

    // WaitRequests
    std::unordered_map<WaitRequest, ScheduleInfo*, WaitRequestHash>
        wait_requests_;

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