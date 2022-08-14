#include "http/http_tasklet.h"
#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"
#include "util/string_piece.h"

using namespace baize;

const char dummy_response[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "hello";

void http_connection(net::TcpStreamSptr stream)
{
    net::Buffer* buf = stream->read_buffer();
    net::HttpTasklet tasklet;
    while (1) {
        bool timeout = false;
        int rn = stream->AsyncRead(10000, timeout);
        if (timeout) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " timeout";
            break;
        }
        if (rn <= 0) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " read "
                     << rn;
            break;
        }
        LOG_INFO << "http content:\n{\n" << buf->debug_string_piece() << "}";

        int err = tasklet.Parse(buf);
        if (err < 0) {
            LOG_INFO << "http parse failed in connection "
                     << stream->peer_ip_port();
            break;
        } else if (err == 0) {
            continue;
        }

        auto req_vector = tasklet.moved_requests();
        LOG_INFO << "get " << req_vector.size() << " http request from "
                 << stream->peer_ip_port();
        stream->AsyncWrite(dummy_response, sizeof(dummy_response));
    }
    LOG_INFO << "connection " << stream->peer_ip_port() << " close";
}

void http_server()
{
    net::TcpListener listener(6060);
    listener.Start();
    LOG_INFO << "http server start in port 6060";
    while (1) {
        net::TcpStreamSptr stream = listener.AsyncAccept();
        LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
        runtime::current_loop()->Do([stream] { http_connection(stream); });
    }
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    runtime::EventLoop loop;
    loop.Do(http_server);
    loop.Start();
}