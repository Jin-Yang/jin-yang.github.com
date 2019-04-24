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

<!--
#### R (TASK_RUNNING，可执行状态)

只有在该状态的进程才可能在 CPU 上运行。而同一时刻可能有多个进程处于可执行状态，这些进程的 task_struct 结构（进程控制块）被放入对应 CPU 的可执行队列中（一个进程最多只能出现在一个 CPU 的可执行队列中）。进程调度器的任务就是从各个 CPU 的可执行队列中分别选择一个进程在该 CPU 上运行。<br><br>

    很多教科书将正在 CPU 上执行的进程定义为 RUNNING 状态、而将可执行但是尚未被调度执行的进程定义为 READY 状态，这两种状态在 Linux 下统一为 TASK_RUNNING 状态。</li><br><li>

    S (TASK_INTERRUPTIBLE，可中断的睡眠状态)<br>
    处于这个状态的进程因为等待某某事件的发生（比如等待 socket 连接、等待信号量），而被挂起。这些进程的 task_struct 结构被放入对应事件的等待队列中。当这些事件发生时（由外部中断触发、或由其他进程触发），对应的等待队列中的一个或多个进程将被唤醒。<br><br>

    通过 ps 命令我们会看到，一般情况下，进程列表中的绝大多数进程都处于 TASK_INTERRUPTIBLE 状态（除非机器的负载很高）。毕竟 CPU 就这么一两个，进程动辄几十上百个，如果不是绝大多数进程都在睡眠， CPU 又怎么响应得过来。</li><br><li>

    D (TASK_UNINTERRUPTIBLE，不可中断的睡眠状态)<br>
    与 TASK_INTERRUPTIBLE 状态类似，进程处于睡眠状态，但是此刻进程是不可中断的。不可中断，指的并不是 CPU 不响应外部硬件的中断，而是指进程不响应异步信号。<br><br>

    绝大多数情况下，进程处在睡眠状态时，总是应该能够响应异步信号的。否则你将惊奇的发现， kill -9 竟然杀不死一个正在睡眠的进程了！<br><br>

    而 TASK_UNINTERRUPTIBLE 状态存在的意义就在于，内核的某些处理流程是不能被打断的。如果响应异步信号，程序的执行流程中就会被插入一段用于处理异步信号的流程（这个插入的流程可能只存在于内核态，也可能延伸到用户态），于是原有的流程就被中断了。<br><br>

    在进程对某些硬件进行操作时（比如进程调用 read 系统调用对某个设备文件进行读操作，而 read 系统调用最终执行到对应设备驱动的代码，并与对应的物理设备进行交互），可能需要使用 TASK_UNINTERRUPTIBLE 状态对进程进行保护，以避免进程与设备交互的过程被打断，造成设备陷入不可控的状态。这种情况下的 TASK_UNINTERRUPTIBLE 状态总是非常短暂的，通过 ps 命令基本上不可能捕捉到。</li><br><li>

    T (TASK_STOPPED or TASK_TRACED，暂停状态或跟踪状态)<br>
    向进程发送一个 SIGSTOP 信号，它就会因响应该信号而进入 TASK_STOPPED 状态（除非该进程本身处于 TASK_UNINTERRUPTIBLE 状态而不响应信号）。SIGSTOP 与 SIGKILL 信号一样，是强制的，不允许用户进程通过 signal 系列的系统调用重新设置对应的信号处理函数。<br><br>

    向进程发送一个 SIGCONT 信号，可以让其从 TASK_STOPPED 状态恢复到 TASK_RUNNING 状态。<br><br>

    当进程正在被跟踪时，它处于 TASK_TRACED 这个特殊的状态。“正在被跟踪”指的是进程暂停下来，等待跟踪它的进程对它进行操作。比如在 gdb 中对被跟踪的进程下一个断点，进程在断点处停下来的时候就处于 TASK_TRACED 状态。而在其他时候，被跟踪的进程还是处于前面提到的那些状态。<br><br>

    对于进程本身来说， TASK_STOPPED 和 TASK_TRACED 状态很类似，都是表示进程暂停下来。而 TASK_TRACED 状态相当于在 TASK_STOPPED 之上多了一层保护，处于 TASK_TRACED 状态的进程不能响应 SIGCONT 信号而被唤醒。只能等到调试进程通过 ptrace 系统调用执行 PTRACE_CONT 、 PTRACE_DETACH 等操作（通过 ptrace 系统调用的参数指定操作），或调试进程退出，被调试的进程才能恢复 TASK_RUNNING 状态。
</li></ul>


有一类垃圾却并非这么容易打扫，那就是我们常见的状态为 D (Uninterruptible sleep) ，以及状态为 Z (Zombie) 的垃圾进程。这些垃圾进程要么是求而不得，像怨妇一般等待资源(D)，要么是僵而不死，像冤魂一样等待超度(Z)，它们在 CPU run_queue 里滞留不去，把 Load Average 弄的老高老高，没看过我前一篇blog的国际友人还以为这儿民怨沸腾又出了什么大事呢。怎么办？开枪！kill -9！看你们走是不走。但这两种垃圾进程偏偏是刀枪不入的，不管换哪种枪法都杀不掉它们。无奈，只好reboot，像剿灭禽流感那样不分青红皂白地一律扑杀！

贫僧还是回来说正题。怨妇 D，往往是由于 I/O 资源得不到满足，而引发等待，在内核源码 fs/proc/array.c 里，其文字定义为“ "D (disk sleep)", /* 2 */ ”（由此可知 D 原是Disk的打头字母），对应着 include/linux/sched.h 里的“ #define TASK_UNINTERRUPTIBLE 2 ”。举个例子，当 NFS 服务端关闭之时，若未事先 umount 相关目录，在 NFS 客户端执行 df 就会挂住整个登录会话，按 Ctrl+C 、Ctrl+Z 都无济于事。断开连接再登录，执行 ps axf 则看到刚才的 df 进程状态位已变成了 D ，kill -9 无法杀灭。正确的处理方式，是马上恢复 NFS 服务端，再度提供服务，刚才挂起的 df 进程发现了其苦苦等待的资源，便完成任务，自动消亡。若 NFS 服务端无法恢复服务，在 reboot 之前也应将 /etc/mtab 里的相关 NFS mount 项删除，以免 reboot 过程例行调用 netfs stop 时再次发生等待资源，导致系统重启过程挂起。

D是处于TASK_UNINTERRUPTIBLE的进程，深度睡眠，不响应信号。 一般只有等待非常关键的事件时，才把进程设为这个状态。

　　冤魂 Z 之所以杀不死，是因为它已经死了，否则怎么叫 Zombie（僵尸）呢？冤魂不散，自然是生前有结未解之故。在UNIX/Linux中，每个进程都有一个父进程，进程号叫PID（Process ID），相应地，父进程号就叫PPID（Parent PID）。当进程死亡时，它会自动关闭已打开的文件，舍弃已占用的内存、交换空间等等系统资源，然后向其父进程返回一个退出状态值，报告死讯。如果程序有 bug，就会在这最后一步出问题。儿子说我死了，老子却没听见，没有及时收棺入殓，儿子便成了僵尸。在UNIX/Linux中消灭僵尸的手段比较残忍，执行 ps axjf 找出僵尸进程的父进程号（PPID，第一列），先杀其父，然后再由进程天子 init（其PID为1，PPID为0）来一起收拾父子僵尸，超度亡魂，往生极乐。注意，子进程变成僵尸只是碍眼而已，并不碍事，如果僵尸的父进程当前有要务在身，则千万不可贸然杀之。
注意：不是所有状态为Z的进程都是无法收拾的，很可能是那个短暂的状态刚好被你发现了。
-->


### 特殊状态处理

`TASK_STOPPED`，进程终止，通常是由于向进程发送了 `SIGSTOP`、`SIGTSTP`、`SIGTTIN`、`SIGTTOU` 信号，此时可以通过 `kill -9(SIGKILL) pid` 尝试杀死进程，如果不起作用则 `kill -18 pid` ，也就是发个 `SIGCONT` 信号过去。


#### defunct

子进程是通过父进程创建的，子进程的结束和父进程的运行是一个异步过程，父进程永远无法预测子进程到底什么时候结束。当一个进程完成它的工作终止之后，它的父进程需要调用 `wait()` 或者 `waitpid()` 取得子进程的终止状态。

孤儿进程：一个父进程退出，相应的一个或多个子进程还在运行，那么那些子进程将成为孤儿进程。孤儿进程将被 init 进程所收养，并由 init 进程收集它们的完成状态。注意，孤儿进程没有危害，最终仍然回被 init 回收。

僵尸进程：一个进程使用 fork 创建子进程，如果子进程退出，而父进程并没有调用 wait 或 waitpid 获取子进程的状态信息，那么子进程的进程描述符仍然保存在系统中，仍然占用进程表，显示为 defunct 状态。可以通过重启或者杀死父进程解决。

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

## Uninterruptable

Linux 中有一个 `uninterruptable` 状态，此时的进程不接受任何的信号，包括了 `kill -9` ，通常是在等待 IO，比如磁盘、网络、其它外设等。如果 IO 设备出现了问题，或者 IO 响应慢，那么就会有很多进程处于 D 状态。

当出现了这类的进程后，要么等待 IO 设备满足请求，要么重启系统。

所以，为什么会出现这一状态？为什么内核不能正常回收？

### 正常 Sleep

一般来说，当一个进程在系统调用中正常休眠时，它会收到异步的信号，例如 SIGINT，此时会做如下的处理：

1. 系统调用立即返回，并返回 -EINTR 错误码；
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
