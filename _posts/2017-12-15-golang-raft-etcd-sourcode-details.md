---
title: ETCD 源码解析
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd
description:
---

在上篇 [ETCD 示例源码](/post/golang-raft-etcd-example-sourcode-details.html) 中介绍了 ETCD 代码中 RAFT 相关的示例代码，接着介绍与 ETCD 相关的代码。

<!-- more -->

## 数据结构

简单介绍下一些常见的数据结构。

### type Storage interface

定义了存储的接口。

{% highlight go %}
type Storage interface {
    // InitialState returns the saved HardState and ConfState information.
    InitialState() (pb.HardState, pb.ConfState, error)
    // Entries returns a slice of log entries in the range [lo,hi).
    // MaxSize limits the total size of the log entries returned, but
    // Entries returns at least one entry if any.
    Entries(lo, hi, maxSize uint64) ([]pb.Entry, error)
    // Term returns the term of entry i, which must be in the range
    // [FirstIndex()-1, LastIndex()]. The term of the entry before
    // FirstIndex is retained for matching purposes even though the
    // rest of that entry may not be available.
    Term(i uint64) (uint64, error)
    // LastIndex returns the index of the last entry in the log.
    LastIndex() (uint64, error)
    // FirstIndex returns the index of the first log entry that is
    // possibly available via Entries (older entries have been incorporated
    // into the latest Snapshot; if storage only contains the dummy entry the
    // first log entry is not available).
    FirstIndex() (uint64, error)
    // Snapshot returns the most recent snapshot.
    // If snapshot is temporarily unavailable, it should return ErrSnapshotTemporarilyUnavailable,
    // so raft state machine could know that Storage needs some time to prepare
    // snapshot and call Snapshot later.
    Snapshot() (pb.Snapshot, error)
}
{% endhighlight %}

其中官方提供的 [Github Raft Example](https://github.com/coreos/etcd/tree/master/contrib/raftexample) 中使用的是库自带 MemoryStorage 。

### type Ready struct

对于这种 IO 网络密集型的应用，提高吞吐最好的手段就是批量操作，ETCD 与之相关的核心抽象就是 Ready 结构体。

{% highlight go %}
// Ready encapsulates the entries and messages that are ready to read,
// be saved to stable storage, committed or sent to other peers.
// All fields in Ready are read-only.
type Ready struct {
    // The current volatile state of a Node.
    // SoftState will be nil if there is no update.
    // It is not required to consume or store SoftState.
    *SoftState

    // The current state of a Node to be saved to stable storage BEFORE
    // Messages are sent.
    // HardState will be equal to empty state if there is no update.
    pb.HardState

    // ReadStates can be used for node to serve linearizable read requests locally
    // when its applied index is greater than the index in ReadState.
    // Note that the readState will be returned when raft receives msgReadIndex.
    // The returned is only valid for the request that requested to read.
    ReadStates []ReadState

    // Entries specifies entries to be saved to stable storage BEFORE
    // Messages are sent.
    Entries []pb.Entry

    // Snapshot specifies the snapshot to be saved to stable storage.
    Snapshot pb.Snapshot

    // CommittedEntries specifies entries to be committed to a
    // store/state-machine. These have previously been committed to stable
    // store.
    CommittedEntries []pb.Entry

    // Messages specifies outbound messages to be sent AFTER Entries are
    // committed to stable storage.
    // If it contains a MsgSnap message, the application MUST report back to raft
    // when the snapshot has been received or has failed by calling ReportSnapshot.
    Messages []pb.Message

    // MustSync indicates whether the HardState and Entries must be synchronously
    // written to disk or if an asynchronous write is permissible.
    MustSync bool
}
{% endhighlight %}

### type node struct

在 `raft/node.go` 中定义了 `type node struct` 对应的结构，一个 RAFT 结构通过 Node 表示各结点信息，该结构体内定义了各个管道，用于同步信息，下面会逐一遇到。

{% highlight go %}
type node struct {
    propc      chan pb.Message
    recvc      chan pb.Message
    confc      chan pb.ConfChange
    confstatec chan pb.ConfState
    readyc     chan Ready
    advancec   chan struct{}
    tickc      chan struct{}
    done       chan struct{}
    stop       chan struct{}
    status     chan chan Status
}
{% endhighlight %}

其实现，就是通过这些管道在 RAFT 实现与外部应用之间来传递各种消息。

### type raft struct

在 `raft/raft.go` 中定义了 `type raft struct` 结构，其中有两个关键函数指针 `tick` 和 `step`，在不同的状态时会调用不同的函数，例如 Follower 中使用 `tickElection()` 和 `stepFollower()` 。

{% highlight go %}
type raft struct {
	id uint64

	Term uint64
	Vote uint64

	readStates []ReadState

	// the log
	raftLog *raftLog

	maxInflight int
	maxMsgSize  uint64
	prs         map[uint64]*Progress

	state StateType

	votes map[uint64]bool

	msgs []pb.Message

	// the leader id
	lead uint64
	// leadTransferee is id of the leader transfer target when its value is not zero.
	// Follow the procedure defined in raft thesis 3.10.
	leadTransferee uint64
	// New configuration is ignored if there exists unapplied configuration.
	pendingConf bool

	readOnly *readOnly

	// number of ticks since it reached last electionTimeout when it is leader
	// or candidate.
	// number of ticks since it reached last electionTimeout or received a
	// valid message from current leader when it is a follower.
	electionElapsed int

	// number of ticks since it reached last heartbeatTimeout.
	// only leader keeps heartbeatElapsed.
	heartbeatElapsed int

	checkQuorum bool
	preVote     bool

	heartbeatTimeout int
	electionTimeout  int
	// randomizedElectionTimeout is a random number between
	// [electiontimeout, 2 * electiontimeout - 1]. It gets reset
	// when raft changes its state to follower or candidate.
	randomizedElectionTimeout int

	tick func()          // 两个重要的函数指针
	step stepFunc

	logger Logger
}
{% endhighlight %}

Node 代表了 etcd 中一个节点，是 RAFT 协议核心部分实现的代码，而在 EtcdServer 的应用层与之对应的是 raftNode ，两者一对一，raftNode 中有匿名嵌入了 node 。


## 整体架构

Etcd 服务端主要由几大组件构成，各部分介绍如下：

* `EtcdServer[etcdserver/server.go]` 主进程，直接或者间接包含了 raftNode、WAL、snapshotter 等多个核心组件，可以理解为一个容器。
* `raftNode[etcdserver/raft.go]` 对内部 RAFT 协议实现的封装，暴露简单的接口，用来保证写事务的集群一致性。

<!--
Store 管理维护Etcd数据库
Wal 管理事务日志
Snapshotter 负责数据快照，管理store数据库在内存中和磁盘上的相互转换。

raftNode除了负责集群间raft消息交互，还负责事务和快照的存储，保持数据一致性。
-->


### 处理流程

这里的采用的是异步状态机，基于 GoLang 的 Channel 机制，RAFT 状态机作为一个 Background Thread/Routine 运行，会通过 Channel 接收上层传来的消息，状态机处理完成之后，再通过 Ready() 接口返回给上层。

其中 `type Ready struct` 结构体封装了一批更新操作，包括了：

* `pb.HardState` 需要在发送消息前持久化的消息，包含当前节点见过的最大的 term，在这个 term 给谁投过票，已经当前节点知道的 commit index；
* `Messages`  需要广播给所有 peers 的消息；
* `CommittedEntries` 已经提交但是还没有apply到状态机的日志；
* `Snapshot` 需要持久化的快照。

库的使用者从 `type node struct` 结构体提供的 ready channel 中不断 pop 出一个个 Ready 进行处理，库使用者通过如下方法拿到 Ready channel 。

{% highlight go %}
func (n *node) Ready() <-chan Ready { return n.readyc }
{% endhighlight %}

应用需要对 Ready 的处理包括:

1. 将 HardState、Entries、Snapshot 持久化到 storage；
1. 将 Messages 非阻塞的广播给其他 peers；
1. 将 CommittedEntries (已经提交但是还没有应用的日志) 应用到状态机；
1. 如果发现 CommittedEntries 中有成员变更类型的 entry，则调用 node 的 `ApplyConfChange()` 方法让 node 知道；
1. 调用 `Node.Advance()` 告诉 raft node 这批状态更新处理完，状态已经演进了，可以给我下一批 Ready 让我处理了。

注意，上述的第 4 部分和 RAFT 论文中的内容有所区别，论文中只要节点收到了成员变更日志就应用，而这里实际需要等到日志提交之后才会应用。

## 启动流程

ETCD 服务器是通过 EtcdServer 结构抽象，对应了 `etcdserver/server.go` 中的代码，包含属性 `r raftNode`，代表 RAFT 集群中的一个节点，启动入口在 `etcdmain/main.go` 文件中。

<!--
在服务器启动过程中，会调用raftNode(etcdserver/raft.go)的start方法：
-->

{% highlight text %}
main()                                etcdmain/main.go
 |-checkSupportArch()
 |-startEtcdOrProxyV2()               etcdmain/etcd.go
   |-newConfig()
   |-setupLogging()
   |-startEtcd()
   | |-embed.StartEtcd()              embed/etcd.go
   |   |-startPeerListeners()
   |   |-startClientListeners()
   |   |-EtcdServer.ServerConfig()    生成新的配置
   |   |-EtcdServer.NewServer()       etcdserver/server.go正式启动RAFT服务<<<1>>>
   |   |-EtcdServer.Start()           开始启动服务
   |   | |-EtcdServer.start()
   |   |   |-wait.New()               新建WaitGroup组以及一些管道服务
   |   |   |-EtcdServer.run()         etcdserver/raft.go 启动应用层的处理协程<<<2>>>
   |   |-Etcd.servePeers()            启动集群内部通讯
   |   | |-etcdhttp.NewPeerHandler()  启动http服务
   |   | |-v3rpc.Server()             启动gRPC服务 api/v3rpc/grpc.go
   |   |   |-grpc.NewServer()         调用gRPC的接口创建
   |   |   |-pb.RegisterKVServer()    注册各种的服务，这里包含了多个
   |   |   |-pb.RegisterWatchServer()
   |   |-Etcd.serveClients()          启动协程处理客户请求
   |   |-Etcd.serveMetrics()
   |-notifySystemd()
   |-select()                         等待stopped
   |-osutil.Exit()
{% endhighlight %}

在标记 1 处会启动 RAFT 协议的核心部分，也就是 `node.run()[raft/node.go]` 。

在标记 2 处启动的是 ETCD 应用层的处理协程，对应了 `raftNode.start()[etcdserver/raft.go]` 。

这里基本上是大致的启动流程，主要是解析参数，设置日志，启动监听端口等，接下来就是其核心部分 `etcdserver.NewServer()` 。

### 启动RAFT

应用通过 `raft.StartNode()` 来启动 raft 中的一个副本，函数内部会通过启动一个 goroutine 运行。

{% highlight text %}
NewServer()                           etcdserver/server.go 通过配置创建一个新的EtcdServer对象，不同场景不同
 |-store.New()
 |-wal.Exist()
 |-restartNode()                      etcdserver/raft.go 已有WAL，直接根据SnapShot启动，最常见场景
 | |-readWAL()                        读取WAL
 | |-NewCluster()                     每个会对应一个新的集群配置
 | |-raft.RestartNode()               raft/node.go 真正做重启节点的函数
 |   |-newRaft()                      raft/raft.go 新建一个type raft struct对象
 |   | |-raft.becomeFollower()        成为Follower状态
 |   |-newNode()                      raft/node.go 新建一个type node struct对象
 |   |-node.run()                     raft/node.go RAFT协议运行的核心函数，会单独启动一个协程<<<1>>>
 |-NewAuthStore()
 |                                    <====会根据不同的启动场景执行相关任务
 |-startNode()                        新建一个节点，前提是没有WAL日志，且是新配置结点 etcdserver/raft.go
 | |-raft.NewMemoryStorage()
 | |-raft.StartNode()                 启动一个节点raft/node.go，开始node的处理过程<<<start>>>
 |   |-newRaft()                      创建RAFT对象raft/raft.go
 |   |-raft.becomeFollower()          这里会对关键对象初始化以及赋值，包括step=stepFollower r.tick=r.tickElection函数
 |   | |-raft.reset()                 开始启动时设置term为1
 |   | | |-raft.resetRandomizedElectionTimeout() 更新选举的随机超时时间
 |   |-raftLog.append()               将配置更新日志添加
 |   |-raft.addNode()
 |   |-newNode()                      新建节点
 |   |-node.run()                     raft/node.go 节点运行，会启动一个协程运行 <<<long running>>>
 |     |-newReady()                   新建type Ready对象
 |     |-raft.tick()                  等待n.tickc管道，这里实际就是在上面赋值的tickElection()函数
 |
 |-time.NewTicker()                   在通过&EtcdServer{}创建时新建tick时钟 etcdserver/server.go
{% endhighlight %}

启动的后台程序也就是 `node.run()`。

## 客户端发送请求

这里是通过 `clientv3` 发送数据，该端口使用的是 gRPC 通讯，关于客户端的使用方式，可以参考代码 clientv3 目录下的 example 示例，例如 `example_kv_test.go` 。

{% highlight text %}
clientv3.New()                 clientv3/client.go
 |-newClient()
 | |-Client{}                  示例化Client对象
 | |-newHealthBalancer()       etcd实现的负载均衡策略
 | |-Client.dial()             开始建立链接
 | | |-Client.dialSetupOpts()
 | | |-grpc.DialContext()      真正建立链接
 | |-NewCluster()              新建集群配置
 | |-NewKV()                   其入参是上述创建的Client
 |   |-RetryKVClient()         新建KV对象时指定了remote参数<<<1>>>
 |     |-NewKVClient()         调用proto生成的函数接口建立链接
 |-client.autoSync()           单独协程开启自动重连

cli.Put()                      实际上对应了kv.go中的实现
 |-kv.Put()                    kv.go 中的实现
   |-kv.Do()                   调用该函数实现，统一实现接口，根据类型调用不同的接口
     | <<<tPut>>>
     |-pb.PutRequest{}         构造proto中指定的请求
     | |-kv.remote.Put()       在如上新建客户端时，将kv.remote设置为了RetryKVClient()返回值
     |=retryKVClient.Put()     上述调用实际上就是该函数
       |-rkv.kc.Put()          最终的gRPC调用接口，发送请求并处理返回值
{% endhighlight %}

上述的最终调用，在外层会封装一个 `retryf()` 函数，也就是如果有异常会直接重试。

<!--
https://blog.csdn.net/zoumy3/article/details/79521190
https://yuerblog.cc/2017/12/12/etcd-v3-sdk-usage/

etcd v3 的简单使用示例
https://zhuanlan.zhihu.com/p/36719209
-->


## 服务端处理请求

服务器 RPC 接口的定义在 `etcdserver/etcdserverpb/rpc.proto` 文件中，对应了 `service KV` 中的定义，而真正的启动对应了 `api/v3rpc/grpc.go` 中的实现。

以 KV 存储为例，其对应了 `NewQuotaKVServer()` 中的实现，这里实际上是封装了一层，用来检查是否有足够的空间。

### Put

例如，对于 Put 请求，对应了该函数中的实现。

{% highlight text %}
quotaKVServer.Put() api/v3rpc/quota.go 首先检查是否满足需求
 |-quotoAlarm.check() 检查
 |-KVServer.Put() api/v3rpc/key.go 真正的处理请求
   |-checkPutRequest() 校验请求参数是否合法
   |-RaftKV.Put() etcdserver/v3_server.go 处理请求
   |=EtcdServer.Put() 实际调用的是该函数
   | |-raftRequest()
   |   |-raftRequestOnce()
   |     |-processInternalRaftRequestOnce() 真正开始处理请求
   |       |-context.WithTimeout() 创建超时的上下文信息
   |       |-raftNode.Propose() raft/node.go
   |         |-raftNode.step() 对于类型为MsgProp类型消息，向propc通道中传入数据
   |-header.fill() etcdserver/api/v3rpc/header.go填充响应的头部信息
{% endhighlight %}

此时，实际上已经将添加记录的请求发送到了 RAFT 协议的核心层处理。

### Range

没有操作单个 key 的方法，即使是读取单个 key，也是需要使用 Range 方法。

上述的 quota 检查实际上只针对了 Put Txn 操作，其它的请求，例如 Range 实际上会直接调用 `api/v3rpc/key.go` 中的实现。

{% highlight text %}
kvServer.Range() api/v3rpc/key.go
 |-checkRangeRequest()
 |-RaftKV.Range()
 |-header.fill()
{% endhighlight %}

## 日志复制

在 RAFT 协议中，整个集群所有变更都必须通过 Leader 发起，如上其入口为 `node.Propose()` 。

{% highlight go %}
func (n *node) Propose(ctx context.Context, data []byte) error {
	return n.step(ctx, pb.Message{Type: pb.MsgProp, Entries: []pb.Entry{{Data: data}}})
}
{% endhighlight %}

这里消息类型是 `pb.MsgProp` ，对于 leader 来说，实际上调用的是 `stepLeader()` 函数。

{% highlight go %}
case pb.MsgProp:
	r.appendEntry(m.Entries...)
	r.bcastAppend()
	return
{% endhighlight %}

## RAFT 核心处理

### 状态机简介

在 RAFT 协议实现的代码中，`node[raft/node.go]` 是其核心的实现，也是整个分布式算法的核心所在。

另外，通过 `raftNode[etcdserver/raft.go]` 对 node 进一步封装，只对 EtcdServer 暴露了 `startNode()`、`start()`、`apply()`、`processMessages()` 等少数几个接口。

其中核心部分是通过 `start()` 方法启动的一个协程，这里会等待从 readyc 通道上报的数据。

### 状态机处理

如上，在添加数据时，已经添加到了 propc 管道中，此时会触发 `node.run()[raft/node.go]` 中协程。

{% highlight text %}
node.run()                       raft/node.go 单独的协程调用
 |-newReady()                    获取已经就绪的数据，也就是msgs []pb.Message中的数据，保存到了rd.Messages中
 |-Ready.containsUpdates()       判断是否有更新，以决定是否将数据发送到readyc的管道中
 |-hasLeader()                   如果leader已经变化，那么需要获取最新的propc管道
 |                               等待propc获取数据
 |-raft.Step()                   raft/raft.go
   |-raft.step()                 这里是一个函数指针，不同状态调用函数有所区别
   |=stepLeader()                对于Leader来说，也就是同步到其它节点
   | |
   | |  <<<pb.MsgProp>>>
   | |-raft.appendEntry()        将日志添加到raftlog的unstable entry中，等待commit变成stable entry
   | | |                             放到storage中，最终变成snapshot
   | | |-raftLog.lastIndex()     raft/log.go 日志最新的ID，并对每个消息赋值ID
   | | |-raftLog.append()        将新的entry加到unstable entry中
   | | | |-unstable.truncateAndAppend()
   | | |-raft.getProgress().maybeUpdate() 这里更新了leader自己的Match
   | | |-raft.maybeCommit()      只增加了自己的commit，未收到其它节点的返回消息，此时不会更新commit index
   | |   |-raftLog.maybeCommit() 会读取raft.prs中的内容，也就是Progress
   | |     |-raftLog.commitTo()  修改commitIndex
   | |
   | |-raft.bcastAppend()        将entry广播到其它的节点，也就是日志复制
   | | |-raft.sendAppend()       构造pb.MsgApp类型的消息结构体开始发送
   | |   |-raft.send()
   | |     |-append()            添加到msgs []pb.Message中，这里相当于一个发送的缓冲区
   | |
   | |  <<<pb.MsgAppResp>>>
   | |-maybeUpdate()             从其它节点获取到的响应消息，更新本地计数
   | |-raft.maybeCommit()        判断是否提交成功，如果更新成功则广播
   | |-raft.bcastAppend()
   |
   |=stepFollower()              对于Follower来说
     |
     |  <<<pb.MsgProp>>>
     |-raft.send()               直接添加到msgs []pb.Message数组中并转发给Leader
     |
     |  <<<pb.MsgApp>>>
     |-raft.handleAppendEntries()
       |-raft.send()
{% endhighlight %}

注意，这里在处理时，readyc 和 advancec 只有一个是有效值。

{% highlight go %}
if advancec != nil { /* 如果 advance 不空，则把 readyc 置空 */
	readyc = nil
} else { /* 每次循环都会创建一个新的ready对象，其中包含了数据msgs */
	rd = newReady(r, prevSoftSt, prevHardSt)
	if rd.containsUpdates() { /* 如果raft.msgs中队列大小不为0，表示有数据发出 */
		readyc = n.readyc
	} else {
		readyc = nil
	}
}

if lead != r.lead {
	if r.hasLeader() {//当前raft节点r中lead不为空，表示已经存在leader
		if lead == None {
			r.logger.Infof("raft.node: %x elected leader %x at term %d", r.id, r.lead, r.Term)
		} else {
			r.logger.Infof("raft.node: %x changed leader from %x to %x at term %d", 
					    r.id, lead, r.lead, r.Term)
		}
		propc = n.propc
	} else {
		r.logger.Infof("raft.node: %x lost leader %x at term %d", r.id, lead, r.Term)
		propc = nil
	}
	lead = r.lead
}
{% endhighlight %}


在下个循环中，会通过 `newReady()` 读取数据，也就是 `msgs []pb.Message` 中的数据，并发送到 readyc 管道中。接着会触发消息的发送，也就是 `raftNode.start()[etcdserver/raft.go]` 中的处理。

{% highlight text %}
raftNode.start() etcdserver/raft.go 单独协程处理，包括发送消息
 |
 | <<<readyc>>>                这里会等待raft/node.go中node.Ready()返回的管道
 |-rd := <- r.Ready()          阻塞等待readyc管道中的消息，包括上述提交的数据
 |-apply{}                     构造apply对象，其中包括了已经提交的日志，SnapShot等
 |-updateCommittedIndex()
 | |-raftReadyHandler.updateCommittedIndex()
 | applyc<-ap                  添加到管道中，并等待提交完成
 |-transport.Send()            将数据，真正发送到对端
 |
 |-raftNode.processMessages()  会根据不同类型的消息进行一些异常的处理
 |-transport.Send()            rafthttp/transport.go 发送请求消息
 |-storage.Save()              这里同时会对日志以及SnapShot进行持久化处理
{% endhighlight %}





























### 消息发送

在消息通过 `append(r.msgs, m)` 添加到了发送缓冲区中之后，接着就是如何通过网络层发送数据。

在搜索 `r.msgs` 是，实际用的只有在 `newReady()` 函数中，也就是上述的处理协程中，对应了 `node.run()` 函数，此时会发送到 readyc 管道中。

其中，raft/node.go 中有如下的实现。

func (n *node) Ready() <-chan Ready { return n.readyc }

也就是是说，实际处理 readyc 请求是在 `raftNode.start()[etcdserver/raft.go]` 中。

















### 消息发送

一般在 `raft/raft.go` 文件中，会通过 `r.send()` 发送，也就是 `raft.send()` 发送消息时，例如，如下是处于 Follower 状态时的处理函数 `stepFollower()` 。

{% highlight go %}
func stepFollower(r *raft, m pb.Message) {
	switch m.Type {
	case pb.MsgProp:
		if r.lead == None {
			r.logger.Infof("%x no leader at term %d; dropping proposal", r.id, r.Term)
			return
		} else if r.disableProposalForwarding {
			r.logger.Infof("%x not forwarding to leader %x at term %d; dropping proposal",
				r.id, r.lead, r.Term)
			return
		}
		m.To = r.lead
		r.send(m)
	// ... ...
	}
}
{% endhighlight %}

在同一个文件中，最终会调用 `append(r.msgs, m)`，那么这个消息是在什么时候消费的呢？

在 `type node struct` 结构体中，存在一个 readyc 的管道。

{% highlight go %}
type node struct {
	readyc chan Ready
}
{% endhighlight %}

在 `raft/node.go` 中存在一个 `node.run()` 函数，会读取所有的消息，然后同时通过管道发送。<!-- ？？？如果没有消息更新，这里会阻塞吗？？？？-->

{% highlight go %}
func newReady(r *raft, prevSoftSt *SoftState, prevHardSt pb.HardState) Ready {
	rd := Ready{
	    Entries:          r.raftLog.unstableEntries(),
		CommittedEntries: r.raftLog.nextEnts(),
		Messages:         r.msgs,
	}
	... ...
}
{% endhighlight %}

也就是说，在 `node.go` 里 `node.run()` 中构建了 Ready 对象，对象里就包涵被赋值的 msgs，并最终写到 `node.readyc` 这个管道里，如下是对应这个 case 的实现：

{% highlight go %}
case readyc <- rd:
	r.msgs = nil
	r.readStates = nil
	advancec = n.advancec
{% endhighlight %}

这里的 msgs 已经读取过并写入到了管道中，直接设置为空，并会赋值 advancec，在 `etcdserver/raft.go` 的 `raftNode.start()` 中，会起一个单独的协程读取数据；其中读取的函数实现在 raft/node.go 中：

{% highlight go %}
func (n *node) Ready() <-chan Ready { return n.readyc }
{% endhighlight %}

应用层 (也就是etcd) 读取到的 Ready 里面包含了 Vote 消息，会调用网络层发送消息出去，并且调用 Advance() 。

<!--
### 消息接收处理

其它 node 接收到网络层消息后，会调用 raft.Step() 函数。

raft.Step()  raft/raft.go
 |-raft.becomeFollower() 如果本地的term小于消息的term，就把自己置为follower
 |-raft.send() 当接收到的消息term一致时，就返回voteRespMsg为其投票

voteRespMsg 的返回信息被之前的发送方接收到了之后，就会计算收到的选票数目是否大于所有 node 的一半，如果大于则自己成为 leader，否则将自己置为 follower；

stepCandidate() raft/raft.go
 |-[case myVoteRespType] 如果收到了投票返回的消息
   |-raft.poll() 检查是否满足多数派原则


case myVoteRespType:
    gr := r.poll(m.From, m.Type, !m.Reject)
    switch r.quorum() {
    case gr:
        if r.state == StatePreCandidate {
            r.campaign(campaignElection)
        } else {
            r.becomeLeader()
            r.bcastAppend()
        }
    case len(r.votes) - gr:
        r.becomeFollower(r.Term, None)
    }
在成为leader之后，和上面的两个角色一样的，最重要的是step被置为了stepLeader，具体stepLeader中涉及到的一些操作，更多的是下一个问题会用到，这里就不多说了。

func (r *raft) becomeLeader() {
    r.step = stepLeader
}
-->

在 `raft/node.go->run()` 函数中，是一个节点 (Node) 的主要处理过程，开始处于 Follower 状态，然后随着 `case <-n.tickc` 进行，开始进入选举。

<!--
## 代码走读
### 数据结构


在 campaign() 中的实现选举逻辑时，实际上实现了两个阶段 PreElection 和 Election 。？？？？

## 1. Leader选举

EtcdServer.Start()
 |-EtcdServer.start()
   |-EtcdServer.run()
     |-raftNode.start() 启动新的协程处理请求
       |-raftNode.Tick()  对于r.ticker.C管道的时钟处理
       |- 针对r.Ready()的请求

raftNode.start()

开始启动的时候，首先会设置为 Follower 状态，而且 term 为 1 。

当 node 初始化完成之后，通过 `node.run()` 开始运行，这里会启动单独的协程读取如上的管道。



raft.tickElection() 时钟处理函数，默认时间间隔为500ms raft/raft.go
 |-raft.Step() 如果超时，则开始发起选举，构造消息发送给自己，消息类型为MsgHup
   |-raft.campaign() 开始进入选举
     |-raft.becomeCandidate() 进入选举后状态由Follower转换为Candidate
	 | |-指针函数修改step=stepCandidate tick=tickElection
	 | |-raft.reset() 重置，同时term会增加1
     |-raft.send() 向每个节点发送voteMsg消息

-->

## Progress

RAFT 实现的内部，本身还维护了一个子状态机。

{% highlight text %}
                            +--------------------------------------------------------+
                            |                  send snapshot                         |
                            |                                                        |
                  +---------+----------+                                  +----------v---------+
              +--->       probe        |                                  |      snapshot      |
              |   |  max inflight = 1  <----------------------------------+  max inflight = 0  |
              |   +---------+----------+                                  +--------------------+
              |             |            1. snapshot success
              |             |               (next=snapshot.index + 1)
              |             |            2. snapshot failure
              |             |               (no change)
              |             |            3. receives msgAppResp(rej=false&&index>lastsnap.index)
              |             |               (match=m.index,next=match+1)
receives msgAppResp(rej=true)
(next=match+1)|             |
              |             |
              |             |
              |             |   receives msgAppResp(rej=false&&index>match)
              |             |   (match=m.index,next=match+1)
              |             |
              |             |
              |             |
              |   +---------v----------+
              |   |     replicate      |
              +---+  max inflight = n  |
                  +--------------------+
{% endhighlight %}

详细可以查看 [raft/design.md](https://github.com/coreos/etcd/blob/master/raft/design.md) 中的介绍，对于 Progress 实际上就是 Leader 维护的各个 Follower 的状态信息，总共分为三种状态：probe, replicate, snapshot 。

应该是 AppendEntries 接口的一种实现方式，为每个节点维护两个 Index 信息：A) matchIndex 已知服务器的最新 Index，如果还未确定则是 0；<!-- 作用是啥??????；--> B) nextIndex 用来标示需要从那个索引开始复制。那么 Leader 实际上就是将 nextIndex 到最新的日志复制到 Follower 节点。

<!--
如果多数派已经收到了请求，并进行了相关的处理，在响应之前主崩溃，那么新的日志可能在集群写入成功吗???????
-->



## 参考

两种不同的实现方式 [Github CoreOS-etcd](https://github.com/coreos/etcd)、[Github Hashicorp-raft](https://github.com/hashicorp/raft) 。

<!--
据说一个性能比lmdb好很多的存储引擎
https://github.com/leo-yuriev/libmdbx


RAFT论文的中文翻译
http://www.opscoder.info/ectd-raft-library.html
http://vlambda.com/wz_xberuk7dlD.html
http://chenneal.github.io/2017/03/16/phxpaxos%E6%BA%90%E7%A0%81%E9%98%85%E8%AF%BB%E4%B9%8B%E4%B8%80%EF%BC%9A%E8%B5%B0%E9%A9%AC%E8%A7%82%E8%8A%B1/
https://www.jianshu.com/p/ae462a2d49a8
https://www.jianshu.com/p/ae1031906ef4
http://blog.neverchanje.com/2017/01/30/etcd_raft_core/
http://www.cnblogs.com/foxmailed/p/7173137.html
https://www.jianshu.com/p/5aed73b288f7

https://zhuanlan.zhihu.com/p/27767675









##

ETCD V3 底层的存储引擎是 Bolt，也就是通过 KV 结构保存上报的数据，最新的项目代码可以参考 [coreos bbolt](https://github.com/coreos/bbolt)。

简单来说，通过 BoltDB 的 MVCC 保证单机数据一致性，通过 RAFT 保证集群数据的一致性。

### MVCC

在内存中维护了一个 BTree 结构，对应的结构体如下：

type treeIndex struct {
	sync.RWMutex
	tree *btree.BTree
}

这个树中的 Key 是用户传入的 Key ，而 Value 却不是用户传入的 Value。

关于 etcd 的版本信息有如下的特性：

* 每个事务有唯一事务ID(Main ID)，在全局范围内递增且不连续；
* 一个事务可以包含多个修改操作，如 PUT、DELETE 等，每个操作为一次 Revision，共享同一个 MainID；
* 一个事务内连续的多个操作从 0 开始递增编号，称为 SubID；
* 每个 Revision 通过 (MainID, SubID) 唯一标识。

其中 revision 通过 `type revision struct` 进行定义，而在内存索引中，每个用户的原始 key 会关联一个 keyIndex 结构，在该结构体中维护了多版本信息。

type keyIndex struct {
	key         []byte // 用户定义的原始Key
	modified    revision // 该Key的最后一次修改对应的Revision信息
	generations []generation // 保存的多版本信息
}

type generation struct {
	ver     int64
	created revision // 创建时的第一个版本号
	revs    []revision
}

当用户多次更新这个 key 时，对应的 revs 数组就会不断追加记录本次的 Revision 信息。

多版本中，每次操作都被以版本号的形式单独记录下来，而每个版本对应的数据则保存在 BoltDB 中。

在 BoltDB 存储时，会将每次的 Revision 作为 Key 进行序列化，首先从内存中获取对应的版本号，然后查询最终的数据。

在多版本控制中，一般会采用 compact 来压缩历史版本，即当历史版本到达一定数量时，会删除一些历史版本，只保存最近的一些版本，在 `keyIndex` 之前有相关的解析。

tombstone就是指delete删除key，一旦发生删除就会结束当前的generation，生成新的generation，小括号里的(t)标识tombstone。
compact(n)表示压缩掉revision.main <= n的所有历史版本，会发生一系列的删减操作，可以仔细观察上述流程。

#### 总结

内存中通过 BTree 维护是用户 `key -> keyIndex` 的映射，而 keyIndex 中维护了多版本的 Revision 信息，然后再通过 Revision 映射到磁盘 BoltDB 中的用户值。

## Watch机制

也就是时间通知机制，同样是基于 MVCC 多版本实现，客户端可以指定监听的版本，如果有历史版本数据，同时会推送当时的数据快照，后续的变化值同样会发送到客户端。

newWatchableStore() 会新建unsynced、synced两个newWatcherGroup对象
 |-syncWatchersLoop() 启动后台处理Watcher的协程
   |-watcherGroup.size() watcher_group.go 如果有未同步的客户端则调用下面函数同步，否则等待100ms
   |-syncWatchers() 在循环中不断调用该函数处理，如下是真正的处理过程
     |-watcherGroup.choose() 1. 选择一批未同步的客户端
       |-watcherGroup.chooseAll() 2. 返回最小的版本号，并删除一些重复的Wather

MVCC/Watcher介绍
https://yuerblog.cc/2017/12/10/principle-about-etcd-v3/

其中会有两个队列，分别是 sync 和 unsync ，后者表示还没有完成通过，在放到 sync 队列之前会先将 unsync 队列中的请求处理完。





https://www.compose.com/articles/utilizing-etcd3-with-go/









## Etcd VS. BoltDB


1. 客户端的请求通过 propc 管道传递给内部的 RAFT 协议层；
2. 收到集群内其它节点的响应后，对投票进行计数，如果超过半数则提交？？？？
3. 从 Ready() 中读取已经提交后的日志，并发送给 readyc 管道 node.run()[raft/node.go]；备注：readyc 处理完之后，会继续更新 advancec 管道。
4. 在 raftNode 中会阻塞等待 readyc 管道，当读取到之后，会构建一个 apply 对象，并再次传递给 applyc 管道；同时会阻塞等待处理完成。raftNode.start()[etcdserver/raft.go]
5. 接着调用 EtcdServer 中阻塞等待的协程，也就是 ap := <-s.r.apply() ，此时会直接调用后台将数据写入。

ETCD 会启动几个后台的协程。

node.run() raft/node.go


真正读取数据。

store.Get() store/store.go
 |-store.internalGet() 用于获取数据
 | |-walkFunc 函数指针，用来递归查找
 | | |-parent.Children[name] 从map中获取节点
 | |-store.walk() 遍历各个节点
 |-loadInternalNode() store/node_extern.go
   |-node.Read() store/node.go

flock / funlock / mmap / munmap

newBackend() mvcc/backend/backend.go
 |-bolt.Open()
 |-backend{} 实例化一个后端对象
 |-newBatchTxBuffered() 批量缓存对象
 |-backend.run() [协程]后台的批量刷新任务
   |-time.NewTimer()
   |-batchTx.CommitAndStop() 最后事件退出时保存数据
   |-batchTx.Commit() 周期性的提交数据
   |-time.Reset() 重置定时器


网络发送
状态机应用
WAL追加

storeTxnWrite.put() mvcc/kvstore_txn.go
 |-newRevBytes()
 |-revToBytes()
 |-kv.Marshal() 对数据序列化
 |-UnsafeSeqPut() mvcc/backend/batch_tx.go 真正的数据持久化
 |-kvindex.Put() 添加到key->revision索引

kvindex 实际上是内存中的 btree ，可以参考 mvcc/index.go 中的实现。


实际上 `type Ready struct` 封装了多种消息的更新，

type Ready struct {

// Messages specifies outbound messages to be sent AFTER Entries are
    // committed to stable storage.
    // If it contains a MsgSnap message, the application MUST report back to raft
    // when the snapshot has been received or has failed by calling ReportSnapshot.

	用来保存在提交持久化之后应该发送的消息
	Messages []pb.Message
}

## Backend 写入

EtcdServer.run() etcdserver/server.go
 |-raftStorage.Snapshot()
 |-NewFIFOScheduler() ???不清楚干嘛用的
 |
 | <<<applyc>>> 这里调用的是raftNode.apply()返回后的管道
 |-Schedule() 会新建一个FIFO调度队列处理如下的请求
 |-EtcdServer.applyAll()
   |-EtcdServer.applySnapshot()
   |-EtcdServer.applyEntries()
     |-EtcdServer.apply()
	   |-EtcdServer.applyEntryNormal()
         |-applyV3.Apply() etcdserver/apply.go 这里会根据操作类型调用特定的接口处理
           |-applyV3.Put()
             |-KV().Write()
               |-txn.Put()
                 |-tw.tx.UnsafeSeqPut()

也就是说，在 run() 协程中，会消费 applyc 管道中的数据，并持久化到磁盘中，那么 applyc 的数据是从哪里来的？


整体来说，这个库实现了raft协议核心的内容，比如append log的逻辑，选主逻辑，snapshot，成员变更等逻辑。需要明确的是：library没有实现消息的网络传输和接收，库只会把一些待发送的消息保存在内存中，用户自定义的网络传输层取出消息并发送出去，并且在网络接收端，需要调一个library的函数，用于将收到的消息传入library，后面会详细说明。同时，library定义了一个Storage接口，需要library的使用者自行实现。


客户端发起的状态更新请求首先都会被记录在日志中，待主节点将更新日志在集群多数节点之间完成同步以后，便将该日志项内容在状态机中进行应用，进而便完成了一次客户的更新请求。



ETCD-RAFT 核心库实际上没有实现日志的追加逻辑，WAL 需要应用来实现，重点讨论：

1. 应用如何调用 WAL 库完成日志追加；
2. WAL 库如何管理日志；
3. WAL 如何与协议核心相互配合完成日志内容的同步。

### WAL

相关的代码在 wal 目录下，用来处理日志的追加、日志文件的切换、日志的回放等操作。

日志只有 read 和 appending 两种方式，且两种模式不会同时出现，在所有老日志读取完成之后，会变为 appending 模式，此时只增加不能修改之前日志，通过 `type WAL struct` 表示一个日志。

WAL 在持久化时采用的是 protobuf 协议，对应了 wal/walpb/record.proto 中的格式，而其编码解码对应的实现为 wal/{decoder.go,encoder.go} 。



Create() wal/wal.go
 |-CreateDirAll() 创建临时目录

### RaftLog

实际上 RAFT 协议的核心工作是在集群节点之间复制日志，协议核心需要了解当前日志的复制情况，这个结构便是 `type raftLog struct`，其实现在 raft/log.go 文件中。

在 raftLog 结构体中记录了当前日志的状态。

type raftLog struct {
	storage Storage    // 最近一次snapshot之后所有稳定的日志
	unstable unstable  // 未提交的entries，日志缓存，用于集群各个节点间复制日志，最后持久化到存储中
	committed uint64   // 已经在集群内完成提交的最大索引值
	applied uint64     // 已经将日志应用到状态机的最近一次提交 applied<=committed
	logger Logger
}

type unstable struct {
   snapshot *pb.Snapshot
   entries []pb.Entry
   offset  uint64
}

unstable 在内存中使用数组维护所有的更新日志项，在 Leader 中保存了客户端的所有更新请求；在 Follower 中维护了从 Leader 节点复制后的日志项。

任何节点的日志都会先保存在 unstable 结构中，然后再由内部状态机将 unstable 维护的日志项交给应用层处理，并由应用层将日志项进行持久化并转发至系统其它节点。

注意：将日志项追加到 Storage 的动作是由应用完成的，而不是 raft 协议核心处理层。





















最终调用的是 `type Raft interface` 中实现的接口，实际上就是 `etcdserver/server.go` 或者示例 `contrib/raftexample/raft.go` 中的接口实现。

quotaKVServer.Put() api/v3rpc/quota.go 首先检查是否满足需求
 |-quotoAlarm.check() 检查
 |-kvServer.Put() api/v3rpc/key.go 真正的处理请求
   |-checkPutRequest() 校验请求参数是否合法
   |-RaftKV.Put() etcdserver/v3_server.go 处理请求
   |=EtcdServer.Put() 实际调用的是该函数，这里会重新构建一个Internal的Proto对象
   | |-raftRequest()
   |   |-raftRequestOnce()
   |     |-processInternalRaftRequestOnce() 真正开始处理请求，这里会完成阻塞等待处理完成
   |       |-reqIDGent.Next() 获取最新的一个消息ID
   |       |-Marshal() 执行序列化
   |       |-Wait.Register(id) 注册一个ID到一个全局的MAP中，实际上是一个管道
   |       |-context.WithTimeout() 创建超时的上下文信息
   |       |-raftNode.Propose() raft/node.go
   |       | |-raftNode.step() 对于类型为MsgProp类型消息，向propc通道中传入数据
   |       |-
   |       |-ctx.Done() 请求处理超时
   |
   |-header.fill() etcdserver/api/v3rpc/header.go填充响应的头部信息

注意，在上述发送请求的时候，实际上先完成了一次 proto 格式的转换，会将所有的操作统一转换为 `raft_internal.proto` 中定义的 `message InternalRaftRequest` 对象，例如 Put 操作，会将 `message PutRequest` 进行转换。


这里实际上有个同步机制，也就是在 `pkg/wait` 中的实现，简单来说，在提交完请求之后，会在这里的 map 中添加一条记录，同时返回一个管道，并阻塞在管道中。

在完成处理之后，也就是已经将数据 Apply 成功，会向该管道中写入 applyResult 对象。

对于后者是在 `applyEntryNormal()` 中实现。

在示例的代码中有 `blocks until accepted by raft state machine` 的一段注释，不过感觉这里只能确保数据发送到了状态机中，并不能确保提交成功。

node.Propose() raft/node.go

在收到 InternalRaftRequest 对象之后，是如何判断这里的消息类型的？

https://blog.csdn.net/zg_hover/article/details/81840556
http://xargin.com/about-beanstalkd/
https://segmentfault.com/a/1190000016067218














quotaKVServer.Put() api/v3rpc/quota.go 首先检查是否满足需求
 |-quotoAlarm.check() 检查
 |-kvServer.Put() api/v3rpc/key.go 真正的处理请求
   |-checkPutRequest() 校验请求参数是否合法
   |-RaftKV.Put() etcdserver/v3_server.go 处理请求
   |=EtcdServer.Put() 实际调用的是该函数，这里会重新构建一个Internal的Proto对象
   | |-raftRequest()
   |   |-raftRequestOnce()
   |     |-processInternalRaftRequestOnce() 真正开始处理请求，这里会完成阻塞等待处理完成
   |       |-reqIDGent.Next() 获取最新的一个消息ID
   |       |-Marshal() 执行序列化
   |       |-Wait.Register(id) 注册一个ID到一个全局的MAP中，实际上是一个管道
   |       |-context.WithTimeout() 创建超时的上下文信息
   |       |-raftNode.Propose() raft/node.go
   |       | |-raftNode.step() 对于类型为MsgProp类型消息，向propc通道中传入数据
   |       |-
   |       |-ctx.Done() 请求处理超时
   |
   |-header.fill() etcdserver/api/v3rpc/header.go填充响应的头部信息

注意，在上述发送请求的时候，实际上先完成了一次 proto 格式的转换，会将所有的操作统一转换为 `raft_internal.proto` 中定义的 `message InternalRaftRequest` 对象，例如 Put 操作，会将 `message PutRequest` 进行转换。


这里实际上有个同步机制，也就是在 `pkg/wait` 中的实现，简单来说，在提交完请求之后，会在这里的 map 中添加一条记录，同时返回一个管道，并阻塞在管道中。

在完成处理之后，也就是已经将数据 Apply 成功，会向该管道中写入 applyResult 对象。

对于后者是在 `applyEntryNormal()` 中实现。


在收到 InternalRaftRequest 对象之后，是如何判断这里的消息类型的？

在提交完之后，

linearizableReadLoop


## Pre-Vote 算法

RAFT 依赖于一个特性，集群中所有的节点总是使用它所观察到的当前最大 Term 。一般 term id 使用的是 64-bits ，正常很难会溢出，当在修复一个故障节点后重新加入集群，即使之前的集群是正常的，此时的 term 仍然会加 1 。

例如，一个集群有三个节点，其中一个节点因为网络被隔离，每次选举超时都会增加 Term 值，因为被隔离，始终无法完成选举。









https://www.jianshu.com/p/27329f87c104
https://www.jianshu.com/p/21acb670ccf1
https://zhuanlan.zhihu.com/p/29865583

http://blog.sina.com.cn/s/blog_4b146a9c0102yml3.html
http://www.ituring.com.cn/book/tupubarticle/16510
https://www.jianshu.com/p/ae1031906ef4
https://blog.csdn.net/xxb249/article/details/80779577
https://www.cnblogs.com/foxmailed/p/7161878.html

https://www.jianshu.com/p/ef1ac201f685
https://bbs.huaweicloud.com/blogs/f65bc75d3ba811e89fc57ca23e93a89f
-->


{% highlight text %}
{% endhighlight %}
