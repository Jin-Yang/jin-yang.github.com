---
title: 报文超过 MTU
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,mtu
description: 正常来说，通过 TCP 传输的报文应该不会超过 MTU 大小，一般是 1500 大小。但是，如果通过 tcpdump 获取报文，那么可能会出现报文大小超过 MTU 的报文，甚至超过 20K 。
---

正常来说，通过 TCP 传输的报文应该不会超过 MTU 大小，一般是 1500 大小。

但是，如果通过 tcpdump 获取报文，那么可能会出现报文大小超过 MTU 的报文，甚至超过 20K 。

<!-- more -->

## 大于 MTU 报文

通过类似 tcpdump 工具抓网络报文时，会发现部分超过了 MTU 大小的报文，甚至可以达到 20K+ 。

{% highlight text %}
... Flags [.], seq 295539:310019, ack 14, win 705, options [...], length 14480
... Flags [.], seq 310019:318707, ack 14, win 705, options [...], length 8688
... Flags [.], seq 318707:338979, ack 14, win 705, options [...], length 20272
... Flags [.], seq 338979:352011, ack 14, win 705, options [...], length 13032

{% endhighlight %}
其中网卡的 MTU 可以通过 `ifconfig | grep mtu` 查看，一般来说，普通网卡是 `1500` ，虚拟的回环是 `65536` ，包括在 Windows 通过 wireshark 抓包，也存在同样的问题。

## TSO

简单来说，是由于开启了 TCP Segment Offload, TSO 选项导致。

在支持 TSO 的网卡上，为了降低 CPU 的负载，提高网络的出口带宽，TSO 提供一些较大的缓冲区来缓存 TCP 发送的包，然后由网卡负责把缓存的大包拆分成多个小于 MTU/MSS 的包。

也就是，原本由内核处理的拆包，交给了网卡来处理。

而通过 tcpdump 抓包则是抓取的内核到网卡上的路径，所以会有上述的问题。如果在交换机处抓包，那么对应的大小会小于 MTU 。

## 其它

通过 `ethtool` 工具可以看到很多网卡相关的特性。

{% highlight text %}
# ethtool -k eth0
rx-checksumming: on
tx-checksumming: on
scatter-gather: on
tcp-segmentation-offload: on
udp-fragmentation-offload: on
generic-segmentation-offload: on
generic-receive-offload: on
large-receive-offload: off
{% endhighlight %}

其中大部分的优化是将本来由内核做的操作，交给了网卡，例如 TSO、UFO 和 GSO 是对应网络发送，在接收方向上对应的是 LRO、GRO ，具体含义的话还是自己看吧。

可以通过如下方式开启或者关闭这些特性。

{% highlight text %}
# ethtool -K eth1 gro off
# ethtool -K eth1 lro off
# ethtool -K eth1 tso off
{% endhighlight %}

<!--
linux tcp GSO和TSO实现
http://www.cnhalo.net/2016/09/13/linux-tcp-gso-tso/
http://packetbomb.com/how-can-the-packet-size-be-greater-than-the-mtu/

Maximum Transmission Unit, MTU
Maximum Receive Unit, MRU
-->


{% highlight text %}
{% endhighlight %}
