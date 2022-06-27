#ifndef BAIZE_TCPSTREAM_H
#define BAIZE_TCPSTREAM_H

#include "util/noncopyable.h"

#include <memory>

namespace baize
{

namespace net
{

class TcpStream: noncopyable
{
public:
    TcpStream(int fd);
    ~TcpStream();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
    
} // namespace net
    
} // namespace baize


#endif //BAIZE_TCPSTREAM_H