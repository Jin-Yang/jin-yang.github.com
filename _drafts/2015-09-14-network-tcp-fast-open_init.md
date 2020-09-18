---
Date: Auguest 05, 2015
title: TCP Fast Open (TFO) 介绍
layout: post
comments: true
language: chinese
category: [linux, network]
---

TFO 相关标准是由 Google 提出，目前已经是正式标准。该优化简单来说，是对 TCP 的增强，在 3 次握手期间也用来交换数据。

这主要是由于当前的很多链接都是短链接，通常建立链接之后，只有几次数据交互，尤其像现在广泛使用的 HTTP 协议。这样就导致很多无效的网络交互，而 TFO 正是基于此种场景的优化。

接下来我们会介绍 TFO 相关的内容。
<!-- more -->

# 介绍

要注意，TFO默认是关闭的，因为它有一些特定的适用场景，下面我会介绍到。

TCP_FASTOPEN 参数。




在内核中有两个控制参数：

1. net.ipv4.tcp_fastopen

    对不同的位置位将对应不同的功能，0 表示关闭，1 表示

The tcp_fastopen file can be used to view or set a value that enables the operation of different parts of the TFO functionality. Setting bit 0 (i.e., the value 1) in this value enables client TFO functionality, so that applications can request TFO cookies. Setting bit 1 (i.e., the value 2) enables server TFO functionality, so that server TCPs can generate TFO cookies in response to requests from clients. (Thus, the value 3 would enable both client and server TFO functionality on the host.)

The tcp_fastopen_cookies file can be used to view or set a system-wide limit on the number of pending TFO connections that have not yet completed the three-way handshake. While this limit is exceeded, all incoming TFO connection attempts fall back to the normal three-way handshake.



 我来简单的介绍下这个东西.想了解详细的设计和实现还是要去看上面的rfc。

 1 http的keepalive受限于idle时间，据google的统计(chrome浏览器),尽管chrome开启了http的 keepalive(chrome是4分钟)，可是依然有35%的请求是重新发起一条连接。而三次握手会造成一个RTT的延迟，因此TFO的目标就是去除 这个延迟，在三次握手期间也能交换数据。

 2 RFC793允许在syn数据包中带数据，可是它要求这些数据必须当3次握手之后才能给应用程序，这样子做主要是两个原因，syn带数据可能会引起2个问 题。第一个是有可能会有前一个连接的重复或者老的数据的连接(syn+data的数据)，这个其实就是三次握手的必要性所决定的。第二个就是最重要的，也 就是安全方面的，为了防止攻击。

 3 而在TFO中是这样解决上面两个问题的，第一个问题，TFO选择接受重复的syn,它的观点就是有些应用是能够容忍重复的syn+data的(幂等的操 作)，也就是交给应用程序自己去判断需不需要打开TFO。比如http的query操作(它是幂等的).可是比如post这种类型的，就不能使用TFO， 因为它有可能会改变server的内容. 因此TFO默认是关闭的，内核会提供一个接口为当前的tcp连接打开TFO。为了解决第二个问题，TFO会有一个Fast Open Cookie(这个是TFO最核心的一个东西),其实也就是一个tag。

 4 启用TFO的tcp连接也很简单，就是首先client会在一个请求中(非tfo的)，请求一个Fast Open Cookie(放到tcp option中),然后在下次的三次握手中使用这个cookie(这个请求就会在3次握手的时候交换数据).

 下面的张图就能很好的表示出启用了TFO的tcp连接：






Google研究发现TCP三次握手是页面延迟时间的重要组成部分，所以他们提出了TFO：在TCP握手期间交换数据，这样可以减少一次RTT。根据测试数据，TFO可以减少15%的HTTP传输延迟，全页面的下载时间平均节省10%，最高可达40%。

目前互联网上页面平均大小为300KB，单个object平均大小及中值大小分别为7.3KB及2.4KB。所以在这种情况下，多一次RTT无疑会造成很大延迟。

在HTTP/1.1中，我们有了keep-alive。但实际应用并不理想，根据Google对一个大型CDN网络的统计，平均每次建连只处理了2.4个请求。

为了证实TCP三次握手是目前页面访问的重要性能瓶颈之一，Google工程师分析了Google的web服务器日志及Chrome浏览器统计数据。

2011年6月，对Google web服务器进行连续7天数十亿次针对80端口的请求分析，包括搜索、Gmail、图片等多个服务。其中第一次建连被称为cold requests，复用连接的请求称为warm requests。

图中描述了在cold requests及all requests中TCP握手时间在整个延迟时间中的占比。在cold requests中，TCP握手时间占延迟时间的8%-28%；即使统计所有的请求，TCP握手时间也占到了延迟时间的5%-7%。



简单说明：客户端通过TCP连接到服务器时，可以在SYN报文携带数据，这将提升TCP的效率(4%-41%)[5]。前提是在这个SYN报文中，有代表客户端的在之前的TCP连接中服务器产生的cookie字段。在客户端和服务器第一次的TCP连接创建过程中，是通常的三次握手过程，但是服务器会产生cookie作为后续TCP连接的认证信息，这就避免了恶意攻击。

对于客户端用户程序来说，可直接使用sendto等带有对端地址的系统调用发送数据，如果是第一次连接（或者cookie过期），则退化到正常三次握手过程，如果是非第一次连接，则可以继续创建连接且能够直接将数据交付给应用层处理。




# 参考

[RFC-7413, TCP Fast Open][TCP-Fast-Open]，在 RFC 中的相关定义。

[TFO Paper][TFO-paper]，Yuchung Cheng 写的论文，Google 工程师，从国立台湾大学获得学士学位，加州大学圣迭戈分校获得博士学位。

[www.lwn.net][TFO-linux]，TFO 在 Linux 中实现的相关介绍。

[TCP-Fast-Open]:   https://tools.ietf.org/html/draft-ietf-tcpm-fastopen-10             "RFC-7413, TCP Fast Open"
[TFO-paper]:       http://conferences.sigcomm.org/co-next/2011/papers/1569470463.pdf   "Papper, TCP Fast Open"
[TFO-linux]:       http://lwn.net/Articles/508865/                                     "lwn.net 中关于TFO的介绍"

http://www.vants.org/?post=210
http://blog.csdn.net/hanhuili/article/details/8540227
