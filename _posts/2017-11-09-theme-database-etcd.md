---
title: 【专题】Etcd 数据库
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![RAFT Logo]({{ site.url }}/images/databases/etcd-logo.png "RAFT Logo"){: .pull-center width="70%" }

PAXOS 算法从 90 年提出到现在已经有二十几年了，不过其流程过于复杂，目前较多的有 Chubby、libpaxos ，以及 Zookeeper 修改后的 Zookeeper Atomic Broadcase, ZAB 。

RAFT 是斯坦福的 Diego Ongaro、John Ousterhout 两人设计的一致性算法，在 2013 年发布了论文 《In Search of an Understandable Consensus Algorithm》，目前已经有近十多种语言的实现，其中使用较多的是 ETCD 。

### 基本概念

* [RAFT 协议简介](/post/raft-consensus-algorithms-introduce.html) 一个为真实世界应用建立的协议，注重落地性和可理解性。

### ETCD

* [ETCD 基本简介](/post/golang-raft-etcd-introduce.html) 主要介绍 ETCD 如何使用，包括安装、部署、使用以及常见的介绍。
* [ETCD 示例源码](/post/golang-raft-etcd-example-sourcode-details.html) 源码中关于如何 RAFT 协议的示例代码，直接使用的是内存数据库。
* [ETCD 源码解析](/post/golang-raft-etcd-sourcode-details.html) 除了上述的示例代码，这里简单介绍其代码的实现。
* [ETCD 网络模块](/post/golang-raft-etcd-sourcode-network.html)
* [ETCD 存储模块](/post/golang-raft-etcd-sourcode-storage.html)
* [ETCD BoltDB 存储](/post/golang-raft-etcd-backend-boltdb.html)
* [ETCD 一致性读](/post/golang-raft-etcd-sourcode-consistent-reading.html)

{% highlight text %}
{% endhighlight %}
