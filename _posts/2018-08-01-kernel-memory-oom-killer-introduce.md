---
title:
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

<!-- more -->

<!--

## OOM

1. 原理以及可配置参数是什么？
2. 内存的计算方式？
3. cgroup 中内存计算方式，感觉应该不是RSS？
4. OOM之后如何判断是怎么引起的？
5. 与ulimit限制的关系是什么？


## 简述

系统中的物理内存都会有个可用的上限，为了防止系统不可用，Linux 内核会通过

Linux 内核有个机制叫 OOM killer（Out-Of-Memory killer），该机制会监控那些占用内存过大，尤其是瞬间很快消耗大量内存的进程，为了防止内存耗尽而内核会把该进程杀掉。典型的情况是：某天一台机器突然ssh远程登录不了，但能ping通，说明不是网络的故障，原因是sshd进程被OOM killer杀掉了（多次遇到这样的假死状况）。重启机器后查看系统日志/var/log/messages会发现Out of Memory: Kill process 1865（sshd）类似的错误信息。


防止重要的系统进程触发(OOM)机制而被杀死：可以设置参数/proc/PID/oom_adj为-17，可临时关闭linux内核的OOM机制。内核会通过特定的算法给每个进程计算一个分数来决定杀哪个进程，每个进程的oom分数可以/proc/PID/oom_score中找到。我们运维过程中保护的一般是sshd和一些管理agent。


当物理内存和交换空间都被用完时，如果还有进程来申请内存，内核将触发 OOM Killer 。


### 防止被杀

为了防止进程被杀，可以将 `/proc/<PID>/oom_adj` 设置为 `-17` ，详细可以参考 `linux/oom.h` 中关于 `OOM_DISABLE` 宏的定义。

例如，对于一些常用的系统进程可以通过如下的方式设置，也可以添加到启动脚本里，或者 crontab 中。

pgrep -f "/usr/sbin/sshd" | while read PID;do echo -17 > /proc/$PID/oom_adj;done

### 行为

OOM 操作时的行为与两个参数相关，也就是 `/proc/sys/vm/{panic_on_oom,oom_kill_allocating_task}` 两个文件。

#### 选择进程

> Linux 中在出现了一些无法处理的场景时会触发 `Kernel Panic` ，类似于 Windows 中的蓝屏，此时只能重启服务器。
https://www.52os.net/articles/linux-force-kernel-panic-1.html
http://www.hulkdev.com/posts/oom_killer
http://blog.51cto.com/laoxu/1267097
http://hongjiang.info/tag/oom-killer/
http://blog.chinaunix.net/uid-29242873-id-3942763.html
https://segmentfault.com/a/1190000008268803
https://segmentfault.com/a/1190000008125359
https://blog.csdn.net/liukuan73/article/details/43238623
https://blog.csdn.net/kris_fei/article/details/8859854
https://www.kernel.org/doc/gorman/html/understand/understand007.html
https://lwn.net/Articles/529927/
https://github.com/torvalds/linux/blob/master/Documentation/cgroup-v1/memcg_test.txt
其行为如下：

1.检查文件/proc/sys/vm/panic_on_oom，如果里面的值为2，那么系统一定会触发panic
2.如果/proc/sys/vm/panic_on_oom的值为1，那么系统有可能触发panic（见后面的介绍）
3.如果/proc/sys/vm/panic_on_oom的值为0，或者上一步没有触发panic，那么内核继续检查文件/proc/sys/vm/oom_kill_allocating_task
3.如果/proc/sys/vm/oom_kill_allocating_task为1，那么内核将kill掉当前申请内存的进程
4.如果/proc/sys/vm/oom_kill_allocating_task为0，内核将检查每个进程的分数，分数最高的进程将被kill掉（见后面介绍）

进程被kill掉之后，如果/proc/sys/vm/oom_dump_tasks为1，且系统的rlimit中设置了core文件大小，将会由/proc/sys/kernel/core_pattern里面指定的程序生成core dump文件，这个文件里将包含
pid, uid, tgid, vm size, rss, nr_ptes, nr_pmds, swapents, oom_score_adj
score, name等内容，拿到这个core文件之后，可以做一些分析，看为什么这个进程被选中kill掉。

这里可以看看ubuntu默认的配置：

## 源码分析

mem_cgroup_oom_synchronize() cgroup中可能触发的OOM的入口
 |-mem_cgroup_out_of_memory()
   |-oom_kill_process() 
   | |-dump_header() 对应了invoked oom-killer    

正常的入口应该是 `out_of_memory()`

当然，本文所述的也不是所有的cache不能被释放的情形。那么，在你的应用场景下，还有那些cache不能被释放的场景呢？

mincore 
fincore

Documentation/cgroups/memory.txt
https://lwn.net/Articles/529927/
OOM日志分析
https://blog.csdn.net/kickxxx/article/details/50337647

业务进程使用的内存主要有以下几种情况：

1. 用户空间的匿名映射页 (（Anonymous pages in User Mode address spaces），比如调用malloc分配的内存，以及使用MAP_ANONYMOUS的mmap；当系统内存不够时，内核可以将这部分内存交换出去；
2. 用户空间的文件映射页（Mapped pages in User Mode address spaces），包含map file和map tmpfs；前者比如指定文件的mmap，后者比如IPC共享内存；当系统内存不够时，内核可以回收这些页，但回收之前可能需要与文件同步数据；
3. 文件缓存（page in page cache of disk file）；发生在程序通过普通的read/write读写文件时，当系统内存不够时，内核可以回收这些页，但回收之前可能需要与文件同步数据；
4. buffer pages，属于page cache；比如读取块设备文件。

其中（1）和（2）是算作进程的RSS，（3)和（4)属于page cache。

按照 [Memory Types](https://techtalk.intersec.com/2013/07/memory-part-1-memory-types/) 中的分类


可以通过 `systemd-cgtop -m` 查看，这里内存读取的实际上就是 `memory.usage_in_bytes` 中的内容。

很多的使用特性实际上可以参考 [Memory Resource Controller Implementation Memo](https://github.com/torvalds/linux/blob/master/Documentation/cgroup-v1/memcg_test.txt) 中的介绍。

很不错的文章，包括了Linux相关的内容
https://segmentfault.com/u/wuyangchun
关于cgroup内存相关的介绍
https://segmentfault.com/a/1190000008125359


插件进程ID、父进程ID、进程组ID、会话ID、用户名、组名、有效用户名、有效组名、SUID等信息，建议在测试时使用会话 ID。

ps -eo pid,ppid,pgid,tid,tgid,sid,user,group,euser,egroup,suid,cmd | grep 43430

当脚本 fork 了很多子进程之后(`sleep 1000&`)，如果直接 kill 对应的脚本，那么子进程实际上可能无法退出。实际上，在 `kill(3)` 中有如下的介绍:

If pid is negative, but not -1, sig shall be sent to all processes (excluding an unspecified set of system processes) whose process group ID is equal to the absolute value of pid, and for which the process has permission to send a signal.

也就是说在 `fork()` 出来子进程后，需要设置进程组 ID 信息，可以通过 `setpgrp()` 或者 `setpgid()` 进行设置。

#include <unistd.h>
pid_t setpgrp(void);
int setpgid(pid_t pid, pid_t pgid);

实测，如果通过 `kill(-PGID)` 方式，对于后台运行的任务实际上并未退出，使用 `killpg(PGID)` 接口一样。即使注册了 `SIGCHLD` 处理函数，实际上也只能接收到其直接的子进程，而对于孙子进程也是束手无策。

当然，也可以在脚本中将信号传递给对应的子进程，可以参考 [如何将SIGTERM信号传播到Bash脚本中的子进程](https://www.jianshu.com/p/603c08eab77b) 。


在 libev 中，如果要使用自己的 `SIGCHLD` 需要确保 `EV_CHILD_ENABLE` 关闭掉。


所以，对于像 [github tini](https://github.com/krallin/tini) 这样的进程，需要确保其以 `PID 1` 的方式启动，这样对于 Zombies 进程会自动被 tini 托管。

也可以通过注册一个 subreaper 的方式 (Linux >= 3.4) ，即使其运行的 PID 不是 1 也会收到相关的信号。

## 孤儿进程

当子进程还在运行时，如果其父进程被 `kill` 掉，那么这个子进程会变成孤儿进程 (Orphaned Process)，然后它会被 init 进程接管。

其它的一些场景，例如容器、DevOps的Agent，一般并非是 init 进程，对于容器一般是其主进程。

那么对于这些场景，如果出现了孤儿进程，那么谁会来接管这个进程？

### 内核处理

在 Linux 内核中，通过 `find_new_reaper()` 函数完成，其中的注释介绍了其处理方式，接管过程分为了三步：

1. 找到相同线程组里其它可用的线程，注意，是线程而非进程。
2. 如果没有找到则处理 subreaper 处理，下面详解。
3. 最后，使用 PID 为 1 的进程来接管孤儿进程。

其实这里的 subreaper 会比较复杂一些，下面简单介绍。

### SubReaper

可以参考 `prctl(2)` 中关于 `PR_SET_CHILD_SUBREAPER` 参数的介绍，其中 `prctl` 是内核暴露的函数用来查看或者修改进程的信息。

当进程被标记为 `subreaper` 之后，其创建的子进程，包括子进程的子进程都会标记为拥有一个 subreaper。

Upon termination of a process that is orphaned (i.e., its immediate parent has already terminated)
and marked as having a subreaper, the nearest still living ancestor subreaper will receive a SIGCHLD
signal and be able to wait(2) on the process to discover its termination status.

https://blog.phusion.nl/2015/01/20/docker-and-the-pid-1-zombie-reaping-problem/
http://adoyle.me/blog/orphaned-process-and-zombie-process-and-docker.html

简单来说，有一台 Windows 机器(B)可以连接到外网，同时可以连接到一台私有网段机器(A)。

----- 在(A)创建git代码仓库
cd /codes/project
git init
touch README.md
git add -A .
git commit -m "create project"

----- 切换到project父目录，并创建一个project-bare目录
cd ..
mkdir project-bare
cd project-bare

------ 从原仓库创建bare仓库，并以此作为"中央"仓库，其它机器(含本地)会往这里push/pull
git clone --bare ../project .

------ 在本地将project-bare添加为remote
cd ../project
git remote add origin ../project-bare
git branch --set-upstream-to=origin/master master

然后在机器 B 上连接机器 A 。

git clone ssh://<username>@<ip>:<port>:/codes/project-bare/project-bare.git project
git clone ssh://jinyang@10.120.185.240:/home/jinyang/experiment/project-bare project

clone 下来之后，在机器 B 上做修改，然后 commit/push 之后，在机器 A 上就可以 pull 到了；反之亦然。

那么，在机器 B 上对应了两个远端，包括了从 github 上下载，以及私网上的代码。

### 多个远端

这里会设置别名。

git remote add github https://github.com/Jin-Yang/MiniAgent.git

git config --global --unset http.proxy
git config --global --unset https.proxy



为减少日志文件占用的空间，很多情况下会将日志文件以天或周为周期打包成 `tar.gz` 包保存，但这样会导致压缩包不方便查看，在不解压可以通过 `zutils` 工具查看，例如 `zcat` `zcmp` `zdiff` `zgrep`。


----- 直接查看压缩包
$ zcat xxx.tar.gz
$ zgrep 'xxx' xxx.tar.gz

如果出现 `Binary file (standard input) matches` 类似的内容，可以添加 `--binary-files=text` 参数。主要是因为压缩使用的是 `tar czvf` 参数打包，实际上经过 `tar` 与 `gzip` 两层压缩，导致其直接不能管道。

可以通过 `tar tvf xxx.tar.gz` 不解压查看所包含的文件。

-->


{% highlight text %}
{% endhighlight %}
