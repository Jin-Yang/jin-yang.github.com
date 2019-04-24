---
title: ETCD 网络模块
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,etcd
description:
---

ECTD 的网络模块分为了两部分，一部分是内部 RAFT 协议交互所使用的，另外一部分对外提供服务的接口，包括了 V2(http) 以及 V3(gRPC) 两种方式。

<!-- more -->



{% highlight text %}
{% endhighlight %}
