#include "log/AsyncLogging.h"

#include "log/LogFile.h"
#include "time/Timestamp.h"

using namespace baize;

log::AsyncLogging::AsyncLogging(const string& basename,
                                off_t rollSize,
                                int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    thread_([=] { threadFunc(); }, "AsyncLoggingThread"),
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

log::AsyncLogging::~AsyncLogging()
{
    if (running_) {
        stop();
    }
}

void log::AsyncLogging::append(const char* logline, int len)
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
        cond_.notify();
    }
}

void log::AsyncLogging::flush() { output_->flush(); }

void log::AsyncLogging::start()
{
    running_ = true;
    thread_.start();
    wg_.wait();
}

void log::AsyncLogging::stop()
{
    running_ = false;
    cond_.notify();
    thread_.join();
}

void log::AsyncLogging::threadFunc()
{
    assert(running_ == true);
    wg_.done();
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
                cond_.waitForSeconds(flushInterval_);
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
                     time::Timestamp::now().toFormatString().c_str(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output_->append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2,
                                 buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite) {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output_->append(buffer->data(), buffer->length());
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
        output_->flush();
    }
    output_->flush();
}
