#ifndef BAIZE_EVENTLOOP_H_
#define BAIZE_EVENTLOOP_H_

#include <sys/epoll.h>

#include <map>
#include <memory>
#include <vector>

#include "runtime/async_park.h"
#include "runtime/routine_pool.h"
#include "time/time_wheel.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{

class EventLoop  // noncopyable
{
public:  // types and constant
    friend class AsyncPark;

    using FunctionCallBack = std::function<void()>;

    static const int kRoutinePoolSize = 10000;
    static const int kEpollEventSize = 16;

public:  // special function
    EventLoop(int routine_num = kRoutinePoolSize);
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

public:  // normal function
    void Start();

    // 添加并启动一个新的协程
    void Do(RoutineCallBack func, string routine_name = "baize-routine");

    void EnablePoll(AsyncPark* park);
    void DisablePoll(AsyncPark* park);

    // time wheel
    void AddTimer(time::Timer* timer) { time_wheel_->AddTimer(timer); }
    void DelTimer(time::Timer* timer) { time_wheel_->DelTimer(timer); }

private:
    void RunRoutineInLoop(Routine* routine);
    void RunInLoop(FunctionCallBack func);

private:
    int epollfd_;
    std::unique_ptr<RoutinePool> routine_pool_;
    std::vector<FunctionCallBack> functions_;
    std::vector<epoll_event> events_;
    time::TimeWheelUptr time_wheel_;
};

/**
 *  global function
 */
EventLoop* current_loop();

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_EVENTLOOP_H