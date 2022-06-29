#ifndef BAIZE_ROUTINE_H
#define BAIZE_ROUTINE_H

#include <boost/context/all.hpp>

namespace baize
{
    
namespace net
{

class Routine // noncopyable
{
public:
    using RoutineCb = std::function<void()>;
    Routine(RoutineCb func);
    ~Routine();

    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;
    
    void call();
    static void hangup();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
    
} // namespace net

} // namespace baize


#endif //BAIZE_ROUTINE_H