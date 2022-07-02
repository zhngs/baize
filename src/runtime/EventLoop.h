#ifndef BAIZE_EVENTLOOP_H
#define BAIZE_EVENTLOOP_H

#include "runtime/RuntimeType.h"
#include "time/TimeType.h"
#include "util/types.h"

#include <map>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace baize
{

namespace time
{
class TimerQueue;
}

namespace runtime
{

class Routine;

class EventLoop //noncopyable
{
public:
    using RoutineId = uint64_t;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void start();
    void schedule(WaitRequest req);

    // called by main routine
    void addAndExecRoutine(RoutineCallBack func);

    // don't have to call this function manually, routine call remove self
    void removeRoutine(RoutineId id);
    bool hasRoutine(RoutineId id);
    

    // called by other routine
    void addRoutine(RoutineCallBack func);
    void addWaitRequest(int fd, int mode, uint64_t routineid);
    void runInMainRoutine(FunctionCallBack func);
    void backToMainRoutine();

    void registerPollEvent(int fd);
    void unregisterPollEvent(int fd);
    void epollControl(int op, int fd, epoll_event* ev);

    time::TimerId runAt(time::Timestamp time, time::TimerCallback cb);
    time::TimerId runAfter(double delay, time::TimerCallback cb);
    time::TimerId runEvery(double interval, time::TimerCallback cb);
    void cancelTimer(time::TimerId timerId);

    static EventLoop* getCurrentLoop();
    RoutineId getCurrentRoutineId();
    int getEpollfd() { return epollfd_; }
    string getEpollEventString(int events);   

private:
    std::map<RoutineId, std::unique_ptr<Routine>> routines_;
    std::map<WaitRequest, RoutineId> waitRequests_;

    std::vector<FunctionCallBack> functions_;

    int epollfd_;
    std::vector<epoll_event> events_;

    std::unique_ptr<time::TimerQueue> timerqueue_;
};

    
} // namespace runtime

    
} // namespace baize


#endif //BAIZE_EVENTLOOP_H