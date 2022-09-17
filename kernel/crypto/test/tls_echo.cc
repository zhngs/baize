#include "crypto/tls_stream.h"
#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"
#include "unistd.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;

void tls_echo_connection(TcpStreamSptr stream, SslConfig& config)
{
    TlsStreamSptr tls_stream = TlsStream::AsServer(config, std::move(stream));
    if (!tls_stream) {
        LOG_INFO << "tls accepted failed";
        return;
    }

    char buf[1024];
    while (1) {
        int rn = tls_stream->AsyncRead(buf, sizeof(buf));
        if (rn <= 0) {
            LOG_ERROR << "tls stream read failed";
            break;
        }
        LOG_INFO << "tls stream recv data: " << StringPiece(buf, rn);

        int wn = tls_stream->AsyncWrite(buf, rn);
        if (wn != rn) {
            LOG_ERROR << "tls stream write failed";
            break;
        }
    }
}

void tls_echo_server()
{
    TcpListener listener(6060);

    SslConfig config;
    config.set_tls_server("./cert.crt", "./cert.key");

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
        current_loop()->Do(
            [stream, &config] { tls_echo_connection(stream, config); });
    }
}

void tls_echo_client()
{
    TcpStreamSptr stream = TcpStream::AsyncConnect("127.0.0.1", 6060);
    if (!stream) {
        LOG_ERROR << "connect failed";
        return;
    }

    SslConfig config;
    config.set_tls_client();
    TlsStreamSptr tls_stream = TlsStream::AsClient(config, std::move(stream));
    if (!tls_stream) {
        LOG_INFO << "tls connected failed";
        return;
    }

    int wn = tls_stream->AsyncWrite("hello world", 11);
    if (wn != 11) {
        LOG_ERROR << "tls stream write failed";
        return;
    }

    char buf[1024];
    int rn = tls_stream->AsyncRead(buf, sizeof(buf));
    if (rn <= 0) {
        LOG_ERROR << "tls stream read failed";
        return;
    }
    LOG_INFO << "server reply: " << StringPiece(buf, rn);
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop(10);
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
        return 0;
    }
    if (strcmp(argv[1], "-s") == 0) {
        loop.Do(tls_echo_server);
    } else if (strcmp(argv[1], "-c") == 0) {
        loop.Do(tls_echo_client);
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    }
    loop.Start();
}