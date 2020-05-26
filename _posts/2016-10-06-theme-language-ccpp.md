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

## 程序基本概念

不只是 C 相关代码，实际上是 Linux 中的基本介绍。

* [程序简介](/post/program-exec-basic-concept-introduce.html) 与执行程序相关的一些基本概念，例如 ELF 格式、内存分布等。
* [编译链接](/post/program-c-complie-link.html) 编译链接过程，以及静态库和动态库基本概念的介绍。
* [ELF 详解](/post/program-c-elf-details.html) 介绍 ELF 文件格式的详细内容。
* [安全编译选项](/post/program-c-gcc-security-options.html) GCC 提供的一系列安全编译的选项，简单介绍。
* [解析二进制](/post/program-executable-binary-parse-tools.html) 分析二进制文件时常用的命令，例如readelf、nm、ldd等。

### 程序加载

详细介绍从 Bash 启动运行，到最终调用到函数执行。

* [加载过程](/post/program-c-load-process.html) 包括从Bash到内核加载，再到用户态的ld加载解析。
* [Preload 简介](/post/program-c-preload-introduce.html) 允许应用在加载其它动态库之前先加载，可以用来替换某些函数。
* [PLT GOT 介绍](/post/program-c-load-process-plt-got.html) 介绍动态库在运行过程中如何寻找到函数的地址。

#### 其它

* [Linux 系统调用](/post/kernel-syscall.html) 关于系统调用的介绍，同时也包括了 VDSO 。

## C 语言

* [数据对齐](/post/language-c-structure-align-basic-introduce.html) 结构体以及内存的对齐方式。
* [位域和字节序](/post/language-c-bit-field-and-endian-introduce.html) 位域或位段相关概念，以及大小端字节序相关的内容。
* [inline 简介](/post/language-c-inline-concept-introduce.html) 有点类似于 C 中的宏，但是又有所区别。


### 杂项

* [C 语言字符串](/post/program-c-string-stuff.html) 与字符串相关的函数，例如格式化、查找、转换等等。
* [C 语言通配符](/post/program-c-string-linux-wildcard-introduce.html) 也就是 Linux 中与通配符相关的内容。
* [C 网络编程](/post/program-c-network.html) 简单介绍在 Linux C 中进行网络编程时常用到的一些技巧。
* [C 函数指针](/post/program-c-tips-function-pointer.html) 相比来说函数指针是比较复杂的了，这里简单介绍。
* [C 语言的奇技淫巧](/post/program-c-tips.html)，整理下 C 语言中常用的技巧。
* [Linux IO 多路复用](/post/linux-program-io-multiplexing.html)，通过 IO 多路复用提高系统性能，包括了 select、poll、epoll 。
* [Linux AIO](/post/linux-program-aio.html)，简单介绍下 Linux 平台下的异步读写模型。


## 工具集

### 自动编译

* [Makefile](/post/linux-makefile-auto-compile-introduce.html) 也就是最基本的 Makefile ，其它工具一般最后都是生成该文件。
* [AutoTools](/post/linux-autotools-auto-compile-introduce.html) 比较老也是比较经典的自动编译工具。
* [CMake](/post/linux-cmake-auto-compile-introduce.html) 最常用的自动编译工具。
* [Linux Package 管理](/post/linux-package-config-introduce.html) Linux 提供的一种工具，通常在动态库编译时的参数配置。

### gcc

#### 其它

* [特性定制](/post/program-c-language-gcc-some-stuff.html) 对函数、告警等选项的定制，主要是 pragma、attribute 属性的介绍。
* [常用技巧](/post/language-c-gcc-some-basic-tips.html) 常用的参数、64位编译32位代码等等。

### gdb

* [GDB 基本使用](/post/program-c-gdb-basic-usage-introduce.html) 一些常用的使用方法。
* [GDB 栈帧简介](/post/program-c-gdb-stack-frame-introduce.html) 函数传参、局部变量使用的方式。
* [GDB 死锁分析](/post/program-c-gdb-deadlock-analyze-introduce.html)
* [GDB INIT 使用](/post/program-c-gdb-init-scripts-introduce.html)
* [DWARF 格式简介](/post/program-c-gdb-dwarf-format-introduce.html)

<!--
https://segmentfault.com/a/1190000020465136
-->

### 测试

* [Linux C Mock Wrap](/post/linux-c-mock-wrap-unit-test.html) 介绍在 C 中如何进行单元测试。
* [C 代码覆盖率](/post/language-c-coverage-basic-introduce.html) C 语言中所使用的代码覆盖率工具。
* [C/C++ 竞态检查](/post/language-c-some-sanitizers-introduce.html) 一些常见的 Sanitizer 介绍，包括其原理。
* [Fuzzing 测试](/post/program-c-fuzzing-test-introduce.html) 也就是模糊测试，介绍一些基本原理以及使用方法。

### 其它

介绍一些乱七八糟的东西。

* [GCC 强弱符号、引用](/post/program-c-strong-weak-symbol-reference.html) 一些 GCC 中与强弱符号、引用的相关介绍。
* [替换 glibc malloc](/post/linux-c-program-replace-glibc-memory-function-introduce.html) 简单介绍如何替换掉 glibc 中的一系列内存相关函数。
* [GNU 内联汇编](/post/linux-c-gnu-inline-assembly-language-introduce.html) GNU 中如何所用内联汇编。
* [Volatile 使用简介](/post/linux-c-volatile-statement-introduce.html) C 语言中与 volatile 相关的介绍。
* [Linux 时间函数](/post/linux-timer-functions.html) 介绍下 Linux 中与时间相关的函数以及如何选择。
* [Linux 信号安全](/post/linux-signal-safe-introduce.html) 一般会在信号处理里打印日志，不过也可能会因此导致发生死锁。
* [Socket 关闭方式](/post/language-c-socket-close-method.html) 主要介绍了通过 close() 以及 shutdown() 方式关闭 Socket 。
* [Linux C Flock 使用](/post/linux-c-flock-introduce.html) Linux 中实现的一个建议性锁，通常用于 PIDFile 的实现。
* [UDP 通讯优化](/post/linux-c-udp-optimize-introduce.html) UDP 并非一个面向连接的协议，与 TCP 不同，有其相关的优化方法。
* [Linux C 网络编程](/post/program-c-network.html) 整理了 Linux C 经常使用的网络编程技巧。
* [Linux C 错误信息](/post/language-c-error-message-usage-introduce.html) C 中在打印错误信息的时候应该注意那些。
* [Linux umask 使用](/post/linux-umask-and-open-introduce.html) 通过 umask 可以在不修改代码直接调整文件的默认打开权限。
* [Linux Fail Points](/post/linux-c-fail-point-introduce.html) BSD 中的一种构造异常的测试机制。

## C++

可以认为是在 C 的基础上添加了面向对象的功能，其编译、链接、调试等基本都可以通过一个工具链完成。

### 基本概念

* [C++ 基本概念](/post/language-cpp-basic-syntax-introduce.html)
* [C++ 构造析构](/post/language-cpp-basic-constructor-destructor-introduce.html)
* [C++ STL 使用](/post/language-cpp-stl-basic-usage-introduce.html)


### 使用详解

* [C++ 内存分布](/post/language-cpp-memory-layout-introduce.html)
* [C++ 内存模型](/post/language-cpp-memory-module-introduce.html) 通过对内存模型的统一约束，提供了跨平台的能力。
* [C++ 内存模型](/post/language-cpp-some-pitfalls.md.html)

### 并发编程

* [C++ 互斥量](/post/language-cpp-concurrency-mutex-introduce.html)
* [C++ 条件变量](/post/language-cpp-concurrency-condition-variable-introduce.html)
* [C++ Promise Future](/post/language-cpp-concurrency-promise-future-introduce.html)


### 其它

* [C++ 右值引用](/post/language-cpp-basic-right-value-reference-introduce.html)
* [C++ 可调用对象](/post/language-cpp-basic-callable-introduce.html) 包括了函数、类、lambda 等。

## libev

libev 是一个基础的高性能事件库，提供了跨平台能力，而且代码很少，使用非常灵活。除了基础的 IO、定时器、信号的处理之外，同时还提供了一些循环中经常使用的 hook 处理，以及常用

* [libev 使用简介](/post/linux-libev.html) 一个 C 编写的高性能事件循环库，简单介绍其使用方法。
* [libev 源码详解](/post/linux-libev-source-code-details-introduce.html) 详细介绍 libev 内部的使用原理。
* [libev 时间处理](/post/linux-libev-timers.html) 简单介绍下 libev 库中与时间相关的内容。
* [libev 信号处理](/post/linux-libev-source-code-signal-process-details.html) 信号的处理非常敏感，如果处理不当很容易出现问题。

## 并发编程

早期 CPU 通过提高主频来提升 CPU 的性能，不过因为工艺问题以及摩尔定律的终结，目前更加倾向于多核心的发展，对于编程来说，更像一个小型的分布式系统，也导致多线程编程要难很多。

另外，为了弥补 CPU 与主存处理速度的差异，在两者之间增加了多级缓存，提升性能的同时，也带来很多编程上的问题，尤其对于 Lock-Free 的编程。

<!--
内存一致性模型（memory consistency model）就是用来描述多线程对共享存储器的访问行为，在不同的内存一致性模型里，多线程对共享存储器的访问行为有非常大的差别。这些差别会严重影响程序的执行逻辑，甚至会造成软件逻辑问题。在后面的介绍中，我们将分析不同的一致性模型里，多线程的内存访问乱序问题。
-->

### 线程编程

* [Linux 线程同步](/post/program-c-linux-pthreads-synchronize.html) 线程编程时经常使用的同步方式，如锁、条件变量、信号量等。
* [Linux 线程编程技巧](/post/program-c-linux-pthreads-tips.html) 常见技巧，例如 TSD 。

<!--
针对thread线程编程的封装
https://github.com/tinycthread/tinycthread
-->

### 硬件基础

* [CPU 硬件介绍](/post/computer-hardware-details-introduce.html)

### 编程方法

并发编程涉及到很多的知识点，包括了从编译器、到 CPU 、内存与 Cache 的关系等等，都有可能会导致代码与实际执行不一致，原则是保证单线程下的一致。

* [Memory Reordering 简析](/post/linux-c-memory-reordering-basic-introduce.html) 包括了编译器乱序、CPU 乱序等，及其实例。
* [内存屏障简析](/post/linux-c-memory-barriers-basic-introduce.html) 也就是为了处理乱序时使用的机制。
* [GCC 原子操作](/post/linux-c-gcc-atomic-operation-introduce.html) GCC 实际上已经提供了一些简单的原子操作，这里简单介绍。
* [Lock Free 编程](/post/linux-c-program-lock-free-queue-introduce.html)


#### 相关资料

* [Preshing on Programming](https://preshing.com/) 一个不过的 Blog 很多关于无锁编程相关的内容。

<!--
## C 跨平台支持

### 目标判断

在编写跨平台 C 代码时，往往需要根据平台区分代码，这就需要依赖编译器中预定义的宏，然后在源代码中利用它们判断目标操作系统。

#### Windows

详细可以参考官方提供的宏定义 [Predefined Macros](https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros)，用于判断操作系统类型的宏如下：

* `_WIN32` 定义为 1 时编译目标是 32 位以及 64 位，可以是 ARM 或 x86；
* `_WIN64` 定义为 1 时编译目标是 64 位 ARM 或 x86。

需要注意的是，对于 64 位 Windows 系统，`_WIN32` 和 `_WIN64` 都会定义。

https://blog.masterliu.net/gcc-predefined-macros/


一个用户的函数trace
https://github.com/namhyung/uftrace
-->


{% highlight text %}
{% endhighlight %}
