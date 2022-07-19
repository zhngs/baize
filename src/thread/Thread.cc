#include "thread/Thread.h"

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "log/Logger.h"

using namespace baize;

thread_local pid_t t_tid;
thread_local char t_tidstringbuf[32];
thread_local int t_tidstringlen;
thread_local const char *t_threadname;

pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

class ThreadNameInitializer {
public:
  ThreadNameInitializer() {
    t_tid = gettid();
    t_tidstringlen =
        snprintf(t_tidstringbuf, sizeof(t_tidstringbuf), "%5d ", t_tid);
    t_threadname = "mainThread";
  }
} init;

namespace baize {
namespace thread {

void *startThread(void *arg) {
  Thread *pThread = static_cast<Thread *>(arg);
  t_tid = gettid();
  pThread->tid_ = t_tid;
  pThread->waitgroup_.done();

  t_tidstringlen =
      snprintf(t_tidstringbuf, sizeof(t_tidstringbuf), "%5d ", t_tid);
  t_threadname = pThread->threadname_;
  ::prctl(PR_SET_NAME, t_threadname);

  pThread->func_();

  return nullptr;
}

} // namespace thread
} // namespace baize

pid_t thread::getCurrentTid() { return t_tid; }

string thread::getCurrentTidString() { return t_tidstringbuf; }

const char *thread::getCurrentThreadName() { return t_threadname; }

bool thread::isMainThread() { return t_tid == ::getpid(); }

thread::Thread::Thread(ThreadFunc func, const char *name)
    : func_(func), threadname_(name), pthreadId_(0), tid_(0), started_(false),
      joined_(false), waitgroup_(1) {}

thread::Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void thread::Thread::start() {
  assert(!started_);
  started_ = true;
  int ret = pthread_create(&pthreadId_, nullptr, startThread, this);
  if (ret) {
    LOG_SYSFATAL << "pthread_creat failed";
  }
  waitgroup_.wait();
}

void thread::Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  int ret = pthread_join(pthreadId_, nullptr);
  if (ret) {
    LOG_SYSERR << "pthread_join failed";
  }
}