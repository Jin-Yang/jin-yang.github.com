---
title: ETCD 一致性读
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd
description:
---

在分布式系统中，存在多种一致性模型，不同模型给应用提供的数据保证也不同，其代价也略有区别。一般来说，一致性越强，代价越高，同时应用也越友好。

<!-- more -->

## 简介

RAFT 的实现模式是一个 `Leader` 和多个 `Followers`，所有的更新请求都经由 Leader 处理，而 Leader 再通过日志同步的方式复制到 Follower 节点。

而对于读请求处理则没有进行限制，所有的节点都可以处理用户的读请求，但是由于部分原因，可能会导致数据不一致：

* Leader 向 Follower 复制数据存在时间差，Follower 的状态总的落后于 Leader，而且 Follower 之间的状态也可能存在差异，直接读取会导致不一致；
* 如果只从 Leader 读取数据，当发生网络分区时，剩余节点已选出了新 Leader 但是旧 Leader 并没有感知到，从而出现脑裂，而此时旧 Leader 的数据已经过时。

也就是说，如果不对读流程作特殊处理，就会导致非一致性的读。

<!-- 而线性一致性的两个要求：一致性读和读最新数据更是无从谈起。 -->

### ReadIndex


etcd 通过 `ReadIndex` 机制实现线性一致读，简单来说就是：Leader 在处理读请求时，需要与集群多数节点确认自己依然是 Leader，然后读取已经被应用到应用状态机的最新数据。

<!--
基本原理包含了两方面内容：
Leader首先通过某种机制确认自己依然是Leader；
Leader需要给客户端返回最近已应用的数据：即最新被应用到状态机的数据。
-->

## 数据结构

在 `raft/read_only.go` 中保存了与只读查询相关的数据结构。

{% highlight go %}
type ReadState struct {
        Index      uint64  // 读请求产生时当前节点的Commit
        RequestCtx []byte  // 客户端读请求的唯一标识，由应用判断请求
}

type readIndexStatus struct {
        req   pb.Message            // 在处理客户端读请求时向协议核心发起的ReadIndex请求
        index uint64                // Leader当前的Commit信息
        acks  map[uint64]struct{}   // 记录Followers响应，Follower确认了Leader的心跳消息后会记录一次
}

type readOnly struct {
        option           ReadOnlyOption
        pendingReadIndex map[string]*readIndexStatus
        readIndexQueue   []string   // 所有ReadIndex请求数组
}
{% endhighlight %}

#### ReadState

记录每个客户端的读请求的状态，在使用时需要确保调用 `ReadIndex()` 函数，并最终会通过 Ready 返回给应用，由应用负责处理客户端的读请求。

为了保证线性一致性读，应用需要根据该请求发起时的节点 Commit 信息决定返回何时的数据。

#### readIndexStatus

用来追踪 Leader 向 Followers 发送的心跳信息的响应。

#### readOnly

管理全局的读 `ReadIndex` 请求。

<!--
option：暂时不确定含义；
pendingReadIndex：保存所有的待处理的ReadIndex请求，实现上是一个map，其中key为请求的唯一标识，一般为节点为请求生成的唯一request id；
-->






<!--
kvServer.Range() api/v3rpc/key.go
 |-checkRangeRequest() 实际上只是检查key的长度是否为0
 |-RaftKV.Range() etcdserver/v3_server.go 通过RaftKV定义的是接口，实际的实现是在EtcdServer中
   |-linearizableReadNotify() 默认是线性读，此时会阻塞等待处理完成
     |-EtcdServer.readwaitc  会向该管道中发送一个空结构体
	 |-
 |-header.fill()

对于 readwaitc 管道而言，另外一端是在 `linearizableReadLoop()` 函数中，也就是一个单独的协程。


ECTD 默认采用 Linearizable 方式读取，此时可以获取到整个集群当前状态，不过延迟较大；为了提高性能，可以使用 Serializable 方式读取，因为是读取的本地数据，那么可能会导致读取到老数据。



linearizableReadLoop()
 |-readwaitc 等待管道信号
 |-newNotifier() etcdserver/util.go 新建一个notifier对象
 |-node.ReadIndex() raft/node.go 这里会发送一个消息
   |-node.step() 向recvc发送一个pb.MsgReadIndex消息

https://zhuanlan.zhihu.com/p/31050303
https://zhuanlan.zhihu.com/p/31118381
http://masutangu.com/2018/07/etcd-raft-note-3/
http://masutangu.com/2018/07/etcd-raft-note-4/



上述的示例是注册了一个

linearizableReadLoop()
 |-reqIDGen.Next() 获取唯一的标示，作为请求的ID
 |-readwaitc 等待管道信号
 |-newNotifier() etcdserver/util.go 新建一个notifier对象，每次请求都会新建一个，实际上就一个管道和错误信息
 |-node.ReadIndex() raft/node.go 这里会发送一个消息
 | |-node.step() 向recvc发送一个pb.MsgReadIndex消息
 |-readStateC 等待接收管道信号
 |-getAppliedIndex() etcdserver/server.go 实际上就是获取最新的appliedIndex
 |-applyWait.Wait() 等待请求的
 |-notifier.notify() etcdserver/util.go 通过关闭管道方式

每次请求都会新建一个 readNotifier 对象，
-->

## 参考

关于一致性模型的更多描述可参考 [Strong consistency models](https://aphyr.com/posts/313-strong-consistency-models) 。

{% highlight text %}
{% endhighlight %}
