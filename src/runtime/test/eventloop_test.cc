#include "runtime/EventLoop.h"

#include "log/Logger.h"
#include "net/TcpListener.h"
#include "net/TcpStream.h"
#include "time/Timestamp.h"

using namespace baize;
using namespace baize::net;
using namespace baize::time;
using namespace baize::runtime;

int64_t g_sendbytes = 0;
int64_t g_sendbytes_last = 0;

int64_t g_readbytes = 0;
int64_t g_readbytes_last = 0;
int64_t g_msg = 0;

Timestamp g_last_time;

void server_print() {
  Timestamp current_time(Timestamp::now());
  double sec = elapsedInSecond(current_time, g_last_time);
  double read_bytes = static_cast<double>(g_readbytes - g_readbytes_last);
  double speed = read_bytes / sec / 1024 / 1024;
  double bytes_msg = read_bytes / static_cast<double>(g_msg);

  LOG_INFO << "discard server read speed " << speed << " MiB/s, " << g_msg
           << " Msg/s, " << bytes_msg << " bytes/msg";

  g_readbytes_last = g_readbytes;
  g_last_time = current_time;
  g_msg = 0;
}

void discard_connection(TcpStreamSptr conn) {
  char buf[65536];
  g_last_time = Timestamp::now();
  TimerId id = getCurrentLoop()->runEvery(1, server_print);
  while (1) {
    int rn = conn->asyncReadOrDie(buf, sizeof(buf));
    if (rn == 0)
      break;
    g_msg++;
    g_readbytes += rn;
  }
  LOG_INFO << "discard_connection finish";
  getCurrentLoop()->cancelTimer(id);
}

void discard_server() {
  TcpListener listener(6070);
  listener.start();

  while (1) {
    TcpStreamSptr stream = listener.asyncAccept();
    stream->setTcpNoDelay();
    getCurrentLoop()->addRoutine([stream] { discard_connection(stream); });
    LOG_INFO << "accept connection " << stream->getPeerIpPort();
  }
}

void client_print() {
  Timestamp current_time(Timestamp::now());
  double sec = elapsedInSecond(current_time, g_last_time);
  double send_bytes = static_cast<double>(g_sendbytes - g_sendbytes_last);
  double speed = send_bytes / sec / 1024 / 1024;

  LOG_INFO << "discard client write speed " << speed << " MiB/s";

  g_sendbytes_last = g_sendbytes;
  g_last_time = current_time;
}

void discard_client() {
  char buf[1024];
  string message(1024, 'z');
  TcpStreamSptr stream = TcpStream::asyncConnect("127.0.0.1", 6070);
  if (!stream)
    return;
  stream->setTcpNoDelay();

  getCurrentLoop()->runEvery(1, client_print);
  g_last_time = Timestamp::now();
  while (1) {
    int wn = stream->asyncWriteOrDie(message.c_str(), message.size());
    g_sendbytes += wn;
  }
  LOG_INFO << "discard_client finish";
  stream->shutdownWrite();
  while (stream->asyncReadOrDie(buf, sizeof(buf)) != 0) {
  }
}

int main(int argc, char *argv[]) {
  log::Logger::setLogLevel(log::Logger::DEBUG);
  EventLoop loop;
  if (argc != 2) {
    LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    return 0;
  }
  if (strcmp(argv[1], "-s") == 0) {
    loop.addAndExecRoutine(discard_server);
  } else if (strcmp(argv[1], "-c") == 0) {
    loop.addAndExecRoutine(discard_client);
  } else {
    LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
  }
  loop.start();
}