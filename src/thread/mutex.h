#ifndef BAIZE_MUTEX_H_
#define BAIZE_MUTEX_H_

#include <assert.h>
#include <pthread.h>

namespace baize
{

namespace thread
{

class MutexLock  // noncopyable
{
public:
    MutexLock() { assert(pthread_mutex_init(&mutex_, nullptr) == 0); }
    ~MutexLock() { assert(pthread_mutex_destroy(&mutex_) == 0); }
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

    void Lock() { assert(pthread_mutex_lock(&mutex_) == 0); }
    void Unlock() { assert(pthread_mutex_unlock(&mutex_) == 0); }

    // getter
    pthread_mutex_t* mutex() { return &mutex_; }

private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard  // noncopyable
{
public:
    MutexLockGuard(MutexLock& mutex) : mutex_(mutex) { mutex_.Lock(); }
    ~MutexLockGuard() { mutex_.Unlock(); }
    MutexLockGuard(const MutexLockGuard&) = delete;
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;

private:
    MutexLock& mutex_;
};

#define MutexLockGuard(x) error "Missing guard object name"

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_MUTEX_H
