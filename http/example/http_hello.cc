#include "http/file_reader.h"
#include "http/http_server.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;

void HttpEntry(const HttpRequest& req, HttpResponse& rsp)
{
    LOG_INFO << req.all_data_;
    LOG_INFO << "method: " << req.method_;
    LOG_INFO << "path: " << req.path_;
    LOG_INFO << "version: " << req.version_;
    for (auto& head : req.headers_) {
        LOG_INFO << "header: {" << head.first << ":" << head.second << "}";
    }
    LOG_INFO << "body: " << req.body_;

    rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
    rsp.AppendHeader("Content-Type", "text/plain; charset=utf-8");
    rsp.AppendHeader("Content-Length", "5");
    rsp.AppendBody("hello");
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    runtime::EventLoop loop;

    HttpServer server(6060, HttpEntry);
    server.Start();

    loop.Start();
}