#ifndef BAIZE_NONCOPYABLE_H
#define BAIZE_NONCOPYABLE_H
// copy from muduo and make some small changes

namespace baize
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

} //namespace baize

#endif //BAIZE_NONCOPYABLE_H