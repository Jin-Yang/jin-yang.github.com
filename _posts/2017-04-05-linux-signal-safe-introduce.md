---
title: Linux 信号安全
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,signal,信号安全
description: 最近遇到比较奇葩的问题，在信号处理函数中，为了方便查看收到的是何信号，会打印相关的日志，不过也因此在连续收到信号时导致死锁。这里简单排查下原因，以及如何进行规避。
---

最近遇到比较奇葩的问题，在信号处理函数中，为了方便查看收到的是何信号，会打印相关的日志，不过也因此在连续收到信号时导致死锁。

这里简单排查下原因，以及如何进行规避。

<!-- more -->

## 检查当前调用栈

查看当前的调用栈。

{% highlight text %}
----- 通过pstack查看
$ pstack PID

----- 通过gdb连接过去
$ gdb attach PID
(gdb) info thread    各线程的栈信息
(gdb) thread apply all backtrace 所有线程的栈信息，类似于pstack命令
(gdb) thread 5       切换到某个线程
(gdb) where          查看当前栈信息
#0  0x0000003d1a80d4c4 in __lll_lock_wait () from /lib64/libpthread.so.0
#1  0x0000003d1a808e1a in _L_lock_1034 () from /lib64/libpthread.so.0
#2  0x0000003d1a808cdc in pthread_mutex_lock () from /lib64/libpthread.so.0
#3  0x0000000000400a9b in func1 () at lock.cpp:18
#4  0x0000000000400ad7 in thread1 (arg=0x0) at lock.cpp:43
#5  0x0000003d1a80673d in start_thread () from /lib64/libpthread.so.0
#6  0x0000003d19cd40cd in clone () from /lib64/libc.so.6
(gdb) frame 3        切换到加锁API函数的上一层
(gdb) print your_mutex  查看锁信息
{% endhighlight %}

如果线程阻塞的栈不变，一般为 `__lll_lock_wait()` 或者 `__lll_lock_wait_private()` ，那么基本可以确定是由于发生了死锁导致。

### 示例

如下是一个可能发生死锁的示例程序。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void int_handler(int signum)
{
        time_t tt;
        char timestr[12];
        struct tm timenow;

        time(&tt);
        localtime_r(&tt, &timenow);
        strftime(timestr, sizeof(timestr), "%m%d-%H%M%S", &timenow);
        printf("%s Got a int signal %d\n", timestr, signum);
}

void quit_handler(int signum)
{
        printf("%ld Got a quit signal %d\n", time(NULL), signum);
}

int main()
{
        time_t now;
        struct tm ltime;

        signal(SIGINT, int_handler);
        signal(SIGQUIT, quit_handler);

        now = time(NULL);
        while(1)
                localtime_r(&now, &ltime);
        return 0;
}
{% endhighlight %}

可以连续发送多次信号进行测试，或者使用如下命令。

{% highlight text %}
while true; do pid=`pidof your-program`; if [ -n "$pid" ]; then kill $pid; sleep 0.01; else sleep 1; fi; done

while true; do ./daemon/your-program; echo "start" ; done
{% endhighlight %}


<!--
排查示例
https://www.ibm.com/developerworks/cn/linux/l-cn-deadlock/
-->


## localtime死锁

简单来说，对应的堆栈为。

{% highlight text %}
#0  0x0000003f6d4f805e in __lll_lock_wait_private () from /lib64/libc.so.6
#1  0x0000003f6d49dcad in _L_lock_2164 () from /lib64/libc.so.6
#2  0x0000003f6d49da67 in __tz_convert () from /lib64/libc.so.6
{% endhighlight %}

### 源码解析

实际上在 [time/localtime.c](https://github.com/bminor/glibc/blob/09533208febe923479261a27b7691abef297d604/time/localtime.c) 对该函数有如下的实现：

{% highlight c %}
/* Return the `struct tm' representation of *T in local time,
   using *TP to store the result.  */
struct tm *
__localtime_r (const time_t *t, struct tm *tp)
{
  return __tz_convert (t, 1, tp);
}
weak_alias (__localtime_r, localtime_r)

/* Return the `struct tm' representation of *T in local time.  */
struct tm *
localtime (const time_t *t)
{
  return __tz_convert (t, 1, &_tmbuf);
}
libc_hidden_def (localtime)
{% endhighlight %}


也就是说，无论 `localtime()` 还是 `localtime_r()` 都是调用 `__tz_convert()` 完成实际功能，该函数的实现在 `time/tzset.c` 文件中。

其中有一部分代码是通过 `__libc_lock_lock (tzset_lock);` 加锁后的处理，而该锁是通过 `__libc_lock_define_initialized (static, tzset_lock)` 定义的 static 全局变量。

`localtime()` 和 `localtime_r()` 的实现都通过加锁实现了访问，但是 `localtime()` 同时会使用一个全局变量，所以后者不是线程安全的。

但这两个函数都不是信号安全的，如果在信号处理函数中使用，就要考虑到死锁的情况。比如，程序调用 `localtime_r()`，加锁后信号发生，信号处理函数中也调用 `localtime_r()` 的话，会因为获取不到锁所以一直阻塞。

### 死锁场景

最常见的是，也就是上述的，在日志打印时间调用了 `localtime()` 函数，而在信号处理函数中同时会打印日志，那么就可能会出现这一问题。

如果使用的是多进程，各个 localtime() 的调用都是安全的，另外，还有一个场景，是在多线程中同时 `fork()` 子进程。

后面的场景中，因为变量是共享的，那么如果多线程 `fork()` 子进程，而此时的某个线程在该函数的加锁阶段，子进程以 COW 方式共享主进程的内存空间，所以对应 `localtime()` 的锁也是被占用的情况，那么就可能导致子进程一直阻塞。

### 解决方案

对于部分场景，如果我们对锁有控制权，那么就可以在调用 `fork()` 创建子进程前，通过 glibc 库提供的函数 `pthead_atfork()` 加解锁，达到一致状态。

{% highlight c %}
#include <pthread.h>
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
{% endhighlight %}

简单来说，创建子进程前在父进程中会调用 prepare 函数；创建子进程成功后，父进程会调用 parent 而子进程会调用 child 。这样，可以在 prepare 中释放所有的锁，parent 中按需要进行加锁。

由于没有办法操作 localtime 使用的锁，所以上述方式行不通。这样，只能是选择折中的办法，例如日志可以通过定时更新时间缓存的方式执行。

<!--
#### time

一般日志打印时间时，先通过 time() 函数获取时间戳，返回 time_t(long int) 类型的值，然后再进行格式化。

常用时间转换函数：

struct tm {
	int tm_sec;         /* seconds */
	int tm_min;         /* minutes */
	int tm_hour;        /* hours */
	int tm_mday;        /* day of the month */
	int tm_mon;         /* month */
	int tm_year;        /* year */
	int tm_wday;        /* day of the week */
	int tm_yday;        /* day in the year */
	int tm_isdst;       /* daylight saving time */
};
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
struct timeval{
    time_t tv_sec;
    suseconds_t tv_usec;
};

time_t time(time_t *t);
   获取从1970-01-01 00:00:00 +0000(UTC)开始计算的秒数，返回值和入参值相同；
char *asctime(const struct tm *tm);
   将一个tm结构体转换为字符串形式的时间；
char *ctime(const time_t *timep);
   将秒数转换为一个字符串行为的时间；
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
   将秒数转换为一个本地的tm结构体，分别是GMT时间和本地时间；
time_t mktime(struct tm *tm);
   将tm结构体转换成时间戳；
double difftime(time_t time1, time_t time0);
   比较并返回相差的秒数；
int gettimeofday(struct timeval *tv, struct timezone *tz);
   获取时间以及时区信息。

tzset

#include <time.h>
#include <stdio.h>
#include <unistd.h>

void show_tm(struct tm * t)
{
    printf("%d-%d-%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
    printf("  ");
    printf("%d:%d:%d\n", t->tm_hour, t->tm_min, t->tm_sec);
}

int main(void)
{
        time_t begin, end;
        struct tm * t;

        begin = time(NULL);
        printf("Current time %ld\n", begin);
#if 0
		t = localtime(&curr_time);
		show_tm(t);
        printf("%s", ctime(&curr_time));
        printf("%s", asctime(t));
#endif
        sleep(1);
        end = time(NULL);
        printf("End     time %ld\n", end);
        printf("Differe time %f\n", difftime(end, begin));

        return 0;
}
-->

### 关于 pthread_atfork()

当父进程有多线程时，子进程继承父进程所有的互斥量、读写锁和条件变量的状态，如果父进程中的线程占有锁 (任一线程)，那么子进程同样占有这些锁，当尝试重新获取锁时会导致一直阻塞。

如果子进程马上调用 exec 类函数，老的地址空间被丢弃，所以锁的状态无关紧要；否则，就需要清除锁的状态。

{% highlight c %}
#include <wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;

void *another(void *arg)
{
        (void) arg;
        printf("Sub-thread lock\n");
        pthread_mutex_lock(&mutex);
        printf("Sub-thread locking\n");
        sleep(2);
        pthread_mutex_unlock(&mutex);
        printf("Sub-thread unlock\n");
        return NULL;
}

void child()
{
        pthread_mutex_unlock(&mutex);
}

int main()
{
        pthread_t tid;

        pthread_mutex_init(&mutex, NULL);
        pthread_create(&tid, NULL, another, NULL);

        sleep(1); /* Just ensure the thread got mutex */

        pthread_atfork(NULL, NULL, child);
        int pid = fork();
        if(pid < 0) {
                pthread_join(tid, NULL);
                pthread_mutex_destroy(&mutex);
                return 1;
        } else if (pid == 0) { /* child */
                printf("Sub-process lock\n");
                pthread_mutex_lock(&mutex);
                printf("Sub-process locking\n");
                pthread_mutex_unlock(&mutex);
                printf("Sub-process unlock\n");
                exit(0);
        } else {
                wait(NULL);
        }
        pthread_join(tid, NULL);
        pthread_mutex_destroy(&mutex);
        printf("Main routine exit\n");

        return 0;
}
{% endhighlight %}

为了解决上述的死锁问题，需要在 `fork()` 调用前加入 `pthread_atfork()` 对应的代码。

一般使用的方式是，在 prepare 中执行加锁，在 parent 和 child 中实现解锁，这样可以保证在进入子进程前已经获得了锁，而在子进程中释放锁。

测试发现，锁可以多次释放，因此可以在进入子进程时把锁都释放掉。

## 线程安全、信号安全

一般来说线程是操作系统调度的最小单元，进程是资源分配的最小单元；一个进程可以派生多个线程，这些线程独立运行共享进程资源，那么在使用共享资源时，就需要考虑避免竞争条件、死锁、互斥等。

### 线程安全 Thread-Safe

在多线程 (单线程不存在) 并发执行场景中，如果一个函数可以安全地被多个线程并发调用，可以说这个函数是线程安全的。也就是说，一个线程安全的函数允许任意地被任意的线程调用，其它开发只需要关注业务逻辑。

有时候很难判断一个是否线程安全，不过如果有如下几条，那么说明这个函数是线程不安全的：

1. 函数中访问、分配全局变量和堆。
2. 使用了其他线程不安全的函数或者变量。

因此在编写线程安全函数时，要注意两点：

1. 减少对临界资源的依赖，尽量避免访问全局变量、静态变量或其它共享资源，如果必须要使用则需要添加互斥锁；
2. 线程安全的函数所调用到的函数也应该是线程安全的，如果调用了非线程安全函数，同样需要加互斥锁保护。


### 可重入 Re-entrant

一个函数想要成为可重入的函数，必须满足下列要求：

* 不能使用静态或者全局的非常量数据
* 不能够返回地址给静态或者全局的非常量数据
* 函数使用的数据由调用者提供
* 不能够依赖于单一资源的锁
* 不能够调用非可重入的函数

在 [OpenGroup Definitions](http://pubs.opengroup.org/onlinepubs/000095399/basedefs/xbd_chap03.html) 中有对上述三个概念的介绍。

{% highlight text %}
Reentrant Function
A function whose effect, when called by two or more threads, is guaranteed to be as if
the threads each executed the function one after another in an undefined order, even if
the actual execution is interleaved.

Thread-Safe
A function that may be safely invoked concurrently by multiple threads. Each function
defined in the System Interfaces volume of IEEE Std 1003.1-2001 is thread-safe unless
explicitly stated otherwise. Examples are any "pure" function, a function which holds
a mutex locked while it is accessing static storage, or objects shared among threads.

Async-Signal-Safe Function
A function that may be invoked, without restriction, from signal-catching functions.
No function is async-signal-safe unless explicitly described as such.
{% endhighlight %}

简单来说：

{% highlight text %}
Reentrant:
   不使用全局变量；
   不调用non-reentrant函数。
Thread-safe:
   可以访问全局变量，不过需要加锁
   每次调用它返回不同的结果也没关系
Async-Signal-Safe:
   只有几个固定的函数是 signal-safe 的，可以通过 man 7 signal 查看；
   使用了锁的一定不是信号安全的，除非屏蔽了信号；
{% endhighlight %}

可重入函数一定是线程安全的，也是异步信号安全。

Nginx、MySQL 都分别实现了一堆的格式化函数，如 `ngx_vslprintf()`、`my_safe_snprintf()`，同时 Nginx 中的时间是定时更新的。

## 总结

总结一下，这种有全局锁的函数都不是信号安全的，比如 localtime()、gmttime()、free()、malloc() 等，但是无法使用 pthread_atfork() 来清理，因此在多线程中使用 fork 需要谨慎。

关于信号安全的函数可以通过 `man 7 signal` 查看。


{% highlight text %}
{% endhighlight %}
