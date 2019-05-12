---
title: cgroup 之 cpuset 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: cgroup,cpuset
description: 在内核中可以通过 CPUSET 限制进程只使用某几个 CPU，更准确的说是某个几个逻辑核。
---

在内核中可以通过 CPUSET 限制进程只使用某几个 CPU，更准确的说是某个几个逻辑核。

<!-- more -->

## 基本概念

实际上，在 Linux 内核中，资源调度的单位是逻辑核，如果开启了超线程 (Hyper-Threading)，那么单个 CPU 物理核会模拟出 2 个逻辑核。

CPU 相关的信息可以通过 `/proc/cpuinfo` 查看，例如：

{% highlight text %}
----- 物理 CPU 数量
$ cat /proc/cpuinfo | grep "physical id" | sort | uniq
----- 每块 CPU 的核心数
$ cat /proc/cpuinfo | grep "cores" | uniq
----- 查看主机总的逻辑线程数
$ cat /proc/cpuinfo | grep "processor" | wc -l
{% endhighlight %}

## NUMA

最早 Intel 在 Nehalem 架构上实现了 Non-Uniform Memory Access, NUMA，以取代之前一直使用的 FSB 前端总线的架构，用以对抗 AMD 的 HyperTransport 技术。

除了商业战略的考量之外，实际上内存控制器逐渐从传统的北桥移到了 CPU 中，这样也可以更方便的实现 NUMA 。

在 SMP 多 CPU 架构中，传统上多 CPU 是以总线的方式访问内存，但是这样会导致总线存在资源争用和一致性问题，而且随着 CPU 数量不断增加，总线的争用会更加剧烈，这也就导致 4 核 CPU 的跑分甚至不足 2 核 CPU 的 1.5 倍。

采用这种方案，理论上来说实现 12Core 以上的 CPU 已经没有太大的意义。

另外，如果主机板上如果插有多块物理 CPU (每块 CPU 独占一块面积，一般都有独立风扇)，那么十之八九就是 NUMA 架构。

### 解决方案

![numa arch]({{ site.url }}/images/linux/container/numa-arch-introduce.jpg "numa arch"){: .pull-center }

NUMA 的解决方案模型有点类似于 MapReduce，将 CPU 划分到多个 Node，每个 Node 有自己独立的内存空间，不同的 Node 之间通过高速互联通讯，通道被成为 QuickPath Interconnect, QPI 。

当然，这种方式引入的问题是，如果一个进程 (内存数据库) 所需的内存超过了 Node 的边界，那么就意味着需要通过 QPI 获取另一 Node 中的内存资源，尽管 QPI 的理论带宽远高于传统的 FSB ，但是仍然会导致性能的波动。

### numactl

可以通过如下命令查看。

{% highlight text %}
# numactl --hardware
available: 2 nodes (0-1)
node 0 cpus: 0 1 2 3 4 5 12 13 14 15 16 17
node 0 size: 130677 MB
node 0 free: 1453 MB
node 1 cpus: 6 7 8 9 10 11 18 19 20 21 22 23
node 1 size: 131056 MB
node 1 free: 614 MB
node distances:
node   0   1
  0:  10  21
  1:  21  10
{% endhighlight %}

也就是这个系统总共有 2 个 Node ，每个 Node 各有 16 个 CPU 和 128G 内存。

一个 NUMA 节点包括一个内存节点和属于同一块 CPU 的若干个逻辑核，这里的编号在后面配置 cpuset 时会用到。

其它的一些常用命令。

{% highlight text %}
----- 指定Python程序在Node0上运行
# numactl --cpubind=0 --membind=0 python param
----- 也可以使用所有的资源
# numactl --interleave=all mongod -f /etc/mongod.conf
----- 查看numa的统计状态，如果other_node过高则需要重新规划
# numastat
{% endhighlight %}

<!--
从物理态详解
https://zhuanlan.zhihu.com/p/33621500
CPU性能天梯图
https://www.mydrivers.com/zhuanti/tianti/cpu/index.html

http://cenalulu.github.io/linux/numa/
-->

## 使用

如果要使用 cpuset 控制器，需要同时配置 `cpuset.cpus` 和 `cpuset.mems` 两个文件参数，都是通过短横线和逗号表示的区间，例如 `0-7,16-23`。

{% highlight text %}
# echo "0-1" > /sys/fs/cgroup/cpuset/foobar/cpuset.cpus
# echo "0" > /sys/fs/cgroup/cpuset/foobar/cpuset.mems
{% endhighlight %}

然后将当期的 bash 添加到这个组中，接下来就可以直接启动一个压测命令进行测试了。

{% highlight text %}
# echo $$ > /sys/fs/cgroup/cpuset/tiger/cgroup.procs
{% endhighlight %}

在代码中可以通过 `sched_setaffinity(2)` `mbind(2)` `set_mempolicy(2)` 设置 CPU 和内存的相关性。

## FAQ

### 设置失败

{% highlight text %}
# cgcreate -g cpuset:small
# echo "0-1" > /sys/fs/cgroup/cpuset/small/cpuset.cpus
# cgexec -g cpuset:small sleep 1
cgroup change of group failed
{% endhighlight %}

在官方文档中有如下的介绍。

> Some subsystems have mandatory parameters that must be set before you can move a task into a cgroup which uses any of those subsystems. For example, before you move a task into a cgroup which uses the cpuset subsystem, the cpuset.cpus and cpuset.mems parameters must be defined for that cgroup.

也就是说，作为 CPUSET 策略生效的必要条件，`cpus` 和 `mems` 必须强制指定，分别表示进程被绑定的内核，以及 cpu 的 NUMA 内存节点。

{% highlight text %}
# echo "0" > /sys/fs/cgroup/cpuset/small/cpuset.mems
# cgexec -g cpuset:small sleep 1
{% endhighlight %}

注意，在设置 `cpuset.mems` 需要保证对应的 NUMA 是存在的，可以通过 `numactl -H` 查看。


{% highlight text %}
{% endhighlight %}
