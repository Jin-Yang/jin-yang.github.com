---
title: Linux 内存介绍
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

一般来说，业务进程使用的内存主要有以下几种场景：

* 用户空间的匿名映射页，包括了 `malloc()` `mmap(..., MAP_ANONYMOUS, ...)` 所使用内存，当系统内存不够时，内核可以将这部分内存交换出去；
* 用户空间的文件映射页，包括了 `mmap(..., fd, ..)` `tmpfs` ，前者是指定文件的 mmap ，后者比如 IPC 共享内存，当系统内存不够时，内核可以回收这些页，但回收之前可能需要与文件同步数据；
* 文件缓存，也就是 Cache ，用来缓存程序通过 `read()/write()` 读写文件时，当系统内存不够时，内核可以回收这些页，如果是脏页需要与文件同步数据；
* Buffer Cache，也就是 Buffer ，用来缓存元数据、块设备文件信息等。

其中 1 2 属于进程的 RSS ，3 4 属于 Page Cache。




{% highlight text %}
{% endhighlight %}
