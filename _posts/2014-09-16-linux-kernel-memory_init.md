---
title: Kernel 内存管理
layout: post
comments: true
language: chinese
category: [misc]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---

Linux 或者 Unix 的设计理念是：一切都是文件！

<!-- more -->


Slab Allocator 是 Linux 内核中的内存分配机制，各内核子系统、模块、驱动程序都会使用，可以从 /proc/meminfo 查看可以回收 (SReclaimable) 以及不可回收 (SUnreclaim) 的内存数，更详细的可以查看 /proc/slabinfo 文件。

另外需要注意的是，slab 其实是一个统称，内核从 2.6.23 版本之后就已经从 Slab 进化成 [Slub](https://www.kernel.org/doc/Documentation/vm/slub.txt) 了；可以直接查看 /sys/kernel/slab 目录是否存在，如果存在就是 slub 否则是 slab 。
http://events.linuxfoundation.org/sites/events/files/slides/slaballocators.pdf
http://events.linuxfoundation.org/images/stories/pdf/klf2012_kim.pdf
http://www.cnblogs.com/tolimit/p/4654109.html
怎样诊断slab泄露问题
http://linuxperf.com/?p=148
https://www.mawenbao.com/research/linux-ate-my-memory.html
http://farll.com/2016/10/high-memory-usage-alarm/
https://linux-audit.com/understanding-memory-information-on-linux-systems/


{% highlight text %}
{% endhighlight %}
