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

一般把编译阶段产生的乱序称为 Compiler Reordering，也即 Software Memory Reordering；把运行阶段产生的乱序称为 CPU Memory Reordering，也叫做 Hardware Memory Reordering，这两者之间区别很大。

## 编译乱序

编译乱序就是在编译阶段，编译器为了优化程序的执行效率，自动将内存操作指令重排，从而使得读写内存的指令与程序定义的操作顺序不一致。

这也就意味着，不同的编译器，甚至不同的参数，最终编译的结果也会有所出入。

例如，对于如下简单的 c 程序。

{% highlight c %}
int A, B;

void foobar(void)
{
	A = B + 1;
	B = 0;
}
{% endhighlight %}

通过命令编译 `gcc -S foobar.c` 得到汇编程序 (语法为 AT&T 可以使用 `-masm=intel` 指定为 Intel 语法)，打开 foobar.s 查看汇编代码如下：

{% highlight text %}
movl	B(%rip), %eax
addl	$1, %eax
movl	%eax, A(%rip)     // store to A
movl	$0, B(%rip)       // store to B
{% endhighlight %}

可以看到，上述的汇编代码严格按照程序中定义顺序执行 load 和 store 指令，即先保存变量 a 的值，后保存变量 b 的值，也就是说，这段代码并没有产生任何内存乱序。

接下来通过 `gcc -O2 -S foobar.c` 重新编译 foobar.c ，其中相关代码如下：

{% highlight text %}
movl	B(%rip), %eax
movl	$0, B(%rip)       // store to B
addl	$1, %eax
movl	%eax, A(%rip)     // store to A
{% endhighlight %}

可以看到，汇编指令先执行变量 `b = 0` 的存储，之后才执行 `a = b + 1` 的操作，表明变量 a 和 b 的 store 操作没有按照他们在程序中定义的顺序来执行，即产生了 Compiler Reordering。

之所以这么优化，是因为读一个在内存中而不是在 Cache 中的共享变量需要很多周期，所以编译器就 "自作聪明" 的让读操作先执行，从而隐藏掉一些指令执行的 Latency，提高程序的性能。

很显然，在单线程的场景下，最终仍然会得到 `a = 1` 以及 `b = 0`，也就是说，即乱序没有影响单线程的执行结果，这也是最基本的原则。

### 防止乱序

{% highlight c %}
int A, B;

void foobar(void)
{
	A = B + 1;
	asm volatile("" ::: "memory");
	B = 0;
}
{% endhighlight %}

然后，通过 `gcc -O2 -S foobar.c` 重新编译 foobar.c ，其中相关代码如下：

{% highlight text %}
movl	B(%rip), %eax
addl	$1, %eax
movl	%eax, A(%rip)  // store to A
movl	$0, B(%rip)    // store to B
{% endhighlight %}

## CPU 乱序

关于 x86 的内存模型可能出现的乱序可以参考 [Intel® 64 Architecture Memory Ordering White Paper](http://www.cs.cmu.edu/~410-f10/doc/Intel_Reordering_318147.pdf) 中的相关介绍，这里介绍其中一种。

假设最终的机器码如下，分别在两个 CPU 上执行。

{% highlight text %}
  Processor #1  |  Processor #2
----------------+----------------
  mov [X], 1    |  mov [Y], 1
  mov r1, [Y]   |  mov r2, [X]
{% endhighlight %}

上面的 r1 和 r2 分别表示 CPU 中的通用寄存器，例如 x86_64 中的 eax 。

正常来说，上述的最终结果应该是 `r1=1 r2=1` ，但实际上根据 Intel 的文档，最终的结果是可能都是 0 的。

简单来说，CPU 是允许上述的执行是乱序的，只要确保在单个线程的执行结果是正确的即可，那么也就意味着，上述的代码可以通过如下的方式执行。

{% highlight text %}
  Processor #1  |  Processor #2
----------------+----------------
  mov r1, [Y]   |
                |  mov r2, [X]
  mov [X], 1    |
                |  mov [Y], 1
{% endhighlight %}

### 测试

使用如下的代码进行测试，可以通过 `gcc -o order -O2 order.c -lpthread` 进行编译。

{% highlight c %}
#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define USE_CPU_FENCE        0
#define USE_RAND_DELAY       0
#define USE_SINGLE_HW_THREAD 0

#define MT_IA        397
#define MT_LEN       624

#if USE_RAND_DELAY
/* Mersenne Twister */
struct mt {
	int index;
	unsigned int buffer[MT_LEN];
};

unsigned int mt_integer(struct mt *mt)
{
	// Indices
	int i1, i2, j;
	unsigned int s, r;

	i1 = mt->index;
	i2 = mt->index + 1;
	j = mt->index + MT_IA;

	if (i2 >= MT_LEN)
		i2 = 0;      // wrap-around
	if (j >= MT_LEN)
		j -= MT_LEN; // wrap-around

	// Twist
	s = (mt->buffer[i1] & 0x80000000) | (mt->buffer[i2] & 0x7fffffff);
	r = mt->buffer[j] ^ (s >> 1) ^ ((s & 1) * 0x9908B0DF);
	mt->buffer[mt->index] = r;
	mt->index = i2;

	// Swizzle
	r ^= (r >> 11);
	r ^= (r << 7) & 0x9d2c5680UL;
	r ^= (r << 15) & 0xefc60000UL;
	r ^= (r >> 18);

	return r;
}

void mt_init(struct mt *mt, unsigned int seed)
{
	int i;

	/*
	 * Initialize by filling with the seed, then iterating
	 * the algorithm a bunch of times to shuffle things up.
	 */
	for (i = 0; i < MT_LEN; i++)
		mt->buffer[i] = seed;
	mt->index = 0;
	for (i = 0; i < MT_LEN * 100; i++)
		mt_integer(mt);
}
#endif

sem_t begin1, begin2, end;
int X, Y;
int r1, r2;

void *worker1_func(void *arg)
{
	(void) arg;
#if USE_RAND_DELAY
	struct mt mt;
	mt_init(&mt, 1);
#endif
	for (;;) {
		sem_wait(&begin1);  // Wait for signal
#if USE_RAND_DELAY
		while (mt_integer(&mt) % 8 != 0);  // Random delay
#endif

		X = 1;
#if USE_CPU_FENCE
		asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
		asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
		r1 = Y;

		sem_post(&end);  // Notify transaction complete
	}
	return NULL;  // Never returns
};

void *worker2_func(void *arg)
{
	(void)arg;
#if USE_RAND_DELAY
	struct mt mt;
	mt_init(&mt, 2);
#endif
	for (;;) {
		sem_wait(&begin2);  // Wait for signal
#if USE_RAND_DELAY
		while (mt_integer(&mt) % 8 != 0);  // Random delay
#endif

		Y = 1;
#if USE_CPU_FENCE
		asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
		asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
		r2 = X;

		sem_post(&end);  // Notify transaction complete
	}
	return NULL;  // Never returns
};

int main(void)
{
	pthread_t thd1, thd2;
	int detected = 0, iterations;

	sem_init(&begin1, 0, 0);
	sem_init(&begin2, 0, 0);
	sem_init(&end, 0, 0);

	pthread_create(&thd1, NULL, worker1_func, NULL);
	pthread_create(&thd2, NULL, worker2_func, NULL);

#if USE_SINGLE_HW_THREAD
	cpu_set_t cpus;

	CPU_ZERO(&cpus);
	CPU_SET(0, &cpus);
	pthread_setaffinity_np(thd1, sizeof(cpu_set_t), &cpus);
	pthread_setaffinity_np(thd2, sizeof(cpu_set_t), &cpus);
#endif

	for (iterations = 1; ; iterations++) {
		X = 0;
		Y = 0;

		// Signal both threads
		sem_post(&begin1);
		sem_post(&begin2);

		// Wait for both threads
		sem_wait(&end);
		sem_wait(&end);

		// Check if there was a simultaneous reorder
		if (r1 == 0 && r2 == 0) {
			detected++;
			printf("%d reorders detected after %d iterations\n", detected, iterations);
		}
	}

	return 0;
}
{% endhighlight %}

如上的代码逻辑很简单，工作线程用来处理上述的逻辑，主进程进行同步以及触发。

在工作线程中，每次真正执行处理逻辑时，会添加随机的延迟，不过会导致出现乱序的概率会有效的降低，可以将其关闭之后观察。

另外，为了防止由于代码的优化导致上述的场景失效，最好检查最终生成的汇编代码。

{% highlight text %}
$ gcc -O2 -c -S ordering.c
worker1_func:
	... ...
	movl	$begin1, %edi
	call	sem_wait
	movl	$1, X(%rip)
	movl	Y(%rip), %eax
	movl	$end, %edi
	movl	%eax, r1(%rip)
	... ...
{% endhighlight %}

如上生成的是 AT&T 格式的汇编，也可以通过 `-masm=intel` 参数指定是 Intel 格式。

另外需要注意的是，在所有的平台上，信号量会保持 Release 和 Require 的语义，也就是说，所有写入共享内存的在 `sem_post()` 执行后可见，而读取在 `sem_wait()` 后可见。

在这里，也就意味着，在工作线程中，可以确保 `X=0 Y=0` 在真正执行处理逻辑前已经同步，而结果在执行完之后对所有线程可见。

如下是在 CentOS 上的执行结果。

{% highlight text %}
$ ./ordering
1 reorders detected after 57768 iterations
2 reorders detected after 101070 iterations
3 reorders detected after 130491 iterations
4 reorders detected after 130766 iterations
5 reorders detected after 135479 iterations
6 reorders detected after 142153 iterations
... ...
{% endhighlight %}

### 解决方案

最简单的，可以将两个线程绑定到同一个核中，这是因为在同一个核中不会出现乱序，即使多个线程在循环的调度执行。

另外，也可以添加一个 Barrier 防止乱序，这里解决的是 Store 之后做 Load 操作，也就是 StoreLoad Barrier ，不过在 Intel 中 `mfence` 会防止所有的乱序出现。

## 其它

### CPU Memory Barrier

理论上来说，对于不同的硬件内存模型，可能产生的内存乱序的种类并不相同 (实际上，主要是由于 Write Buffer 的存在而产生)。

按照 Load+Store 的四种排列方式，也就是四种不同的 CPU 内存乱序，也应该存在对应的四种 barrier。然而针对 Intel CPU 而言，主要有三种内存 Barrier 。

{% highlight text %}
Store Barrier   确保 Barrier 前后的 store 操作不会发生乱序;
Load Barrier    确保 Barrier 前后的 load 操作不会发生乱序;
Full Barrier    确保 Barrier 前后的内存操作不会发生乱序;
{% endhighlight %}

可以通过 CPU 提供的如下的指令来显示的达到 Barrier 的目的：

{% highlight c %}
#define STORE_BARRIER() __asm__ __volatile__("sfence")
#define LOAD_BARRIER() __asm__ __volatile__("lfence")
#define FULL_BARRIER() __asm__ __volatile__("mfence")
{% endhighlight %}

同时，任何带有 lock 操作的指令以及某些原子操作指令均可以当做隐式的 Barrier，例如：

{% highlight c %}
__asm__ __volatile__("lock; addl $0,0(%%esp)");
__asm__ __volatile__("xchgl (%0),%0");
{% endhighlight %}

也可以将 Compiler Barrier 和 CPU Barrier 通过一条指令来实现：

{% highlight c %}
#define ONE_BARRIER() __asm__ __volatile__("mfence":::"memory")
{% endhighlight %}

最后需要说明的是，不同于编译屏障，CPU 内存屏障是在 CPU 上执行的指令，任何在程序中定义的 CPU 屏障最后都会编译成为汇编代码中的指令，例如，使用上面的 `LOAD_BARRIER()` 会在汇编代码中相应的位置插入 `lfence` 指令。

### 跨平台

如上的 `mfence` 是 x86_64 中使用的，那么对于其它平台可能是不同的指令，所以像 Linux 内核中，会通过 `smp_rmb()` `smp_wmb()` `smp_mb()` 来适配不同的平台。

## 参考

* [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) x86 的编程手册。
* [Weak vs. Strong Memory Models](https://preshing.com/20120930/weak-vs-strong-memory-models/) 对于 Weak 和 Strong 类型的总结。

<!--
https://preshing.com/20120515/memory-reordering-caught-in-the-act/
https://preshing.com/20120625/memory-ordering-at-compile-time/



False Sharing
https://blog.csdn.net/u011499747/article/details/78379838
https://www.cnblogs.com/cyfonly/p/5800758.html


https://zhuanlan.zhihu.com/p/57300417

CPU核心示意图
https://www.expreview.com/49967-2.html
https://upload.wikimedia.org/wikipedia/commons/6/64/Intel_Nehalem_arch.svg
https://upload.wikimedia.org/wikipedia/commons/6/60/Intel_Core2_arch.svg
https://zhuanlan.zhihu.com/p/67739048
http://kaiyuan.me/2018/04/21/consistency-concept/

-->

{% highlight text %}
{% endhighlight %}
