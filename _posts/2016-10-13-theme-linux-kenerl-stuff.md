---
title: 【专题】Linux 内核专题
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## 基本概念

介绍下 Linux 内核相关的内容。

* [Linux 内核编译](/post/kernel-compile.html)，简单介绍如何手动编译内核。
* [Linux 硬件启动](/post/kernel-hardware-startup.html)，从内核加电之后，简单介绍如何从硬件加载启动。
* [Linux 启动过程](/post/kernel-bootstrap.html)，通过 GRUB 启动之后，然后开始加载内核，再次简单介绍。
* [Linux 内核模块](/post/kernel-modules.html)，简单介绍下 Linux 中的内核模块编写，包括了内核签名机制的配置。
* [Linux 调度系统](/post/linux-kernel-scheduler.html)，与内核的进程调度相关的内容。
* [Linux 进程相关](/post/linux-kernel-process.html)，简单介绍进程相关的东西，如进程创建、优先级、进程之间的关系等。
* [Linux IO 多路复用](/post/linux-program-io-multiplexing.html)，通过 IO 复用，可以有效提高程序的效率，增加吞吐。

* [Linux 系统调用](/post/kernel-syscall.html) 系统包括了用户态和内核态，内核态用于执行一些权限较高的任务。

## 进程相关

* [Linux 进程执行](/post/linux-kernel-process-introduce.html) 与进程相关的操作，主要是执行命令、守护进程等。
* [Linux 进程状态](/post/linux-process-state-introduce.html) 介绍不同的进程状态，以及一些常见的异常处理方法。

## 信号相关

* [Linux 信号机制](/post/kernel-signal-introduce.html) 进程间通信机制中唯一的异步通信机制。
* [Linux 信号安全](/post/linux-signal-safe-introduce.html) 在信号处理函数中一般会打印日志，但也同时引入了死锁的风险。
* [Linux 信号 VS. 线程](/post/linux-signal-vs-thread.html) 线程中一般在指定线程同步方式处理。



{% highlight text %}
{% endhighlight %}
