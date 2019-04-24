---
title: ETCD 简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd
description: Etcd 是一个分布式可靠的键值存储系统，提供了与 ZooKeeper 相似的功能，通过 GoLang 开发而非 Java ，采用 RAFT 算法而非 PAXOS 算法。相比来所，etcd 的安装使用更加简单有效。
---

Etcd 是一个分布式可靠的键值存储系统，提供了与 ZooKeeper 相似的功能，通过 GoLang 开发而非 Java ，采用 RAFT 算法而非 PAXOS 算法。

相比来所，etcd 的安装使用更加简单有效。

<!-- more -->

![ectd logo]({{ site.url }}/images/databases/raft/etcd-horizontal-color.svg "etcd logo"){: .pull-center width="60%" }

## 简介

A distributed, reliable key-value store for the most critical data of a distributed system.

严格来说，ETCD 主要用于保存一些元数据信息，一般小于 1GB 对大于 1GB 的可以使用新型的分布式数据库，例如 TiDB 等，通常适用于 CP 场景。

## 安装

可以直接从 [github release](https://github.com/coreos/etcd/releases) 下载非源码包，也就是已经编译好的二进制包，一般包括了 etcd + etcdctl 。

### 源码安装

下载 ectd 源码构建，在源码中，实际上已经包含了工程所使用的库，在编译时可以直接修改 build 脚本，例如对于 raftexample 的编译，在该脚本中会设置一堆的环境变量，以引用本项目中的三方库。

{% highlight text %}
----- 需要go编译器支持，设置好GOPATH环境变量
$ go version
$ echo $GOPATH

----- 新建目录并下载代码，并编译
$ mkdir -p $GOPATH/src/github.com/coreos
$ cd $GOPATH/src/github.com/coreos
$ git clone https://github.com/coreos/etcd.git
$ cd etcd && git checkout v3.1.0
$ ./build
$ ./bin/etcd
{% endhighlight %}

### 单机单进程测试

启动单进程服务，并进行测试。

{% highlight text %}
----- 启动单个本地进程，会监听127.1:2379端口
$ ./etcd

----- 使用API v3版本，并测试添加获取参数
$ export ETCDCTL_API=3
$ ./etcdctl put foo bar
OK
$ ./etcdctl get foo
foo
bar

$ ./etcdctl --write-out=table --endpoints=localhost:2379 member list

----- 只打印值信息，不打印key
$ ./etcdctl get foo --print-value-only
bar
----- 打印十六进制格式
$ ./etcdctl get foo --hex
\x66\x6f\x6f
\x62\x61\x72
----- 指定范围为foo~foo3
$ ./etcdctl get foo foo3
foo
foo1
foo2
foo3
----- 指定前缀，且只显示前两个
$ ./etcdctl get --prefix --limit=2 foo
foo
foo1
{% endhighlight %}



<!--
--name
  集群中节点名，可区分且不重复即可；
--listen-peer-urls
监听的用于节点之间通信的url，可监听多个，集群内部将通过这些url进行数据交互(如选举，数据同步等)
--initial-advertise-peer-urls
建议用于节点之间通信的url，节点间将以该值进行通信。
--listen-client-urls
监听的用于客户端通信的url,同样可以监听多个。
--advertise-client-urls
建议使用的客户端通信url,该值用于etcd代理或etcd成员与etcd节点通信。
--initial-cluster-token etcd-cluster-1
节点的token值，设置该值后集群将生成唯一id,并为每个节点也生成唯一id,当使用相同配置文件再启动一个集群时，只要该token值不一样，etcd集群就不会相互影响。
--initial-cluster
也就是集群中所有的initial-advertise-peer-urls 的合集
--initial-cluster-state new
新建集群的标志

./etcd --name infra0 --initial-advertise-peer-urls http://10.0.1.111:2380 \
  --listen-peer-urls http://10.0.1.111:2380 \
  --listen-client-urls http://10.0.1.111:2379,http://127.0.0.1:2379 \
  --advertise-client-urls http://10.0.1.111:2379 \
  --initial-cluster-token etcd-cluster-1 \
  --initial-cluster infra0=http://10.0.1.111:2380,infra1=http://10.0.1.109:2380,infra2=http://10.0.1.110:2380 \
  --initial-cluster-state new

./etcd --name infra1 --initial-advertise-peer-urls http://10.0.1.109:2380 \
  --listen-peer-urls http://10.0.1.109:2380 \
  --listen-client-urls http://10.0.1.109:2379,http://127.0.0.1:2379 \
  --advertise-client-urls http://10.0.1.109:2379 \
  --initial-cluster-token etcd-cluster-1 \
  --initial-cluster infra0=http://10.0.1.111:2380,infra1=http://10.0.1.109:2380,infra2=http://10.0.1.110:2380 \
  --initial-cluster-state new

./etcd --name infra2 --initial-advertise-peer-urls http://10.0.1.110:2380 \
  --listen-peer-urls http://10.0.1.110:2380 \
  --listen-client-urls http://10.0.1.110:2379,http://127.0.0.1:2379 \
  --advertise-client-urls http://10.0.1.110:2379 \
  --initial-cluster-token etcd-cluster-1 \
  --initial-cluster infra0=http://10.0.1.111:2380,infra1=http://10.0.1.109:2380,infra2=http://10.0.1.110:2380 \
  --initial-cluster-state new

按如上配置分别启动集群，启动集群后，将会进入集群选举状态，若出现大量超时，则需要检查主机的防火墙是否关闭，或主机之间是否能通过2380端口通信，集群建立后通过以下命令检查集群状态。

数据持久化基于多版本并发控制，


性能压测的核心指标包括了：延迟(Latency)、吞吐量(Throughput)；延迟表示完成一次请求的处理时间；吞吐量表示某个时间段内完成的请求数。

一般来说，当吞吐量增加增加时，对应的延迟也会同样增加；低负载时 1ms 以内可以完成一次请求，高负载时可以完成 3W/s 的请求。对于 ETCD 来说，会涉及到一致性以及持久化的问题，所以其性能与网络带宽、磁盘IO的性能紧密相关。

其底层的存储基于 BoltDB，一个基于 GoLang 的 KV 数据库，支持 ACID 特性，与 lmdb 相似。

https://coreos.com/etcd/docs/latest/op-guide/performance.html
-->


<!--
configuration management, service discovery, and coordinating distributed work. Many organizations use etcd to implement production systems such as container schedulers, service discovery services, and distributed data storage. Common distributed patterns using etcd include leader election, distributed locks, and monitoring machine liveness.
-->

### 单机集群测试

在搭建本地集群时，可以直接使用 goreman 工具，默认使用的是当前目录下的 Procfile 配置文件，运行前需要确保配置正确。

{% highlight text %}
----- 检查配置是否合法
$ goreman check
----- 启动，或者指定配置文件启动
$ goreman start
$ goreman -f MyProcfile start
----- 查看当前的状态
$ goreman run status
----- 停止、启动、重启某个进程(stop start restart)
$ goreman run stop PROCESS_NAME
{% endhighlight %}

简单来说，直接通过 `goreman start` 启动即可，此时会在当前目录下生成 `infra{1,2,3}.etcd` 三个目录，用于保存各个进程的信息。

## API

实际上 API 基本上决定了 etcd 提供了哪些服务，通过 HTTP API 对外提供服务，这种接口更方便各种语言对接，命令行可以使用 httpie 或者 curl 调用。

数据按照树形的结构组织，类似于 Linux 的文件系统，也有目录和文件的区别，不过一般被称为 nodes，其中数据相关的 endpoint 都是以 `/v2/keys` 开头 (v2 表示当前 API 的版本)，比如 `/v2/keys/names/cizixs` 。

要创建一个值，只要使用 PUT 方法在对应的 url endpoint 设置就行。如果对应的 key 已经存在， PUT 也会对 key 进行更新。

### CURD

{% highlight text %}
----- 不存在则创建，否则修改，当超过TTL后，会自动删除
http PUT http://127.0.0.1:2379/v2/keys/message value=="hello, etcd" ttl==5
http GET http://127.0.0.1:2379/v2/keys/message
http DELETE http://127.0.0.1:2379/v2/keys/message
{% endhighlight %}

在创建 key 的时候，如果它所在路径的目录不存在，会自动被创建，所以在多数情况下我们不需要关心目录的创建，如果要单独创建一个目录可以指定参数 `dir=true`。

{% highlight text %}
http PUT http://127.0.0.1:2379/v2/keys/anotherdir dir==true
{% endhighlight %}

注意，ECTD 提供了类似 Linux 中 `.` 开头的隐藏机制，以 `_` 开头的节点也是默认隐藏的，不会在列出目录的时候显示，只有知道隐藏节点的完整路径，才能够访问它的信息。


### 监听机制

通过监听机制，可以在某个 key 发生变化时，通知对应的客户端，主要用于服务发现，集群中有信息更新时可以被及时发现并作出相应处理。

{% highlight text %}
http http://127.0.0.1:2379/v2/keys/foo wait==true
{% endhighlight %}

使用 `recursive=true` 参数，可以用来监听某个目录。

### 比较更新

在分布式环境中，需要解决多个客户端的竞争问题，通过 etcd 提供的原子操作 CompareAndSwap (CAS)，可以很容易实现分布式锁。简单来说，这个命令只有在客户端提供的条件成立的情况下才会更新对应的值。

{% highlight text %}
http PUT http://127.0.0.1:2379/v2/keys/foo prevValue==bar value==changed
{% endhighlight %}

只有当之前的值为 bar 时，才会将其更新成 changed 。

### 比较删除

同样是原子操作，只有在客户端提供的条件成立的情况下，才会执行删除操作；支持 prevValue 和 prevIndex 两种条件检查，没有 prevExist，因为删除不存在的值本身就会报错。

{% highlight text %}
http DELETE http://127.0.0.1:2379/v2/keys/foo prevValue==bar
{% endhighlight %}

### 监控集群

Etcd 还保存了集群的数据信息，包括节点之间的网络信息，操作的统计信息。

<!--
/v2/stats/leader  会返回集群中 leader 的信息，以及 followers 的基本信息
/v2/stats/self 会返回当前节点的信息
/v2/state/store：会返回各种命令的统计信息
-->

### 成员管理

在 `/v2/members` 下保存着集群中各个成员的信息。

<!--
/version   获取版本服务器以及集群的版本号
-->


## 常见操作


### etcdctl

这个实际上是封装了 HTTP 请求的一个客户端，用于更方便的与服务端进行交互。

{% highlight text %}
----- 设置一个key的值
$ etcdctl set /message "hello, etcd"
hello, etcd

----- 获取key的值
$ etcdctl get /message
hello, etcd

----- 获取key值的同时，显示更详细的元数据信息
$ etcdctl -o extended get /message
Key: /message
Created-Index: 1073
Modified-Index: 1073
TTL: 0
Index: 1073

hello, etcd

----- 如果获取的key不存在，则会直接报错
$ etcdctl get /notexist
Error:  100: Key not found (/notexist) [1048]

----- 设置key的ttl，过期后会被自动删除
$ etcdctl set /tempkey "gone with wind" --ttl 5
gone with wind

----- 如果key的值是"hello, etcd"，就把它替换为"goodbye, etcd"
$ etcdctl set --swap-with-value "hello, world" /message "goodbye, etcd"
Error:  101: Compare failed ([hello, world != hello, etcd]) [1050]
$ etcdctl set --swap-with-value "hello, etcd" /message "goodbye, etcd"
goodbye, etcd

----- 仅当key不存在时创建
$ etcdctl mk /foo bar
bar
$ etcdctl mk /foo bar
Error:  105: Key already exists (/foo) [1052]

----- 自动创建排序的key
$ etcdctl mk --in-order /queue job1
job1
$ etcdctl mk --in-order /queue job2
job2
$ etcdctl ls --sort /queue
/queue/00000000000000001053
/queue/00000000000000001054

----- 更新key的值或者ttl，只有当key已经存在的时候才会生效，否则报错
$ etcdctl update /message "I'am changed"
I'am changed
$ etcdctl get /message
I'am changed
$ etcdctl update /notexist "I'am changed"
Error:  100: Key not found (/notexist) [1055]
$ etcdctl update --ttl 3 /message "I'am changed"
I'am changed
$ etcdctl get /message
Error:  100: Key not found (/message) [1057]

----- 删除某个key
$ etcdctl mk /foo bar
bar
$ etcdctl rm /foo
PrevNode.Value: bar
$ etcdctl get /foo
Error:  100: Key not found (/foo) [1062]

----- 只有当key的值匹配的时候，才进行删除
$ etcdctl mk /foo bar
bar
$ etcdctl rm --with-value wrong /foo
Error:  101: Compare failed ([wrong != bar]) [1063]
$ etcdctl rm --with-value bar /foo

----- 创建一个目录
$ etcdctl mkdir /dir

----- 删除空目录
$ etcdctl mkdir /dir/subdir/
$ etcdctl rmdir /dir/subdir/

----- 删除非空目录
$ etcdctl rmdir /dir
Error:  108: Directory not empty (/dir) [1071]
$ etcdctl rm --recursive /dir

----- 列出目录的内容
$ etcdctl ls /
/queue
/anotherdir
/message

----- 递归列出目录的内容
$ etcdctl ls --recursive /
/anotherdir
/message
/queue
/queue/00000000000000001053
/queue/00000000000000001054

----- 监听某个key，当key改变的时候会打印出变化
$ etcdctl watch /message
changed

----- 监听某个目录，当目录中任何node改变的时候，都会打印出来
$ etcdctl watch --recursive /
[set] /message
changed

----- 一直监听，除非CTRL + C导致退出监听
$ etcdctl watch --forever /message
new value
chaned again
Wola

----- 监听目录，并在发生变化的时候执行一个命令
$ etcdctl exec-watch --recursive / -- sh -c "echo change detected."
change detected.
change detected.

----- 检查集群的健康状态
$ etcdctl cluster-health

----- 查看集群的成员列表
$ etcdctl member list
{% endhighlight %}

**注意** 默认只保存了 1000 个历史事件，所以不适合有大量更新操作的场景，这样会导致数据的丢失，其使用的典型应用场景是配置管理和服务发现，这些场景都是读多写少的。

### ClientV3

在 ETCD 的源码目录下保存了一个 clientv3 的代码，详细可以参考 [ETCD ClientV3](https://github.com/coreos/etcd/tree/master/clientv3) 。

#### etcdctl V3

{% highlight text %}
----- 使用V3版本需要提前设置环境变量，否则etcdctl --version查看
$ ETCDCTL_API=3 ./etcdctl version
etcdctl version: 3.3.1
API version: 2

----- 查看当前集群的列表，默认使用本地2379端口，也可以通过参数指定
$ ETCDCTL_API=3 ./etcdctl member list
$ ETCDCTL_API=3 ./etcdctl --endpoints=127.0.0.1:2379,127.0.0.1:22379,127.0.0.1:32379 member list

----- CURD，可以指定输出格式、前缀匹配
$ ETCDCTL_API=3 ./etcdctl put foo "Hello World!"
$ ETCDCTL_API=3 ./etcdctl get foo
$ ETCDCTL_API=3 ./etcdctl --write-out="json" get foo
$ ETCDCTL_API=3 ./etcdctl --prefix get foo
$ ETCDCTL_API=3 ./etcdctl --prefix del foo

----- 查看集群状态
$ ETCDCTL_API=3 ./etcdctl --write-out=table endpoint status
$ ETCDCTL_API=3 ./etcdctl endpoint health

----- 管理集群成员add remove update list
$ ETCDCTL_API=3 ./etcdctl --write-out=table member list

----- 查看告警
$ ETCDCTL_API=3 ./etcdctl alarm list
{% endhighlight %}

### 压测

在源码中内置了一个压测工具 `tools/benchmark` ，类似于 raftexample ，同样可以通过修改 `build` 文件编译。

详细的使用方法可以查看源码中的文档 [Github op-guide performance](https://github.com/coreos/etcd/blob/master/Documentation/op-guide/performance.md) 。

{% highlight text %}
$ go build -o "${out}/benchmark" ${REPO_PATH}/tools/benchmark || return
{% endhighlight %}

{% highlight text %}
----- 可以先查看当前集群的状态
$ ETCDCTL_API=3 ./etcdctl --endpoints=127.0.0.1:2379,127.0.0.1:22379,127.0.0.1:32379 \
     --write-out=table endpoint status

$ ./benchmark --endpoints=127.0.0.1:2379 --target-leader --conns=1 --clients=1 \
	put --key-size=8 --sequential-keys --total=10000 --val-size=256
{% endhighlight %}

<!--
https://indico.cern.ch/event/560399/contributions/2262460/attachments/1318051/1975404/slides.pdf
-->

## 参考


<!--
https://tonydeng.github.io/2015/10/19/etcd-application-scenarios/
http://jolestar.com/etcd-architecture/
http://www.infoq.com/cn/articles/etcd-interpretation-application-scenario-implement-principle

场景一：服务发现（Service Discovery）
场景二：消息发布与订阅
场景三：负载均衡
场景四：分布式通知与协调
场景五：分布式锁、分布式队列
场景六：集群监控与Leader竞选

EtcdServer.start()

startPeer()




mini transaction支持原子性比较多个键值并且操作多个键值。之前的CompareAndSwap实际上一个针对单个key的mini transaction。一个简单的例子是 Tx(compare: A=1 && B=2, success: C = 3, D = 3, fail: C = 0, D = 0)。当etcd收到这条transcation请求，etcd会原子性的判断A和B当前的值和期待的值。如果判断成功，C和D的值会被设置为3。

-I 只显示头
-i 显示头以及返回信息

curl -i http://127.0.0.1:2379/version
curl -i http://127.0.0.1:2380/members

ETCD V3 对外提供的 API 接口保存在 `etcdserver/api/etcdhttp` 目录下，常见的 URI 包括：
/debug/vars 获取调试信息，包括启动参数、资源使用等
/version 当前版本信息
/health 服务器的健康状态

V2 VS. V3
https://www.compose.com/articles/etcd2to3-new-apis-and-new-possibilities/
https://blog.gopheracademy.com/advent-2015/etcd-distributed-key-value-store-with-grpc-http2/
https://github.com/go-up/go-example
https://doc.oschina.net/grpc?t=60133
https://grpc.io/docs/quickstart/go.html

V2 版本只提供了基本的原子操作 CAS(Compare And Swap)，并在此基础上实现分布式锁；在 V3 上则实现了 MVCC 事务模型，抛弃了原来的原子操作。

那么其并发事务的隔离模型是怎样的？如何处理提交时的冲突？

实际上是直接把 MVCC 的版本机制暴露给了用户，在事务提交冲突时完全由用户控制是回滚还是忽略冲突直接提交，从而给用户以最大的灵活性，这也就是 STM 的代码。

software transactional memory

Serializable reads

Linearized Reads

如果要保证线性读，那么客户端需要从主上读取数据。

## 一致性模型

简单来说，可以根据不同的场景定义模型，从而使写的程序可预测。

Strong consistency models，一步步介绍一致性模型
https://aphyr.com/posts/313-strong-consistency-models

## 参考
Serializability and Distributed Software Transactional Memory with etcd3
https://coreos.com/blog/transactional-memory-with-etcd3.html
http://lday.me/2017/02/01/0003_seri-stm-etcd3/


相比 V2 而言 V3 提供了 gRPC 通讯，对于不支持 gRPC 的语言，etcd 提供 JSON 的 grpc-gateway 网关，作为 RESTful 代理，翻译 HTTP/JSON 请求为 gRPC 消息。

https://leonlee110.github.io/kubernetes/2018/03/31/learning-etcd-by-code-1

embed.StartEtcd()
EtcdServer.Start()
EtcdServer.servePeers()
 |-serve()
   |-grpc.NewServer() 新建一个gRPC的服务器etcdserver/api/v3rpc/grpc.go
EtcdServer.serveClients()



在 `type Node interface` 中定义了一些与 RAFT 核心相关的接口函数，

Propose() 用来提交日志
Ready() 返回一个Ready管道，表示已经提交的日志

在 raft/node.go 中会启动一个后台程序，对应了 `func (n *node) run(r *raft)` 函数，

在 ETCD RAFT 的实现中有两个核心的数据结构 `type node struct` 以及 `type raft struct` ，前者定义了用于 RAFT 核心与应用层的通讯的通讯管道，后者则保存了 RAFT 协议中需要保存的状态信息。


在 etcdserver/server.go 文件中定义了 type EtcdServer struct 结构体，保存了与 ETCD 的服务端相关的设置。注意，这里涉及到了一堆的继承关系，EtcdServer -> raftNodeConfig -> raft.Node ，也就是 Propose 对应了 type Node interface 中的函数接口。



processInternalRaftRequestOnce() V3接口提交日志
 |-EtcdServer.r.Propose() 实际上是raftNode中


数据的提交分成了 4 步：

1. 数据写入到Leader的本地存储，一般是内存；
2. Leader向各个Follower同时开始发送数据，并判断是否达到多数派提交成功；
3. Leader在提交成功后发起Apply请求，将日志应用到Leader所在的状态机。
4. Follower接收到Apply请求之后，同时将日志应用到Follower所在的状态机。

继任Leader是否可以将已经复制超过半数的 log 提交掉？不能，

目前还有疑问???????

如果做批量发送？批量的策略是什么？通过Ready对象实现；目前还没有确认是批量N条还是批量N秒内的数据
如何判断是否在集群中持久化成功？正常来说，commit成功之后就可以返回给客户端在集群中已经提交成功。
如果多数派的数据永久丢失，那么如何恢复？实际上在正式的论文中没有讨论，在FM-RAFT中有相关的介绍，也就是通过InitializeCluster接口
怎样定义一个节点是否是健康的？对于Leader？对于Follower可以周期接收到从Leader中发送的心跳信息，可以提供用户只读的是lastApplied的数据，也就是已经持久化到状态机的数据。
Term溢出(TermID一般是无符号整形)会导致什么问题？
https://coolshell.cn/articles/11466.html
https://developer.apple.com/library/content/documentation/Security/Conceptual/SecureCodingGuide/Introduction.html

## Lexical Bytes-Order

也可以称为 Alphabetical Order，另外还有 Numerical Order，例如 1 10 2 是属于前者。



一致性区分
https://github.com/coreos/etcd/blob/master/Documentation/learning/api_guarantees.md


http://www.ituring.com.cn/book/tupubarticle/16510
https://coreos.com/blog/announcing-etcd-3.3

系统调用的错误注入
https://lrita.github.io/2017/06/27/systemtap-inject-syscall-error/

当前系统信息收集
https://github.com/coreos/mayday
分布式的测试工具
https://github.com/coreos/dbtester


自习周报：CoreOS 的黑魔法
https://zhuanlan.zhihu.com/p/29882654







Linearizability 用来保证针对某个对象的一系列操作在墙上时间 (wall-clock) 是有序的，通常针对的是分布式系统。也就是说，针对多个节点可以完成原子操作，例如写完成后，其它节点可以立即读取到当前的状态。

Serializability 保证一系列针对多个对象的操作与针对对象的序列操作是相同的，通常是针对数据库的最高级别操作。

Strict Serializability 实际上是结合了上述的两种方式。

Linearizable read requests go through a quorum of cluster members for consensus to fetch the most recent data. 
Serializable read requests are cheaper than linearizable reads since they are served by any single etcd member, instead of a quorum of members, in exchange for possibly serving stale data.


https://aphyr.com/posts/313-strong-consistency-models







## Braft

braft 库本身踩的坑倒不多，更多的是库的使用过程中踩的坑：

on_snapshot_load 的时候没有清空状态机导致状态数据错乱

on_apply 的时候因为一些随机算法或者是因素导致主从执行结果不一致

apply 的时候卡住了，切从又切成主，这个过程中这条数据被其他节点成功 apply 了，就会导致 log 被正常的执行了两遍

on_leader_stop 的时候 leader 上的一些任务没有 cancel 掉导致 job 的下游节点出错；

这里面说明一下 braft 的测试情况，主要分为三部分：test 目录下面的 unit test；jepsen 目录下的 atomic example 的 jepsen 测试；分布式存储业务系统的压力和异常测试集群，在上百台服务器上注入类似 jepsen 的进程 kill/stop、网络划分、节点间单通、文件系统读写出错等异常。

分布式线性测试
https://github.com/anishathalye/porcupine
https://www.jianshu.com/p/2e65e6f37c76
https://github.com/jepsen-io/jepsen

### Config

删除节点
$ curl -L http://127.0.0.1:12380/3 -XDELETE

server1: ./bin/siteserver --id 1 --cluster http://127.0.0.1:12379,http://127.0.0.1:22379,http://127.0.0.1:32379 --port 12380 --grpc 127.0.0.1:50050
server2: ./bin/siteserver --id 2 --cluster http://127.0.0.1:12379,http://127.0.0.1:22379,http://127.0.0.1:32379 --port 22380 --grpc 127.0.0.1:50051
server3: ./bin/siteserver --id 3 --cluster http://127.0.0.1:12379,http://127.0.0.1:22379,http://127.0.0.1:32379 --port 32380 --grpc 127.0.0.1:50052

lost the TCP streaming connection with peer

ETCDCTL_API=3 ./etcdctl --write-out=table --endpoints=127.0.0.1:15379,127.0.0.1:25379,127.0.0.1:35379 endpoint status --cluster
ETCDCTL_API=3 ./etcdctl --endpoints=127.0.0.1:15379,127.0.0.1:25379,127.0.0.1:35379 member list

curl -L http://127.0.0.1:33380/66d2723d-9a5f-4132-ac26-04811e40c178

在读取 WAL 时，会将回放后的日志数据添加到 ents[] 数组中，而在 `serveChannels()` 函数中获取数据时实际上日志在 `CommittedEntries[]` 中。。。。

tail
curl -L http://127.0.0.1:33580/1412cfd6-adc9-4c3f-b1aa-bde1ebb40279 -XPUT -d '1412cfd6-adc9-4c3f-b1aa-bde1ebb40279-hello'

head
curl -L http://127.0.0.1:33580/66d2723d-9a5f-4132-ac26-04811e40c178

## 如果存在Snapshot的时候，实际上
##### 如果发生分区，如何判断当前主机是否在多数派集群中。
## 当前节点无法感知到状态信息

V2 VS. V3

### RPC

通过 gRPC 通信替换原有的 HTTP 方式，相比来说其报文解析速度更快，允许多路复用，而非每次请求建立一个链接。

### Leases

在 V2 中，每个 Key 会同时携带一个 TTL ，用来标示过期时间。为了防止到期自动删除，客户端需要定期刷新，这样就会导致即使是一个空集群，仍然会有定期的刷新流量。

而在 V3 中，通过 Lease 替换掉了 TTL ，每个 Key 可以绑定到 Lease 上，这样可以降低同步的网络带宽。

https://coreos.com/blog/etcd3-a-new-etcd.html
https://zhuanlan.zhihu.com/p/31050303
https://zhuanlan.zhihu.com/p/31118381
http://masutangu.com/2018/07/etcd-raft-note-3/
http://masutangu.com/2018/07/etcd-raft-note-4/

一个P2P的传输实现
https://github.com/Librevault/librevault
https://github.com/syncthing/syncthing
https://github.com/bittorrent

## 模块

由于 GoLang 的特点，其内部的通讯大部分使用的是

Client 客户端，接收用户发送的请求，包括了V2和V3两个版本
type raftNode struct[etcdserver/raft.go] applyc msgSnapC readStateC 管道，这个结构体里的核心就是如下的Config配置
    type raftNodeConfig struct[etcdserver/raft.go] 这里实际上保存了实现RAFT相关的核心模块
		type storage struct[etcserver/storage.go] storage 真正持久化的操作，主要是通过WAL和Snapshot完成持久化，不包含具体的业务逻辑，例如存储可以采用KV结构；这里会在处理Ready()结构体时进行处理
			type WAL struct[wal/wal.go] WAL日志的处理
				func (w *WAL) Save(st raftpb.HardState, ents []raftpb.Entry) error
				func (w *WAL) SaveSnapshot(e walpb.Snapshot) error 保存的是snapshot的元数据信息，标示此时触发了一次Snapshot操作
			type Snapshotter struct[snap/snapshotter.go] 将snapshot保存到磁盘
		type node struct[raft/node.go] 处理RAFT协议内部交互的核心，该结构体与type raft struct协同，驱动数据流转，而raft更像只提供了接口*****
			func (n *node) run(r *raft) 通过管道进行交互的核心处理 <<<<<<RAFT协议核心处理逻辑>>>>>>
		type MemoryStorage struct[raft/storage.go] raftStorage 缓存在内存中的数据，主要是临时保存在内存中的临时日志
		type Transport struct[rafthttp/transport.go] transport 与服务器通讯的接口

在 StartNode() 函数处会将 type node struct 和 type raft struct 结合起来，由 raft 通过数据流来驱动 node 的接口
type raft struct[raft/rafg.go] RAFT协议核心处理逻辑，这里只有关键的处理流程，并不包含例如日志持久化、网络通讯等模块
	func (r *raft) send(m pb.Message) 进行一些合法性检查，然后将消息添加到mailbox中
	func (r *raft) sendAppend(to uint64) 发送数据给peer节点，包括了消息的组装逻辑
	func (r *raft) becomeLeader() 角色转换
	func (r *raft) Step(m pb.Message) error 接收到消息之后的处理逻辑，不同角色会调用不同的函数，例如 stepLeader() stepCandidate() stepFollower()
	func (r *raft) handleAppendEntries(m pb.Message) 处理具体的消息
	type raftLog struct 处理RAFT日志，保存在内存中，包括了已经提交/应用的日志ID
		func (l *raftLog) commitTo(tocommit uint64)

在进行持久化数据时，其核心的处理逻辑也就是在 `raftNode.start()[etcdserver/raft.go]` 中，首先会通过 WAL 持久化状态信息，然后将日志添加到内存中。注意，对于业务来说，此时并未持久化数据。

ETCD 的入口在 main.go 文件中，真正调用的是 `etcdmain/main.go` 文件中的 `Main()` 函数，这里会启动三类监听端口：A) 内部通讯 `servePeers()`；B) 接收客户端请求 `serveClients()`；C) 内部指标数据 `serveMetrics()`。


有很多不错的介绍，尤其是其中的异常处理
https://alexstocks.github.io/html/etcd.html
Restore命令
https://blog.zhesih.com/2017/10/04/the-restore-in-etcd-v3/

## 客户端 数据提交

ETCD 提供了 V2 V3 两个版本的客户端，这里介绍最新的 V3 版本的 gRPC 使用方式。
https://coreos.com/etcd/docs/latest/learning/api.html

注意只是针对客户端，内部 Peers 之间的通讯仍然采用的是老版本的方式。

Protobuf 的协议格式文件在 `etcdserver/etcdserverpb/rpc.proto` 中定义，除了对客户端的 proto 文件之外，还有用于内部通讯序列化时所使用的格式。另外，为了提供 HTTP 接口，同时采用了一个 grpc-gateway 的统一解决方案，实际上也就是一个反向代理。

而具体的调用处理逻辑在 `etcdserver/api/v3rpc` 文件中，


startClientListeners


NewQuotaKVServer() 对实际KV服务提供的一层封装，每次请求会检查是否有足够的配额处理该请求，包括的存储的大小
 |-NewKVServer()

serveClients()[embed/etcd.go] 启动客户端，这里实际上会启动一个协程进行处理
 |-serve()[embed/serve.go] 协程中真正处理客户端的请求
 | |-v3rpc.Server() 这里会注册gRPC提供的几类接口，例如 KV Watch Lease Auth Maintenace 等

etcdserver/api/v3rpc 目录下有 quota.go key.go 两者都提供了 `Put()` 接口，其中 Quota 提供了对 KVServer 的封装，主要用来检查是否有足够的配额来处理本次的请求。

最终处理请求是在 `node.Propose()[raft/node.go]` 中，此时会构建一个 MsgProp 类型的报文，并发送到 `node.propc` 管道中。注意，如果此时集群中没有 Leader 的话会直接将报文丢弃掉。

在启动的 `node.run()` 函数中，会从 propc 管道中读取数据，然后调用 raft.Step() 函数，对于 Leader 而言，调用实际上就是 `stepLeader()` 。

对于 Leader 而言，会调用 appendEntry + bcastAppend 保存到本节点的内存中，并进行广播。

在 sendAppend 中，会将当前 raftLog 中需要发送的消息组装成 MsgApp 类型，然后添加到 raft.msgs[] 数组中

ETCD 提供了类似的消息总线的实现，所有的消息，包括了内部消息，都会通过 raft.Step() 统一进行处理。

raft.Step() 会对接收到的报文逐条进行处理


消息接收



如果是对端发送过来的消息，则会发送到 peer.recvc 管道中，而在 `startPeer()` 函数中，会启动一个协程消费 `recvc` 管道，并调用 Process() 函数进行处理，实际上调用是 EtcdServer.Process() 函数。

EtcdServer.Process()
 |-raft.Step()

接收到的消息

streamReader.run() rafthttp/stream.go 处理接收的模块
 |-streamReader.dial() 建立连接，使用的是HTTP协议，然后反序列化

假设消息为日志复制，那么对于 Follower 来说，会在 stepFollower() 函数中进行处理，

func stepFollower(r *raft, m pb.Message) {
    switch m.Type {
    case pb.MsgProp:
        ...
    case pb.MsgApp:
        r.electionElapsed = 0 
        r.lead = m.From
        r.handleAppendEntries(m)
    }
}

不管节点的角色是什么，最终提交日志都是通过 `handleAppendEntries()` 函数，

## SnapShot

相关的结构体在 `raft/raftpb/raft.proto` 文件中通过 Protobuf 定义。

type ConfState struct {
    nodes     []uint64
    learners  []uint64
}

type SnapshotMetadata struct {
    conf_state ConfState 
    index     uint64               
    term      uint64
}

type Snapshot struct {
    data     []byte
    metadata SnapshotMetadata 
}

https://zhuanlan.zhihu.com/p/29865583


Snapshot 是系统在某个时间点的快照，保存了当前系统的状态数据，这样的好处有：A) 之前的日志可以删除节省空间 (包括磁盘和内存)，重启的恢复速度会加快；B) 新节点加入时，直接将 SnapShot 以及之后的日志复制过去即可。

### 保存位置

与日志类似 SnapShot 也会在多个地方存储，包括了 `raftLog.unstable` `raftLog.storage` 以及 WAL 。

#### unstable

unstable 日志中的 snapshot 来自于 Leader 节点的 SnapMsg 消息。

其中 storage 中的 snapshot 来源于：A) 节点自身生成的 SnapShot ，那么此时节点的应用已经包含了 SnapShot 状态；B) Leader 会通过 SnapMsg 将 SnapShot 复制到 Follower 的 unstable 中，然后再通知 Follower 应用到 storage 中。

前者主要是进行日志压缩，后者是为了减少数据同步时的网络和 IO 消耗 (相比日志复制，数据量要小很多)。

其次，因为unstable log中的snapshot唯一来源是Leader节点的消息同步，因此，该snapshot需要被转交给应用，由应用完成重放后再删除；而storage中的snapshot则是由应用调用storage.CreateSnapshot主动创建，会保存在storage结构中，直到再次创建snapshot时被新的锁替代。

从上面的对比可知道，unstable log和storage中存储的snapshot内容并不一致。

WAL 存储的是 SnapShot 的元信息，主要包含日志更新的 `{term，index}` 二元组，其目的是用于节点启动时重放日志的索引。重启时会先加载 SnapShot 数据，然后再重放该 SnapShot 以后的 WAL 更新日志即可，从而提高启动效率。

SnapShot 由 RAFT 协议层之上的应用层完成，这取决于应用存储类型。

## 执行流程

因为 SnapShot 记录的是应用的状态数据，所以，SnapShot 也必须由应用来进行。

以etcd示例应用为例，snapshot的触发时机被嵌入在请求的处理流程之中，具体来说，每次上层应用从raft协议核心处理层获取到日志项后，处理该日志项的过程中便插入一个是否需要进行snapshot的判断处理，如下代码来自raftexample/raft.go

CoreOS的黑魔法 几篇很不错的文章
https://zhuanlan.zhihu.com/p/29882654

在 `etcdserver/raft.go` 

EtcdServer.run()
 |-raftNode.start()
 | |-raft.Ready() 等待readyc管道的输出
 | |-updateCommittedIndex() 这里对应了etcdserver/server.go中的函数
     |-EtcdServer.getCommittedIndex() 一个全局变量

https://blog.zhesih.com/2017/10/04/the-put-in-etcd-v3/

etcd 中线性一致性读的具体实现
https://zhengyinyong.com/etcd-linearizable-read-implementation.html
https://coreos.com/etcd/docs/latest/op-guide/maintenance.html

EtcdServer.applySnapshot()
 |-openSnapshotBackend() 会打开一个新的snapshot

ETCD 启动时设置了参数 `--snapshot-count` 即 index 变化达到该值时，就会生成 `.snap` 文件，详见 `triggerSnapshot()` 中的实现。

EtcdServer.applyAll()
 |-EtcdServer.triggerSnapshot() 判断是否需要生成snapshot
 |-

协程分类

EtcdServer.run() 将已经提交的数据通过applyAll()持久化到数据库中

https://blog.zhesih.com/2017/10/02/snap-file-of-etcd-v3/
一些在运维过程中可能出现的告警信息
https://blog.zhesih.com/2017/10/27/warning-log-about-etcd/
https://www.cnblogs.com/davygeek/p/8524477.html

在 node.run() 函数中，会通过 newReady() 来构建当前的消息内容，如果有需要修改的，那么会通过 readyc 发送给应用层，然后会通过 advancec 管道等待应用层完成。

在应用层做相应的处理完成相关的处理之后，会调用 Raft.Advance() 函数，实际上就是向 advancec 管道发送一个空的结构体，用来触发协议层的处理，主要是将 raftLog 中 unstable 的内容通过 stableTo() 。

在 node.run() 函数中，有关于 advancec 和 readyc 的转换，

在 EtcdServer 中拿到了已经提交数据信息后，会向 applyc 管道发送相关的信息，然后在 EtcdServer.run()[etcdserver/server.go] 函数中会等待，并调用 applyAll() 函数，实际上会经过 sched.Schedule() 的调度。

handleAppendEntries() 也就是Follower接收到MsgApp之后的处理
 |-raftLog.maybeAppend() 
   |-raftLog.matchTerm()
     |-raftLog.term() 会根据index判断范围是否合法

raftLog.firstIndex()
 |-unstable.maybeFirstIndex() 如果有从主节点发送过来的snapshot操作则返回
 |-MemoryStorage.FirstIndex() 存储中索引
   |-MemoryStorage.firstIndex() 也就是ents[0].Index + 1 ????????? 为什么加1
raftLog.lastIndex()
 |-unstable.maybeLastIndex() 如果有数据则会返回最后一条的index，也就是offset+len(entries)-1，如果没有数据则返回snapshot中的index
 |-MemoryStorage.LastIndex()
   |-MemoryStorage.lastIndex() ents[0].Index + len() - 1 

firstIndex 已经保存到SnapShot的日志号

在 unstable 中，貌似 snapshot 和 entries 不会同时存在。

https://draveness.me/etcd-introduction


Follower 是如何将 Apply 后的数据持久化的？？？


所有接收到的消息保存在 raftLog 中，包括了已经提交或者未提交的数据。


未提交的数据保存在 unstable 中，相互关系可以从 `newLog()` 函数或者 `newReady()` 函数中获取。

例如在 `newReady()` 函数中，获取

unstable 保存的是从其它节点



其中 firstIndex 和 lastIndex 都是通过函数动态获取的，

日志保存

https://www.codedump.info/post/20180922-etcd-raft/

progress.maybeUpdate() 更新Match以及Next
 |-raft.maybeCommit() 用来判断是否有数据提交

func (r *raft) maybeCommit() bool {
        // TODO(bmizerany): optimize.. Currently naive
        mis := make(uint64Slice, 0, len(r.prs))
        for _, p := range r.prs {
                mis = append(mis, p.Match) // 获取所有Follower的Match信息
        }
        sort.Sort(sort.Reverse(mis)) // 从大到小进行排序
        mci := mis[r.quorum()-1]
        return r.raftLog.maybeCommit(mci, r.Term)
}

func (r *raft) quorum() int {
	return len(r.prs)/2 + 1
}

如果节点数量为 5 ，那么会从 mis 中读取第三最新日志索引就是复制到过半数节点的日志索引，也就是意味着这个位置的日志可以提交了。

raftLog.maybeCommit() 函数会尝试将保存在日志中的条目向前移动，为了防止网络上的重复报文，在尝试提交的时候会同时检查 term 是否合法。

firstIndex()

1. 首先是 snapshot 中的 index + 1；为啥保存在了unstable中，给人的直观感觉就是没有持久化的数据
2. 然后才是日志中保存的第一条记录


raftLog.append() 用来添加日志
 |-unstable.truncateAndAppend()

stableTo()
stableSnapTo()

假设有一个 Worker 协程来处理请求，内部为一个死循环，

如果缓冲队里满之后，发送的协程默认会阻塞，那么此时可以通过 `context` 的 `WithTimeout()` 接口设置一个合理的超时时间。


https://blog.zhesih.com/2017/10/04/the-db-file-in-etcd-v3/

客户端的使用方式
https://yuerblog.cc/2017/12/12/etcd-v3-sdk-usage/
https://zhuanlan.zhihu.com/p/36719209


数据提交过程只能是由 Leader 发起，代码中在 `stepLeader()` 函数中，

所有的通讯数据都会通过 Ready 结构体返回，

可以通过 IsLocalMsg() 函数判断是否为本地的消息，还是需要发送给 Peers 的消息。

所有的消息都是通过 raft.Step() 函数进行处理，所以在 etcd 的消息中，有些消息是节点内部使用的，有些会通过网络发送到 Peers 节点处。

MsgHup            MessageType = 0  // 当Follower节点的选举计时器超时，会发送MsgHup消息
MsgBeat           MessageType = 1    //Leader发送心跳，主要作用是探活，Follower接收到MsgBeat会重置选举计时器，防止Follower发起新一轮选举
MsgProp           MessageType = 2   //客户端发往到集群的写请求是通过MsgProp消息表示的
MsgApp            MessageType = 3  //当一个节点通过选举成为Leader时，会发送MsgApp消息
MsgAppResp        MessageType = 4  //MsgApp的响应消息
MsgVote           MessageType = 5      //当PreCandidate状态节点收到半数以上的投票之后，会发起新一轮的选举，即向集群中的其他节点发送MsgVote消息
MsgVoteResp       MessageType = 6  //MsgVote选举消息响应的消息
MsgSnap           MessageType = 7		//Leader向Follower发送快照信息
MsgHeartbeat      MessageType = 8			//Leader发送的心跳消息
MsgHeartbeatResp  MessageType = 9    //Follower处理心跳回复返回的消息类型
MsgUnreachable    MessageType = 10  //Follower消息不可达
MsgSnapStatus     MessageType = 11   //如果Leader发送MsgSnap消息时出现异常，则会调用Raft接口发送MsgUnreachable和MsgSnapStatus消息
MsgCheckQuorum    MessageType = 12     //Leader检测是否保持半数以上的连接
MsgTransferLeader MessageType = 13		//Leader节点转移时使用，本地消息
MsgTimeoutNow     MessageType = 14    //Leader节点转移超时，会发该类型的消息，使Follower的选举计时器立即过期，并发起新一轮的选举
MsgReadIndex      MessageType = 15   //客户端发往集群的只读消息使用MsgReadIndex消息（只读的两种模式：ReadOnlySafe和ReadOnlyLeaseBased）
MsgReadIndexResp  MessageType = 16   //MsgReadIndex消息的响应消息
MsgPreVote        MessageType = 17   //PreCandidate状态下的节点发送的消息
MsgPreVoteResp    MessageType = 18  //预选节点收到的响应消息


在运行时，即使有少数派，仍然允许其可以恢复运行。注意，此时需要防止由于其它服务器恢复导致脏数据。



ID [%d, %d]

支持 Leaner ，也就是只复制集群的数据，但是并不完成投票相关内容。可以理解为一个只读节点。

接收到报文后同样在 raft.Step() 函数中，通过 isUpToDate() 判断 Candidate 的日志是否早于当前节点的日志，只有是才会允许返回投票报文，否则会返回拒绝。

对于 PreVote 来说，返回的是 PreVoteResp 报文。

对于响应报文，实际的处理是在 step() 钩子函数中，也就是在 `becomePreCandidate()` 函数中设置的 `stepCandidate()` 函数。

与投票相关的统计有两个关键的函数，分别为 `poll()` 和 `quorum()` ，其中 `poll()` 会根据获取到的信息统计当前已经投赞成票的票数，而 `quorum()` 返回的是半数节点信息。

### 场景一

三台机器 A B C 之前是正常的，A 发生网络隔离，导致 Term 不断增加。网络恢复之后，是否会引起其它节点重新选主？Term 会如何进行变化？

首先，该节点由于日志落后过多，在 isUpToDate() 会返回 false ，进而导致 A 不会成为 Leader 。

对于，Leader 而言，接收到了 `MsgPreVote` 或 `MsgVote` 报文，在 `raft.Step()` 函数中会进行处理。注意，对于通过 `campaign()` 函数发送的 MsgVote 报文，对应的 Context 是空。????????????? 还没有看懂Leader的处理，貌似是啥也不做


RAFT 中的有效日志保存在 raftLog 中，详细可以查看 isUpToDate() 函数的处理方式。????????? 为什么首先尝试返回unstable中的SnapShot相关的Term信息？unstable 中到底保存的是啥信息？



这个应该是写的比较实在的，有很多相关的优化细节
https://youjiali1995.github.io/raft/etcd-raft-leader-election/


## 状态切换的细节

1. 会忽略几乎所有


## 选举优化

在标准的 RAFT 协议中，只做了基本的束了，而真实世界中却复杂的多。

只实现基本的 leader election 会有几个问题：

* 可能会同时存在多个 Leader。发生网络分区后，原有 Leader 在 minority，而 majority 部分会在超时后重新选举出新 Leader，而旧 Leader 由于接收不到新 Leader 的消息会认为自己依然是 Leader，只是在老 Leader 中数据不会提交成功，所以无数据安全性问题。
* 一个坏的节点可能会干扰到集群的正常工作。单个节点分区后，不断尝试发起选举，而每次选举都会自增 Term ，这样就会在网络恢复后重连导致原有集群进行不必要的 Leader Election。

为了解决上述问题，ETCD 实现了几种优化方案：

### Check Quorum

出现了网络分区之后，应该只有 majority 部分的 Leader 能正常工作，为此，Leader 在达到 electionTimeout 超时间隔后，会触发检查其它节点的活跃情况，如果活跃节点少于 majority ，那么会自动切换到 Follower 状态。

其中 Check Quorum 功能可以通过配置开关，Leader 每个 electionTimeout 之后会发起一次 Check Quorum 操作，

raft.tickHeartbeat() Leader的心跳时间回调函数
 |-raft.Step() 超过了electionTimout之后，发送MsgCheckQuorum类型的消息
 |-raft.stepLeader() 对于Leader来说，实际上调用的是改函数
 |-raft.checkQuorumActive() 会统计Progress中RecentActive为true的节点数，也就是当前活跃的节点数

其中 RecentActive 会在接收处理报文的时候设置为 true ，也就是说，只要各个 Peers 与 Leader 在 electionTimeout 周期内有通讯，那么该节点就会被标记为活跃。

#### 总结

Check Quorum 机制不能阻止多 Leader 的产生，但能够减少同时存在的时间，减少无用功，不会影响集群的正常使用和一致性。

* 新写入旧 Leader 的请求由于不满足多数派，会导致在 Commit 时失败；
* 同样绕过日志复制来读取数据的请求，同样会有相应的措施保证线性一致性；
* 在 Election Timeout 之后，Leader 会变为 Follower，那么之前连到该节点的客户端就会连到新的 Leader 上进行正常的请求。

对客户端的影响最大时间间隔是 `2 * electionTimout` ，在第一个周期的开始，接收到多数节点的消息，在结束末尾状态清零，然后在下个超时检测到位少数派。

### Pre-Vote

发生分区后的节点能够干扰集群的原因是其 Term 比其它节点要大，在恢复之后分区的节点日志一般落后其它节点，不会收到多数派投票，但是会导致原正常工作的多数派集群的抖动。尤其是当网络抖动比较严重时，可能会影响到集群的正常工作。

那么，最简单的方式就是减少这种 Term 的无谓增大，也就是增加 Pre-Vote 阶段:

* 在 PreVote 阶段只有收到多数派的投票，才会增加 Term 并进行正常的选举过程；
* 若 PreVote 失败，由于没有实际增加 Term 值，那么当网络恢复后，该节点收到 Leader 的消息重新成为 Follower ，从而避免了集群的抖动；

同样，ETCD 支持通过配置开启 PreVote 机制，在原有的 RAFT 协议基础上，新增了一个 StatePreCandidate 状态。

当在 tickElection() 中超时后，会通过 `raft.Step()` 函数发送一个 `MsgHup` 消息，在 `Step()` 函数中，有如下的调用逻辑。

if r.preVote {
    r.campaign(campaignPreElection)
} else {
    r.campaign(campaignElection)
}

也就是说，如果设置了 `preVote` 会进入 `PreElection` 步骤。

raft.Step()
 |-campaign() 在处理内部的MsgHup消息时会先执行PreElection操作，进入竞选逻辑
   |-becomePreCandidate() 设置为PreVote状态
   |-becomeCandidate() 设置为Candidate状态

然后发送选举报文到其它节点，可以是 `MsgPreVote` 或者 `MsgVote` 类型。



一些注意点：

* 单个节点可以给多个 PreVote 节点的投票；
* PreVote 实际不会增加节点的 Term，但发送的 MsgPreVote 消息中 Term 会加 1 ，也就是竞选的 Term；
* PreVote 失败回退为 Follower ，但是其 Term 不变；

成为 PreCandidate 不会设置 r.lead = None；
当给 Pre-Vote 投票时，发送的 MsgPreVoteResp 的 term 是接收到的 MsgPreVote 的 term 而不是本地的 term，因为本地 term 一般会比 竞选的 term 小，节点会忽略落后的消息；
Pre-Vote 成功不能保证该节点会成为 leader，有可能在 Pre-Vote 成功后集群其他节点写入了新的 entry 或者发生 split vote，此时会回退为 follower， 重新进行 Pre-Vote

那么增加了 PreVote 流程之后，那么选举的时间有扩大到了两倍。

### Leader Lease

PreVote 只解决了对于 log 落后的节点不会自增 Term 导致集群抖动，但是仍有可能节点 log 没有落后但仍发起 PreVote (除了如下介绍的，还有在后面的 Cluster Membership Change 场景中也会遇到) 。

最简单的问题场景如下：

1. 有 S1 S2 S3 三个节点，S1 为 Leader 且三个节点 log 相同；
2. 发生网络分区，其中 S1 S3 互通，S2 S3 互通，S1 S2 不通；
3. S2 发起 PreVote，收到 S3 投票后，进行正常 Election 并成为新的 Leader，其中 Term 会 +1 ；
4. S1 从 S3 获取到最新的 Term 然后降级为 Follower ；
5. S1 因为接收不到最新 Leader S2 的心跳，那么在超时之后同样会进入选举流程；
6. 接着会依次反复，直到 S1 和 S2 之间的网络恢复正常。

注意，上述的场景中，客户端不再发送请求，也就是说三个节点的日志相同。如果 S1 仍然在处理用户请求，那么原生的 RAFT 也可以正常处理，S2 在尝试发起选举时会由于其日志落后导致选举失败。

#### 解决方案

ETCD 解决这个问题采用的方法是 Leader Lease，也就是说节点不应该在集群正常工作的情况下给其它节点投票。

* 当节点在 electionTimeout 时间内接收到了 Leader 的消息，就不会改变自己的 term 也不会给其它节点投票。

注意，在使用 Leader Lease 时候需要开启 Check Quorum 。简单来说，Check Quorum 是从 Leader 角度判断自己是否合法，而 Leader Lease 是从 Follower 角度判断 Leader 是否合法。

若只有 Leader Lease 而没有 Check Quorum 则可能会有下面这种情况，假设有 5 个节点，分别为 S1~S5 。

* 其中 S1 为 Leader，S2-S5 为 follower；
* 发生了网络分区 S1 S2 互通，S2-S4 互通，S5 不通;
* 在没有 Leader lease 的情况下，S2-S4 会选举出新的 Leader；但开启了 Leader Lease 后，S2 仍然可以接收到 S1 的心跳，那么会忽略其它节点的投票消息，不会选举出新的 Leader；

所以从 S2-S4 看，S1 S5 故障，无法保证在 Majority 正常的情况下保证集群正常工作。

为此，ETCD 中开启 Check Quorum 就开启了 Leader Lease 。

可以直接在代码中搜索 inLease 的处理。

## 总结

Check Quorum 和 PreVote 作用有些类似，但可以互补，一般会同时开启来提高可用性:

* Check Quorum 保证了集群正常工作的情况下，不会受到坏节点的影响，但是只要有一个节点 Term 升高，为了使这个节点正常工作，Leader Election 不可避免；
* PreVote 在大多数情况下避免了节点 Term 的升高。

### 遗留问题

ETCD 的实现会忽略 Term 低于自己的消息，在使用 Check Quorum 或 PreVote 时会带来问题：

* 开启 Check Quorum 时，被网络分隔的节点在网络恢复后一直发送投票请求，但是其他节点 inLease 会忽略它的消息，同时这个节点忽略 leader 的消息，导致这个节点不能 stable。
* 开启 PreVote 时，发起 PreVote 的节点的 Term 比其他节点高，但是 log 却落后，导致这个节点不能 stable。这可能发生在 Pre-Vote 成功，然后正常 vote 时， 该节点被 partition，同时集群正常工作写入新的 log，在网络恢复后，这个节点发起的 Pre-Vote 不会成功，且会忽略 leader 的消息，不会 stable。
还有种情况比较 tricky，发生在变更配置时，Pre-Vote 从 false 变为 true，若 term 高的节点 log 落后可能会造成集群死锁，不会选举出 leader。 比如有 3 个节点，term 越高的节点 log 越旧，导致没有一个节点能够 Pre-Vote 成功。(见 issue #8501和 pr #8525)。


原则 <1> 无论何种方式，所有节点的 Term 值不会减小。

https://cloud.tencent.com/developer/article/1394643

批量升级方案

1. 最简单的是

ETCD 李响 作者，目前在美国阿里工作，Youtube 上有其演讲的相关视频


有部分分区的介绍
https://www.yangcs.net/posts/etcd-server-learner/
很不错的GitBook文档，包括了Learner Client的详细介绍
https://etcd.readthedocs.io/en/latest/index.html


node.ProposeConfChange() 会构建一个MsgProp类型的报文，其中Entry中包含了一个EntryConfChange类型的日志
 |-node.Step()
 |-node.step() 发送到node.propc管道中
 |-raft.Step() 调用到raft中的Step函数
 |-raft.stepLeader() 对于Leader来说真正执行的函数，会检查当前是否有配置请求还没有处理完，如果是则会直接忽略

在 etcdserver/server.go 中的协程中，会获取已经应用成功的日志，也就是调用 applyAll() 函数。

EtcdServer.applyAll()
 |-EtcdServer.applyEntries()
 |-EtcdServer.apply()
 |-EtcdServer.applyConfChange() 如果是EntryConfChange类型的报文
 |-node.ApplyConfChange() 会通过confc管道发送，并等待confstatec管道输出，也就是一个同步操作

接着在 node.run() 函数中会等待处理，最终会调用 addNode() removeNode() 之类的接口函数。

https://www.cnblogs.com/foxmailed/p/7190642.html

注意

1. 如果已经存在了配置变更的请求，那么会直接将请求忽略 (沉默忽略，没有日志，没有报错)

选举介绍的不错
https://youjiali1995.github.io/raft/etcd-raft-leader-election/

##############################
## 核心处理逻辑
##############################

Leader选举 日志复制 集群状态变更 日志压缩

### Leadership Transfer

## 日志复制

包括了 A) 内存缓存日志；B) WAL；C) Snapshot 。

### Memory

#### 从 Unstable 转换为 Stable

<<<<<<<1>>>>>>> 非常方便构造测试用例

### WAL

为了保证 RAFT 核心协议的简单，该模块是独立实现的，保存在 wal 目录下，其实现也很简单。 TODO 实现的细节以后再看

## Leader 转换

用于集群维护以及负载均衡。

## ReadOnly

在分布式系统中，共识算法 (Consensus Algorithm) 和一致性 (Consistency) 经常被讨论，简单来说，RAFT 和 Paxos 提供了共识算法或者原子广播，而要获得一致性 (例如线性一致性) ，还需要一些其它的特性来保证。

https://zhuanlan.zhihu.com/p/47117804

ETCD 实现的 RAFT 协议提供了 ReadOnly 特性，用来批量处理只读请求。

只读请求有两种模式：A) ReadOnlySafe 不受节点之间时钟差异和网络分区的影响 (推荐)；B) ReadOnlyLeaseBased 基于时钟，当时钟不可控时 (停止、向后漂移) 会导致 ReadIndex 操作异常。


##############################
## Snapshot
##############################

Snapshot 会保存在 unstable log、storage 和 WAL 。

首先，unstable log中的snapshot来自于Leader节点的SnapMsg消息，即unstable log中的snapshot是被动接收和存储的，这在我们后面的snapshot复制流程会详细描述。storage的snapshot来源有2：第一，来自于节点自身生成的snapshot，如果是这样，那么该节点的应用肯定已经包含了snapshot状态，因此，该snapshot无需在应用的状态机中进行重放，其主要目的是进行日志压缩；第二，Leader节点的SnapMsg会将snapshot复制到Follower的unstable log中，进而通知到Follower的应用，再进一步将其应用到storage。这个snapshot的主要目的是将Leader的应用状态复制到当前的Follower节点，同时相比于日志复制，它减少了数据同步的网络和IO消耗。

其次，因为unstable log中的snapshot唯一来源是Leader节点的消息同步，因此，该snapshot需要被转交给应用，由应用完成重放后再删除；而storage中的snapshot则是由应用调用storage.CreateSnapshot主动创建，会保存在storage结构中，直到再次创建snapshot时被新的锁替代。

从上面的对比可知道，unstable log和storage中存储的snapshot内容并不一致。

WAL主要存储snapshot的元信息，包括snapshot所包含的日志更新的{term，index}二元组。WAL存储元信息的目的在于，节点启动时重放日志的索引。节点启动时首先加载snapshot数据，接下来再重放该snapshot以后的更新日志即可，提高启动效率。

snapshot的数据存储方法由使用etcd-raft的应用实现，这取决于应用存储的数据类型。

server
mvcc
## FAQ

在 Leader 中保存了已经发送给各个 Server 的日志索引，包括了：A) nextIndex B) matchIndex

原则上来说，这两个的索引应该是相差 1 的，也有可能是批量发送导致差异大于 1 。那么两个变量的作用是啥，如果中间有日志没有被确认，那么 Leader 将如何进行处理？

如果发生 term 的回环，那么将会发生什么？

https://my.oschina.net/fileoptions/blog/1637873
https://studygolang.com/articles/14731
https://www.codedump.info/post/20180922-etcd-raft/
https://youjiali1995.github.io/categories/
https://blog.zhesih.com/2017/11/05/raft-follower/

没有代码生成器的ProtoBuff实现
https://github.com/cloudwu/pbc
https://github.com/cloudwu/sproto
https://github.com/protobuf-c/protobuf-c-rpc
很简单的一个协议
https://github.com/clibs/amp
不错的通用库
https://github.com/JeffBezanson/libsupport

在终端打印表格
https://github.com/marchelzo/libtable

https://github.com/begeekmyfriend/bplustree
https://github.com/antirez/rax
https://github.com/armon/libart
https://github.com/greensky00/avltree
-->

{% highlight text %}
{% endhighlight %}
