#ifndef BAIZE_EVENTLOOP_H
#define BAIZE_EVENTLOOP_H

#include "runtime/RuntimeType.h"

#include <map>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace baize
{

namespace runtime
{

class Routine;

class EventLoop //noncopyable
{
public:
    EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void start();
    void addRoutine(RoutineCallBack func);
    void addWaitRequest(int fd, int mode, uint64_t routineid);
    
    void runInLoop(FunctionCallBack func);

    static EventLoop* getCurrentLoop();
    int getEpollfd() { return epollfd_; }
private:
    using RoutineId = uint64_t;
    std::map<RoutineId, std::unique_ptr<Routine>> routines_;
    std::map<WaitRequest, RoutineId> waitRequests_;

    std::vector<FunctionCallBack> functions_;

    int epollfd_;
    std::vector<epoll_event> events_;
};

    
} // namespace runtime

    
} // namespace baize


#endif //BAIZE_EVENTLOOP_H