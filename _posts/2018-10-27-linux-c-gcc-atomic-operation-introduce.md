---
title: GCC 原子操作
layout: post
comments: true
language: chinese
category: [program]
keywords: gcc,atomic
description:
---

实际上目前大部分的语言都提供了原子操作的能力，包括了 GoLang、JAVA、C/C++ 等，当然这些能力大部分都需要硬件平台的支撑。

这里简单介绍 gcc 中提供的一些原子操作指令。

<!-- more -->

## 原子操作

Windows、C++ 实际上都提供了一些原子操作指令，这里简单介绍的是 `GCC 4.1.2` 版本之后提供的内置原子操作，可以直接对 `1 2 4 8` 字节的数值或指针类型，进行原子 `加 减 与 或 异或` 等操作。

其中接口示例如下。

{% highlight c %}
//----- 比较*ptr与oldval的值，如果相等则将newval更新到*ptr并返回true
bool __sync_bool_compare_and_swap (type *ptr, type oldval, type newval, ...)
//----- 比较*ptr与oldval的值，如果相等则将newval更新到*ptr并返回操作之前*ptr的值
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...)

//----- 将value加、减、或、与、异或到*ptr上，结果更新到*ptr，并返回操作前*ptr的值
type __sync_fetch_and_add (type *ptr, type value, ...) 
type __sync_fetch_and_sub (type *ptr, type value, ...)
type __sync_fetch_and_or (type *ptr, type value, ...)
type __sync_fetch_and_and (type *ptr, type value, ...)
type __sync_fetch_and_xor (type *ptr, type value, ...)
type __sync_fetch_and_nand (type *ptr, type value, ...)
{% endhighlight %}

<!--
type __sync_add_and_fetch (type *ptr, type value, ...) 
// 将value加到*ptr上，结果更新到*ptr，并返回操作之后新*ptr的值
type __sync_sub_and_fetch (type *ptr, type value, ...) 
// 从*ptr减去value，结果更新到*ptr，并返回操作之后新*ptr的值
type __sync_or_and_fetch (type *ptr, type value, ...) 
// 将*ptr与value相或， 结果更新到*ptr，并返回操作之后新*ptr的值
type __sync_and_and_fetch (type *ptr, type value, ...) 
// 将*ptr与value相与，结果更新到*ptr，并返回操作之后新*ptr的值
type __sync_xor_and_fetch (type *ptr, type value, ...)
// 将*ptr与value异或，结果更新到*ptr，并返回操作之后新*ptr的值
type __sync_nand_and_fetch (type *ptr, type value, ...) 
// 将*ptr取反后，与value相与，结果更新到*ptr，并返回操作之后新*ptr的值
__sync_synchronize (...) 
// 发出完整内存栅栏
type __sync_lock_test_and_set (type *ptr, type value, ...)
// 将value写入*ptr，对*ptr加锁，并返回操作之前*ptr的值。即，try spinlock语义
void __sync_lock_release (type *ptr, ...) 
// 将0写入到*ptr，并对*ptr解锁。即，unlock spinlock语义
-->

## 示例

利用 Linux 中的多线程，对一个全局的变量进行累加操作。

{% highlight c %}
#include <stdio.h>
#include <pthread.h>

#define COUNT_PER_WORKER  5000000
#define WORKER_NUM        10

int sum = 0;
pthread_t workers[WORKER_NUM];

void *worker_func(void *arg)
{
        (void) arg;
        int i;

        printf("Worker thread %08lx startup.\n", pthread_self());
        for (i = 0; i < COUNT_PER_WORKER; ++i) {
#ifdef USE_ATOMIC
                __sync_fetch_and_add(&sum, 1);
#else
                sum++;
#endif
        }
        return NULL;
}

int main(void)
{
        int i;

        for (i = 0; i < WORKER_NUM; ++i)
                pthread_create(&workers[i], NULL, worker_func, NULL);
        for (i = 0; i < WORKER_NUM; ++i)
                pthread_join(workers[i], NULL);

        printf("Sum expect %d, got %d.\n", COUNT_PER_WORKER * WORKER_NUM, sum);
        return 0;
}
{% endhighlight %}

可以通过如下方式进行编译，其中前者没有使用原子操作，会导致最后累加的值远小于预期。

{% highlight text %}
$ gcc -o atomic -lpthread atomic.c
$ gcc -DUSE_ATOMIC -o atomic -lpthread atomic.c
{% endhighlight %}

<!--
## 参考

[GCC的原子操作](http://blog.sina.com.cn/s/blog_6f5b220601013zw3.html)
-->

{% highlight text %}
{% endhighlight %}
