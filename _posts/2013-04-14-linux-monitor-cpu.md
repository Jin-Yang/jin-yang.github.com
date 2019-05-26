---
title: Linux 监控之 CPU
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,monitor,监控,cpu
description: Linux 中的系统监控，包括了常见的计算、内存、网络等监控指标，以及相应的工具。
---

Linux 中的系统监控，包括了常见的计算、内存、网络等监控指标，以及相应的工具。

<!-- more -->

## CPU Usage VS. LOAD

Linux 系统的 CPU 使用率就是 CPU 的使用状况，也就是一段时间之中，CPU 用于执行任务占用的时间与总的时间的比率。

<!--
{% highlight text %}
%user Percentage of CPU utilization that occurred while executing at the user level (application). Note that this field includes time spent running virtual processors. （未标志nice值的）用户态程序的CPU占用率。
%nice Percentage of CPU utilization that occurred while executing at the user level with nice priority. 标志了nice值的用户态程序的CPU占用率。
%system Percentage of CPU utilization that occurred while executing at the system level (kernel). Note that this field includes time spent servicing hardware and software interrupts. 系统态（内核）程序的CPU占用率。
%iowait Percentage of time that the CPU or CPUs were idle during which the system had an outstanding disk I/O request. I/O等待的CPU占用率。
%steal Percentage of time spent in involuntary wait by the virtual CPU or CPUs while the hypervisor was servicing another virtual processor. 这个一般是在虚拟机中才能看到数值，比如：我的VPS供应商CPU overcommitment很严重，故我偶尔能看到%steal值有点高。
%idle Percentage of time that the CPU or CPUs were idle and the system did not have an outstanding disk I/O request. %idle越高，说明CPU越空闲。
{% endhighlight %}
-->

### 相关文件

CPU 的全局性能指标保存在 ```/proc/stat``` 文件中，大部分监控都是读取该文件，如 dstat 。对于文件的解读可以参考 man 5 proc 中的 ```/proc/stat``` 部分，也可以参考 [online man  5 proc](http://man7.org/linux/man-pages/man5/proc.5.html) 。

进程和线程的统计可参考 ```/proc/<pid>/stat``` 和 ```/proc/<pid>/task/<tid>/stat```；需要注意的是，对于不同的内核版本， ```/proc/stat``` 内容会有所区别：```steal (since Linux 2.6.11)```、```guest (since Linux 2.6.24)```、```guest_nice (since Linux 2.6.33)``` 。

{% highlight text %}
cpu  3816877 11951 424208 1254811 21564 0 2153 0 0 0
cpu0 958728 2517 108301 895882 15136 0 1048 0 0 0
cpu1 953763 3080 108605 117901 2376 0 431 0 0 0
cpu2 955060 3789 102803 119840 2140 0 588 0 0 0
cpu3 949325 2563 104497 121187 1910 0 85 0 0 0
intr 88396062 67 62144 0 0 0 0 0 0 1 126484 0 0 2126483 ... ...
ctxt 156849121
btime 1488368299
processes 126703
procs_running 2
procs_blocked 0
softirq 76359224 43 47319594 111 58877 726684 9 63206 16513537 0 11677163
{% endhighlight %}

对于上述文件的内容，简单介绍如下。

{% highlight text %}
cpu cpuN
    CPU使用情况，都是从系统启动到当前的值，单位是节拍 (Jiffies)，总计分为10列；
  **user
    普通进程处于用户态的运行时间，不包括nice为负的进程消耗时间。
  **nice
    nice值为负的进程所占用的用户态时间。
  **system
    处于核心态的运行时间，也包括 IRQ 和 softirq 时间。
    如果系统CPU占用率高，表明系统某部分存在瓶颈，通常值越低越好。
  **idle
    空闲时间，不包含 IO 等待时间。
  **iowait
    等待 IO 响应的时间(since 2.5.41)。
    如果系统花费大量时间等待 IO ，说明存在 IO 瓶颈。
  **irq
    处理硬中断的时间(since 2.6.0-test4)。
  **softirq
    处理软中断的时间(since 2.6.0-test4)。
  **steal
    当采用虚拟环境时，运行在其它操作系统上的时间，此时hypervisor在为另一个虚拟处理器服务，具体监控指标详见后面。
  **guest
    内核运行在虚拟机上的时间。
  **guest_nice
    同上。
intr
    中断信息，首列为系统启动以来，发生的所有的中断的次数；接着的每个数对应特定中断自系统启动以来所发生的次数。
ctxt
    系统启动以来CPU发生的上下文交换的次数。
btime
    给出了从系统启动到现在为止的时间，单位为秒。
processes
    自系统启动以来所创建的任务的个数目。
procs_running
    当前运行队列的任务的数目。
procs_blocked
    当前被阻塞的任务的数目。
{% endhighlight %}

在 `/proc/stat` 文件中定义了 CPU 相关的指标，需要注意其单位，如果单纯计算比例 (例如使用率) 可以忽略。

各个字段的计算规则如下：

{% highlight text %}
Total CPU time since boot = user+nice+system+idle+iowait+irq+softirq+steal
Total CPU Idle time since boot = idle + iowait
Total CPU usage time since boot = Total CPU time since boot - Total CPU Idle time since boot
Total CPU percentage = Total CPU usage time since boot/Total CPU time since boot X 100
{% endhighlight %}

<!--
https://github.com/Leo-G/DevopsWiki/wiki/How-Linux-CPU-Usage-Time-and-Percentage-is-calculated
-->

注意：`guest` 和 `guest_nice` 已经统计到了 `user` 和 `nice` 中，所以在计算总量的时候应该忽略。

PS: 如果 iowait 的值过高，表示硬盘存在 I/O 瓶颈。如果 idle 值高但系统响应慢时，有可能是 CPU 等待分配内存，此时应加大内存容量。

### 内核实现

/proc/stat 文件在 ```proc_stat_init()@fs/proc/stat.c``` 函数中实现，会在内核初始化时调用；对上述文件的读写，会调用 proc_stat_operations 所定义的函数。

{% highlight c %}
static const struct file_operations proc_stat_operations = {
    .open       = stat_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
{% endhighlight %}

打开文件会调用函数 stat_open() ，该函数首先申请大小为 size 的内存，来存放临时数据，也是我们看到的 stat 里的最终数据；文件中的数据由 show_stat() 函数填充。

{% highlight c %}
static int show_stat(struct seq_file *p, void *v)
{
    int i, j;
    unsigned long jif;
    u64 user, nice, system, idle, iowait, irq, softirq, steal;
    u64 guest, guest_nice;
    u64 sum = 0;
    u64 sum_softirq = 0;
    unsigned int per_softirq_sums[NR_SOFTIRQS] = {0};
    struct timespec boottime;

    user = nice = system = idle = iowait =
        irq = softirq = steal = 0;
    guest = guest_nice = 0;
    getboottime(&boottime);
    jif = boottime.tv_sec;

    for_each_possible_cpu(i) {
        user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
        nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
        system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
        idle += get_idle_time(i);
        iowait += get_iowait_time(i);
        irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
        softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
        steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
        guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
        guest_nice += kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
        sum += kstat_cpu_irqs_sum(i);
        sum += arch_irq_stat_cpu(i);

        for (j = 0; j < NR_SOFTIRQS; j++) {
            unsigned int softirq_stat = kstat_softirqs_cpu(j, i);

            per_softirq_sums[j] += softirq_stat;
            sum_softirq += softirq_stat;
        }
    }
    sum += arch_irq_stat();

    seq_puts(p, "cpu ");
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(user));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(nice));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(system));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(idle));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(iowait));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(irq));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(softirq));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(steal));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(guest));
    seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(guest_nice));
    seq_putc(p, '\n');

    for_each_online_cpu(i) {
        /* Copy values here to work around gcc-2.95.3, gcc-2.96 */
        user = kcpustat_cpu(i).cpustat[CPUTIME_USER];
        nice = kcpustat_cpu(i).cpustat[CPUTIME_NICE];
        system = kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
        idle = get_idle_time(i);
        iowait = get_iowait_time(i);
        irq = kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
        softirq = kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
        steal = kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
        guest = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
        guest_nice = kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
        seq_printf(p, "cpu%d", i);
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(user));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(nice));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(system));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(idle));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(iowait));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(irq));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(softirq));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(steal));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(guest));
        seq_put_decimal_ull(p, ' ', cputime64_to_clock_t(guest_nice));
        seq_putc(p, '\n');
    }
    seq_printf(p, "intr %llu", (unsigned long long)sum);

    /* sum again ? it could be updated? */
    for_each_irq_nr(j)
        seq_put_decimal_ull(p, ' ', kstat_irqs_usr(j));

    seq_printf(p,
        "\nctxt %llu\n"
        "btime %lu\n"
        "processes %lu\n"
        "procs_running %lu\n"
        "procs_blocked %lu\n",
        nr_context_switches(),
        (unsigned long)jif,
        total_forks,
        nr_running(),
        nr_iowait());

    seq_printf(p, "softirq %llu", (unsigned long long)sum_softirq);

    for (i = 0; i < NR_SOFTIRQS; i++)
        seq_put_decimal_ull(p, ' ', per_softirq_sums[i]);
    seq_putc(p, '\n');

    return 0;
}
{% endhighlight %}

在上述的函数中，会依次统计 cpu 的各个参数，下面以 user 为例。


<!--
{% highlight text %}
user = kcpustat_cpu(i).cpustat[CPUTIME_USER];
user = per_cpu(kernel_cpustat, i).cpustat[CPUTIME_USER];
user = (*SHIFT_PERCPU_PTR(&(kernel_cpustat), per_cpu_offset(i))).cpustat[CPUTIME_USER];
{% endhighlight %}


kernel/sched/cputime.c


其中包括了 cpu 的心能指标，进程切换次数ctxt、内核启动的时间btime、所有创建的进程processes、正在运行进程的数量procs_running、阻塞的进程数量procs_blocked、所有io等待的进程数量、软中断的信息。

在top、sar、vmstat、mpstat等命令中可以看到CPU使用率通常包含如下几种统计（摘自 man sar）：

top
[root@jay-linux ~]# sar -u 1 5
[root@jay-linux ~]# vmstat -n 1 5
[root@jay-linux ~]# mpstat -P ALL 1 5


Linux的Load（系统负载），是一个让新手不太容易了解的概念。top/uptime等工具默认会显示1分钟、5分钟、15分钟的平均Load。具体来说，平均Load是指，在特定的一段时间内统计的正在CPU中运行的(R状态)、正在等待CPU运行的、处于不可中断睡眠的(D状态)的任务数量的平均值。
我估计也没说的太清楚，看下wikipedia上的一段话吧：An idle computer has a load number of 0. Each process using or waiting for CPU (the ready queue or run queue) increments the load number by 1. Most UNIX systems count only processes in the running (on CPU) or runnable (waiting for CPU) states. However, Linux also includes processes in uninterruptible sleep states (usually waiting for disk activity), which can lead to markedly different results if many processes remain blocked in I/O due to a busy or stalled I/O system.
还有man sar中的解释：The load average is calculated as the average number of runnable or running tasks (R state), and the number of tasks in uninterruptible sleep (D state) over the specified interval.
对于一个系统来说，多少的Load算合理，多少又算Load高呢？
一般来说，对于Load的数值不要大于系统的CPU核数（或者开启了超线程，超线程也当成CPU core吧）。当然，有人觉得Load等于CPU core数量的2倍也没事，不过，我自己是在Load达到CPU core数量时，一般都会去查看下是什么具体原因导致load较高的。
Linux中查看Load的命令，推荐如下：
[root@jay-linux ~]# top
[root@jay-linux ~]# uptime
[root@jay-linux ~]# sar -q 1 5
最后，说一下CPU使用率和Load的关系吧。如果主要是CPU密集型的程序在运行（If CPU utilization is near 100 percent (user + nice + system), the workload sampled is CPU-bound.），那么CPU利用率高，Load一般也会比较高。而I/O密集型的程序在运行，可能看到CPU的%user, %system都不高，%iowait可能会有点高，这时的Load通常比较高。同理，程序读写慢速I/O设备（如磁盘、NFS）比较多时，Load可能会比较，而CPU利用率不一定高。这种情况，还经常发生在系统内存不足并开始使用swap的时候，Load一般会比较高，而CPU使用率并不高。

本文就简单说这么多了，想了解更全面的信息，可参考以下方法：
1. man sar, man top （认真看相关解析，定有收获）
2. wikipedia：http://en.wikipedia.org/wiki/Load_%28computing%29
3. 帮助理解Load：http://blog.scoutapp.com/articles/2009/07/31/understanding-load-averages
4. 帮助理解load的中文博客：http://www.blogjava.net/cenwenchu/archive/2008/06/30/211712.html
http://os.51cto.com/art/201012/240719.htm
</p>


> 通过 uptime 可以查看当前时间、已经运行时间、用户、负载等信息，top 命令第一行也是。
> <pre style="font-size:0.8em;line-height:1.2em;">$ uptime
> 18:02:41 up 41 days, 23:42,  3 users,  load average: 0.20, 0.32, 0.45</pre>
> 显示当前时间 "18:02:41" ，已经运行的时间 "up 41 days, 23:42" ，登录用户数 "3 users" ，过去 1, 5, 15 分钟的平均负载 "load average: 0.20, 0.32, 0.45" 。






一般程序分为了 CPU/IO 密集型，不同类型的进程表现以及瓶颈也有所区别。如果是 CPU 密集型，当增大压力时，就很容易把 CPU 利用率打满；而对于 IO 密集型，增大压力时，CPU 使用率不一定能上去。

对于后者，大文件读写的 CPU 开销远小于小文件读写的开销；这时因为在 IO 吞吐量一定时，小文件的读写更加频繁，需要更多的 CPU 来处理 IO 中断。



在Linux/Unix下，CPU利用率分为用户态，系统态和空闲态，分别表示CPU处于用户态执行的时间，系统内核执行的时间，和空闲系统进程执行的时间。平时所说的CPU利用率是指：CPU执行非系统空闲进程的时间 / CPU总的执行时间。

在Linux的内核中，有一个全局变量：Jiffies。 Jiffies代表时间。它的单位随硬件平台的不同而不同。系统里定义了一个常数HZ，代表每秒种最小时间间隔的数目。这样jiffies的单位就是1/HZ。Intel平台jiffies的单位是1/100秒，这就是系统所能分辨的最小时间间隔了。每个CPU时间片，Jiffies都要加1。 CPU的利用率就是用执行用户态+系统态的Jiffies除以总的Jifffies来表示。

在Linux系统中，可以用/proc/stat文件来计算cpu的利用率(详细的解释可参考：http://www.linuxhowtos.org/System/procstat.htm)。这个文件包含了所有CPU活动的信息，该文件中的所有值都是从系统启动开始累计到当前时刻。
-->


## 参考

<!--
https://linux.cn/article-6201-1.html    *****非常经典：关于现代 CPU，程序员应当更新的知识

http://www.cnblogs.com/yjf512/archive/2012/12/10/2811823.html

/proc/stat的CPU负载信息说明
http://myssh.igigo.net/post/xiao-za-sui/-proc-statde-cpufu-zai-xin-xi-shuo-ming

Understanding CPU Steal Time - when should you be worried
?http://blog.scoutapp.com/articles/2013/07/25/understanding-cpu-steal-time-when-should-you-be-worried

Linux CPU利用率计算原理及内核实现
http://ilinuxkernel.com/?p=333

http://www.samirchen.com/linux-cpu-performance/

https://linux.cn/article-6201-1.html    *****非常经典：关于现代 CPU，程序员应当更新的知识
-->

{% highlight text %}
{% endhighlight %}
