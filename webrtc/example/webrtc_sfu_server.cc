#include "log/logger.h"
#include "runtime/event_loop.h"
#include "webrtc/pc/webrtc_server.h"

using namespace baize;
using namespace baize::net;

int main(int argc, char* argv[])
{
    set_log_info();
    runtime::EventLoop loop;

    WebRTCServer webrtc_server;
    loop.Do([&webrtc_server] { webrtc_server.StartSignalServer(6060); });
    loop.Do([&webrtc_server] { webrtc_server.StartMediaServer(6061); });

    loop.Start();
}