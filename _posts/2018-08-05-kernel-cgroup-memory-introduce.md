---
title:
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

<!-- more -->

也就是说 RSS=file_rss + anon_rss

### /proc/[pid]/statm

对应了内核中的 `proc_pid_statm()` 函数，真正计算变量的是在 `proc_pid_statm()` 函数中。

top的SHR=file_rss。实际上，进程使用的共享内存，也是算到file_rss的，因为共享内存基于tmpfs。

所以这里看到比较多的是 `MM_FILEPAGES` 和 `MM_ANONPAGES` 两种类型，那么这两种类型的在什么时候分配的呢？

{% highlight text %}
缺页异常的入口
do_page_fault() arch/x86/mm/fault.c
__do_page_fault()
handle_mm_fault()
__handle_mm_fault()
handle_pte_fault()
do_linear_fault()
do_cow_fault()
 |-do_set_pte()
{% endhighlight %}

<!--


cgroup提供了自动清除能力
https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/resource_management_guide/sec-common_tunable_parameters


#### memory.limit_in_bytes

设置后立即生效，等物理内存使用超过了限制后 failcnt 会加一，然后尝试将物理内存中的数据迁移到 swap 上去，如果空间不足，那么就会触发 OOM 机制。

#### memory.oom_control

用于控制超过内存上限后的行为，默认设置的为 0 ，也就是触发 OOM ；也可以设置为 1 ，此时不会启动 OOM ，当内存不足时会暂停该进程直到有空余的内存之后再继续运行。

同时，该文件中还包含一个只读的 `under_oom` 字段，用来表示当前是否已经进入 OOM 状态，也即是否有进程被暂停了。

注意，root cgroup 的 OOM 是不能被禁用的。

#### memory.force_empty

写入 0 时会立即触发系统尽可能的回收该分组所占用的内存，主要用于移除该 cgroup 前尽快回收该分组所占用的内存，这样在迁移时会快些。

#### memory.swappiness

默认和全局的 swappiness (/proc/sys/vm/swappiness) 一样，功能相同，有一点区别是如果被设置成 0 就算系统配置的有交换空间，也不会使用交换空间。

#### memory.soft_limit_in_bytes

其特点是，当系统内存充裕时该配置不起任何作用；当系统内存吃紧时，系统会尽量的将组的内存限制在该配置值之下，注意是尽量而非 100% 保证。

当系统的内存吃紧时，所有的 cgroup 会一起竞争内存资源，会出现某些组出现内存饥饿的情况。如果配置了 soft limit 那么当系统内存吃紧时，会让超过 soft limit 限制的 cgroup 释放出超过 soft limit 的那部分内存，也有可能更多，这样其它 cgroup 就有了更多的机会分配到内存。

也就是说，这是当系统内存不足时的一种妥协机制，给次等重要的进程设置 soft limit，使得当系统内存吃紧时，把机会让给其它重要的进程。

注意：在系统内存吃紧时，而分组中又收到了内存申请的请求，那么此时会触发内存回收操作，如果是频繁触发，那么会严重影响当前 cgroup 的性能。

#### memory.pressure_level

用来监控该组中内存压力，当压力过大时会尝试先回收部分内存，从而会影响内存的分配速度，通过监控当前 cgroup 的内存压力，可以在有压力时采取一定的行动来改善当前 cgroup 的性能，例如关闭不重要的服务。

目前有三种压力水平：
* low 分配内存之前需要先回收内存中的数据，此时回收的是在磁盘上有对应文件的内存数据。
* medium 系统已经开始频繁使用交换空间了。
* critical 该组快撑不住了，系统随时有可能 kill 掉该组中的进程。


如何配置相关的监听事件呢？和memory.oom_control类似，大概步骤如下：
利用函数eventfd(2)创建一个event_fd
打开文件memory.pressure_level，得到pressure_level_fd
往cgroup.event_control中写入这么一串：<event_fd> <pressure_level_fd> <level>
然后通过读event_fd得到通知
注意： 多个level可能要创建多个event_fd，好像没有办法共用一个（本人没有测试过）

#### memory thresholds

我们可以通过cgroup的事件通知机制来实现对内存的监控，当内存使用量穿过（变得高于或者低于）我们设置的值时，就会收到通知。使用方法和memory.oom_control类似，大概步骤如下：
利用函数eventfd(2)创建一个event_fd
打开文件memory.usage_in_bytes，得到usage_in_bytes_fd
往cgroup.event_control中写入这么一串：<event_fd> <usage_in_bytes_fd> <threshold>
然后通过读event_fd得到通知
-->

{% highlight text %}
{% endhighlight %}
