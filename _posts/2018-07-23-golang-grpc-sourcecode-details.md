---
title: GoLang gRPC 源码解析
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: grpc,golang
description: gRPC 是一个通用的 RPC 框架，这里简单看下 GoLang 中 gRPC 的实现。
---

gRPC 是一个通用的 RPC 框架，这里简单看下 GoLang 中 gRPC 的实现。

<!-- more -->

![grpc introduce]({{ site.url }}/images/go/grpc-introduce.png "grpc introduce"){: .pull-center width="70%" }

## 简介

整个 RPC 服务的调用流程如下：

1. client 调用 client stub，这是一次本地过程调用；
2. client stub 将参数打包成消息，然后发送这个消息，打包过程(序列化)也叫做 marshalling；
3. client 所在的系统将消息发送给 server；
4. server 的将收到的包传给 server stub；
5. server stub 解包得到参数，解包(反序列化) 也被称作 unmarshalling；

最后 server stub 调用服务过程，返回结果按照相反的步骤传给 client 。

### 目录结构

在 gRPC 目录下有许多常用的包，例如：

##### metadata

定义了 gRPC 所支持的元数据结构，包中方法可以对 MD 进行获取和处理。

credentials：实现了grpc所支持的各种认证凭据，封装了客户端对服务端进行身份验证所需要的所有状态，并做出各种断言
codes：定义了grpc使用的标准错误码，可通用


## Client

对于代码中，实际调用流程为。

{% highlight text %}
c := pb.NewGreeterClient(conn)
r, err := c.SayHello(ctx, &pb.HelloRequest{Name: name})
{% endhighlight %}


{% highlight text %}
Dial()                        clientconn.go
 |-DialContext()              除了个函数的入参，同时会新建一个CTX
   |-context.WithCancel()     创建所使用的上下文
{% endhighlight %}

<!--
https://blog.csdn.net/omnispace/article/details/80166975
https://guidao.github.io/grpc_balancer.html
http://vinllen.com/golang-net-rpcyuan-ma-fen-xi/
-->

## Server

一般来说分为了如下几步操作：

1. 实例化Server
2. 注册Service
3. 监听并接收连接请求
4. 连接与请求处理

对于代码中，实际调用流程为。

{% highlight text %}
s := grpc.NewServer()
pb.RegisterGreeterServer(s, &server{})
// Register reflection service on gRPC server.
reflection.Register(s)
if err := s.Serve(lis); err != nil {
	log.Fatalf("failed to serve: %v", err)
}
{% endhighlight %}

### 实例化Server

一个 Server 结构代表对外服务的单元，每个 Server 可以注册多个 Service，每个 Service 可以有多个方法。主程序需要实例化 Server，然后注册 Service，最后调用 `s.Serve()` 开始接收请求。

服务实例对应的结构体在 `server.go` 中定义，核心的成员变量包括了如下：

{% highlight text %}
type Server struct {
        opts options

        mu     sync.Mutex // guards following
        lis    map[net.Listener]bool
        conns  map[io.Closer]bool
        serve  bool
        drain  bool
        cv     *sync.Cond          // signaled when connections close for GracefulStop
        m      map[string]*service // 服务的映射，其中Key对应了protoc生成的ServiceDesc中的ServiceName字段
}
{% endhighlight %}

创建实例的过程如下。

{% highlight text %}
NewServer()
 |-Server()           实例化Server对象
 |-sync.NewCond()     同步用条件变量
{% endhighlight %}

函数 `RegisterService()` 用于向服务器注册 gRPC 实现的相关服务，一般是由 IDL(Interface Description Language) 自动生成的语言调用，需要在正式开始提供服务前调用。

### 注册Service

例如上述的示例中，会通过 `RegisterGreeterServer()` 注册服务，也就是通过 `protoc` 编译生成的函数接口。

{% highlight go %}
type GreeterServer interface {
        // Sends a greeting
        SayHello(context.Context, *HelloRequest) (*HelloReply, error)
}

func RegisterGreeterServer(s *grpc.Server, srv GreeterServer) {
        s.RegisterService(&_Greeter_serviceDesc, srv)
}
{% endhighlight %}

也就是说定义的服务需要是 `type GreeterServer interface` 的实现，其定义了函数的名称，以及其入参等。

同时，如上的示例会生成如下的服务描述代码。

{% highlight go %}
var _Greeter_serviceDesc = grpc.ServiceDesc{
        ServiceName: "helloworld.Greeter",      // 服务名
        HandlerType: (*GreeterServer)(nil),
        Methods: []grpc.MethodDesc{             // 非流式方法，单次调用
                {
                        MethodName: "SayHello",
                        Handler:    _Greeter_SayHello_Handler, // 最终调用的方法
                },
        },
        Streams:  []grpc.StreamDesc{},
        Metadata: "helloworld.proto",
}
{% endhighlight %}

其调用流程大致如下。

{% highlight text %}
RegisterGreeterServer
 |-RegisterService()
 | |-register()             这里会通过反射获取的上述的ServiceDesc以及具体实现的接口
 |   |-map[]                检查是否已经注册，其中key是服务描述结构体中的ServiceName字段
 |   |-service{}            实例化服务对象
{% endhighlight %}



### 监听并接收请求

{% highlight text %}
Serve()                 server.go
 |-Accept()                            接收链接请求
 |-handleRawConn()                     启动一个单独的协程处理每个请求，因此不会阻塞Accept循环
   |-useTransportAuthenticator()       建立握手
   |-newHTTP2Transport()               新的http2流，完成握手建立连接
   |-Server.addConn()                  添加到conns成员中
   |-serve()                           实际上调用的就是如下函数
   |=serveStreams()                    真正开始处理请求
   | |-HandleStreams()                 连接的处理细节，会一直循环读取数据，直到出现错误
   | | |                                 退出transport/handler_server.go，会通过传入的函数指针处理
   | | |-handleStream()                通过函数指针传入，新请求的处理细节，真正的回调处理函数，
   | | | |                               只有HEADER类型的帧才会处理这一类型；真正的业务处理逻辑
   | | | |-Method()                    解析流获取请求信息，提取服务名、方法名等信息，判断服务是否已经注册
   | | | |-processUnaryRPC()           简单的RPC调用(一元)，核心处理函数
   | | | | |-sendResponse()            发送响应
   | | | |-processStreamingRPC()       流方式的RPC调用
   | | | |-WriteStatus()               返回数据
   | | |-runStream()
   | |-wg.Wait()                       等待上述的请求处理完成
   |-removeConn()
{% endhighlight %}

在接收请求时，如果失败会从 5ms 开始指数增长休眠一段时间，最大是 1 秒。


<!--
Python gRPC
https://www.jianshu.com/p/14e6f5217f40
https://github.com/grpc/grpc/tree/master/examples/python
https://github.com/grpc/grpc.github.io/blob/master/docs/tutorials/basic/python.md
http://blog.51cto.com/lansgg/1931961
https://dev.lightning.community/guides/python-grpc/
-->

{% highlight text %}
{% endhighlight %}
