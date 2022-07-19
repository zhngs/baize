#include "log/Logger.h"
#include "runtime/EventLoop.h"
#include "time/TimerQueue.h"

using namespace baize;
using namespace baize::time;
using namespace baize::runtime;

void test() { LOG_INFO << "test"; }

void test2() { LOG_INFO << "test2"; }

void timer() {
  EventLoop *loop = getCurrentLoop();
  LOG_INFO << "timer start";
  loop->runAfter(2, test2);
  loop->runEvery(1, test);
}

int main() {
  log::Logger::setLogLevel(log::Logger::INFO);
  EventLoop loop;
  loop.addAndExecRoutine(timer);
  loop.start();
}