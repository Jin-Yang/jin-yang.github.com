---
title: CPU 物理架构
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

![cpu logo](/{{ site.imgdir }}/linux/cpu-logo.jpg "cpu logo")

## 简介

最初 CPU 的频率越高，则性能越好，而频率与制程密切相关，但是制程存在一个天花板，目前一般到 10nm，当然也有 7nm 和最新的 5nm ；为了提升性能， cpu 开始采用多核，即在一个封装里放多个 core，而且又发明了超线程技术 Hyper-threading 。

> CPU 制程技术最小能做到多少纳米？
>
> 其实从科技发展的角度来说，并没有绝对的极限，我们能确定的是，芯片制程越小，单位体积的集成度越高，就意味着处理效率和发热量越小。但受制于切割工艺的极限，以目前的情况来看，理论上的制程极限我们还是可以简单的分析出来的。
>
> 我们知道，硅原子大小半径为110皮米，也就是0.11纳米，直径0.22nm。虽然3D晶体管的出现已经让芯片不再全部依赖制程大小，而制程工艺的提升，也意味着会决定3D晶体管横面积大小，不过，在不破坏硅原子本身的前提下，芯片制造目前还是有理论极限的，在0.5nm左右，之所以不是0.2nm，是因为本身硅原子之间也要保持一定的距离。而从实际角度上看，Intel在9nm制程上已经出现了切割良品率低和漏电率低的问题，所以0.5nm这个理论极限在目前的科学技术上看，几乎是不可能的。

目前使用最多的是 SMP 架构。

> 其中有常见的概念，直接复制 WikiPedia 中的解释。
>
> [SMP(Symmetric multiprocessing)](https://en.wikipedia.org/wiki/Symmetric_multiprocessing), involves a multiprocessor computer hardware architecture where two or more identical processors are connected to a single shared main memory and are controlled by a single OS instance.
>
> [NUMA(Non-Uniform Memory Access)](https://en.wikipedia.org/wiki/Non-Uniform_Memory_Access), is a computer memory design used in multiprocessing, where the memory access time depends on the memory location relative to a processor. Under NUMA, a processor can access its own local memory faster than non-local memory, that is, memory local to another processor or memory shared between processors. NUMA architectures logically follow in scaling from symmetric multiprocessing (SMP) architectures.

NUMA 常见问题可参考 [Frequently Asked Questions About NUMA](http://lse.sourceforge.net/numa/faq/index.html)，或者 [本地文档](/reference/linux/monitor/NUMA Frequently Asked Questions.html) 。

### 内存模型

通常有两种不同的内存管理方式：

* UMA(Uniform Memory Acess)，将可用的内存以连续的方式组织起来，可能有小的缺口，每个处理器访问各个内存区的速度都是一样的；
* NUMA(Non-Uniform Memory Access)，系统中每个处理器都有本地内存，可以支持特别快速的访问，各个处理器之间通过总线连接起来，以支持对其他的处理器的内存访问。

在 CentOS 中可以通过如下命令查看是否为 NUMA。

{% highlight text %}
# yum install numactl -y             # 安装numactl命令
# numactl --hardware
available: 2 nodes (0-1)             # 当前机器有2个NUMA node，编号0-1
node 0 cpus: 0 1 2 3
node 0 size: 8113 MB                 # node 0物理内存大小
node 0 free: 182 MB
node distances:                      # node距离，可简单认为是CPU本node内存访问和跨node内存访问的成本
node   0   1                         # 可知跨node的内存访问成本(20)是本地node内存(10)的2倍
  0:  10  20
  1:  20  10

$ grep -i numa /var/log/dmesg
[xxxxx] No NUMA configuration found  # 非NUMA，否则是
{% endhighlight %}

当然如果开机时间过长，可能会导致日志已经被覆盖，此时可以通过如下方式查看。

{% highlight text %}
$ cat /proc/cpuinfo | grep "physical id" | wc -l     # 查看当前计算机中的物理核个数
$ ls /sys/devices/system/node/ | grep node | wc -l   # 有多少个node

$ lscpu -p                                           # 查看CPU架构
{% endhighlight %}

在 /proc/cpuinfo 中的 physical id，描述的就是 Socket 的编号。

如果物理核数目有多个，而且 node 的个数也有多个，说明这是一个 NUMA 系统，每个目录内部有多个文件和子目录描述该 node 内 cpu、内存等信息，比如说 node0/meminfo 描述了 node0 内存相关信息；如果 node 的个数只有 1 个，说明这是一个 SMP 系统。

关于 NUMA 的详细信息可以通过 dmidecode 查看。


## 核数信息

下图展示了与核相关的概念，显示的是 NUMA 节点，也可以理解为物理核。

![cpu topology](/{{ site.imgdir }}/linux/node-cpu-topology.jpg "cpu topology"){: width="250px" }

如果是一个 NUMA node 节点，那么会包括一个或者多个 Socket，以及与之相连的 local memory；一个多核的 Socket 有多个 Core，如果 CPU 支持 HT，OS 还会把这个 Core 看成 2 个 Logical Processor。

### 查看信息

最简单的可以通过 `/proc/cpuinfo` 查看跟 core 相关的信息，这里包括了 socket、物理核、逻辑核的概念。

{% highlight text %}
# cat /proc/cpuinfo | grep "physical id" | sort -u | wc -l   # 显示socket的数目

# cat /proc/cpuinfo
... ...
    cpu cores : 4              # 一个socket有4个物理核
    core id   : 1              # 一个core在socket内的编号，同样是物理核编码
... ...

# cat /proc/cpuinfo | grep "cpu cores" | uniq | cut -d: -f2  # 每个socket的物理核数目

# cat /proc/cpuinfo | grep processor                         # 查看有多少个逻辑核
{% endhighlight %}


### Cache

关于 CPU 中的 Cache 信息，可以通过 /proc/cpuinfo 查看，分为 L1、L2、L3，其中 L1 又分为独立的指令 cache 和数据 cache。

{% highlight text %}
processor       : 0
cache size      : 12288 KB     # L3 Cache的总大小
cache_alignment : 64
{% endhighlight %}

详细的 cache 信息可以通过 sysfs 查看，也即在 ```/sys/devices/system/cpu/``` 目录下，其中 cpu 目录下的编号是逻辑 cpu，且该目录下包括了 index0 (L1数据cache)、index1 (L1指令cache)、index2 (L2 cache)、index3 (L3 cache，对应cpuinfo里的cache) 。

<!--
在 indexN 目录下的内容为：
* type，cache 的类型，分为数据和指令，常见的有 Data、Instruction。
* Level，相应的等级。
* Size，大小。
coherency_line_size * physical_line_partition * ways_of_associativity * number_of_sets = size 。
shared_cpu_map，标示被那些 CPU 共享。
-->

## 参考

关于 CPU 的顶层布局可以参考 [玩转CPU Topology](http://www.searchtb.com/2012/12/%E7%8E%A9%E8%BD%ACcpu-topology.html)，或者 [本地文档](/reference/linux/monitor/all_about_cpu_topology.mht)；另外一个可以参考的是 [13 种在 Linux 系统上检测 CPU 信息的工具](https://linux.cn/article-5104-1.html?pr)，或者 [本地文档](/reference/linux/monitor/How to check CPU info on Linux.mht)，[check-cpu-info-linux.pdf](/reference/linux/monitor/check-cpu-info-linux.pdf) 。

LIKWID 工具介绍可参考 [Lightweight performance tools](http://tools.zih.tu-dresden.de/2011/downloads/treibig-likwid-ParTools.pdf) 或者 [本地文档](/reference/linux/monitor/treibig-likwid-ParTools.pdf)，源码查看 [Github](https://github.com/RRZE-HPC/likwid) 。

{% highlight text %}
{% endhighlight %}
