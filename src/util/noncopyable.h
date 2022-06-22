#ifndef BAIZE_NONCOPYABLE_H
#define BAIZE_NONCOPYABLE_H

namespace baize
{

namespace util
{

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

class copyable
{
protected:
    copyable() = default;
    ~copyable() = default;
};

} //namespace util

} //namespace baize

#endif //BAIZE_NONCOPYABLE_H