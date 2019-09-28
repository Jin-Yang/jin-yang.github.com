---
title: C 无锁编程
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,lock-free
description:
---


<!-- more -->

## 原子操作

在多线程编程时，通常需要对一些常见的基本类型进行操作，如 `int` `float` 等等，一般为了解决竞态条件，通常是通过 mutex、spinlock 等进行保护。

如果不进行保护，那么实际得到的值是什么？可以从如下程序进行测试。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#define INC_TO 1000000 // one million...

int global_int = 0;

pid_t gettid( void )
{
        return syscall( __NR_gettid );
}

void *thread_routine(void *arg)
{
        int i;
        int proc_num = (int)(long)arg;
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(proc_num, &set);

        if (sched_setaffinity(gettid(), sizeof(cpu_set_t), &set)) {
                perror( "sched_setaffinity" );
                return NULL;
        }

        for (i = 0; i < INC_TO; i++) {
                global_int++;
        }

        return NULL;
}

int main(void)
{
        int procs = 0;
        int i;
        pthread_t *thrs;

        /* Getting number of CPUs */
        procs = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if (procs < 0) {
                perror("sysconf");
                return -1;
        }

        thrs = (pthread_t *)malloc(sizeof(pthread_t) * procs);
        if (thrs == NULL) {
                perror( "malloc" );
                return -1;
        }

        printf("Starting %d threads...\n", procs);

        for (i = 0; i < procs; i++) {
                if (pthread_create(&thrs[i], NULL, thread_routine, (void *)(long)i)) {
                        perror( "pthread_create" );
                        procs = i;
                        break;
                }
        }

        for (i = 0; i < procs; i++)
                pthread_join(thrs[i], NULL);

        free(thrs);

        printf("After doing all the math, global_int value is: %d\n", global_int);
        printf("Expected value is: %d\n", INC_TO * procs);

        return 0;
}
{% endhighlight %}

如上程序中，每个 CPU 会绑定一个线程，并对一个线程累加，不同的平台可能会有所区别，源码可以从 [github atomics.c]({{ site.example_repository }}/linux/pthread/atomics.c) 上下载。

一般输出的内容如下，当然不同的平台也可能会输出 `4000000`。

{% highlight text %}
$ ./atomics
Starting 4 threads...
After doing all the math, global_int value is: 2933043
Expected value is: 4000000
{% endhighlight %}

在编译使用 `-O2` 参数时会输出 `4000000`，实际上这是编译器优化的效果，将原来的循环直接替换成了加 1000000 。

对于 CPU 操作，每次读写、累加都是原子操作，但是几个操作的组合将不再是原子操作。

### 原理

原子操作对于 CPU 来说很简单，在访问内存时，可以通过特定的指令可以锁定 Front Serial Bus, FSB 。FSB 就是 CPU 与内存通讯的总线，当锁 FSB 时，CPU 就无法访问内存，从而达到原子操作。

内核中很早就在使用原子操作了，而 gcc 在 4.1.2 才支持用户模式下的原子操作。

假设，有如下的伪代码，看看当两个线程同时操作时会发生什么问题。

{% highlight text %}
decrement_atomic_value();
if (atomic_value() == 0)
	fire_a_gun();
{% endhighlight %}

假设其执行顺序如下。

![hello world logo]({{ site.url }}/images/linux/pthread-atomic-two-threads.png "hello world logo"){: .pull-center }

对于上述的执行顺序，实际上 line3 不会执行。

### 实现

gcc 中提供了 加法、减法、或、异或、与、与非，每类分别有两个函数，一个返回操作之前的值，一个返回之后的值。

{% highlight text %}
type __sync_fetch_and_add (type *ptr, type value);
type __sync_fetch_and_sub (type *ptr, type value);
type __sync_fetch_and_or (type *ptr, type value);
type __sync_fetch_and_and (type *ptr, type value);
type __sync_fetch_and_xor (type *ptr, type value);
type __sync_fetch_and_nand (type *ptr, type value);

type __sync_add_and_fetch (type *ptr, type value);
type __sync_sub_and_fetch (type *ptr, type value);
type __sync_or_and_fetch (type *ptr, type value);
type __sync_and_and_fetch (type *ptr, type value);
type __sync_xor_and_fetch (type *ptr, type value);
type __sync_nand_and_fetch (type *ptr, type value);
{% endhighlight %}

其中的 type 可以取如下的值，包括了 `int` `unsigned int` `long` `unsigned long` `long long` `unsigned long long` 几种类型。



{% highlight text %}
{% endhighlight %}
