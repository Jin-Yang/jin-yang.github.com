---
title: Linux Wireshark
layout: post
comments: true
category: [linux, network]
language: chinese
keywords: 网络,wireshark,网络包
description: Wireshark (之前被称为 Ethereal) 是一个网络封包分析软件，接收网络封包，并尽可能显示出最为详细的网络报文资料。
---

Wireshark (之前被称为 Ethereal) 是一个网络封包分析软件，接收网络封包，并尽可能显示出最为详细的网络报文资料。

接下来见看看如何使用 Wireshark 。

<!-- more -->

![Wireshark Logo]({{ site.url }}/images/network/wireshark-logo.png "Wireshark Logo"){: .pull-center width="40%"}

## 简介

在 CentOS 中，wireshark 的安装包是在官方库中的，可以直接通过如下方式安装。

{% highlight c %}
# yum install wireshark-gnome
{% endhighlight %}

Wireshark 可以通过颜色标示报文，默认绿色是 TCP 报文、深蓝色是 DNS、浅蓝是 UDP、黑色标识出有问题的 TCP 报文 (比如乱序报文)。

### 示例

常用表达式包括了 `eq` `==` `and` `or` `!` 和 `not` 。

{% highlight text %}
----- 针对源或者目的地址过滤
ip.src == 192.168.0.1
ip.dst == 192.168.0.1
----- 可以是源或者目的地址
ip.addr == 192.168.0.1
ip.src == 192.168.0.1 or ip.dst == 192.168.0.1

----- 过滤特定的协议
http or telnet

----- 针对特定端口过滤
tcp.port == 80
udp.port >= 2048
{% endhighlight %}

<!--
　　四、针对长度和内容的过滤
　　（1）针对长度的过虑（这里的长度指定的是数据段的长度）
　　         表达式为：udp.length < 30   http.content_length <=20
　　（2）针对数据包内容的过滤
　　　　  表达式为：http.request.uri matches "vipscu"  （匹配http请求中含有vipscu字段的请求信息）
　　
    tcp dst port 3128

    显示目的TCP端口为3128的封包。
    ip src host 10.1.1.1
    显示来源IP地址为10.1.1.1的封包。
    host 10.1.2.3
    显示目的或来源IP地址为10.1.2.3的封包。
    src portrange 2000-2500
    显示来源为UDP或TCP，并且端口号在2000至2500范围内的封包。
    not imcp
    显示除了icmp以外的所有封包。（icmp通常被ping工具使用）
    src host 10.7.2.12 and not dst net 10.200.0.0/16
    显示来源IP地址为10.7.2.12，但目的地不是10.200.0.0/16的封包。
    (src host 10.4.1.12 or src net 10.6.0.0/16) and tcp dst portrange 200-10000 and dst net 10.0.0.0/8
    显示来源IP为10.4.1.12或者来源网络为10.6.0.0/16，目的地TCP端口号在200至10000之间，并且目的位于网络10.0.0.0/8内的所有封包。
-->

## 参考

* [Sample Captures](https://wiki.wireshark.org/SampleCaptures) 可以离线参考的示例，包括了 MySQL、ARP、HTTP等协议。
* [一站式学习Wireshark](http://blog.jobbole.com/70919/) 一系列介绍 WireShark 的文章。

{% highlight text %}
{% endhighlight %}
