---
title: Linux 信号 VS. 线程
layout: post
comments: true
language: chinese
category: [misc]
keywords: signal,thread,linux
description: 在开发多线程应用时，考虑到线程安全，一般会通过 `pthread_mutex()` 去保护全局变量。如果应用中使用了信号，信号在被处理后应用程序还将正常运行，那么此时就需要正确的处理。一般来说，需要在指定的线程中以同步的方式处理，以避免由于处理异步信号而给程序运行带来的不确定性风险。
---

在开发多线程应用时，考虑到线程安全，一般会通过 `pthread_mutex()` 去保护全局变量。如果应用中使用了信号，信号在被处理后应用程序还将正常运行，那么此时就需要正确的处理。

一般来说，需要在指定的线程中以同步的方式处理，以避免由于处理异步信号而给程序运行带来的不确定性风险。

<!-- more -->

## 简介

在 Linux 环境中，每个进程都有自己的 Signal Mask，以及对应的 Signal Action ，这个行为集合决定了进程该如何处理信号。那么对于多线程来说：

* 信号发生时，哪个线程会收到信号？
* 每个线程是否都有自己的 mask 及 action ？
* 每个线程能按自己的方式处理信号么？

### 信号如何接收

这要是情况而定，场景如下：

* 如果是异常信号 (例如 SIGPIPE、SIGEGV 等)，则只有产生异常的线程收到并处理；
* 如果是用 `pthread_kill()` 产生的内部信号，则只有 `pthread_kill()` 参数中指定的目标线程收到并处理；
* 如果是外部使用 `kill` 命令产生信号 (如 SIGINT、SIGHUP 等)，则会遍历所有线程，直到找到一个不阻塞该信号的线程来处理 (一般是从主线程找起，而且只有一个线程能收到信号) 。
* 可以向指定的线程发送信号，该信号会被对应的线程处理，前提是该线程未阻塞对应的信号，如果阻塞则选择下一个未阻塞的线程。

可以使用 kill + 线程 ID 向指定的线程发送信号，这也就意味着，如果线程没有阻塞改信号，那么就可以正常处理。

### 是否有独立 mask 和 action

子线程的 mask 是会从主线程继承而来，每个线程都有自己独立的 Signal Mask，但所有线程共享进程的 Signal Action 。

这意味着，可以在线程中调用 `pthread_sigmask()` 来决定本线程阻塞哪些信号；但你不能调用 `sigaction()` 来指定单个线程的信号处理方式。

如果在某个线程中调用了 `sigaction()` 处理某个信号，那么这个进程中的未阻塞这个信号的线程在收到这个信号都会按同一种方式处理这个信号。

## 示例

如果有多个线程为阻塞信号，那么我们无法确定那个线程会接收信号，但是可以默认将所有线程屏蔽，然后选择某个线程 (例如主进程) 做信号处理。

这里重点测试下外部发送 kill 信号到进程的场景。

### 1. 信号屏蔽

在 Linux 多线程应用中，可以通过 `pthread_sigmask()` 设置本线程的信号掩码，除了 SIGSEGV SIGKILL SIGSTOP 无法被阻塞外都可以阻塞；当接着调用 `pthread_create()` 创建新线程时，此线程的信号掩码会被新创建的线程继承。

可通过 `sigprocmask()` 或者 `pthread_sigmask()` 屏蔽信号，如果是线程之前屏蔽，那么新创建的线程会继承屏蔽后的信号；如果是创建之后设置，那么只会影响到新的线程。

1. 不屏蔽信号，此时主线程会接收到信号 (一般进程接收到信号后发送到主线程)，打印日志信息，但是不会直接退出，需要等待线程休眠 60s 之后；
2. 通过 `pthread_sigmask()` 屏蔽信号，此时主线程信号被屏蔽，交由其它线程处理，在 `sleep()` 时被中断而直接退出。

示例代码如下。

{% highlight c %}
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define gettid()    syscall(__NR_gettid)

#define log_info(fmt, args...)  do {                                    \
	printf("[%ld] %ld info : " fmt, gettid(), time(NULL), ## args); \
} while(0)
#define log_error(fmt, args...) do {                                    \
	printf("[%ld] %ld error: " fmt, gettid(), time(NULL), ## args); \
} while(0)

#define THD_NUMS   10

void sighandler(int signo)
{
        log_info("Thread %lu received signo %d.\n", gettid(), signo);
}

void *thr1_fn(void *arg)
{
        (void) arg;

        log_info("Worker thread started.\n");
        while (1)
                sleep(1);

        return NULL;
}

int main(void)
{
        int i, rc;
        sigset_t bset;
        struct sigaction action;
        pthread_t thds[THD_NUMS];

        sigemptyset(&bset);
        sigaddset(&bset, SIGINT);
#if 1
        if (pthread_sigmask(SIG_BLOCK, &bset, NULL) != 0) {
                log_error("Set pthread mask failed.");
                exit(1);
        }
#else
        if (sigprocmask(SIG_BLOCK, &bset, NULL) != 0) {
                log_error("Set process mask failed.");
                exit(1);
        }
#endif

        /* register but still blocked now */
        action.sa_flags = 0;
        action.sa_handler = sighandler;
        sigaction(SIGINT, &action, NULL);

        log_info("Main thread started.\n");

        for (i = 0; i < THD_NUMS; i++) {
                rc = pthread_create(&thds[i], NULL, thr1_fn, NULL);
                if (rc != 0) {
                        log_error("Create thread failed, %s.\n", strerror(rc));
                        exit(1);
                }
        }

        /* only accpet SIGINT in main thread */
        if (pthread_sigmask(SIG_UNBLOCK, &bset, NULL) != 0) {
                log_error("Set pthread mask failed.");
                exit(1);
        }

        for (i = 0; i < THD_NUMS; i++)
                pthread_join(thds[i], NULL);

        return 0;
}
{% endhighlight %}

<!--
rlen = recv(sock_fd, buf, len, MSG_WAITALL);
if ((rlen == -1) && (errno == EINTR)){
    // this kind of error is recoverable, we can set the offset change
    //‘rlen’ as 0 and continue to recv
}
-->



## 最佳实践

在 `POSIX.1` 规范定义了 `sigwait()` `sigwaitinfo()` 和 `pthread_sigmask()` 等接口，可以实现：A) 以同步的方式处理异步信号；B) 在指定线程中处理信号。

进程中，可以通过 `kill(getpid(), signo)` 将信号发送到进程，而线程中则可以通过调用 `pthread_kill(pthread_t thread, int sig)` 将信号发送给指定的线程，则线号处理函数会在此指定线程的上下文背景中执行。

### SigWait

`sigwait()` 提供了一种等待指定信号到来，并以串行的方式从信号队列中取出信号进行处理的机制，如果新产生的信号不在指定的信号集内，则 `sigwait()` 继续等待。

其中的测试程序如下，用来测试 sigwait 在信号处理时的一些规则。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

void* sigmgr_thread()
{
	int sig;
	sigset_t waitset;
	pthread_t ppid = pthread_self();

	pthread_detach(ppid);

	sigemptyset(&waitset);
	sigaddset(&waitset, SIGRTMIN);
	sigaddset(&waitset, SIGRTMIN + 2);
	sigaddset(&waitset, SIGRTMAX);
	sigaddset(&waitset, SIGUSR1);
	sigaddset(&waitset, SIGUSR2);

	while (1)  {
		if (sigwait(&waitset, &sig) < 0) {
			fprintf(stderr, "sigwaitinfo() failed, %s\n", strerror(errno));
			continue;
		}
		fprintf(stdout, "Receive signal. %d\n", sig);
	}
}


int main()
{
	sigset_t bset, oset;
	pid_t pid = getpid();
	pthread_t ppid;

	sigemptyset(&bset);
	sigaddset(&bset, SIGRTMIN);
	sigaddset(&bset, SIGRTMIN + 2);
	sigaddset(&bset, SIGRTMAX);
	sigaddset(&bset, SIGUSR1);
	sigaddset(&bset, SIGUSR2);

	if (pthread_sigmask(SIG_BLOCK, &bset, &oset) != 0)
		fprintf(stderr, "Set pthread mask failed\n");

	kill(pid, SIGRTMAX);
	kill(pid, SIGRTMAX);
	kill(pid, SIGRTMIN + 2);
	kill(pid, SIGRTMIN);
	kill(pid, SIGRTMIN + 2);
	kill(pid, SIGRTMIN);
	kill(pid, SIGUSR2);
	kill(pid, SIGUSR2);
	kill(pid, SIGUSR1);
	kill(pid, SIGUSR1);

	pthread_create(&ppid, NULL, sigmgr_thread, NULL);

	sleep(1);

	return 0;
}
{% endhighlight %}

可以得到如下的结论。

* 对于非实时信号，相同信号不能在信号队列中排队；对于实时信号，相同信号可以在信号队列中排队。
* 如果信号队列中有多个实时以及非实时信号排队，实时信号并不会先于非实时信号被取出，信号数字小的会先被取出。

如 `SIGUSR1(10)` 会先于 `SIGUSR2(12)`，`SIGRTMIN(34)` 会先于 `SIGRTMAX(64)`， 非实时信号因为其信号数字小而先于实时信号被取出。

另外，`sigwaitinfo()` 以及 `sigtimedwait()` 也提供了与 `sigwait()` 函数相似的功能。




## 参考

<!--
线程与信号的使用经典案例
https://www.ibm.com/developerworks/cn/linux/l-cn-signalsec/index.html




libev的signal处理
https://blog.csdn.net/gqtcgq/article/details/49688027
内核处理信号的相关介绍
http://www.cnblogs.com/mickole/articles/3189764.html

一般来说，在 Linux 中的 `pthread_t` 类型是通过 `typedef unsigned long int pthread_t` 重定义的，忘了在哪里看到的

在如下的测试用例中。

while (1) {
	kill(pid, SIGTERM);
	sleep(10);
}

假设在 `sleep(10)` 时收到了外部信号，那么会直接退出 `sleep()` 函数，并返回剩余的睡眠秒数。

如果在主进程中注册了信号处理函数，那么可以通过 `kill(pid, SIGINT)` 发送信号给主进程，通过 `pthread_kill(tid, SIGINT)` 发送给其它线程。


#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define THDNUM_WORKER 5

void *worker_thread(void *args)
{
        (void) args;
        pthread_t tid = pthread_self();

        pthread_detach(tid);
        while (1) {
                //printf("#%ld Worker thread working.\n", syscall(__NR_gettid));
                sleep(10);
        }
}

#if 0
void *sigmgr_thread(void *args)
{
        (void) args;
        int rc;
        siginfo_t info;
        sigset_t waitset;
        pthread_t tid = pthread_self();

        rc = pthread_detach(tid);
        if (rc)
                fprintf(stderr, "detach manager thread failed, %s.\n",
                                strerror(rc));
        sigemptyset(&waitset);
        sigaddset(&waitset, SIGRTMIN);
        sigaddset(&waitset, SIGUSR1);

        fprintf(stdout, "[info] start signal manager thread 0x%lx.\n", tid);
        while (1)  {
                rc = sigwaitinfo(&waitset, &info);
                if (rc < 0) {
                        fprintf(stderr, "fetch the signal failed, %s.\n", strerror(errno));
                        continue;
                }
                fprintf(stdout, "Manager thread got signal %d.\n", rc);
                if (info.si_signo == SIGUSR1) {
                        printf("Manager thread 0x%lx, receive SIGUSR1.\n", tid);
                } else if (info.si_signo == SIGRTMIN) {
                        printf("Manager thread 0x%lx, receive SIGRTMIN.\n", tid);
                }
        }
}
#endif

void signal_handler(int signum)
{
        printf("#%ld Thread got a signal %d.\n", syscall(__NR_gettid), signum);
}

int main(void)
{
        int i;
        pid_t pid;
        pthread_t wtid[THDNUM_WORKER];

#if 0
        int rc;
        pthread_t ppid;
        sigset_t bset, oset;

        /*
         * Block SIGRTMIN and SIGUSR1 which will be handled in dedicated
         * thread sigmgr_thread(). Newly created threads will inherit the
         * pthread mask from its creator.
         */
        sigemptyset(&bset);
        sigaddset(&bset, SIGRTMIN);
        sigaddset(&bset, SIGUSR1);
        rc = pthread_sigmask(SIG_BLOCK, &bset, &oset);
        if (rc != 0)
                fprintf(stderr, "Set pthread mask failed, %s.\n", strerror(errno));

        /*
         * Create the dedicated thread sigmgr_thread() which will handle
         * SIGUSR1 and SIGRTMIN synchronously.
         */
        pthread_create(&ppid, NULL, sigmgr_thread, NULL);
#endif
        signal(SIGTERM, signal_handler);

        /* Create 5 worker threads */
        for (i = 0; i < (int)(sizeof(wtid) / sizeof(wtid[0])); i++)
                pthread_create(&wtid[i], NULL, worker_thread, NULL);

        printf("#%ld Main thread start.\n", syscall(__NR_gettid));
        /* send out 50 SIGUSR1 and SIGRTMIN signals. */
        pid = getpid();
        for (i = 0; i < 50; i++) {
                //kill(pid, SIGUSR1);
                //printf("#%d Main thread, send SIGUSR1, %d times.\n", pid, i);
                //kill(pid, SIGRTMIN);
                //kill(pid, SIGTERM);
                pthread_kill(wtid[0], SIGTERM);
                printf("#%d Main thread, send SIGRTMIN, %d times.\n", pid, i);
                sleep(10);
        }

        return 0;
}

为了提高服务器的处理性能，在网络编程时大部分都会采用非阻塞式的 Socket ，不同的接口修改的方式略有区别，这里简单介绍。

-->

{% highlight text %}
{% endhighlight %}
