---
title: RAFT 协议简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: raft,paxos
description: Paxos 一直是分布式协议的标准，但是 Paxos 难于理解，更难以实现，例如 Google 的分布式锁系统 Chubby 在实现 Paxos 协议时就遇到很多坑。来自 Stanford 的新的分布式协议研究称为 RAFT，它是一个为真实世界应用建立的协议，主要注重协议的落地性和可理解性。
---

Paxos 一直是分布式协议的标准，但是 Paxos 难于理解，更难以实现，例如 Google 的分布式锁系统 Chubby 在实现 Paxos 协议时就遇到很多坑。

来自 Stanford 的新的分布式协议研究称为 RAFT，它是一个为真实世界应用建立的协议，主要注重协议的落地性和可理解性。

<!-- more -->

## 简介

RAFT 算法在许多方面和现有的一致性算法都很相似，其独有的特性如下：

* 强领导者，日志只从领导者发送给其它从服务器，简化了对日志复制的管理。
* 领导选举，在所有一致性算法的心跳机制上增加了一个随机计时器来选举领导者，从而可以更加简单快捷地解决冲突。
* 日志一致，简单来说就是通过 Term 和 Index 保证所有的日志及其顺序全局唯一，同时不允许存在空洞 (Paxos允许)。
* 成员变更，使用一种共同一致的方法来处理集群成员变换的问题，在变更过程中集群依然可以继续工作。

### 复制状态机

一致性算法实际上是基于这一模型的，需要保证复制日志相同，简单来说就是初始状态相同，以相同的顺序处理指令，那么各个节点会产生相同的状态以及状态序列。目前使用比较多的是基于日志的复制方式，这一复制过程也被称为原子广播。

实际系统中使用的一致性算法通常含有以下特性：

* 安全性 safty，也就是在非拜占庭错误情况下，包括网络延迟、分区、丢包、冗余和乱序等错误都可以保证正确。
* 可用性 livness，集群中只要多数机器、客户端可以相互通讯，那么集群就是可用的，如果机器异常崩溃，大部分场景可以从持久化中恢复。
* 非时序依赖，通常不会因为物理时钟、消息延迟导致可用性问题。

### 拜占庭问题

这一问题实际上是 Lamport 老爷子在 《The Byzantine Generals Problem》中提出的，这实际上是一个虚构的故事，准确来说是研究分布式系统容错性的时候编出的一个故事。

不同的版本可能会略有出入，简单如下。

假设有9位将军投票，其中1名叛徒。8名忠诚的将军中出现了4人投进攻，4人投撤离的情况。这时候叛徒可能故意给4名投进攻的将领送信表示投票进攻，而给4名投撤离的将领送信表示投撤离。这样一来在4名投进攻的将领看来，投票结果是5人投进攻，从而发起进攻；而在4名投撤离的将军看来则是5人投撤离。这样各支军队的一致协同就遭到了破坏。

拜占庭将军问题更像是一个错误模型，简单地说，Byzantine Generals Problem 是针对所谓的 Byzantine Failure 来说的，而 Byzantine Failure 是指分布式系统中的某一恶意节点允许做任意事情去干扰系统的正常运行 (包括选择性不传递消息、选择性伪造消息等)，如何保证在这一场景下确保整个系统不会异常。

简单来说，就是在缺少可信的中央节点和可信任的通道的情况下，分布在网络中的各个节点如何达成共识的问题。

#### Practical Byzantine Fault Tolerance, PBFT

经典的 PBFT 的解决方案是需要所有节点均交换数据，有明显的扩展性问题。

#### Proof of Work, POW

在出现比特币之前，解决分布式系统一致性问题主要是 PAXOS 及其衍生算法，仅适用于中心化的分布式系统，这样的系统的没有不诚实的节点，也就是不会发送虚假错误消息，但允许出现网络不通或宕机出现的消息延迟。

在区块链的实现中通过 POW 来解决这一问题。

<!-- 拜占庭故障是最严重最难处理的，当前的系统都是通过如何规避而非解决这一问题。-->

<!--
http://marknelson.us/2007/07/23/byzantine/
https://www.zhihu.com/question/23167269
一篇经典的英文介绍
http://marknelson.us/2007/07/23/byzantine/
区块链核心技术：拜占庭共识算法之PBFT
http://blog.liqilei.com/bai-zhan-ting-gong-shi-suan-fa-zhi-pbftjie-xi/
什么是拜占庭将军问题，关于POW工作量证明的解释
https://learnblockchain.cn/2018/02/05/bitcoin-byzantine/
区块链研习 | 看懂“拜占庭容错”，也就看懂了区块链的核心技术
https://www.leiphone.com/news/201709/YAd57zwnq8C1IGc2.html
论文
Practical Byzantine Fault Tolerance, PBFT
-->


## RAFT 算法

算法中的大部分工作是为了消除不确定性，例如不允许日志有空洞。为了方便理解，将算法分成了几个模块，包括了领导选举、日志复制、安全性、成员变更。

日志管理主要是通过 Leader 负责，从客户端接收日志，并将日志复制到其它从服务器上，同时通知其它从服务器什么时候将日志应用到状态机上，通过这一机制大大简化了对复制日志的管理。

Leader 会直接决定日志存放的位置，而不需要和其它服务器进行协商，当 Leader 宕机后，会从其它服务器上重新选举出来。

### 关于 Term

也就是一个任期，此时的 Leader 不变，每次都会通过发送的消息进行同步更新，如果当前服务器保存的 term 较小，那么就会更新到更大的值。

其中 term 的标识从 1 开始单调递增，而且总是以选举开始，总的时间不定，可能无限长，也可能只有选举时间。而且因为网络的延迟，各个节点看到相同term的时间点不同，极端情况下可能会有丢失某个 term 的情况。

Term 相当于不依赖墙上时钟的逻辑时间，同样可以用来做一些常规的判断，例如判断是否是一个过期的 Leader、如果收到了一个过期的请求则直接丢弃。

如果 Candidate 和 Leader 发现自己的 term 过期，那么会自动切换到 Follower 状态。

<!--
### 状态以及接口

#### 持久化状态

也就是在响应服务间的 RPC 调用前，需要持久化到磁盘中的数据，这个是对于所有服务器而言：

current_term 当前看到的最大 Term 值，启动时是 0 然后单调递增；
vote_for 投票给那个服务器；WHY??????
log 也就是在复制时的日志集，包含了操作指令，以及 Leader 的当前 Term，可以有多个，从 1 开始递增。

#### 非持久化

一般保存在内存中，或者启动时可以从日志中恢复。

所有机器中会保存如下内容：两个索引值什么区别WHY???????

commit_index 已知被提交日志的最大索引值，从 0 开始单调递增；
last_applied 已经应用到状态机的最大索引值，从 0 开始单调递增；

Leader 中包含的非持久化信息 (完成选举后需要重新初始化)：

next_index 对于每个服务器，需要发送给它们的下个日志索引值；
match_index 对于每个服务器，已经发送的最高索引值；

#### AppendEntries

用于复制日志，同时也会用作 Heartbeat(此时entries为空) 。

参数：

term  Leader的当前term值
leaderId 标识当前的leader，用于客户端的重定向
prevLogIndex 用于标识最近提交的log
prevLogTerm
entries[] 也就是真实的日志，可以多条；如果是心跳那么为空
leaderCommit 已经提交的日志序号，当Follower发现对应的日志被提交后会将日志应用到日志中；

响应：

1. 如果当前缓存的 term 大于消息中的 term ，则直接丢弃，并返回失败；可能是网络较差导致上个任期的请求在本次任期中到达。
2. 如果日志中找到了对应的 pervLogTerm 值，但是没有 pervLogIndex ，则直接返回失败；此时 Leader 会依次递减直到返回成功，从而保证日志的一致性。
3. 如果找到了匹配的 pervLogIndex ，但是对应的 term 值不匹配，此时需要直接删除之前的日志；可能是因为由于频繁重启导致日志混乱，删除的日志通常是未达成多数派导致的脏数据，详见Figure7。
4. 将不在日志中的条目添加到本地的日志中，一般是缓存中。
5. 如果 leaderCommit > commitIndex (本地缓存)，那么更新当前服务器已知的最近提交日志，也就是 commitIndex = min(leaderCommit, index of last new entry)。

然后会返回成功。

#### RequestVote

用于 Candidate 收集投票，当 Follower 在选举的超时时间内没有收到 AppendEntries 请求或者 RequestVote 请求，就认为当前 Follower 由于某些原因宕掉了，于是开始发起选举。

参数：

term Candidate的当前term值；
candidateId 标示是那个服务发起的选举请求；
lastLogIndex
lastLogTerm

响应：

1. 如果当期缓存的 term 大于消息中的 term 值，那么直接返回失败+当前的term；一般是由于当期那已经选出了主。
2. 如果votedFor是 NULL 或者发起请求的 CandidateID ，且 Candidate 的日志比当前服务器的日志更新，那么会进行投票。
3. 否则不进行投票。是否直接不返回结果??????

在比较那个日志更新时，首先比较日志当前的 term 值，然后是 index ，其中值比较大的表示日志更新。

???????? 5.1 什么场景下可能会导致无法选出

### 领导选举

当前的 Leader 宕机时，如何重新选举一个 Leader 出来。

### 日志复制

RAFT 采用了强一致的日志模型，不允许日志间有空洞出现，从而简化了日志复制过程。只有 Leader 从客户端接收日志请求(Append-Only)，同时单向复制到其它节点，并保证日志及其顺序相同。

日志中包含了三部分：A) Log Index 所有机器保持一致，从 1 开始单调递增；B) 日志提交时的 Term 值，也就是主节点的 Term；C) 该操作的日志内容。这也就意味着，只要是 index 和 term 相同，那么保存了相同的命令，且在整个流程中唯一。

一般的处理流程为：

1. 接收用户请求，并添加到本地的日志中(保存在内存中)；
2. 调用 AppendEntries() 接口将所有请求发送给其它机器；
3. 当接收到多数响应后，由 Leader 决定是否添加到状态机(是否持久化)，此时称为已经提交(commited)；客户端是否需要持久化??????
4. 响应客户端。
5. 当从节点发现了已经提交的日志时，就应用到本地的状态机。

如果因为丢包、网络慢、宕机、主机负载高等原因导致日志添加失败，那么 Leader 会一直尝试发送请求，直到所有日志复制成功。

这样，RAFT 可以保证已经 commited 的请求会最终持久化，并可以最终被集群中的其它机器执行。


#### 异常处理

在论文的 F7 中列举了一系列可能出现的异常场景，RAFT 采取的方式是强制所有的节点与主节点保持一致。这里有两个问题，A) 如何确定哪些是最新的日志；B) 如何将最新的日志进行同步。

Leader 会为每个 Server 维护两个值：A) nextIndex 领导者下次需要发送给客户的日志序号；B)

关于第二步，是通过 AppendEntries 调用实现的。Leader 中通过 nextIndex 保存了下次需要发送的日志序号；每次 Leader 启动时，会将该值设置为 Leader 的当前序号，在调用 AppendEntries 时，对应的节点如果 term 匹配了，但是日志序号不匹配，会返回失败，此时 Leader 会依次递减该值，直到返回成功。

这也就意味着 Leader 的日志是 Append Only 。

### 安全性

也就是状态机的安全，如果一条日志已经确认添加到了状态机里面，整个集群就不会在相同日志索引上出现不同的指令。

实际上，这里会利用上述的领导选举、日志复制机制，然后再添加一定的限制确保整个集群的状态机正常。仅以上面的机制，可能会导致一个日志 index 较低的机器被选举为 Leader，进而导致对客户端来说部分已经提交的日志被覆盖。

简单来说，就是确保一台机器被选举为 Leader 时，该节点含有全部的已提交日志。

#### 保障机制

1. 投票时必须满足多数派原则才可能当选 Leader ；
2. 不允许上个 term 的日志通过多数派的机制进行提交；

对于第二点解释如下。

在日志复制阶段讨论过，只要 Leader 已经确认多数派机器已经收到了日志就认为该日志已经完成了提交，那么在提交之前如果主宕机，在选举出的新 Leader 会保证日志被提交。

在 F8 中，如果允许老的 term 同样以多数派的方式提交，可能会导致上一个 term 被提交的日志，在下个 term 中被覆盖掉。为此，RAFT 不允许上个 term 的日志通过多数派的机制进行提交。


到此为止，RAFT 的核心功能特性已经完成。

5.4.3 Safety Argument

F8(b): S1


客户端的状态未知场景：

1. F8(b) 在 (a) 时如果 S1 宕机，会导致当时的客户端链接异常，此时的状态未知：A) 有可能未满足多数派而提交失败；B) 满足了多数派，但是没有来得及响应，新主会重新复制日志。

### 成员变更

## 时间和可用性

RAFT 的安全性不会依赖时间，而可用性 (系统可以及时响应客户端) 不可避免的会依赖时间，

## SnapShot

RAFT 中每个服务器独立的创建快照，只包括已经被提交的日志，主要的目的是将状态机写入到快照中，同时也包含一些少量的元数据。

其中元数据包括：A) 最近的日志索引，也就是状态机最后的应用日志；B) 最近的任期，也就是快照是的任期。

到快照中：最后被包含索引指的是被快照取代的最后的条目在日志中的索引值（状态机最后应用的日志），最后被包含的任期指的是该条目的任期号。

这两个数据用于支持 AppendEntries 方法，第一个条目附加日志请求时的一致性检查，因为检查时需要确认最后的索引值和任期号。一旦最新的快照更新成功，那么之前的日志和快照就可以被删除了。

### SnapShot复制

正常来说，各个服务器独立创建快照，但是，当 Follower 节点上线时，Leader 会将该 Follower 上落后的日志复制过去，如果 Follower 下线时间比较久，就可能会导致很多被回收的历史日志无法在 Follower 上进行回收，进而导致各个节点的状态不一致。

因为日志回收是在对当前应用进行 SnapShot 之后进行的，而被回收的日志的状态已经反映在 SnapShot 中了，因此可以直接复制 SnapShot 以及 SnapShot 之后的更新日志到 Follower 。

https://zhuanlan.zhihu.com/p/29865583
https://my.oschina.net/fileoptions/blog/1637873

## Linearizable Read

https://www.jianshu.com/p/d888642ef72c
有很多对于一致性的介绍
http://opass.logdown.com/
介绍一致性读的实现
https://zhuanlan.zhihu.com/p/31050303

就是读请求需要读到最新已提交的数据，而非老数据。

在分布式系统中，存在多种一致性模型，包括了严格一致性、线性一致性、顺序一致性等，不同的一致性模型给应用提供的数据保证也不同，其代价也不一样，一致性越强，代价越高；但是一致性越强，对应用的使用也就越友好。

关于一致性模型的介绍可以查看 [Strong Consistency Models](https://aphyr.com/posts/313-strong-consistency-models) 。

首先，现在的 RAFT 模型中，Follower 和 Leader 之间必然存在延迟，而且各个 Follower 的状态不同。

按照 RAFT 协议的规定，Leader 有最新的状态，所以如果读请求访问的是 Leader ，那么正常直接返回数据给客户端即可。但真实的场景是，当出现网络分区时，如果其它的 Follower 已经选出了新的 Leader ，同时提交了一堆的请求；而此时老的 Leader 还没有发现自己已经不是 Leader 了，那么就可能返回了老的数据，也就是 Stale Read。

如果不对读流程作任何的特殊处理，上述限制就会导致一个非一致性的读，而线性一致性的两个要求：一致性读和读最新数据更是无从谈起。

在论文的 Section 8 中有对此的介绍，通过 ReadIndex 的机制来实现线性一致读，简单来说就是先要确认两个内容：

1. 确认需要返回最新的数据。Leader 必须维护最近提交的日志索引，正常的任期中会保存该信息；如果是被新选举为主，为了获取该信息，需要发送一个 NOOP 请求。
2. 当前仍然为主。在响应 Read-Only 请求前，需要先确认当前节点是否仍然为主；RAFT 通过心跳的多数派原则确认。

RAFT 作者在其 [Consensus: Bridging Theory and Practice Section-6.4](https://ramcloud.stanford.edu/~ongaro/thesis.pdf) 提出了一种叫做 ReadIndex 的方案：

也就是当主接收到读请求时，将当前 CommitIndex 记录下来，记作 ReadIndex ，在返回结果给客户端之前，主需要先确定自己现在还是不是真的主，方法是给其它所有节点发送一次心跳，如果收到了多数派的响应，说明至少这个读请求到达这个节点时，这个节点仍然是主，这时只需要等到 Commit Index 被 应用到状态机后，即可返回结果。


### 源码解析

关于线性读的实现在 raft/read_only.go 中。


## 性能指标

一般的性能指标通过两个因素决定：A) 延迟(latency)，完成一次接口调用的操作时间；B) 吞吐量 (throughput)，在某个时间期间内(一般是秒)完成操作的总数量。一般随着并发量的增加，平均延迟会同时增加。

对于三个成员组成的 ETCD，通常在轻负载下可以在低于 1ms 内完成一个请求，并在重负载下可以每秒完成超过 30000 个请求。

对于 ETCD 这类分布式系统而言，延迟一般受限于两个物理约束：网络延迟和磁盘延迟。完成一次请求的最小时间包括了两部分时间的相加：A) 各成员之间的网络往返时延(Round Trip Time, RTT)；B) 需要提交数据到持久化存储的 fdatasync 时间。

一般一个数据中心的 RTT 在毫秒内，机械硬盘一般是 10ms，SSD 通常可以低于 1ms，同时为在高负载下提高吞吐量，一般会将多个请求打包。

另外需要考虑的是序列化、反序列化，存储引擎 BoltDB 的 MVCC，通常可以在 10us 内完成；还有就是做快照合并时可能会造成延迟尖峰 (Latency Spike)，在机械盘上尤其明显。

MustSync() 通过该接口判断是否需要强制同步刷新到磁盘

在 example 的示例程序中。

rc.node.Ready() 每次通过获取请求后都会检查是否需要SnapShot
rc.maybeTriggerSnapshot()

## 保活性

异常时返回客户端的最大延迟时间为任期的超时时间。

etcd-raft的Leader节点维护了集群Follower节点的日志同步状态，以此作为下一次日志复制的线索。


日志实现在 `type raft struct` 中的 `raftLog *raftLog` 对象，会在 `newRaft()` 函数中进行初始化。

raft.newRaft() 新建RAFT对象
 |-newLog() 会从SnapShot中读入commitIndex和lastApplied的日志序号

raft.stepLeader()
 |-raft.sendAppend() 发送日志
 | |-Progress.becomeSnapshot() 切换到snapshot状态
 |-raft.send() 发送MsgSnap日志请求给客户端

raft.handleSnapshot() 在接收到SnapShot消息之后执行，包括Follower和Candidate raft/raft.go

## 成员变更

增加成员或者变换成员角色都属于这一类，在变更时要求同一个任期内不能同时出现两个 Leader，不过从论文的 Figure-10 中可以看到，如果老的配置中 Server1 是主，直接修改配置，在新增两个主机之后可能会在某个时间出现两台主的情况。

为此，在做成员变更时采用两阶段 (Two-Phases) 提交最新配置。

RAFT 在修改集群配置时引入了中间状态，称之为共同一致 (Joint Consensus)，此时会包含了新旧两种配置。通过这种方式可以完成配置调整，同时不影响集群的正常工作。


真正配置切换的时机是主节点发起新配置请求时，不管是否已经提交，同时带来的问题是，如果这条成员变更日志最终没有commit，在发生leader切换的时候，成员组就需要回滚到旧的成员组。？？？怎么回滚？？什么场景会出现

Etcd-RAFT 为了实现简单，将切换成员组的时机选在 Apply 成员变更日志的时候，这一部分在 raft/doc.go 中有相关介绍。

node.ProposeConfChange() raft/node.go 开始提交成员变更申请
 |-Marshal()
 |-node.Step() 发送类型为EntryConfChange的报文，对应stepLeader()函数

在对应的 RAFT 处理协程中，会判断报文类型是否为 e.Type == pb.EntryConfChange，如果是且 pendingConf 为真，也就是有报文正在处理，那么就会重置该报文的类型，从而忽略。

http://www.cnblogs.com/foxmailed/p/7190642.html


## LeaderShip Transfer

在运维场景使用较多，简单来说，就是将集群中的 Leader 身份转给另外的节点。

RAFT 协议要求新主必须包含所有已经提交的日志，有其固定的选主协议，而 PAXOS 对选主没有要求，任何一个成员都可以成为主，选主协议需要单独实现。

为了实现这一功能，作者提出了一个方案作为 RAFT 扩展。

其原理是，保证 Transferee 拥有和原领导者有相同的日志，期间需要停写，然后给 Transferee 发送一个特殊的消息，让这个 Follower 可以马上进行选主，而不用等到选举超时。

TransferLeadership() raft/node.go 开始执行切换，对应的消息为MsgTransferLeader

上述的接口可以在任一主机上发送，会尝试将该节点提升为 Leader，对于 stepFollower() 中会直接给主发送报文；在 stepLeader() 函数中，的处理会比较复杂。首先检查是否有 Transfer 在执行。

## Linearizable Read

ChangeOver
FailOver

/post/golang-etcd-introduce.html
## 源码编译

可以直接通过 git 下载代码，如果网络不通，也可以直接下载然后编译。

$ git clone https://github.com/coreos/etcd.git
$ cd etcd
$ ./build

最终会在当前目录下生成 bin 目录。

一般放在 $GOROOT 目录下，如果该环境变量不存在则会保存到 $GOPATH/src 目录下。

https://zhuanlan.zhihu.com/distributed-storage

/post/golang-etcd-introduce.html
下载完成后可以直接放到任意目录下，然后通过如下命令查看版本信息。
./etcd --version
ETCDCTL_API=3 ./etcdctl version

为了方便使用，可以直接将这两个二进制文件复制到 `/usr/bin` 目录下。

https://coreos.com/etcd/docs/latest/

https://github.com/coreos/etcd/blob/master/Documentation/dev-guide/local_cluster.md


在本地集群测试时，直接使用 goreman 工具。

----- 直接将日志重定向到一个临时文件
goreman start > /tmp/goreman.log


关于配置项可以查看 [Configuration flags](https://coreos.com/etcd/docs/latest/op-guide/configuration.html) 中的介绍。

--name
  集群中节点名，可区分且不重复即可；
--listen-peer-urls
  用于集群内部之间通信的URL，包括选举、数据同步等，可监听多个；
--listen-client-urls
  用于接收来自客户端的请求，同样可以监听多个；
--advertise-client-urls
  用于客户端或者代理访问集群，可以用来发现集群的节点，一般是上述listen-client-urls的集合？？？？；

----- 查看当前成员
ETCDCTL_API=3 etcdctl --write-out=table --endpoints="http://localhost:2379,http://localhost:22379" member list


----- 获取key的值
$ etcdctl get /message

ETCDCTL_API=3 etcdctl --write-out=table endpoint status


https://www.jianshu.com/p/5aed73b288f7
http://www.cnblogs.com/foxmailed/p/7137431.html
http://blog.neverchanje.com/2017/01/30/etcd_raft_core/
https://www.jianshu.com/p/ae1031906ef4
https://www.jianshu.com/p/ae462a2d49a8
http://chenneal.github.io/2017/03/16/phxpaxos%E6%BA%90%E7%A0%81%E9%98%85%E8%AF%BB%E4%B9%8B%E4%B8%80%EF%BC%9A%E8%B5%B0%E9%A9%AC%E8%A7%82%E8%8A%B1/
http://vlambda.com/wz_xberuk7dlD.html
https://github.com/maemual/raft-zh_cn/blob/master/raft-zh_cn.md
http://www.opscoder.info/ectd-raft-library.html

etcd-raft的线性一致读方法一：ReadIndex
https://zhuanlan.zhihu.com/p/31050303


http://dockone.io/article/2955
-->


## 参考

#### 常见参考

官方地址 [raft.github.io](https://raft.github.io/)，常见的参考地址有：
1. [Github RAFT 中文翻译](https://github.com/maemual/raft-zh_cn)，比较不错的中文翻译，仅供参考；
2. [In Search of an Understandable Consensus Algorithm(Extended Version)](https://ramcloud.stanford.edu/~ongaro/thesis.pdf) 简版论文，也可以参考 [本地文档](/reference/databases/RAFT/0-In Search of an Understandable Consensus Algorithm.pdf)；
3. 从理论应用到实践的论文 [Raft consensus algorithm](https://github.com/ongardie/dissertation)，也就是作者的博士毕业论文，[本地文档](/reference/databases/RAFT/1-CONSENSUS BRIDGING THEORY AND PRACTICE.pdf)；
4. 关于实现的细节补充可以参考 [Four modifications for the Raft consensus algorithm FM-RAFT](http://openlife.cc/system/files/4-modifications-for-Raft-consensus.pdf)，以及 [本地文档](/reference/databases/RAFT/3-4-modifications-for-Raft-consensus.pdf) 。

#### 其它

一些常见一致性算法可以参考 [Github Awesome Consensus](https://github.com/dgryski/awesome-consensus) 。

{% highlight text %}
{% endhighlight %}
