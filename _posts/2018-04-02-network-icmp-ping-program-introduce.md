---
title: ICMP Ping
layout: post
comments: true
language: chinese
category: [network]
keywords: network,icmp,ping
description:
---

Internet Control Message Protocol, ICMP(RFC-792) 基于 IP 协议，工作在七层协议的第三层，主要用来解析网络路由情况，通过返回错误信息进行分析定位。

<!-- more -->

## 简介

ICMP 报文封装在 IP 报文内部，通过 RAW_SOCK 接收到的报文同时会包含 IP 头部信息，这也就意味着在使用时，需要将 IP 的头先拆掉，然后再获取 ICMP 报文。

### ICMP 报文



<!--
IP 报文头为 20 字节 [IP头部结构详解](http://codingstone.com/content.php?blockTableName=network&blogID=2) 。

ICMP 报文头根据 type 和 code 的不同，其对应的大小也有所区别，详见 [使用Python的Socket模块构建一个UDP扫描工具](http://www.cnnetsec.com/2308.html) 。

关于 ICMP 也可以参考 http://courses.cs.vt.edu/cs4254/fall04/slides/raw_6.pdf

ICMP 报文格式
http://www.cnblogs.com/jingmoxukong/p/3811262.html
-->



Ping 使用的是 ICMP 报文，其报头为 8 字节，数据报长度最大为 64K 字节。

#### Echo/Echo Reply Message

{% highlight text %}
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Identifier          |        Sequence Number        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Data ...
+-+-+-+-+-+-+-
{% endhighlight %}

Ping 是用这一报文发送以及接收，其中 Type(8) 是请求报文，而 Type(0) 是应答报文。

注意，如果需要扫描多个个主机，那么返回的报文中只有 ID 以及 SEQ ，所以，可以将相关的信息添加到 Data 字段中，这个字段是二进制的，可以以 `\0'` 终止。

<!--
GitHub 上不错的Hack技巧
ICMP Tunnel


type of service (ToS


对于通过 SOCK_RAW + ICMP 实现套接字，会接收到所有发送到本机的报文。

1. Destination Host Unreachable

* 校验和算法，把被校验的数据 16 位进行累加，若数据字节长度为奇数，则数据尾部补一个字节的 0 以凑成偶数，然后取反码，。

此算法适用于IPv4、ICMPv4、IGMPV4、ICMPv6、UDP和TCP校验和，更详细的信息请参考RFC1071，校验和字段为上述ICMP数据结构的icmp_cksum变量。
标识符�D�D用于唯一标识ICMP报文, 为上述ICMP数据结构的icmp_id宏所指的变量。
顺序号�D�Dping命令的icmp_seq便由这里读出，代表ICMP报文的发送顺序，为上述ICMP数据结构的icmp_seq宏所指的变量。
-->

## 代码实现

### 非 root 套接字

新版本的内核中提供了低权限的 Socket 访问方式，内核会自动过滤报文，不太适合大批量的监控，这里仅简单介绍下。

ICMP 套接字的目的是允许在不设置 SUID 或者 CAP_NET_RAW 权限的时候允许 ping 程序的使用，详细的实现可以查看内核的邮件列表 [add IPPROTO_ICMP socket kind](https://lkml.org/lkml/2011/5/10/389) 。

是否支持是通过内核的 `net.ipv4.ping_group_range` 指定，这是一对整数，指定了允许使用 ICMP 套接字的组 ID 的范围，默认为 `1 0` 也就意味着没有人能够使用这个特性。

可以通过如下命令修改。

{% highlight text %}
# sysctl -w net.ipv4.ping_group_range='0 10'
{% endhighlight %}

然后可以通过如下方式创建 ICMP 的套接字。

{% highlight text %}
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_ICMP)
{% endhighlight %}

如果系统不支持这个特性，在创建套接字的时候会得到 `Protocol not supported` 的错误，而如果没有权限，则会得到 `Permission denied` 的错误。

它的类型和 UDP 套接字一样，是 `SOCK_DGRAM` 而非 `SOCK_RAW`，这也就意味着你不会收到 20 字节的 IP 头，而且内核会计算校验和，并且填充 ICMP ID，在接收到响应后，内核会只把相应 ID 的 ICMP 响应返回给程序，不需要自己或者要求内核过滤了。

### Socket 选项

可以通过 `setsockopt()` 设置相关的参数，常用的参数如下。

* `SOL_SOCKET` `SO_BINDTODEVICE` 绑定到某个网络设备上，也就是从某个指定的网卡发送数据。
* `SOL_SOCKET` `SO_MARK` 用来添加标记，然后作策略路由之类的动作。
* `SOL_SOCKET` `SO_TIMESTAMP` 让协议栈接受到一个网络帧时为其打上时间戳，并将此时间戳作为一笔附加数据，与网络帧数据一起递交到上层协议。

#### Mark 标记

用于将特定的数据包打上标签，供 `iptables` 配合 `TC` 做 `QOS` 流量限制、应用策略路由等。

系统可用模块，对 CentOS 而言可通过 `ls /usr/lib64/xtables/ | grep -i mark` 查看，其中大写的为标记模块，小写的为匹配模块。

<!--
ls /usr/lib/iptables/|grep -i mark
-->

{% highlight text %}
----- 查看关于Mark标记的帮助信息
# iptables -j MARK --help
# iptables -m mark --help

----- 将所有TCP数据标记1
# iptables -t mangle -A PREROUTING -p tcp -j MARK --set-mark 1

----- 匹配标记1的数据并保存数据包中的MARK到连接中
# iptables -t mangle -A PREROUTING -p tcp -m mark --mark 1 -j CONNMARK --save-mark
{% endhighlight %}

#### 策略路由

标签并不是设置在数据包内容中，而是在内核中数据包的载体上，如果需要在数据包内容中设置标签，可以使用 TOS 规则目标，也就是修改 IP 数据包头的 TOS 值。

{% highlight text %}
----- 将从网络接口tun0进入的、目标端口为5222的TCP数据包设置mark值为1
# iptables -t mangle -A PREROUTING -j MARK --set-mark 1 -i tun0 -p tcp --dport 5222
{% endhighlight %}

接着，根据设置的 mark 值可用来设定策略路由，比如，把 mark 值为 1 的数据包交由网关 192.168.0.1 转发。

{% highlight text %}
----- 1. 确定一张空路由表，这里选定300
# ip route show table 300
----- 2. 在表中添加路由条目
# ip route add default via 192.168.0.1 table 300
----- 3. 查看当前路由规则
# ip rule list
----- 4. 为mark值为1的数据包指定路由表策略
# ip rule add fwmark 0x1 table 300
{% endhighlight %}

通过这种方法，可以使用 iptables 根据匹配规则设置 mark，再由路由模块根据 mark 值进行路由决策，从而实现复杂的策略路由。

#### TIMESTAMP

也就是在内核的协议栈中，在接收到报文之后以及交接给上层协议栈处理之前，添加时间戳。


<!--
此时对端的机器会将当前机器的时间戳添加到 IP 包中。

但是，这依赖于两台机器上的时间戳，如果偏差比较大，那么会导致获取到的延迟有问题，所以尽量使用本地的时间戳。
-->

### 接收

常用于 socket 的发送接收消息，函数声明如下。

{% highlight text %}
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

#include <sys/socket.h>

struct iovec {                   /* Scatter/gather array items */
   void  *iov_base;              /* Starting address */
   size_t iov_len;               /* Number of bytes to transfer */
};

struct msghdr {
   void         *msg_name;       /* optional address */
   socklen_t     msg_namelen;    /* size of address */
   struct iovec *msg_iov;        /* scatter/gather array */
   size_t        msg_iovlen;     /* # elements in msg_iov */
   void         *msg_control;    /* ancillary data, see below */
   size_t        msg_controllen; /* ancillary data buffer len */
   int           msg_flags;      /* flags on received message */
};
{% endhighlight %}

看起来比较复杂，实际上结构成员可以分为四组：

* 套接口地址成员 msg_name 与 msg_namelen；
* IO向量引用 msg_iov 与 msg_iovlen；
* 附属数据缓冲区成员 msg_control 与 msg_controllen；
* 接收信息标记位 msg_flags，详细可以查看 man 手册。

其中 `msg_control` 指向的是一个 `struct cmsghdr` 结构体。

<!--
https://blog.csdn.net/u014209688/article/details/71311973
https://www.cnblogs.com/jimodetiantang/p/9190958.html
https://ivanzz1001.github.io/records/post/linux/2017/11/04/linux-msghdr
-->

### Socket 缓冲区

<!--
Linux 中可以设置 socket 缓冲区的大小，貌似对于 RAW_SOCKET 和 TCP 等都有效，没有从代码上确认。
-->

使用 `RAW_SOCKET` 时，例如发送 ICMP 报文，因为不存在 TCP 中的滑动窗口、限流等机制，在流量过大时极易引起报文在缓冲区的静默丢失。此时可以通过 tcpdump 获取，但是无法通过 `read()`、`recvmsg()` 等接口无法接收到数据。

接收缓冲区的大小可以使用 `setsockopt()` 设置 `SO_RCVBUF` 选项，其默认和最大值可以通过如下命令查看。

{% highlight text %}
$ cat /proc/sys/net/core/rmem_default
$ cat /proc/sys/net/core/rmem_max

$ cat /proc/sys/net/core/wmem_default
$ cat /proc/sys/net/core/wmem_max
{% endhighlight %}

在设置 Socket 的时候，如果期望设置的缓存大于上述的 max ，实际上会得到两倍于 max 值。

<!--
### TCP 缓冲区

其中 TCP 的读写缓存可以通过如下方式查看，因为可以通过协议自身可以进行限流，那么由于缓存不足导致丢包的概率极小，只是效率问题：

$ cat /proc/sys/net/ipv4/tcp_rmem
4096	87380	   6291456
4*1024  85*1024    6*1024*1024

$ cat /proc/sys/net/ipv4/tcp_wmem
4096	16384	   4194304
4*1024  16*1024    4*1024*1024

$ cat /proc/sys/net/ipv4/tcp_mem
88371	117831	176742

其中对应了三个数值，分别为最小、默认、最大值。

在大多数的 Linux 中 rmem_max 和 wmem_max 被分配的值为 128 k，在一个低延迟的网络环境中，一般是足够用的，对于负载和延迟较高的网络，就需要调整内存使用方法。

详细可以参考
https://segmentfault.com/a/1190000000473365
https://my.oschina.net/guol/blog/115837

https://jvns.ca/blog/2016/08/24/find-out-where-youre-dropping-packets/
https://jvns.ca/blog/2017/09/05/finding-out-where-packets-are-being-dropped/

http://veithen.github.io/2014/01/01/how-tcp-backlog-works-in-linux.html

sysctl net.core.rmem_max

sysctl net.core.rmem_max=16777216
sysctl net.core.rmem_default=16777216
sysctl net.core.wmem_max=8388608
sysctl net.core.wmem_default=8388608

echo 'net.core.wmem_max=8388608' >> /etc/sysctl.conf
echo 'net.core.rmem_max=16777216' >> /etc/sysctl.conf

int rc, optval;
int optlen = sizeof(optval);

rc = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
printf("xxxxxxxxxxxx %d %d\n", rc, optval);

int sbuf = 1024 * 1024 * 8;
err = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

echo 8388608 > /proc/sys/net/core/rmem_default
-->

### 其它

ICMP 报文在 Linux C 中通过 `struct icmp[netinet/ip_icmp.h]` 定义，而报文中的消息体，则根据不同 `TYPE`、`CODE` 会有所区别。

在 C 中，通过 `union` 进行适配，为了方便使用，同时又定义了很多辅助的宏定义。

## 其它

### 抓包

可以通过如下命令获取包。

{% highlight text %}
----- ICMP echo request
$ tcpdump -nni eth0 -e icmp[icmptype] == 8

----- ICMP echo reply
$ tcpdump -nni eth0 -e icmp[icmptype] == 0

----- 保存到文件中
$ tcpdump -nni eth0 -w /tmp/icmp.pcap -e icmp[icmptype] == 0

----- 读取文件
$ tcpdump -nn -r icmp.pcap
{% endhighlight %}

### TTL

Time To Live, TTL 存活时间，表示这个 PING 数据包能在网络上存活多少时间，实际上就是用来表示可以被路由转发多少次，在 IP 头中通过 8bits 表示。

执行 Ping 操作时，会在本机发送一个数据包，正常来说，数据包会经过一定的路由到达目的主机。不过，由于很多原因可能会导致一些数据包不能正常传送到目的主句，为了防止由于数据包一直传输导致网络负载增加，增加的 TTL 作为包的存活时间。

当数据包传送到一个路由器之后，TTL 就自动减 1，如果减到 0 了还是没有传送到目的主机，那么就自动丢弃，此时一般是 Request timed out 了。

{% highlight text %}
64 bytes from 192.168.9.1: icmp_seq=1 ttl=60 time=0.909 ms
{% endhighlight %}

目前测试来看，当 ICMP 报文发送之后，其返回的报文实际上会重新设置(依赖目标机器的设置)，也即是说 TTL 的值是单程的。

在 Linux 命令行中，可以通过 `-t` 参数进行设置。例如，返回的 TTL 为 60，那么你可以初步猜测对端设置的初始 TTL 为 64，那么通过 `-t 2` 设置时候，就会出现 `Time to live exceeded` 的报错。

#### 系统设置

Windows 一般在 `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters` 注册表中设置。

在 Linux 中，可以通过如下方式修改默认的 TTL 。

{% highlight text %}
----- 临时修改
# sysctl net.ipv4.ip_default_ttl=129
# echo 129 > /proc/sys/net/ipv4/ip_default_ttl
----- 保存到配置文件中
$ cat /etc/sysctl.conf
net.ipv4.ip_default_ttl=129
{% endhighlight %}

各个系统的默认值如下。

{% highlight text %}
Linux              64/255
Windows NT/2000/XP 128
Windows 98         32
UNIX               255
{% endhighlight %}

### QoS

Quality of Service, QoS 服务质量，提供的质量越好，表示有越低的延迟、丢包、抖动等，同时其吞吐量和可靠性要更高。简单来说，就是利用包的特定标志位，告诉路由器如何处理包，是先还是后。

在第二层(Link Layer)和第三层(IP)都有标示位，其中 IP 层采用的是 8Bits，包括 IPv4 和 IPv6 都有，只是其位置不同。

## 参考

可以参考 [Github liboping](https://github.com/octo/liboping) 。

关于各个操作系统的默认 TTL 可以参考 [Default TTL (Time To Live) Values of Different OS](https://subinsb.com/default-device-ttl-values/) 。

<!--
http://gienmin.blogspot.com/2013/12/qos_18.html
http://www.map.meteoswiss.ch/map-doc/ftp-probleme.htm
-->

{% highlight text %}
{% endhighlight %}
