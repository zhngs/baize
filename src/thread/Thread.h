#ifndef BAIZE_THREAD_H
#define BAIZE_THREAD_H

#include <pthread.h>

#include <functional>

#include "thread/WaitGroup.h"
#include "util/types.h"

namespace baize
{

namespace thread
{

pid_t getCurrentTid();
string getCurrentTidString();
const char* getCurrentThreadName();
bool isMainThread();

class Thread  // noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    Thread(ThreadFunc func, const char* name);
    ~Thread();
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start();
    void join();

    friend void* startThread(void* arg);

private:
    ThreadFunc func_;
    const char* threadname_;
    pthread_t pthreadId_;
    pid_t tid_;
    bool started_;
    bool joined_;
    WaitGroup waitgroup_;
};

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_THREAD_H