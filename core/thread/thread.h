#ifndef BAIZE_THREAD_H_
#define BAIZE_THREAD_H_

#include <pthread.h>

#include <functional>

#include "thread/wait_group.h"
#include "util/types.h"

namespace baize
{

namespace thread
{

pid_t tid();
string tidstring();
string threadname();

bool InMainThread();

class Thread  // noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    friend void* StartThread(void* arg);

    Thread(const char* name, ThreadFunc func);
    ~Thread();
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void Start();
    void Join();

private:
    ThreadFunc func_;
    string threadname_;
    pthread_t pthreadid_;
    pid_t tid_;
    bool started_;
    bool joined_;
    WaitGroup waitgroup_;
};

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_THREAD_H