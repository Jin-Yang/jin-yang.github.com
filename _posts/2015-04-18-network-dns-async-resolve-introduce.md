---
title: cares 异步 DNS 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: cares,dns,async
description: c-ares 是一个 C89 实现的异步请求 DNS 的实现，一些常见的软件 (如 curl、NodeJS、WireShark 等等) 都使用了这一软件库。 这里简单介绍其使用方法。
---

c-ares 是一个 C89 实现的异步请求 DNS 的实现，一些常见的软件 (如 curl、NodeJS、WireShark 等等) 都使用了这一软件库。

这里简单介绍其使用方法。

<!-- more -->

## 简介

在 CentOS 中直接通过 `yum install c-ares-devel` 安装即可。

{% highlight text %}
$ ./buildconf
$ ./configure
{% endhighlight %}

此时将在 `.libs` 目录下生成相应的动态库和静态库。

### 通用函数

一些常用的函数。

{% highlight text %}
ares_library_init()
用于初始化c-ares库，实际只有在Windows平台上会执行初始化操作；

ares_library_cleanup()
与上述的init()对应，用于清理c-ares的资源；
{% endhighlight %}

### Channel

其中 c-ares 会将 `ares_channel` 作为整个异步请求的上下文 (context)，所以在发起一个请求时，应该先初始化 `ares_channel` 。

在使用时，需要通过 `ares_init()` 或者 `ares_init_options()` 函数进行初始化，其中前者使用的是默认参数，后者可以根据需要进行自定义，可以通过如下方式进行修改：

{% highlight text %}
int ares_init_options(ares_channel *channelptr, struct ares_options *options, int optmask);
{% endhighlight %}

`struct ares_options *options` 定义了那些可以修改的参数，而 `int optmask` 用于指定使用前面结构体中的那些字段，可以在单次调用中通过位与设置多个参数。

在 CentOS 中定义在 `/usr/include/ares.h` 头文件中，摘抄如下。

{% highlight c %}
#define ARES_OPT_FLAGS          (1 << 0)
#define ARES_OPT_TIMEOUT        (1 << 1)
#define ARES_OPT_TRIES          (1 << 2)
#define ARES_OPT_NDOTS          (1 << 3)
#define ARES_OPT_UDP_PORT       (1 << 4)
#define ARES_OPT_TCP_PORT       (1 << 5)
#define ARES_OPT_SERVERS        (1 << 6)
#define ARES_OPT_DOMAINS        (1 << 7)
#define ARES_OPT_LOOKUPS        (1 << 8)
#define ARES_OPT_SOCK_STATE_CB  (1 << 9)
#define ARES_OPT_SORTLIST       (1 << 10)
#define ARES_OPT_SOCK_SNDBUF    (1 << 11)
#define ARES_OPT_SOCK_RCVBUF    (1 << 12)
#define ARES_OPT_TIMEOUTMS      (1 << 13)
#define ARES_OPT_ROTATE         (1 << 14)
#define ARES_OPT_EDNSPSZ        (1 << 15)

struct ares_options {
  int flags;
  int timeout;    // 设置第一次查询的超时时间，默认是5秒
  int tries;      // 如果失败重试次数
  int ndots;
  unsigned short udp_port;
  unsigned short tcp_port;
  int socket_send_buffer_size;
  int socket_receive_buffer_size;
  struct in_addr *servers; // 设置DNS服务器的地址，可通过ares_init_options()或ares_set_servers()配置
  int nservers;            // 服务器的地址数
  char **domains;          // 如果提交的主机名小于ndots时，会尝试添加domain后进行搜索
  int ndomains;
  char *lookups;           // 用字符串表示查询的方式，包括了
  ares_sock_state_cb sock_state_cb; // socket状态变化时的回调函数
  void *sock_state_cb_data;         // 调用删除回调时的入参私有变量
  struct apattern *sortlist;
  int nsort;
  int ednspsz;
};
{% endhighlight %}

### Process

c-ares 将一些常用的接口进行了封装，但是本身并不会通过数据进行驱动，需要由用户在不同的阶段进行调用，从而可以适配不同用户的不同需求。

{% highlight text %}
int ares_fds(ares_channel channel, fd_set *read_fds, fd_set *write_fds);
int ares_getsock(ares_channel channel, ares_socket_t *socks, int numsocks);
struct timeval *ares_timeout(ares_channel channel, struct timeval *maxtv, struct timeval *tv);
void ares_process(ares_channel channel, fd_set *read_fds, fd_set *write_fds);
void ares_process_fd(ares_channel channel, ares_socket_t read_fd, ares_socket_t write_fd);
{% endhighlight %}

如果失败，默认第一次会在 5s 后重试，总共重试 4 次，可以通过 `ares_init_options()` 进行设置。

## 源码解析

源码其实比较简单，无非就是如何发送请求报文，然后接收报文，并根据当前的处理状态进行相应的处理。

<!--

libev最新版本
https://github.com/kindy/libev


## FAQ

### 状态修改

当 socket 的状态发生变化时会直接调用 `SOCK_STATE_CALLBACK()` 宏。

## TODO

如果在 `/etc/resolv.conf` 中配置的有非法的地址，也就是该地址一直没有返回数据，那么可能会导致失败，此时会尝试下一个地址。

当服务端没有响应时如何进行处理。
所有的 DNS 相关查询都可以通过 `ares_search()` 搜索，
ares_init_options()
 |-init_by_options() 通过入参方式指定
 |-init_by_environment()
 |-init_by_resolv_conf()
 |-init_by_defaults() 如果仍有部分没有设置，则会在这里配置
init_by_resolv_conf(0
set_search()

ares_gethostbyname()
 |-next_lookup() 开始查找，会根据lookups决定是本地还是DNS查询
   |-ares_search() DNS查询，可以是TCP或者UDP包，大部分功能可以直接调用该接口实现
     |-ares__is_onion_domain() 根据RFC 7686规范，对于onion网络是需要解析的
     |-single_domain() 判断传入参数最后是否为'.'
   |-file_lookup() 直接查询文件中的映射

ares_query() 这是真正单个查找的接口，其它的接口实际上是封装部分处理逻辑
 |-ares_create_query()
 |-ares_send()
   |-ares__send_query()
     |-open_udp_socket()

ares_process_fd()
 |-processfds()
   |-read_udp_packets()
   | |-socket_recvfrom()
   | | |-recvfrom()
   | |-process_answer() 处理接收到的报文，核心的如id、rcode
   |   |-ares__send_query() 发送数据，分为了TCP UDP
   |     |-open_udp_socket()
   |     |-socket_write()
   |-process_broken_connections()

ares_destroy() 清理所有与channel相关的资源
 |-ares__destroy_servers_state() 清理所有与Server相关的资源
   |-ares__close_sockets() 关闭socket，此时会调用设置的状态回调函数

https://github.com/socketry/async-dns
https://github.com/CesiumComputer/sldr
https://github.com/ibc/em-udns
https://github.com/Mons/libevares
https://github.com/wahern/dns



https://lrita.github.io/2017/05/01/c-ares/

DNS协议简单实用，一个不错的入门C实例
https://payloads.online/archivers/2018-02-10/1

## 抓包分析

tcpdump -i eth0 -nn -X port 53


DNS 解析的服务端口是 53 ，也可以使用 `domain` 代替。


DNS常见的攻击、异常操作
https://www.zybuluo.com/xunuo/note/1075101
直接通过Socket发送接收请求的示例
https://payloads.online/archivers/2018-02-10/1
-->

## 参考

官方网站 [c-ares.haxx.se](https://c-ares.haxx.se/) 以及相关的文档 [c-ares.haxx.se/docs.html](https://c-ares.haxx.se/docs.html) 。


{% highlight text %}
{% endhighlight %}
