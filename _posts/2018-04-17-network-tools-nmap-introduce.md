---
title: NMAP 使用
layout: post
comments: true
language: chinese
category: [network,misc]
keywords: nmap,network
description: Nmap 是一款网络扫描和主机检测的非常有用的工具，不仅可以收集信息，同时可用来作为一个漏洞探测器或安全扫描器。
---

Nmap 是一款网络扫描和主机检测的非常有用的工具，不仅可以收集信息，同时可用来作为一个漏洞探测器或安全扫描器。

<!-- more -->

![nmap logo]({{ site.url }}/images/network/nmap-logo.jpg "nmap logo"){: .pull-center width="50%" }


## 简介

NMAP 的三大功能。

1. 端口扫描，嗅探服务器对外所提供服务；
2. Ping 检测，探测主机是否在线；
3. 主机类型判断、路由次数、开放软件版本等。

在如下的测试中，可以通过如下方式查看网络报文。

{% highlight text %}
----- 通过TCPDUMP采集相关主机的请求报文
$ tcpdump -nni eth0 host 192.168.9.1
{% endhighlight %}

可以通过如下命令清除防火墙规则。

{% highlight text %}
# iptables -F
{% endhighlight %}

## Ping 检测

一个系统管理员可能只会使用 Ping 来检测某个主机是否存活，但一个安全人员可能会使用各种方法绕过防火墙进行检测。

Nmap 会根据当前扫描的网络来改变它的扫描方式：

1. 本地网络发送 ARP 数据包；
2. 非本地网路依次发送：A) ICMP echo 请求；B) TCP SYN 到端口 443；C) TCP ACK 到端口 80；D) ICMP timestap 请求。

nmap 默认会扫描端口以及主机是否存活 (Ping检测)，通过 `-sP` 或者 `-sn` 可以关闭端口扫描只进行 Ping 检测。

{% highlight text %}
----- 只Ping本地网络地址
$ nmap -sP 192.168.9.1
13:04:40.906188 ARP, Request who-has 192.168.9.1 (ff:ff:ff:ff:ff:ff) tell 192.168.9.2, length 28
13:04:40.906918 ARP, Reply 192.168.9.1 is-at 28:6e:d4:89:08:db, length 42
{% endhighlight %}

如下方式可以关闭 ARP 扫描，模拟访问远端的服务器。

{% highlight text %}
----- 只执行Ping扫描，同时不使用ARP扫描
nmap --disable-arp-ping -sP 192.168.9.1

12:59:22.828037 IP 192.168.9.2 > 192.168.9.1: ICMP echo request, id 51387, seq 0, length 8
12:59:22.828071 IP 192.168.9.2.52186 > 192.168.9.1.443: Flags [S], seq 1090282149, win 1024
12:59:22.828092 IP 192.168.9.2.52186 > 192.168.9.1.80: Flags [.], ack 1090282149, win 1024, length 0
12:59:22.828127 IP 192.168.9.2 > 192.168.9.1: ICMP time stamp query id 25509 seq 0, length 20
12:59:22.829172 IP 192.168.9.1 > 192.168.9.2: ICMP echo reply, id 51387, seq 0, length 8
12:59:22.829204 IP 192.168.9.1.443 > 192.168.9.2.52186: Flags [R.], seq 0, ack 1090282150, win 0, length 0
12:59:22.829222 IP 192.168.9.1.80 > 192.168.9.2.52186: Flags [R], seq 1090282149, win 0, length 0
12:59:22.829230 IP 192.168.9.1 > 192.168.9.2: ICMP time stamp reply id 25509 seq 0: org 00:00:00.000
{% endhighlight %}

可以看到，依次发送了：

1. ICMP echo rquest
2. TCP SYNC 到端口 443
3. TCP ACK 到端口 80
4. ICMP time stamp query

可以在对端主机上设置防火墙，那么再检测就会报错。

{% highlight text %}
# iptables -I INPUT -p ICMP -j DROP
# iptables -I INPUT -p tcp --tcp-flags ALL ACK --dport 80 -j DROP
# iptables -I INPUT -p tcp --tcp-flags ALL SYN --dport 443 -j DROP
{% endhighlight %}

### 其它方式

其它常见的 Ping 检测方式，以及通过防火墙的屏蔽方式。

{% highlight text %}
----- TCP SYN Ping 默认是 80 端口，可通过-PS443指定其它端口
# nmap --disable-arp-ping -sP -PS 192.168.9.1
# iptables -I INPUT -p tcp --tcp-flags ALL SYN -j DROP

----- TCP ACK Ping 默认是 80 端口，可通过-PA443指定其它端口
# nmap --disable-arp-ping -sP -PA 192.168.9.1
# iptables -I INPUT -p tcp --tcp-flags ALL ACK -j DROP

----- ICMP echo，也就是最通用的 PING 检测。
# nmap --disable-arp-ping -sP -PE 192.168.9.1
# iptables -A INPUT -p icmp -m icmp --icmp-type echo-request -j DROP

----- ICMP TimeStamp请求，类似ICMP echo请求，发送和返回会带上时间戳。
# nmap --disable-arp-ping -sP -PP 192.168.9.1
# iptables -I INPUT -p ICMP -j DROP

----- UDP检测，向40125发送一个UDP请求
# nmap --disable-arp-ping -sP -PU 192.168.9.1
# iptables -I INPUT -p udp -j DROP

----- 修改协议，可以发送有IP头中特定协议号数据包，默认是ICMP echo、IGMP、IPv4
# nmap --disable-arp-ping -sP -PO 192.168.9.1
# iptables -I INPUT -p IP -j DROP

----- 好吧，完全不扫描，直接返回成功PN
# nmap --disable-arp-ping -sP -PN 192.168.9.1
{% endhighlight %}

<!--
https://blog.csdn.net/q1007729991/article/details/72600130
-->

## 端口扫描

通过 `-Pn/P0` 关闭 Ping 检测，只扫描端口，为了方便查看针对单个端口的操作，这里直接指定 22 端口。

{% highlight text %}
----- SYN 扫描，执行两次握手，也称为隐藏扫描，速度快，默认方式
# nmap --disable-arp-ping -Pn -sS -p22 192.168.9.1

----- TCP 扫描，需要建立三次握手
# nmap --disable-arp-ping -Pn -sT -p22 192.168.9.1

----- ACK 扫描，用于确定 TCP 端口是否被防火墙过滤
# nmap --disable-arp-ping -Pn -sA -p22 192.168.9.1

----- UDP 扫描，DHCP DNS SNMP TFTP采用的是UDP协议
# nmap --disable-arp-ping -Pn -sU -p22 192.168.9.1
{% endhighlight %}

注意，如果使用 UDP 扫描 TCP 端口可能会误判。

另外，可以通过 `-sV` 获取开放端口的版本号信息。

## 主机类型识别

主机识别依赖于端口扫描，需要关闭上述的 `-sP/sn` 选项，可以使用 `-sS(TCP SYN)`、`-sT(Connect)`、`-sF(FIN)` 。

{% highlight text %}
---- 获取远程主机的系统类型及开放端口
# nmap -sS -P0 -sV -O 192.168.9.1
    -sS 使用TCP SYN扫描，又称半开放或隐身扫描
    -P0 关闭ICMP pings.
    -sV 打开系统版本检测(同时会检测开放端口软件的版本号)
    -O 尝试识别远程操作系统
    -A 同时打开操作系统指纹和版本检测
{% endhighlight %}

可以通过 `-v` 查看扫描的处理过程信息。

## 其它

`-T[0-5]` 指定扫描速度，数值越大扫描速度越快。

#### 指定主机

可以是如下的多种方式的组合。

{% highlight text %}
192.168.1.1-25  网段中的部分IP地址
192.168.1.1/24  指定子网
-iL iplist.txt  指定文件中的IP

--exclude 排除指定IP
--excludefile 或者通过文件指定
{% endhighlight %}


<!--
NSE检测引擎
https://thief.one/2017/05/02/1/
指纹检测数据库
https://zhuanlan.zhihu.com/p/27655551

Idle scan  FTP bounce（FTP反弹）, fragmentation scan（碎片扫描）, IP protocol scan（IP协议扫描）,以上讨论的是几种最主要的扫描方式.
https://blog.csdn.net/dong976209075/article/details/7771159
-->


{% highlight text %}
{% endhighlight %}
