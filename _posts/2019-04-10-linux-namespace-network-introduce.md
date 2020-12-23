---
title: 网络 Namespace
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

## Namespace

{% highlight text %}
---- 查看帮助信息
# ip netns help

----- 当前NS的列表
# ip netns list

----- 添加NS，会在/var/run/netns目录下创建
# ip netns add foobar

----- 删除全部或者指定的NS
# ip -all netns delete
# ip netns delete foobar
{% endhighlight %}

对于每个 Network Namespace 来说，它会有自己独立的网卡、路由表、ARP 表、iptables 等和网络相关的资源。

然后通过 `ip netns exec` 命令可以在 NS 中执行命令。

{% highlight text %}
----- 查看网卡信息
# ip netns exec foobar ip addr

----- lo网卡默认关闭，可以通过如下命令打开
# ip netns exec foobar ip link set lo up

----- ping回环地址
# ip netns exec foobar ping -c 3 127.0.0.1
{% endhighlight %}

## veth pair

全称是 Virtual Ethernet Pair ，也就是一个成对的端口，从一端进入的数据包都将从另一端出来，反之一样。

通过 veth 可以在不同的 NS 之间进行通信，其实现很简单，可参考 `drivers/net/veth.c` 。

{% highlight text %}
----- 创建veth pair
# ip link add type veth

----- 也可以在创建的时候指定名称
# ip link add vethfoo type veth peer name vethbar
# ip link delete dev vethbar type veth

----- 查看刚创建的网络设备
# ip addr


----- 将vethfoo添加到刚创建的foobar中，并启动
# ip link set vethfoo netns foobar
# ip netns exec foobar ip link set vethfoo up
# ip netns exec foobar ip addr add 10.0.1.1/24 dev vethfoo
# ip netns exec foobar ip addr

----- 启动本地的vethbar，并检查连接
# ip link set vethbar up
# ip addr add 10.0.1.2/24 dev vethbar
# ip netns exec foobar ping -c 3 10.0.1.2
{% endhighlight %}

一旦将 veth 的对端添加到另一个 Network Namespace 中，那么在当前 Namespace 中就看不到它了，那么如何能知道这个其对端在哪？

{% highlight text %}
----- 查看设备的序号，也就是8
# ip netns exec foobar ethtool -S vethfoo
NIC statistics:
     peer_ifindex: 8

----- 如果在其它的NS中，需要在多个NS中通过exec执行
# ip link | grep ^8
{% endhighlight %}

## 网桥

通过 veth pair 可以在两个不同的 Network Namespace 中进行通讯，但只能实现两个网络接口之间的通信，如果想实现多个网络接口之间的通信，就可以使用网桥。

简单来说，网桥就是把一台机器上的若干个网络接口 "连接" 起来，结果就是，某个网口收到的报文会被复制给其它网口并发送出去。

### 工作原理

网桥对报文的转发基于 MAC 地址，会解析收发报文，读取目标 MAC 地址信息，和自己记录的 MAC 表结合，来决策报文的转发目标网口。

为了实现这些功能，网桥会学习源 MAC 地址，在转发报文时，网桥只需要向特定的网口进行转发，从而避免不必要的网络交互。

如果它遇到一个自己从未学习到的地址，就无法知道这个报文应该向哪个网口转发，就将报文广播给所有的网口 (报文来源的网口除外)。

![bridge]({{ site.url }}/images/docker/veth-pair-bridge-arch.png "bridge"){: .pull-center width="60%" }

{% highlight text %}
{% endhighlight %}
