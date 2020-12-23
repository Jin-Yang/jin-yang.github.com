---
title: Flink 流处理简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: flink
description:
---


<!-- more -->

## 流处理

按照其发展阶段大致分为了四个阶段。

1. Hadoop 承载的 MapReduce 框架；
2. 支持 DAG 框架的计算引擎，主要是批处理任务，例如 Tez 和 Oozie；
3. 支持 Job 内的 DAG ，例如 Spark；
4. 大数据统一计算引擎，包括流处理、批处理、MachineLearing、图计算等，例如 Flink 。

可以看到数据计算模型分成了两类：

* 流式，只要数据在产生，那么计算就会持续地进行，一般支持低延迟、Exactly-Once 保证；
* 批处理，在预定义的时间内运行计算，当计算完成时释放计算机资源，需要支持高吞吐、高效处理；

一般会将流式和批处理分别进行处理，但是 Flink 在实现时，从另一个视角看待流处理和批处理，将二者统一起来。Flink 支持流处理，而批处理作为一种特殊的流处理，只是它的输入数据流被定义为有界的。

其中 Flink 是一个针对流数据和批数据的分布式处理引擎，代码主要是由 Java 实现，部分代码是 Scala，其处理的主要场景是流数据，批数据只是流数据的一个极限特例而已。

### 安装

直接从 [Flink Downloads](https://flink.apache.org/downloads.html) 页面上下载相应的版本，例如 `Apache Flink 1.9.1 for Scala 2.11`，然后通过 `bin/start-cluster.sh` 脚本启动即可，默认会监听 8081 端口，可以直接访问。

默认解压后的文件权限有问题，可以通过 `chown` 命令修改。

#### 示例

在安装包中的 `examples` 目录下有很多测试用例，可以使用一个简单的 WorkCount 示例。

{% highlight text %}
$ nc -l -p 9000
$ bin/flink run examples/streaming/SocketWindowWordCount.jar --hostname 127.0.0.1 --port 9000
{% endhighlight %}

该程序会每隔 5 秒统计一次接收到数据中的单词个数，可以查看 `log/flink-root-taskexecutor-0-HOSTNAME.out` 。

<!--
Golang中的流处理
https://github.com/chrislusf/gleam
https://github.com/chrislusf/glow

https://www.cnblogs.com/fanzhidongyzby/p/6297723.html
https://cloud.tencent.com/developer/article/1191942
http://lionheartwang.github.io/blog/2018/03/05/flink-framwork-introduction/
https://github.com/apache/flink/blob/master/flink-examples/flink-examples-streaming/src/main/java/org/apache/flink/streaming/examples/socket/SocketWindowWordCount.java

* JobManager 作为协调分布式执行的 Master ，可以用来调度任务、失败任务恢复、设置 CheckPoint 等，运行时至少需要一个 Master ，也可以设置多个，其中一个是 Master 其它的是 StandBy 。
* TaskManager 也就是一个 Worker ，用于执行一个 Dataflow 的任务、数据缓冲以及数据交换，在运行时至少会存在一个 Worker 处理器。

## TaskManager

每个 TaskManager 是一个独立的 JVM 进程，可以在独立的线程上执行一个或多个子任务，通过 Slots 来控制任务的并发。

## DataFlow

Flink 程序由 Source、Transformation、Sink 三个核心组件组成，分别用于获取数据，对数据进行转换，最终数据的输出，在各个组件之间流转的数据称为流 Streams 。

### 容错机制

## Exactly-Once

Flink 作为实时计算引擎，在实际的业务场景中会涉及到不同的组件，但并不是所有的组件都支持 Exactly-Once 的语义。

## 参考

[Apache Flink Documentation](https://ci.apache.org/projects/flink/flink-docs-stable/) 官方的参考文档。

http://www.sohu.com/a/303217982_349139

在流处理系统中，对数据的处理提供了 3 种级别的语义定义，以此来衡量这个流处理系统的能力。

* At Most Once 最多一次。每条数据最多被处理一次，也意味着数据有丢失 (没被处理) 的可能，尽力而为。
* At Least Once 最少一次。每条数据至少被处理一次，可以保证数据不会丢 (至少被处理过一次)，但是可能会被重复处理。
* Exactly Once 恰好一次。每条数据正好被处理一次，没有数据丢失，也没有重复的数据处理。

最后一条是三个语义里要求最高的，也就是说，接收到数据，有且只会影响一次输出，即使出现网络丢包、机器故障等异常场景，仍然可以保证数据不丢失、不重复。

## Exactly Once

即使在各种故障的情况下，流应用程序中的所有算子都保证事件只会被处理一次。通常有两种方式来保证这一语义：

* 分布式快照、状态检查点，保存的状态包括了当前应用状态以及输入的位置信息。
* 至少一次事件传递和对重复数据去重，保证操作的幂等。

其中快照和检查点是受到 Chandy-Lamport 分布式快照算法的启发，通过这种机制，流应用程序中每个算子的所有状态都会定期做 CheckPoint ，当发生失败时，每个算子的所有状态都回滚到最新的全局一致点，在回滚期间，所有的处理都会暂停。

另外，源也会重置为与最近 CheckPoint 相对应的正确偏移量，那么，整个流基本上是回到最近一次的一致状态，然后程序可以从该状态重新启动。

### CheckPoint

Flink 并未采用幂等的机制，采用的是 CheckPoint 机制，会周期性的生成 CheckPoint 信息，并持久化到存储系统上，例如 HDFS ，持久化过程是异步操作。

当异常时，Flink 会从最近一次持久化的任务开始处理。???异步持久化，恢复使用最近保存成功的CheckPoint，那么如何保证的不会处理多次???

#### Two-Phase Commit

这需要数据的消费端支持事务，例如 Kafka ，可以在发生异常的时候能够进行回滚。

https://streaml.io/blog/exactly-once
https://juejin.im/post/5cf0ebe05188251c064813be

Kafka设计解析语义与事务机制原理
http://www.jasongj.com/kafka/transaction/
-->

{% highlight text %}
{% endhighlight %}
