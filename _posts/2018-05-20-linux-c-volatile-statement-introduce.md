---
title: C 语言 Volatile 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: linux,c,language,volatile
description: C 语言中的 volatile 关键字大部分都见过，但是很少有人能明确其具体使用的场景。实际上，上层编程很少会用到，一般只有涉及到 IO、中断等与硬件交互的底层编程才会经常使用。这里简单介绍其使用场景，使用方法。
---

C 语言中的 volatile 关键字大部分都见过，但是很少有人能明确其具体使用的场景。实际上，上层编程很少会用到，一般只有涉及到 IO、中断等与硬件交互的底层编程才会经常使用。

这里简单介绍其使用场景，使用方法。

<!-- more -->

## 简介

简单来说，在 C 中，如果将变量定义为 `volatile` 类型，这也就意味着该变量可能随时被修改，每次使用时应该重新读取，而非使用保存在寄存器中的值。

那么这句话如何去理解。

### 编译器

通过编译器将 C 代码转换为机器码，然后可以直接在硬件平台上运行，而不再需要源码。

与其它技术类似，在代码转换过程中，编译器会对代码进行优化，以降低生成机器码量，提高代码的执行速度，常见的优化方法有：A) 将内存变量缓存到寄存器；B) 调整指令顺序充分利用 CPU 。

那么带来的副作用的话就是，有部分非预期内的优化导致原有逻辑运行异常。

简单来说，在线程内读取一个变量时，为提高存取速度，编译器优化时有时会先把变量读取到一个寄存器中；以后，再取变量值时，就直接从寄存器中取值。

线程内修改同样会同步到寄存器，但是其它线程修改了值，对于本线程是不可见的。

### 详解

假设有如下的代码。

{% highlight c %}
uint32 status = 0;
while (status == 0) {
	/* 代码执行逻辑，不会修改 status 的值 */
}
{% endhighlight %}

在 `while` 循环中不会修改 `status` 变量的值，那么对于编译器来说，会认为 `status` 一直不会修改，而直接将代码优化为类似 `while(1)` 的死循环。

但是实际上，`status` 的值会通过内存映射在 IO 设备中被修改。

当然，也可以通过关闭优化修复现在的问题，但是同样会带来其它的影响：

1. 每个编译器的优化方式不同，这段代码可能会不可移植；
2. 仅仅因为这一段代码，可能会降低整个系统的运行效率。

实际上，这一场景就是 `volatile` 经常使用的。

另外，需要注意，上述使用 `uint32` 类型只是为了方便说明，真正使用时，一般都是指针，然后将指针通过映射指向 IO 设备。

### 定义

在 C 语言标准 `ISO/IEC 9899 C11` 的 6.7.3 中，有如下的定义：

> An object that has volatile-qualified type may be modified in ways unknown to the implementation or have other unknown side effects.
>
> A volatile declaration may be used to describe an object corresponding to a memory-mapped input/output port or an object accessed by an asynchronously interrupting function. Actions on objects so declared shall not be ‘‘optimized out’’ by an implementation or reordered except as permitted by the rules for evaluating expressions.

也即是，如上所说的，如果将变量定义为 `volatile` 类型，这也就意味着该变量可能随时被修改，每次使用时应该重新读取。

## 示例

出了上述与底层硬件处理，另外的场景就是全局变量，最常见的有：A) ISR 中断服务中修改全局变量；B) 多线程编程中修改全局变量。在真正使用时，后者会更多。

多线程编程时，可以通过消息队列、共享内存等进行通讯，而全局变量实际上是一个简化后的共享内存。当两个线程通过全局变量共享数据，由于内核调度可能会将两个线程调度到不同的 CPU 核，那么其寄存器、上下文、L1/L2缓存都有可能会缓存该变量的值。

那么为了获取到最新的值，就需要明确告知编译器每次都要获取最新的值。

### 不使用优化

{% highlight c %}
#include <stdio.h>

int main(void)
{
        const int local = 10;
        int *ptr = (int *)&local;

        printf("Initial value of local : %d\n", local);
        *ptr = 100;
        printf("Modified value of local: %d\n", local);

        return 0;
}
{% endhighlight %}

上述代码，通过 `gcc volatile.c -o volatile --save-temps` 进行编译，也就是没有进行优化，生成后的代码会较大。

其中 `--save-temps` 参数会保存中间生成的临时文件，包括了

* `.i` 预处理后的代码。
* `.s` 汇编代码。
* `.o` 编译后机器码。

{% highlight text %}
$ gcc volatile.c -o volatile --save-temps
$ ./volatile
Initial value of local : 10
Modified value of local: 100
$ ls -l volatile.s
-rw-r–r– 1 root root 773 Jan 01 16:21 volatile.s
{% endhighlight %}

### 优化后代码

代码不变，编译方式如下。

{% highlight text %}
$ gcc -O3 volatile.c -o volatile --save-temps
$ ./volatile
Initial value of local : 10
Modified value of local: 10
$ ls -l volatile.s
-rw-r–r– 1 root root 642 Jan 01 16:21 volatile.s
{% endhighlight %}

### 增加声明

{% highlight c %}
#include <stdio.h>

int main(void)
{
        volatile const int local = 10;
        int *ptr = (int *)&local;

        printf("Initial value of local : %d\n", local);
        *ptr = 100;
        printf("Modified value of local: %d\n", local);

        return 0;
}
{% endhighlight %}

{% highlight text %}
$ gcc -O3 volatile.c -o volatile --save-temps
$ ./volatile
Initial value of local : 10
Modified value of local: 100
$ ls -l volatile.s
-rw-r–r– 1 root root 694 Jan 01 16:21 volatile.s
{% endhighlight %}

注意，上述示例，不同的编译器优化的结果也不同，最新的 gcc 即使不添加 `volatile` 仍然可以输出期望的结果。

## 误使用

实际上在内核中有关于 `volatile` 的一些讨论，可以参考 [The trouble with volatile](https://lwn.net/Articles/233479/) 。

简单来说，`volatile` 无法解决一些常见的同步机制 (spin-lock、mutex等) ，所以对于共享数据一定要加锁；在加锁之后，可以保证数据不再是 `volatile` 易变的了，那么也就是没有必要再使用 `volatile` 了。

## 常见场景

相关的使用场景总结如下。

1. mmap 映射后的 IO 空间，或者设备寄存器访问。
2. 在 ISR 通过全局变量访问。
3. 多线程编程中的共享全局变量。


<!--
Understanding “volatile” qualifier in C
https://www.geeksforgeeks.org/understanding-volatile-qualifier-c-set-1-introduction/
-->


{% highlight text %}
{% endhighlight %}
