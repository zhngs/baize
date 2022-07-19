#include "thread/WaitGroup.h"

#include "log/Logger.h"
#include "thread/Thread.h"

using namespace baize;
using namespace baize::thread;

int main() {
  WaitGroup wg(3);
  Thread thread1(
      [&] {
        int i = 0;
        LOG_INFO << i++;
        wg.done();
        LOG_INFO << i++;
        wg.done();
        LOG_INFO << i++;
        wg.done();
      },
      "WaitGroupThread1");
  thread1.start();
  LOG_INFO << "before wait";
  wg.wait();
  LOG_INFO << "after wait";
  thread1.join();
}