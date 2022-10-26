#include "net/net_buffer_pool.h"

namespace baize
{

namespace net
{

BufferPool::BufferPool(int size)
{
    for (int i = 0; i < size; i++) {
        buffers_.emplace_back(std::make_unique<Buffer>());
    }
}

BufferPool::~BufferPool() {}

BufferPool::BufferUptr BufferPool::AllocBuffer()
{
    auto del = [this](Buffer* buffer) { BufferDelete(buffer); };
    if (!buffers_.empty()) {
        auto& buf = buffers_.back();
        Buffer* ptr = buf.release();
        buffers_.pop_back();
        ptr->TakeAll();
        return BufferUptr(ptr, del);
    } else {
        return BufferUptr(nullptr, del);
    }
}

void BufferPool::BufferDelete(Buffer* buffer)
{
    buffer->TakeAll();
    buffers_.emplace_back(std::unique_ptr<Buffer>(buffer));
}

}  // namespace net

}  // namespace baize
