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

### 是否有独立 mask 和 action

子线程的 mask 是会从主线程继承而来，每个线程都有自己独立的 Signal Mask，但所有线程共享进程的 Signal Action 。

这意味着，可以在线程中调用 `pthread_sigmask()` 来决定本线程阻塞哪些信号；但你不能调用 `sigaction()` 来指定单个线程的信号处理方式。

如果在某个线程中调用了 `sigaction()` 处理某个信号，那么这个进程中的未阻塞这个信号的线程在收到这个信号都会按同一种方式处理这个信号。

## 示例

这里重点测试下外部发送 kill 信号到进程的场景。

### 1. 信号屏蔽

在 Linux 多线程应用中，可以通过 `pthread_sigmask()` 设置本线程的信号掩码，除了 SIGSEGV SIGKILL SIGSTOP 无法被阻塞外都可以阻塞；当接着调用 `pthread_create()` 创建新线程时，此线程的信号掩码会被新创建的线程继承。

可通过 `sigprocmask()` 或者 `pthread_sigmask()` 屏蔽信号，如果是线程之前屏蔽，那么新创建的线程会继承屏蔽后的信号；如果是创建之后设置，那么只会影响到新的线程。

1. 不屏蔽信号，此时主线程会接收到信号 (一般进程接收到信号后发送到主线程)，打印日志信息，但是不会直接退出，需要等待线程休眠 60s 之后；
2. 通过 `pthread_sigmask()` 屏蔽信号，此时主线程信号被屏蔽，交由其它线程处理，在 `sleep()` 时被中断而直接退出。

示例代码如下。

{% highlight c %}
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define gettid()    syscall(__NR_gettid)

#define log_info(...)  do { printf(" info: " __VA_ARGS__); putchar('\n'); } while(0);
#define log_error(...) do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);

void sighandler(int signo)
{
        log_info("Thread %lu received signo %d.", gettid(), signo);
}

void *thr1_fn(void *arg)
{
        (void) arg;
        int tid = gettid();
        struct sigaction action;

        action.sa_flags = 0;
        action.sa_handler = sighandler;
        sigaction(SIGINT, &action, NULL);

        log_info("Thread %d started.", tid);
        if (sleep(60) != 0)
                log_info("Thread %d interrupted.", tid);
        log_info("Thread %d ends.", tid);

        return NULL;
}

int main(void)
{
        int rc;
        pthread_t t1;
        sigset_t bset;

        sigemptyset(&bset);
        sigaddset(&bset, SIGINT);

        log_info("Main thread pid %lu", gettid());

        rc = pthread_create(&t1, NULL, thr1_fn, NULL);
        if (rc != 0) {
                log_error("Create thread failed, %s.", strerror(rc));
                exit(1);
        }

#if 0
        if (pthread_sigmask(SIG_BLOCK, &bset, NULL) != 0) {
                log_error("Set pthread mask failed.");
                exit(1);
        }

        if (sigprocmask(SIG_BLOCK, &bset, NULL) != 0) {
                log_error("Set process mask failed.");
                exit(1);
        }
#endif
        pthread_join(t1, NULL);

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
-->

{% highlight text %}
{% endhighlight %}
