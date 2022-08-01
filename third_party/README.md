# baize 依赖

## 1. 前言

c++的依赖管理我推荐使用自行编译源码的方式进行链接，baize 在顶层的 CMakeLists.txt 中设置了所依赖的第三方库的头文件路径和库文件路径，只需要将所需要的依赖库放在 third_party 目录编译后，即可进行全局构建

## 2.boost-context

baize 依赖 boost-context1.66 版本，这里推荐在 third_party 中下载 boost 源码

下载地址：https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/boost_1_66_0.tar.gz

将 boost_1_66_0.tar.gz 下载到 third_party 目录后，执行如下命令

```shell
$ tar -xvf boost_1_66_0.tar.gz
$ cd boost_1_66_0
$ ./bootstrap.sh
# 这里只编译boost-context
$ ./b2 --with-context variant=release
```

## 3.quiche

baize 支持 quic 协议，可以通过顶层的 CMakeLists.txt 打开 quic 的编译选项，该选项默认关闭，想要支持 quic 需要依赖第三方库 quiche

quiche 托管在 github 上，下载和编译的命令如下

```shell
$ cd third_party
$ git clone --recursive https://github.com/cloudflare/quiche
$ cd quiche
$ cargo build --release --features ffi
```
