---
Date: October 19, 2013
title: Linux 进程切换与协程
layout: post
comments: true
language: chinese
category: [linux]
---

在本文中介绍了 Linux 的进程切换以及协程的相关文档，包括了上下文相关信息、Linux 进程切换的过程以及协程相关的信息。

<!-- more -->


# 简介

处理器在任何时候总会处于以下状态中的一种：

1. 内核态，与进程无关，处于中断上下文，处理中断请求。
2. 内核态，与进程无关，用于处理 softirq 或者 tasklet 。
3. 内核态，运行于进程上下文，处理进程在内核态的一些请求。
4. 用户态，运行于用户空间。

上述的状态是有顺序关系的，其中后两者可以相互抢断；而其它的则只能按照顺序优先级，例如当一个 CPU 上跑着 softirq 时，其它的 softirq 是不会执行的，但是硬件的中断可以抢占。

接下来看看什么是上下文。




## 上下文

一个进程执行时，其中 CPU 所有寄存器的值、进程的状态信息、进程打开的文件、进程相关的内存信息、堆栈中的内容等被称为该进程的上下文。

另外，在硬件信号触发中断时，会导致内核调用中断处理程序，进入内核空间。而在这一过程的硬件相关的信息，也要传递给内核，内核通过这些参数进行中断处理，这一环境也就被称为中断上下文。

通常来说，一个进程的上下文就被分为如下的三个部分：

* 用户级上下文: 正文、数据、用户堆栈以及共享存储区；
* 寄存器上下文: 通用寄存器、程序寄存器 (IP)、处理器状态寄存器 (EFLAGS)、栈指针 (ESP)；
* 系统级上下文: 进程控制块 (task_struct)、内存管理信息 (mm_struct、vm_area_struct、pgd、pte)、内核栈。

当发生进程调度时，进行进程切换就是上下文切换 (context switch)，操作系统必须对上面提到的全部信息进行切换，包括保存现在的进程上下文，并切换到新的进程上下文，新调度的进程才能运行。




## 进程切换

Linux 进程切换是通过 schedule() 函数完成的，用于选择那个进程可以运行，何时投入运行。其中，会涉及到具体使用那个调度类执行，本文中不会涉及太多，可以通过如下命令查看所有的调度类：

{% highlight text %}
$ grep kernel/sched -rne 'const struct sched_class .* = {'
{% endhighlight %}

假设，已经选中了需要切换的进程，那么进程切换的流程基本如下：

{% highlight text %}
schedule()
 |-__schedule()
   |-pick_next_task()           # 选择需要调度的进程
   |-context_switch()           # 切换到新的MM以及寄存器状态
     |-switch_mm()              # 把虚拟内存从一个进程映射切换到新进程中
     |-switch_to()              # 保存、恢复寄存器状态以及栈信息
     |-barrier()                # 内存屏障
{% endhighlight %}

可见，在进行进程切换时，就涉及了上述进程上下文中的全部信息，同时会导致该进程相关的 CPU Cache 都失效了。






# 参考

关于上下文信息可以参考 Documentation/DocBook/kernel-hacking.xml 的内容，需要通过 make help 查看内核所有的命令，通过 make htmldocs 生成 html 格式的文档。

介绍如何测试上下文切换的成本，可以参考一篇相关的论文 [Quantifying The Cost of Context Switch](http://www.cs.rochester.edu/u/cli/research/switch.pdf) 。


关于进程切换的耗时强烈推荐参考 [How long does it take to make a context switch?](http://blog.tsunanet.net/2010/11/how-long-does-it-take-to-make-context.html) 这篇文章，其中包括了多种类型的比较，以防万一保存了一个 mhtml 格式的 [本地版本](/reference/linux/How long does it take to make a context switch.mhtml) 。



<!--
http://blog.csdn.net/zhoudaxia/article/details/7375836            Linux进程调度
http://blog.amalcao.me/blog/2014/07/10/cyu-yan-xie-cheng-xi-tong-jie-shao-zhi-yi/#/index
-->
