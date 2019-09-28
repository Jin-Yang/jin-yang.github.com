---
title: Linux 进程监控
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

<!-- more -->

## 内存

通过 top 可以看到三个关于内存的监控指标，包括了 `VIRT` `RES` `SHR` 。

{% highlight text %}
VIRT  --  Virtual Memory Size (KiB)
	The total amount of virtual memory used by the task.  It includes all code,
	data and shared libraries plus pages that have been swapped out and pages
	that have been mapped but not used.
{% endhighlight %}

因为 Linux 使用的是虚拟地址空间，所有地址空间通过映射表维护，也就是 VIRT 包括了所有的虚拟映射表。

包括了一个任务使用的虚拟内存的总量，包括了代码、数据、共享库、已换出的页、已经被映射但是还没被使用的页，也就是包含了所有内存映射的空间，包括了未实际分配的内存空间。

{% highlight text %}
RES  --  Resident Memory Size (KiB)
	The non-swapped physical memory a task is using.
{% endhighlight %}

一个进程实际已经分配的物理内存空间。

### Proc

上述两个指标分别对应了 `/proc/<pid>/stat` 文件中的 23 24 两个字段的值，在 proc 文件系统中，与进程内存统计相关的有如下几个文件，包括了 `/proc/<pid>/stat` `/proc/[pid]/statm` 。

各个字段详细的名称可以通过 `man 5 proc` 查看。

#### /proc/[pid]/stat

{% highlight text %}
/proc/[pid]/stat
(23) vsize  %lu
        Virtual memory size in bytes.
(24) rss  %ld
        Resident Set Size: number of pages the process has
        in real memory. This is just the pages which count
        toward text, data, or stack space.  This does not
        include pages which have not been demand-loaded in,
        or which are swapped out.
{% endhighlight %}

该文件在内核中对应了 `do_task_stat()` 函数，其中 `vsize` 通过调用 `task_vsize()` 获取，而 `rss` 对应了 `get_mm_rss()` ，其函数实现如下。

{% highlight c %}
unsigned long task_vsize(struct mm_struct *mm)
{
	return PAGE_SIZE * mm->total_vm;
}

static inline unsigned long get_mm_rss(struct mm_struct *mm)
{
	return get_mm_counter(mm, MM_FILEPAGES) + get_mm_counter(mm, MM_ANONPAGES);
}
{% endhighlight %}

也就是说，RSS 对应了匿名和基于文件的内存映射。

#### /proc/[pid]/statm

这里包括了详细的内存信息。

{% highlight text %}
  size       (1) total program size
             (same as VmSize in /proc/[pid]/status)
  resident   (2) resident set size
             (same as VmRSS in /proc/[pid]/status)
  share      (3) shared pages (i.e., backed by a file)
  text       (4) text (code)
  lib        (5) library (unused in Linux 2.6)
  data       (6) data + stack
  dt         (7) dirty pages (unused in Linux 2.6)
{% endhighlight %}

该文件对应到内核的 `proc_pid_statm()` 函数。

<!--
https://blog.csdn.net/gfgdsg/article/details/42709943
https://blog.csdn.net/Helloguoke/article/details/21644473
-->

## 其它

### 进程名称

也就是 `/proc/<PID>/stat` 中的第二列的值，在解析该文件时需要处理一些特殊的字符，建议使用反向查找，测试示例如下。

直接将 sleep 的进程复制过来，然后进行测试。

首先是含有空格的，这个比较简单，直接复制然后运行即可。

{% highlight text %}
$ cp /bin/sleep /tmp/foo\ bar
$ /tmp/foo\ bar
$ cat /proc/<PID>/stat
13094 (foo bar) ... ...
{% endhighlight %}

然后是带有小括号的命令，包括了 `(` 或者 `)` ，不过这个在 bash 中解析执行会有问题，所以需要通过 C 代码来调用执行，测试方式如下。

{% highlight text %}
$ cp /bin/sleep /tmp/foo\(bar
$ cat /tmp/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	execl("/tmp/foo(bar", "foo(bar", "1000", NULL);
	return 0;
}
$ gcc -o main /tmp/main.c
$ ./main
$ cat /proc/<PID>/stat
27076 (foo(bar) ... ...
{% endhighlight %}

所以，在使用 `/proc/<PID>/stat` 文件时，应该逆向查找，否则在某些条件下可能会报错。

### 进程名长度

也就是 `/proc/<PID>/stat` 的第二个字段或者是 `/proc/<PID>/comm` ，可以查看 `man 2 prctl` 中关于 `PR_SET_NAME` 字段的介绍，

{% highlight text %}
PR_SET_NAME (since Linux 2.6.9)
    The name can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
{% endhighlight %}

也就是说，需要 16 字节长的缓冲区，包含了 `0` 终止符。

另外，在内核的 `struct task_struct` 中，其中有一个字段 `char comm[TASK_COMM_LEN];`，对应的长度也是 16 字节。

{% highlight text %}
{% endhighlight %}
