---
title: Linux C 编程杂项
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,aio
description:
---


<!-- more -->

## eventfd

eventfd 在内核版本，2.6.22 以后有效。

{% highlight text %}
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);

参数：
    initval 用于初始化计数器；
    flags   设置标志位，常见的有如下：
       EFD_NONBLOCK
           设置为非阻塞状态，否则在调用read()时，如果计数器值为0，则会一直堵塞。
       EFD_CLOEXEC
           我的理解是，这个标识被设置的话，调用exec后会自动关闭文件描述符，防止泄漏。
{% endhighlight %}

这个函数会创建一个事件对象 (eventfd object), 用来实现进程或者线程间的等待/通知 (wait/notify) 机制，内核会为这个对象维护一个 64 位的计数器，可以通过 `initval` 初始化。

<!--
write 将缓冲区写入的8字节整形值加到内核计数器上。

read 读取8字节值， 并把计数器重设为0. 如果调用read的时候计数器为0， 要是eventfd是阻塞的， read就一直阻塞在这里，否则就得到 一个EAGAIN错误。
如果buffer的长度小于8那么read会失败， 错误代码被设置成 EINVAL。
-->

## CLOEXEC

关于 `open()` 函数的 `O_CLOEXEC` 模式，以及 `fcntl()` 函数的 `FD_CLOEXEC` 选项，总结如下：

1. 调用 `open()` 时使用 `O_CLOEXEC` 模式打开的文件描述符，在执行 `exec()` 调用新程序中关闭，且为原子操作。

2. 调用 `fcntl()` 设置 `open()` 打开的文件描述符为 `FD_CLOEXEC` 选项时，其效果和在 `open()` 时使用 `O_CLOEXEC` 选项相同，不过不再是原子操作，可能存在竞态条件。

3. 通过 `fork()` 调用产生的子进程中不被关闭。

4. 调用 `dup` 族类函数得到的新文件描述符将清除 `O_CLOEXEC` 模式。

<!--
测试程序如下：

{% highlight text %}
    #include <unistd.h>
    #include <sys/types.h>
    #include <fcntl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <errno.h>

    #define err_sys(fmt, arg...) \
    do { \
        printf(fmt, ##arg);\
        printf("\nerrno:%d %s\n", errno, strerror(errno));\
        exit(EXIT_FAILURE);\
    } while (0)


int main()
{
        int fd, fd2, val;
        pid_t pid;

    #ifdef _O_CLOEXEC
        if ((fd = open("my.txt", O_RDWR | O_CREAT | O_CLOEXEC, 0600)) < 0)
    #else
        if ((fd = open("my.txt", O_RDWR | O_CREAT, 0600)) < 0)
    #endif
            err_sys("open error");

    #ifdef _DUP
        if ((fd2 = dup(fd)) < 0)
            err_sys("dup error");
        if (write(fd2, "123", 3) < 0)
            err_sys("write error");
    #endif

    #ifdef _FCNTL_CLOEXEC
        if ((val = fcntl(fd, F_GETFD)) < 0)
            err_sys("fcntl(F_GETFD) error");

        val |= FD_CLOEXEC;
        if (fcntl(fd, F_SETFD, val) < 0)
            err_sys("fcntl( F_SETFD) error");
    #endif

    #ifndef _FORK
        if (execl("/bin/sleep", "sleep", "10000", (void*)0) < 0)
            err_sys("execl error");
    #else
     switch ((pid = fork())) {
            case -1:
                err_sys("fork error");
            case 0:
                sleep(10000);
                break;
            default:
                sleep(10000);
                break;
        }
    #endif

        return 0;
    }
{% endhighlight %}

通过宏_O_CLOEXEC编译进O_CLOEXEC选项
#gcc -D_O_CLOEXEC -o cloexec cloexec.c
执行程序
#./cloexec
查看进程和进程资源，注意执行execl后进程名字变为sleep了
#ps aux|grep sleep
root      7900  0.0  0.0   4200   280 pts/0    S+   11:45   0:00 sleep 10000
root      7915  0.0  0.1   4384   836 pts/1    S+   11:49   0:00 grep --color=auto sleep

#lsof -p 7900
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF   NODE NAME
sleep   7900 root  cwd    DIR    8,1     4096 163741 /home/zozy/AUP
sleep   7900 root  rtd    DIR    8,1     4096      2 /
sleep   7900 root  txt    REG    8,1    26156 131621 /bin/sleep
sleep   7900 root  mem    REG    8,1  2919792     12 /usr/lib/locale/locale-archive
sleep   7900 root  mem    REG    8,1  1730024  15701 /lib/i386-linux-gnu/libc-2.15.so
sleep   7900 root  mem    REG    8,1   134344  15713 /lib/i386-linux-gnu/ld-2.15.so
sleep   7900 root    0u   CHR  136,0      0t0      3 /dev/pts/0
sleep   7900 root    1u   CHR  136,0      0t0      3 /dev/pts/0
sleep   7900 root    2u   CHR  136,0      0t0      3 /dev/pts/0

可以看出进程资源中没有文件my.txt


不带宏_O_CLOEXEC编译
#gcc  -o nocloexec cloexec.c
执行程序
#./nocloexec

查看进程和进程资源
#ps aux|grep sleep
root      7925  0.0  0.0   4200   280 pts/0    S+   11:51   0:00 sleep 10000
root      7928  0.0  0.1   4384   836 pts/1    S+   11:52   0:00 grep --color=auto sleep

#lsof -p 7925
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF   NODE NAME
sleep   7925 root  cwd    DIR    8,1     4096 163741 /home/zozy/AUP
sleep   7925 root  rtd    DIR    8,1     4096      2 /
sleep   7925 root  txt    REG    8,1    26156 131621 /bin/sleep
sleep   7925 root  mem    REG    8,1  2919792     12 /usr/lib/locale/locale-archive
sleep   7925 root  mem    REG    8,1  1730024  15701 /lib/i386-linux-gnu/libc-2.15.so
sleep   7925 root  mem    REG    8,1   134344  15713 /lib/i386-linux-gnu/ld-2.15.so
sleep   7925 root    0u   CHR  136,0      0t0      3 /dev/pts/0
sleep   7925 root    1u   CHR  136,0      0t0      3 /dev/pts/0
sleep   7925 root    2u   CHR  136,0      0t0      3 /dev/pts/0
sleep   7925 root    3u   REG    8,1        0 163759 /home/zozy/AUP/my.txt

可以看出进程资源中有文件my.txt

测试fcntl函数可以通过设置编译宏-D_FCNTL_CLOEXEC测试，编译测试过程同上，同理通过开启-D_O_CLOEXEC -D_FORK编译选项测试使用O_CLOEXEC模式的描述符在子进程中的状态，通过宏-D_DUP编译选项测试dup函数对O_CLOEXEC的影响，编译测试过程略。
-->









{% highlight text %}
{% endhighlight %}
