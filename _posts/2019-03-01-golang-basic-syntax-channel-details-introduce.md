---
title: GoLang 管道详解
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: 
description:
---

在 Go 语言中，通过协程和管道实现了 Communicating Sequential Processes, CSP 模型，两者承担了通信和同步中的重要角色。

<!-- more -->

## 简介

通过 Channel 提供了如下的语义：

* 协程安全，需要遵从 Go 的内存模型；
* 在不同的协程之间发送消息，提供 FIFO 语义；
* 可以让协程阻塞、非阻塞。


<!--
运行时调度器和内存管理系统是如何支持Channel的，

### 实现

内存在 `$GOROOT/src/runtime/chan.go` 中实现，其实就是一个带锁的环形队列。
type hchan struct {
  ...
  buf      unsafe.Pointer // 指向一个环形队列
  ...
  sendx    uint   // 发送 index
  recvx    uint   // 接收 index
  ...
  lock     mutex  //  互斥量
}

当通过 `ch := make(chan int, 3)` 创建管道时，会在堆中分配空间、初始化，并返回一个指针，在使用时直接传递指针即可，无需在取地址。

#### 接收和发送

在发送和接收的过程中，会将对象复制一份，这样，所有协程只会共享 `hchan` 这个结构体。

#### 阻塞和恢复

Go 的调度器是 `M:N` 调度模型，既 `N` 个协程会运行于 `M` 个 OS 线程中。换句话说，一个 OS 线程中，可能会运行多个协程。

如果要运行一个协程 `G` ，在一个线程 `M` 中，必须持有一个改协程的上下文 `P` 。

https://blog.lab99.org/post/golang-2017-10-04-video-understanding-channels.html
https://github.com/gophercon/2017-talks/blob/master/KavyaJoshi-UnderstandingChannels/Kavya%20Joshi%20-%20Understanding%20Channels.pdf
https://zhuanlan.zhihu.com/p/27917262
https://i6448038.github.io/2019/04/11/go-channel/


## Resiliency

### Breaker

断路器，主要是为了防止由于某个节点故障导致整个调用链路上的整体调用异常，最终导致服务不可用。

也就是说，断路器是保证即使生产者服务宕机，可以确保整个服务优雅地处理问题，并将应用的其余部分从级联故障中保存下来。

### Retrier

重试机制，通常来说某个服务会部署多个实例 (主机) ，当一个实例宕机之后应该要重试其它机器。

这也就意味着，Retrier 一般会包含在 Breaker 中。

https://github.com/eapache/go-resiliency

通过Turbine进行Stream Aggregator
https://medium.com/netflix-techblog/hystrix-dashboard-turbine-stream-aggregator-60985a2e51df
https://blog.csdn.net/weixin_38748858/article/details/100781369
https://about.sourcegraph.com/go/grpc-in-production-alan-shreve
Netflix 最新的限流神器 Concurrency Limits
https://fredal.xin/netflix-concuurency-limits
https://medium.com/@NetflixTechBlog/performance-under-load-3e6fa9a60581
https://github.com/Netflix/concurrency-limits
高效的流式处理
https://github.com/bmizerany/perks


【繁中字幕】手嶌葵 - テルーの唄（歌集バージョン）
-->



{% highlight text %}
{% endhighlight %}
