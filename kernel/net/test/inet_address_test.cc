#include "net/inet_address.h"

#include "log/logger.h"

using namespace baize;
using namespace baize::net;

int main()
{
    InetAddress addr;
    LOG_INFO << "InetAddress len:" << sizeof(addr);
    sockaddr_in addr4;
    LOG_INFO << "sockaddr_in len:" << sizeof(addr4);
    sockaddr_in6 addr6;
    LOG_INFO << "sockaddr_in6 len:" << sizeof(addr6);
}