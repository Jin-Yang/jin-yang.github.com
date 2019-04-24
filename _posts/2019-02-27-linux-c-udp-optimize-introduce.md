---
title: UDP 通讯优化
layout: post
comments: true
language: chinese
category: [program]
keywords: udp,dns,statsd,icmp
description: 有部分应用场景采用的仍然是无连接协议，例如 DNS、StatsD 等，都是采用的 UDP 。 UDP 不是面向连接的，所以不能像 TCP 通过建立多个连接来提高对服务器的并发访问，如果通过多线程共享一个 UDP Socket 可能会无法充分利用所有的 CPU 资源。 这里简单介绍其优化方法，当然，这里的策略也适用与像 ICMP 这样的协议。
---

有部分应用场景采用的仍然是无连接协议，例如 DNS、StatsD 等，都是采用的 UDP 。

UDP 不是面向连接的，所以不能像 TCP 通过建立多个连接来提高对服务器的并发访问，如果通过多线程共享一个 UDP Socket 可能会无法充分利用所有的 CPU 资源。

这里简单介绍其优化方法，当然，这里的策略也适用与像 ICMP 这样的协议。

<!-- more -->

## Reuse Port

Google 为了解决 DNS 服务器性能问题，就给 Linux 内核打了一个 `SO_REUSEPORT` 的 Patch，用来优化 UDP 服务器在多核机器上的性能。

REUSEPORT 允许多线程 (或多进程) 服务器的每个线程都可以监听同一个端口，并且每个线程都拥有一个独立的 Socket，而不是所有线程都共享同一个  Socket。

如果没有设置该选项，那么在尝试绑定同一个端口时会返回报错。

### 分发策略

使用了 ReusePort 策略之后，因为存在了多个 Socket ，那么服务端收到客户端发送的报文后，会按照四元组 `<ClientIP ClientPort ServerIP ServerPort>` 的 Hash 值进行包的分发，目的有如下几个

* 保证同一个客户度过来的包会发送到同一个 Socket 上；
* 当客户端量足够多时，基本可以将请求均衡到所有的 Socket 上。

这也就意味着在压测时，需要使用尽量多的客户端，以保证压力的均衡。

## Recvmmsg

这是一个批量的系统 API 接口，可以从 Socket 中一次读取多个 UDP 数据包，而像 `recvmsg()` `recvfrom()` 一次只能读取一个报文。

注意，通过该接口读取的多个报文不一定是属于同一台机器的，大部分是属于不同的机器。

## 参考

<!--
使用reuseport和recvmmsg优化UDP服务器_应用服务器
https://blog.csdn.net/chenycbbc0101/article/details/52469754
-->

{% highlight text %}
{% endhighlight %}
