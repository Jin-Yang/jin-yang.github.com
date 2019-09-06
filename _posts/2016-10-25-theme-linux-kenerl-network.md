---
title: 【专题】Linux 网络专题
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

对与内核中网络部分的介绍。

* [Linux 网络协议栈简介](/post/network-introduce.html)，简单介绍一下与 Linux 协议栈相关的内容。
* [Linux 网络设置](/post/network-setting.html)，Linux 中一些常见的网络设置。
* [Netstat VS. ss](/post/network-nettools-vs-iproute2.html)，netstat 和 ss 命令是比较典型的网络监控工具，在此介绍对比下。
* [Linux 常用网络命令](/post/network-commands.html)，简单记录常用网络命令，如 tcpdump、netcat 等。
* [Linux 网络常见监控项以及报错](/post/linux-monitor-network.html)，与网络相关的调试、查看方法，当然也包括了报错相关的内容。
* [Linux 通讯协议](/post/network-protocols.html)，简单记录下 Linux 常见的通讯协议，如 SNMP 协议。
* [Linux 的防火墙](/post/network-netfilter-iptables.html)，Linux 中的防火墙策略，包括 netfilter 和 iptables 。
* [TCP/IP 协议简介](/post/network-tcpip-introduce.html)，简单介绍一下与 Linux 协议栈相关的内容。
* [TCP/IP 协议之 TIME_WAIT](/post/network-tcpip-timewait.html)，简单介绍下 TCP 协议栈中，TIME_WAIT 这一特殊的状态值。
* [TC 使用简介](/post/network-traffic-control-introduce.html)

<!--
* [Linux 中的 socketfs](/post/network-socketfs.html)，也就是 Linux 中应用层与内核网络协议栈之间的中间层。
* [TCP/IP 简介之一](/post/network-tcpip-introduce-1.html)
* [TCP/IP 简介之二](/post/network-tcpip-introduce-2.html)
* [TCP/IP 之 timestamp 选项](/post/network-tcpip-timestamp.html)
* [Linux 网络超时与重传](/post/network-timeout-retries.html)，主要介绍TCP的三次握手、数据传输、链接关闭阶段都有响应的重传机制。
* [Linux IP 隧道技术](/post/network-ip-tunneling.html)，说明下网络协议栈是如何实现隧道的，实际上就是将不同协议进行封装。
* [Linux Wireshark](/post/network-wireshark.html)，介绍 Linux 中的 Wireshark 使用方式。
-->

## DNS

在通过浏览器访问某个网站时，或者说访问网络上的服务器时，可以直接使用 IP 地址，但是对于人类来说很难记忆，为此引入了域名。

而为了可以做到自动解析，于时就有了 DNS 。

* [DNS 基本概念](/post/network-dns-basic-introduce.html) 介绍 DNS 协议相关的内容，dig nslookup 等工具使用。
* [DNS 协议详解](/post/network-dns-protocol-details-introduce.html) 关于 DNS 协议的详细内容介绍。
* [DNSmasq 使用](/post/linux-dnsmasq-introduce.html) 一个本地的 DNS 解决方案，可以提供 DNS DHCP 等功能。
* [resolv.conf 简介](/post/network-dns-resolv-conf-usage-introduce.html) 也就是 `/etc/resolv.conf` 配置文件的使用。
* [c-ares 异步 DSN 简介](/post/network-dns-async-resolve-introduce.html) cares 提供了简单的异步 DSN 解析，很多开源工具使用。

## HTTP2

* [HTTP2 协议简介](/post/linux-network-http2-protocol-introduce.html) 
* [HTTP2 使用简介](/post/linux-network-http2-basic-usage-introduce.html) 

{% highlight text %}
{% endhighlight %}
