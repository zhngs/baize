# baize

## 1.简介

`baize是一个使用modern c++编写的轻量级的基于协程的网络库`:

- 协程的实现依赖有栈协程 boost-context，稳定性和协程切换性能都很优秀
- baize 的 runtime 部分借鉴了 go 协程的 epoll 思路，基础设施组件借鉴了 muduo，net 部分基于协程的写法相对于 muduo 大大简化，且在某些场景下吞吐量是 muduo 的 2 倍，测试数据见下方章节

## 2.baize 的定位

baize 只是我业余的作品，目前不具备工业强度，初衷是在阅读完 muduo 和 libco 源码后，想要将 muduo 协程化

- baize 适合想要了解协程并且刚学习完 muduo 的新手，协程相比于传统的基于回调的事件循环，写法上大大简化
- baize 对于简单的网络编程实验完全可以胜任，例如我在 quic 文件夹中封装了 cloudflare/quiche 的简单用法
- 我对 baize 的希冀是可以孵化出一些工业级的代码

## 3.编译

构建系统选择 cmake，编译方法如下，但要提前安装 boost-context 依赖，版本为 1.66 及以上

```shell
$ cd baize
$ mkdir build
$ cd build
$ cmake ..
$ make
```

编译结束后，可执行文件在 build/bin 目录下，库文件在 build/lib 目录下

## 4.目录介绍

baize 源代码在 src 目录下，quic 目录是我针对 quic 协议编写的实验性质的代码

目前 baize 核心代码分为如下目录：

- log，日志库
- net，网络部分核心，提供基于协程的异步接口
- runtime，epoll 和协程调度核心
- thread，简单封装了 thread，mutex，cond，waitgroup
- time，时间戳和 timerfd 功能
- util，简单的工具

## 5.例子

下述代码是一个简单的 discrad 服务

```cpp
void discard_connection(TcpStreamSptr conn)
{
    char buf[65536];
    while (1) {
        // 异步read
        int rn = conn->asyncReadOrDie(buf, sizeof(buf));
        if (rn == 0) break;
    }
    LOG_INFO << "discard_connection finish";
}

void discard_server()
{
    TcpListener listener(6070);
    listener.start();

    while (1) {
        // 异步accept
        TcpStreamSptr stream = listener.asyncAccept();
        stream->setTcpNoDelay();
        getCurrentLoop()->addRoutine([stream]{ discard_connection(stream); });
        LOG_INFO << "accept connection " << stream->getPeerIpPort();
    }
}

int main(int argc, char* argv[])
{
    EventLoop loop;
    // 添加协程
    loop.addAndExecRoutine(discard_server);
    loop.start();
}
```

## 6.测试

- 在我的云服务器上执行 eventloop_test 程序，这是一个模仿 muduo 的 netty_discard 的程序，可以看到测试结果为 214MiB/s，在 top 窗口看到两个程序的 cpu 使用率大致都为 70%

```shell
# 窗口a
$ eventloop_test -s
INFO 20220717 13:46:56.245741 [ discard server read speed 214.054 MiB/s, 128791 Msg/s, 1742.81 bytes/msg ] /root/baize/src/runtime/test/eventloop_test.cc:31:server_print -> routine2 -> mainThread:2680311
# 窗口b
$ eventloop_test -c
```

- 执行 muduo 的 netty_discard 测试代码，测试数据为 105MiB/s，在 top 窗口看到两个程序使用率都为 70%

```shell
# 窗口a
./netty_discard_client 127.0.0.1 1024
# 窗口b
./netty_discard_server
102.015 MiB/s 58.786 Ki Msgs/s 1777.01 bytes per msg
```

- 结论：可以看到两者每条消息的长度一致，但是 baize 收到消息的数量大概是 muduo 的 2 倍，原因其实在于 baize 的协程模型上。baize 使用的是 epoll 的边沿触发，在协程执行中会尽可能地读或写，直到返回 eagain，muduo 的思路是在读之前会先 epoll 一次，然后读一次，相当于多了一次 epoll 的系统调用，吞吐量自然低了一些。muduo 这样的做法目的是兼顾公平性和吞吐量，baize 实现公平性的方法如下

- 思考：本机测试下，baize 的吞吐量数据非常接近数据传输的极限，muduo 则是因为多了一次系统调用导致吞吐量少了一半。baize 想要做到公平性，解决方法也不难，模仿操作系统的调度就可以了，思路是在一个协程执行的过程中，维护一个异步调用次数，每使用一次异步接口，该值减一，等减到 0 的时候挂起协程，等到合适的时机再调度回来，这样在省下系统调用的同时也满足了公平性，并且吞吐量得到了提高，这个思路其实类似于操作系统给进程一个时间片，等到时间片用完触发中断切换进程，计算机世界的技术是如此的相似和有意思！

## 7.后续开发

- baize 仍处于待开发状态，大多数代码是借鉴的 muduo 并少部分修改，我写的核心的协程和网络部分大概只有 1000 行代码。作者在鹅厂工作平时较忙，所以目前没有一个明确的开发路线，我希望更多的有志之士可以加入进来完善 baize，具体可以加入`qq群542350316`一起讨论编码技术，完善 baize 开发路线
- "东望山有兽，名曰白泽，能言语。王者有德，明照幽远则至。" ————白泽是古代的瑞兽，希望 baize 也能带来祥瑞

## 8.代码风格

- 代码风格偏向于 google 的 styleguide，但做出了少部分改变，比如缩进采用 4 个空格，所有详细的格式配置在.clang-format 文件里
- 代码格式化工具使用 clang-format，linux 内核也使用该工具，format.sh 是格式化所有后缀为 h 或 cc 文件的脚本，使用方法如下

```shell
$ cd baize
$ ./format
```

## 9.感谢

感谢陈硕大神能提供 muduo 这样优秀的网络库作为学习榜样，感谢每一个为 baize 网络库 star 的开发者，祝 coding 愉快！
