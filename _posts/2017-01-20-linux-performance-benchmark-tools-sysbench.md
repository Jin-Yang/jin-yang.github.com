---
title: Sysbench 压测工具
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: sysbench
description: Linux 中有很多压测工具，不同的场景会各有优劣，例如磁盘压测工具 fio 。Sysbench 算是一个比较通用的压测或者基准测试工具，这是一个开源、模块化、跨平台的多线程性能测试工具，可以用来进行 CPU、内存、IO、线程、数据库的性能测试，其中数据库支持 MySQL、Oracle 和 PostgreSQL 。这里简单介绍其使用方法。
---

Linux 中有很多压测工具，不同的场景会各有优劣，例如磁盘压测工具 fio 。

Sysbench 算是一个比较通用的压测或者基准测试工具，这是一个开源、模块化、跨平台的多线程性能测试工具，可以用来进行 CPU、内存、IO、线程、数据库的性能测试，其中数据库支持 MySQL、Oracle 和 PostgreSQL 。

这里简单介绍其使用方法。

<!-- more -->

## 简介

所谓的基准测试，主要是对测试对象，进行定量、可复现、可对比的测试，不关心业务逻辑，相比来说更加简单、直接、易于测试，数据可以由工具生成，不要求真实。

另外一种是压力测试，此时一般需要考虑具体的业务逻辑，需要确保压测模型尽量服务期望场景，所以一般是真实数据。

### 安装

在 CentOS 中，对应的 RPM 包仓库在 EPEL 中，可以通过如下命令安装。

{% highlight text %}
# yum --enablerepo=epel install sysbench
{% endhighlight %}

建议使用上述的二进制安装方式，源码安装要复杂一些。

### 常用命令

基本的使用方式如下。

{% highlight text %}
$ sysbench [options]... [testname] [command]
{% endhighlight %}

对于具体的参数可以通过 `sysbench --help` 查看，包括了常用的参数、内置的压测引擎等。

相关的子类压测，同样有帮助文档，例如 `sysbench --test=cpu help` 查看与 CPU 压测相关的一些帮助文档。

### 通用参数

{% highlight text %}
--num-threads=N            创建测试线程的数目，默认为 1；
--max-requests=N           总请求数，与--max-time选择一个设置即可，默认值为 10000；
--max-time=N               总执行时间，单位为秒，默认值为0，也就是不限制；
--forced-shutdown=STRING   当超过--max-time后强制中断，默认为off；
--thread-stack-size=SIZE   每个线程的stack大小，默认为64K；
{% endhighlight %}

## 系统基准测试

### OS 基准测试

{% highlight text %}
$ sysbench --test=cpu --cpu-max-prime=2000 run
$ sysbench --num-threads=3 --test=cpu --cpu-max-prime=2000000 run
{% endhighlight %}

<!--
## Memory 基准测试

主要是针对不同的块大小进行内存的连续读写或者随机读写测试。

--memory-block-size=SIZE    size of memory block for test [1K]
--memory-total-size=SIZE    total size of data to transfer [100G]
--memory-scope=STRING       memory access scope {global,local} [global]
--memory-hugetlb=[on|off]   allocate memory from HugeTLB pool [off]
--memory-oper=STRING        type of memory operations {read, write, none} [write]
--memory-access-mode=STRING memory access mode {seq,rnd} [seq]

./sysbench --test=memory --memory-block-size=8K --memory-total-size=1G --memory-access-mode=seq run

https://wiki.gentoo.org/wiki/Sysbench
https://www.cnblogs.com/chenmh/p/5866058.html
https://www.cnblogs.com/kismetv/p/7615738.html
-->

{% highlight text %}
{% endhighlight %}
