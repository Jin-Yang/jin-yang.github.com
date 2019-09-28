---
Date: October 19, 2013
title: Linux 中断
layout: post
comments: true
category: [linux]
language: chinese
---


中断在很大程度上解放了 CPU，提高了 CPU 的执行效率，弥补了 CPU 与外设运行速度之间的差距。

<!-- more -->

内核主要通过中断来管理硬件，与其相似的还有异常。

异常是指 CPU 内部出现的中断，即在 CPU 执行特定指令时出现的非法情况；异常也称为同步中断，也就是只有在一条指令执行后才会发出中断，不会在指令执行期间发生异常。常见的有除 0 错误，缺页异常，这个是不能屏蔽的。

中断通常是由其他硬件设备随机产生，可能会在指令执行过程中产生中断，分为可屏蔽中断和不可屏蔽中断。不可屏蔽中断如电源掉电、物理存储的奇偶校验等，可屏蔽中断。






# 中断原理

从硬件来开中断是由设备产生的一种电信号，并直接送入中断控制器 (Programmable Interrupt Controller, PIC)，再向 CPU 发送相应的信号，然后会 CPU 会中断当前处理的任务去处理中断。

## 8259A

在最早的 8086 阶段，CPU 是没有集成 PIC 的，但是提供了两个外接引脚 NMI 和 INTR，其中 NMI 为不可屏蔽中断，通常用于电源掉电和物理存储器奇偶校验；INTR 是可屏蔽中断，主要用于接受外部硬件的中断信号，可以进行屏蔽。

通常采用两个 8259A 级联实现，每个芯片提供 8 个中断源，总共可以提供 15 个中断信号源，使用之前需要对其进行初始化。

![8259a]{: .pull-center}

而 8259A 只适用于单 CPU ，当出现了 SMP 后，Intel 引入了 APIC (Advanced Programmable Interrupt Controller)，用于解决中断在各个 CPU 之间的路由关系，也即可以将中断传递给各个 CPU Cores ，该控制器最早在 Pentium 4 中出现。



## APIC

APIC 经历了 APIC、xAPIC、x2APIC，其基本架构没有变化，只是其通讯的总线有所改变，或者对部分的功能进行了扩展。

该组件包含两部分组成：Local APIC 和 I/O APIC 。Local APIC 位于 CPU 内部，负责传递中断信号到指定的处理器，每个 CPU 都会对应一个，同时它还有一个 Timer 功能，可以为所属的处理器提供本地时钟功能，而且还可以给发送中断消息给其他处理器 IPI (Inter Processor Interrupt)。

![ACPI]{: .pull-center}

I/O APIC 一般位于南桥芯片上，用来是收集来自 I/O 设备的中断信号，并按照配置将中断发送到 Local APIC，系统中最多可拥有 8 个 I/O APIC。

IO APIC 通过 LINT0 和 LINT1 引脚与 CPU 相连，相比于 8259，IOAPIC 可以处理更多的外设中断，如 ICH9 中单个 IOAPIC 可以支持 24 个中断，而且可以将接收到的中断分发到不同的处理器中。

每个 Local APIC 都有 32 位的寄存器，一个本地时钟以及为本地中断保留的两条额外的 IRQ 线 LINT0 和 LINT1，所有本地 APIC 都连接到 I/O APIC，形成一个多级 APIC 系统。



## MSI/MSI-X

在 PCI 总线中，所有需要提交中断请求的设备，必须能够通过 INTx 引脚提交中断请求，而 MSI 是可选机制。而在 PCIe 总线中，PCIe 设备必须支持 MSI 或者 MSI-X 中断请求机制，可以不支持 INTx 中断。

MSI (Message-Signaled Interrupts) 也就是基于消息信号的中断，相比 APIC 来说，更加灵活，性能更高。

MSI 中断究其本质，就是一个存储器读写事件，将 MSI Address 设置为内存中的某个地址，产生中断时，中断源会在 MSI Address 所在的地址写入 MSI Data。例如，如果有四条 MSI 中断线，就会依次写入 Data、Data+1、Data+2、Data+3 在内存中，依次来区分中断源设备。

对于设备来说，会在自己的配置空间定义了自己的 Capabilities list，如果该设备支持 MSI 中断，在此 list 中必定有一个节点的 ID=0x5D，其中 0x5D 表明是 MSI 中断节点，其位置由设备自定义。

MSI-X 是 MSI 的扩展，可以让一个硬件设备初始化多个中断向量，支持多个 CPU 同时处理一个 PCIe 设备的中断任务。




## 中断查看

要启用 MSI/MSI-X 类型的中断，需要在内核编译过程中带上相关的编译参数。通过 /proc/interrupts 可以查看系统的中断统计信息，以及中断类型，列表中如果有 IO-APIC 说明正在使用 APIC；如果看到 XT-PIC 则意味着正在使用 8259A 芯片；有 MSI 信息则说明是用 MSI 中断。


在 /proc/interrupts 中，第 1 列是中断号；接着是 CPUx 表示接收到的中断请求的次数；接着是对当前中断的描述，在 request_irq() 函数中传入。另外，NMI 和 LOC 是系统所使用的驱动，用户是无法访问的。

IRQ 号决定了中断的优先级，越小意味着优先级越高。

* IRQ0：系统时钟，不能改变。
* IRQ1：键盘控制器，不能改变。
* IRQ3：串口 2 的串口控制器，如有串口4 也会使用这个中断。
* IRQ4：串口 1 的串口控制器，如有串口3 也会使用这个中断。
* IRQ5：并口 2 和 3 或 声卡。
* IRQ6：软盘控制器。
* IRQ7：并口 1 被用于打印机，若没有打印机，可以用于其它的并口。

当然，现在通常只有 IRQ0 和 IRQ1，其它的上述中断已经成为了历史。


另外，IRQ 有一个关联的绑定属性 smp_affinity，该参数可以用来指定执行 ISR 的 CPU 核，该配置的内容保存在 /proc/irq/NUM/smp_affinity 文件中，可以通过 root 用户查看/修改该值。

该文件会一个十六进制的掩码，代表了系统中所有 CPU 核，以网卡 eth0 为例：

{% highlight text %}
# grep eth0 /proc/interrupts
 57:   5   0   2   1203   PCI-MSI-edge   eth0
# cat /proc/irq/57/smp_affinity
8
{% endhighlight %}

其中绑定关系用的是二进制，其对应关系为 0001(1)-CPU0、0101(5)-CPU0/2 ，那么上述 eth0 的 ISR 绑定到了 CPU3。

当然这还有一个前提，就是 irqbalance 服务需要关闭。irqbalance 是个服务进程，用来自动绑定和平衡 IRQ 的，可以通过 ps 查看是否有该进程。



## Linux 中断查看

Linux 内核中定义了 softirq 类型，通常来说不需要添加其它类型的软中断，如果需要一般使用 tasklets 。

{% highlight c %}
enum {   // include/linux/interrupt.h
    HI_SOFTIRQ=0,
    TIMER_SOFTIRQ,
    NET_TX_SOFTIRQ,
    NET_RX_SOFTIRQ,
    BLOCK_SOFTIRQ,
    BLOCK_IOPOLL_SOFTIRQ,
    TASKLET_SOFTIRQ,
    SCHED_SOFTIRQ,
    HRTIMER_SOFTIRQ,
    RCU_SOFTIRQ,    /* Preferable RCU should always be the last softirq */

    NR_SOFTIRQS
};
{% endhighlight %}

有如下的几种方式可以查看软中断的统计信息，包括在那些 CPU 上执行了多少次。

{% highlight text %}
----- 每种类型的软中断分别在每个CPU上执行了多少次
$ cat /proc/softirqs
                    CPU0       CPU1       CPU2       CPU3
          HI:        148         86         96         66
       TIMER:   93155814   83650552   78772010   82808729
      NET_TX:      11483      14361      29725       6904
      NET_RX:    2885712     452024    2343460     263921
       BLOCK:    8943601        842       2215       1086
BLOCK_IOPOLL:         58          0          1         16
     TASKLET:   19240313        848     221255        983
       SCHED:   20381866   17177463   11667301   10782048
     HRTIMER:          0          0          0          0
         RCU:   16624528   15482367   14754726   14837976

----- 查看每种中断的执行次数，第一列代表softirq总数，而后每一列分别对应一种软中断类型
$ cat /proc/stat |grep "softirq"
softirq 497658376 396 340734538 63039 5986156 8993118 75 19475944 60410520 0 61994590

----- 查看各个中断的执行次数
$ cat /proc/interrupts
           CPU0       CPU1       CPU2       CPU3
  0:         34          0          0          0   IO-APIC-edge      timer
  1:     459391          0          8          2   IO-APIC-edge      i8042
  7:         24          0          0          0   IO-APIC-edge
  8:          0          0          1          0   IO-APIC-edge      rtc0
  9:     857316         11       9635         83   IO-APIC-fasteoi   acpi
 12:   13855205         21        547         16   IO-APIC-edge      i8042
 16:          0          0          0          0   IO-APIC-fasteoi   mmc0
 19:   13747528         37       8297         14   IO-APIC-fasteoi   ath9k
 21:        942          9         30          3   IO-APIC-fasteoi   ehci_hcd:usb3
 40:        117          0         82          0   PCI-MSI-edge      snd_hda_intel
 41:         10          0          6          0   PCI-MSI-edge      mei_me
 42:    9241838        717       2096       1017   PCI-MSI-edge      0000:00:1f.2
 43:   19183739          6     245680          6   PCI-MSI-edge      i915

----- 同样可以通过/proc/stat查看中断出现的次数
$ cat /proc/stat |grep intr
intr 1006147094 34 459401 0 0 0 0 0 24 1 867368 0 0 13855789 ... ...
{% endhighlight %}

除了直接查看文件之外，也可以通过 dstat、vmstat 等指令获取中断的次数；可以通过 mpstat 命令查看每个 CPU 上 softirq 的开销。一般情况下，中断总数略大于软中断数。


## 中断亲和性

Linux 默认会在初始化时将所有的 CPU 分配给中断。

{% highlight c %}
static void __init init_irq_default_affinity(void)
{
    alloc_cpumask_var(&irq_default_affinity, GFP_NOWAIT);
    cpumask_setall(irq_default_affinity);
}
{% endhighlight %}

可以通过 cat /proc/irq/default_smp_affinity 查看当前的默认值。

实际上，很多设备不支持一个中断号被多个 CPU 处理，通常只有 CPU0 在真正处理中断请求，进而会导致 CPU0 由于压力过大产生问题，如响应时间增加，甚至可能产生丢包甚至 hang 住。

可以通过人工绑定，例如将一个网卡中断请求绑定到一个固定的 CPU core 上，步骤如下：

1. 确定网卡队列的中断号，cat /proc/interrutps \| grep "eth0-TxRx-0"；

2. 进入 /proc/irq/${IRQ}/ 查看其中的两个文件，smp_affinity 和 smp_affinity_list，改任意一个文件，另一个文件会同时更改；    
    smp_affinity 采用 16 进制掩码的方式，1 代表 CPU0，6 代表 CPU2、CPU1 ；    
    smp_affinity_list 采用 10 进制，可读性高，6 代表 CPU6，0-2,4-6 代表 0,1,2,4,5,6 ；

目前，很多的网卡、RAID 卡是支持多队列的，而实际上很多的硬件设备是不支持多队列的，在绑定的时候需要注意的几个点：

1. 有些中断还会落到 CPU0 上，因此最好不要将 CPU0 绑定到网卡中断。

2. 打散尽量按照物理 CPU 绑定，不要使用逻辑核。








# Linux 中断实现

设备中断会打断内核中进程的正常调度和运行，为了提高效率，必然要求中断服务尽可能的短小精悍，但是，有些 ISR 却需要大量的耗时处理。

为了提高系统的响应能力，Linux 将中断处理程序分为两个部分：上半部 (top half) 和下半部 (bottom half)。上半部处理时中断是被屏蔽的，所以通常用来处理一些比较紧急的任务，而且要尽可能快；下半部分通常就是正常的中断处理程序。

不过，对于上半部分和下半部分之间的划分没有特别严格的规则，通常是靠驱动程序开发人员自己的编程习惯来划分，不过还是有些习惯供参考：

* 如果该任务对时间比较敏感，将其放在上半部中执行。
* 如果该任务要保证不被其他中断打断，放在上半部中执行，因为此时系统中断是关闭的。
* 如果该任务和硬件相关，一般放在上半部中执行。

如果中断要处理的工作本身就很少，则完全可以直接在上半部全部完成。

如上所述，对于耗时的不太紧急的任务，一般会在下半部执行，而随着下半部的不断演化，已经从最原始的 Bottom Half 衍生出软中断 (softirq-2.3引入)、tasklet (2.3引入)、工作队列 (work queue-2.5引入)。






<!--
对于 x86 系统，实模式和保护模式的中断处理机制不同，两者的最本质差别就是在保护模式中引入了中断描述表，而正常 Linux 工作于保护模式下，在此重点说明保护模式下的中断机制。

在保护模式下，CPU 会根据中断描述符表 (IDT) 决定中断、异常的处理，当中断或异常发生时，通过中断向量号对应的 IDT 表项决定动作。IDT 的每一项为一个门描述符，保存了对应中断的权限信息以及相应程序的入口。在 x86 CPU 中，门被分为多种类型，且权限的检查十分繁琐，而 Linux 实际只使用了这个机制很简单的一部分。

Intel 最多支持 256 各中断源，也就是 256 个 IDT 项，其中前 32 项是由 Intel 定义的异常 (0~19) 和保留项 (20~31)；剩余的为可屏蔽中断 (32~255)，可以直接通过 INT num 调用。其中异常包括了除法错误、不可屏蔽中断 (Non-maskable external interrupt)、Breakpoint (INT3 用于调试)等。


在实地址模式中，CPU 把内存中从 0 开始的 1KB 空间作为一个中断向量表，表中的每个表项占四个字节，由两个字节的段地址和两个字节的偏移量组成，这样构成的地址就是相应中断处理程序的入口地址。

但是在保护模式下，由 4 个字节的表项构成的中断向量表已经不能满足要求了；在保护模式下，中断向量表中的表项由 8 个字节组成，被称为中断描述表 (Interrupt Descriptor Table，IDT)，每项叫做一个门描述符 (Gate Descriptor) 。而且不再局限于位置 0 开始，而是可以存放在任意位置。


用户进程可以通过 INT 指令发出一个中断请求，其中断请求向量在 ０~255 之间，可以将 DPL 设置为 0 ，防止用户使用 INT 指令模拟非法中断和异常。而系统调用，必须让用户进程可以从用户态进入内核态，此时可以将 DPL 设置为 3 来实现。
-->





[8259a]:         /images/linux/interrupt-8259A.png                "古老的8259A中断控制芯片"
[ACPI]:          /images/linux/interrupt-xACPI.jpg                "ACPI中断控制芯片架构"
