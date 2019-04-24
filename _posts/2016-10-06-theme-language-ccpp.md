---
title: 【专题】C/C++ 编程语言
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![C/C++ Logo]({{ site.url }}/images/ccpp-logo.png "C/C++ Logo"){: .pull-center width="350"}

简单介绍下与 C 语言相关的内容。

<!--* [C 持续集成](/post/program-c-continuous-integration.html)，一些与 C 语言的持续集成相关的工具集。-->

## C 语言

* [C 编译链接](/post/program-c-complie-link.html)，与 C 语言相关的编译链接概念
* [C 加载过程](/post/program-c-load-process.html)，通过动态库可以减小空间，提高效率，这里简单介绍加载过程。
* [GCC 安全编译选项](/post/program-c-gcc-security-options.html) GCC 中提供的一系列安全编译的选项，简单介绍。

#### 杂项

* [C 语言字符串](/post/program-c-string-stuff.html) 与字符串相关的函数，例如格式化、查找、转换等等。
* [C 语言通配符](/post/program-c-string-linux-wildcard-introduce.html) 也就是 Linux 中与通配符相关的内容。
* [C 网络编程](/post/program-c-network.html) 简单介绍在 Linux C 中进行网络编程时常用到的一些技巧。
* [C 函数指针](/post/program-c-tips-function-pointer.html) 相比来说函数指针是比较复杂的了，这里简单介绍。
* [C 语言的奇技淫巧](/post/program-c-tips.html)，整理下 C 语言中常用的技巧。
* [Linux IO 多路复用](/post/linux-program-io-multiplexing.html)，通过 IO 多路复用提高系统性能，包括了 select、poll、epoll 。
* [Linux AIO](/post/linux-program-aio.html)，简单介绍下 Linux 平台下的异步读写模型。




### 测试

* [Linux C Mock Wrap](/post/linux-c-mock-wrap-unit-test.html) 介绍在 C 中如何进行单元测试。
* [C 代码覆盖率](/post/language-c-coverage-basic-introduce.html) C 语言中所使用的代码覆盖率工具。

### 编译

* [Makefile](/post/linux-makefile-auto-compile-introduce.html) 也就是最基本的 Makefile ，其它工具一般最后都是生成该文件。
* [AutoTools](/post/linux-autotools-auto-compile-introduce.html) 比较老也是比较经典的自动编译工具。
* [CMake](/post/linux-cmake-auto-compile-introduce.html) 最常用的自动编译工具。
* [Linux Package 管理](/post/linux-package-config-introduce.html) Linux 提供的一种工具，通常在动态库编译时的参数配置。

### 其它

介绍一些乱七八糟的东西。

* [GCC 常用技巧](/post/program-c-language-gcc-some-stuff.html) 包括了一些 attribute 属性的介绍。
* [GCC 强弱符号、引用](/post/program-c-strong-weak-symbol-reference.html) 一些 GCC 中与强弱符号、引用的相关介绍。
* [替换 glibc malloc](/post/linux-c-program-replace-glibc-memory-function-introduce.html) 简单介绍如何替换掉 glibc 中的一系列内存相关函数。
* [GNU 内联汇编](/post/linux-c-gnu-inline-assembly-language-introduce.html) GNU 中如何所用内联汇编。
* [C 语言 inline 简介](/post/language-c-inline-concept-introduce.html) 有点类似于 C 中的宏，但是又有所区别。
* [Volatile 使用简介](/post/linux-c-volatile-statement-introduce.html) C 语言中与 volatile 相关的介绍。
* [Linux 时间函数](/post/linux-timer-functions.html) 介绍下 Linux 中与时间相关的函数以及如何选择。
* [Linux 信号安全](/post/linux-signal-safe-introduce.html) 一般会在信号处理里打印日志，不过也可能会因此导致发生死锁。
* [Socket 关闭方式](/post/language-c-socket-close-method.html) 主要介绍了通过 close() 以及 shutdown() 方式关闭 Socket 。
* [Linux C Flock 使用](/post/linux-c-flock-introduce.html) Linux 中实现的一个建议性锁，通常用于 PIDFile 的实现。
* [Linux C 位域和大小端](/post/language-c-bit-field-and-endian-introduce.html) C 语言提供的位域以及大小端，常用在通讯协议的处理。
* [UDP 通讯优化](/post/linux-c-udp-optimize-introduce.html) UDP 并非一个面向连接的协议，与 TCP 不同，有其相关的优化方法。

## libev

* [libev 事件库](/post/linux-libev.html)，一个 C 编写的高性能事件循环库，类似库还有 libevent、libubox 等。
* [libev 时间处理](/post/linux-libev-timers.html)，简单介绍下 libev 库中与时间相关的内容。

## 线程编程

* [Linux 线程同步](/post/program-c-linux-pthreads-synchronize.html)，线程编程时经常使用的同步方式，如锁、条件变量、信号量等。
* [Linux 线程编程技巧](/post/program-c-linux-pthreads-tips.html)，简单介绍下 Linux 中与线程相关的编程一些常见技巧。

<!--
针对thread线程编程的封装
https://github.com/tinycthread/tinycthread
-->

## 原子操作

* [GCC 原子操作](/post/linux-c-gcc-atomic-operation-introduce.html) GCC 实际上已经提供了一些简单的原子操作，这里简单介绍。
* [Lock Free 编程](/post/linux-c-program-lock-free-queue-introduce.html)

{% highlight text %}
{% endhighlight %}
