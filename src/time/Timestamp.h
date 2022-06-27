#ifndef BAIZE_TIMESTAMP_H
#define BAIZE_TIMESTAMP_H
// copy from muduo and make some small changes

#include "util/noncopyable.h"
#include "util/types.h"


namespace baize
{
    
namespace time
{

class Timestamp: public copyable
{
public:
    Timestamp(): us_(0) {}
    Timestamp(int64_t us): us_(us) {}

    static Timestamp now();

    string toFormatString(); 

    bool valid() { return us_ > 0; }

    static const int kusPerSec = 1000000;
private:
    int64_t us_;
};

} // namespace time

} // namespace baize


#endif //BAIZE_TIMESTAMP_H