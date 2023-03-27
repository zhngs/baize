# baize 依赖

## 1. 前言

c++的依赖管理我推荐使用自行编译源码的方式进行链接，baize 在顶层的 CMakeLists.txt 中设置了所依赖的第三方库的头文件路径和库文件路径，只需要将所需要的依赖库放在 third_party 目录编译后，即可进行全局构建

## 2.boost-context(必需)

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

## 3.quiche(非必需)

baize 支持 quic 协议，可以通过顶层的 CMakeLists.txt 打开 quic 的编译选项，该选项默认关闭，想要支持 quic 需要依赖第三方库 quiche

quiche 托管在 github 上，下载和编译的命令如下

```shell
$ cd third_party
$ git clone --recursive https://github.com/cloudflare/quiche
$ cd quiche
$ cargo build --release --features ffi
```

## 4.openssl(非必需)

如果不想编译加解密的部分，可以注释 kernel 目录下 CMakeLists.txt 文件中 crypto 的编译选项

baize 支持加解密，需要使用 openssl，版本为 1.1.1，下载地址在https://www.openssl.org/source/old/1.1.1/

下载结束后执行以下命令

```shell
$ tar -xvf openssl-1.1.1s.tar.gz
$ cd openssl-1.1.1s
$ ./config \
--prefix={baize的绝对路径}/third_party/openssl \
--openssldir={baize的绝对路径}/third_party/openssl \
no-shared \
-DOPENSSL_TLS_SECURITY_LEVEL=2 \
enable-ec_nistp_64_gcc_128
$ make
$ make install
```

`make install`会将 openssl 编译出来的文件安装到 third_party/openssl 目录下，后面链接此目录下的库文件

## 5.libsrtp(非必需)

baize 支持 webrtc 协议栈，需要使用 libsrtp，github 地址为 https://github.com/cisco/libsrtp

写流媒体服务程序这个库是很常用的，baize 目前使用的版本为 2.4.2，下载的时候注意 release 版本，地址为 https://github.com/cisco/libsrtp/archive/refs/tags/v2.4.2.tar.gz

下载结束后执行以下命令

```shell
$ tar -xvf libsrtp-2.4.2.tar.gz
$ cd libsrtp-2.4.2
$ ./configure --enable-openssl --with-openssl-dir={编译出来的openssl路径}
$ make
```
