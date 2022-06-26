#include "log/Logger.h"
#include "net/Socket.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"

#include <stdio.h>

using namespace baize;
using namespace baize::net;

void echo_server()
{
    InetAddress listen_addr(6060);
    Socket listen_socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    listen_socket.bindAddress(listen_addr);
    listen_socket.listen();

    while (1) {
        char buf[4096];    
        InetAddress peer_addr;
        int connfd = listen_socket.accept(&peer_addr);
        if (connfd < 0) {
            LOG_SYSERR << "accept failed";
            continue;
        }
        LOG_INFO << "accept connection " << peer_addr.getIpPort();
        while (1) {
            ssize_t rn = sockets::read(connfd, buf, sizeof(buf));
            if (rn == 0) break;
            if (rn < 0) {
                LOG_SYSERR << "read failed";
                continue;
            }
            ssize_t wn = sockets::write(connfd, buf, rn);
            assert(rn == wn);
        }
    }
}

void echo_client()
{
    InetAddress server_addr("127.0.0.1", 6060);
    Socket conn_socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    conn_socket.connect(server_addr);
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