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




static void cares_search_hook(void *arg, int status, int timeouts, unsigned char *abuf, int alen)
{
        (void) arg;
        struct hostent *host = NULL;

        log_info("invoking search result callback, status %d timeouts %d.", status, timeouts);
        if (status != ARES_SUCCESS) {
                log_error("fail to send query, %s.", ares_strerror(status));
                return;
        }

        status = ares_parse_a_reply(abuf, alen, &host, NULL, NULL);
        if (status != ARES_SUCCESS) {
                log_error("failed to lookup, %s.", ares_strerror(status));
                return;
        }
        dump_hostent(host);
}
        /* set DNS server instead of /etc/resolv.conf */
        struct ares_addr_node svrs = {
                .next = NULL,
                .family = AF_INET, /* OR AF_INET6 */
                .addr.addr4.s_addr = inet_addr("10.0.32.117"),
                //.addr.addr4.s_addr = inet_addr("10.0.32.118"),
                //.addr.addr4.s_addr = inet_addr("10.0.35.134"), /* valid */
        };
        rc = ares_set_servers(dns.ares.channel, &svrs);
        if (rc != ARES_SUCCESS) {
                log_error("c-ares library init error, %s.", ares_strerror(rc));
                return -1;
        }

#ifdef CARES_GETHOSTADDR
        /* NOTE: this depends on how the DNS server implied. */
        struct in_addr ip;
        inet_aton("8.8.8.8", &ip);
        ares_gethostbyaddr(dns.ares.channel, &ip, sizeof(ip), AF_INET6, cares_gethostaddr_hook, NULL);
#endif

        ares_search(dns.ares.channel, "www.baidu.com", C_IN, T_A, cares_search_hook, NULL);

static void dump_hostent(struct hostent *host)
{
        int i;
        char ipaddr[INET6_ADDRSTRLEN];
        const char *type = "Unknown";

        log_info("====> dump host name, official '%s' address length %dBytes",
                        host->h_name, host->h_length);
        log_info("---- hostname alias:");
        for (i = 0; host->h_aliases[i]; i++)
                log_info("    %s", host->h_aliases[i]);

        if (host->h_addrtype == AF_INET)
                type = "IPv4";
        else if (host->h_addrtype == AF_INET6)
                type = "IPv6";
        log_info("---- %s address list:", type);
        for (i = 0; host->h_addr_list[i]; ++i) {
                inet_ntop(host->h_addrtype, host->h_addr_list[i], ipaddr, sizeof(ipaddr));
                log_info("    IP[%d]: %s", i, ipaddr);
        }
}


A VS. CNAME

##

其中域名的 A(Address) 记录保存的是域名与 IP 对应的记录，一个域名可以对应多个 IP 地址，从而做到负载均衡。

CNAME(Canonical Name) 记录了一个域名和别名对应的记录，那么当 DNS 查询到的主机名对应的是一个 CNAME 类型时，会继续查询其右面的名称再进行查询，一直追踪到最后的 PTR 或 A 名称，成功查询后才会做出回应。

通过 CNAME 记录允许将多个名字映射到同一台计算机。

与 A 记录不同的是，CNAME 别名记录设置的可以是一个域名的描述而不一定是 IP 地址，通常用于同时提供 WWW 和 MAIL 服务的计算机。

URL转发： 如果没有一台独立的服务器（也就是没有一个独立的IP地址）或者还有一个域名 B ，想访问 A 域名时访问到 B 域名的内容，这时就可以通过 URL 转发来实现。

转发的方式有两种：隐性转发和显性转发

隐性转发的时候 www.abc.com 跳转到 www.123.com 的内容页面以后，地址栏的域名并不会改变（仍然显示 www.abc.com ）。网页上的相对链接都会显示 www.abc.com

#include <netdb.h>

struct hostent {
  char  *h_name;      // official name 所谓的规范名，例如www.baidu.com的为www.a.shifen.com
  char **h_aliases;   // 一般来说只有一个，不过为了方便记录也可能会有多个
  int    h_addrtype;  // 主机IP的类型，包括了IPv4(AF_INET) IPv6(AF_INET6)
  int    h_length;    // IP地址的长度，通常是为了方便转换
  char **h_addr_list; // 地址的列表，是一个二维的数组，单个长度通过h_length定义
}
#define h_addr h_addr_list[0]  /* for backward compatibility */

glibc 中提供了一个阻塞类型的 DNS 解析函数，也就是 `gethostbyname(3)` 函数，该函数会返回一个 `struct hostent` 指针，代表了 DNS 的解析结果。

ares_timeout() 用于计算超时时间，一般用于设置像select这类的等待超时时间

测试场景：
1. 向服务端发送请求，服务端会返回报错 (ServFail)；
2. 通过UDP、TCP发送请求，但是服务端没有返回；




异步 DNS

https://lrita.github.io/2017/05/01/c-ares/

域名解析在网络应用中几乎不可避免，而系统本身的 `gethostbyname()` API 是同步的，会严重阻塞程序运行，为了提高 DNS 查询的速度，通常有几种解决方法。

1. 本地 DNS Cache Server ，常见的是 dnsmasq 。
2. 代码中增加 DNS Cache，这个在很多网络应用程序中都很常见，比如 squid 。
3. 异步 DNS 查询，在解析的过程中，不会影响到业务逻辑的正常运行。

对于异步 DNS 解析来说，其中的解决方案包括了：adns [tadns](https://github.com/davidgfnet/tadns)(适合了解原型实现) 。
https://github.com/vstakhov/librdns

GNU adns
https://www.gnu.org/software/adns/
https://github.com/kbandla/adns
https://github.com/wahern/dns
https://github.com/c-ares/c-ares
https://blog.csdn.net/mumumuwudi/article/details/47164531
https://github.com/getdnsapi/getdns
http://wangxuemin.github.io/2015/07/31/c-ares%20%E4%B8%80%E4%B8%AAC%E8%AF%AD%E8%A8%80%E7%9A%84%E5%BC%82%E6%AD%A5DNS%E8%A7%A3%E6%9E%90%E5%BA%93/

-->

## 参考

官方网站 [c-ares.haxx.se](https://c-ares.haxx.se/) 以及相关的文档 [c-ares.haxx.se/docs.html](https://c-ares.haxx.se/docs.html) 。


{% highlight text %}
{% endhighlight %}
