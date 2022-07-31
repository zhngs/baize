#include "log/async_logging.h"

#include "time/time_stamp.h"

namespace baize
{

namespace log
{

AsyncLogging::AsyncLogging(const string& basename,
                           off_t rollsize,
                           int flushinterval)
  : flushInterval_(flushinterval),
    running_(false),
    basename_(basename),
    rollSize_(rollsize),
    thread_("async_logging_thread", [=] { ThreadFunc(); }),
    wg_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(std::make_unique<Buffer>()),
    nextBuffer_(std::make_unique<Buffer>()),
    buffers_(),
    output_(std::make_unique<LogFile>(basename_, rollSize_))
{
    buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging()
{
    if (running_) {
        Stop();
    }
}

void AsyncLogging::Append(const char* logline, int len)
{
    thread::MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    } else {
        buffers_.push_back(std::move(currentBuffer_));

        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            currentBuffer_.reset(new Buffer);  // Rarely happens
        }
        currentBuffer_->append(logline, len);
        cond_.Notify();
    }
}

void AsyncLogging::Flush() { output_->Flush(); }

void AsyncLogging::Start()
{
    running_ = true;
    thread_.Start();
    wg_.Wait();
}

void AsyncLogging::Stop()
{
    running_ = false;
    cond_.Notify();
    thread_.Join();
}

void AsyncLogging::ThreadFunc()
{
    assert(running_ == true);
    wg_.Done();
    BufferPtr newBuffer1(std::make_unique<Buffer>());
    BufferPtr newBuffer2(std::make_unique<Buffer>());
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            thread::MutexLockGuard lock(mutex_);
            if (buffers_.empty()) {
                // unusual usage!
                cond_.WaitForSeconds(flushInterval_);
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf,
                     sizeof(buf),
                     "Dropped log messages at %s, %zd larger buffers\n",
                     time::Timestamp::Now().date().c_str(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output_->Append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2,
                                 buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite) {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output_->Append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output_->Flush();
    }
    output_->Flush();
}

}  // namespace log

}  // namespace baize