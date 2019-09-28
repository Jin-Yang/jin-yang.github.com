---
title: GDB 死锁分析
layout: post
comments: true
language: chinese
category: [program]
keywords:  program,c,deadlock
description:
---

pthread 是 POSIX 标准的多线程库，其源码位于 glibc 中的 Native POSIX Thread Library, NPTL 目录下，大部分的应用都是基于 pthread 来实现多线程的并行与同步管理。

<!-- more -->

## 简介

通过线程可以提高调度效率，包括了更加轻量级的上下文切换，避免不必要的 mm_switch 。

在 Linux 中 pthread 所提供的同步机制核心要依赖与内核的 futex 机制。

在用户空间会执行变量的原子增加操作，一般是 CAS 操作，如果没有冲突，那么就会立即返回，不会发生上下文切换。

只有发生冲突后，才会调用 futex 系统调用，然后切换到内核态。

nptl 的实现会通过 futex 中的值标识来表示锁的状态，包括了：A) 0 锁空闲；B) 1 没有 waiter ，解锁之后无需调用 futex_wake ；C) 2 有 waiter ，那么解锁之后需要调用 futex_wake 。

{% highlight text %}
pthread_mutex_lock()   nptl/pthread_mutex_lock.c
 |-__pthread_mutex_lock()
   |-LLL_MUTEX_LOCK()    最主要的实现函数，也就是lll_lock()的宏定义
     |-__lll_lock()
       |-atomic_compare_and_exchange_bool_acq()	尝试从0变为1，成功返回0(获得锁返回)，否则返回>0
         |-__lll_lock_wait() 返回的是非0进入阻塞，会调用futex并将值设置为2
{% endhighlight %}

上述真实的代码是在汇编文件中实现，对于 C 代码可以参考 `lowlevellock.c` 中的实现。

{% highlight c %}
void __lll_lock_wait (int *futex, int private)
{
	/* 非第一个线程会阻塞在这里 */
	if (*futex == 2)  
		lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */
 
	/* 第一个线程会阻塞在这里 */
	while (atomic_exchange_acq (futex, 2) != 0)
		lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */
}
{% endhighlight %}

在 `__lll_lock_wait()` 函数中，第一个没有获取锁的线程会进入 while 循环，并将 futex 赋值成为 2 ，等待 lock 被释放后成为 0 ，这第一个 waiter 被唤醒，`atomic_exchange_acq()` 则会赋予 futex 继续是 2，但是返回 0 跳出获取到 lock 。

{% highlight text %}
pthread_mutex_unlock()
 |-__pthread_mutex_unlock()
   |-__pthread_mutex_unlock_usercnt() 做一些恢复owner nusers的操作
     |-lll_unlock()   // 将futex值赋为0，并对oldval比较，如果是2，说明有waiter，则futex_wake，1则不需要
	   |-lll_futex_wake()
{% endhighlight %}

另外，内核中的 futex 模块的 waiter 队列是 FIFO 的，根据参数 mutex unlock 后只会 wake up 一个 waiter 。

<!--
https://www.cnblogs.com/xiaojianliu/articles/8638871.html
http://kexianda.info/2017/08/17/%E5%B9%B6%E5%8F%91%E7%B3%BB%E5%88%97-5-%E4%BB%8EAQS%E5%88%B0futex%E4%B8%89-glibc-NPTL-%E7%9A%84mutex-cond%E5%AE%9E%E7%8E%B0/

https://github.com/rouming/dla
-->









## 示例

如下是一个会产生死锁的示例。

{% highlight c %}
#include <unistd.h>
#include <pthread.h>

struct foobar {
        pthread_mutex_t mutex1;
        pthread_mutex_t mutex2;
};

void *thread1(void *arg)
{
        struct foobar *d;

        d = (struct foobar *)arg;
        while (1) {
                pthread_mutex_lock(&d->mutex1);
                sleep(1);
                pthread_mutex_lock(&d->mutex2);

                pthread_mutex_unlock(&d->mutex2);
                pthread_mutex_unlock(&d->mutex1);
        }
}

void *thread2(void *arg)
{
        struct foobar *d;

        d = (struct foobar *)arg;
        while (1) {
                pthread_mutex_lock(&d->mutex2);
                sleep(1);
                pthread_mutex_lock(&d->mutex1);

                pthread_mutex_unlock(&d->mutex1);
                pthread_mutex_unlock(&d->mutex2);
        }
}

int main(void)
{
        pthread_t tid[2];
        struct foobar data = {
                PTHREAD_MUTEX_INITIALIZER,
                PTHREAD_MUTEX_INITIALIZER
        };

        pthread_create(&tid[0], NULL, &thread1, &data);
        pthread_create(&tid[1], NULL, &thread2, &data);

        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);

        return 0;
}
{% endhighlight %}

注意，如果使用了 `-O2` 参数 + `strip` 操作，那么对于一些已经优化的符号 (static inline) 在使用 gdb 的时候就可能不存在。

那么需要添加 `-rdynamic` 参数，那么即使执行了上面的两个操作，那么仍然可以通过 gdb 使用。

不过，因为优化之后可能导致符号对应的地址与原函数不匹配，有可能是在函数的末尾，那么，对于 gdb 来说，就是设置了断点，但是可能不生效。

此时，只能通过如下方法查看其反汇编代码。

{% highlight text %}
(gdb) disassemble /r 0x401365,0x401370
(gdb) info break
(gdb) delete 1            # 删除序号为1的断点，如果不加参数，则删除所有
(gdb) break *(0x400990)   # 根据地址设置断点
{% endhighlight %}

### 无符号

在发行版本中，一般会将调试信息删除，但是，因为 mutex 的数据结构是固定的，所以仍然可以通过 gdb 进行查看。

在 `__lll_lock_wait()` 所在的帧处，对于 x86_64 可以通过 `p *(pthread_mutex_t*)$rdi` 查看，而 x86_32 可以通过 `p *(pthread_mutex_t*)$ebx` 查看。

注意，如果没有 `debuginfo` 包，一般会报 `No symbol table is loaded.` 的错误，也就是对应的 `pthread_mutex_t` 符号没有加载，那么可以通过如下方式查看。

{% highlight text %}
(gdb) print *((int*)($rdi))                # lock字段
$4 = 2
(gdb) print *((unsigned int*)($rdi)+1)     # count字段
$5 = 0
(gdb) print *((int*)($rdi)+2)              # owner字段
$6 = 12275
{% endhighlight %}

然后可以通过 `/proc/<PID>/maps` 确定其所属的地址空间，基本确定发生死锁的是本二进制，还是在库中。

<!--
kill -0 <PID> 可以用来判断进程是否存在。

exit() 也可能会失败。

Thread 1 (Thread 0x7f1597429740 (LWP 24703)):
#0  0x00007f159642922c in __lll_lock_wait_private () from /lib64/libc.so.6
#1  0x00007f15963a7694 in _L_lock_4325 () from /lib64/libc.so.6
#2  0x00007f15963a1368 in _int_free () from /lib64/libc.so.6
#3  0x0000000000419417 in abuff_destory ()
#4  0x0000000000437081 in ?? ()
#5  0x00007f159635de09 in __run_exit_handlers () from /lib64/libc.so.6
#6  0x00007f159635de55 in exit () from /lib64/libc.so.6
#7  0x000000000041543f in _start ()

__lll_lock_wait_private() 前面为啥是一个固定的数字，作用是啥？？？？

死锁有几种情况。

1. 信号处理不安全。如果主进程正在通过 malloc() 申请内存，此时发生了中断，而中断处理函数同时会申请或者释放内存，那么此时就可能会发生死锁，即使在同一个线程内。

注意，像 `localtime()` `free()` `malloc()` 等都不是信号安全的。
/post/linux-signal-safe-introduce.html

正常输出，无 -g -rdynamic

 [0]: ./foobar() [0x400775]
 [1]: /lib64/libc.so.6(+0x36280) [0x7fea491ae280]
 [2]: ./foobar() [0x400827]
 [3]: ./foobar() [0x40083d]
 [4]: ./foobar() [0x40085c]
 [5]: /lib64/libc.so.6(__libc_start_main+0xf5) [0x7fea4919a3d5]
 [6]: ./foobar() [0x400689]

在编译的时候最好加上 `-O0` 选项，也就是不进行优化，否则有可能很多函数的调用过程都被替换掉了，有可能通过反汇编出来的信息都不对。

简单来说，`[]` 的地址可以通过 `objdump -d` 反汇编获取到。

-->


{% highlight python %}
{% endhighlight %}
