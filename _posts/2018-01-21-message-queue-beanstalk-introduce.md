---
title: Beanstalk 使用简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: c,beanstalk,message queue
description:
---

这是 FaceBook 实现的一个消息队列，其协议是基于 ASCII 的，上报的数据可以通过 binlog 进行持久化，其高可用类似于 Memcached 的方式，也就是各个进程之间不知道相互的存在。

支持优先级、延时、超时重发、预留等机制，支持分布式的后台任务和定时任务处理。

这里简单介绍其使用方法。

<!-- more -->

![beanstalk logo]({{ site.url }}/images/linux/beanstalk-logo.jpg "beanstalk logo"){: .pull-center }

## Beanstalk

简单介绍其常用概念：

* JOB 一个需要异步处理的任务，作为基本单元，属于其中的某个 Tube ；
* Tube 任务队列，类似于 Kafka 的 Topic，用于存储同一类型的任务，每个 Tube 由一个就绪队列和延迟队列组成，每个 Job 的所有状态迁移在对应的 Tube 中完成。

Tube 会按需求创建，当其变为空 (No ready/delayed/buried jobs) 且没有任何客户端引用是，将会被自动删除。

<!--
常见的错误码包括了如下：

OUT_OF_MEMORY 服务器内存不足，客户端应该稍后重试
INTERNAL_ERROR 内部错误，正常不会发生，此时意味着可能出现了Bugs
BAD_FORMAT 发送的指令格式错误，例如不是以\r\n结尾、要求整型值等等
UNKNOWN_COMMAND 未知的命令，客户端发送的指令服务器不支持
-->

### 任务的生命周期



{% highlight text %}
----- 正常生命周期
   put            reserve               delete
  -----> [READY] ---------> [RESERVED] --------> *poof*


   put with delay               release with delay
  ----------------> [DELAYED] <------------.
                        |                   |
                 kick   | (time passes)     |
                        |                   |
   put                  v     reserve       |       delete
  -----------------> [READY] ---------> [RESERVED] --------> *poof*
                       ^  ^                |  |
                       |   \  release      |  |
                       |    `-------------'   |
                       |                      |
                       | kick                 |
                       |                      |
                       |       bury           |
                    [BURIED] <---------------'
                       |
                       |  delete
                        `--------> *poof*
{% endhighlight %}


<!--
一个工作任务job当client使用put命令时创建。在整个生命周期中job可能有四个工作状态：ready，reserved，delayed，buried。在put之后，一个job的典型状态是ready，在ready队列中，它将等待一个worker取出此job并设置为其为reserved状态。worker占有此job并执行，当job执行完毕，worker可以发送一个delete指令删除此job。

Status	Description
ready	等待被取出并处理
reserved	如果job被worker取出，将被此worker预订，worker将执行此job
delayed	等待特定时间之后，状态再迁移为ready状态
buried	等待唤醒，通常在job处理失败时

#### 常用指令

如上，Beanstalk 采用的是 ASCII 指令模式。

生产者包括了 put(产生消息)、use(切换Tube) 指令；

消费者指令包括了 reserve(消费消息)、release(重新放回队列)

注意，如果生产者设置了 TTR 那么需要消费者在这一时间范围内处理完成，如果超时则会重新转换为 READY 状态。release 一般用于任务执行失败后重新放入到队列中，可以直接准备下次执行，或者等待一段时间后执行。
-->

## Tubes

一个服务器有一个或者多个 tubes，用来储存统一类型的 job 。

<!--
每个tube由一个就绪队列与延迟队列组成。每个job所有的状态迁移在一个tube中完成。consumers消费者可以监控感兴趣的tube，通过发送watch指令。consumers消费者可以取消监控tube，通过发送ignore命令。通过watch list命令返回所有监控的tubes，当客户端预订一个job，此job可能来自任何一个它监控的tube。

当一个客户端连接上服务器时，客户端监控的tube默认为defaut，如果客户端提交job时，没有使用use命令，那么这些job就存于名为default的tube中。

tube按需求创建，无论他们在什么时候被引用到。如果一个tube变为空（即no ready jobs，no delayed jobs，no buried jobs）和没有任何客户端引用，它将会被自动删除。
-->

## 常用命令

{% highlight text %}
----- 链接到DB
$ telnet 127.0.0.1 11300

----- 查看tubes
$ list-tubes
$ list-tube-used
$ list-tubes-watched
{% endhighlight %}

## 参考

官网 [beanstalkd](http://kr.github.io/beanstalkd/) 及其中文协议的结合 [Beanstalkd中文协议](https://github.com/kr/beanstalkd/blob/master/doc/protocol.zh-CN.md)。


<!--
SocketPair进程间通讯
http://blog.csdn.net/PirLCK/article/details/52526523

人工智能之机器学习常见算法
http://blog.csdn.net/baihuaxiu123/article/details/51475384

https://github.com/kr/beanstalkd/blob/master/doc/protocol.zh-CN.md
http://www.hulkdev.com/posts/think_in_beanstalkd
https://segmentfault.com/a/1190000009604082
-->

{% highlight text %}
{% endhighlight %}
