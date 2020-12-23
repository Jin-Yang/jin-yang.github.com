---
title: Socket 关闭方式
layout: post
comments: true
language: chinese
usemath: true
category: [linux]
keywords: socket,close,so_linger
description: 在 Linux 中，对于 Socket 来说，可以通过系统调用 `close()` 关闭，通过还提供了一个 `shutdown()` 接口，可以直接将 socket 关闭。那么两种方式有什么区别？
---

在 Linux 中，对于 Socket 来说，可以通过系统调用 `close()` 关闭，通过还提供了一个 `shutdown()` 接口，可以直接将 socket 关闭。

那么两种方式有什么区别？

<!-- more -->

## close

Linux 会将大部分的操作抽象成文件，那么对于网络来说 `close()` 除了将内核文件描述符关闭、释放之外，还会处理 TCP 中的缓存，以及断开连接。

默认操作是，调用 `close()` 后立即返回，如果有数据残留在套接口缓冲区中则系统将试着将这些数据发送给对方。

### SO_LINGER

可以通过 `setsockopt()` 函数设置 `SO_LINGER` 选项，用于指定 `close()` 函数关闭链接时的处理方式，修改默认的配置，其中配置使用如下结构体：

{% highlight c %}
struct linger {
     int l_onoff;  /* 0 = off, nozero = on */
     int l_linger; /* linger time */
};
{% endhighlight %}

有如下三种场景：

* `l_onoff = 0` 。默认场景，此时的 `l_linger` 被忽略，`close()` 会立即返回，在关闭 Socket 前会将缓冲区中的数据发送到对端。
* `l_onoff = 1 l_linger = 0` 。丢弃缓冲区中的数据，直接发送一个 RST 报文，而非正常的四次握手关闭，避免了 `TIME_WAIT` 状态。
* `l_onoff = 1 l_linger = N` 。尝试将缓冲区中的数据发送，超时时间通过 `l_linger` 指定。

如果套接口缓冲区中仍残留数据，进程将处于睡眠状态，直到 A) 所有数据发送完且被对方确认，然后正常关闭；B) 超时部分数据丢失。

## shutdown

这个是与套接字相关的关闭方式，可以选择关闭双向连接的一部分，其声明如下。

{% highlight text %}
#include <sys/socket.h>
int shutdown(int socket, int how);
{% endhighlight %}

其中 `how` 参数决定了关闭的方式。

<!--
https://blog.csdn.net/lgp88/article/details/7176509
https://blog.csdn.net/feiyinzilgd/article/details/5894300
http://c.biancheng.net/cpp/html/3044.html
-->

{% highlight text %}
{% endhighlight %}
