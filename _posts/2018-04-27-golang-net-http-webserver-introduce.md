---
title: GoLang HTTP 使用简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,webserver,net,http
description: 除去细节，理解 HTTP 构建的网络应用只要关注客户端和服务端的处理，服务器主要用来接收客户端的请求，然后返回响应，在接收请求并处理的过程中，最重要的莫过于路由 (Router)。这里简单介绍如何使用 `net/http` 构建 HTTP 请求。
---

除去细节，理解 HTTP 构建的网络应用只要关注客户端和服务端的处理，服务器主要用来接收客户端的请求，然后返回响应，在接收请求并处理的过程中，最重要的莫过于路由 (Router)。

这里简单介绍如何使用 `net/http` 构建 HTTP 请求。

<!-- more -->

## 示例

如下是一个最简单的示例。

{% highlight go %}
package main

import (
        "fmt"
        "net/http"
)

func IndexHandler(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintln(w, "hello world")
}

func FoobarHandler(w http.ResponseWriter, r *http.Request) {
        w.Write([]byte("Hi, foobar\n"))
}

func main() {
        http.HandleFunc("/", IndexHandler)
        http.Handle("/foobar", http.HandlerFunc(FoobarHandler))
        http.ListenAndServe("127.0.0.1:8000", nil)
}
{% endhighlight %}

然后可以通过 `curl http://127.0.0.1:8000` 访问。

### 错误处理

通常有几种方式。

#### http.Error()

在标准的 `net/http` 库中提供了一个 `Error()` 函数，可以通过如下方式使用。

{% highlight go %}
func ServeHTTP(w http.ResponseWriter, r *http.Request) {
    err := riskyFunc(r)
    if err != nil {
        http.Error(w, err.Error(), http.StatusInternalServerError)
        return
    }
}
{% endhighlight %}

此时的错误信息将直接返回给用户，如果有些系统相关的信息返回那么可能会导致部分安全问题。

其中 `Error()` 函数在 `net/http/server.go` 中定义，代码如下。

{% highlight go %}
func Error(w ResponseWriter, error string, code int) {
        w.Header().Set("Content-Type", "text/plain; charset=utf-8")
        w.Header().Set("X-Content-Type-Options", "nosniff")
        w.WriteHeader(code)
        fmt.Fprintln(w, error)
}
{% endhighlight %}

#### 自定义

实现也很简单。

### Middleware

<!--
大多数现代Web组件栈允许通过栈式/组件式中间件“过滤”请求，这样就能干净地从web应用中分离出横切关注点（译注：面向方面程序设计中的概念？）。 本周我尝试在Go语言的http.FileServer中植入钩子，发现实现起来十分简便，让我非常惊讶。

让我们从一个基本的文件服务器开始说起：

func main() {
    http.ListenAndServe(":8080", http.FileServer(http.Dir("/tmp")))
}

这段程序会在端口8080上开启一个本地文件服务器。那么我们该如何在这其中植入钩子从而能够在文件请求处理之前执行一些代码？来看一下http.ListenAndServe的方法签名：

func ListenAndServe(addr string, handler Handler) error

看起来http.FileServer返回了一个Handler---给定一个根目录就能知道如何处理文件请求。那我们来看看Handler接口：

type Handler interface {
    ServeHTTP(ResponseWriter, *Request)
}

任何对象只要实现了 `ServeHTTP()` 就是一个 Handler，。那么似乎我们需要做的事情就是构造一个自己的Handler---封装http.FileServer的处理流程。 Go语言的net/http标准库模块内置了一个帮助函数http.HandlerFunc，用于将普通函数转变为请求处理函数（handler）：

type HandlerFunc func(ResponseWriter, *Request)

那么我们这样封装http.FileServer就可以了：

func OurLoggingHandler(h http.Handler) http.Handler {
    return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request)) {
        fmt.Println(*r.URL)
        h.ServeHTTP(w ,r)
    })
}

func main() {
    fileHandler := http.FileServer(http.Dir("/tmp"))
    wrappedHandler := OurLoggingHandler(fileHandler)
    http.ListenAndServe(":8080", wrappedHandler)
}

Go语言的net/http标准库模块有很多内置的处理函数，如TimeoutHandler和RedirectHandler， 可以相同的方式混合匹配使用。
-->

{% highlight go %}
package main

import (
	"fmt"
	"net/http"
)

func LogMidWare(h http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("logging %#v\n", *r.URL)
		h.ServeHTTP(w, r)
	})
}

func main() {
	http.Handle("/", LogMidWare(http.HandlerFunc(FoobarHandler)))
	http.ListenAndServe("127.0.0.1:8000", nil)
}
{% endhighlight %}

## 源码解析

对应的代码在 `$GOPATH/src/net/http` 中实现。

在 `server.go` 中，有默认的 Mux 实现，也就是 `DefaultServeMux` 。

{% highlight go %}
var DefaultServeMux = &defaultServeMux
var defaultServeMux ServeMux
{% endhighlight %}

如果用户没有创建自己的 MUX 那么实际使用的就是该变量。

### Handler注册

默认调用的 `Handler()` 以及 `HandleFunc()` 实现如下。

{% highlight go %}
func Handle(pattern string, handler Handler) {
	DefaultServeMux.Handle(pattern, handler)
}

func HandleFunc(pattern string, handler func(ResponseWriter, *Request)) {
        DefaultServeMux.HandleFunc(pattern, handler)
}
{% endhighlight %}

而根据如上的定义可知，真正调用的是 `ServeMux` 中的实现，上述两个接口真正调用的实际上是 `ServeMux.Handle()` 函数。

而真正有效的是 `mux.m[pattern] = muxEntry{h: handler, pattern: pattern}` 这行代码。

### 监听服务端

在注册完用户需要的 Handler 之后，就开始调用 `http.ListenAndServe()` 监听端口并处理请求。

除此之外，还提供了 TLS 版本的 `http.ListenAndServeTLS()` 接口。

{% highlight text %}
ListenAndServe()
 |-net.Listen() 开始监听
 |-Server.Serve()
   |-Accetp()
   |-Server.newConn() 新建链接
   |-conn.serve() 启动单独的协程开始处理
     |-serverHandler.ServeHTTP() 真正的处理
       |-ServeMux.ServeHTTP() 如果没有定义，那么使用默认的MUX
       | |-ServeMux.Handler() 这里会从Request.URL.Path中取出请求的路径
       | | |-ServeMux.handler()
       | |   |-Servemux.match() 匹配map表中的路由Entry规则，也就是通过Handle()/HandleFunc()注册的回调函数
	   | |-Handler.ServeHTTP() 这里实际上已经实现了MAP的功能，也就是路由
       |-Handler.ServeHTTP() 如果在初始化时有自定义的Handler，这里会调用用户的代码
{% endhighlight %}

对于用户自定义的实现，有两种方式，一种是在新建 http.Server 时设置 Handler 成员，此时需要同时处理 URI 的路由规则。

另外一种，也是最常用的，自定义 MUX 以及 Handler 规则。

## 服务端

对于服务器来说，在接收请求的过程中，最重要的就是路由 (Router)，也就是实现一个 Multiplexer 器。

其中有一个内置的 DefautServeMux，当然也可以自定义，其目的就是为了找到真正的处理函数 (Handler)，并构建 Response 。

### Handler

<!--
用来生成 HTTP 响应的头和正文，实际上

。任何满足了http.Handler接口的对象都可作为一个处理器。通俗的说，对象只要有个如下签名的ServeHTTP方法即可：
-->

<!--
https://segmentfault.com/a/1190000006812688
-->

{% highlight go %}
package main

import (
        "fmt"
        "net/http"
        "time"
)

type customHandler struct {
}

func (cb *customHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintln(w, "Custom Handler!!! URI %s", r.URL.Path)
}

func main() {
        var server *http.Server = &http.Server{
                Addr:           ":8080",
                Handler:        &customHandler{},
                ReadTimeout:    10 * time.Second,
                WriteTimeout:   10 * time.Second,
                MaxHeaderBytes: 1 << 20,
        }
        server.ListenAndServe()
}
{% endhighlight %}

## 参考

<!--
https://www.alexedwards.net/blog/a-recap-of-request-handling
https://blog.csdn.net/xxb249/article/details/80779577
https://www.codetd.com/article/1766635

https://gowebexamples.com/http-server/


## ServeMux VS. Handler

Handler 负责输出 HTTP 响应的头和正文，任何满足了 `http.Handler` 接口的对象都可作为一个处理器。

type Handler interface {
	ServeHTTP(ResponseWriter, *Request)
}

Go 语言的 HTTP 包自带了几个函数用作常用处理器，比如FileServer，NotFoundHandler 和 RedirectHandler。我们从一个简单具体的例子开始：

https://segmentfault.com/a/1190000006812688

rafthttp/transport.go

func (t *Transport) Handler() http.Handler {
	pipelineHandler := newPipelineHandler(t, t.Raft, t.ClusterID)
	streamHandler := newStreamHandler(t, t, t.Raft, t.ID, t.ClusterID)
	snapHandler := newSnapshotHandler(t, t.Raft, t.Snapshotter, t.ClusterID)
	mux := http.NewServeMux() //http 请求路由
	mux.Handle(RaftPrefix, pipelineHandler) /* /raft */
	mux.Handle(RaftStreamPrefix+"/", streamHandler)  /* /raft/stream/ */
	mux.Handle(RaftSnapshotPrefix, snapHandler)      /* /raft/snapshot */
	mux.Handle(ProbingPrefix, probing.NewHandler())  /* /raft/probing */
	return mux
}

func newPeerHandler(cluster api.Cluster, raftHandler http.Handler, leaseHandler http.Handler) http.Handler {
        mh := &peerMembersHandler{
                cluster: cluster,
        }

        mux := http.NewServeMux()
        mux.HandleFunc("/", http.NotFound)
        mux.Handle(rafthttp.RaftPrefix, raftHandler)
        mux.Handle(rafthttp.RaftPrefix+"/", raftHandler)
        mux.Handle(peerMembersPrefix, mh)
        if leaseHandler != nil {
                mux.Handle(leasehttp.LeasePrefix, leaseHandler)
                mux.Handle(leasehttp.LeaseInternalPrefix, leaseHandler)
        }
        mux.HandleFunc(versionPath, versionHandler(cluster, serveVersion))
        return mux
}

-->

为了防止由于压力过大导致雪崩，可以限制客户端的数量，详细可以参考 `golang.org/x/net/netutil/listen.go` 中关于 `LimitListener` 的实现。

{% highlight go %}
{% endhighlight %}
