---
title: libev 信号处理
curtag: libev
layout: post
comments: true
language: chinese
tag: [libev]
category: [linux,program]
keywords: linux,program,libev,event loop,signal,信号安全
description:
---

信号的处理非常敏感，如果处理不当很容易出现问题，如果在信号处理函数中使用了非信号安全函数，那么很容易发生死锁。

这里简单介绍 libev 中是如何处理信号的。

<!-- more -->

## 简介

使用示例如下。

{% highlight c %}
#include <stdio.h>
#include <libev/ev.h>

static void sigint_cb (EV_P_ ev_signal *w, int revents)
{
        puts("catch SIGINT");
        ev_break (EV_A_ EVBREAK_ALL);
}

int main (void)
{
        EV_P EV_DEFAULT;
        static ev_signal signal_watcher;

        ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
        ev_signal_start(EV_A_ &signal_watcher);

        ev_loop(EV_A_ 0);

        return 0;
}
{% endhighlight %}

## 同步处理

Linux 中的信号时异步发生的，一般是从内核态切换到用户态时进行检查，从而从用户代码角度看，就是异步处理。

而 libev 会将异步信号转换为同步。

常用的同步化方案有 `signalfd`、`pipe`、`eventfd`、`sigwaitinfo` 等，在 libev 中采用的是前两种，将对异步信号的处理，转化成对文件描述符的处理，也就是将 `ev_signal` 转化为处理 `ev_io` ；而最后一种，需要单独起一个信号处理线程。

libev 中使用的是 `signalfd` 或者 `pipe` 的方式。

### signalfd

signalfd 是最简单方便的信号同步机制，很容易将异步信号转化成对文件描述符的监听。

下面首先看一下使用 signalfd 时的信号处理流程，其函数声明为。

{% highlight text %}
#include <sys/signalfd.h>
int signalfd(int fd, const sigset_t*mask, intflags);

参数:
    fd: -1 生成新文件描述符；或者指定存在有效的 fd ，而 mask 会替换掉之前相关联的信号集。
    mask: 这个文件描述符接受的信号集，可以通过sigsetops()宏初始化。
{% endhighlight %}

函数使用示例如下：

{% highlight c %}
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signalfd.h>

#define handle_error(msg) do {           \
        perror(msg); exit(EXIT_FAILURE); \
} while (0)

int main(void)
{
        int sfd;
        ssize_t rc;
        struct signalfd_siginfo fdsi;

        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGQUIT);

        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
                handle_error("sigprocmask");

        if ((sfd = signalfd(-1, &mask, 0)) == -1)
                handle_error("signalfd");

        while(1) {
                rc = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
                if (rc != sizeof(struct signalfd_siginfo))
                        handle_error("read");

                if (fdsi.ssi_signo == SIGINT) {
                        printf("Got SIGINT\n");
                } else if (fdsi.ssi_signo == SIGQUIT) {
                        printf("Got SIGQUIT\n");
                        exit(EXIT_SUCCESS);
                } else {
                        printf("Read unexpected signal\n");
                }
        }

        return 0;
}
{% endhighlight %}



## 源码解析

对应的结构体展开后的成员对象如下：

{% highlight c %}
typedef struct ev_signal {
	int active;
	int pending;
	int priority;
	void *data;
	void (*cb)(EV_P_ struct ev_signal *w, int revents);
	struct ev_watcher_list *next;
	int signum;
} ev_signal;
{% endhighlight %}

包括 cb 在内之前的都是比较标准的成员，其中 signum 记录了信号量，成员结构体通过 list 链接。另外，在 ev.c 内部，通过 `ANSIG` 结构体维护了一个数组结构，用来组织 `ev_signal` 结构体。

{% highlight c %}
typedef struct {
    sig_atomic_t volatile pending;   // 信号处于未决状态，也就是触发但尚未处理
#if EV_MULTIPLICITY
    struct ev_loop *loop;
#endif
    ev_watcher_list *head;           // 该信号所注册的信号处理回调函数
} ANSIG;
static ANSIG signals [EV_NSIG - 1];
{% endhighlight %}

`signals` 是 ANSIG 类型数组，下标是 `信号值 - 1`，也就是说，每个信号都有对应的 ANSIG 结构。


## 参考

* [Linux 信号安全]({{ production_url }}/post/linux-signal-safe-introduce.html) 介绍信号安全相关的内容。

{% highlight text %}
{% endhighlight %}
