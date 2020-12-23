---
title: Valgrind 工具使用
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: 
description:
---

Valgrind 可以用于构建动态分析工具，包括一个工具集，每个工具可以进行一类的调试、分析、测试，以帮助完善程序。

该工具采用的是模块化方式，可以很容易创建新的工具而又不会扰乱现有工具。

<!-- more -->

## 简介

* memcheck 内存异常检测，包括未释放、重复释放等异常；
* sgcheck 用来检测堆和全局数组的溢出，可以与 memcheck 互补；
* cachegrind 缓存和分支预测分析器，可以用来提高程序的性能；
* callgrind 函数调用图缓存生成分析器，与上一工具功能略有重叠，可以收集一些其它信息；
* helgrind 多线程的异常检测，可以发现一些多线程导致的问题；
* DRD 与 helgrind 类似，采用了不同的实现方式，可以找到不同的问题；
* massif 堆分析器，对堆内存的使用进行分析；
* DHAT 另一种不同的堆分析器，有助于理解块的生命周期、块的使用和布局的低效等问题；

## memcheck

一个内存使用情况的检查工具，可以处理如下的几类错误：

* 使用未初始化的内存，主要是一些变量。
* 内存多次释放、释放后访问、未释放等。
* 对堆栈内存空间的非法访问，包括了写入和读取。

### 内存操作

包括了内存未释放、多次释放等操作，其中前者默认只打印内存泄漏信息，如果要查看详细的地址需要添加 `--leak-check=full` 参数。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (void)
{
        char *buff = NULL;

        buff = (char *)malloc(5);
        if (buff == NULL)
                return -1;

        free(buff);
        free(buff);

        return 0;
}
{% endhighlight %}

上面代码出现了两次的释放操作，会有如下的报错信息。

{% highlight text %}
==43613== Invalid free() / delete / delete[] / realloc()
==43613==    at 0x4C2ACBD: free (vg_replace_malloc.c:530)
==43613==    by 0x400722: main (in /tmp/test/race)
==43613==  Address 0x541f040 is 0 bytes inside a block of size 5 free'd
==43613==    at 0x4C2ACBD: free (vg_replace_malloc.c:530)
==43613==    by 0x40071A: main (in /tmp/test/race)
==43613==  Block was alloc'd at
==43613==    at 0x4C29BC3: malloc (vg_replace_malloc.c:299)
==43613==    by 0x40070A: main (in /tmp/test/race)
{% endhighlight %}

如果把上面的两个 `free()` 操作注释掉，同时添加 `--leak-check=full` 参数，那么会报如下的错误。

{% highlight text %}
==35580== 5 bytes in 1 blocks are definitely lost in loss record 1 of 1
==35580==    at 0x4C29BC3: malloc (vg_replace_malloc.c:299)
==35580==    by 0x400714: main (in /tmp/test/race)
{% endhighlight %}

### 非法访问

包括了非法读取以及非法写入，例如如下的示例。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
        char *buff;

        buff = (char *)malloc(5);
        if (buff == NULL)
                return -1;
        strcpy(buff, "01234");

        printf("buffer is '%s'\n", buff);
        free(buff);

        return 0;
}
{% endhighlight %}

其中的缓存只申请了 5 个字节，但是包括字符串结束符 `\0` 总共写入了 6 个字符，之所以报两个，估计是因为 `strcpy()` 正常只允许写入 4 个字节。

另外，在打印的时候，会多访问一个字符串的结束符。

{% highlight text %}
==23247== Invalid write of size 2
==23247==    at 0x400771: main (in /tmp/test/race)
==23247==  Address 0x541f044 is 4 bytes inside a block of size 5 alloc'd
==23247==    at 0x4C29BC3: malloc (vg_replace_malloc.c:299)
==23247==    by 0x40075A: main (in /tmp/test/race)
==23247== 
==23247== Invalid read of size 1
==23247==    at 0x509EEF9: vfprintf (vfprintf.c:1635)
==23247==    by 0x50A5328: printf (printf.c:34)
==23247==    by 0x400780: main (in /tmp/test/race)
==23247==  Address 0x541f045 is 0 bytes after a block of size 5 alloc'd
==23247==    at 0x4C29BC3: malloc (vg_replace_malloc.c:299)
==23247==    by 0x40075A: main (in /tmp/test/race)
==23247== 
{% endhighlight %}

### 其它

另外几个常用的参数示例如下。

{% highlight text %}
--show-reachable=yes
--trace-children=yes

--leak-check=<no|summary|yes|full>[default:summary]
  在退出时检查是否有泄漏，其中summary只显示有多少泄漏，而yes或full会打印每次泄漏的详细信息。
--error-exitcode=1
  当出现错误时，指定返回的错误码，对于一些根据退出码判断是否正常的工具有效。
{% endhighlight %}

## helgrind

数据竞争 (Data Race) 是指在非线程安全的情况下，多线程对同一个地址空间进行写操作，一般会使用互斥锁或者读写锁进行保护，但是有可能会因为笔误或者设计缺陷仍然存在数据竞争的可能性。

如下的代码中，正常来说应该使用写锁，但是误写成了读锁。

{% highlight c %}
#include <unistd.h>
#include <pthread.h>

static int racy;
static pthread_rwlock_t rwlock;

static void *thread_func(void *arg)
{
        (void) arg;
        pthread_rwlock_rdlock(&rwlock);
        racy++;
        pthread_rwlock_unlock(&rwlock);
        usleep(100 * 1000);

        return 0;
}

int main(void)
{
        pthread_t thread1;
        pthread_t thread2;

        pthread_rwlock_init(&rwlock, 0);
        pthread_create(&thread1, 0, thread_func, 0);
        pthread_create(&thread2, 0, thread_func, 0);

        pthread_join(thread1, 0);
        pthread_join(thread2, 0);
        pthread_rwlock_destroy(&rwlock);

        return 0;
}
{% endhighlight %}

然后执行如下操作，其中输出是简化之后的结果。

{% highlight text %}
$ gcc -o race race.c -O2 -rdynamic -lpthread
$ valgrind --tool=helgrind ./race
... ...
==11510== Possible data race during write of size 4 at 0x6010D8 by thread #3
==11510== Locks held: none
==11510==    at 0x400AD3: thread_func (in /tmp/test/race)
==11510==    by 0x4C3081E: mythread_wrapper (hg_intercepts.c:389)
==11510==    by 0x4E42DD4: start_thread (pthread_create.c:307)
==11510==    by 0x5154EAC: clone (clone.S:111)
==11510== 
==11510== This conflicts with a previous write of size 4 by thread #2
==11510== Locks held: none
==11510==    at 0x400AD3: thread_func (in /tmp/test/race)
==11510==    by 0x4C3081E: mythread_wrapper (hg_intercepts.c:389)
==11510==    by 0x4E42DD4: start_thread (pthread_create.c:307)
==11510==    by 0x5154EAC: clone (clone.S:111)
==11510==  Address 0x6010d8 is 0 bytes inside data symbol "racy"
{% endhighlight %}

如上标示了在地址 `0x6010D8` 处有 4 个字节存在数据竞争，分别有两个线程在尝试访问这一地址，而且没有持有锁；最后一行标示了这个变量是 racy 。

## Suppression

可以通过 `--gen-suppressions=all` 参数生成 `suppression` 示例，然后略微编辑即可生成所需的 `suppression` 文件。

文件中的每个 `suppression` 以 `{}` 开始结束，并由如下的行组成。

* 第一行，对应的名称，可以通过这个名称对应 `suppression`；
* 第二行，所使用的工具，以及异常类型，例如 `Memcheck:Leak`；
* 第三行，如果第二行的工具需要参数，则在这行指定；
* 剩余行，代表了上下文，类似于调用栈。

其中上下文可以指定函数 `fun` 或者动态库 `obj` ，可以通过 `*` 或者 `?`  匹配任意字符，也可以使用 `...` 匹配上下文中的一条。

{% highlight text %}
{
   foobar
   Memcheck:Leak
   match-leak-kinds: possible
   ...
   fun:foobar
   ...
}
{
   g_type_register_static
   Memcheck:Cond
   obj:*
   fun:g_type_register_static
   ...
}
{% endhighlight %}

对于 `Memcheck` 来说，常用的关键字如下。

* Value1, Value2, Value4, Value8, Value16:代表1-16字节的未初始化变量的使用
* Cond (or its old name, Value0)：表示未初始化的cpu条件变量
* Addr1, Addr2, Addr4, Addr8, Addr16：表示1-16字节的不可addressable的内存访问
* Jump：表示跳转到一个不可addressable的地方
* Param：表示系统调用syscall的参数错误，这个类型需要另外一行指定syscall的那个参数
* Free：表示不匹配的内存释放
* Overlap：表示在memcpy时source和destination有重叠
* Leak：表示内存泄漏




<!--
https://ivanzz1001.github.io/records/post/cplusplus/2018/11/14/cpluscplus-valgrind_usage
-->


{% highlight text %}
{% endhighlight %}
