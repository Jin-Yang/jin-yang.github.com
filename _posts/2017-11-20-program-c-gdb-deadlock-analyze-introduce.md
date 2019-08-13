---
title: GDB 死锁分析
layout: post
comments: true
language: chinese
category: [program]
keywords:  program,c,deadlock
description:
---


<!-- more -->

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


<!--

如果使用了 `-O2` 参数 + `strip` 操作，那么对于一些已经优化的符号 (static inline) 在使用 gdb 的时候就可能不存在。

那么需要添加 `-rdynamic` 参数，那么即使执行了上面的两个操作，那么仍然可以通过 gdb 使用。

不过，因为优化之后可能导致符号对应的地址与原函数不匹配，有可能是在函数的末尾，那么，对于 gdb 来说，就是设置了断点，但是可能不生效。

此时，只能通过如下方法查看其反汇编代码。

(gdb) disassemble /r 0x401365,0x401370

(gdb) info break
(gdb) delete 1            # 删除序号为1的断点，如果不加参数，则删除所有
(gdb) break *(0x400990)   # 根据地址设置断点

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
