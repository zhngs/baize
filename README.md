# baize(重构中)

![](https://img.shields.io/github/forks/zhngs/baize)
![](https://img.shields.io/github/stars/zhngs/baize)
![](https://img.shields.io/github/license/zhngs/baize)

## 🌈 简介

**baize 是 modern c++编写的基于协程的轻量级高性能网络库**🔥

- baize 的设计哲学是 `尽可能简单高效，用更少的代码做更多的事`，如果使用 baize 和其他基于回调的网络库作对比，就能体会到 baize 的清爽与简洁 ⭐️
- baize 对协程的选型是经过考虑的，`不使用无栈`的协程，因为无栈的协程是编译器来管理，使用者无法对程序具有更好的掌控力，而有栈的协程可以自己管理上下文切换，并且无栈协程要更新到 c++20 才能使用，标准太新未必是件好事，想要使用无栈协程我推荐 rust 的网络库 `tokio`✨,这是一个非常活跃的项目，有很多人去维护
- baize `不使用对称`的协程，因为对称的协程需要好的调度算法，这无疑增加了复杂性，我认为协程能够做到在 IO 返回 EAGAIN 的时候调度走即可，想要使用 `有栈对称`协程可以去使用 `go`，其语言层面和标准库层面都提供了直接支持
- baize 是 `有栈非对称`的协程模型，并且不支持协程嵌套，因为这会增加非必要的软件复杂度，baize 上下文切换依赖 boost 库中的 context 组件，其稳定性和上下文切换性能都很优异 🏆
- baize 的 runtime 部分借鉴了 go 协程的 epoll 思路，基础设施组件借鉴了 muduo，net 部分基于协程的写法相对于 muduo 大大简化，且在某些场景下吞吐量是 muduo 的 2 倍，测试数据见下方章节

## ✨ 特性

- [x] 有栈非对称协程
- [x] 协程池
- [ ] 协程之间的 channel，正在支持中
- [ ] 协程跨线程，目前不打算支持
- [x] TCP/UDP
- [x] TLS/HTTPS
- [ ] 跨平台，目前只支持 linux

## :magic_wand: 编译

baize 使用 c++14 标准，请使用满足标准的 g++版本，构建系统选择 cmake

在编译之前，请阅读 third_party 下的 [README.md](./third_party/README.md)，确保已经满足 baize 的第三方依赖

```shell
$ cd baize
$ mkdir build
$ cd build
$ cmake ..
$ make
```

编译结束后，可执行文件在 build/bin 目录下，库文件在 build/lib 目录下

## :receipt: 目录

baize 的核心源代码在 kernel 下，分为如下目录：

- log，高性能日志库
- net，网络核心，提供基于协程的异步接口
- runtime，协程调度核心
- process，进程方面的封装，如接管信号，将程序变为守护进程
- thread，线程能力以及同步原语
- time，时间轮功能
- util，实用功能代码

其他目录不是核心代码：

- script，脚本如生成火焰图，格式化代码风格
- third_party，第三方依赖
- example，简单的网络编程示例
- http，简单的 http server
- quic，quic 协议的示例程序
- webrtc，支持中的最小的 webrtc sfu 协议栈

项目代码风格偏向于 google 的 styleguide，但做出了少部分改变，比如缩进采用 4 个空格，所有详细的格式配置在.clang-format 文件里

## 🌰 例子

下述代码是一个简单的 tcp echo 服务

```cpp
#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;

void echo_connection(TcpStreamSptr stream)
{
    Buffer read_buf;
    while (1) {
        int rn = stream->AsyncRead(read_buf);
        LOG_INFO << "read " << rn << " bytes from connection "
                 << stream->peer_ip_port();
        if (rn <= 0) break;

        int wn = stream->AsyncWrite(read_buf.read_index(),
                                    read_buf.readable_bytes());
        LOG_INFO << "write " << wn << " bytes to connection "
                 << stream->peer_ip_port();
        if (wn != read_buf.readable_bytes()) break;

        read_buf.TakeAll();
    }
    LOG_INFO << "connection " << stream->peer_ip_port() << " close";
}

void echo_server()
{
    TcpListener listener(6060);

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
        current_loop()->Do([stream] { echo_connection(stream); });
    }
}

int main()
{
    EventLoop loop;
    loop.Do(echo_server);
    loop.Start();
}
```

## ⏰ 性能

在一台云服务器上执行 tcp_discard 程序，这是一个模仿 muduo 的 netty_discard 的程序，可以看到测试结果为 214MiB/s，在 top 窗口看到两个程序的 cpu 使用率大致都为 70%

```shell
# 窗口a
$ tcp_discard -s
INFO 20220717 13:46:56.245741 [ discard server read speed 214.054 MiB/s, 128791 Msg/s, 1742.81 bytes/msg ] /root/baize/src/runtime/test/tcp_discard.cc:31:server_print -> routine2 -> mainThread:2680311
# 窗口b
$ tcp_discard -c
```

执行 muduo 的 netty_discard 测试代码，测试数据为 105MiB/s，在 top 窗口看到两个程序使用率都为 70%

```shell
# 窗口a
./netty_discard_client 127.0.0.1 1024
# 窗口b
./netty_discard_server
102.015 MiB/s 58.786 Ki Msgs/s 1777.01 bytes per msg
```

`结论`：可以看到两者每条消息的长度一致，但是 baize 收到消息的数量大概是 muduo 的 2 倍，原因其实在于 baize 的协程模型上。baize 使用的是 epoll 的边沿触发，在协程执行中会尽可能地读或写，直到返回 eagain，muduo 的思路是在读之前会先 epoll 一次，然后读一次，相当于多了一次 epoll 的系统调用，吞吐量自然低了一些。muduo 这样的做法目的是兼顾公平性和吞吐量，但如果连接多的时候，muduo 和 baize 的差别就不大了

`思考`：本机测试下，baize 的吞吐量数据非常接近数据传输的极限，muduo 则是因为多了一次系统调用导致吞吐量少了一半。baize 实现公平性的方法是 `模仿操作系统的调度`，在一个协程执行的过程中，维护一个异步调用次数，每使用一次异步接口，该值减一，等减到 0 的时候挂起协程，等到合适的时机再调度回来，这样在省下系统调用的同时也满足了公平性，并且吞吐量得到了提高，这个思路其实类似于操作系统给进程一个时间片，等到时间片用完触发中断切换进程，计算机世界的技术是如此的相似和有意思 😄！

## 💌 开发

baize 仍处于待开发状态，核心的协程和网络部分代码非常轻量，感兴趣的伙伴可以加入 `qq群621642409`讨论编码技术

"东望山有兽，名曰白泽，能言语。王者有德，明照幽远则至。" ————白泽是古代的瑞兽，希望 baize 也能带来祥瑞

## :partying_face: 致谢

感谢陈硕大神能提供 muduo 这样优秀的网络库作为学习榜样

感谢每一个为 baize 网络库 ⭐️star 的开发者，祝 coding 愉快 🥂！
