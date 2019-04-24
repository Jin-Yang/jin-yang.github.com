---
title: Linux ptrace 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: ptrace,gdb,strace
description: ptrace() 是一个由 Linux 内核提供的系统调用，允许一个用户态进程检查、修改另一个进程的内存和寄存器，通常用在类似 gdb、strace 的调试器中，用来实现断点调试、系统调用的跟踪。 你想过怎么实现对系统调用的拦截吗？你尝试过通过改变系统调用的参数来愚弄你的系统 kernel 吗？你想过调试器是如何使运行中的进程暂停并且控制它吗？ 这里简单介绍如何使用该接口。
---

`ptrace()` 是一个由 Linux 内核提供的系统调用，允许一个用户态进程检查、修改另一个进程的内存和寄存器，通常用在类似 gdb、strace 的调试器中，用来实现断点调试、系统调用的跟踪。

你想过怎么实现对系统调用的拦截吗？你尝试过通过改变系统调用的参数来愚弄你的系统 kernel 吗？你想过调试器是如何使运行中的进程暂停并且控制它吗？

这里简单介绍如何使用该接口。

<!-- more -->

## 简介

在执行系统调用之前，内核会先检查当前进程是否处于被 "跟踪" traced 状态，如果是，内核暂停当前进程并将控制权交给跟踪进程，使跟踪进程得以察看或者修改被跟踪进程的寄存器。

其中函数的声明如下：

{% highlight text %}
#include <sys/ptrace.h>
long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);

ptrace有四个参数: 
1). enum __ptrace_request request：指示了ptrace要执行的命令。
2). pid_t pid: 指示ptrace要跟踪的进程。
3). void *addr: 指示要监控的内存地址。
4). void *data: 存放读取出的或者要写入的数据。
{% endhighlight %}

如下是一个示例，父进程会 fork 出了一个子进程，然后跟踪它。

{% highlight c %}


#include <stdlib.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>

#define log_info(...)  do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0);
#define log_error(...) do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);

int main(void)
{
        pid_t pid;
        int status, insyscall = 0;
        long orax, rax, params[3];
        struct user_regs_struct regs;

        pid = fork();
        if (pid < 0) {
                log_error("fork failed, %s.", strerror(errno));
                exit(EXIT_FAILURE); /* 1 */
        } else if (pid == 0){
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                execlp("/usr/bin/ls", "ls", NULL);
                exit(EXIT_SUCCESS);
        }
        log_info("current pid %d, child pid %d.", getpid(), pid);

        while (1) {
                wait(&status);
                //wait4(pid, &sta, 0, &ru);
                if (WIFEXITED(status)) {
                        log_info("child exited with %d.", status);
                        break;
                }
                orax = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, NULL);
                if (orax == SYS_write) {
                        if (insyscall == 0) {
                                insyscall = 1;
                                params[0] = ptrace(PTRACE_PEEKUSER, pid, 8 * RDI, NULL);
                                params[1] = ptrace(PTRACE_PEEKUSER, pid, 8 * RSI, NULL);
                                params[2] = ptrace(PTRACE_PEEKUSER, pid, 8 * RDX, NULL);
                                log_info("write called with %ld, %ld, %ld.",
                                                params[0], params[1], params[2]);

                                ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                                log_info("write called with %lld, %lld, %lld.",
                                                regs.rdi, regs.rsi, regs.rdx);
                        } else {
                                insyscall = 0;
                                rax = ptrace(PTRACE_PEEKUSER, pid, 8 * RAX, NULL);
                                log_info("write returned with %ld.", rax);
                        }
                }
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        }

        return 0;
}
{% endhighlight %}


#### 进程状态切换

在子进程中调用 `exec` 函数之前，首先通过 `PTRACE_TRACEME` 参数告知内核允许等待其它进程跟踪，那么在执行到 `execve()` 函数时会将控制权交换给父进程。

此时父进程在使用 `wait()` 函数来等待来自内核的通知，现在它得到了通知，那么接下看可以开始察看子进程都作了些什么，比如看看寄存器的值之类。

#### 查看状态

在进入系统调用时，内核会将 eax 设置为系统调用号，那么此时可以使用 `PTRACE_PEEKUSER` 获取对应的参数，然后再通过`PTRACE_CONT` 使子进程继续运行。

其中实际的系统调用号可以在头文件 `asm/unistd_64.h` 中查看；在使用 `PTRACE_PEEKUSER` 时入参通过 `ORIG_RAX` 查看，而返回值通过 `RAX` 查看。

`PTRACE_SYSCALL` 类似于 `PTRACE_CONT` ，不过在接下来系统调用的入参或者出参时仍然会阻塞。

示例 2 获取一个具体系统调用的参数，实际上也就是如何获取寄存器中的值，这里采用了两种方式，一种与上述获取系统调用号方式一致，另外一种是通过一次调用直接获取。

注意，不同的系统调用获取参数的方式也有所区别，详细可以查看 `man 2 syscall` 中的 `Architecture calling conventions` 内容。

例如，对于 `x86_64` 中的 `write()` 来说，其声明如下：

{% highlight c %}
ssize_t write(int fd, const void *buf, size_t count);
{% endhighlight %}

那么对应的系统调用号为 `rax` ，三个入参 `fd` `buf` `count` 分别对应了寄存器 `rdi` `rsi` `rdx` 三个。


## 示例

{% highlight c %}
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>

#define log_info(...)  do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0);
#define log_error(...) do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);

void reverse(pid_t pid, long addr, int len)
{
        char *buff, tmp;
        long i, j, value, offset;
        union {
                long value;
                char chars[sizeof(long)];
        } data;

        buff = (char *)malloc(len + 1);
        if (buff == NULL)
                exit(1);

        for (i = 0, offset = 0; i < len / (long)sizeof(long); i++, offset += sizeof(long)) {
                value = ptrace(PTRACE_PEEKDATA, pid, addr + offset, NULL);
                memcpy(buff + offset, &value, sizeof(long));
        }
        if (len % sizeof(long)) {
                value = ptrace(PTRACE_PEEKDATA, pid, addr + offset, NULL);
                memcpy(buff + offset, &value, len % sizeof(long));
        }
        buff[len] = 0;

        for(i = 0, j = len - 2; i <= j; ++i, --j) {
                tmp = buff[i];
                buff[i] = buff[j];
                buff[j] = tmp;
        }

        for (i = 0, offset = 0; i < len / (long)sizeof(long); i++, offset += sizeof(long)) {
                memcpy(data.chars, buff + offset, sizeof(long));
                ptrace(PTRACE_POKEDATA, pid, addr + offset, data.value);
        }
        if (len % sizeof(long)) {
                memcpy(data.chars, buff + offset, sizeof(long));
                ptrace(PTRACE_POKEDATA, pid, addr + offset, data.value);
        }
        free(buff);
}
int main()
{
        pid_t pid;
        int status, insyscall = 0;
        long orax, rax, params[3];

        pid = fork();
        if (pid < 0) {
                log_error("fork failed, %s.", strerror(errno));
                exit(EXIT_FAILURE); /* 1 */
        } else if (pid == 0){
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                execlp("/usr/bin/ls", "ls", NULL);
                exit(EXIT_SUCCESS);
        }
        log_info("current pid %d, child pid %d.", getpid(), pid);

        while (1) {
                wait(&status);
                //wait4(pid, &sta, 0, &ru);
                if (WIFEXITED(status)) {
                        log_info("child exited with %d.", status);
                        break;
                }
                orax = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, NULL);
                if (orax == SYS_write) {
                        if (insyscall == 0) {
                                insyscall = 1;
                                params[0] = ptrace(PTRACE_PEEKUSER, pid, 8 * RDI, NULL); /* fd */
                                params[1] = ptrace(PTRACE_PEEKUSER, pid, 8 * RSI, NULL); /* buff */
                                params[2] = ptrace(PTRACE_PEEKUSER, pid, 8 * RDX, NULL); /* count */
                                log_info("write called with %ld, %ld, %ld.",
                                                params[0], params[1], params[2]);
                                reverse(pid, params[1], params[2]);
                        } else {
                                insyscall = 0;
                                rax = ptrace(PTRACE_PEEKUSER, pid, 8 * RAX, NULL);
                                log_info("write returned with %ld.", rax);
                        }
                }
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        }

        return 0;
}
{% endhighlight %}

<!--
ptrace参考
http://recursiveg.me/2014/05/programming-with-ptrace-part4/
http://www.kgdb.info/playing_with_ptrace_part_i/
gettimeofday() 的成本
https://russelltao.iteye.com/blog/1405353
-->


{% highlight text %}
{% endhighlight %}
