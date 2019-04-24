---
title: Linux 线程同步
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: linux,program,pthread
description: 线程的三个主要同步原语：互斥锁 (mutex)、信号量 (semaphore) 和条件变量 (cond)。其中 mutex 和 sem 都是对应 futex 进行了简单的封装，在不存在冲突的情况下就不用陷入到内核中进行仲裁；而且 pthread_join 也是借助 futex 来实现的。简单介绍下 Linux 中与线程相关的编程。
---

线程的三个主要同步原语：互斥锁 (mutex)、信号量 (semaphore) 和条件变量 (cond)。其中 mutex 和 sem 都是对应 futex 进行了简单的封装，在不存在冲突的情况下就不用陷入到内核中进行仲裁；而且 pthread_join 也是借助 futex 来实现的。

简单介绍下 Linux 中与线程相关的编程。

<!-- more -->

<!-- 而 cond 则是依靠 mutex 和信号来实现其语意，也就是说信号存在丢失的情况。-->

## 简介

Linux 线程在核内是以轻量级进程的形式存在的，拥有独立的进程表项，而所有的创建、同步、删除等操作都在核外的 `pthread` 库中进行。

### 同步 VS. 互斥

当有多个线程的时候，经常需要去同步这些线程以访问同一个数据或资源。

例如，假设有一个程序，其中一个线程用于把文件读到内存，而另一个线程用于统计文件中的字符数。当然，在把整个文件调入内存之前，统计它的计数是没有意义的。但是，由于每个操作都有自己的线程，操作系统会把两个线程当作是互不相干的任务分别执行，这样就可能在没有把整个文件装入内存时统计字数。为解决此问题，你必须使两个线程同步工作。

所谓互斥，是指散布在不同进程之间的若干程序片断，当某个进程运行其中一个程序片段时，其它进程就不能运行它们之中的任一程序片段，只能等到该进程运行完这个程序片段后才可以运行。如果用对资源的访问来定义的话，互斥某一资源同时只允许一个访问者对其进行访问，具有唯一性和排它性。但互斥无法限制访问者对资源的访问顺序，即访问是无序的

所谓同步，是指散步在不同进程之间的若干程序片断，它们的运行必须严格按照规定的某种先后次序来运行，这种先后次序依赖于要完成的特定的任务。如果用对资源的访问来定义的话，同步是指在互斥的基础上（大多数情况），通过其它机制实现访问者对资源的有序访问。在大多数情况下，同步已经实现了互斥，特别是所有写入资源的情况必定是互斥的。少数情况是指可以允许多个访问者同时访问资源。


## 锁

### 互斥锁


{% highlight text %}
----- 初始化一个互斥锁，可以通过PTHREAD_MUTEX_INITIALIZER宏进行初始化
int pthread_mutex_init(pthread_mutex_t* mutex, const thread_mutexattr_t* mutexattr);

----- 获取互斥锁，如果被锁定，当前线程处于等待状态；否则，本线程获得互斥锁并进入临界区
int pthread_mutex_lock(pthread_mutex_t* mutex);

----- 与lock操作不同的是，在尝试获得互斥锁失败后，不会进入阻塞状态，而是返回线程继续执行
int pthread_mutex_trylock(pthread_mutex_t* mutex);

----- 释放互斥锁，被阻塞的线程会获得互斥锁然后执行
int pthread_mutex_unlock(pthread_mutex_t* mutex);

----- 用来释放互斥锁的资源
int pthread_mutex_destroy(pthread_mutex_t* mutex);
{% endhighlight %}

使用方式如下。

{% highlight text %}
pthread_mutex_t mutex;
pthread_mutex_init(&mutex,NULL);

void pthread1(void* arg)
{
   pthread_mutex_lock(&mutex);
   ... ...  //临界区
   pthread_mutex_unlock(&mutex);
}

void pthread2(void* arg)
{
   pthread_mutex_lock(&mutex);
   ... ...  //临界区
   pthread_mutex_unlock(&mutex);
}
{% endhighlight %}

### 读写锁

读写锁与互斥量类似，不过读写锁允许更高的并行性，适用于读的次数大于写的次数的数据结构。一次只有一个线程可以占有写模式的读写锁，但是多个线程可以同时占有读模式的读写锁。

{% highlight text %}
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr);

int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);

int pthread_rwlock_unlock(pthread_rwlock_t* rwlock);
int pthread_destroy(pthread_rwlock_t* rwlock);
{% endhighlight %}



### 自旋锁

互斥锁或者读写锁是靠阻塞线程的方式是使其进入休眠，自旋锁是让线程不断循判断自旋锁是否已经被解锁，而不会使其休眠，适用于占用自旋锁时间比较短的场景。

## 条件变量

信号量只有锁和不锁两种状态，当条件变量和信号量一起使用时，允许线程以无竞争的方式等待特定的条件发生。也就是说，条件本身是由互斥量保护，线程在改变条件状态之前必须先锁住互斥量。

{% highlight text %}
----- 动态初始化条件变量，也可以通过PTHREAD_COND_INITIALIZER宏进行初始化
int pthread_cond_init(pthread_cond_t* cond,const pthread_condattr_t* attr);

----- 阻塞等待条件变量为真，注意线程会被条件变量一直阻塞。
int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);

----- 与wait类似，但是可以设置超时时间，在经历tspr时间后，即使条件变量不满足，阻塞也被解除
int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex,const struct timespec* tspr);

----- 唤醒因为条件变量阻塞的线程，至少会唤醒一个等待队列上的线程
int pthread_cond_signal(pthread_cond_t* cond);

----- 唤醒等待该条件的所有线程
int pthread_cond_broadcast(pthread_cond_t* cond);

----- 销毁条件变量
int pthread_cond_destroy(pthread_cond_t* cond);
{% endhighlight %}

注意，如果有多个线程在等待信号的发生，那么在通过 `pthread_cond_signal()` 发送信号时，会根据调度策略选择等待队列中的一个线程。

### 示例

比较典型的应用是生产者和消费者模型。如下的示例中，两个线程共享 `x` 和 `y` 变量，条件变量用于表示 `x > y`  成立。

{% highlight text %}
int x, y;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//----- 等待x>y成立后执行
pthread_mutex_lock(&mut);
while (x <= y) {
        pthread_cond_wait(&cond, &mut);
}
/* operate on x and y */
pthread_mutex_unlock(&mut);

//----- 修改x,y，可能会导致x>y，如果是那么就触发条件变量
pthread_mutex_lock(&mut);
/* modify x and y */
if (x > y)
        pthread_cond_broadcast(&cond);
pthread_mutex_unlock(&mut);
{% endhighlight %}

这个处理逻辑看起来还比较简单，但是为什么在被唤醒后还要进行条件判断，也就是为什么要使用 `while` 循环来判断条件。实际上更多的是为了跨平台的兼容，由于对于调度策略没有明确的规定，那么不同的实现就可能会导致 "惊群效应"。

例如，多个线程在等待信号量，同时被唤醒，那么只有一个线程会进入临界区进行处理，那么此时就必须要再次条件判断。

### 原理

首先重点解释下 `pthread_cond_wait()` 函数，该函数功能可以直接通过 `man 3p pthread_cond_wait` 查看；简单来说，其实现的功能是，释放 `mutex` 并阻塞到 `cond` ，更重要的是，这两步是 **原子操作** 。

在解释之前，为了防止与 NTPL 中的实现混淆，暂时去掉 `pthread` 头，仅为方便说明。

#### 1. 基本条件变量

{% highlight text %}
----- 如上，在等待事件时，通常会通过while()进行条件检测
while(pass == 0)
    cond_wait(...);
 
----- 当条件被改变时，通过signal()函数通知到其它线程
pass = 1;
cond_signal(...)
{% endhighlight %}

#### 2. 互斥量保护

注意，如上的两个线程中都涉及到了共享变量 `pass` ，那么为了防止竞态条件的发生，需要通过加锁进行保护，也就是如下的内容：

{% highlight text %}
mutex_lock(mtx);
while(pass == 0)
    cond_wait(...);
/* more operations on pass */
mutex_unlock(mtx);

mutex_lock(mtx);
pass = 1;
/* more operations on pass */
cond_signal(...);
mutex_unlock(mtx); // 必须要在cond_signal之后，否则同样存在竞态条件
{% endhighlight %}

#### 3. 条件变量语义

实际在上述的示例中，隐含了 `cond_wait()` 必须要自动释放锁，也就是说如果只是等待条件变量，实际的内容如下：

{% highlight text %}
mutex_lock(mtx);
while(pass == 0) {
 mutex_unlock(mtx);
 cond_just_wait(cv);
 mutex_lock(mtx);
}
mutex_unlock(mtx);

mutex_lock(mtx);
pass = 1;
cond_signal(...);
mutex_unlock(mtx);
{% endhighlight %}

<!--
久而久之，程序员发现unlock, just_wait, lock这三个操作始终得在一起。于是就提供了一个pthread_cond_wait()函数来同时完成这三个函数。另外一个证据是，signal()函数是不需要传递mutex参数的，所以关于mutex参数是用于同步wait()和signal()函数的说法更加站不住脚。所以我的结论是：传递的mutex并不是为了防止wait()函数内部的Race Condition！而是因为调用wait()之前你总是获得了某个mutex（例如用于解决此处pass变量的Race Condition的mutex），并且这个mutex在你调用wait()之前必须得释放掉，调用wait()之后必须得重新获取。所以，pthread_cond_wait()函数不是一个细粒度的函数，却是一个实用的函数。

### 竞态条件讨论

其中第一行和第二行的两个操作必须是 "原子化" 的操作，第三行可以分离出去。之所以要求第一、二行必须是原子操作，是因为要保证：如果线程 A 先进入 wait 函数 (可能还在释放mtx)，那么必须保证其它线程在其之后调用 broadcast 时能够将线程 A 唤醒。

对于上述的操作：

mutex_lock(mtx);        // a1
while(pass == 0) {      // a2
 mutex_unlock(mtx);  // a3
 cond_just_wait(cv); // a4
 mutex_lock(mtx);    // a5
}
mutex_unlock(mtx);

mutex_lock(mtx);    // b1
pass = 1;           // b2
cond_signal(...);   // b3
mutex_unlock(mtx);  // b4

如果执行序列是：a1, a2, a3, b1, b2, b3, b4, a4，那么线程 A 将不会被唤醒，也就是说 wait() 是在 signal() 之前调用的，所以不满足上文提到的保证。


先将线程附加到等待队列
释放mutex
进入等待

感兴趣的同学的可以看下源码（pthread_cond_wait.c），附加到等待队列这个操作是加锁的，所以可以保证之前发起的signal不会错误得唤醒本线程，而之后发起的signal必然唤醒本线程。因此，下面的代码是绝对不会出错的：// 线程A，条件测试
pthread_mutex_lock(mtx);        // a1
while(pass == 0) {              // a2
    pthread_cond_wait(cv, mtx); // a3
}
pthread_mutex_unlock(mtx);      // a4

// 线程B，条件发生修改，对应的signal代码
pthread_mutex_lock(mtx);   // b1
pass = 1;                  // b2
pthread_mutex_unlock(mtx); // b3
pthread_cond_signal(cv);   // b4
如果线程A先运行，那么执行序列必然是：a1, a2, a3, b1, b2, b3, b4, a4。如果线程B先运行，那么执行序列可能是：b1, b2, b3, b4, a1, a2, a4也可能是：b1, b2, b3, a1, a2, a3, b4, a4

http://blog.sina.com.cn/s/blog_967817f20101bsf0.html
另外需要了解Memory Barrier和Automatic Operation，无锁编程
pthread_cleanup_push
?????pthread_cancel 和取消点
-->


## 信号量


{% highlight text %}
----- 初始化信号量，pshared为0只能在当前进程的线程间共享，否则可以进程间共享，value给出了信号量的初始值
int sem_init(sem_t* sem, int pshared,unsigned int value);
----- 阻塞线程，直到信号量的值大于0，解除阻塞后同时会将sem的值减一
sem_wait(sem_t* sem);
----- 是wait的非阻塞版本
sem_trywait(sem_t* sem);
----- 增加信号量的值，会利用线程的调度策略选择一个已经被阻塞的线程
sem_post(sem_t* sem);
----- 释放信号量所占用的资源
sem_destroy(sem_t* sem);
{% endhighlight %}


<!--
pthread_mutex_t mutex;
sem_t full,empty;

void producer(void* arg){
    while(1){
    sem_wait(&empty);//need to produce. the the empty of resource need minus 1
    pthread_mutex_lock(&mutex);
    ...//produce a resource
    pthread_mutex_unlock(&mutex);
    sem_post(&full); //have produced a resource, the the full of resource need add 1
    }
}
void consumer(void* arg){
    while(1){
    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    ...//consume a resource
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
    }
}
-->

## 取消点

一个线程可以通过 `pthread_cancel()` 向另外一个线程发送结束请求，接收此请求的线程可以通过线程的两个属性来决定是否取消，以及是同步 (延时) 取消还是异步 (立即) 取消；而且即使该函数调用成功返回，也不代表线程就结束了。

可以通过如下的两个函数设置线程的属性：

{% highlight text %}
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);

PTHREAD_CANCEL_ENABLE
  可取消，不过有两种取消方式，一种是同步取消，另外一种是异步取消。
PTHREAD_CANCEL_DISABLE
  不可取消，那么接收取消请求后，它依然继续执行。
PTHREAD_CANCEL_DEFERRED
  可取消，会在下一个取消点退出；简单来说，取消点就是程序在运行的时候检测是否收到取消请求，常见的有如下函数：
    pthread_join() pthread_cond_wait() pthread_cond_timedwait() pthread_testcancel() sem_wait() sigwait()
PTHREAD_CANCEL_ASYNCHRONOUS
  异步取消，也就是说线程在收到取消请求之后，立即取消退出。
{% endhighlight %}



## 线程退出

一般来说，POSIX 的线程终止有两种情况：A) 正常退出，线程主动调用 pthread_exit() 或者从线程函数中 return 都将使线程正常退出，这是可预见的退出方式；B) 非正常终止，是指线程在其它线程的干预下，或者由于自身运行出错 (如访问非法地址) 而退出，这种退出方式是不可预见的。

无论是那种方式，都会存在资源释放的问题，在不考虑因运行出错而退出的前提下，如何保证线程终止时能顺利的释放掉自己所占用的资源，特别是锁资源，就是一个必须考虑解决的问题。

最常见的场景是，线程为了访问临界资源而为其加锁，但在访问过程中被外界取消，如果允许异步取消，或者在打开独占锁之前的运行路径上存在取消点，则该临界资源将永远处于锁定状态得不到释放。

### 清理函数

默认情况下，一个线程是可取消的并且是同步取消的。对于可以取消的线程，需要考虑退出时该线程后续的资源清理，例如，线程执行到一个锁内时，已经获得锁了，这时异常退出了，那么此时就是一个死锁的问题。

POSIX 提供了如下的两个函数用于注册自动释放资源，也就是在 push 和 pop 之间的终止动作，包括 pthread_exit() 和取消点的取消。

{% highlight text %}
#include <pthread.h>
void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
{% endhighlight %}

可多次调用 push 函数，将采用 FIFO 方式管理，这两个函数实际上是宏定义，定义如下：

{% highlight c %}
#define pthread_cleanup_push(routine,arg)                   \
	{                                                   \
		struct _pthread_cleanup_buffer _buffer;     \
		_pthread_cleanup_push (&_buffer, (routine), (arg));

#define pthread_cleanup_pop(execute)                        \
		_pthread_cleanup_pop (&_buffer, (execute)); \
	}
{% endhighlight %}

上述使用了两个 `{` `}` ，导致这两个函数必须成对出现，而且必须位于程序的同一级别的代码段中才能通过编译，如下是常见的调用方式。

如果函数没有异常退出，那么可以通过 `phread_cleanup_pop()` 弹出这些栈里的函数，此时的参数要为 0，如果非 0 的话，弹出的同时也会执行这些函数的。例如，对于死锁问题可以做如下处理：

{% highlight text %}
pthread_cleanup_push(pthread_mutex_unlock, (void *) &mutex);
pthread_mutex_lock(&mutex);
/* do some work */
pthread_mutex_unlock(&mutex);
pthread_cleanup_pop(0);
{% endhighlight %}

## 死锁实例

下面是一段在 Linux 平台下能引起线程死锁的小例子。

{% highlight c %}
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *thread1(void *arg)
{
	(void) arg;
	printf("Threads #1 enter\n");
	pthread_mutex_lock(&mutex);
	printf("Threads #1 111\n");
	pthread_cond_wait(&cond, &mutex);
	printf("Threads #1 222\n");
	pthread_mutex_unlock(&mutex);
	printf("Threads #1 quit\n");
	pthread_exit(NULL);
}

void *thread2(void *arg)
{
	(void) arg;
	printf("Threads #2 enter\n");
	sleep(10);
	printf("Threads #2 mutex\n");
	pthread_mutex_lock(&mutex);
	printf("Threads #2 111\n");
	pthread_cond_broadcast(&cond);
	printf("Threads #2 222\n");
	pthread_mutex_unlock(&mutex);
	printf("Threads #2 quit\n");
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, thread1, NULL);
	pthread_create(&tid2, NULL, thread2, NULL);
	printf("Create 2 threads\n");

	sleep(5);
	printf("Cancel thread #1\n");
	pthread_cancel(tid1);

	printf("Join threads\n");
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	return 0;
}
{% endhighlight %}

这个实例程序仅仅是使用了条件变量和互斥量进行一个简单的线程同步：thread1 等待条件变量，thread2 启动 10s 后发送广播信号尝试唤醒 thread1 。

如果将 `pthread_cancel()` 注释掉，实际上程序可以按照预期执行，问题是，具体是什么原因导致的？















<!--
## futex

Futex 可以简单理解为保存锁的状态以及一个等待该锁的任务等待队列。

### 源码解析

内核中的实现在 `kernel/futex.c` 文件中，

初始化时会创建一个 hash 表，
http://blog.csdn.net/npy_lp/article/details/7331245

当非竞态时，只需要在用户空间处理即可，只有在出现竞态时才会进入到内核空间中；而且针对非竞态进行了优化。

其中地址是整数对齐的，而且只能通过原子的汇编指令操作。

futex_init()
 |-alloc_large_system_hash()  分配名为futex的hash表
 |-futex_detect_cmpxchg()

-->

## 参考

<!--
http://www.cnblogs.com/zhaoyl/p/3620204.html
http://blog.csdn.net/zmxiangde_88/article/details/7997874

http://www.cnblogs.com/blueclue/archive/2010/06/11/1754899.html
https://yangwenbo.com/articles/linux-implementation-of-posix-thread-cancellation-points.html
-->

<!--
线程会在主体函数退出时自动终止，同时也可以在接收到另一个线程发来的终止请求而强制终止。

主动停止是一个线程向目标发送 Cancel 信号，但是如何处理这个 Cancel 信号则是由目标线程自己决定，可以选择忽略、立即停止(在取消点或者异步模式下)、或者在下一个取消点退出。

默认是在接收到 Cancel 信号后在下个取消点退出，在异步方式下直接退出，其退出操作等同于调用 `pthread_exit(PTHREAD_CANCELED)` 函数；另外，需要注意线程的取消与线程的工作方式 (Joinable/Detached) 无关。

### 取消点

根据 POSIX 标准，pthread_join()、pthread_testcancel()、pthread_cond_wait()、pthread_cond_timedwait()、sem_wait()、sigwait() 等函数以及 read()、write() 等会引起阻塞的系统调用都是取消点，而其他 pthread 函数都不会引起取消动作。

不过有些和 C 库结合不好，CANCEL 信号会使线程从阻塞的系统调用中退出，并置 EINTR 错误码。

4 程序设计方面的考虑

1. 如果线程处于无限循环中，且循环体内没有执行至取消点的必然路径，则线程无法由外部其他线程的取消请求而终止。因此在这样的循环体的必经路径上应该加入pthread_testcancel()调用。

2. 当pthread_cancel()返回时，线程未必已经取消，可能仅仅将请求发送给目标线程，而目标线程目前没有到达取消点，如果要知道线程在何时中止，就需要在取消它之后调用pthread_join()。有一个例外是当线程被detach后，不能这样处理：
a) 当join一个已经detached的线程时，返回EINVAL；
b) 如果join后该线程设置为detached，则detach将不起作用。
因此，如果知道一个线程可能会以分离方式运行，就不需要在pthread_cancel()后调用pthread_join()。

int pthread_cancel(pthread_t thread);
 发送Cancel信号给线程，成功返回0，否则是非0，注意发送成功并不意味着对应的线程会立即终止。
int pthread_setcancelstate(int state, int *oldstate);
 设置本线程对Cancel信号的响应，其中state有两种值 PTHREAD_CANCEL_ENABLE 和 PTHREAD_CANCEL_DISABLE，如果 oldstate 非空则保存之前的状态以便恢复。
int pthread_setcanceltype(int type, int *oldtype);
    设置本线程取消动作的执行时机，只有当上述设置为 Enable 时才有效，其取值类型同样有两种 PTHREAD_CANCEL_DEFFERED 和 PTHREAD_CANCEL_ASYCHRONOUS，分别表示运行到下个取消点退出还是立即退出。
void pthread_testcancel(void);
    检查本线程是否处于Canceld状态，如果是，则进行取消动作，否则直接返回。
int pthread_kill(pthread_t thread, int sig);
    向指定线程发送信号，如果线程代码没有实现信号处理函数，则会执行默认行为，一般是直接退出进程。其中 0 是保留值，用于判断线程是否存在。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *func(void *args)
{
        (void) args;
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        while (1) {
                printf("thread before sleep\n");
                //pthread_testcancel();
                sleep(1);
                //pthread_testcancel();
                printf("thread after sleep\n");
        }
        return NULL;
}

int main(void)
{
        pthread_t thrd;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&thrd, &attr, func, NULL)) {
                perror("pthread_create error");
                exit(EXIT_FAILURE);
        }

        if (pthread_cancel(thrd) == 0)
                printf("pthread_cancel OK\n");
        //pthread_join(thrd, NULL);

        //sleep(10);
        printf("Main routine quit\n");

        return 0;
}
{% endhighlight %}

如上的示例中 sleep() 也是一个取消点，可以通过如下线程函数进行测试，如果没有 `pthread_testcancel()` 实际上无法取消。

void *thread_fun(void *arg) 
{ 
    int i=1; 
    printf("thread start \n"); 
    while(1) 
    { 
        i++; 
        pthread_testcancel(); 
    } 
    return (void *)0; 
}




### 分析

进一步打印的话，thread1 执行到了 pthread_cond_wait() 处，也就是释放了 mutex；而 thread2 执行时会阻塞到 pthread_mutex_lock() 函数处。

实际上 pthread_cond_wait() 是取消点之一，那么正常来说在这一点已经可以取消掉了，所以 thread1 应该执行到最后的退出并结束，此时 mutex 已经解锁了，不应该发生死锁的。

如上所述，取消类型有两种：延迟取消 DEFERRED，系统默认的取消类型，即在线程到达取消点之前，不会出现真正的取消；另外一种是异步取消 ASYNCHRONOUS，使用异步取消时，线程可以在任意时间取消。

而实际上及时通过 `pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);` 设置了异步，同样会发生死锁。

### 取消点实现

严格来说是 GNU 中取消点的实现，因为 pthread 是在 glibc 中实现的，现在也就是 NPTL 的实现方式。

其中 pthread_cond_wait() 实现的源码在 nptl/pthread_cond_wait.c 中，实际调用的是 `futex_wait_cancelable()` 函数：

/* 先将线程取消类型设置为异步取消 */
oldtype = __pthread_enable_asynccancel ();
int err = lll_futex_timed_wait (futex_word, expected, NULL, private);
/* 当线程被唤醒时，线程取消类型被修改回之前的值，一般是延迟取消 */
__pthread_disable_asynccancel (oldtype);

这就意味着，所有在 `__pthread_enable_asynccancel()` 之前接收到的取消请求都会等待 __pthread_enable_asynccancel 执行之后进行处理，所有在 __pthread_disable_asynccancel 之前接收到的请求都会在 __pthread_disable_asynccancel 之前被处理，所以真正的 Cancellation Point 是在这两点之间的一段时间。

对于上述的额示例，在 main 函数调用 pthread_cancel 前，thread1 已经进入了 pthread_cond_wait 函数并将自己列入等待条件的线程列表中 (lll_futex_wait)；而当 pthread_cancel 被调用时，tid1 线程仍在等待，取消请求发生在 __pthread_disable_asynccancel 前，所以会被立即响应。

但是 pthread_cond_wait 为注册了一个线程清理程序，

/* Before we block we enable cancellation. Therefore we have to install a cancellation handler.  */
__pthread_cleanup_push (&amp;buffer, __condvar_cleanup, &amp;cbuffer);

那么这个线程清理程序 __condvar_cleanup 干了什么事情呢？我们可以注意到在它的实现最后（glibc-2.6/nptl/pthread_cond_wait.c）：

/* Get the mutex before returning unless asynchronous cancellation is in effect.  */
__pthread_mutex_cond_lock (cbuffer->mutex);

也就是 __condvar_cleanup 在最后将 mutex 重新锁上了。而这时候 thread1 还在休眠(sleep(10))，等它醒来时，mutex 将会永远被锁住，这就是为什么 thread1 陷入无休止的阻塞中。


6. 如何避免因此产生的死锁

由于线程清理函数 pthread_cleanup_push 使用的策略是先进后出(FILO)，那么我们可以在 pthread_cond_wait 函数前先注册一个线程处理函数：

void cleanup(void *arg)
{
  pthread_mutex_unlock(&amp;mutex);
}
void* thread0(void* arg)
{
  pthread_cleanup_push(cleanup, NULL);  // thread cleanup handler
  pthread_mutex_lock(&amp;mutex);
  pthread_cond_wait(&amp;cond, &amp;mutex);
  pthread_mutex_unlock(&amp;mutex);
  pthread_cleanup_pop(0);
  pthread_exit(NULL);
}

这样，当线程被取消时，先执行 pthread_cond_wait 中注册的线程清理函数 __condvar_cleanup，将 mutex 锁上，再执行 thread0 中注册的线程处理函数 cleanup，将 mutex 解锁。这样就避免了死锁的发生。

7. 结论

多线程下的线程同步一直是一个让人很头痛的问题。POSIX 为了避免立即取消程序引起的资源占用问题而引入的 Cancellation Points 概念是一个非常好的设计，但是不合适的使用 pthread_cancel 仍然会引起线程同步的问题。了解 POSIX 线程取消点在 Linux 下的实现更有助于理解它的机制和有利于更好的应用这个机制。

pthread_cleanup_push(pthread_mutex_unlock, (void*) &mut);
pthread_mutex_lock(&mut);
/* do some work */
pthread_mutex_unlock(&mut);
pthread_cleanup_pop(0);
或者
void cleanup(void *arg)
{   
    pthread_mutex_unlock(&mutex);
}

void* thread0(void* arg)
{   
    pthread_cleanup_push(cleanup, NULL); // thread cleanup handler    p
    thread_mutex_lock(&mutex);   
    pthread_cond_wait(&cond, &mutex);   
    pthread_mutex_unlock(&mutex);   
    pthread_cleanup_pop(0);   
    pthread_exit(NULL);
}
-->


实际上 Mutex 锁的性能很好，可以参考 [Mutex锁的性能](http://chenyufei.info/blog/2012-12-26/pthread-mutex-is-fast-on-linux/) 。



{% highlight text %}
{% endhighlight %}
