---
title: P2P 协议简介
layout: post
comments: true
language: chinese
category: [program,network,linux]
keywords:
description:
---


<!-- more -->

## 简介

BitTorrent 实际上是一个协议族，其基本协议是 [BEP-3](http://www.bittorrent.org/beps/bep_0003.html) ，其它的大部分协议都是围绕这个来进行扩展或补充。

如果要通过 P2P 协议进行下载，必须要包含如下的几个文件：

* 种子文件，一般后缀为 `.torrent` ，本质上是一个由 bencode 编码的文本文件，把资源分成很多虚拟块，并记录每个块的 hash 值，还包括了文件大小、名字、Tracker服务器等信息。
* BT 客户端，也就是专门解析 BT 协议的程序，例如迅雷、电驴等。
* Tracker 服务器，记录着 Peer 和种子相关信息，起着中心调控的作用。

在下载资源的时候，大致的流程如下：

1. 客户端解码种子文件，得到 Tracker 服务器的地址和资源信息；
2. 通过和 Tracker 服务器沟通得到其它已经下载该资源的 Peers 信息；
3. 再和这些 Peers 沟通得到自己想要的部分。

可以看到，在上述的流程中，Tracker 是整个流程的关键部分，如果不能通过 Tracker 读取到 Peers 的相关信息，那么也就无法下载资源。

### DHT 协议

为了弥补 Tracker 的单点问题，于是出现了 Distributed Hash Table, DHT 协议，该协议基于 [Kademlia](http://www.ic.unicamp.br/~bit/ensino/mo809_1s13/papers/P2P/Kademlia-%20A%20Peer-to-Peer%20Information%20System%20Based%20on%20the%20XOR%20Metric%20.pdf) 建立。

<!--
https://juejin.im/entry/57aaf1630a2b5800587503b5

https://libtorrent.org/
Bencode的编码与解码
https://www.cnblogs.com/technology/p/BEncoding.html

通过WireShark解析Torrent协议
https://www.aneasystone.com/archives/2015/05/analyze-magnet-protocol-using-wireshark.html
一个很简单的协议实现
https://github.com/drusong/BitTorrent

一些 TrackerServer 的列表
https://github.com/ngosang/trackerslist

https://github.com/libtorrent/libtorrent
https://github.com/willemt/yabtorrent
https://www.irif.fr/~jch//software/bittorrent/
http://react-guide.github.io/react-router-cn/docs/Introduction.html
https://github.com/joeyguo/blog/issues/2
http://marcobotto.com/overview-of-ui-router-react-and-comparison-with-react-router/


Micro Transport Protocol, μTP，因为 μ 输入困难 μTP 通常被写为 uTP 。这是一个由 BitTorrent 公司开发的协议，在 UDP 协议之上实现可靠传输与拥塞控制等特性，可以克服多数防火墙和 NAT 的阻碍，从而大大提高用户的连接性以及下载速度。

关于该协议的详细内容可以参考 [uTorrent Transport Protocol](http://www.bittorrent.org/beps/bep_0029.html) 中的内容。

max_window 最大窗口，在传输过程中 (in-flight) 可能的最大字节，也就是已经被发送但是还没有收到响应的报文；
cur_window 当前窗口，当前正在传输中的字节数。
wnd_size   窗口大小，也就是对端建议使用的窗口大小，同时会限制传输过程中的字节数。

只有当 (cur_window + packet_size) <= min(max_window, wnd_size) 时才可以发送数据，

A socket may only send a packet if cur_window + packet_size is less than or equal to min(max_window, wnd_size). The packet size may vary, see the packet sizes section.

-->

## 参考

* 一个资源嗅探器的网站 [bthub.io](http://bthub.io/) 。
* 官方网站 [bittorrent.org](http://bittorrent.org/) 包括了很多相关协议的开发文档。
* 一个传输可视化的示例网站 [mq8.org](http://mg8.org/processing/bt.html) 。

{% highlight text %}
{% endhighlight %}
