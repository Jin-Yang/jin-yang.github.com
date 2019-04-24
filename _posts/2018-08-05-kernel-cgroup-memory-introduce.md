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

缺页异常的入口
do_page_fault() arch/x86/mm/fault.c
__do_page_fault()
handle_mm_fault()
__handle_mm_fault()
handle_pte_fault()
do_linear_fault()
do_cow_fault()
 |-do_set_pte()


{% highlight text %}
{% endhighlight %}
