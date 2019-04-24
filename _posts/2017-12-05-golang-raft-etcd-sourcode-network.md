---
title: ETCD 网络模块
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd
description:
---

在使用 RAFT 协议内核时，需要单独实现网络的通讯协议，也就是集群中各个结点之间的通讯，除此之外还有客户端与服务器之间的通讯。

这里简单介绍其实现方式。

<!-- more -->

## 集群通讯

实际上，在 ETCD 中有两个与网络交互的流处理对象：`streamReader` 和 `streamWriter`，分别用来处理网络的读写流量。

<!--
在如下的函数中，最终真正调用的处理函数就是 `ServeHTTP()` 接口。

NewPeerHandler() etcdserver/api/etcdhttp/peer.go
 |-newPeerHandler()
   |-http.NewServeMux() 初始化一个Serve Multiplexer结构
   |-mux.HandleFunc()/Handle() 注册一堆的URI
     |-raftHandler() /raft
     |-peerMembersHandler() /members

### 会话建立

Transport.Handler() rafthttp/transport.go
 |-http.NewServeMux() 新建HTTP的MUX对象
 |-mux.Handle()
   |-pipelineHandler() /raft
   |-streamHander() /raft/stream
   | |-peerGetter.Get() rafthttp/peer.go 根据对端的ID获取
   | |-newCloseNotifier() 这里只是构建一个closeNotifier对象
   | |-attachOutgoingConn()
   | | |-streamWriter.attach() rafthttp/stream.go 对于V3版本
   | |   |-streamWriter.connc 将新建的链接添加到cw.connc管道中
   | |-closeNotifier.closeNotify() 等待请求处理完成
   |-snapHandler() /raft/snapshot
   |-probing.NewHandler() /raft/probing

在上述的处理中，`/raft/stream` 处理较为复杂，如果抓包可以发现这里实际上没有响应的报文。

当上述的请求发送到管道之后，管道对端的处理是在 `streamWriter.run()` 中，一般会起一个单独的协程。

### 消息发送

Transport.Send() rafthttp/transport.go 开始发送
 |-peer.send() rafthttp/peer.go 向对端发送数据
   |-peer.pick() 根据消息类型选择具体的管道，包括pipeline.msgc、writec等
   | |-streamWriter.writec() 如果是V3版本，返回的是streamWriter.msgc
   |-writec <- m 然后将消息发送到管道中

管道对端的处理同样是在 `streamWriter.run()` 中，这里发送完之后实际上没有关闭，一直保持着长连接。

### 消息接收

链接的建立是由 streamReader 发起，也就是上述没有响应的 HTTP 请求。

streamReader.run() rafthttp/stream.go
 |-dial() 开始建立链接，主要是构建HTTP的请求头
 |-decodeLoop() 然后就开始循环处理接收到的请求
   |-messageDecoder() 如果是V3版本
   |-decode() rafthttp/msg_codec.go 这里对应的是raft/raftpb/raft.proto结构
   |-streamReader.propc 如果是MsgPro消息，则发送到改管道中
   |-streamReader.recvc 否则发送到该管道中

注意，上述的两个管道实际处理流程是相同的，为了防止由于无 Leader 导致处理协程阻塞，所以单独起一个协程处理 MsgPro 类型的消息。
-->



## gRPC

V3 版本中与客户端的通讯，使用 gRPC 替换掉了 HTTP ，而服务器各个节点之间的通讯还是使用类似 HTTP 的请求(与真正的 HTTP 请求类似，但是有部分区别，后面详述)。

其中与服务端相关的 RPC 保存在 `etcdserver/etcdserverpb/rpc.proto` 文件中，通过 service 定义了一系列的 RPC 请求方法，对应的请求和响应报文则在最后定义。

可以使用 `scripts/genproto.sh` 生成 go 文件，此时会生成两个 `rpc.pb.go` 以及 `rpc.pb.gw.go` 后者为一个反向代理，用于将 http 请求转换为 grpc 再发送到后端，可以使用 curl 命令调试。

### 服务启动

其入口在 `etcdserver/api/v3rpc/grpc.go` 文件中，而对应各个服务的实现保存在 `etcdserver/api/v3rpc` 目录下。

<!--

### 建立链接

首先需要链接到数据库，其入参包括了地址以及超时时间，

其中 Client 结构体的定义如下，Cluster、KV、Lease 等是以嵌入类型的方式引入，大部分是定义了 API 接口，代表了整个客户端的几大核心功能板块。

type Client struct {
    Cluster             集群管理，用来操作集群，包括增加、删除、更新节点
    KV                  clientv3/kv.go 主要的KV操作
    Lease               租约相关，例如申请一个TTL=10秒的租约
    Watcher             观察订阅，用来监听最新的数据变化
    Auth                管理ETCD的用户和权限，属于管理员操作
    Maintenance         维护ETCD，如主动迁移Leader的节点

    Username string
    Password string
}

https://yuerblog.cc/2017/12/12/etcd-v3-sdk-usage/

ETCDCTL_API=3 ./etcdctl get /test/hello
-->

### 示例代码

{% highlight go %}
package main

import (
        "fmt"
        "github.com/coreos/etcd/clientv3"
        "golang.org/x/net/context"
        "time"
)

var (
        dialTimeout    = 5 * time.Second
        requestTimeout = 10 * time.Second
        endpoints      = []string{"127.0.0.1:2379"}
)

func main() {
        cli, err := clientv3.New(clientv3.Config{
                Endpoints:   []string{"127.0.0.1:2379"},
                DialTimeout: dialTimeout,
        })
        if err != nil {
                println(err)
        }
        defer cli.Close()

        fmt.Println("Start to running ...")
        ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
        _, err = cli.Put(ctx, "/test/hello", "world")
        cancel()

        ctx, cancel = context.WithTimeout(context.Background(), requestTimeout)
        resp, err := cli.Get(ctx, "/test/hello")
        cancel()

        for _, ev := range resp.Kvs {
                fmt.Printf("%s : %s\n", ev.Key, ev.Value)
        }

        _, err = cli.Put(context.TODO(), "key", "xyz")
        ctx, cancel = context.WithTimeout(context.Background(), requestTimeout)
        _, err = cli.Txn(ctx).
                If(clientv3.Compare(clientv3.Value("key"), ">", "abc")).
                Then(clientv3.OpPut("key", "XYZ")).
                Else(clientv3.OpPut("key", "ABC")).
                Commit()
        cancel()

        rch := cli.Watch(context.Background(), "/test/hello", clientv3.WithPrefix())
        for wresp := range rch {
                for _, ev := range wresp.Events {
                        fmt.Printf("%s %q : %q\n", ev.Type, ev.Kv.Key, ev.Kv.Value)
                }
        }

        if err != nil {
                println(err)
        }
}
{% endhighlight %}

通过如下命令运行。

{% highlight text %}
GOPATH=$YOUR_WORKSPACE/gopath:$GOPATH go run main.go
{% endhighlight %}

<!--
https://blog.csdn.net/zoumy3/article/details/79521190
https://yuerblog.cc/2017/12/12/etcd-v3-sdk-usage/
https://www.compose.com/articles/utilizing-etcd3-with-go/
https://grpc.io/blog/coreos
http://blog.zhesih.com/2017/10/03/gRPC-in-ETCD-V3/
-->




{% highlight text %}
{% endhighlight %}
