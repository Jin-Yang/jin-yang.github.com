---
title: CAP 理论简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: cap theory,distributed system
description:
---


<!-- more -->

## 简介

当一个系统中机器数增加时，系统内可能发生失败的几率会随着结点数增加而呈指数级增加。

{% highlight text %}
P(any failure) = 1 - P(individual node not failing)^(number of nodes)
{% endhighlight %}








## 基本概念

分布式领域 CAP 理论，可以简单介绍如下：

* 一致性，数据一致更新，每次都会读取到最近写入的结果；
* 可用性，如果没有失败，用户的请求在一定时间内都会得到应答；
* 分区容忍性，即使节点之间的连接关闭，其它两个属性也会得到保证。

正常来说，分布式系统中各节点互通，但可能因为一些原因 (网络、GC、BUG 等 <!-- https://github.com/aphyr/partitions-post -->)，使得有些节点间不通，那么网络就分成了几块区域，数据散布在这些不连通的区域中，形成分区。

如果数据只在单个结点保存，分区后就会导致不连通的网络无法访问数据，这时分区就是无法容忍的，最简单的就是复制多份数据，以提高容忍性。

然而，复制多份数据就带来了一致性问题，要保证多结点的数据一致需要等待多数甚至全部结点写成功，不过此时就带来了可用性的问题。

总的来说，数据节点越多分区容忍性越高；但要复制更新数据就越多，一致性就越难保证；为了保证一致性，更新所有节点数据所需要的时间就越长，可用性就会降低。

![cap]({{ site.url }}/images/databases/cap-introduce.png "cap"){: .pull-center }

也就是说，任何分布式系统只可同时满足二点，没法三者兼顾，对于系统的架构设计，需要从中进行取舍。

<!--
关系数据库的ACID模型拥有 高一致性 + 可用性 很难进行分区：
Atomicity原子性：一个事务中所有操作都必须全部完成，要么全部不完成。
Consistency一致性. 在事务开始或结束时，数据库应该在一致状态。
Isolation隔离层. 事务将假定只有它自己在操作数据库，彼此不知晓。
Durability. 一旦事务完成，就不能返回。

跨数据库两段提交事务：2PC (two-phase commit)， 2PC is the anti-scalability pattern (Pat Helland) 是反可伸缩模式的，JavaEE中的JTA事务可以支持2PC。因为2PC是反模式，尽量不要使用2PC，使用BASE来回避。

BASE模型反ACID模型，完全不同ACID模型，牺牲高一致性，获得可用性或可靠性：
Basically Available基本可用。支持分区失败(e.g. sharding碎片划分数据库)
Soft state软状态 状态可以有一段时间不同步，异步。
Eventually consistent最终一致，最终数据是一致的就可以了，而不是时时高一致。

BASE思想主要强调基本的可用性，如果你需要High 可用性，也就是纯粹的高性能，那么就要以一致性或容忍性为牺牲，BASE思想的方案在性能上还是有潜力可挖的。
-->



## 理解CAP

一般任务 CAP 三者并不对等，P 是基础，只能从 CA 之间做选择；在全球分布式系统中，网络分区是一个自然的事实，甚至被认为是必然的。


在这样的情况下，有两种声音：
    因为分区是必然的，系统设计时，只能实现AP和CP系统，CA系统是不可能的。
    从技术上来说，分区确实会出现，但从效果来说，或者从概率来说，分区很少出现，可以认为系统不会发生分区。由于分区很少发生，那么在系统不存在分区的情况下没什么理由牺牲C或A。

从更广阔的分布式计算理论背景下审视CAP理论，可以发现C，A，P三者并不对等。

CAP之父在《Spanner，真时，CAP理论》一文中写道：如果说Spanner真有什么特别之处，那就是谷歌的广域网。Google通过建立私有网络以及强大的网络工程能力来保证P，在多年运营改进的基础上，在生产环境中可以最大程度的减少分区发生，从而实现高可用性。

从Google的经验中可以得到的结论是，一直以来我们可能被CAP理论蒙蔽了双眼，CAP三者之间并不对称，C和A不是P的原因啊（P不能和CA trade-off，CP和AP中不存在tradeoff，tradeoff在CA之间）。提高一个系统的抗毁能力，或者说提高P（分区容忍能力）是通过提高基础设施的稳定性来获得的，而不是通过降低C和A来获得的。也就说牺牲C和A也不能提高P。



还有一种说法是，放弃C不是为了获得A，而是为了低延迟（延迟不也是可用性的内涵吗？我这里有疑问）。PNUTS为了降低WAN上的消息事务的延迟（几百毫秒，对于像亚马逊和雅虎这样的企业需要实施的许多Web应用程序来说，成本太高），采用放弃一致性的做法。



而CA是系统的刚性强需求，但是CA两者也不对等。系统无论如何要保证一致性（无论事先还是事后，这是系统设计的最大不变性约束，后文会详述），在这个前提下，可以谈谈可用性的程度。Google的Spanner就是这样的思路。



总结：P是一个自然的事实，CA是强需求。三者并不对等。



补充：文章写完之后，看到最新出版的文章《分布式数据库中一致性与可用性的关系》，值得一读。


4.2  保证不发生分区，CA也不容易兼得


在分布式系统中，安全性，活性是本质需求，并且广泛的研究结果是分布式系统中一直存在一个广泛意义的trade-off：在不可靠的分布式系统中无法同时实现安全性和活性。分布式系统的设计中充满了安全性和活性的trade-off，FLA著名的论文《Impossibility of Distributed Consensus withOne Faulty process》就是说我们不可能设计一个算法既可以绝对保证一致性（安全性）又无需时间等待的实现一致性（活性）。



CAP就是这个trade-off的的集中体现。分别对应于:

    Safety:非正式的说，一个算法没有任何坏的事情发生，那么该算法就是安全的。CAP中的C就是典型的safety属性：所有对客户的响应都是正确的。

    Liveness:相反，一个算法最终有有一些好的事情发生，那么该算法就是活性的。CAP中的A就是典型的liveness属性：所有的客户最终都会收到回应。



FLA中的故障是指：

Unreliable:有很多种方式可以让一个系统不可靠，CAP中的P，以及其他故障：系统崩溃，消息丢失，恶意攻击，拜占庭故障等。



所以，CAP理论可以说是FLA不可能性的不同表达方式。P只是Unreliable的一种极端形式而已。在Google的Chubby文章中，指出paxos协议维护系统的safety，引入时钟来保证liveness，由此克服FLA的不可能性。实际上，基本的Paxos协议可以保证值一旦被选出后就一定不会改变，但不能保证一定会选出值来。换句话说，这个投票算法不一定收敛。所以在系统设计时，paxos算法的使用是有条件的。



在数据库领域，CAP也正是ACID和BASE长期博弈(tradeoff)的结果。ACID伴随数据库的诞生定义了系统基本设计思路，所谓先入为主。2000年左右，随着互联网的发展，高可用的话题被摆上桌面，所以提出了BASE。从此C和A的取舍消长此起彼伏，其结晶就是CAP理论。

从ACID和BASE来说，ACID是为了保证一致性而诞生，因而侧重一致性；BASE是为了高可用系统的设计而诞生，因而侧重可用性。在分解C和A的情况时，肯定要涉及P，所以CAP理论统一了这一切。如果非要说酸碱，或者说酸碱平衡，那就是平衡于CAP理论。



CAP并不与ACID中的A（原子性）冲突，值得讨论的是ACID中的C（一致性）和I（隔离性）。ACID的C指的是事务不能破坏任何数据库规则，如键的唯一性。与之相比，CAP的C仅指单一副本这个意义上的一致性，因此只是ACID一致性约束的一个严格的子集。如果系统要求ACID中的I（隔离性），那么它在分区期间最多可以在分区一侧维持操作。事务的可串行性（serializability）要求全局的通信，因此在分区的情况下不能成立。



C与A之间的取舍可以在同一系统内以非常细小的粒度反复发生，而每一次的决策可能因为具体的操作，乃至因为牵涉到特定的数据或用户而有所不同。我们在分布式系统设计的两大原则中讨论过保持一致性的手段：同步复制和异步复制，结合复制协议的各种模式，请参考下表。例如简单满足了C，但延迟升高了，吞吐量下来了，还有什么可用性？我觉得延迟是包含在可用性的范围内的，不可用就是延迟的极大极限值。还有文章就只讨论一致性，可用性和性能问题（比如阿里何登成的《数据一致性-分区可用性-性能——多副本强同步数据库系统实现之我见》），说明在不考虑分区的情况下，CA问题依然是系统设计的难点。


重新审视本文的时候，恰好看到一个新的理论PACELC：even when the system is running normally in theabsence of partitions, one has to choose between latency (L) and consistency(C). 可谓和我的想法不谋而合。

PACELC：In case of network partitioning (P) in a distributed computersystem, one has to choose between availability (A) and consistency (C) (as perthe CAP theorem), but else (E), even when the system is running normally in theabsence of partitions, one has to choose between latency (L) and consistency(C).（https://en.wikipedia.org/wiki/PACELC_theorem）


可用性并不是简单的网络连通，服务可以访问，数据可以读取就是可用性，对于互联网业务，可用性是完整的用户体验，甚至会延伸到用户现实生活中（补偿）。有的系统必须容忍大规模可靠分布式系统中的数据不一致，其中原因就是为了在高并发条件下提高读写性能。



必须容忍大规模可靠分布式系统中的数据不一致，有两个原因：在高并发条件下提高读写性能， 并要区分物理上导致的不一致和协议规定的不一致：

    节点已经宕机，副本无法访问（物理）

    法定数模型会使部分系统不可用的分区情况，即使节点已启动并运行（paxos协议）

    网络断开，节点孤立（物理）



所以，保证不发生分区，CA也不是免费午餐：尽管保证了网络可靠性，尽量不发生分区，同时获得CA也不是一件简单的事情。



CA系统才是真正的难点。



宣称是CA系统的，目前有两家：一家是Google的Spanner，一家是Alibaba的OceanBase。


4.3 发生分区时，也不要随意牺牲CA


虽然架构师仍然需要在分区的前提下对数据一致性和可用性做取舍，但具体如何处理分区和恢复一致性，这里面有不计其数的变通方案和灵活度。

当发生分区时，系统设计人员不应该盲目地牺牲一致性或可用性。当分区存在或可感知其影响的情况下，就要预备一种策略去探知分区并显式处理其影响。这样的策略应分为三个步骤：探知分区发生，进入显式的分区模式以限制某些操作，启动恢复过程以恢复数据一致性并补偿分区期间发生的错误。



这一切都需要在系统设计之初考虑到，并在测试时模拟各种故障保证覆盖到你的测试点。

构建高度稳健的基础设施永远是第一要务，所以我不认为网络分区与CA属性是对等的。











## 参考

<!--
https://github.com/stone-note/articles
https://mp.weixin.qq.com/s/gV7DqSgSkz_X56p2X_x_cQ
-->

关于数据库中一致性和可用性的探讨可以参考 [分布式数据库中一致性与可用性的关系](http://www.jos.org.cn/ch/reader/create_pdf.aspx?file_no=5433&journal_id=jos) ，或者直接参考 [本地文档](/reference/databases/distributed-database-consistency-available.pdf) 。

{% highlight text %}
{% endhighlight %}
