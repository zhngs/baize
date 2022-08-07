#ifndef BAIZE_ASYNCLOGGING_H_
#define BAIZE_ASYNCLOGGING_H_

#include <atomic>
#include <memory>
#include <vector>

#include "log/log_file.h"
#include "thread/thread.h"
#include "util/fixed_buffer.h"

namespace baize
{

namespace log
{

class AsyncLogging  // noncopyable
{
public:
    AsyncLogging(const string& basename, off_t rollsize, int flushinterval = 3);
    ~AsyncLogging();
    AsyncLogging(const AsyncLogging&) = delete;
    AsyncLogging& operator=(const AsyncLogging&) = delete;

    void Append(const char* logline, int len);
    void Flush();
    void Start();
    void Stop();

private:
    void ThreadFunc();

    typedef FixedBuffer<4096 * 1024> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;
    std::atomic<bool> running_;
    const string basename_;
    const off_t rollSize_;
    thread::Thread thread_;
    thread::WaitGroup wg_;
    thread::MutexLock mutex_;
    thread::Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    std::unique_ptr<LogFile> output_;
};

}  // namespace log

}  // namespace baize

#endif  // BAIZE_ASYNCLOGGING_H