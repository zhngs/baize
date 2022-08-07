#include "thread/thread.h"

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "log/logger.h"

namespace baize
{

namespace thread
{

thread_local pid_t t_tid;
thread_local string t_tidstring;
thread_local string t_threadname;

// global getter
pid_t tid() { return t_tid; }
string tidstring() { return t_tidstring; }
string threadname() { return t_threadname; }
bool is_main_thread() { return t_tid == ::getpid(); }

pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void CallAfterFork()
{
    t_tid = gettid();
    t_tidstring = std::to_string(t_tid);
    t_threadname = "main_thread";
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer() { CallAfterFork(); }
} init_thread;

void* StartThread(void* arg)
{
    Thread* pThread = static_cast<Thread*>(arg);
    t_tid = gettid();
    pThread->tid_ = t_tid;
    t_tidstring = std::to_string(t_tid);
    t_threadname = pThread->threadname_;
    ::prctl(PR_SET_NAME, t_threadname.c_str());

    pThread->waitgroup_.Done();
    pThread->func_();

    return nullptr;
}

Thread::Thread(const char* name, ThreadFunc func)
  : func_(func),
    threadname_(name),
    pthreadid_(0),
    tid_(0),
    started_(false),
    joined_(false),
    waitgroup_(1)
{
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        pthread_detach(pthreadid_);
    }
}

void Thread::Start()
{
    assert(!started_);
    started_ = true;
    int ret = pthread_create(&pthreadid_, nullptr, StartThread, this);
    if (ret) {
        LOG_SYSFATAL << "pthread_creat failed";
    }
    waitgroup_.Wait();
}

void Thread::Join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    int ret = pthread_join(pthreadid_, nullptr);
    if (ret) {
        LOG_SYSERR << "pthread_join failed";
    }
}

}  // namespace thread

}  // namespace baize