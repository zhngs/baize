#ifndef BAIZE_WAIT_REQUEST_H_
#define BAIZE_WAIT_REQUEST_H_

#include <functional>

#include "runtime/routine.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{
string epoll_event_string(int events);

using WaitMode = int;
const WaitMode kWaitReadable = 1;
const WaitMode kWaitWritable = 2;

struct WaitRequest {
    WaitRequest(int fd, WaitMode wait_mode) : fd_(fd), wait_mode_(wait_mode) {}
    string debug_string()
    {
        char buf[64];
        string wait_mode;
        if (wait_mode_ == kWaitReadable) {
            wait_mode = "READ";
        } else if (wait_mode_ == kWaitWritable) {
            wait_mode = "WRITE";
        } else {
            wait_mode = "UNKNOWN";
        }
        snprintf(buf,
                 sizeof(buf),
                 "{ fd:%d, wait_mode:%s }",
                 fd_,
                 wait_mode.c_str());
        return buf;
    }

    bool operator==(const WaitRequest& rhs) const
    {
        return (fd_ == rhs.fd_ && wait_mode_ == rhs.wait_mode_);
    }

    int fd_;
    WaitMode wait_mode_;
};

struct WaitRequestHash {
    size_t operator()(const WaitRequest& key) const
    {
        return key.fd_ * 2 + key.wait_mode_;
    }
};

struct ScheduleInfo {
    explicit ScheduleInfo()
      : routineid_(current_routineid()), epoll_revents_(0), selected_(0)
    {
    }
    string debug_string()
    {
        char buf[64];
        snprintf(buf,
                 sizeof(buf),
                 "{ routineid:%ld, epoll_revents:[%s], selected:%s }",
                 routineid_,
                 epoll_event_string(epoll_revents_).c_str(),
                 (selected_ ? "yes" : "no"));
        return buf;
    }

    RoutineId routineid_;
    int epoll_revents_;
    int selected_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_WAIT_REQUEST_H_