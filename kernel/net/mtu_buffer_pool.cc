#include "net/mtu_buffer_pool.h"

namespace baize
{

namespace net
{

MTUBufferPool::MTUBufferPool(int list_size)
{
    for (int i = 0; i < list_size; i++) {
        buffers_.emplace_back(std::make_unique<FixedBuffer<MTU>>());
    }
}

MTUBufferPool::~MTUBufferPool() {}

MTUBufferPool::PacketUptr MTUBufferPool::PacketBuffer()
{
    auto del = [this](MTUBuffer* buffer) { ViewDelete(buffer); };
    if (!buffers_.empty()) {
        MTUBufferUptr& buf = buffers_.back();
        MTUBuffer* ptr = buf.release();
        buffers_.pop_back();
        ptr->reset();
        return PacketUptr(ptr, del);
    } else {
        return PacketUptr(nullptr, del);
    }
}

void MTUBufferPool::ViewDelete(MTUBuffer* buffer)
{
    buffer->reset();
    buffers_.emplace_back(MTUBufferUptr(buffer));
}

}  // namespace net

}  // namespace baize
