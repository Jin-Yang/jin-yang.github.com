---
title: 一致性模型简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: consistency
description: 现实生活中，对于一个系统的组成会经常发生网络分区、消息丢失、发送延迟、消息重复、消息乱序等等，造成这一问题的原因有很多，包括光纤、交换机、网卡、主机硬件、操作系统、磁盘、虚拟化层、程序等等，都可能会出现各种各样的问题。此时就需要有一个比较直观的正确性模型，包括如何定义、描述。
---

现实生活中，对于一个系统的组成会经常发生网络分区、消息丢失、发送延迟、消息重复、消息乱序等等，造成这一问题的原因有很多，包括光纤、交换机、网卡、主机硬件、操作系统、磁盘、虚拟化层、程序等等，都可能会出现各种各样的问题。

此时就需要有一个比较直观的正确性模型，包括如何定义、描述。

<!-- more -->

## 简介

在学习分布式理论时，经常会遇到几个常用的名词，例如：顺序一致性 (Sequential Consistency)、因果一致性 (Causality Consistency)、线性一致性 (Linearizability Consistency)，这些名词很容易混淆。

因为摩尔定律，计算机硬件的发展速度要比软件快很多，所以并行相关的研究是最先从硬件开始的，例如 CPU 从主存中读取数据的策略、多线程编程等等。

通常我们会将一个读写操作抽象成原子操作，但实际上，例如对于写入操作，在实际开始写入，到写入完成，再到其它线程读取到，都需要经历一个过程。

常见的 Sequential 和 Linearizability 概念开始都是跟并发 (Concurrency) 编程有关的，只是这些结论同样适用于分布式下。

也就是说，虽然这一模型目前使用较多的场景是分布式系统中，而实际上最先出现的是在单机模型中，所谓的单机模型，严格来说是 Shared-Memory Multi-Processor System 。

对于如下的模型，应该返回的是 `aabd` ，也是我们正常的理解。

但实际上读取数据的时候，可能返回任意的值，例如 `a` `b` `x` 等等。

那么如何定义一个系统的正确性，简单来说，就是明确操作与状态的相关规则，而系统中的所有操作都应该遵循这些规则。这样，上层的程序设计时就不需要考虑除此之外的异常场景。


## Linearizability

多个进程对同一个全局资源进行操作，而在操作过程中会消耗时间，最快的可能一个时钟周期搞定，而其它的可能需要几百个时钟周期，也就意味着 **每个操作可能在发送请求到接收到响应之间的任意时间点发生** 。

这也就意味着，所有的操作会有两个特性：

* 每个操作包括了请求 (Invocation) 和响应 (Response) 两个事件，而且请求一定早于响应。
* 每个操作的实际生效时间在请求和响应之间的任意时间。

其中线性读，意味着，当一个操作完成之后，其它所有进程的都应该读取到它操作后的状态。基于这一模型，那么就可以实现 CAS 操作了，这也是互斥锁、信号量、列表、元组等等的基础。

同时也就意味着不会读取到发起请求前的老数据，读取到的是发起请求到结束请求之间的数据 (可能会发生修改) 。

实际上这也是最严格的方式。

## Sequential Consistency

在提出线性一致性之前，Lamport 早在 1979 年就提出了顺序一致性的概念：

> A multiprocessor system is sequentially consistent if the result of any execution is the same as if the operations of all the processors were executed in some sequential order, and the operations of each individual processor appear in this sequence in the order specified by its program.

另外，Lamport 上述的定义是基于单机的，如上所述，可以扩展推广到分布式系统领域。

这个定义实际上对系统提出了两条访问共享对象时的约束：

1. 从单个线程 (或进程) 的角度上看，其指令的执行顺序以程序中的顺序为准；
2. 从所有线程 (或进程) 的角度上看，指令的执行保持一个单一的顺序。

其中，约束 1 保证了单进程的所有指令是按照程序中的顺序执行，也就是符合我们编码最初的预期，这里针对的是执行顺序，**而非结果**；约束 2 保证了所有的内存操作都是原子的或者说实时生效的，其执行顺序不必严格遵循时间顺序。

下图中横轴表示程序中的顺序 (注意不再是时间序)，然后观察如下的两种执行结果是否满足顺序一致性要求。

{% highlight text %}
     x.W(2)  x.W(3)         x.R(5)
P1 ----+-------+--------------+----->
                    x.W(5)
P2 -------------------+------------->

     x.W(2)  x.W(3)         x.R(5)
P1 ----+-------+--------------+----->
         x.W(5)
P2 --------+------------------------>
{% endhighlight %}

如上实际已经假设了原子操作，那么只需要找出一个满足执行结果而且符合上述两个约束的执行顺序即可。

那么可以得到结果是 `P1 x.write(2) -> P1 x.write(3) -> P2 x.write(5) -> P1 x.read(5)` ，这表明其执行结果满足顺序一致性。

也就是说，SC 放松了一致性要求：A) 操作的执行顺序不必严格按照真实的时间序执行；B) 不同线程的操作执行先后没有任何要求，只需保证执行是原子性即可。

对于单机系统来说，如果要保证顺序一致性，例如 x86 ，最简答的就是直接添加 `mfence` 屏障即可。

## 其它

### Linearizability VS. Serializability

这两个概念是分布式、数据库中常见而且容易混淆的概念，简单来说，其区别可以概括为。

> Linearizability: single-operation, single-object, real-time order.
> Serializability: multi-operation, multi-object, arbitrary total order.

在 Linearizability 的定义里有顺序化 (Sequential)，而在 Serializability 的定义里有串行化 (Serial)，所以会导致这两个概念非常容易混淆。

实际上，这是两个不同领域的问题。

#### Linearizability

用来保证的是针对单个对象的多个操作行为，而且是在严格时序上的操作顺序。

简单来说，写对于应用来说是立即生效的，也就是说，当写入完成后，所有后续 (严格时序) 的读操作，获得的都是最新值；当读获取到一个值后，后面的操作将读取到该值或者最新的值。

对应了 CAP 理论中的 C 。

#### Serializability

保证的是针对多个对象的一组操作 (一般是事务)，确保这组操作等价于某个顺序执行过程。

注意，上述的顺序没有明确说是何种顺序，不必是严格的时间序，只需要某个可以确定存在的执行顺序即可，有点类似 SC 。

这是事务 (ACID) 的隔离 (Isolation) 级别，事务是有可能会读写多个对象的，该特性用于保证不同事务在执行时的效果跟串行化 (前一个事务结束了后一个事务才开始，没有重叠) 执行是一样。

注意，该特性不保证多个事务彼此之间的顺序，它只是一个保证数据库正确性条件。

#### Strict Serializability

也就是将 Linearizability 和 Serializability 结合起来，例如在事务 T1 中写入 X 的值，然后事务 T2 再读取 X 的值，那么对于 SS 来说，需要严格保证 T1 先于 T2 执行。

对于只提供了 Serializability 能力的数据库来说，是允许 T2 先于 T1 执行的。

实际上，如果数据库使用的锁来保证 (锁粒度较大)，那么就是 SS ；如果采用了 MVCC 实现，那么可能就只是 Serializability 。

#### 总结

两者的实现都必须要有一个中间协调者才可以。

真实场景是，默认数据库不会使用 Serializability ，多核的系统也不会使用 Linearizability ，如果要使用，那么会有非常高的成本，所以使用较多的是 Read Commit 以及 SC 。

## 参考

* [How to Make a Multiprocessor Computer That Correctly Executes Multiprocess Programs](https://dl.acm.org/citation.cfm?id=1311750) Lamport 关于顺序一致性最早描述。


<!--
https://mp.weixin.qq.com/s/CEaAdb88O4TBU5aPN47Lsw
http://www.bailis.org/blog/linearizability-versus-serializability/
https://cse.buffalo.edu/~stevko/courses/cse486/spring13/lectures/26-consistency2.pdf

http://kaiyuan.me/2018/04/21/consistency-concept/
-->

{% highlight go %}
{% endhighlight %}
