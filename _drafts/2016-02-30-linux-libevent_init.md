---
title: libevent
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql
description: 保存一下经常使用的经典 MySQL 资源。
---

libevent 是一个轻量级的开源的高性能的事件库，适用于 windows、linux、bsd 等多种平台，根据不同的平台，会选择使用 select、epoll、kqueue 等系统调用管理事件机制。

下面简单介绍下。

<!-- more -->

## 简介

很多程序在使用 [libevent](http://libevent.org/) 库，包括了 memcached、tmux、tor 等，主要有如下的特点：

* 事件驱动，高性能，轻量级；
* 开源，代码相当精炼、易读;
* 跨平台，支持 Windows、Linux、BSD 和 Mac OS；
* 支持多种 IO 多路复用技术，如 epoll、poll、select、kqueue 等，不同平台会选择不同函数；
* 支持 IO、定时器和信号等事件；
* 采用 Reactor 模式。

接下来，看看如何安装。

### 安装

在 CentOS 中，可以直接通过 yum 安装。

{% highlight text %}
----- 直接通过yum安装
# yum install libevent

----- 查看安装的库
$ rpm -ql libevent
/usr/lib64/libevent-2.0.so.5
/usr/lib64/libevent-2.0.so.5.1.9
/usr/lib64/libevent_core-2.0.so.5
/usr/lib64/libevent_core-2.0.so.5.1.9
/usr/lib64/libevent_extra-2.0.so.5
/usr/lib64/libevent_extra-2.0.so.5.1.9
/usr/lib64/libevent_openssl-2.0.so.5
/usr/lib64/libevent_openssl-2.0.so.5.1.9
/usr/lib64/libevent_pthreads-2.0.so.5
/usr/lib64/libevent_pthreads-2.0.so.5.1.9
{% endhighlight %}

可以看到，libevent 会安装如下的库：

* libevent_core<br>所有核心的事件和缓冲功能，包含了 event_base、evbuffer、bufferevent 以及工具函数；
* libevent_extra<br>包括了程序可能需要的协议特定功能，包括 HTTP、DNS 和 RPC；
* libevent<br>因为历史原因而存在，包含 libevent_core 和 libevent_extra 的内容，以后可能会去掉；
* libevent_pthreads<br>添加基于 pthread 可移植线程库的线程和锁定实现，独立于 core，这样使用 libevent 时就不需要链接到 pthread，除非是以多线程方式使用 libevent。


<!--
下载之后解压，然后进入目录就可以安装了。

[mjf@localhost libevent-2.0.22-stable]$ ./configure --prefix=/home/mjf/lib  (prefix是我配置目录，默认可以不加)
[mjf@localhost libevent-2.0.22-stable]$ make
[mjf@localhost libevent-2.0.22-stable]$ sudo make install

注：./configure --prefix=/home/mjf/lib 因为如果我不加prefix，后面执行示例程序的时候会如下错误：error while loading shared libraries: libevent-2.0.so.5: cannot open shared object file: No such file or directory， 所以我自己定制了路径，就没问题了，当然你的可能没问题，那就不用加，install之后直接在/usr/lib或者/usr/local/lib里面就能看到了libevent*.so了。


三、libevent的功能。
Libevent提供了事件通知，io缓存事件，定时器，超时，异步解析dns，事件驱动的http server以及一个rpc框架。

事件通知：当文件描述符可读可写时将执行回调函数。

Io缓存：缓存事件提供了输入输出缓存，能自动的读入和写入，用户不必直接操作io。

定时器：libevent提供了定时器的机制，能够在一定的时间间隔之后调用回调函数。

信号：触发信号，执行回调。

异步的dns解析：libevent提供了异步解析dns服务器的dns解析函数集。

事件驱动的http服务器：libevent提供了一个简单的，可集成到应用程序中的HTTP服务器。

RPC客户端服务器框架：libevent为创建RPC服务器和客户端创建了一个RPC框架，能自动的封装和解封数据结构。

四、Reactor(反应器)模式
libevent是一个典型的reactor模式的实现。这里做一下简单介绍：
我们知道，普通的函数调用机制如下：程序调用某个函数，函数执行，程序等待，函数将结果返回给调用程序（如果含有函数返回值的话），也就是顺序执行的。
而Reactor模式的基本流程如下：应用程序需要提供相应的接口并且注册到reactor反应器上，如果相应的事件发生的话，那么reactor将自动调用相应的注册的接口函数（类似于回调函数）通知你，所以libevent是事件触发的网络库。
-->

### 示例程序

如下是一个简单的程序，每隔 1 秒输出 ```"Hello World!"``` 。

{% highlight c %}
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <event.h>

void on_time(int sock, short event, void *arg)
{
    struct timeval tv;
    printf("Hello World!\n");

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    event_add((struct event*)arg, &tv);  // 重新添加定时事件，默认会自动删除
}

int main(void)
{
    struct event ev_time;
    struct timeval tv;

    event_init();                             // 初始化
    evtimer_set(&ev_time, on_time, &ev_time); // 设置定时事件

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    event_add(&ev_time, &tv);                 // 添加定时事件

    event_dispatch();                         // 事件循环

    return 0;
}
{% endhighlight %}

然后通过如下方式编译。

{% highlight text %}
$ gcc -levent example.c -o example
{% endhighlight %}

然后执行即可。

### 使用流程

如上，

基本应用场景也是使用
libevnet
的基本流程，下面来考虑一个最简单的场景，使用
livevent
设置定时器，应用程序只需要执行下面几个简单的步骤即可。
1
）首先初始化
libevent
库，并保存返回的指针
struct event_base * base = event_init();
实际上这一步相当于初始化一个
Reactor
实例；在初始化
libevent
后，就可以注册事件了。
2
）初始化事件
event
，设置回调函数和关注的事件
evtimer_set(&ev, timer_cb, NULL);
事实上这等价于调用
event_set(&ev, -1, 0, timer_cb, NULL);
event_set
的函数原型是：
void event_set(struct
event *ev,
int fd,
short event,
void (*cb)(int,
short, void *), void *arg)
ev
：执行要初始化的
event
对象；
fd
：该
event
绑定的“句柄”，对于信号事件，它就是关注的信号；
event
：在该
fd
上关注的事件类型，它可以是
EV_READ, EV_WRITE, EV_SIGNAL
；
cb
：这是一个函数指针，当
fd
上的事件
event
发生时，调用该函数执行处理，它有三个参数，
调用时由
event_base
负责传入，按顺序，实际上就是
event_set
时的
fd, event
和
arg
；
arg
：传递给
cb
函数指针的参数；
由于定时事件不需要
fd
，并且定时事件是根据添加时（
event_add
）的超时值设定的，因此
这里
event
也不需要设置。
这一步相当于初始化一个
event handler
，在
libevent
中事件类型保存在
event
结构体中。
注意：
libevent
并不会管理
event
事件集合，这需要应用程序自行管理；
3
）设置
event
从属的
event_base
event_base_set(base, &ev);
这一步相当于指明
event
要注册到哪个
event_base
实例上；
4
）是正式的添加事件的时候了
event_add(&ev, timeout);
基本信息都已设置完成，只要简单的调用
event_add()
函数即可完成，其中
timeout
是定时值；
10
这一步相当于调用
Reactor::register_handler()
函数注册事件。
5
）程序进入无限循环，等待就绪事件并执行事件处理
event_base_dispatch(base)


## 参考

可以参考官方网站 [libevent – an event notification library](http://libevent.org/)，以及 [github libevent](https://github.com/libevent/libevent)，以及 [libevent 源码深度剖析](/reference/linux/libevent.pdf) 。

{% highlight text %}
{% endhighlight %}
