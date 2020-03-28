---
title: C++ 内存模型
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->


## 分类

CPU 对内存的操作共分两种，分别是 load 和 store，也就是对应了读写内存。

因此，理论上存在四种 CPU 内存乱序：Load-Load，Load-Store，Store-Store，Store-Load 乱序，当然这跟硬件设备相关。例如，在 Strongly-ordered memory 环境例如 X86/X64 下，唯一允许产生的乱序是 Store-Load 乱序。

----- Store Store Reordering
a = 3; b = 4; --> b = 4; a = 3;

----- Store Load Reordering
a = 3; load(b); --> load(b); a = 3;

----- Load Load Reordering
load(a); load(b); --> load(b); load(a);

----- Load Store Reordering
load(a); b = 4; --> b = 4; load(a);

在单线程没有明显的差异，但是在多线程中就会导致异常，尤其是在 lock-free 编程中。

### 内存模型

所谓的内存模型，主要是讨论如何描述一个内存操作在各个线程间的可见性的问题。

修改操作不能及时被所有内存看到的原因很多，为了提高内存的读写效率，CPU 通常会在 Cache 中缓存临时结果，这就会导致 CPU 的一个内存写操作之后，不能立即被其它 CPU 看到。

其它更加详细的介绍可以参考 [Memory Barriers: a Hardware View for Software Hackers](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.152.5245&rep=rep1&type=pdf) 中的内容。

C++11 中的 atomic library 中定义了以下 6 种语义来对内存操作的行为进行约定，这些语义分别规定了不同的内存操作在其它线程中的可见性问题：

enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
};

#### relaxed

不做任何要求，编译器、CPU 等允许按照任意它们认为合适的方式来加以优化处理。

#### Release Acquire

这两个基本都是同时出现，Acquire/Release 分别用于读写操作：

如果一个线程 A 对一块内存 m 以 release 的方式进行修改，那么在线程 A 中，所有在该 release 操作之前进行的内存操作，都在另一个线程 B 对内存 m 以 acquire 的方式进行读取之后，变得可见。

http://www.cnblogs.com/catch/p/3803130.html





#### 避免乱序

Compiler Reordering 是编译器为了提高程序的执行效率而生成的，很多时候，希望避免这种乱序的发生，可以通过两种方式声明：显式内存屏障 (Memory Barrier) 和隐式的内存屏障。

对于 GCC 及 clang 编译器而言，可以使用如下指令来显示地阻止编译器产生乱序：

#define barrier() __asm__ __volatile__("":::"memory")  // linux
#define barrier() _ReadWriteBarrier()                  // win

barrier() 指令指示编译器不要将该指令之前的 Load-store 操作移动到该指令之后执行。

我们修改上面的程序如下，并再次使用 -O2 选项来生成汇编代码。此时你会发现 gcc 将不会把变量 b 的 store 操作提前到变量 a 的 store 操作之前

int foo()
{
    a = b + 1;
    barrier();
    b = 0;
    return 1;
}

需要指明的是，上述的 barrier() 并不是一条需要 CPU 执行的指令，其只是让应用程序告知编译器不要产生跨过该指令的编译乱序，在生成的汇编代码中不包括任何与 barrier 相关的指令，这一点与后续的 CPU barrier 不同。

除了上述显示的 barrier() 以外，程序中的其他元素，例如同步原语（e.g. Mutex，RWLock，信号量，CPU 屏障等），非 inline 函数以及 C++11 提供的 non-relaxed 原子操作等均可以扮演 compiler barrier 的作用。关于 C++11 提供的内存模型和原子操作，后续有时间再专门介绍。

### CPU memory reordering

也就是在运行时产生的乱序，也被称为 Hardware memory reordering。

相比于编译乱序而言，CPU 乱序的产生背景以及对程序的影响都要更为复杂，其涉及到了 CPU 体系结构以及 cache coherence 协议。当然，要把所有的东西都说明白可能需要一篇很长的文章，这里我们尽量先用简单的例子来证明 CPU reordering 的存在，然后介绍一下几种不同的 CPU reordering 的类型以及相应的 barrier。关于 CPU reordering 的产生原因，后面再用一篇文章细细讲解。

对于如下的两个程序 P1 和 P2，假设 X 和 Y 的值初始均为 1， 且 P1 和 P2 分别由两个线程运行在一个双核 CPU 上，我们的问题是：两个线程并行执行得到的 r1 以及 r2 的值会不会同时为 0 ？

  |---------------|          |---------------|
  |1  mov [X], 1  |          |1  mov [Y], 1  |
  |2  mov r1, [Y] |          |2  mov r2, [X] |
  |---------------|          |---------------|
         P1                         P2
大多数人可能都会认为 r1 和 r2 是不可能同时为 0 的。然而根据 Intel Arch Specification，Intel CPU 的核心是可以根据规则自由地重排指令的执行顺序，从而导致两个核都可能先执行对于 r1 和 r2 的复制操作，从而导致最终得到 r1 以及 r2 均为 0。

如果你是一个主动思考的人，看到这里一定会有一个疑问：这种 CPU 产生的乱序难道没有违背先前所制定的产生 memory reordering 的前提吗？为了清楚地说明这个问题，我们再重申一下任何体系架构产生 memory reordering 的前提要求:

从单个线程的角度来看，任何一段程序在 Reordering 前与 Reordering 后，拥有相同的执行效果

我们分别看执行 P1 和 P2 的两个线程，如果我们从 P1 的线程角度角度来看，先执行指令 1 还是 指令 2 对 P1 本身来讲没有任何影响；同样的，如果单从 P2 的角度来看，先执行指令 1 还是 指令 2 对其本身也没有任何影响，因此这种乱序完全满足上述的前提。（这个例子也充分解释了什么叫做”从单线程的角度”）。

接下来我借用 Preshing 的代码来证明 CPU memory reordering 的存在。为了支持在 MacOS 下的运行，用 dispatch_semaphore_t 替换了 Linux 下的 sem_t，完整修改过的程序见 github。这里要向 Preshing 表示感谢，同样作为一个游戏制造者，我从他的 Blog 学习到了很多，也只能自愧不如，然后见贤思齐了。

程序的主体通过两个线程分别执行如下程序，也就是完整地实现了上述关于 X 和 Y 的伪代码：

void *thread1Func(void *param) {
    MersenneTwister random(1);
    for (;;) {
        sem_wait(&beginSema1);
        while (random.integer() % 8 != 0) {}  // 产生一个随机地延迟

        X = 1;
#if USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // 阻止 CPU 乱序
#else
        asm volatile("" ::: "memory");  // 阻止编译乱序
#endif
        r1 = Y;
        sem_post(&endSema);
    }
    return NULL;  // Never returns
};

然后主程序进行如下同步，其执行多次地循环来输出 r1 和 r2 的值，当检测到 r1 和 r2 均为 0 时，表明上述的 CPU reordering 产生了：

int main()
{
    sem_init(&beginSema1, 0, 0);
    sem_init(&beginSema2, 0, 0);
    sem_init(&endSema, 0, 0);

    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, thread1Func, NULL);
    pthread_create(&thread2, NULL, thread2Func, NULL);

    int detected = 0;
    for (int iterations = 1; i < MAX_ITER; iterations++)
    {
        X = 0; Y = 0;
        // Signal both threads
        sem_post(&beginSema1);
        sem_post(&beginSema2);
        // Wait for both threads
        sem_wait(&endSema);
        sem_wait(&endSema);
        // Check if there was a simultaneous reorder
        if (r1 == 0 && r2 == 0)
        {
            detected++;
            printf("%d reorders after %d iters\n", detected, iterations);
        }
    }
    return 0;
}
可以看到 main 函数通过 beginSema1，beginSema2 以及 endSema 进行同步。beginSema 扮演了 CPU memory barrier 的作用，确保在两个线程开始运行时，其看到的 X 和 Y 的值已经被设为 0，同时 endSema 也扮演了 CPU barrier 的作用，确保经过两个线程修改得到的 r1 和 r2 已经传播到 main 线程中。这个解释你可能暂时不太理解，等以后充分介绍了 acquire-release 语义之后应该就会明白了。

修改 USE_CPU_FENCE 为 0 ，并且执行 main 函数中的主循环 300 万次，输出结果如下：

22308 reorders detected after 2999891 iterations
即出现了 22308 次 CPU 乱序，如果设置 USE_CPU_FENCE 为 1， 并重新编译运行该程序，结果将检测不到任何的 CPU 乱序。

## 参考

关于 Memory Reordering 的实例可以参考 [Memory Reordering Caught in the Act](http://preshing.com/20120515/memory-reordering-caught-in-the-act/)，或者 [Github Mem-Reordering](https://github.com/jszakmeister/mem-reordering) 。



https://github.com/rigtorp/awesome-lockfree

## Acquire and Release Semantics
http://preshing.com/20120913/acquire-and-release-semantics/
http://preshing.com/20170612/can-reordering-of-release-acquire-operations-introduce-deadlock/


并发HashTable
https://github.com/efficient/libcuckoo
https://github.com/preshing/junction

有关 memory reordering 的简介到这里就先结束了。涉及到内存模型，还有非常多额外的内容，例如

Cache coherence protocol (eg. MSI, MESI)
acquire-release semantics
Sequential consistency
Synchronize-with relation
Strongly- or Weakly-ordered memory


对内存屏障的不错介绍
http://kaiyuan.me/2017/09/22/memory-barrier/

http://www.cnblogs.com/catch/p/3803130.html

https://rethinkdb.com/blog/lock-free-vs-wait-free-concurrency/
http://kaiyuan.me/2017/12/14/lock-free-prog1/
http://www.cnblogs.com/gaochundong/p/lock_free_programming.html
http://preshing.com/20120612/an-introduction-to-lock-free-programming/

Is Parallel Programming Hard, And, If So, What Can You Do About It?
https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html
对并发的一些概念介绍
http://opass.logdown.com/?page=4

http://lday.me/2017/12/02/0018_cpp_atomic_summary/

原子操作的成本是啥？需要多少个时钟周期？在 SMP NUMA 中是否会阻塞其它 CPU 的运行？是否会阻塞内存访问？会导致 CPU 流水线异常吗？对 Cache 的影响是啥？
https://stackoverflow.com/questions/2538070/atomic-operation-cost
http://cliffc.org/blog/2009/04/14/odds-ends/
https://fgiesen.wordpress.com/2014/08/18/atomics-and-contention/
https://spcl.inf.ethz.ch/Publications/.pdf/atomic-bench.pdf

CPU Cache 详解，缓存的管理一般使用的是MESI协议
http://cenalulu.github.io/linux/all-about-cpu-cache/

与 lock-free algorithm 息息相关，会涉及到 Cache coherence 协议、CPU 体系结构、Sequential consistency 概念等问题。

https://blog.csdn.net/zdy0_2004/article/details/48013829

Cache 写机制 Write Through、Write Back 。


内存的操作分为了 Load Store 操作。

代码在编译、执行阶段都有可能会出现乱序，导致程序的执行结果与预期不符。

如果要保证严格的代码执行顺序必然要付出代价，为了提高灵活性，C++11 提供了一套内存模型，其中定义了六种内存顺序模型。

typedef enum memory_order {
	memory_order_relaxed,
	memory_order_consume,
	memory_order_acquire,   *
	memory_order_release,   *
	memory_order_acq_rel,
	memory_order_seq_cst    *
} memory_order;

根据不同的使用场景，用户在执行原子操作时可以选择对应的模型。

http://blog.jobbole.com/106516/

## GCC

在 C++11 提供内存模型前，GCC 提供了一套 `__sync_XXX` 的原子操作集合，主要针对 Intel 官方提供的能力。

type __sync_fetch_and_OP (type *ptr, type value, ...)
type __sync_OP_and_fetch (type *ptr, type value, ...)
bool __sync_bool_compare_and_swap (type *ptr, type oldval, type newval, ...)
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...)
__sync_synchronize (...)

type __sync_lock_test_and_set (type *ptr, type value, ...)
void __sync_lock_release (type *ptr, ...)

上面的 `OP` 代表了一些列操作，包括了 `add` `sub` `or` `and` `xor` `nand` 常见操作，`type` 用于表示数据类型，包括了 `int` `long` `long long` ，可以是有符号或者无符号。

[Intel Memory Ordering White Paper](http://www.cs.cmu.edu/~410-f10/doc/Intel_Reordering_318147.pdf)

C++ 的 [原子操作库](https://zh.cppreference.com/w/cpp/atomic)，不同的函数有具体的示例可以参考 。

https://en.wikipedia.org/wiki/MOESI_protocol

https://zhuanlan.zhihu.com/p/48157076
https://zhuanlan.zhihu.com/p/48161056

内存模型，其参考中包括了C相关
https://en.cppreference.com/w/cpp/atomic/memory_order

大部分的模型实际上就是调用了不同的 CPU 原子指令。


GCC 这种 Full Barrier 操作有效，但是带来的成本也很高：A) 编译器和处理器无法进行优化；B) 为了保证变量对其它 CPU 可见，需要硬件级别的同步 (MESI)。

C++11 提供的内存模型，可以保证在保证业务正确的前提下，将对性能的影响降到最低。

C++11 的所有原子操作都会有一个可选的参数来标识使用的内存模型，默认使用最强模式 Sequentially Consistent 。


对应到 CPU 提供的是 `sfence` `lfence` `mfence` 指令，用于保证内存的读写顺序。

* `sfence` Store Fence 串行化写操作，但是不会影响到读操作；
* `mfence` Memory Fence

另外还有一种 `("":::"memory")` 是用来确保编译器不发生乱序的。

https://www.felixcloutier.com/x86/sfence

https://www.zhihu.com/question/29465982


在 GCC 4.9 之后的版本中，提供了 `__atomic` 类函数，支持 C++11 内存模型，同时替换掉了原有的 `__sync` 类型的接口。
https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html

synchronized
mfence sfence lfence

Acquire and Release Fences
https://preshing.com/20130922/acquire-and-release-fences/

Acquire

十分经典的一个示例
https://zhuanlan.zhihu.com/p/41872203
https://mariadb.org/wp-content/uploads/2017/11/2017-11-Memory-barriers.pdf

https://zhuanlan.zhihu.com/p/41872203
http://www.cs.cmu.edu/~410-f10/doc/Intel_Reordering_318147.pdf

C++11 提供了六种 Memory Order 模型，虽然有六种，但理解了四种同步的情形基本就差不多了。

1. Relaxed Ordering 在单个线程内，所有原子操作都是按照代码顺序执行，不同线程则不限制。



3. Release -- consume: 我去，我只想同步一个 x 的读写操作，结果把 release 之前的写操作都顺带同步了？如果我想避免这个额外开销怎么办？用 release -- consume 呗。同步还是一样的同步，这回副作用弱了点：在线程 B acquire x 之后的读操作中，有一些是依赖于 x 的值的读操作。管这些依赖于 x 的读操作叫 赖B读. 同理在线程 A 里面, release x 也有一些它所依赖的其他写操作，这些写操作自然发生在 release x 之前了。管这些写操作叫 赖A写. 现在这个副作用就是，只有 赖B读 能看见 赖A写. （卧槽真累）有人问了，说什么叫数据依赖（carries dependency）？其实这玩意儿巨简单：S1. c = a + b;
S2. e = c + d;S2 数据依赖于 S1，因为它需要 c 的值。


### Sequential Consistency

对所有的变量的所有原子操作都同步，这也就意味着所有的原子操作，跟一个线程顺序执行的结果相似。

    Thread#1     |      Thread#2
=================|====================
     y = 1       |  if (x.load() == 2)
   x.store(2)    |      assert(y == 1)

对于编译器和 CPU 来说，这两个是独立的，也就意味着可以根据需要乱序执行。

### Release Acquire

不同线程的两个原子操作顺序不定，这就需要对执行顺序进行同步。

线程 A 原子性地把值写入 x 的值 Release，线程 B 原子性地读取 x 的值 Acquire，这样线程 B 可以保证读取到 x 的最新值。

也就是说，线程 A 中所有在 Release x 之前的写操作，在线程 B Acquire x 之后任何读操作都可见，也就是保证了执行的顺序。

std::atomic<int> a{0};
int b = 0;

            Thread#1               |               Thread#2
===================================|===========================================
              b = 1                |  while (a.load(memory_order_acquire) != 1);
 a.store(1, memory_order_release); |           std::cout << b << '\n';

其中 `memory_order_release` 保证在这个操作之前的内存访问 (Memory Accesses) 不会乱序到这个操作之后执行，但是不保证该操作之后的内存访问是否会乱序到该操作之前。

这个主要用于在准备某些资源后，通过上述的 API 将状态 "Release" 给别的线程。

而 `memory_order_acquire` 该操作之后的内存访问不会乱序到之前，所以，通常用来判断或者等待某个资源，一旦满足某个条件后就可以安全的 "Acquire" 消费这些资源了。

在 Intel 中，会使用 XCHG 交换两个数据，可以是内存或者寄存器中的值，如果是内存中的值，会自动添加 Lock 机制，无需显示 Lock 。

另外还有 CMPXCHG 操作，默认不是原子的，如果需要保证原子性，需要在指令前添加一个 LOCK 指令。


cmpxchg(void* ptr, int old, int new)，如果ptr和old的值一样，则把new写到ptr内存，否则返回ptr的值，整个操作是原子的。在Intel平台下，会用lock cmpxchg来实现，这里的lock个人理解是锁住内存总线，这样如果有另一个线程想访问ptr的内存，就会被block住。

好了，让我们来看Linux Kernel中的cmpxchg(网上找来的，我自己机器上没找到对应的头文件，据说在include/asm-i386/cmpxchg.h)实现：

原文：https://blog.csdn.net/penngrove/article/details/44175387 

mov [_x], val    store into memory
mov r, [_x]      load into the register


如果使用的是 `int` 变量，可以看到地址实际上是 4 字节对齐的，在一些原子操作中
https://zhuanlan.zhihu.com/p/41872203
https://preshing.com/20120625/memory-ordering-at-compile-time/
https://preshing.com/20130922/acquire-and-release-fences/
https://www.zhihu.com/question/29465982
https://peeterjoot.wordpress.com/2009/12/04/intel-memory-ordering-fence-instructions-and-atomic-operations/
https://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11/
https://mariadb.org/wp-content/uploads/2017/11/2017-11-Memory-barriers.pdf
https://people.cs.pitt.edu/~xianeizhang/notes/cpp11_mem.html
https://arxiv.org/pdf/1803.04432.pdf
https://www.cnblogs.com/gorden/archive/2011/07/30/2122256.html
https://www.cnblogs.com/gaochundong/p/lock_free_programming.html
https://davmac.wordpress.com/2018/01/28/understanding-the-c-c-memory-model/
http://www.parallellabs.com/2010/03/06/why-should-programmer-care-about-sequential-consistency-rather-than-cache-coherence/
https://zhuanlan.zhihu.com/p/57300417
http://www.bailis.org/blog/linearizability-versus-serializability/
https://cse.buffalo.edu/~stevko/courses/cse486/spring13/lectures/26-consistency2.pdf



内存访问模型
https://software.intel.com/en-us/articles/how-memory-is-accessed
https://www.crucial.cn/learn-with-crucial/memory/what-is-a-dimm-memory
https://software.intel.com/en-us/articles/what-s-new-about-modern-hardware



## 参考


* [C++ Concurrency In Action 2nd](/reference/programs/CPPConcurrencyInAction2ndEdition.pdf)

<!--
http://www.allitebooks.org/c-concurrency-in-action-2nd-edition/

https://www.jb51.net/article/55885.htm
-->

{% highlight text %}
{% endhighlight %}
