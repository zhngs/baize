#ifndef BAIZE_MTU_BUFFER_POOL_H_
#define BAIZE_MTU_BUFFER_POOL_H_

#include <memory>
#include <vector>

#include "util/fixed_buffer.h"

namespace baize
{

namespace net
{

class MTUBufferPool  // noncopyable
{
public:  // types and contant
    static const int MTU = 1500;
    static const int kBufferNum = 1000;
    using MTUBuffer = FixedBuffer<MTU>;
    using MTUBufferUptr = std::unique_ptr<MTUBuffer>;
    using MTUBufferVector = std::vector<MTUBufferUptr>;
    using PacketUptr =
        std::unique_ptr<MTUBuffer, std::function<void(MTUBuffer*)>>;

public:  // special function
    MTUBufferPool(int list_size = kBufferNum);
    ~MTUBufferPool();
    MTUBufferPool(const MTUBufferPool&) = delete;
    MTUBufferPool& operator=(const MTUBufferPool&) = delete;

public:  // normal function
    PacketUptr PacketBuffer();
    int size() { return static_cast<int>(buffers_.size()); }

private:  // private normal function
    void ViewDelete(MTUBuffer* buffer);

private:
    MTUBufferVector buffers_;
    bool empty_ = true;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_MTU_BUFFER_POOL_H_