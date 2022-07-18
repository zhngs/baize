# baize
## 1.简介
`baize是一个使用modern c++编写的轻量级的基于协程的网络库`:
- 协程的实现依赖有栈协程boost-context，稳定性和协程切换性能都很优秀
- baize的runtime部分借鉴了go协程的epoll思路，基础设施组件借鉴了muduo，net部分基于协程的写法相对于muduo大大简化，且在某些场景下吞吐量是muduo的2倍，测试数据见下方章节

## 2.baize的定位
baize只是我业余的作品，目前不具备工业强度，初衷是在阅读完muduo和libco源码后，想要将muduo协程化
- baize适合想要了解协程并且刚学习完muduo的新手，协程相比于传统的基于回调的事件循环，写法上大大简化
- baize对于简单的网络编程实验完全可以胜任，例如我在quic文件夹中封装了cloudflare/quiche的简单用法
- 我对baize的希冀是可以孵化出一些工业级的代码

## 3.编译
构建系统选择cmake，编译方法如下，但要提前安装boost-context依赖，版本为1.66及以上
```shell
$ cd baize
$ mkdir build
$ cd build
$ cmake ..
$ make
```
编译结束后，可执行文件在build/bin目录下，库文件在build/lib目录下

## 4.目录介绍
baize源代码在src目录下，quic目录是我针对quic协议编写的实验性质的代码

目前baize核心代码分为如下目录：
- log，日志库
- net，网络部分核心，提供基于协程的异步接口
- runtime，epoll和协程调度核心
- thread，简单封装了thread，mutex，cond，waitgroup
- time，时间戳和timerfd功能
- util，简单的工具

## 5.例子
下述代码是一个简单的discrad服务
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
- 在我的云服务器上执行eventloop_test程序，这是一个模仿muduo的netty_discard的程序，可以看到测试结果为214MiB/s，在top窗口看到两个程序的cpu使用率大致都为70%
```shell
# 窗口a
$ eventloop_test -s
INFO 20220717 13:46:56.245741 [ discard server read speed 214.054 MiB/s, 128791 Msg/s, 1742.81 bytes/msg ] /root/baize/src/runtime/test/eventloop_test.cc:31:server_print -> routine2 -> mainThread:2680311 
# 窗口b
$ eventloop_test -c
```
- 执行muduo的netty_discard测试代码，测试数据为105MiB/s，在top窗口看到两个程序使用率都为70%
```shell
# 窗口a
./netty_discard_client 127.0.0.1 1024
# 窗口b
./netty_discard_server
102.015 MiB/s 58.786 Ki Msgs/s 1777.01 bytes per msg
```
- 结论：可以看到两者每条消息的长度一致，但是baize收到消息的数量大概是muduo的2倍，原因其实在于baize的协程模型上，会尽可能地读或写，直到返回eagain，muduo的思路是在读之前会先epoll一次，然后读一次，相当于多了一次epoll的系统调用，吞吐量自然低了一些。muduo这样的做法可以兼顾公平性和吞吐量，baize则是失去了公平性，假如一个协程一直写socket(例如udp)，并且不返回eagain，那么这个线程上其他的事件(例如定时器)都不会得到响应。

- 思考：本机测试下，baize的吞吐量数据非常接近数据传输的极限，muduo则是因为多了一次系统调用导致吞吐量少了一半，我认为muduo的做法是合理的。baize想要做到公平性，解决方法也不难，模仿操作系统的调度就可以了，思路是在一个协程执行的过程中，维护一个异步调用次数，每使用一次异步接口，该值减一，等减到0的时候挂起协程，等到合适的时机再调度回来，这样在省下系统调用的同时也满足了公平性，这个思路其实类似于操作系统给进程一个时间片，等到时间片用完触发中断切换进程，计算机世界的技术是如此的相似和有意思！

## 7.后续开发
- baize仍处于待开发状态，大多数代码是借鉴的muduo并少部分修改，我写的核心的协程和网络部分大概只有1000行代码。作者在鹅厂工作平时较忙，所以目前没有一个明确的开发路线，我希望更多的有志之士可以加入进来完善baize，具体可以加入`qq群542350316`一起讨论编码技术，完善baize开发路线
- "东望山有兽，名曰白泽，能言语。王者有德，明照幽远则至。"，白泽是古代的瑞兽，希望baize也能带来祥瑞

## 8.感谢

感谢陈硕大神能提供muduo这样优秀的网络库作为学习榜样，感谢每一个为baize网络库star的开发者，祝coding愉快！