---
title: Linux Hang Task 简介
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: hang task,D
description: 长期以来，处于 D 状态的进程都是让人比较烦恼的问题，此时不能接收信号，不能 kill 掉，用户对此基本是无能为力，而且也很难知道发生的原因，一般来说只能重启服务器恢复。 正常来说 D 状态的任务只有在 IO 操作时会有，而且会很快完成，只有在极端的异常场景下才会出现问题，例如磁盘损坏、NFS 的 bug 等等，不过如果驱动写的完善的话，一般会增加超时机制，原则上不会出现永久的 D 状态进程。 也就是说，只有在内核驱动不合理的时候可能会导致进程长期处于 D 状态，无法唤醒，类似于死锁状态。
---

长期以来，处于 D 状态的进程都是让人比较烦恼的问题，此时不能接收信号，不能 `kill` 掉，用户对此基本是无能为力，而且也很难知道发生的原因，一般来说只能重启服务器恢复。

正常来说 D 状态的任务只有在 IO 操作时会有，而且会很快完成，只有在极端的异常场景下才会出现问题，例如磁盘损坏、NFS 的 bug 等等，不过如果驱动写的完善的话，一般会增加超时机制，原则上不会出现永久的 D 状态进程。

也就是说，只有在内核驱动不合理的时候可能会导致进程长期处于 D 状态，无法唤醒，类似于死锁状态。

<!-- more -->

## 简介

针对这种情况，内核在 3.10.0 版本之后提供了 hung task 机制，用来检测系统中长期处于 D 状体的进程，如果存在，则打印相关警告和进程堆栈。

如果配置了 `hung_task_panic` ，则会直接发起 panic 操作，然后结合 kdump 可以搜集到相关的 vmcore 文件，用于定位分析。

其基本原理也很简单，系统启动时会创建一个内核线程 `khungtaskd`，定期遍历系统中的所有进程，检查是否存在处于 D 状态且超过 120s 的进程，如果存在，则打印相关警告和进程堆栈，并根据参数配置决定是否发起 panic 操作。

### 配置项

与 hung task 相关的配置项主要有如下几个，可以直接通过 `echo 'xx' > file` 的方式进行修改。

{% highlight text %}
----- 处于D状态的超时时间，默认是120s
$ cat /proc/sys/kernel/hung_task_timeout_secs

----- 发现hung task之后是否触发panic操作
$ cat /proc/sys/kernel/hung_task_panic

----- 每次检查的进程数
$ cat /proc/sys/kernel/hung_task_check_count

----- 为了防止日志被刷爆，设置最多的打印次数
$ cat /proc/sys/kernel/hung_task_warnings
{% endhighlight %}

### 示例

{% highlight text %}
INFO: task ps:211718 blocked for more than 120 seconds.
"echo 0 > /proc/sys/kernel/hung_task_timeout_secs" disables this message.
[46953.928424] ps              D ffffffff816ac3a9     0 211718 211717 0x00000080
[46953.928426] Call Trace:
[46953.928429]  [<ffffffff816acc49>] schedule+0x29/0x70
[46953.928431]  [<ffffffff816ae5c5>] rwsem_down_read_failed+0xf5/0x170
[46953.928434]  [<ffffffff81341a28>] call_rwsem_down_read_failed+0x18/0x30
[46953.928439]  [<ffffffff816abb60>] down_read+0x20/0x30
[46953.928442]  [<ffffffff811ba6f1>] __access_remote_vm+0x51/0x1f0
[46953.928445]  [<ffffffff811c145f>] access_remote_vm+0x1f/0x30
[46953.928448]  [<ffffffff81286573>] proc_pid_cmdline_read+0x193/0x560
[46953.928452]  [<ffffffff8120f4ff>] vfs_read+0x9f/0x170
[46953.928456]  [<ffffffff812100df>] SyS_read+0x7f/0xe0
[46953.928460]  [<ffffffff816b9195>] system_call_fastpath+0x1c/0x21
{% endhighlight %}

## 内核分析

在内核启动时会在 `hung_task_init()` 函数中启动一个内核线程。

{% highlight text %}
hung_task_init()
 |-atomic_notifier_chain_register() 注册panic通知链，在发生panic时执行相关的操作
 |-kthread_run() 启动一个内核线程，处理函数为watchdog()，线程名为khungtaskd
   |-watchdog()
     |-set_user_nice() 设置为普通优先级，不影响业务运行
	 |-schedule_timeout_interruptible() 设置内核定时器，默认为120s
	 |-atomic_xchg() 检测是否需要运行
	 |-check_hung_uninterruptible_tasks() 真正执行检测
	   |-check_hung_task()
{% endhighlight %}

在 `check_hung_uninterruptible_tasks()` 函数中会设置每次检查的进程数，以及批量检查的上限 `HUNG_TASK_BATCHING` 。

打印调试信息的时候需要在编译时打开 `CONFIG_DEBUG_MUTEXS` 开关。

<!--
https://www.cnblogs.com/muahao/p/7562324.html
-->

### 内核通知链

在 Linux 内核中，各子系统之间有很强的关联关系，例如某些子系统可能对其它子系统产生的事件感兴趣。

为了在各个子系统之间发送信息，Linux 内核引入了通知链技术，只在内核的子系统之间使用，而不能够在内核和用户空间进行事件的通知。

<!--
https://learning-kernel.readthedocs.io/en/latest/kernel-notifier.html
-->

{% highlight text %}
{% endhighlight %}
