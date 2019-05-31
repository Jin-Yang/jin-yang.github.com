---
title: HTTP2 协议简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: http2,network,hpack
description: HTTP2 也就是超文本传输协议第 2 版，保持了与 HTTP1 相同的基本语义，例如 方法语义 (GET PUST PUT DELETE 等)、状态码 (200 404 500 等)、URL 等等，相比来说做了如下的优化。链路复用，通过单个 TCP 连接支持多个通道；头部压缩，解析会更快、更小等等。
---

HTTP2 也就是超文本传输协议第 2 版，基于 TCP 连接的一个上层协议，允许在一个 TCP 连接上多路复用，支持优先级、流量控制、服务器推送、首部HPACK压缩、

保持了与 HTTP1 相同的基本语义，例如 方法语义 (GET PUST PUT DELETE 等)、状态码 (200 404 500 等)、URL 等等，相比来说做了如下的优化。

通过单个 TCP 连接支持多个通道；头部压缩，解析会更快、更小等等。

<!-- more -->

![http2]({{ site.url }}/images/network/http2-introduce.png  "http2"){: .pull-center width="60%" }

## 简介

HTTP2 传输的数据是二进制的，相比 HTTP1.X 的纯文本数据，二进制数据的传输体积更小，更易于解析，纯文本帧在解析的时候还要考虑处理空格、大小写、空行和换行等问题，而二进制帧就不存在这个问题。

![http2 frame]({{ site.url }}/images/network/http2-binary-frame.svg "http2 frame"){: .pull-center width="60%" }

在 HTTP2 中，有如下的三个概念，也就是：流 (Stream)、消息 (Message) 和帧 (Frame)。

* Stream 处于一个连接中的虚拟双向二进制数据流，通过一个唯一的整数进行标识，可以包含一个或者多个 Message 。
* Message 一个完整的请求或者响应，比如请求、响应等，由一个或多个 Frame 组成。
* Frame HTTP2 通讯的最小单位，每个帧包含帧首部帧，表示当前帧所属的流，承载着特定类型的数据，如 HTTP 首部、负荷等等。

![http2 conception]({{ site.url }}/images/network/http2-connection-stream-frame.svg "http2 conception"){: .pull-center width="70%" }

每个 HTTP2 连接上传输的帧都关联到一个 "流"，这是一个逻辑上的概念，作为一个独立的双向帧序列，用于在客户端和服务器端之间进行通讯。

可以包含多个并发的流，任何一端都可以交错地插入帧。

![http2 multiplexing]({{ site.url }}/images/network/http2-multiplexing.svg "http2 multiplexing"){: .pull-center width="70%" }

## 优先级

优先级是为了设置对端在并发的多个流之间分配资源的方式，尤其是当发送容量有限时，可以选择优先发送的流。

流之间可以相互依赖，只有当所依赖的流完成后，才会再处理当前流，同时每个依赖后都跟着一个权重 (weight)，用来确定依赖于相同的流的可分配可用资源的相对比例。

### 简介

每个资源都获取一个 Stream ID 来标识连接上的资源，有三个参数用于定义资源优先级：

* 流依赖 Stream Dependencies。只有当依赖的流数据发送完成之后，才会发送当前流，默认使用共享的 Stream0 。
* 权重 Weight。分配 1~256 之间的数字，标识在多个数据流共享连接时分配给此数据流的带宽量，按照权重比例分配。
* 独占位 Exclusive Bit。用来表示在不与任何其它数据流共享带宽的情况下下载。

浏览器不一定同时知道所有资源，因此服务器能够在新请求到达时确定请求的优先级也很关键。

### 流依赖

注意，因为流可以在不同状态之间切换，如果被依赖的流不在当前依赖树中 (如流的状态为 idle )，被依赖的流会使用一个默认优先级。

当依赖一个流时，该流会添加进父级的依赖关系中，共享相同父级的依赖流不会相对于彼此进行排序，比如 B 和 C 依赖 A，新添加一个依赖流 D，BCD 的顺序是不固定的。

{% highlight text %}
    A                 A
   / \      ==>      /|\
  B   C             B D C
{% endhighlight %}

可以通过独占标识 (exclusive) 插入一个新层级，这会导致该流成为父级的唯一依赖流，而其它依赖流变为其子级。例如插入一个新带有独占标识的依赖流 E 。

{% highlight text %}
                      A
    A                 |
   /|\      ==>       E
  B D C              /|\
                    B D C
{% endhighlight %}

在依赖关系树中，只有当一个流依赖的所有流被关闭或者无法继续时，这个流才应该被分配资源。

### 权重

相同父级的依赖流按权重比例分配资源，比如 B 和 C 都依赖于 A ，其权重值分别为 4 和 12 ，那么理论上 B 能分配的资源只有 C 的三分之一。

### 优先级调整

在正常使用流的过程中，可以通过 PRIORITY 帧来调整流优先级。

如果父级重新设置了优先级，则依赖流会随其父级流一起移动；若调整优先级的流带有独占标识，会导致新的父流的所有子级依赖于这个流

如果一个流 A 调整为依赖自己的一个子级 (包括孙子) D，则首先将子级 D 移至 A 的父级之下(即同一层)，然后再移动 A 的整棵子树，移动的依赖关系保持其权重。

{% highlight text %}
    X                X                X                 X
    |               / \               |                 |
    A              D   A              D                 D
   / \            /   / \            / \                |
  B   C     ==>  F   B   C   ==>    F   A       OR      A
     / \                 |             / \             /|\
    D   E                E            B   C           B C F
    |                                     |             |
    F                                     E             E
               (intermediate)   (non-exclusive)    (exclusive)
{% endhighlight %}

如上示例将 A 调整依赖 D ，调整的步骤为：A) 现将 D 移至 X 之下；B) 把 A 调整为 D 的子树；C) 如果 A 调整时带有独占标识根据第一点 F 也归为 A 子级。

### 优先级管理

当依赖树中的某个节点被删除，那么子级会调整为了依赖父级，权重会根据被删除节点和自身的权重重新计算。

{% highlight text %}
          X(v:1.0)               X(v:1.0)
         / \                    /|\
        /   \                  / | \
      *A     B       ==>      /  |  \
    (w:16) (w:16)            /   |   \
      / \                   C   *D    B
     /   \                (w:8)(w:8)(w:16)
    C    *D
 (w:16) (w:16)
 R(C)=16/(16+16)=1/2 ==>  R(C)=8/(8+16)=1/3
{% endhighlight %}

如果上述 A D 不可用，那么图中的 B C 就会各占一半的资源，当 A 被移除后，C 和 D 会按照权重分配 A 的权重，也就是都变成了 8 ，此时 D 仍然不可用，那么 A 会占用 1/3 的资源。

<!--
可以在 HEADERS 帧中的 PRIORITY 字段指定一个新建流的优先级，也可以通过 PRIORITY 帧调整流优先级。

http2_scheduler_open() 建立依赖关系，并设置优先级、

可以参考 [gRPC over HTTP2](https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md) 中的介绍，也就是 gRPC 如何使用 HTTP2 的。
https://github.com/h2o/h2o/issues/2002

微内核与宏内核的区别
https://www.zhihu.com/question/20314255

## nghttp2

这个库里面实际上有很多不错的脚本。

nghttp2_session_send() 对应了真正发送数据
 |-nghttp2_session_pop_next_ob_item()
   |-nghttp2_outbound_queue_top() 我去，貌似使用的是队列
nghttp2_session_reprioritize_stream()
 |-nghttp2_stream_dep_add_subtree()
-->

## 帧 Frame

HTTP2 的最小数据单位是帧，所有帧以 9 字节的帧头并跟着 `0~16,383` 字节的数据。

{% highlight text %}
+-----------------------------------------------+
|                 Length (24)                   |
+---------------+---------------+---------------+
|   Type (8)    |   Flags (8)   |
+-+-------------+---------------+-------------------------------+
|R|                 Stream Identifier (31)                      |
+=+=============================================================+
|                   Frame Payload (0...)                      ...
+---------------------------------------------------------------+
{% endhighlight %}

### PING

PING：Type=0x6，发送端测量最小的RTT时间，检测连接是否可用


PING 帧定义了以下的Flags:ACK (0x1) ： 位1表示PING帧是一个PING响应。1）PING帧可以被任何终端任何时刻发送。2）PING帧必须在载体中包含一个8字节长度的数据。收到不含ACK的PING帧必须发送一个有ACK的PING响应，带相同的载荷。PING响应应设置比其他帧更高的优先级。流ID为0，不和任何流关联；


### SETTINGS

1）设置帧由两个终端在连接开始时发送，连接生存期的任意时间发送。2）设置帧的参数将替换参数中现有值，不能识别的忽略。3）设置帧总是应用于连接，而不是一个单独的流。

流ID必须为0；SETTINGS 帧定义了以下的 Flags：ACK (0x1) ： 位1，表示设置帧被接收端接收并应用。

如果设置了ACK，设置帧的载体必须为空。


## 流 Stream

流是一个逻辑上的概念，代表 HTTP/2 连接中在客户端和服务器之间交换的独立双向帧序列，每个帧的 Stream Identifier 字段指明了它属于哪个流，流有以下特性:

单个 h2 连接可以包含多个并发的流，两端之间可以交叉发送不同流的帧
流可以由客户端或服务器来单方面地建立和使用，或者共享
流可以由任一方关闭
帧在流上发送的顺序非常重要，最后接收方会把相同 Stream Identifier (同一个流) 的帧重新组装成完整消息报文





<!--
SETTINGS  ======>

<<<<< SETTINGS 连接配置参数帧 Type=0x4 用于设置参数

消息体可以包含多个参数，每个包含一个 16 位标识以及一个 32 位的值。

HTTP2的优先级介绍
https://juejin.im/post/5c1d9b8ae51d4559746922de
https://blog.csdn.net/liujiyong7/article/details/64478317



https://http2.github.io/

http2交互流程
http://www.blogjava.net/yongboy/archive/2015/03/18/423570.html
https://www.jianshu.com/p/40378501d1fc

golang http2
https://juejin.im/entry/5b3359966fb9a00e4e47c35e
curl http2支持
https://www.sysgeek.cn/curl-with-http2-support/
通过wireshark分析
https://imququ.com/post/http2-traffic-in-wireshark.html
HTTP2实现
一个不错的库，curl在使用
https://github.com/nghttp2/nghttp2
很小，基本上就一个文件
https://github.com/douglascaetano/http2
依赖一个小型的HPACK库和S2N安全库
https://github.com/64/hh
https://github.com/awslabs/s2n/
https://github.com/pyos/libcno
https://github.com/gregory144/prism-web-server
https://github.com/Niades/http2/tree/dev/src
https://github.com/pepsi7959/http2
https://github.com/NickNaso/nhttp2

https://github.com/h2o/h2o  *** 直接参考这个吧
https://github.com/nghttp2/nghttp2 功能比较详细就是有些复杂
https://github.com/h2o/picohttpparser
https://www.slideshare.net/kazuho/h2o-20141103pptx

https://github.com/ngtcp2/ngtcp2
https://github.com/douglascaetano/http2
https://www.cnblogs.com/yingsmirk/p/5248506.html
https://ye11ow.gitbooks.io/http2-explained/content/part6.html
https://www.jianshu.com/p/47d02f10757f
https://github.com/zqjflash/http2-protocol
https://blog.csdn.net/zqjflash/article/details/50179235


这里的blog有很多不错的关于HTTP2协议的介绍
https://imququ.com/post/content-encoding-header-in-http.html


## HTTP2 Priority

* 每个 Stream 通过一个 `1~256` 的数字来表示权重；
* 每个 Stream 都应该标明它的依赖有哪些。

https://segmentfault.com/a/1190000006923359

被依赖的应该先发送，然后按照优先级进行发送。
https://http2.github.io/http2-spec/

HTTP2 非常非常详细的介绍
https://juejin.im/post/5b88a4f56fb9a01a0b31a67e

## 流量控制

流控的目标是通过流量窗口进行约束，给接收端控制当下想要接受的流量大小。具体的算法流程为：

* 两端设置一个流量控制窗口 Window 初始值；
* 每次发送 DATA 帧都会减小 Window 值，减小的是帧的大小，如果 Window 小于帧大小，那么这个帧会被拆分；
* 当窗口等于 0 时，将不会发送任何帧；
* 接收端可以通过 WINDOW_UPDATE 帧指定窗口的增量。

比如说，发送端的初始 Window 为 100，当发送了一个 DATA 帧长度 70，这时 Window 值为 30；如果接收端回送 WINDOW_UPDATE(70)，那么发送端的 Window 恢复到 100 。

do_emit_writereq()
-->


## 参考

* HTTP2 的协议，也就是 RFC7540 的详细介绍 [Hypertext Transfer Protocol Version 2](https://tools.ietf.org/rfc/rfc7540.txt) 。
* 关于 HTTP2 的所有版本实现可以参考 [HTTP2 Implementations](https://github.com/http2/http2-spec/wiki/Implementations) 。
* [High Performance Browser Networking HTTP2](https://hpbn.co/http2/) 或者 [本地文档](/reference/linux/http2_protocol.html)。

{% highlight text %}
{% endhighlight %}
