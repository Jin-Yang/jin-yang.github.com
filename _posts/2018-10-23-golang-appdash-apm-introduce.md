---
title: AppDash APM 使用
layout: post
comments: true
language: chinese
category: [golang,linux]
keywords: appdash,golang,apm,
description:
---

目前的产品架构，分布式系统得到了大范围的应用，使得系统更加灵活，不过同时也给开发、运维人员也带来了很大的难题，如何监控和优化分布式系统的行为。

这里简单介绍下一个小众、简单的 APM 监控工具。

<!-- more -->

## 简介

Google 在 2010 年发表了著名论文《[Dapper, a Large-Scale Distributed Systems Tracing Infrastructure](/reference/devops/Dapper A Large Scale Distributed Systems Tracing.pdf)》([中文版](http://bigbully.github.io/Dapper-translation/))，一个 Google 内部使用的一个分布式系统跟踪基础设施。

这里介绍的 Appdash 是开源的一款用 Go 实现的分布式系统跟踪工具套件，它同样是以 Google 的 dapper 为原型设计和实现的。

业界按照对业务的侵入性排序为 `Pinpoint->Zipkin(Java)->CAT`，Golang 中参考 `Jaeger(Golang)`，这是参考 Zipkin 的实现。

### 常见概念

下图是一个分布式调用的例子，客户端发起请求，请求首先到达负载均衡器，接着经过认证服务、计费服务，然后请求资源，最后返回结果。

![trace example]({{ site.url }}/images/devops/trace-example.png "trace example"){: .pull-center width="50%" }

数据被采集存储后，分布式追踪系统一般会选择使用包含时间轴的时序图来呈现这个 trace。

![trace example timeline]({{ site.url }}/images/devops/trace-example-timeline.png "trace example timeline"){: .pull-center width="50%" }

但在数据采集过程中，部分的系统需要侵入用户代码，那么如果不同系统的 API 不兼容，就会导致如果要切换追踪系统，往往会带来较大改动。

为了提高不同 Trace 系统的兼容性，于是提出了 [OpenTracing](https://opentracing.io/) 标准，用于提供平台无关、厂商无关的 API，使得开发人员能够方便的添加追踪系统的实现。

### Jaeger

目前在对 Opentracing 的实现中，使用比较广的是 Jaeger 和 Zipkin ，Jaeger 是 Uber 推出的一款开源分布式追踪系统。

![jaeger architecture]({{ site.url }}/images/devops/jaeger-architecture.png "jaeger architerure"){: .pull-center width="80%" }

如上所示，Jaeger 的架构分成了如下几个模块：

* Jaeger Client 为不同语言实现了符合 OpenTracing 标准的 SDK，应用程序通过 API 写入数据，然后发送给 Agent 。
* Agent 安装在宿主机上的一个监听 UDP 端口常驻进程，用来接收 span 数据，它会将数据批量发送给 collector。
* Collector 接收 Agent 发送来的数据，然后将数据写入后端存储，被设计成无状态的组件。
* Data Store 后端存储支持将数据写入 Cassandra、Elastic Search。
* Query 接收查询请求，从后端存储系统中检索 trace 并通过 UI 进行展示。


<!--
#### Span

Span指的是一个服务调用的跨度，在实现中用SpanId标识。根服务调用者的Span为根span（root span)，在根级别进行的下一级服务调用Span的Parent Span为root span。以此类推，服务调用链构成了一棵tree，整个tree构成了一个Trace。

Appdash中SpanId由三部分组成：TraceID/SpanID/parentSpanID，例如： 34c31a18026f61df/aab2a63e86ac0166/592043d0a5871aaf。TraceID用于唯一标识一次Trace。traceid在申请RootSpanID时自动分配。

在上面原理图中，我们也可以看到一次Trace过程中SpanID的情况。图中调用链大致是：

frontservice:
        call  serviceA
        call  serviceB
                  call serviceB1
        … …
        call  serviceN

对应服务调用的Span的树形结构如下：

frontservice: SpanId = xxxxx/nnnn1，该span为root span：traceid=xxxxx, spanid=nnnn1，parent span id为空。
serviceA: SpanId = xxxxx/nnnn2/nnnn1，该span为child span：traceid=xxxxx, spanid=nnnn2，parent span id为root span id:nnnn1。
serviceB: SpanId = xxxxx/nnnn3/nnnn1，该span为child span：traceid=xxxxx, spanid=nnnn3，parent span id为root span id:nnnn1。
… …
serviceN: SpanId = xxxxx/nnnnm/nnnn1，该span为child span：traceid=xxxxx, spanid=nnnnm，parent span id为root span id:nnnn1。
serviceB1: SpanId = xxxxx/nnnn3-1/nnnn3，该span为serviceB的child span，traceid=xxxxx, spanid=nnnn3-1，parent span id为serviceB的spanid：nnnn3

【Event】

个人理解在Appdash中Event是服务调用跟踪信息的wrapper。最终我们在Appdash UI上看到的信息，都是由event承载的并且发给Appdash Server的信息。在Appdash中，你可以显式使用event埋点，吐出跟踪信息，也可以使用Appdash封装好的包接口，比如 httptrace.Transport等发送调用跟踪信息，这些包的底层实现也是基于event的。event在传输前会被encoding为 Annotation的形式。

【Recorder】

在Appdash中，Recorder是用来发送event给Appdash的Collector的，每个Recorder会与一个特定的span相关联。

【Collector】

从Recorder那接收Annotation（即encoded event）。通常一个appdash server会运行一个Collector，监听某个跟踪信息收集端口，将收到的信息存储在Store中。
-->


## AppDash

可以直接从 [sourcegraph.com](http://sourcegraph.com/sourcegraph/appdash) 上下载，在 `appdash/cmd` 中包含了一个简单的示例，也可以通过如下命令安装。

{% highlight text %}
$ go get -u sourcegraph.com/sourcegraph/appdash/cmd/...
{% endhighlight %}

另外，appdash 自带一个服务端的示例，在源码的 `examples/cmd/webapp` 目录下，在编译执行 webapp 会看到如下结果：

{% highlight text %}
$ webapp
... ... Appdash web UI running on HTTP :8700
[negroni] listening on :8699
{% endhighlight %}

这一示例包含了 appdash server、front service、backend service，直接运行上述编译后的程序，然后打开 [http://localhost:8700](http://localhost:8700) 连接就可以看到 appdash server 的 UI 界面，一个非常简单的 trace 页面展示。

然后访问 [http://localhost:8699/](http://localhost:8699/) ，就可以触发了一次 trace，然后在 appdash server UI 可以看到如下界面。

![appdash]({{ site.url }}/images/go/appdash-webapp.png "appdash"){: .pull-center width="90%" }

## 参考

<!--
https://studygolang.com/articles/4473
-->

* [Uncertainty in Aggregate Estimates from Sampled Distributed Traces](/reference/devops/Uncertainty in Aggregate Estimates from Sampled Distributed Traces.pdf)
* [The OpenTracing Semantic Specification](https://github.com/opentracing/specification/blob/master/specification.md) OpenTracing 标准的定义，规定了不同语言间需要实现的函数、类型等。

{% highlight text %}
{% endhighlight %}
