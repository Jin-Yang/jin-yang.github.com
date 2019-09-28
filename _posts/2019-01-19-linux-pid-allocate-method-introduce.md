---
title: Linux PID 分配方法
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,pid
description:
---

在 Linux 系统中，每个进程都有一个非负整型表示的唯一进程 ID，虽然在主机级别是唯一的，但是进程的 ID 可以重用，一个进程停止后，其它的进程可以复用该 ID 。

Linux 采用延迟重用的算法，在大部分场景下会使得新进程 ID 不同于最近终止进程所使用的 ID，以防止将新进程误认为是使用同一 ID 的某个已终止的先前进程。

这里主要讨论了 Linux 中分配进程 ID 的方法以及源码实现。

<!-- more -->

## PID 分配

在 Linux 系统中，内核分配 PID 的范围是 `(RESERVED_PIDS, PID_MAX_DEFAULT)`，因为需要支持 namespace ，实际上在每个 namespace 中是唯一的，PID 依次连续分配，也就意味着不同的 namespace 可能会出现相同的 PID 。

### 修改最大 PID

Linux 中设置了系统最大 PID 的大小，在内核中对应了 `kernel/pic.c` 文件中的全局变量 `int pid_max` ，默认通过宏 `PID_MAX_DEFAULT` 指定。

在用户空间可以通过如下的方式进行查看和修改。

{% highlight text %}
----- 查看当前系统最大的PID
$ cat /proc/sys/kernel/pid_max
32768    # <-> 0x8000
$ sysctl kernel.pid_max
32768

----- 修改最大PID值，如下的方式只是临时生效
# echo 65536 > /proc/sys/kernel/pid_max
# sysctl -w kernel.pid_max=65536
{% endhighlight %}

如果需要持久化需要修改配置文件 `/etc/sysctl.conf` ，添加 `kernel.pid_max=65536` 配置即可。

## 内核实现

Linux 内核中通过位图实现 PID 的分配，每个页 (一般是4096Bytes) 作为一个 pidmap ，然后每个位用来标示一个进程 ID ，所以单个页可以支持 `4096 * 8 = 32768` 个进程，这也是默认的最大进程数。

因为支持 PID 的 namespace 功能，所以实际分配是针对的 ns 级别，每次尝试从最近一次 PID 开始分配，可以通过 `cat /proc/loadavg` 文件最后一个值查看。

### 调用流程

主要包含了两部分，初始化和真正的开始分配 PID 。

在系统启动时，会通过 `pidmap_init()` 函数做些初始化，包括了 PID 的最大、最小值等信息。

Linux 中在 fork 进程时，详细来说是在 `copy_process()` 过程中，当执行完父进程相关信息的复制之后，紧接着便是执行 `alloc_pid()` 方法去分配子进程的 PID 。

<!--
### alloc_pid
-->

## 总结

* PID 分配上限可以通过 `/proc/sys/kernel/pid_max` 查询，一般默认为 32768。
* 对于 `PID<300` 的情况只允许分配一次，一般对应了系统线程，所以一般进程 PID 分配范围是 `(300, 32768)` 。
* 每个 PID 分配成功后，会将当前的 PID 设置为 `last_pid`，下次便从 `last_pid + 1` 开始往下查找。
* 通过位图记录已分配和未分配 PID，单页为 4KB ，对应了默认的最大进程数 32768 。

注意，应为 `last_pid` 的作用，对于当大于 `last_pid + 1` 的进程被杀并回收改 PID 之后，如果再创建新进程，很有可能会复用 PID 。

<!--
http://gityuan.com/2017/08/06/linux_process_pid/
https://www.cnblogs.com/hazir/p/linux_kernel_pid.html
-->

{% highlight text %}
{% endhighlight %}
