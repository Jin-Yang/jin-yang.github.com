---
title: 内存检查工具
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: 
description:
---


<!-- more -->

## mtrace

`mtrace` 是 GNU 的扩展函数，用来跟踪各种的内存分配，实际上就是对内存分配函数添加 `hook` ，在这些 `hook` 函数中记录内存的申请和释放信息。

需要在程序中调用函数，然后在执行代码前添加 `MALLOC_TRACE` 环境变量，该环境变量指定了 `trace` 信息的保存路径。

{% highlight c %}
#include <mcheck.h>

void mtrace(void);
void muntrace(void);
{% endhighlight %}

一般可以在可能出现内存泄漏的位置添加检查，然后在运行前添加环境变量。

{% highlight text %}
export MALLOC_TRACE="mtrace.out"; ./your-execuate-file
{% endhighlight %}

在 `mtrace.out` 文件中，会记录内存的申请释放信息，格式如下。

{% highlight text %}
@ 程序名称:[内存分配调用的地址] +/- 操作的内存地址 部分参数
{% endhighlight %}

其中 `+` 表示分配；`-` 表示释放。

接着可以通过 `mtrace` 命令进行分析，该命令在 `glibc-utils` 包中，是一个 `perl` 脚本，然后通过 `mtrace your-execuate-file mtrace.out` 进行分析。

<!--
https://www.jianshu.com/p/d9e12b66096a
-->

## 其它

当应用层通过类似 `malloc()` `calloc()` 之类的接口申请了一部分内存之后，会自己维护相关的数据结构，常见的有 `ptmalloc2(glibc)` `dlmalloc` `tcmalloc(google)` 等，其对应的策略也略有不同。

这里仅介绍下 `glibc` 也就是 `ptmalloc2` 的策略。

对于小的内存一般是通过 `sbrk()` 申请的，这一类的内存一般不会释放掉，只有通过 `mmap()` 申请的内存会通过 `unmmap()` 释放掉，返回给操作系统。

<!--
http://my.huhoo.net/archives/2010/05/malloptmallocnew.html
https://blog.csdn.net/u013920085/article/details/52847464
https://stackoverflow.com/questions/2215259/will-malloc-implementations-return-free-ed-memory-back-to-the-system/2417950


Memory Debuggers 列举了一些常用的内存调测工具
https://elinux.org/Memory_Debuggers
-->


{% highlight text %}
{% endhighlight %}
