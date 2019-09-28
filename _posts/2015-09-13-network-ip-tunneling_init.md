---
Date: Auguest 05, 2015
title: Linux IP 隧道技术
layout: post
comments: true
language: chinese
category: [linux, network]
---

隧道是一种封装技术，它利用一种网络协议来传输另一种网络协议，即利用一种网络传输协议，将其它协议产生的数据报文封装在它自己的报文中，然后在网络中传输。

隧道可以看做是一条虚拟的点对点连接，隧道的两端需要对数据报文进行封装及解封装，常用的是基于 IP 的隧道。

在此简单介绍下隧道技术。

<!-- more -->

# 隧道技术

隧道技术就是指包括数据封装、传输和解封装在内的全过程。

![tunnel tecnology]({{ site.url }}/images/network/tunnel-arch.jpg){: .pull-center}

Linux 内核支持的 tunnel 有 ipip、gre、sit，另外，还包括了其它非内核隧道的几种。其中 ipip 是最简单的一种隧道；gre 是对 ipip 的一种改进；sit 的作用是连接 ipv4 与 ipv6 的网络。另外，以上所有隧道都需要内核模块 tunnel4.ko 的支持。

在内核之外，还有很多实现隧道的方法，最有名的是 PPP 和 PPTP。

# IP-in-IP

ipip 需要内核模块 ipip.ko，可以通过 modinfo ipip 查看该内核模块的相关信息。

该模式的特点是：A) 只是连接了两个一般情况下无法直接通讯的 IPv4 网络而已，简单有效；B) 不能通过 IPIP 隧道转发广播或者 IPv6 数据包。

{% highlight text %}
                                         INTERNET
| 192.168.100.0/24 | 211.167.237.218 |<------------>| 123.127.177.195 |192.168.200.0/24 |
       eth0                eth1                            eth1              eth0
{% endhighlight %}

网络结构如上，现在就依据上面的结构来建立 tunnel 。

{% highlight text %}
----- 在211.167.237.218上
# modprobe ipip
# ip tunnel add tun1 mode ipip remote 123.127.177.195 local 211.167.237.218 ttl 64
# ip link set tun1 mtu 1480 up
# ip address add 192.168.200.253 brd 255.255.255.255 peer 123.127.177.195 dev tun1
# ip route add 192.168.200.0/24 via 192.168.200.253

----- 在123.127.177.195上
# modprobe ipip
# ip tunnel add tun1 mode ipip remote 211.167.237.218 local 123.127.177.195 ttl 64
# ip link set tun1 mtu 1480 up
# ip address add 192.168.100.253 brd 255.255.255.255 peer 211.167.237.218 dev tun1
# ip route add 192.168.100.0/24 via 192.168.100.253
{% endhighlight %}

其中需要注意的是：

mtu 会增加协议开销，因为它需要一个额外的 IP 包头，一般是每个包增加 20 个字节，如果一个网络的 MTU 是 1500 字节的话，使用隧道技术后，实际的 IP 包长度最长只能有 1480 字节了。因此，如果要使用隧道技术构建一个比较大规模的网络的话，就要仔细研究一下关于 IP 包的分片和汇聚的知识。

ttl 数据包的生存期，如果网络规模比较大就需要提高这个值，不过一般 64 是比较安全的。

<!--
3、ip 要写对端的内网ip，因为这可以减少vpn服务器不是默认网关的麻烦。对方看到的数据包实际是你本地绑定在 tun1 上的ip地址。
4、设置路由后就可以通讯了。
5、这样你已经可以和 你拨入的机器的内网卡通讯了，为了让你拨入方的其他机器也可以和你通讯，你需要增加一个 arp 的响应机制及打开ip转发功能。
arp -Ds source_ip -i lan_eth pub
sysctl -w net.ipv4.ip_forward=1
让你的内网卡可以响应这个ip的arp回应。
-->


# Generic Routing Encapsulation, GRE

需要内核模块 ip_gre.ko ，同样可以通过 modinfo ip_gre 查看模块相关的信息。

gre 最初由 CISCO 开发的隧道协议，能够做一些 ipip 所不支持的功能，如多播以及 IPv6 数据包。使用 gre 与 ipip 的方式相同，只是加载的模块以及 mode 需要修改。


<!--
linux tunnel 技术
http://www.chinaunix.net/old_jh/4/1055425.html

Cisco GRE （隧道协议）
http://www.cfanz.cn/index.php?c=article&a=read&id=87213

linux的tunnel技术实现
http://blog.chinaunix.net/uid-23392781-id-2426607.html

linux中ip tunnel的实现及协议简介
http://blog.csdn.net/starxu85/article/details/4301357
-->
