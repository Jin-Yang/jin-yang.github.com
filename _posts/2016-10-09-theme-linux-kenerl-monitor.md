---
title: 【专题】Linux 监控专题
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->


## 基本监控

记录与监控相关的内容。

* [Linux 监控](/post/linux-monitor.html)，简单记录一下在 Linux 监控中一些比较常见的工具、网站、资料等信息。
* [Linux 监控之 CPU](/post/linux-monitor-cpu.html) 简单介绍下 Linux 中与 CPU 监控相关的内容。
* [Linux 监控之 IO](/post/linux-monitor-io.html)，简单介绍下 Linux 中与 IO 监控、测试相关的内容。

* [Linux 进程监控](/post/linux-monitor-process-introduce.html) 与进程相关的一些监控指标。
* [Linux CGroup 监控](/post/linux-monitor-cgroup-introduce.html)
* [Linux 监控工具](/post/linux-monitor-misc.html) 简单列举一些常见的监控工具 (sar top ps)，以及配置方式等。

### 其它

这里整理下基本的概念。

* [CPU 物理结构](/post/linux-cpu-physical-arch-introduce.html)

## eBPF

在 1992 年，Steven McCanne 和 Van Jacobson 在 BSD Unix 中引入了 Berkeley Packet Filter, BPF 用来在内核态过滤报文，从而减少内核态到用户态复制报文的数量。Linux 在 1997 年，也就是 2.1.75 将该功能引入。

最开始的 BPF 只用于网络包的过滤，而当前除了包过滤，还支持监控调优等等。

* [eBPF 简介](/post/linux-ebpf-basic-usage-introduce.html) 从 BPF 扩展实现 RISC 虚机，提供包过滤、耗时统计、热点分析等功能。
* [BCC 工具使用](/post/linux-ebpf-bcc-tools-introduce.html) 一个使用 Python 编写的工具，使得 eBPF 的使用更加简单有效。

<!--
* [eBPF 从头开始](/post/linux-ebpf-bcc-tools-introduce.html) 使用最基本的工具从头开始编写。
https://bolinfest.github.io/opensnoop-native/
-->

## 内存相关

* [Linux 监控之 Memory](/post/linux-monitor-memory.html) 简单介绍下 Linux 中与 Memory 监控相关的内容。
* [Cache 能否回收](/post/linux-monitor-memory-cache-buffer-introduce.html) 以系统中的 Cache 作为引子，介绍常见的内存使用方式。

## 常用工具

### 性能

可以使用 CPU 性能计数器、tracepoints、kprobes、uprobes 对程序的各个指标进行统计。

* [Perf 使用简介](/post/linux-perf-tools-basic-usage-introduce.html) 随内核发布的一个老牌的性能诊断工具。

### Colletcd

* [Collectd 简介](/post/collectd-introduce.html) 一个 C 语言编写的多线程监控采集程序，对其进行简单的介绍。
* [Collectd 源码解析](/post/collectd-source-code.html) 详细介绍 Collectd 的源码实现。

### 其它

* [Dstat 使用及其原理](/post/details-about-dstat.html) 一个使用 Python 编写的跨平台监控工具。
* [Systemtap 使用简介](/post/linux-systemtap.html) 介绍内核神器 Systemtap 的使用方式，包括了如何使用最新的安全特性。


{% highlight text %}
{% endhighlight %}
