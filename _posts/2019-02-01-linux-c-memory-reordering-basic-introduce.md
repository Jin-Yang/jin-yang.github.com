---
title: Memory Reordering 简析
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: program,linux,memory reordering
description:
---

以 C 语言为例，在编写完源代码之后，需要经过编译，然后在 CPU 上运行，为了提高代码的执行效率，在编译阶段和运行阶段会执行乱序优化，但同时也带来了一些副作用。

这里简单介绍内存乱序的基本概念。

<!-- more -->

![compiler hardware]({{ site.url }}/images/programs/memory-reordering-compiler-hardware.png "compiler harderware"){: .pull-center }

## 简介

简单来说，编译器的开发者和处理器的制造商会遵循一条所谓的中心内存排序原则，也就是 "不能改变单线程程序的行为" 。

因为这一原则，在单线程代码中可以忽略内存乱序，即使在多线程程序中，如果使用了 Mutex、Semaphore 等同步机制，那么仍然可以防止乱序，只有在使用 Lock-Free 时，此时的内存在不受任何互斥保护下被多个线程共享，那么内存乱序的影响才会被看到。

会发生什么样的乱序，是与编译工具和 CPU 相关的。

## 编译乱序

{% highlight text %}
{% endhighlight %}

正常来说，最终读取到的

## CPU 乱序




## 参考

* [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) x86 的编程手册。
* [Weak vs. Strong Memory Models](https://preshing.com/20120930/weak-vs-strong-memory-models/) 对于 Weak 和 Strong 类型的总结。

{% highlight text %}
{% endhighlight %}
