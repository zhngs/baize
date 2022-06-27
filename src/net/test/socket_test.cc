#include "log/Logger.h"
#include "net/Buffer.h"
#include "net/Socket.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"
#include "net/TcpListener.h"


using namespace baize;
using namespace baize::net;

void echo_server()
{
    TcpListener listener(6060);
    while (1) {
        Buffer buf;
        InetAddress peer_addr;
        int connfd = listener.accept(&peer_addr);
        if (connfd < 0) {
            LOG_SYSERR << "accept failed";
            continue;
        }
        Socket conn_socket(connfd);
        sockets::setNonBlock(connfd, false);
        LOG_INFO << "accept connection " << peer_addr.getIpPort();

        while (1) {
            int err = 0;
            ssize_t rn = buf.readFd(connfd, &err);
            if (rn == 0) break;
            if (rn < 0) {
                LOG_SYSERR << "read failed";
                continue;
            }
            assert(rn == implicit_cast<ssize_t>(buf.readableBytes()));
            ssize_t wn = sockets::write(connfd, buf.peek(), buf.readableBytes());
            assert(rn == wn);
            buf.retrieve(wn);
        }
    }
}

void echo_client()
{
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]\n";
    }
    if (strcmp(argv[1], "-s") == 0) {
        echo_server();
    } else if (strcmp(argv[1], "-c") == 0) {
        echo_client();
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]\n";
    }
}