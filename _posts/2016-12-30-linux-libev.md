---
title: libev 使用简介
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,program,libev,event loop
description: libev 与 libevent 类似，是一个 C 语言编写的，高性能的事件循环库，在此详细介绍下。
---

libev 是一个 C 语言编写的，高性能的事件循环库，与此类似的事件循环库还有 libevent、libubox 等，在此详细介绍下 libev 相关的内容。

<!-- more -->

## 简介

关于 libev 详见官网 [http://software.schmorp.de](http://software.schmorp.de/pkg/libev.html)，其帮助文档可以参考 [官方文档](http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod)，安装完之后，可通过 `man 3 ev` 查看帮助信息，文档也在源码中保存了一份，可以通过 `man -l ev.3` 命令查看。

### 安装

安装可以源码安装，或者在 CentOS 中，最简单可通过如下方式安装。

{% highlight text %}
----- 安装库
# yum install libev libev-devel
{% endhighlight %}

### 示例程序

如下是一个简单的示例程序。

{% highlight c %}
#include <ev.h>
#include <stdio.h>

// every watcher type has its own typedef'd struct with the name ev_TYPE
ev_io stdin_watcher;
ev_timer timeout_watcher;

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void stdin_cb (EV_P_ ev_io *w, int revents)
{
	puts ("stdin ready");
	// for one-shot events, one must manually stop the watcher
	// with its corresponding stop function.
	ev_io_stop (EV_A_ w);

	// this causes all nested ev_run's to stop iterating
	ev_break (EV_A_ EVBREAK_ALL);
}

// another callback, this time for a time-out
static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
	puts ("timeout");
	// this causes the innermost ev_run to stop iterating
	ev_break (EV_A_ EVBREAK_ONE);
}

int main (void)
{
	// use the default event loop unless you have special needs
	// struct ev_loop *loop = EV_DEFAULT; /* OR ev_default_loop(0) */
	EV_P EV_DEFAULT; 

	// initialise an io watcher, then start it
	// this one will watch for stdin to become readable
	ev_io_init(&stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
	ev_io_start(EV_A_ &stdin_watcher);

	// initialise a timer watcher, then start it
	// simple non-repeating 5.5 second timeout
	ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
	ev_timer_start(EV_A_ &timeout_watcher);

	ev_loop(EV_A_ 0); /* now wait for events to arrive */

	return 0;
}
{% endhighlight %}

可以通过如下命令编译。

{% highlight text %}
----- 编译示例程序
$ gcc -lev example.c -o example
{% endhighlight %}

### 执行过程

libev 是一个事件循环，首先需要注册感兴趣的事件，libev 会监控这些事件，当事件发生时调用相应的处理函数，也就是回调函数。其处理过程为：

1. 初始化一个事件循环。可以通过 `ev_default_loop(0)` 或者 `EV_DEFAULT` 初始化，两者等价。
2. 定义事件类型。在 `ev.h` 中定义了各种类型的，如 `ev_io`、`ev_timer`、`ev_signal` 等。
3. 注册感兴趣事件。这里被称为 watchers ，这个是 C 结构体。
4. 启动监控。启动上步注册的事件，如 `ev_io_start()`、`ev_timer_start()` 等。
5. 启动 libev 循环。重复 1, 2 步，然后启动 libev 事件循环，直接执行 `ev_run()` 即可。

### 循环体

可以通过 `ev_default_loop()` 初始化一个默认循环，如果支持多实例可以使用 `ev_loop_new()` 创建一个新的，其中包括了部分入参用来标示如何进行处理。

可用标示有。

{% highlight c %}
enum {
	/* the default */
	EVFLAG_AUTO      = 0x00000000U, /* not quite a mask */
	/* flag bits */
	EVFLAG_NOENV     = 0x01000000U, /* do NOT consult environment */
	EVFLAG_FORKCHECK = 0x02000000U, /* check for a fork in each iteration */
	/* debugging/feature disable */
	EVFLAG_NOINOTIFY = 0x00100000U, /* do not attempt to use inotify */
#if EV_COMPAT3
	EVFLAG_NOSIGFD   = 0, /* compatibility to pre-3.9 */
#endif
	EVFLAG_SIGNALFD  = 0x00200000U, /* attempt to use signalfd */
	EVFLAG_NOSIGMASK = 0x00400000U  /* avoid modifying the signal mask */
};
{% endhighlight %}

### 事件设置方式

这里以 `struct ev_io` 为例，有如下的两种设置方式。

将回调函数和关心的事件同时注册。

{% highlight c %}
struct ev_io wstdin;
ev_io_init(&wstdin, stdin_hook, /*STDIN_FILENO*/ 0, EV_READ);
ev_io_start(EV_A &wstdin);
{% endhighlight %}

一般来说，在设置了回调函数之后，很少会进行修改，在首次调用的时候需要修改关注的事件，那么此时就可以将设置回调和设置事件分开。

{% highlight c %}
struct ev_io wstdin;
ev_init(&wstdin, stdin_hook);
... ... /* some time later. */
ev_io_set(&wstdin, /*STDIN_FILENO*/ 0, EV_READ);
ev_io_start(EV_A &wstdin);
{% endhighlight %}


## 使用示例

除了基础的 IO、定时器、信号的处理之外，同时还提供了一些循环中经常使用的 hook 处理，以及一些常用的场景。

{% highlight text %}
### 基础事件能力
struct ev_io        IO事件，包括了Socket、Pipe
struct ev_timer     定时器，采用的是相对时间，基于Bin-Heap
struct ev_periodic  定时器，采用的是UTC时间，基于Bin-Heap
struct ev_signal    = 信号处理
struct ev_child     = SIGCHLD信号的处理
struct ev_stat      文件的监控，Linux中用的是inotify机制

### 扩展事件
struct ev_fork      = 是否是在子进程中运行 使用forks数组
struct ev_embed
struct ev_async     = 异步事件，内部采用Pipe实现 使用asyncs数组

### 循环流程Hook
struct ev_idle      空闲 使用idles数组
struct ev_prepare   每次循环在阻塞之前调用 使用prepares数组
struct ev_check     每次循环在事件处理之后调用 使用checks数组
struct ev_cleanup   当循环退出时会调用 使用cleanups数组，会在ev_loop_destroy()中触发
{% endhighlight %}

<!--
ev_fork 每个循环会检查 PID 是否被修改，如果被修改了，一般意味着到了子进程中运行。在主循环中会检测，同时可以通过 `ev_loop_fork()` 触发。

注意，检测 PID 是否被修改是必选的，用来处理在子进程中的事件处理，也就是会调用 `loop_fork()` 函数。

https://www.cnblogs.com/wunaozai/p/3955156.html
https://blog.csdn.net/yahstudio/article/details/48681481

http://oserror.com/backend/libeasy-all/

内部会维护一个 Pipe ，用于异步事件和信号处理，当然信号也可以采用类似 eventfd 的方式处理，如果都使用 Pipe ，两者会复用。

EV_FREQUENT_CHECK; 检查是否合法
-->

如下简单介绍各种事件的使用方法。

### Timer Watcher

可以设置定时器的启动时间，以及循环的时间间隔。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "libev/ev.h"

ev_timer timeout_watcher;
ev_timer repeate_watcher;
ev_timer oneshot_watcher;

// another callback, this time for a time-out
static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("timeout at %ju\n", (uintmax_t)time(NULL));

        /* this causes the innermost ev_run to stop iterating */
        ev_break (EV_A_ EVBREAK_ONE);
}
static void repeate_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("repeate at %ju\n", (uintmax_t)time(NULL));
}
static void oneshot_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("oneshot at %ju\n", (uintmax_t)time(NULL));
        ev_timer_stop(EV_A_ w);
}

int main (void)
{
        time_t result;
        EV_DEFAULT_DEC; /* OR ev_default_loop(0) */

        result = time(NULL);
        printf("  start at %ju\n", (uintmax_t)result);

        /* run only once in 2s later */
        ev_timer_init(&oneshot_watcher, oneshot_cb, 2.0, 0.);
        ev_timer_start(EV_A_ &oneshot_watcher);

        /* run in 5 seconds later, and repeat every second */
        ev_timer_init(&repeate_watcher, repeate_cb, 5., 1.);
        ev_timer_start(EV_A_ &repeate_watcher);

        /* timeout in 10s later, and also quit. */
        ev_timer_init(&timeout_watcher, timeout_cb, 10., 0.);
        ev_timer_start(EV_A_ &timeout_watcher);

        /* now wait for events to arrive. */
        ev_run(EV_A_ 0);

        return 0;
}
{% endhighlight %}


### Signal Watcher

在收到 `SIGINT` 时做些清理，直接退出。

{% highlight c %}
#include <ev.h>
#include <stdio.h>
#include <signal.h>

static void sigint_cb(EV_P_ ev_signal *w, int revents)
{
        (void) revents;

	printf("catch SIGINT, signal number %d.\n", w->signum);
        ev_break(EV_A_ EVBREAK_ALL);
}

int main (void)
{
        ev_signal wsig;

        // use the default event loop unless you have special needs
        EV_DEFAULT_DEC; /* OR ev_default_loop(0) */

        ev_signal_init(&wsig, sigint_cb, SIGINT);
        ev_signal_start(EV_A_ &wsig);

        ev_run(EV_A_ 0); // now wait for events to arrive

        return 0;
}
{% endhighlight %}

### Child Watcher

fork 一个新进程，给它安装一个 child 处理器等待进程结束，实际上会等待接受 `SIGCHLD` 信号，然后调用相应的事件。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "libev/ev.h"

static void child_cb (EV_P_ ev_child *w, int revents)
{
	(void) revents;
	ev_child_stop(EV_A_ w);
	printf ("process %d exited with status %x\n", w->rpid, w->rstatus);
}

int main (void)
{
	/* use the default event loop unless you have special needs */
        EV_DEFAULT_DEC; /* OR ev_default_loop(0) */
	ev_child cw;

	pid_t pid = fork();
	if (pid < 0) {  /* error */
		perror("fork()");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {  /* child, the forked child executes here */
		sleep(1);
		exit(EXIT_SUCCESS);
	}

	printf("parent %d child %d forked.\n", getpid(), pid);

	/* parent */
	ev_child_init(&cw, child_cb, pid, 0);
	ev_child_start(EV_A_ &cw);

	/* now wait for events to arrive */
        ev_run(EV_A_ 0);

	return 0;
}
{% endhighlight %}

在 libev 中，实际上也是通过注册一个 `SIGCHILD` 信号进行处理的，其回调函数是 `childcb` 。

### Fork Watcher

在 libev 中提供了一个 fork 事件的监控，libev 会在循环中自动检测是否调用了 `fork()` 函数，如果是那么会重新设置事件驱动回调函数。

注意，默认不会自动检测，需要设置相关的参数，例如 `ev_default_loop(EVFLAG_FORKCHECK)`，这样才会在每次循环的时候检测。

除了自动判断，也可以在 `fork()` 子进程之后调用 `ev_loop_fork()` 函数。

{% highlight c %}
#include <ev.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

static void fork_callback(EV_P_ ev_fork *w, int revents)
{
        (void) w;
        (void) revents;

        printf("[%d] fork callback\n", getpid());
}

static void timeout_callback(EV_P_ ev_timer *w,int revents)
{
        (void) w;
        (void) revents;

        printf("[%d] time out\n", getpid());
}

int main(void)
{
	struct ev_loop *loop;
        ev_fork wfork;
        ev_timer wtimer;

	loop = ev_default_loop(EVFLAG_FORKCHECK);

        ev_fork_init(&wfork, fork_callback);
        ev_fork_start(EV_A_ &wfork);

        ev_timer_init(&wtimer, timeout_callback, 1., 1.);
        ev_timer_start(EV_A_ &wtimer);

        pid_t pid;

        pid = fork();
        if (pid < 0) {
                return -1;
        } else if (pid == 0) {
                printf("[%d] Child\n", getpid());
                //ev_loop_fork(EV_A);
                ev_run(EV_A_ 0);
                ev_loop_destroy(EV_A);
                return 0;
        }

        printf("[%d] Parent\n", getpid());

        ev_run(EV_A_ 0);
        ev_loop_destroy(EV_A);

        return 0;
}
{% endhighlight %}

在如上的示例中，使用的是多实例模式，会在子进程中重新执行，所以最好的方式是，如果不需要最好直接关闭。

另外，在创建 epoll 对象时，入参使用了 `EPOLL_CLOEXEC` 参数，也就意味着在 fork 进程时会自动关闭文件描述符。

### Async Watcher

通常用于多个线程之间的事件同步，该事件允许在不同的线程中发送事件消息，内部使用 PIPE 进行通讯。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <ev.h>

#define log_info(fmt, args...) printf("%ju lwpid(%lu) " fmt, time(NULL), syscall(SYS_gettid), ##args)

struct ev_loop *work_loop = NULL;
static struct ev_async wasync;

static void async_cb(EV_P_ struct ev_async *w, int revents)
{
	(void) w;
        log_info("async hook call, event %d loop %p.\n", revents, loop);
}

void *ev_create(void *p)
{
	(void) p;

        log_info("worker thread start!\n");

	sleep(3);
        ev_async_init(&wasync, async_cb);
        ev_async_start(work_loop, &wasync);
        ev_run(work_loop, 0);

	return NULL;
}

int main(void)
{
	int num = 0;
        pthread_t tid;

        work_loop = ev_loop_new(EVFLAG_AUTO);

        pthread_create(&tid, NULL, ev_create, NULL);
	log_info("main thread start!\n");

        while(1) {
		log_info("send async #%d times.\n", num);
                ev_async_send(work_loop, &wasync);
                sleep(1);
                num++;
        }

        return 0;
}
{% endhighlight %}

如上，实际上启动顺序是不影响的，每次起一个线程都会建立一个 PIPE 管道。

### 多实例

也就是在多线程中，初始化不同的实例。

{% highlight c %}
#include <ev.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define log_info(fmt, args...) printf("%ju lwpid(%lu) " fmt, time(NULL), syscall(SYS_gettid), ##args)

static void repeate_hook(EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        (void) loop;

        log_info("repeate\n");
}

void *child1(void *arg)
{
        (void) arg;
        EV_P = ev_loop_new(0);

        ev_timer wtimer;

        log_info("child thread started.\n");

        ev_timer_init(&wtimer, repeate_hook, 0., 1.);
        ev_timer_start(EV_A_ &wtimer);

        ev_run(EV_A_ 0);

        return NULL;
}

int main (void)
{
        EV_DEFAULT_DEC; /* default */
        ev_timer wtimer;
        pthread_t tid1;

        pthread_create(&tid1, NULL, child1, NULL);

        ev_timer_init(&wtimer, repeate_hook, 0., 1.);
        ev_timer_start(EV_A_ &wtimer);

        /* now wait for events to arrive. */
        ev_run(EV_A_ 0);

        pthread_join(tid1, NULL);

        return 0;
}
{% endhighlight %}

注意，全局只能有一个默认的 `struct ev_loop` ，在线程中，需要通过 `ev_loop_new()` 再新建一个。

{% highlight text %}
{% endhighlight %}
