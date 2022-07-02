#ifndef BAIZE_NETTYPE_H
#define BAIZE_NETTYPE_H

#include <memory>

namespace baize
{

namespace net
{
    
class TcpStream;
typedef std::shared_ptr<TcpStream> TcpStreamSptr;

} // namespace net
    
} // namespace baize


#endif //BAIZE_NETTYPE_H