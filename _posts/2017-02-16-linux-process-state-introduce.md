---
title: Linux 进程状态
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,process,state
description: Linux 中的进程在不同的阶段会通过其状态显示，一般来说会有 7 种，相当于一个状态机运行。这里简单介绍，以及一些常见的特殊状态。
---

Linux 中的进程在不同的阶段会通过其状态显示，一般来说会有 7 种，相当于一个状态机运行。

这里简单介绍，以及一些常见的特殊状态。

<!-- more -->

## 进程状态

Linux是一个多用户，多任务的系统，可以同时运行多个用户的多个程序，就必然会产生很多的进程，而每个进程会有不同的状态，可以参考 `task_state_array[]@fs/proc/array.c` 中的内容。

{% highlight text %}
static const char * const task_state_array[] = {
    "R (running)",      /*   0 */
    "S (sleeping)",     /*   1 */
    "D (disk sleep)",   /*   2 */
    "T (stopped)",      /*   4 */
    "t (tracing stop)", /*   8 */
    "X (dead)",         /*  16 */
    "Z (zombie)",       /*  32 */
};
{% endhighlight %}

其中进程的转换过程如下所示。

![process]({{ site.url }}/images/linux/process-status-transform.gif "process"){: .pull-center width="70%" }

##### R (TASK_RUNNING，可执行状态)

只有在该状态的进程才可能在 CPU 上运行，这些进程的 `struct task_struct` 会被放入对应 CPU 的可执行队列中，进程调度器会从可执行队列中分别选择一个进程在该 CPU 上运行。

##### S (TASK_INTERRUPTIBLE，可中断的睡眠状态)

在等待某事件的发生 (例如 socket 连接、信号量等) 而被挂起，对应的 TCB 被放入对应事件的等待队列中，当事件发生时 (由外部中断触发或由其它进程触发)，对应的等待队列中的一个或多个进程将被唤醒。

##### D (TASK_UNINTERRUPTIBLE，不可中断的睡眠状态)

进程同样处于睡眠状态，但该进程是不可中断的，也就是进程不响应异步信号，即使使用 `kill -9` 信号。

##### T (TASK_STOPPED or TASK_TRACED，暂停状态或跟踪状态)

向进程发送一个 SIGSTOP 信号，它就会因响应该信号而进入该状态，其中 SIGSTOP 与 SIGKILL 信号一样，是强制的，不允许用户进程通过 signal 系列的系统调用重新设置对应的信号处理函数。

<!--
有一类垃圾却并非这么容易打扫，那就是我们常见的状态为 D (Uninterruptible sleep) ，以及状态为 Z (Zombie) 的垃圾进程。这些垃圾进程要么是求而不得，像怨妇一般等待资源(D)，要么是僵而不死，像冤魂一样等待超度(Z)，它们在 CPU run_queue 里滞留不去，把 Load Average 弄的老高老高，没看过我前一篇blog的国际友人还以为这儿民怨沸腾又出了什么大事呢。怎么办？开枪！kill -9！看你们走是不走。但这两种垃圾进程偏偏是刀枪不入的，不管换哪种枪法都杀不掉它们。无奈，只好reboot，像剿灭禽流感那样不分青红皂白地一律扑杀！

贫僧还是回来说正题。怨妇 D，往往是由于 I/O 资源得不到满足，而引发等待，在内核源码 fs/proc/array.c 里，其文字定义为“ "D (disk sleep)", /* 2 */ ”（由此可知 D 原是Disk的打头字母），对应着 include/linux/sched.h 里的“ #define TASK_UNINTERRUPTIBLE 2 ”。举个例子，当 NFS 服务端关闭之时，若未事先 umount 相关目录，在 NFS 客户端执行 df 就会挂住整个登录会话，按 Ctrl+C 、Ctrl+Z 都无济于事。断开连接再登录，执行 ps axf 则看到刚才的 df 进程状态位已变成了 D ，kill -9 无法杀灭。正确的处理方式，是马上恢复 NFS 服务端，再度提供服务，刚才挂起的 df 进程发现了其苦苦等待的资源，便完成任务，自动消亡。若 NFS 服务端无法恢复服务，在 reboot 之前也应将 /etc/mtab 里的相关 NFS mount 项删除，以免 reboot 过程例行调用 netfs stop 时再次发生等待资源，导致系统重启过程挂起。

D是处于TASK_UNINTERRUPTIBLE的进程，深度睡眠，不响应信号。 一般只有等待非常关键的事件时，才把进程设为这个状态。

　　冤魂 Z 之所以杀不死，是因为它已经死了，否则怎么叫 Zombie（僵尸）呢？冤魂不散，自然是生前有结未解之故。在UNIX/Linux中，每个进程都有一个父进程，进程号叫PID（Process ID），相应地，父进程号就叫PPID（Parent PID）。当进程死亡时，它会自动关闭已打开的文件，舍弃已占用的内存、交换空间等等系统资源，然后向其父进程返回一个退出状态值，报告死讯。如果程序有 bug，就会在这最后一步出问题。儿子说我死了，老子却没听见，没有及时收棺入殓，儿子便成了僵尸。在UNIX/Linux中消灭僵尸的手段比较残忍，执行 ps axjf 找出僵尸进程的父进程号（PPID，第一列），先杀其父，然后再由进程天子 init（其PID为1，PPID为0）来一起收拾父子僵尸，超度亡魂，往生极乐。注意，子进程变成僵尸只是碍眼而已，并不碍事，如果僵尸的父进程当前有要务在身，则千万不可贸然杀之。
注意：不是所有状态为Z的进程都是无法收拾的，很可能是那个短暂的状态刚好被你发现了。
-->

## 特殊状态

子进程是通过父进程创建的，子进程的结束和父进程的运行是一个异步过程，父进程永远无法预测子进程到底什么时候结束。当一个进程完成它的工作终止之后，它的父进程需要调用 `wait()` 或者 `waitpid()` 取得子进程的终止状态。

当父子进程在不同时间点退出时，那么就可能会进入到异常状态。

#### 孤儿进程

一个父进程退出，相应的一个或多个子进程还在运行，那么那些子进程将成为孤儿进程。

孤儿进程将被 `init` 进程所收养，并由 `init` 进程收集它们的完成状态，也就是说，孤儿进程没有危害，最终仍然回被 `init` 回收。

#### 僵尸进程

一个进程使用 `fork` 创建子进程，如果子进程退出后父进程没有调用 `wait` 或 `waitpid` 获取子进程的状态信息，那么子进程的进程描述符仍然保存在系统中，仍然占用进程表，显示为 `defunct` 状态。

可以通过重启或者杀死父进程解决。

### 示例

在 Linux 中，进程退出时，内核释放该进程所有的部分资源，包括打开的文件、占用的内存等。但仍为其保留一定的信息，包括进程号 PID、退出的状态、运行时间等，直到父进程通过 `wait()` 或 `waitpid()` 来获取时才释放。

如果父进程一直存在，那么该进程的进程号就会一直被占用，而系统所能使用的进程号是有限的，如果大量的产生僵死进程，将因为没有可用的进程号而导致系统不能产生新的进程。

如下是两个示例，分别为孤儿进程和僵尸进程。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    pid_t pid = fork();
    if (fpid &lt; 0) {
        printf("error in fork!");
        exit(1);
    }
    if (pid == 0) { // child process.
        printf("child process create, pid: %d\tppid:%d\n",getpid(),getppid());
        sleep(5);   // sleep for 5s until father process exit.
        printf("child process exit, pid: %d\tppid:%d\n",getpid(),getppid());
    } else {
        printf("father process create\n");
        sleep(1);
        printf("father process exit\n");
    }
    return 0;
}


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//static void sig_child(int signo)
//{
//     pid_t        pid;
//     int        stat;
//     while ((pid = waitpid(-1, &stat, WNOHANG)) &gt; 0)
//            printf("child %d terminated.\n", pid);
//}
int main ()
{
    pid_t fpid;
    // signal(SIGCHLD, sig_child);
    fpid = fork();
    if (fpid &lt; 0) {
        printf("error in fork!");
        exit(1);
    }
    if (fpid == 0) {
        printf("child process(%d)\n", getpid());
        exit(0);
    }
    printf("father process\n");
    sleep(2);
    system("ps -o pid,ppid,state,tty,command | grep defunct | grep -v grep");
    return 0;
}
{% endhighlight %}

第一个是孤儿进程，第二次输出时其父进程 PID 变成了 `init(PID=1)`；第二个是僵尸进程，进程退出时会产生 `SIGCHLD` 信号，父进程可以通过捕获该信号进行处理。


### TASK_STOPPED

`TASK_STOPPED`，进程终止，通常是由于向进程发送了 `SIGSTOP`、`SIGTSTP`、`SIGTTIN`、`SIGTTOU` 信号，此时可以通过 `kill -9(SIGKILL) pid` 尝试杀死进程，如果不起作用则 `kill -18 pid` ，也就是发个 `SIGCONT` 信号过去。

## 孤儿进程接管

如上所述，所谓的孤儿进程是指，当父进程被 `kill` 掉，其子进程就会成为孤儿进程 `Orphaned Process`，并被 `init(PID=1)` 所接管。

那么，孤儿进程如何被接管的？

在 Linux 内核中，有如下的代码 [Kernel find_new_reaper()](https://github.com/torvalds/linux/blob/eae21770b4fed5597623aad0d618190fa60426ff/kernel/exit.c#L479) ，其开头的注释摘抄如下：

{% highlight text %}
/*
 * When we die, we re-parent all our children, and try to:
 * 1. give them to another thread in our thread group, if such a member exists
 * 2. give it to the first ancestor process which prctl'd itself as a
 *    child_subreaper for its children (like a service manager)
 * 3. give it to the init process (PID 1) in our pid namespace
 */
{% endhighlight %}

也就是说，接管分三步：A) 找到相同线程组里其他可用的线程；B) 如果没有找到则进行第二步；C) 最后交由 `PID=1` 的进程管理。

### SubReaper

当一个进程被标记为 `SubReaper` 后，这个进程所创建的所有子进程，包括子进程的子进程，都将被标记拥有一个 `SubReaper` 。

当某个进程成为孤儿进程时，会沿着它的进程树向祖先进程找一个最近的是 `SubReaper` 且运行着的进程，这个进程将会接管这个孤儿进程。

<!--
http://adoyle.me/blog/orphaned-process-and-zombie-process-and-docker.html
-->

### tinit

其功能类似于 `init` 进程，实际上就是模拟 `init` 进程的僵尸进程回收，一般用于容器中，用于回收容器中退出的进程。

<!--
https://github.com/krallin/tini
https://github.com/Yelp/dumb-init

在编译静态二进制文件时会依赖 glibc 的静态库，对于 CentOS 来说，需要通过 `yum install glibc-static` 安装。

可以通过该工程查看 CMake 的编写，以及编写类似 init 进程的注意事项。
-->

## Uninterruptable

Linux 中有一个 `uninterruptable` 状态，此时的进程不接受任何的信号，包括了 `kill -9` ，通常是在等待 IO，比如磁盘、网络、其它外设等。如果 IO 设备出现了问题，或者 IO 响应慢，那么就会有很多进程处于 D 状态。

当出现了这类的进程后，要么等待 IO 设备满足请求，要么重启系统。

所以，为什么会出现这一状态？为什么内核不能正常回收？

### 正常 Sleep

一般来说，当一个进程在系统调用中正常休眠时，它会收到异步的信号，例如 `SIGINT`，此时会做如下的处理：

1. 系统调用立即返回，并返回 `-EINTR` 错误码；
2. 设置的信号回调函数被调用；
3. 如果进程仍然在运行，那么会获取到系统调用返回的错误码，并决定是否继续运行。

也就是说，正常的 Sleep 允许进程收到信号后做一些清理操作。

### 何时出现

进程会通过系统调用来调用 IO 设备，正常来说这一过程很快，用户几乎不会察觉，但是当 IO 设备异常或者设备驱动有 bug ，就可能会出现上述的状态。

例如通过 `read()` 系统调用读取磁盘上的数据，如果是机械磁盘，那么磁盘需要寻道、移动读针、读取数据，然后才会返回结果，那么在读取数据的过程中就处于 uninterruptable 状态。

正常这一过程很快，用户几乎无法感知。

### 为什么

这主要是因为，IO 请求比较特殊，它必须按照固定的顺序执行，甚至有些时序的要求，如果操作不是原子性的，那么就可能导致 IO 设备异常，可能会无法响应下次请求，可能会被死锁，这都跟具体的设备有关。

一个好的设备驱动需要处理这些异常场景，除非出现了 bug ，也就是说只有在一些极端场景下才会出现。

### KillAble

在 2.6.25 版本中引入了新的 `TASK_KILLABLE` 状态，会屏蔽普通信号，但可以响应强制信号，不过这个同样要依赖设备驱动的实现，详细可以参考 [TASK_KILLABLE](https://lwn.net/Articles/288056/) 。

<!--
https://www.ibm.com/developerworks/cn/linux/l-task-killable/index.html
-->


{% highlight text %}
{% endhighlight %}
