---
title: Linux 内存 Proc
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,内存,kernel,内存空间
description:
---

<!-- more -->

## /proc/meminfo

该文件是查看当前 Linux 系统内存使用状况的主要入口，最常用的 free、vmstat、dstat 等命令就是通过它获取数据，该文件的输出实现在 ```fs/proc/meminfo.c``` 文件中。

{% highlight text %}
MemTotal:        8070604 kB   内核可用内存
MemFree:          842800 kB   从内核角度看，当前系统未使用内存(不含可回收内存)，包括LowFree+HighFree(CONFIG_HIGHMEM)
MemAvailable:    3720728 kB   在不发生swap时的最大可用内存，真正可用物理内存
Shmem:            962032 kB   一般是tmpfs使用，如/dev/shm,/run等，另外还有内核中的SysV


Buffers:          104944 kB
Cached:          3734768 kB
SwapCached:         1028 kB
Active:          4575172 kB
Inactive:        2156288 kB
Active(anon):    2779724 kB
Inactive(anon):  1074056 kB
Active(file):    1795448 kB
Inactive(file):  1082232 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:       8127484 kB
SwapFree:        8101540 kB
Dirty:              2156 kB
Writeback:             0 kB
AnonPages:       2891148 kB
Mapped:           554884 kB
Shmem:            962032 kB
Slab:             354444 kB
SReclaimable:     304396 kB
SUnreclaim:        50048 kB
KernelStack:        8144 kB
PageTables:        41536 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:    12162784 kB
Committed_AS:    6948452 kB
VmallocTotal:   34359738367 kB
VmallocUsed:      370112 kB
VmallocChunk:   34358947836 kB
HardwareCorrupted:     0 kB
AnonHugePages:    839680 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
DirectMap4k:      278372 kB
DirectMap2M:     5908480 kB
DirectMap1G:     2097152 kB
{% endhighlight %}

在计算可用内存时，最早一般使用 free+cached，而实际上 cached 中包含了

> 简单介绍下 CONFIG_HIGHMEM 。
>
> 部分 CPU (如ARM) 只能映射 4G 的内存管理空间，这 4G 空间包括了用户空间、内核空间、IO 空间，如果物理内存大于 4G ，那么必定有部分内存在这种情况下是无法管理的，这部分内存也就被称为 "high memory" 。
>
> 简单来说，之所以有 high memory 是因为物理内存超过了虚拟内存，导致内核无法一次映射所有物理内存，为此就需要有临时的映射。注意，创建临时映射的成本很高，需要修改内核的 page table 以及 TLB/MMU 。
>
>  详细可以查看 [Kenel-doc HIGH MEMORY HANDLING](https://www.kernel.org/doc/Documentation/vm/highmem.txt) 文档。

#### MemTotal

系统从加电开始到引导完成，firmware/BIOS 要保留一些内存，Kernel 本身要占用一些内存，最后剩下可供 Kernel 支配的内存就是 MemTotal 。

该值在系统运行期间一般固定不变，详细可查看 dmesg 中的内存初始化信息。

#### MemFree

从内核角度看，当前系统的可用内存，这里会将可以回收的内存也看作是已经分配的内存。

#### MemAvailable

记录当前真正可用的物理内存，注意 MemFree 不能代表全部可用的内存，系统中有些内存虽然已被使用但是可以回收的，比如 cache、buffer、slab 都有一部分可以回收，所以这部分可回收的内存加上 MemFree 才是系统可用的内存。

### Active VS. Inactive

除了通过 ```/proc/meminfo``` 查看外，还可以通过 ```vmstat -a``` 命令查看，与之相关的代码如下：

{% highlight c %}
static int meminfo_proc_show(struct seq_file *m, void *v)
{
    ... ...
    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_page_state(NR_LRU_BASE + lru);

    ... ...
    seq_printf(m,
        "Active:         %8lu kB\n"
        "Inactive:       %8lu kB\n"
        "Active(anon):   %8lu kB\n"
        "Inactive(anon): %8lu kB\n"
        "Active(file):   %8lu kB\n"
        "Inactive(file): %8lu kB\n"
        ... ...
        K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]),
        K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]),
        K(pages[LRU_ACTIVE_ANON]),
        K(pages[LRU_INACTIVE_ANON]),
        K(pages[LRU_ACTIVE_FILE]),
        K(pages[LRU_INACTIVE_FILE]),
        ... ...
}
{% endhighlight %}

为了实现 LRU 功能，正常应该有字段记录最近访问时间，可惜 x86 CPU 硬件并不支持这个特性，只能做到在访问页面时设置一个 Access Bit 标志位，无法记录时间。

所以 Linux 使用了一个折衷的方法，采用 LRU list 列表，把刚访问过的页面放在列首，越接近列尾的就是越长时间未访问过的页面，这样，虽然不能记录访问时间，但利用页面在 LRU list 中的相对位置也可以轻松找到年龄最长的页面。

内核设计了两种 LRU list: active list 和 inactive list，刚访问过的页面放进 active list，长时间未访问过的页面放进 inactive list，这样从 inactive list 回收页面就变得简单了。

内核线程 kswapd 会周期性地把 active list 中符合条件的页面移到 inactive list 中。 <!-- ，这项转移工作是由 refill_inactive_zone() 完成的。 -->

![active inactive list]({{ site.url }}/images/linux/kernel/memory-active-inactive-list.png "active inactive list"){: .pull-center width="80%" }

如上，如果 inactive list 很大，表明在必要时可以回收的页面很多，反之，则说明可以回收的页面不多。另外，这里的内存是用户进程所占用的内存而言的，内核占用的内存 (包括slab) 不在其中。

<!--
至于在源代码中看到的ACTIVE_ANON和ACTIVE_FILE，分别表示anonymous pages和file-backed pages。用户进程的内存页分为两种：与文件关联的内存（比如程序文件、数据文件所对应的内存页）和与文件无关的内存（比如进程的堆栈，用malloc申请的内存），前者称为file-backed pages，后者称为anonymous pages。File-backed pages在发生换页(page-in或page-out)时，是从它对应的文件读入或写出；anonymous pages在发生换页时，是对交换区进行读/写操作。
-->

<!--
/proc/meminfo之谜
http://linuxperf.com/?cat=7
-->


{% highlight text %}
{% endhighlight %}
