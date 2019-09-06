---
title: 内存屏障简析
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

这里所谓的内存屏障实际上主要是 CPU 提供的方式，包括了防止 CPU 乱序，Cache 的更新操作。

<!-- more -->

## 内存屏障类型

内存操作涉及到了 Load 和 Store 两个，那么两两组合之后可能会出现四种方式的乱序，通过某些 Barriers 可以防止这种乱序。总共有 `LoadLoad` `LoadStore` `StoreLoad` `StoreStore` 四种。

![barrier types]({{ site.url }}/images/programs/barrier-types.png "barrier types"){: .pull-center }

这里，直接通过出现乱序的方式标识，例如 StoreLoad 是为了防止出现 Store 跟着 Load 的乱序。

注意，对于真实的 CPU 来说，上述的四种类型并非都会出现。

内存屏障有两个作用：A) 防止指令发生重排；B) 使缓存中的数据失效。可以通过如下方式简单理解最简单的 Load Store 屏障：

1. 在指令前插入 Load Barrier 可以让高速缓存中的数据失效，强制从新从主内存加载数据；
2. 在指令后插入 Store Barrier 能让写入缓存中的最新数据更新写入主内存，让其它线程可见。

### LoadLoad

假设有如下的操作 `Load1; LoadLoad; Load2`，在 Load2 及后续读取操作要读取的数据被访问前，保证 Load1 要读取的数据被读取完毕。

例如，如下的代码。

{% highlight text %}
if (IsPublished)               // Load and check shared flag
{
	LOADLOAD_FENCE();      // Prevent reordering of loads
	return Value;          // Load published value
}
{% endhighlight %}

这里实际并不关心 `IsPublished` 何时被设置为非 1 ，但是一旦改标志位被设置了，那么就需要通过 LoadLoad 栅栏，确保 Value 的值不会比 IsPublished 更老。

也就是说，此时读取到的 IsPublished 值不一定是最新的，但是一旦读取到了，那么就需要保证 Value 的值不能老于 IsPublished 的值。

### StoreStore

对于如下的操作 `Store1; StoreStore; Store2`，在 Store2 及后续写入操作执行前，保证 Store1 的写入操作对其它处理器可见。

例如，对于如下的代码。

{% highlight text %}
Value = x;                     // Publish some data
STORESTORE_FENCE();
IsPublished = 1;               // Set shared flag to indicate availability of data
{% endhighlight %}

这里确保的是，在设置 IsPublished 为 1 后，之前关于 Value 的修改已经对其它处理器可见，这里的 Value 可以是一个原子操作，也可以是一个结构体。

### LoadStore

假设有操作 `Load1; LoadStore; Store2`，通过屏障可以确保，在 Store2 及后续写入操作被刷出前，保证 Load1 要读取的数据被读取完毕。

真实的 CPU 中，可能 Load1 发生了 Cache Miss ，而 Store2 是 Cache Hit 那么就可能导致 Store2 要早于 Load1 执行，那么就需要屏障保证执行顺序。

实际 CPU 中的实现，实际上也就是 LoadLoad 和 StoreStore 其中之一。

### StoreLoad

对于操作 `Store1; StoreLoad; Load2`，在 Load2 以及后续所有读取操作执行前，保证 Store1 的写入对所有处理器可见。

也就是说，对于 StoreLoad 屏障来说，会确保屏障前所有的 Store 对其它处理器可见，而且屏障之后会将所有在屏障时的更新对处理器可见。

换言之，这个屏障保证了屏障前的所有 Store 操作不会发生在屏障之后的所有 Load 操作之后，因此符合符合顺序一致性。

可以简单理解为，StoreLoad 首先将所有最近的局部修改更新到所有处理器，然后等更新完成后，再将所有的更新拉取到本地。

这也是做同步时，成本消耗最高的。

### 总结

一般来说，Store 的成本要高于 Load 操作的成本，所以导致很多的编译器、CPU 等，当 Load 操作与 Store 操作不相关时，一般会将 Store 操作提前，以优化性能。

其中 StoreLoad 的屏障粒度是最大的，在大多数处理器的实现中，这个屏障是个万能屏障，兼具其它三种内存屏障的功能。但是，即使有了其它的三个屏障，仍然无法解决之前乱序可能出现的 `r1=r2=0` 的问题。

也即是说，StoreLoad 屏障，是唯一可以解决 `r1=r2=0` 乱序的方式。

<!--
关于四种屏障的解释
https://preshing.com/20120710/memory-barriers-are-like-source-control-operations/



对于真实的 CPU ，实际上提供的能力是不同的。


另外，在实际使用时，会有不同方式的栅栏 Barrier ，而最终，这些形式仍然会依赖 CPU 的实现。

1. 使用 gcc 中的内联汇编，常在内核中使用，例如 `asm volatile("mfence":::"memory")` ；
2. 一些 C++11 提供的标准库操作，例如 `load(std::memory_order_acquire)`；
3. 对于 POSIX 提供的同步机制，例如 `pthread_mutex_lock` 。

<<<< Acquire & Release 语义>>>>

在多线程编程中，有可能各个线程之间相互竞争获取资源，也有可能将某些信息传递给某个线程。前者一般会使用锁进行保护，而对后者来说，就需要 Acquire 和 Release 相关的操作了，其主要目的是为了消息的可靠传输。

## 简介

所谓的 Acquire 和 Release 操作是为了防止因为乱序，导致源码的处理逻辑与实际的运行顺序不符，最终导致异常。

#### Read-Acquire

Acquire 用来确保的是读共享内存的顺序性，包括了 Read-Modify-Write 操作以及单纯的读取 (Loads)，一般被称为 Read-Acquire 操作。

用来确保的是该指令 **之后** ，所有从共享内存中读取到的数据是符合代码顺序 (未乱序) 的。

#### Write-Release

Release 确保的写入内存后的顺序性，包括了上述的 RMW 以及单纯的写入 (Stores) 操作，通常被称为 Write-Release 。

确保的是该指令 **之前**，已经更新到内存中的数据对所有的线程是可见的。

### 使用

对于这里的 Acquire 和 Release 语义来说，可以通过上述 CPU 提供的栅栏来实现。需要注意的是，相应的栅栏应该添加到 read-acquire 之后，以及 write-release 之前。




----- 监听UDEV事件信息
# udevadm monitor

----- 查看当前的块设备信息
# lsblk
# udevadm info -a -n /dev/sdb

https://opensource.com/article/18/11/udev


happening before VS. happens before

happens-before 是一个术语，用来描述 C++11 Java 之类后面的软件内存模型，大致的定义为：

假设 A 和 B 表示一个多线程进行的操作，如果 A happens-before B ，也就意味着在 B 进行前，A 对 B 的内存影响能有效的被 B 看到。

#### happens-before 并不意味着 happening before

仍然以之前内存乱序的示例，该示例存在 `<1> happens-before <2>` ，但可能不是顺序执行，也就是没有 `happening before` 。

int A, B;

void foobar(void)
{
	A = B + 1;  // <1>
	B = 0;      // <2>
}

接下来通过 `gcc -O2 -S foobar.c` 将代码生成汇编。

movl	B(%rip), %eax
movl	$0, B(%rip)       // store to B
addl	$1, %eax
movl	%eax, A(%rip)     // store to A

从结果可以看出，先是保存了 B 然后才保存 A 的值。

根据定义，`<1>` 的
https://www.jianshu.com/p/977d27852826
http://dreamrunner.org/blog/2014/07/05/multithreadingxiang-guan-zhu-yu-zong-jie/
-->

{% highlight text %}
{% endhighlight %}
