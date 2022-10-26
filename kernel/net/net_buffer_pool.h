#ifndef BAIZE_BUFFER_POOL_H_
#define BAIZE_BUFFER_POOL_H_

#include <memory>
#include <vector>

#include "net/net_buffer.h"

namespace baize
{

namespace net
{

class BufferPool  // noncopyable
{
public:  // types and contant
    static const int kBufferNum = 1000;

    using BufferUptr = std::unique_ptr<Buffer, std::function<void(Buffer*)>>;

public:  // special function
    BufferPool(int size = kBufferNum);
    ~BufferPool();
    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;

public:  // normal function
    BufferUptr AllocBuffer();
    int size() { return static_cast<int>(buffers_.size()); }

private:  // private normal function
    void BufferDelete(Buffer* buffer);

private:
    std::vector<std::unique_ptr<Buffer>> buffers_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_BUFFER_POOL_H_