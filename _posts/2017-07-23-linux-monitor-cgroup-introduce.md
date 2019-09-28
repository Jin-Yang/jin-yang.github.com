---
title: Linux CGroup 监控
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: 
description:
---

<!-- more -->

## CPU

如果 CPU 被 cgroup 限制了，那么可以通过 `cpu.stat` 中的 `throttled_time` 进行查看，也就是由于 cgroup 配置的 CPU 不足导致。

另外，还有一种可能是进程经常被抢占，通常是由于当前 CPU 中的资源不足导致，进程之间频繁切换导致，可以通过 `/proc/<PID>/status` 中的 `nonvoluntary_ctxt_switches` 字段查看。

### 监控文件

cgroup 中的 CPU 监控涉及到了几个文件，包括了 `cpuacct.stat` `cpuacct.usage` `cpu.stat` 三个。

`cpuacct.usage` 统计了所有 CPU 核的累加使用时间，单位是纳秒。在 `cpuacct.stat` 中统计了该控制组中进程用户态和内核态的 CPU 使用量，其单位是 `USER_HZ`。 

注意，相比 `cpuacct.stat` 来说，`cpuacct.usage` 的值会更加精确一些。

#### cpu.stat

该文件主要包含了下面三项统计结果。

* `nr_periods` 使用了多少个 `cpu.cfs_period_us` 里面配置的时间周期；
* `nr_throttled` 在上面的这些周期中，有多少次是受到了限制，也就是 cgroup 中的进程在指定的时间周期中用光了它的配额；
* `throttled_time` 进程被限制使用 CPU 持续了多长时间(纳秒)。

<!--
[CPU Accounting Controller](https://www.kernel.org/doc/Documentation/cgroup-v1/cpuacct.txt)
https://www.kernel.org/doc/Documentation/scheduler/sched-bwc.txt
-->

## MEM

cgroup 中对于内存提供了很精细的控制，会按照不同的范围进行划分，例如物理内存 (memory)、物理 + Swap (memsw)、内核TCP (kmem.tcp) 等等。

每个子类又包括了当前内存使用 `usage_in_bytes`、最大使用内存 `max_usage_in_bytes`、内存限制 `limit_in_bytes`，还包括了内存详细的使用情况 `memory.stat` 。

<!--
### memory.stat

内核的模块在分配资源的时候，为了提高效率和资源的利用率，都是通过 slab 来分配的，在 `/proc/slabinfo` 中保存了相关的信息，也可以通过 `slabtop` 查看。

mmap 可以申请内存？
-->

### memory.stat

在 3.10 内核中对应了 `memcg_stat_show()` 函数，

{% highlight text %}
cache		- # of bytes of page cache memory.
rss		- # of bytes of anonymous and swap cache memory (includes
		transparent hugepages).
rss_huge	- # of bytes of anonymous transparent hugepages.
mapped_file	- # of bytes of mapped file (includes tmpfs/shmem)
swap		- # of bytes of swap usage
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
