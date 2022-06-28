#include "log/Logger.h"
#include "net/Buffer.h"
#include "net/TcpListener.h"
#include "net/TcpStream.h"


using namespace baize;
using namespace baize::net;

void echo_server()
{
    TcpListener listener(6060);
    while (1) {
        Buffer buf;
        TcpStreamSptr stream = listener.accept();
        if (!stream) {
            continue;
        }
        LOG_INFO << "accept connection " << stream->getPeerIpPort();

        while (1) {
            int err = 0;
            ssize_t rn = buf.readFd(stream->getSockfd(), &err);
            if (rn == 0) break;
            if (rn < 0) {
                LOG_SYSERR << "read failed";
                continue;
            }
            assert(rn == static_cast<ssize_t>(buf.readableBytes()));
            ssize_t wn = stream->write(buf.peek(), buf.readableBytes());
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