---
title: C/C++ Sanitizer 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:  sanitizer
description: Sanitizer 英文翻译过来是杀菌剂的意思，Google 提供了很多相关的一些 Sanitizers ，用来检测程序的异常。可以做内存、地址访问、竞态条件的检测，这里简单介绍。
---

Sanitizer 英文翻译过来是杀菌剂的意思，Google 提供了很多相关的一些 Sanitizers ，用来检测程序的异常。

可以做内存、地址访问、竞态条件的检测，这里简单介绍。

<!-- more -->

## 简介

目前有 AddressSanitizer MemorySanitizer ThreadSanitizer LeakSanitizer 几种，不同的编译工具版本，所支持的特性可能有所区别。

### 安装

在 CentOS 中需要安装 `libasan` (AddressSanitizer)、`libtsan` (ThreadSanitizer) 库。

## ThreadSanitizer

又被称为 TSan，是一个检查线程 Data Race 的 C/C++ 工具，集成在新版的 gcc/clang 中，在编译时可以通过添加 `-fsanitize=thread` 参数，那么在运行时会检测出 Data Race 的问题。

所谓的竞态的条件是指，多个线程同时访问相同的变量，而且其中有一个变量会尝试写入。

### 示例

{% highlight c %}
#include <stdio.h>
#include <pthread.h>

int Global;

void *Thread1(void *x)
{
        Global++;
        return NULL;
}

void *Thread2(void *x)
{
        Global--;
        return NULL;
}

int main(void)
{
        pthread_t t[2];
        pthread_create(&t[0], NULL, Thread1, NULL);
        pthread_create(&t[1], NULL, Thread2, NULL);
        pthread_join(t[0], NULL);
        pthread_join(t[1], NULL);
}
{% endhighlight %}

然后通过 `gcc -fsanitize=thread -fPIE -pie -g race.c -o race` 编译，运行的输出结果如下。

{% highlight text %}
==================
WARNING: ThreadSanitizer: data race (pid=24693)
  Read of size 4 at 0x564178840078 by thread T2:
    #0 Thread2 /tmp/main.c:14 (a.out+0x000000000aaf)
    #1 <null> <null> (libtsan.so.0+0x00000002583b)

  Previous write of size 4 at 0x564178840078 by thread T1:
    #0 Thread1 /tmp/main.c:8 (a.out+0x000000000a6a)
    #1 <null> <null> (libtsan.so.0+0x00000002583b)

  Location is global 'Global' of size 4 at 0x564178840078 (a.out+0x000000201078)

  Thread T2 (tid=24696, running) created by main thread at:
    #0 pthread_create <null> (libtsan.so.0+0x000000028e53)
    #1 main /tmp/main.c:22 (a.out+0x000000000b39)

  Thread T1 (tid=24695, finished) created by main thread at:
    #0 pthread_create <null> (libtsan.so.0+0x000000028e53)
    #1 main /tmp/main.c:21 (a.out+0x000000000b18)

SUMMARY: ThreadSanitizer: data race /tmp/main.c:14 in Thread2
==================
ThreadSanitizer: reported 1 warnings
{% endhighlight %}

上述信息显示了在代码的那些地方出错。

需要注意的是：

* 除了加 `-fsanitize=thread` 外，一定要加 `-fPIE -pie` 否则编译报错。
* 通过 `-g` 参数方便显示文件名和行号。
* 如果生成二进制分了编译连接，那么每步都要添加上述参数，并在连接时添加 `-ltsan` 。
* 如果依赖其它静态库，则这些静态库编译时必须指定 `-fPIC` 。

### 实现原理

如果要实现一个竞态条件检测，需要满足如下两点：

1. 知道哪些线程在访问那个内存空间；
2. 知道是否未做同步而直接访问。

在 V2 版本里，是基于 Shadow Memory 机制来实现的，相比来说其效率要更高。

所谓的 Shadow Memory 实际上是原程序内存的镜像，会记录那个线程访问过，以及何时访问的。

在开启了竞态检测之后，会将部分代码插入到原程序中，然后会初始化并维护 Shadow Memory 的内容，这也就是为什么需要依赖 `libtsan` 动态库。

接下来，看看其如何解决上面提到的两个问题。


<!--
#### 那块内存

https://lwn.net/Articles/598486/
-->

## 参考

* [Google Sanitizers](https://github.com/google/sanitizers) 包含了很多的 Sanitizers 工具，很多是 [LLVM](http://llvm.org/) 已经支持。
* [Paper Dynamic Race Detection with LLVM Compiler](https://link.springer.com/chapter/10.1007/978-3-642-29860-8_9) LLVM 编译器实现静态条件检测的论文。

<!--
Introducing the Go Race Detector
https://blog.golang.org/race-detector

https://zhuanlan.zhihu.com/p/38687826
https://zhuanlan.zhihu.com/p/27503041
-->

{% highlight text %}
{% endhighlight %}
