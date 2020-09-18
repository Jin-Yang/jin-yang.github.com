---
title: Kernel 内存映射
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,内存,memory,映射
description:
---


<!-- more -->


## iomem VS. ioports

Linux 在 proc 目录下有 iomem 和 ioports 文件，主要描述了系统的内存和 IO 端口资源分布，两个地址空间分别编制，均是从 0 开始，如果硬件支持 MMIO，port 地址也可以映射到 memory 空间去。

### IO 地址映射

对于外设的访问控制，最终都是通过读写设备上的寄存器实现的，而寄存器不外乎控制寄存器、状态寄存器和数据寄存器，这些外设寄存器也称为 "IO端口"，并且一个外设的寄存器通常是连续编址。不同的 CPU 体系对外设 IO 端口物理地址的编址方式也不同，分为 IO 映射方式 (IO-Mapped) 和内存映射方式 (Memory-Mapped)。

x86 为外设专门实现有单独的地址空间，称为 "IO地址空间"，这是独立于 CPU 和 RAM 的物理地址空间，它将所有外设的 IO 端口均在这一空间进行编址。CPU 通过设立专门的 IN/OUT 指令来访问这一空间中的地址单元，也就是 IO-Mapped ，其地址空间一般只有 64KB(```0000~FFFF```)。

Linux 设计了一个通用的 ```struct resource``` 来描述各种 IO 资源，包括 IO 端口、外设内存、DMA 和 IRQ 等，通过树形结构来管理每一类 IO 资源。

<!--
这里以pci设备为例，硬件的拓扑结构就决定了硬件在内存映射到CPU的物理地址，由于内存访问都是虚拟地址，所有就需要ioremap，此时物理内存是存在的，所以不用再分配内存，只需要做映射即可
应用总结：使用I/O内存首先要申请,然后才能映射,使用I/O端口首先要申请,对I/O端口的请求是让内核知道你要访问该端口,内核并让你独占该端口.
申请I/O端口的函数是request_region, 申请I/O内存的函数是request_mem_region。request_mem_region函数并没有做实际性的映射工作，只是告诉内核要使用一块内存地址，声明占有，也方便内核管理这些资源。重要的还是ioremap函数，ioremap主要是检查传入地址的合法性，建立页表（包括访问权限），完成物理地址到虚拟地址的转换。
在intel的X86平台，GPIO资源也是类似应用，如果IO配置为SCI或者SMI中断，SCI可以产生GPE，然后经历acpi子系统，不过GPE中断号默认是0x10+GPIO端口号。
-->


## 用户虚拟地址到物理地址转换

查看当前进程的虚拟地址映射 ```cat /proc/self/maps```，

通过命令你给 ```ps aux | less``` 查看内存相关参数时，会发现两个比较有疑问的指标 RSS、VSZ (单位是KB)，通过 ```man 1 ps``` 查看对应的解释如下：

 * RSS resident set size, the non-swapped physical memory that a task has used (in kiloBytes). (alias rssize, rsz).
 * VSZ virtual memory size of the process in KiB (1024-byte units). Device mappings are currently excluded; this is subject to change. (alias vsize).

简言之，RSS 就是这个进程实际占用的物理内存；VSZ 就是进程的虚拟内存，包括了还未发生缺页异常加载映射到内存中。如果通过上述的两个指标来评估实际内存使用量，那么是错误的 ！！！

ps 显示的统计结果实际上有个前提假设：如果只有这一个进程在运行。在现在的内核中，显然是不可能的，很多进程会共享一些内存，例如 libc 。




## 其它

### 刷新内存

可以通过手动执行 sync 命令刷新内存，以确保文件系统的完整性，sync 命令将所有未写的系统缓冲区写到磁盘中，包含已修改的 i-node、已延迟的块 I/O 和读写映射文件。

{% highlight text %}
# sync; echo 1 > /proc/sys/vm/drop_caches              // 仅清除页面缓存PageCache
# sync; echo 2 > /proc/sys/vm/drop_caches              // 清除目录项和inode
# sync; echo 3 > /proc/sys/vm/drop_caches              // 清除页面缓存，目录项和inode
{% endhighlight %}

其中第一个是比较安全的，可以在生产环境中使用；最后一个操作比较危险，不建议在生产环境中使用，除非你自己清楚在做什么，源码实现在 fs/drop_caches.c 中。实际上，上述的操作通常针对 IO 的基准测试；否则不建议清除缓存。

可以通过如下命令清除掉交换区的空间。

{% highlight text %}
# swapoff -a && swapon -a
{% endhighlight %}

<!--
可用内存计算方法
http://www.cnblogs.com/feisky/archive/2012/04/14/2447503.html

一次高内存使用率的告警处理

http://farll.com/2016/10/high-memory-usage-alarm/

http://marek.vavrusa.com/c/memory/2015/02/20/memory/

http://careers.directi.com/display/tu/Understanding+and+optimizing+Memory+utilization



Documentation/filesystems/proc.txt

http://www.linuxatemyram.com/

http://rubenlaguna.com/wp/2015/02/22/posix-slash-system-v-shared-memory-vs-threads-shared-memory/

https://www.ibm.com/developerworks/cn/aix/library/au-ipc/
http://blog.jqian.net/post/linux-shm.html
https://www.ibm.com/developerworks/cn/linux/l-cn-slub/
http://blog.csdn.net/bullbat/article/details/7194794
http://blog.scoutapp.com/articles/2009/07/31/understanding-load-averages
https://engineering.linkedin.com/performance/optimizing-linux-memory-management-low-latency-high-throughput-databases

http://kernel.taobao.org/index.php?title=Kernel_Documents/mm_sysctl

https://lwn.net/Articles/422291/
http://linuxperf.com/?p=142

http://blog.csdn.net/ctthuangcheng/article/details/8916065

https://www.kernel.org/doc/gorman/html/understand/understand015.html

http://blog.csdn.net/kickxxx/article/details/8618451

https://www.kernel.org/doc/gorman/html/understand/understand005.html





dentry cache 和 inode cache 区别。



内存泄露检测。

可以通过 pmap 进行检查，原则如下：

1. Virtual Memory(VIRT) 或者 writeable/private (pmap –d) 一直在增长；



2, The RSS is just for your reference, it can’t used as the factor to detect memory leak.

3, although you call ‘free’ or ‘delete’, the virtual memory may not shrink immediately.

4, the same new operation may mapped to different [anon] virtual memory space. Here 1M was alloated to 000000001b56a000    and 00002ac25a77c000.

    000000001b56a000    1156      12      12 rw---    [ anon ]

    00002ac25a77c000    1040      16      16 rw---    [ anon ]

nm and objdump -x readelf

0000000000400000      4K r-x-- memtest               可执行文件
0000000000600000      4K r---- memtest               只读数据.rodata
0000000000601000      4K rw--- memtest               全局数据
00007f6f29e62000   1752K r-x-- libc-2.17.so
00007f6f2a018000   2048K ----- libc-2.17.so
00007f6f2a218000     16K r---- libc-2.17.so
00007f6f2a21c000      8K rw--- libc-2.17.so
00007f6f2a21e000     20K rw---   [ anon ]
00007f6f2a223000    128K r-x-- ld-2.17.so
00007f6f2a431000     12K rw---   [ anon ]
00007f6f2a43f000     12K rw---   [ anon ]
00007f6f2a442000      4K r---- ld-2.17.so
00007f6f2a443000      4K rw--- ld-2.17.so
00007f6f2a444000      4K rw---   [ anon ]
00007ffc38e32000    132K rw---   [ stack ]
  0x7ffc38e51504
00007ffc38f74000      8K r-x--   [ anon ]
ffffffffff600000      4K r-x--   [ anon ]
 total             4164K

内存申请、释放会立即在writeable/private中显示。

pmap -x $(pidof memtest)
pmap -x $(pidof uagent)
top -p $(pidof uagent)

两次申请的内存是否相同？




/proc/PID/{maps,smaps,statm} 中的内容，与 pmap 打印的信息比较相似，接下来看看 Mode 中的信息是什么意思？哪些标示了真正的内存？

首先与 PID 相关的 proc 系统在 fs/proc/base.c 文件中实现，详见 tid_base_stuff 中的入口函数定义。

#### statm

对应了 fs/proc/array.c 中的 proc_pid_statm() 函数。
SHR shared
RES resident
VIRT mm->total_vm
CODE code
DATA data

#### maps




#### smaps

01cbe000-01da9000 r--p 016be000 ca:01 68320258   /usr/sbin/mysqld
Size:                940 kB
Rss:                 236 kB
Pss:                 236 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:       236 kB
Referenced:           76 kB
Anonymous:           236 kB
AnonHugePages:         0 kB
Swap:                  0 kB
KernelPageSize:        4 kB
MMUPageSize:           4 kB
Locked:                0 kB
VmFlags: rd mr mw me dw ac

上述的输出对应 fs/proc/task_mmu.c 文件中的 show_smap() 函数，该函数

show_smap()
 |-walk_page_range()

Linux 内核中，struct vm_area_struct (include/linux/mm_types.h) 是关于虚存管理的最基本单元，描述了一段连续的、具有相同访问属性的虚存空间，该虚存空间的大小为物理内存页面的整数倍，范围是 [vm_start, vm_end) 。


struct vm_area_struct {
  unsigned long vm_start;
  unsigned long vm_end;
  struct vm_area_struct *vm_next, *vm_prev;
  struct rb_node vm_rb;

由于进程所使用到的虚存空间不连续，且访问属性也不同，所以一般一个进程的虚存空间需要多个 vm_area_struct 描述，数目少时通过双向链表组织，否则通过红黑树。


　　假如该vm_area_struct描述的是一个文件映射的虚存空间，成员vm_file便指向被映射的文件的file结构，vm_pgoff是该虚存空间起始地址在vm_file文件里面的文件偏移，单位为物理页面。

　　一个程序可以选择MAP_SHARED或MAP_PRIVATE共享模式将一个文件的某部分数据映射到自己的虚存空间里面。这两种映射方式的区别在于：MAP_SHARED映射后在内存中对该虚存空间的数据进行修改会影响到其他以同样方式映射该部分数据的进程，并且该修改还会被写回文件里面去，也就是这些进程实际上是在共用这些数据。而MAP_PRIVATE映射后对该虚存空间的数据进行修改不会影响到其他进程，也不会被写入文件中。

　　来自不同进程，所有映射同一个文件的vm_area_struct结构都会根据其共享模式分别组织成两个链表。链表的链头分别是：vm_file->f_dentry->d_inode->i_mapping->i_mmap_shared,vm_file->f_dentry->d_inode->i_mapping->i_mmap。而vm_area_struct结构中的vm_next_share指向链表中的下一个节点；vm_pprev_share是一个指针的指针，它的值是链表中上一个节点（头节点）结构的vm_next_share（i_mmap_shared或i_mmap）的地址。

　　进程建立vm_area_struct结构后，只是说明进程可以访问这个虚存空间，但有可能还没有分配相应的物理页面并建立好页面映射。在这种情况下，若是进程执行中有指令需要访问该虚存空间中的内存，便会产生一次缺页异常。这时候，就需要通过vm_area_struct结构里面的vm_ops->nopage所指向的函数来将产生缺页异常的地址对应的文件数据读取出来。

　　vm_flags主要保存了进程对该虚存空间的访问权限，然后还有一些其他的属性。vm_page_prot是新映射的物理页面的页表项pgprot的默认值。



flag 对应了 struct vm_area_struct (include/linux/mm_types.h) 中的 vm_flags ，例如 VM_READ、VM_WRITE、VM_EXEC 等。

关于mm_struct和vm_area_struct的关系可以参考
http://csapp.cs.cmu.edu/3e/ics3/vm/linuxvm.pdf

### 内核中常见操作

1. 给定一个属于某个进程的虚拟地址，要求找到其所属的区间以及 vma_area_struct 结构，该功能是由 find_vma()@mm/mmap.c 中实现。

进程加载过程
http://www.cnblogs.com/web21/p/6222547.html


Linux 中每个页通过 page frame number, PFN 唯一定义。

Linux内存的通解

http://www.tldp.org/LDP/tlk/mm/memory.html



#### statm


关于内存极其推荐的两篇文章 [What every programmer should know about memory](https://www.akkadia.org/drepper/cpumemory.pdf) 以及 [Memory Part1-Part6](https://techtalk.intersec.com/2013/07/memory-part-1-memory-types/)


top + g3 查看内存使用情况，包括了 ```%MEM, VIRT, RES, CODE, DATA, SHR, nMaj, nDRT``` 列，这些信息都是从 /proc/PID/statm 文件中读取的。

size (mapped to VIRT), resident (mapped to RES), share (mapped to SHR), text (mapped to CODE), lib (always 0 on Linux 2.6+), data (mapped to DATA) and dt (always 0 on Linux 2.6+, mapped to nDrt).

http://jzhihui.iteye.com/blog/1447570
http://spockwangs.github.io/2011/08/20/loading-running-and-termination-of-linux-program.html
http://www.cnblogs.com/vampirem/archive/2013/05/30/3108973.html
http://time-track.cn/get-libs-the-process-use.html
http://www.penglixun.com/tech/system/the_diffrents_of_page_cache_and_buffer_cache.html
http://www.cnblogs.com/emperor_zark/archive/2013/03/15/linux_page_1.html
http://blog.csdn.net/mihouge/article/details/6936099
https://techtalk.intersec.com/2013/07/memory-part-1-memory-types/
https://www.thomas-krenn.com/en/wiki/Linux_Page_Cache_Basics
http://www.cnblogs.com/bravery/archive/2012/06/27/2560611.html
http://www.360doc.com/content/12/0925/21/1072296_238157352.shtml
http://lzz5235.github.io/2014/12/10/toolsvmpage-typesc.html
http://www.greenend.org.uk/rjk/tech/dataseg.html
http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html


kpagecount
kpageflags
pagetypeinfo ?
slabinfo
swaps
vmallocinfo
vmstat
zoneinfo

-->





### buddyinfo

是 Linux Buddy 系统管理物理内存的 debug 信息，为了解决物理内存的碎片问题，其把所有空闲的内存，以 2 的幂次方的形式，分成 11 个块链表，分别对应为 1、2、4、8、16、32、... ... 1024 个页块。

Linux 支持 NUMA 技术，NUMA 系统的结点通常是由一组 CPU 和本地内存组成，每个节点都有相应的本地内存，z在 buddyinfo 中通过 Node N 表示一个 NUMA 节点，如果硬件不支持 NUMA，则只有 Node 0。

而每个节点下的内存设备，又可以划分为多个内存区域 (zone)，因此下面的显示中，对于 Node 0 的内存，又划分类 DMA、DMA32、Normal，部分还有 HighMem 区域。

而每行的数字表示上述 11 个链表连续空闲的区域的大小，例如 Normal 中的第二列表示连续两个内存的大小为 ```3506*2*PAGE_SIZE```。

{% highlight text %}
# cat /proc/buddyinfo
Node 0, zone      DMA      2      2      2      1      3      2      0      0      1      1      3
Node 0, zone    DMA32    118    156    996    790    254     88     47     13      0      0      0
Node 0, zone   Normal   2447   3506    613     31      0      0      0      0      0      0      0
{% endhighlight %}









在 Linux 内核启动时，会打印类似如下的内存初始化信息，由于此时内核的引导过程未完成，initrd 以及初始化占用的内存未释放，所以最终内核可用的内存要更大一些。

{% highlight text %}
----- 查看内核打印的内存信息，包括释放的内存
$ dmesg | grep -E "(Memory:|Freeing)"
[    0.000000] Memory: 5428672k/8970240k available (6765k kernel code,
                       686640k absent, 309176k reserved, 4432k data, 1680k init)
[    0.008667] Freeing SMP alternatives: 28k freed
[    0.728176] Freeing initrd memory: 27652k freed
[    2.757835] Freeing unused kernel memory: 1680k freed

----- 计算上述打印的可用内存
$ echo "5428672 + 28 + 27652 + 1680" | bc
5458032

----- 通过free命令查看当前内存
$ free -k
              total        used        free      shared  buff/cache   available
Mem:        8070604     2425256      714864      778268     4930484     4515144

5428672k/8970240k available
    分母表示可用物理内存的大小；
    分子表示可供kernel分配的free memory的大小；

absent：
  不可用的物理内存大小，包括了 BIOS 保留、Kernel 不可用的物理内存；
{% endhighlight %}

<!--

    330548k reserved：
    包括【initrd】和【内核代码及数据】等，详见上面的解释。其中内核代码和部分数据包含在下列统计值中：

        kernel code
          表示kernel的代码，属于reserved memory；
        data
          表示kernel的数据，属于reserved memory；
        init
          表示init code和init data，属于reserved memory，但引导完成之后会释放给free memory；

    available = 物理内存 - absent - reserved
    reserved 包括 kernel code, data, init，还有 initrd 和更多其它的内容；
-->

物理内存可以通过 ```dmidecode --type 17``` 查看，包括了很多详细的物理信息，不过对于虚机来说不支持。


## 其它

### sysrq

Linux 中，有一个非常强大的 sysrq 功能，通过该功能可以查看一些内核的当前运行信息，该信息会打印信息到内核的环形缓冲并输出到系统控制台，一般也会通过 syslog 输出到 ```/var/log/messages``` 。

有些发行版本该功能是关闭的，可以通过如下方式打开。

{% highlight text %}
# echo 1 > /proc/sys/kernel/sysrq           // 打开
# echo 0 > /proc/sys/kernel/sysrq           // 关闭
# vi /etc/sysctl.conf                       // 设置永久生效
kernel.sysrq = 1
{% endhighlight %}

一些常见的功能可以参考如下内容。

{% highlight text %}
# echo "b" > /proc/sysrq-trigger            // 立即重新启动计算机
# echo "o" > /proc/sysrq-trigger            // 立即关闭计算机
# echo "m" > /proc/sysrq-trigger            // 导出内存分配的信息到demsg
# echo "p" > /proc/sysrq-trigger            // 导出当前CPU寄存器信息和标志位的信息
# echo "t" > /proc/sysrq-trigger            // 导出线程状态信息
# echo "c" > /proc/sysrq-trigger            // 故意让系统崩溃
# echo "s" > /proc/sysrq-trigger            // 立即重新挂载所有的文件系统
# echo "u" > /proc/sysrq-trigger            // 立即重新挂载所有的文件系统为只读
{% endhighlight %}


### mtrace

该工具可以用来协助定位内存泄露，默认没有安装，在 CentOS 中可以通过如下方式安装。

{% highlight text %}
# yum install glibc-utils
{% endhighlight %}

假设有如下的测试代码。


{% highlight text %}
$ cat main.c
#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>
int main(int argc, char **argv)
{
    setenv("MALLOC_TRACE", "taoge.log", 1);
    mtrace();

    malloc(2 * sizeof(int));

    return 0;
}

$ gcc -Wall -g -o foobar main.c
$ ./foobar
$ mtrace foobar trace.log
{% endhighlight %}

可以看到，有内存泄露，且正确定位到了代码的行数。实际上，mtrace 原理很简单，就是记录每一对 malloc/free 的调用情况，然后检查是否有内存没有释放。




<!--
// int mallopt(int param, int value);
// mtrace muntrace mcheck mcheck_pedantic mcheck_check_all mprobe
// malloc_stats mallinfo malloc_trim malloc_info

答：brk是系统调用，主要工作是实现虚拟内存到内存的映射，可以让进程的堆指针增长一定的大小，逻辑上消耗掉一块虚拟地址空间，malloc向OS获取的内存大小比较小时，将直接通过brk调用获取虚拟地址。

mmap是系统调用，也是实现虚拟内存到内存的映射，可以让进程的虚拟地址区间切分出一块指定大小的虚拟地址空间vma_struct，一个进程的所有动态库文件.so的加载，都需要通过mmap系统调用映射指定大小的虚拟地址区间，被mmap映射返回的虚拟地址，逻辑上被消耗了，直到用户进程调用unmap，会回收回来。malloc向系统获取比较大的内存时，会通过mmap直接映射一块虚拟地址区间。

malloc是C语言标准库中的函数，主要用于申请动态内存的分配，其原理是当堆内存不够时，通过brk/mmap等系统调用向内核申请进程的虚拟地址区间，如果堆内部的内存能满足malloc调用，则直接从堆里获取地址块返回。

new是C++内置操作符，用于申请动态内存的分配，并同时进行初始化操作。其实现会调用malloc，对于基本类型变量，它只是增加了一个cookie结构, 比如需要new的对象大小是 object_size, 则事实上调用 malloc 的参数是 object_size + cookie， 这个cookie 结构存放的信息包括对象大小，对象前后会包含两个用于检测内存溢出的变量，所有new申请的cookie块会链接成双向链表。
对于自定义类型，new会先申请上述的大小空间，然后调用自定义类型的构造函数，对object所在空间进行构造。
-->







ptmalloc







15600615832


## 内存分配



## Page Allocator



### alloc_page()

通过 ```alloc_page()``` 函数完成页的分配，在使用时需要通过 ```page_address()``` 完成线性地址转换，详细可以参考 [alloc.c]({{ site.example_repository }}/linux/LKM/memory/alloc.c) 示例程序。

内核通过 struct page 描述一个页，所有的页描述符存放在全局 mem_map 数组中，每个 page 代表一个物理页面，整个数组代表着系统中的全部物理页面，数组的下标为页框号 (pfn)，代表了 page 结构对应第几个物理页面。

页面表项的高 20 位对于软件和 MMU 硬件有着不同的意义，对于软件，这是一个物理页面的序号，将这个序号用作下标就可以从 mem_map 找到代表这个物理页面的 page 数据结构，对于硬件，则 (在低位补上12个0后) 就是物理页面的起始地址。

根据是否配置了 ```CONFIG_NUMA``` 会调用不同的函数，一般来说很多发行版本都会配置该选项。

{% highlight c %}
#ifdef CONFIG_NUMA
static inline struct page *
alloc_pages(gfp_t gfp_mask, unsigned int order)
{
    return alloc_pages_current(gfp_mask, order);
}
#else
#define alloc_pages(gfp_mask, order) \
        alloc_pages_node(numa_node_id(), gfp_mask, order)
#endif
{% endhighlight %}

接下来看看实现的详细细节。

{% highlight text %}
alloc_pages()
 |-alloc_pages_current()
   |-get_task_policy()                  获取内存的分配策略
   |-__alloc_pages_nodemask()           buddy内存分配系统的核心部分
     |-first_zones_zonelist()
     |-get_page_from_freelist()         快速分配内存
     | |-zone_watermark_ok()            判断设置的内存水位
     | |-buffered_rmqueue()             从zone中分配order阶的页帧
     |   |-rmqueue_bulk()               对于单页分配
     |     |-__rmqueue()                从伙伴系统中分配指定的页
     |     | |-__rmqueue_smallest()
     |     | | |-expand()               处理请求页小于当前页的情况
     |     | |   |-list_add()           将页一分为二，并添加到链表中
     |     | |-__rmqueue_fallback()
     |     |-__mod_zone_page_state()    更新页的统计状态
     |
     |-__alloc_pages_slowpath()         慢速分配
{% endhighlight %}




<!--

Linux进程的线性地址空间（进程虚拟地址空间分布），0~3G是User地址空间，3~4G是Kernel地址空间。(适用于ARM、X86等，mips按0~2G,2~3G划分)

关于线性地址布局，此图未说明部分：

1.紧接着内核数据区向上是mem_map全局page数组。

2. kernel启动地址并非是0xC0000000，而是PAGE_OFFSET+TEXT_OFFSET（0x8000），而在这32k大小的空间存放着内核一级页表数组swapper_pg_dir（每一项一级页表4个字节，映射1M内存，4G空间共4K项一级页表，占用16k内存），页表在启动阶段setup_arch->paging_init中调用prepare_page_table()和map_lowmem()进行初始化和映射，将在另一篇详细叙述。

3.内核动态加载驱动模块so将被load到紧接着MODULES_VADDR~0xC0000000的16M空间。


小于896M的物理内存直接映射到内核3G~3G+896M线性地址空间内，VA=PA-PHYS_OFFSET+PAGE_OFFSET

大约896M的物理内存通过建立各级页表的方式映射到高端映射区。

每个task在起task_struct内都有一个指针mm指向mm_struct，其控制着该task的所有内存信息，其成员mmap指向vma区链表，pgd指向页全局目录位置。

切换任务时要为下一个task装载其pgd到C2（X86为cr3）。

在cpu寻址时，mmu将虚拟地址按照2级映射方式，映射到对应物理叶匡的地址偏移上去。

而在内存管理上，系统启动阶段使用的是Bootmem，它是以简单的bitmap方式（0：未使用，1：已用）。

系统启动后buddy接手内存管理（以页为单位），分为两种情况，per_cpu_pageset和free_area，前者用来分配单个页（在Linux物理内存描述三个层级中有详细描述），后者用来分配多页的情况，11个链表管理页块的大小从2^1~2^11，每个链表中又分为不同迁移类型的子链表。

#define MIGRATE_UNMOVABLE     0
#define MIGRATE_RECLAIMABLE   1
#define MIGRATE_MOVABLE       2
#define MIGRATE_PCPTYPES      3 /* the number of types on the pcp lists */
#define MIGRATE_RESERVE       3
#define MIGRATE_ISOLATE       4 /* can not allocate from here */

MIGRATE_TYPES是为了有效解决内存碎片问题而引入的，将可移动（用户空间申请的内存可以重新映射）、不可移动（内核空间通常申请的）、可回收（文件映射）等组织在不同的链表中，申请内存时按照不同情况在各自类型的链表中释放，如果所在类型内存不足，会fallback到其他类型链表继续分配。在系统初始化完成时全部内存划分给MOVABLE类型链表，当有其他类型需求时再从MOVABLE中释放生成。

slab分配器是面向对象的分配机制，其建立在buddy基础之上，细节待另一篇详述。
-->


## Buddy System

## Slub

## 缺页异常

可以通过 ```ps -o majflt,minflt -p PID``` 查看进程从启动开始缺页异常的次数，两个参数分别代表了 ```major fault``` 和 ```minor fault``` 。

发成缺页中断后，会进入内核空间，然后执行如下操作。

1. 检查要访问的虚拟地址是否合法。
2. 查找/分配一个物理页。
3. 填充物理页内容（读取磁盘，或者直接置0，或者啥也不干）。
4. 建立映射关系（虚拟地址到物理地址）。
5. 重新执行发生缺页中断的那条指令

如果第 3 步填充空间时，需要读取磁盘，那么这次缺页中断就是 ```majflt```，否则就是 ```minflt``` 。


## page cache

page cache 也就是页高速缓存器，其大小为一页，通常为 4K ，在 Linux 读写文件时，用于缓存文件的逻辑内容，从而加快对磁盘上映像和数据的访问。

<!--
在从外存的一页到内存的一页的映射过程中，page cache与buffer cache、swap cache共同实现了高速缓存功能，以下是其简单映射图，
外存的一页(分解为几块，可能不连续)
|
物理磁盘的磁盘块
|
内存的buffer cache
|
内存的一页(由一个页框划分的几个连续buffer cache构成)
|
页高缓系统
在这个过程中，内存管理系统和VFS与page cache交互，内存管理系统负责维护每项page cache的分配和回收，同时在使用memory map方式访问时负责建立映射；VFS负责page cache与用户空间的数据交换。
-->

三、page cache的管理

在Linux内核中，文件的每个数据块最多只能对应一个page cache项，它通过两个数据结构来管理这些cache项，一个是radix tree，另一个是双向链表。
Radix tree是一种搜索树，Linux内核利用这个数据结构，快速查找脏的(dirty)和回写的（writeback）页面，得到其文件内偏移，从而对page cache进行快速定位。图1是radix tree的一个示意图，该radix tree的分叉为4(22)，树高为4，用来快速定位8位文件内偏移。
另一个数据结构是双向链表，Linux内核为每一片物理内存区域(zone)维护active_list和 inactive_list两个双向链表，这两个list主要用来实现物理内存的回收。这两个链表上除了文件Cache之外，还包括其它匿名 (Anonymous)内存，如进程堆栈等。

四、page cache相关API及其实现
Linux内核中与文件Cache操作相关的API有很多，按其使用方式可以分成两类：一类是以拷贝方式操作的相关接口，如read/write/sendfile等，其中sendfile在2.6系列的内核中已经不再支持；另一类是以地址映射方式操作的相关接口，如mmap等。
第一种类型的API在不同文件的Cache之间或者Cache与应用程序所提供的用户空间buffer之间拷贝数据，其实现原理如图2所示。
第二种类型的API将Cache项映射到用户空间，使得应用程序可以像使用内存指针一样访问文件，Memory map访问Cache的方式在内核中是采用请求页面机制实现的，其工作过程如图3所示。

首先，应用程序调用mmap（图中1），陷入到内核中后调用do_mmap_pgoff（图中2）。该函数从应用程序的地址空间中分配一段区域作为映射的内存地址，并使用一个VMA（vm_area_struct）结构代表该区域，之后就返回到应用程序（图中3）。当应用程序访问mmap所返回的地址指针时（图中4），由于虚实映射尚未建立，会触发缺页中断（图中5）。之后系统会调用缺页中断处理函数（图中6），在缺页中断处理函数中，内核通过相应区域的 VMA结构判断出该区域属于文件映射，于是调用具体文件系统的接口读入相应的Page Cache项（图中7、8、9），并填写相应的虚实映射表。经过这些步骤之后，应用程序就可以正常访问相应的内存区域了。


add_page_to_hash_queue /*加入pache cache hash表*/
add_page_to_inode_queue /*加入inode queue即address_space*/
remove_page_from_inode_queue
remove_page_from_hash_queue
__remove_inode_page /*离开inode queue和hash 表*/
remove_inode_page /*同上*/
add_to_page_cache_locked /*加入inode queue,hash 和lru cache*/
__add_to_page_cache /*同上*/
仅罗列函数add_page_to_hash_queue,以示完整:<FONT
color=green>
static void add_page_to_hash_queue(struct page * page, struct page **p)
{
    struct page *next = *p;
    *p = page; /* page->newNode */
    page->next_hash = next; /* +-----+ */
    page->pprev_hash = p; /* p--> |hashp|-->|oldNode| */
    if (next) /* next----+ */
        next->pprev_hash = &page->next_hash;
        if (page->buffers)
            PAGE_BUG(page); /*证明page 不会同时存在于page cache
            和 buffer cache*/
            /*2.6 已经与此不同了*/
            atomic_inc(&page_cache_size);
}</FONT

page cache 在代码中又称 inode page cache, 足以显示page cache 和inode紧密关联.加入page cache 和加入inode cache是同一个意思.加入page cache意味着同时加入page cache hash表和inode queue(也建立了page和addr sapce的关系). 见函数add_to_page_cache_locked,__add_to_page_cache即可取证.从page cache 删除在程序中叫__remove_inode_page,再次显示inode 和page cache的"一体化".



1、 page cache及swap cache中页面的区分：一个被访问文件的物理页面都驻留在page cache或swap cache中，一个页面的所有信息由struct page来描述。struct page中有一个域为指针mapping ，它指向一个struct address_space类型结构。page cache或swap cache中的所有页面就是根据address_space结构以及一个偏移量来区分的。

2、文件与 address_space结构的对应：一个具体的文件在打开后，内核会在内存中为之建立一个struct inode结构，其中的i_mapping域指向一个address_space结构。这样，一个文件就对应一个address_space结构，一个 address_space与一个偏移量能够确定一个page cache 或swap cache中的一个页面。因此，当要寻址某个数据时，很容易根据给定的文件及数据在文件内的偏移量而找到相应的页面。

3、进程调用mmap()时，只是在进程空间内新增了一块相应大小的缓冲区，并设置了相应的访问标识，但并没有建立进程空间到物理页面的映射。因此，第一次访问该空间时，会引发一个缺页异常。

4、 对于共享内存映射情况，缺页异常处理程序首先在swap cache中寻找目标页（符合address_space以及偏移量的物理页），如果找到，则直接返回地址；如果没有找到，则判断该页是否在交换区 (swap area)，如果在，则执行一个换入操作；如果上述两种情况都不满足，处理程序将分配新的物理页面，并把它插入到page cache中。进程最终将更新进程页表。
注：对于映射普通文件情况（非共享映射），缺页异常处理程序首先会在page cache中根据address_space以及数据偏移量寻找相应的页面。如果没有找到，则说明文件数据还没有读入内存，处理程序会从磁盘读入相应的页 面，并返回相应地址，同时，进程页表也会更新。

5、所有进程在映射同一个共享内存区域时，情况都一样，在建立线性地址与物理地址之间的映射之后，不论进程各自的返回地址如何，实际访问的必然是同一个共享内存区域对应的物理页面。
注：一个共享内存区域可以看作是特殊文件系统shm中的一个文件，shm的安装点在交换区上。

上面涉及到了一些数据结构，围绕数据结构理解问题会容易一些。


## Cache

{% highlight text %}
BufferCache
  块缓冲，通常1K，对应于一个磁盘块，用于减少磁盘IO

PageCache
  页/文件缓冲，通常4K，由若干个磁盘块组成，物理上不一定连续，也即由若干个BufferCache组成
  读文件：不连续磁盘块 >> BufferCache >> PageCache >> 应用程序进程空间
  写文件：PageCache, BufferCache >> 磁盘

SwapCache
  交换空间。
{% endhighlight %}

### Swap Cache

其作用不是为了提高磁盘的 IO 效率，而是为了解决页面在 swap in 和 swap out 时的同步问题，也就是在进行 swap out (将页面内容写入磁盘分区时) 进程如果发起了对换出页面的访问，系统如何对其处理。

由于存在 swap cache ，如果页面的数据还没有完全写入磁盘，这个 page frame 是在 swap cache 中的；等数据完全写入磁盘后，且没有进程对 page frame 访问时，才会真正释放 page frame，然后将其交给 buddy system 。



## 常用函数

### SetPageReserved()

随着 Linux 的长时间运行，空闲页面会越来越少，为此内核采用页面回收算法 (PFRA) 从用户进程和内核高速缓存中回收内存页框，并根据需要把要回收页框的内容交换到磁盘上的交换区。

<!--
调用该函数可以使页面不被交换。

　　#define SetPageReserved(page) set_bit(PG_reserved, &(page)->flags)

　　PG_reserved 的标志说明如下。

　　* PG_reserved is set for special pages, which can never be swapped out. Some

　　* of them might not even exist (eg empty_bad_page)…

virt_to_phys()
-->


<!--
Linux读写物理内存
http://blog.csdn.net/zsf8701/article/details/7814988
-->













<!---  iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii -->


MySQL 5.7 在进行恢复的时候，一般情况下需要进行最多 3 次的 redo-log 扫描：

第一次redo log的扫描，主要是查找MLOG_CHECKPOINT，不进行redo log的解析，如果没有找到MLOG_CHECKPOINT，则说明InnoDB不需要进行recovery，后面的两次扫描可以省略，如果找到了MLOG_CHECKPOINT，则获取MLOG_FILE_NAME到指定列表，后续只需打开该链表中的表空间即可。

第二次扫描是在第一次找到MLOG_CHECKPOINT基础之上进行的，该次扫描会把redo log解析到哈希表中，如果扫描完整个文件，哈希表还没有被填满，则不需要第三次扫描，直接进行recovery就结束

第三次扫描是在第二次基础上进行的，第二次扫描把哈希表填满后，还有redo log剩余，则需要循环进行扫描，哈希表满后立即进行recovery，直到所有的redo log被apply完为止。

redo log全部被解析并且apply完成，整个InnoDB recovery的第一阶段也就结束了，在该阶段中，所有已经被记录到redo log但是没有完成数据刷盘的记录都被重新落盘。然而，InnoDB单靠redo log的恢复是不够的，这样还是有可能会丢失数据(或者说造成主从数据不一致)，因为在事务提交过程中，写binlog和写redo log提交是两个过程，写binlog在前而redo提交在后，如果MySQL写完binlog后，在redo提交之前发生了宕机，这样就会出现问题：binlog中已经包含了该条记录，而redo没有持久化。binlog已经落盘就意味着slave上可以apply该条数据，redo没有持久化则代表了master上该条数据并没有落盘，也不能通过redo进行恢复。这样就造成了主从数据的不一致，换句话说主上丢失了部分数据，那么MySQL又是如何保证在这样的情况下，数据还是一致的？这就需要进行第二阶段恢复。






在函数 ```recv_recovery_from_checkpoint_start()``` 执行完成之后，实际上就可以开始处理用户的事务了。



MLOG_FILE CHECKPOINT 如何写入？？？如何使用？？？kkkk





mlog 如何记录的那些 ibd 时需要读取的。





InnoDB 在 MySQL 启动的时候，会对 redo-log 进行日志回放，通过 recv_sys_t 结构来进行数据恢复和控制的，它的结构如下：

{% highlight text %}
struct recv_sys_t {

mutex_t     mutex;                                 /*保护锁*/
ibool  apply_log_recs;                        /*正在应用log record到page中*/
ibool     apply_batch_on;                     /*批量应用log record标志*/

dulint  lsn;
ulint  last_log_buf_size;

byte*     last_block;                             /*恢复时最后的块内存缓冲区*/
byte*    last_block_buf_start;             /*最后块内存缓冲区的起始位置，因为last_block是512地址对齐的，需要这个变量记录free的地址位置*/

byte*   buf;                 // 从redo-log文件中读取的日志

ulint  len;    /*buf有效的日志数据长度*/

dulint    parse_start_lsn;                       /*开始parse的lsn*/
dulint   scanned_lsn;                           /*已经扫描过的lsn序号*/

ulint   scanned_checkpoint_no;          /*恢复日志的checkpoint 序号*/
ulint  recovered_offset;                       /*恢复位置的偏移量*/

dulint    recovered_lsn;                         /*恢复的lsn位置*/
dulint   limit_lsn;                                  /*日志恢复最大的lsn,暂时在日志重做的过程没有使用*/

ibool   found_corrupt_log;                   /*是否开启日志恢复诊断*/

log_group_t*  archive_group;

mem_heap_t*   heap;                             /*recv sys的内存分配堆,用来管理恢复过程的内存占用*/

  hash_table_t*   addr_hash;       // 以(space_id+page_no)为KEY的Hash表

ulint   n_addrs;                                        /*addr_hash中包含recv_addr的个数*/

};
{% endhighlight %}

在这个结构中，比较复杂的是addr_hash这个哈希表，这个哈希表是用sapce_id和page_no作为hash key,里面存储有恢复时对应的记录内容。

恢复日志在从日志文件中读出后，进行解析成若干个recv_t并存储在哈希表当中。在一个读取解析周期过后，日志恢复会对hash表中的recv_t中的数据写入到ibuf和page中。这里为什么要使用hash表呢？个人觉得是为了同一个page的数据批量进行恢复的缘故，这样可以page减少随机插入和修改。 以下是和这个过程相关的几个数据结构:


{% highlight text %}
/*对应页的数据恢复操作集合*/
struct recv_addr_t {
     ulint   state;          /*状态，RECV_NOT_PROCESSED、RECV_BEING_PROCESSED、RECV_PROCESSED*/
      ulint  space;         /*space的ID*/
       ulint     page_no;    /*页序号*/
  UT_LIST_BASE_NODE_T(recv_t) rec_list;  // 该页对应的log records地址
         hash_node_t     addr_hash;
};

/*当前的记录操作*/
struct recv_t {
     byte    type;             /*log类型*/
      ulint  len;               /*当前记录数据长度*/
       recv_data_t* data;    /*当前的记录数据list*/

  lsn_t  start_lsn;                 // mtr起始LSN
  lsn_t  end_lsn;                   // mtr结尾LSN
  UT_LIST_NODE_T(recv_t)  rec_list; // 该页对应的log records
};

struct recv_data_t {
  recv_data_t*   next;  // 指向下个结构体，该地址之后为一大块内存，用于存储log record消息体
};
{% endhighlight %}

他们的内存关系结构图如下： \
2.重做日志推演过程的LSN关系
除了这个恢复的哈希表以外，recv_sys_t中的各种LSN也是和日志恢复有非常紧密的关系。以下是各种lsn的解释： parse_start_lsn 本次日志重做恢复起始的lsn，如果是从checkpoint处开始恢复，等于checkpoint_lsn。 scanned_lsn 在恢复过程，将恢复日志从log_sys->buf解析块后存入recv_sys->buf的日志lsn. recovered_lsn 已经将数据恢复到page中或者已经将日志操作存储addr_hash当中的日志lsn; 在日志开始恢复时：
parse_start_lsn = scanned_lsn = recovered_lsn = 检查点的lsn。
在日志完成恢复时:
parse_start_lsn = 检查点的lsn
scanned_lsn = recovered_lsn = log_sys->lsn。
在日志推演过程中lsn大小关系如下：
\
3.日志恢复的主要接口和流程
恢复日志主要的接口函数: recv_recovery_from_checkpoint_start 从重做日志组内的最近的checkpoint开始恢复数据
recv_recovery_from_checkpoint_finish 结束从重做日志组内的checkpoint的数据恢复操作
recv_recovery_from_archive_start 从归档日志文件中进行数据恢复
recv_recovery_from_archive_finish 结束从归档日志中的数据恢复操作
recv_reset_logs 截取重做日志最后一段作为新的重做日志的起始位置，可能会丢失数据。
重做日志恢复数据的流程(checkpoint方式) 1.当MySQL启动的时候，先会从数据库文件中读取出上次保存最大的LSN。
2.然后调用recv_recovery_from_checkpoint_start，并将最大的LSN作为参数传入函数当中。
3.函数会先最近建立checkpoint的日志组，并读取出对应的checkpoint信息
4.通过checkpoint lsn和传入的最大LSN进行比较，如果相等，不进行日志恢复数据，如果不相等，进行日志恢复。
5.在启动恢复之前，先会同步各个日志组的archive归档状态
6.在开始恢复时，先会从日志文件中读取2M的日志数据到log_sys->buf，然后对这2M的数据进行scan,校验其合法性，而后将去掉block header的日志放入recv_sys->buf当中，这个过程称为scan,会改变scanned lsn.
7.在对2M的日志数据scan后，innodb会对日志进行mtr操作解析，并执行相关的mtr函数。如果mtr合法，会将对应的记录数据按space page_no作为KEY存入recv_sys->addr_hash当中。
8.当对scan的日志数据进行mtr解析后，innodb对会调用recv_apply_hashed_log_recs对整个recv_sys->addr_hash进行扫描，并按照日志相对应的操作进行对应page的数据恢复。这个过程会改变recovered_lsn。
9.如果完成第8步后，会再次从日志组文件中读取2M数据，跳到步骤6继续相对应的处理，直到日志文件没有需要恢复的日志数据。
10.innodb在恢复完成日志文件中的数据后，会调用recv_recovery_from_checkpoint_finish结束日志恢复操作，主要是释放一些开辟的内存。并进行事务和binlog的处理。
上面过程的示意图如下：





GTID的全称为 global transaction identifier  ， 可以翻译为全局事务标示符，GTID在原始master上的事务提交时被创建。GTID需要在全局的主-备拓扑结构中保持唯一性，GTID由两部分组成：
GTID = source_id:transaction_id

source_id用于标示源服务器，用server_uuid来表示，这个值在第一次启动时生成，并写入到配置文件data/auto.cnf中
transaction_id则是根据在源服务器上第几个提交的事务来确定。

一个GTID的生命周期包括：
1.事务在主库上执行并提交
给事务分配一个gtid（由主库的uuid和该服务器上未使用的最小事务序列号），该GTID被写入到binlog中。
2.备库读取relaylog中的gtid，并设置session级别的gtid_next的值，以告诉备库下一个事务必须使用这个值
3.备库检查该gtid是否已经被其使用并记录到他自己的binlog中。slave需要担保之前的事务没有使用这个gtid，也要担保此时已分读取gtid，但未提交的事务也不恩呢过使用这个gtid.
4.由于gtid_next非空，slave不会去生成一个新的gtid，而是使用从主库获得的gtid。这可以保证在一个复制拓扑中的同一个事务gtid不变。

由于GTID在全局的唯一性，通过GTID，我们可以在自动切换时对一些复杂的复制拓扑很方便的提升新主库及新备库，例如通过指向特定的GTID来确定新备库复制坐标。

当然，使用GTID也有一些限制：
1.事务中的更新包含非事务性存储引擎，这可能导致多个GTID分配给同一个事务。
2. create table…select语句不被支持，因为该语句会被拆分成create table 和insert两个事务，并且这个两个事务被分配了同一个GTID，这会导致insert被备库忽略掉。
3.不支持CREATE/DROP临时表操作

可以看到，支持GTID的复制对一些语句都有一些限制，MySQL也提供了一个选项disable-gtid-unsafe-statements以禁止这些语句的执行。




### ibd页损坏
Page损坏的情况比较多：二级索引页损坏(可以通过OPTIMIZE TABLE恢复)；聚集索引页损坏；表字典损坏；
----- 使用VIM编译
vim -b titles.ibd
----- 通过外部xxd程序改变，注意修改完之后一定要通过-r选项恢复为二进制格式
:%!xxd
:%!xxd -r
当执行check table titles;检查表是否正常时，就会由于页面错误退出，报错信息如下。

buf_page_io_complete()
2017-03-09T08:58:34.750125Z 4 [ERROR] InnoDB: Database page corruption on disk or a failed file read of page [page id: space=NUM, page number=NUM]. You may have to recover from a backup.
len 16384; hex ... ...
InnoDB: End of page dump
InnoDB: Page may be an index page where index id is 48
2017-03-09T08:58:34.804632Z 4 [ERROR] [FATAL] InnoDB: Aborting because of a corrupt database page in the system tablespace. Or,  there was a failure in tagging the tablespace  as corrupt.
2017-03-09 16:58:34 0x7fe6e87b7700  InnoDB: Assertion failure in thread 140629719611136 in file ut0ut.cc line 916

实际上是可以正常重启的，但是一旦查询到该页时，仍然会报错，接下来看看如何恢复其中还完好的数据。

修改my.ini中的innodb_force_recovery参数，默认是0，此时可以修改为1-6，使mysqld在启动时跳过部分恢复步骤，在启动后将数据导出来然后重建数据库；当然，不同的情况可能恢复的数据会有所不同。

1. SRV_FORCE_IGNORE_CORRUPT: 忽略检查到的corrupt页；**
2. SRV_FORCE_NO_BACKGROUND): 阻止主线程的运行，在srv_master_thread()中处理；**

3. SRV_FORCE_NO_TRX_UNDO):不执行事务回滚操作。
4. SRV_FORCE_NO_IBUF_MERGE):不执行插入缓冲的合并操作。
5. SRV_FORCE_NO_UNDO_LOG_SCAN):不查看重做日志，InnoDB存储引擎会将未提交的事务视为已提交。

6. SRV_FORCE_NO_LOG_REDO): 不执行redo log。

另外，需要注意，设置参数值大于0后，可以对表进行select,create,drop操作，但insert,update或者delete这类操作是不允许的。

再次执行check table titles;时，会报如下的错误。
+------------------+-------+----------+---------------------------------------------------+
| Table            | Op    | Msg_type | Msg_text                                          |
+------------------+-------+----------+---------------------------------------------------+
| employees.titles | check | Warning  | InnoDB: The B-tree of index PRIMARY is corrupted. |
| employees.titles | check | error    | Corrupt                                           |
+------------------+-------+----------+---------------------------------------------------+
2 rows in set (2.47 sec)

SELECT * FROM titles INTO OUTFILE '/tmp/titles.csv'
   FIELDS TERMINATED BY ',' ENCLOSED BY '"'
   LINES TERMINATED BY '\r\n';
注意上述在使用LOAD DATA INFILE或者SELECT INTO OUTFILE操作时，需要在配置文件中添加secure_file_priv=/tmp配置项，或者配置成secure_file_priv="/"不限制导入和导出路径。
如何确认哪些数据丢失了？？？？

http://www.runoob.com/mysql/mysql-database-export.html

如下是导致MySQL表毁坏的常见原因：
1、 服务器突然断电或者强制关机导致数据文件损坏；
2、 mysqld进程在修改表时被强制杀掉，例如kill -9；
3、 磁盘故障、服务器宕机等硬件问题无法恢复；
4、 使用myisamchk的同时，mysqld也在操作表；
5、 MySQL、操作系统、文件系统等软件的bug。




表损坏的典型症状：
 　　　　1 、当在从表中选择数据之时，你得到如下错误： 

　　　　　　Incorrect key file for table: '...'. Try to repair it

 　　　　2 、查询不能在表中找到行或返回不完全的数据。

　　　　 3 、Error: Table 'p' is marked as crashed and should be repaired 。

　　　　 4 、打开表失败： Can’t open file: ‘×××.MYI’ (errno: 145) 。

  ER_NOT_FORM_FILE  Incorrect information in file:
  ER_NOT_KEYFILE    Incorrect key file for table 'TTT'; try to repair it
  ER_OLD_KEYFILE    Old key file for table 'TTT'; repair it!
  ER_CANT_OPEN_FILE Can't open file: 'TTT' (errno: %d - %s)
 

　　3.预防 MySQL 表损坏

 　　可以采用以下手段预防mysql 表损坏： 

　　　　1 、定期使用myisamchk 检查MyISAM 表（注意要关闭mysqld ），推荐使用check table 来检查表（不用关闭mysqld ）。

　　　　2 、在做过大量的更新或删除操作后，推荐使用OPTIMIZE TABLE 来优化表，这样既减少了文件碎片，又减少了表损坏的概率。

　　　　3 、关闭服务器前，先关闭mysqld （正常关闭服务，不要使用kill -9 来杀进程）。

　　　　4 、使用ups 电源，避免出现突然断电的情况。

　　　　5 、使用最新的稳定发布版mysql ，减少mysql 本身的bug 导致表损坏。

　　　　6 、对于InnoDB 引擎，你可以使用innodb_tablespace_monitor来检查表空间文件内文件空间管理的完整性。

　　　　7 、对磁盘做raid ，减少磁盘出错并提高性能。

　　　　8 、数据库服务器最好只跑mysqld 和必要的其他服务，不要跑其他业务服务，这样减少死机导致表损坏的可能。

　　　　9 、不怕万一，只怕意外，平时做好备份是预防表损坏的有效手段。

　　4. MySQL 表损坏的修复

　　MyISAM 表可以采用以下步骤进行修复 ：

　　　　1、  使用 reapair table 或myisamchk 来修复。 

　　　　2、  如果上面的方法修复无效，采用备份恢复表。



　　具体可以参考如下做法：

　　阶段1 ：检查你的表

　　　　如果你有很多时间，运行myisamchk *.MYI 或myisamchk -e *.MYI 。使用-s （沉默）选项禁止不必要的信息。 

　　　　如果mysqld 服务器处于宕机状态，应使用--update-state 选项来告诉myisamchk 将表标记为' 检查过的' 。

　　　　你必须只修复那些myisamchk 报告有错误的表。对这样的表，继续到阶段2 。

　　　　如果在检查时，你得到奇怪的错误( 例如out of memory 错误) ，或如果myisamchk 崩溃，到阶段3 。

　　阶段2 ：简单安全的修复

　　　　注释：如果想更快地进行修复，当运行myisamchk 时，你应将sort_buffer_size 和Key_buffer_size 变量的值设置为可用内存的大约25% 。

　　　　首先，试试myisamchk -r -q tbl_name(-r -q 意味着“ 快速恢复模式”) 。这将试图不接触数据文件来修复索引文件。如果数据文件包含它应有的一切内容和指向数据文件内正确地点的删除连接，这应该管用并且表可被修复。开始修复下一张表。否则，执行下列过程：

　　　　在继续前对数据文件进行备份。

　　　　使用myisamchk -r tbl_name(-r 意味着“ 恢复模式”) 。这将从数据文件中删除不正确的记录和已被删除的记录并重建索引文件。

　　　　如果前面的步骤失败，使用myisamchk --safe-recover tbl_name 。安全恢复模式使用一个老的恢复方法，处理常规恢复模式不行的少数情况( 但是更慢) 。

　　　　如果在修复时，你得到奇怪的错误( 例如out of memory 错误) ，或如果myisamchk 崩溃，到阶段3 。 

　　阶段3 ：困难的修复

　　　　只有在索引文件的第一个16K 块被破坏，或包含不正确的信息，或如果索引文件丢失，你才应该到这个阶段。在这种情况下，需要创建一个新的索引文件。按如下步骤操做：

　　　　把数据文件移到安全的地方。

　　　　使用表描述文件创建新的( 空) 数据文件和索引文件：

　　　　shell> mysql db_name

　　　　mysql> SET AUTOCOMMIT=1;

　　　　mysql> TRUNCATE TABLE tbl_name;

　　　　mysql> quit 

　　　　如果你的MySQL 版本没有TRUNCATE TABLE ，则使用DELETE FROM tbl_name 。

　　　　将老的数据文件拷贝到新创建的数据文件之中。（不要只是将老文件移回新文件之中；你要保留一个副本以防某些东西出错。）

　　　　回到阶段2 。现在myisamchk -r -q 应该工作了。（这不应该是一个无限循环）。

　　　　你还可以使用REPAIR TABLE tbl_name USE_FRM ，将自动执行整个程序。

　　阶段4 ：非常困难的修复

　　　　只有.frm 描述文件也破坏了，你才应该到达这个阶段。这应该从未发生过，因为在表被创建以后，描述文件就不再改变了。

 　　　　从一个备份恢复描述文件然后回到阶段3 。你也可以恢复索引文件然后回到阶段2 。对后者，你应该用myisamchk -r 启动。

　　　　如果你没有进行备份但是确切地知道表是怎样创建的，在另一个数据库中创建表的一个拷贝。删除新的数据文件，然后从其他数据库将描述文件和索引文件移到破坏的数据库中。这样提供了新的描述和索引文件，但是让.MYD 数据文件独自留下来了。回到阶段2 并且尝试重建索引文件。
Log Sequence Number, LSN

Sharp Checkpoint 是一次性将 buffer pool 中的所有脏页都刷新到磁盘的数据文件，同时会保存最后一个提交的事务LSN。


fuzzy checkpoint就更加复杂了，它是在固定的时间点发生，除非他已经将所有的页信息刷新到了磁盘，或者是刚发生过一次sharp checkpoint，fuzzy checkpoint发生的时候会记录两次LSN，也就是检查点发生的时间和检查点结束的时间。但是呢，被刷新的页在并不一定在某一个时间点是一致的，这也就是它为什么叫fuzzy的原因。较早刷入磁盘的数据可能已经修改了，较晚刷新的数据可能会有一个比前面LSN更新更小的一个LSN。fuzzy checkpoint在某种意义上可以理解为fuzzy checkpoint从redo  log的第一个LSN执行到最后一个LSN。恢复以后的话，REDO LOG就会从最后一个检查点开始时候记录的LSN开始。

一般情况下大家可能fuzzy checkpoint的发生频率会远高于sharp checkpoint发生的频率，这个事毫无疑问的。不过当数据库关闭，切换redo日志文件的时候是会触发sharp checkpoint，一般情况是fuzzy checkpoint发生的更多一些。

一般情况下，执行普通操作的时候将不会发生检查点的操作，但是，fuzzy checkpoint却要根据时间推进而不停的发生。刷新脏页已经成为了数据库的一个普通的日常操作。

INNODB维护了一个大的缓冲区，以保证被修改的数据不会被立即写入磁盘。她会将这些修改过的数据先保留在buffer pool当中，这样在这些数据被写入磁盘以前可能会经过多次的修改，我们称之为写结合。这些数据页在buffer pool当中都是按照list来管理的，free list会记录那些空间是可用的，LRU list记录了那些数据页是最近被访问到的。flush list则记录了在LSN顺序当中的所有的dirty page信息，最近最少修改信息。

这里着重看一下flush list，我们知道innodb的缓存空间是有限的。如果buffer pool空间使用完毕，再次读取新数据就会发生磁盘读，也就是会发生flush操作，所以说就要释放一部分没有被使用的空间来保证buffer pool的可用性。由于这样的操作是很耗时的，所以说INNODB是会连续按照时间点去执行刷新操作，这样就保证了又足够的clean page来作为交换，而不必发生flush操作。每一次刷新都会将flush list的最老的信息驱逐，这样才能够保证数据库缓冲命中率是很高的一个值。这些老数据的选取是根据他们在磁盘的位置和LSN（最后一次修改的）号来确认数据新旧。

MySQL数据的日志都是混合循环使用的，但是如果这些事物记录的页信息还没有被刷新到磁盘当中的话是绝对不会被覆盖写入的。如果还没被刷新入磁盘的数据被覆盖了日志文件，那数据库宕机的话岂不是所有被覆盖写入的事物对应的数据都要丢失了呢。因此，数据修改也是有时间限制的，因为新的事物或者正在执行的事物也是需要日志空间的。日志越大，限制就越小。而且每次fuzzy checkpoint都会将最老最不被访问的数据驱逐出去，这也保证了每次驱逐的都是最老的数据，在下次日志被覆盖写入的时候都是已经被刷盘的数据的日志信息。最后一个老的,不被访问的数据的事物的LSN就是事务日志的 low-water标记，INNODB一直想提高这个LSN的值以保证buffer pool又足够的空间刷入新的数据，同时保证了数据库事务日志文件可以被覆盖写入的时候有足够的空间使用。将事务日志设置的大一些能够降低释放日志空间的紧迫性，从而可以大大的提高性能。

当innodb刷新 dirty page落盘的时候，他会找到最老的dirty  page对应的LSN并且将其标记为low-water，然后将这些信息记录到事物日志的头部，因此，每次刷新脏页都是要从flush  list的头部进行刷新的。在推进最老的LSN的标记位置的时候，本质上就是做了一次检查点。

当INNODB宕机的时候，他还要做一些额外的操作，第一：停止所有的数据更新等操作，第二：将dirty page in  buffer 的数据刷新落盘，第三：记录最后的LSN，因为我们上面也说到了，这次发生的是sharp checkpoint，并且，这个LSN会写入到没一个数据库文件的头部，以此来标记最后发生检查点的时候的LSN位置。

我们知道，刷新脏页数据的频率如果越高的话就代表整个数据库的负载很大，越小当然代表数据库的压力会小一点。将LOG 文件设置的很大能够再检查点发生期间减少磁盘的IO,总大小最好能够设置为和buffer pool大小相同，当然如果日志文件设置太大的话MySQL就会再crash recovery的时候花费更多的时间（5.5之前）。


http://mysqlmusings.blogspot.com/2011/04/crash-safe-replication.html

http://mysql.taobao.org/monthly/2016/05/01/

http://mysql.taobao.org/monthly/2015/06/01/

http://mysql.taobao.org/monthly/2015/05/01/

http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

https://gold.xitu.io/entry/5841225561ff4b00587ec651

https://yq.aliyun.com/articles/64677?utm_campaign=wenzhang&utm_medium=article&utm_source=QQ-qun&utm_content=m_7935

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=696

http://mysqllover.com/?p=213

http://mysql.taobao.org/monthly/2016/02/03/

http://mysql.taobao.org/monthly/2016/08/07/

http://mysql.taobao.org/monthly/2015/12/01/

http://mysqllover.com/?p=834

http://mysqllover.com/?p=1087

http://www.xuchunyang.com/2016/01/13/deak_lock/

http://mysqllover.com/?p=1119

http://www.askmaclean.com/archives/mysql-recover-innodb.html

http://www.askmaclean.com/archives/mysql%e4%b8%ad%e6%81%a2%e5%a4%8d%e4%bf%ae%e5%a4%8dinnodb%e6%95%b0%e6%8d%ae%e5%ad%97%e5%85%b8.html

http://www.thinkphp.cn/code/430.html

http://www.cnblogs.com/liuhao/p/3714012.html

http://louisyang.blog.51cto.com/8381303/1360394

http://mysql.taobao.org/monthly/2015/05/01/

http://hamilton.duapp.com/detail?articleId=34

https://twindb.com/undrop-tool-for-innodb/

https://twindb.com/tag/stream_parser/

http://jishu.y5y.com.cn/aeolus_pu/article/details/60143284

http://hedengcheng.com/?p=148

read_view

http://www.cnblogs.com/chenpingzhao/p/5065316.html

http://kabike.iteye.com/blog/1820553

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://mysqllover.com/?p=696

http://imysql.com/2014/08/13/mysql-faq-howto-shutdown-mysqld-fulgraceful.shtml

http://11879724.blog.51cto.com/11869724/1872928

http://coolnull.com/3145.html

http://mysqllover.com/?p=594

http://mysqllover.com/?p=87

http://mysqllover.com/?p=581

http://keithlan.github.io/2016/06/23/gtid/

http://mysqlmusings.blogspot.com/2011/04/crash-safe-replication.html

http://www.orczhou.com/index.php/2010/12/more-about-mysql-innodb-shutdown/

https://www.slideshare.net/frogd/inno-db-15344119

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://github.com/zhaiwx1987/innodb_ebook/blob/master/innodb_adaptive_hash.md




隔离级别

详细可以查看row_search_for_mysql()中的实现，实际上也就是row_search_mvcc()函数的实现。

row_search_for_mysql()   
 |-row_search_no_mvcc()       # 对于MySQL内部使用的表(用户不可见)，不需要MVCC机制
 |-row_search_mvcc()

row_search_no_mvcc()用于MySQL的内部表使用，通常是一些作为一个较大任务的中间结果存储，所以希望其可以尽快处理，因此不需要MVCC机制。

事务的隔离级别在trx->isolation_level中定义，其取值也就是如下的宏定义。

#define TRX_ISO_READ_UNCOMMITTED        0
#define TRX_ISO_READ_COMMITTED          1
#define TRX_ISO_REPEATABLE_READ         2
#define TRX_ISO_SERIALIZABLE            3


在不同的隔离级别下，可见性的判断有很大的不同。

READ-UNCOMMITTED
在该隔离级别下会读到未提交事务所产生的数据更改，这意味着可以读到脏数据，实际上你可以从函数row_search_mvcc中发现，当从btree读到一条记录后，如果隔离级别设置成READ-UNCOMMITTED，根本不会去检查可见性或是查看老版本。这意味着，即使在同一条SQL中，也可能读到不一致的数据。

    READ-COMMITTED
    在该隔离级别下，可以在SQL级别做到一致性读，当事务中的SQL执行完成时，ReadView被立刻释放了，在执行下一条SQL时再重建ReadView。这意味着如果两次查询之间有别的事务提交了，是可以读到不一致的数据的。

    REPEATABLE-READ
    可重复读和READ-COMMITTED的不同之处在于，当第一次创建ReadView后（例如事务内执行的第一条SEELCT语句），这个视图就会一直维持到事务结束。也就是说，在事务执行期间的可见性判断不会发生变化，从而实现了事务内的可重复读。

    SERIALIZABLE
    序列化的隔离是最高等级的隔离级别，当一个事务在对某个表做记录变更操作时，另外一个查询操作就会被该操作堵塞住。同样的，如果某个只读事务开启并查询了某些记录，那么另外一个session对这些记录的更改操作是被堵塞的。内部的实现其实很简单：
        对InnoDB表级别加LOCK_IS锁，防止表结构变更操作
        对查询得到的记录加LOCK_S共享锁，这意味着在该隔离级别下，读操作不会互相阻塞。而数据变更操作通常会对记录加LOCK_X锁，和LOCK_S锁相冲突，InnoDB通过给查询加记录锁的方式来保证了序列化的隔离级别。

注意不同的隔离级别下，数据具有不同的隔离性，甚至事务锁的加锁策略也不尽相同，你需要根据自己实际的业务情况来进行选择。





SELECT count_star, sum_timer_wait, avg_timer_wait, event_name FROM events_waits_summary_global_by_event_name WHERE count_star > 0 AND event_name LIKE "wait/synch/%" ORDER BY sum_timer_wait DESC LIMIT 20;




最新的事务ID通过trx_sys_get_new_trx_id()函数获取，每次超过了TRX_SYS_TRX_ID_WRITE_MARGIN次数后，都会调用trx_sys_flush_max_trx_id()函数刷新磁盘。






innodb_force_recovery变量对应源码中的srv_force_recovery变量，
 


当innodb_fast_shutdown设置为0时，会导致purge一直工作近两个小时。？？？？？

从5.5版本开始，purge任务从主线程中独立出来；5.6开始支持多个purge线程，可以通过innodb_purge_threads变量控制。

purge后台线程的最大数量可以有32个，包括了一个coordinator线程，以及多个worker线程。

在innobase_start_or_create_for_mysql()函数中，会创建srv_purge_coordinator_thread以及srv_worker_thread线程。


srv_purge_coordinator_thread()
 |-srv_purge_coordinator_suspend()   如果不需要purge或者上次purge记录数为0，则暂停
 |-srv_purge_should_exit()           判断是否需要退出；fast_shutdown=0则等待所有purge操作完成
 |-srv_do_purge()                    协调线程的主要工作，真正调用执行purge操作的函数
 |
 |-trx_purge()                       防止上次循环结束后又新的记录写入，此处不再使用worker线程
 |
 |-trx_purge()                       最后对history-list做一次清理，确保所有worker退出

srv_worker_thread()


最后一次做trx_purge()时，为了防止执行时间过程，批量操作时不再采用innodb_purge_batch_size(300)指定的值，而是采用20。


InnoDB的数据组织方式采用聚簇索引，也就是索引组织表，而二级索引采用(索引键值,主键键值)组合来唯一确定一条记录。
无论是聚簇索引，还是二级索引，每条记录都包含了一个DELETED-BIT位，用于标识该记录是否是删除记录；除此之外，聚簇索引还有两个系统列：DATA_TRX_ID，DATA_ROLL_PTR，分别表示产生当前记录项的事务ID以及指向当前记录的undo信息。



从聚簇索引行结构，与二级索引行结构可以看出，聚簇索引中包含版本信息(事务号+回滚指针)，二级索引不包含版本信息，二级索引项的可见性如何判断？？？？


InnoDB存储引擎在开始一个RR读之前，会创建一个Read View。Read View用于判断一条记录的可见性。Read View定义在read0read.h文件中，其中最主要的与可见性相关的属性如下：

class ReadView {
private:
  trx_id_t        m_low_limit_id;  //
};


ReadView::prepare()


copy_trx_ids
mtr_commit(struct mtr_t*)                 提交一个mini-transaction，调用mtr_t::commit()
 |-mtr_t::Command::execute()              写redo-log，将脏页添加到flush-list，并释放占用资源
   |-mtr_t::Command::prepare_write()      准备写入日志
   | |-fil_names_write_if_was_clean()
   |-mtr_t::Command::finish_write()   


测试场景#1 Drop Database (innodb_file_per_table=ON)
OFF代表MySQL是共享表空间，也就是所有库的数据都存放在一个ibdate1文件中；ON代表每个表的存储空间都是独立的。

ibd是MySQL数据文件、索引文件，二进制文件无法直接读取；frm是表结构文件，可以直接打开。如果innodb_file_per_table 无论是ON还是OFF，都会有这2个文件，区别只是innodb_file_per_table为ON的时候，数据时放在 .idb中，如果为OFF则放在ibdata1中。






 








#######redo-log文件
redo-log保存在innodb_log_group_home_dir参数指定的目录下，文件名为ib_logfile*；undo保存在共享表空间ibdata*文件中。

InnoDB的redo log可控制文件大小以及文件个数，分别通过innodb_log_file_size和innodb_log_files_in_group控制，总大小为两者之积。日志顺序写入，而且文件循环使用。

简单来说，InnoDB中的两个核心参数innodb_buffer_pool_size、innodb_log_file_size，分别定义了数据缓存和redo-log的大小，而后者的大小也决定了可以允许buffer中可以有多少脏页。当然，也不能因此就增大redo-log文件的大小，如果这样，可能会导致系统启动时Crash Recovery时间增大。




LSN对应了日志文件的偏移量，为了减小故障恢复时间，引入了Checkpoint机制，

InnoDB在启动时会自动检测InnoDB数据和事务日志是否一致，是否需要执行相应的操作？？？保证数据一致性；当然，故障恢复时间与事务日志的大小相关。


checkpoint会将最近写入的LSN




主线程主要完成 purge、checkpoint、dirty pages flush 等操作。



Database was not shutdown normally!   # InnoDB开始Crash Recovery{recv_init_crash_recovery_spaces()}
Starting crash recovery.



1. 读取Checkpoint LSN
2. 从Checkpoint LSN开始向前遍历Redo Log File
   重做从Checkpoint LSN开始的所有Redo日志
3. 重新构造系统崩溃时的事务
   Commit事务，等待Purge线程回收
   Prepare事务，由MySQL Server控制提交或者回滚(与Binlog 2PC相关)
   Active事务，回滚
4. 新建各种后台线程，Crash Recovery完成返回


正常关闭时，会在flush redo log和脏页后，做一次完全同步的checkpoint，并将checkpoint的LSN写到第一个ibdata文件的第一个page中，详细可以参考fil_write_flushed_lsn()。




innobase_start_or_create_for_mysql()
 |-log_group_init()
   |-log_calc_max_ages()


        log_sys->log_group_capacity = smallest_capacity;

        log_sys->max_modified_age_async = margin
                - margin / LOG_POOL_PREFLUSH_RATIO_ASYNC;
        log_sys->max_modified_age_sync = margin
                - margin / LOG_POOL_PREFLUSH_RATIO_SYNC;

        log_sys->max_checkpoint_age_async = margin - margin
                / LOG_POOL_CHECKPOINT_RATIO_ASYNC;
        log_sys->max_checkpoint_age = margin;




http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

http://mysql.taobao.org/monthly/2015/05/01/

http://mysql.taobao.org/monthly/2016/05/01/

http://tech.uc.cn/?p=716

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=620

http://apprize.info/php/effective/6.html

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

数据库内核分享

https://www.slideshare.net/frogd/inno-db-15344119

检查保存到磁盘的最大checkpoint LSN与redo-log的LSN是否一致；


MySQL · 引擎特性 · InnoDB 崩溃恢复过程

http://mysql.taobao.org/monthly/2015/06/01/













https://blogs.oracle.com/mysqlinnodb/entry/repeatable_read_isolation_level_in



http://mysql.taobao.org/monthly/2015/12/01/
http://hedengcheng.com/?p=148
read_view
http://www.cnblogs.com/chenpingzhao/p/5065316.html
http://kabike.iteye.com/blog/1820553
http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/
http://mysqllover.com/?p=696

隔离级别
详细可以查看row_search_mvcc()中的实现


row_search_for_mysql()
 |-row_search_no_mvcc()       # 对于MySQL内部使用的表(用户不可见)，不需要MVCC机制
 |-row_search_mvcc()

row_search_no_mvcc()用于MySQL的内部表使用，通常是一些作为一个较大任务的中间结果存储，所以希望其可以尽快处理，因此不需要MVCC机制。


innodb_force_recovery变量对应源码中的srv_force_recovery变量，




当innodb_fast_shutdown设置为0时，会导致purge一直工作近两个小时。？？？？？

从5.5版本开始，purge任务从主线程中独立出来；5.6开始支持多个purge线程，可以通过innodb_purge_threads变量控制。

purge后台线程的最大数量可以有32个，包括了一个coordinator线程，以及多个worker线程。

在innobase_start_or_create_for_mysql()函数中，会创建srv_purge_coordinator_thread以及srv_worker_thread线程。


srv_purge_coordinator_thread()
 |-srv_purge_coordinator_suspend()   如果不需要purge或者上次purge记录数为0，则暂停
 |-srv_purge_should_exit()           判断是否需要退出；fast_shutdown=0则等待所有purge操作完成
 |-srv_do_purge()                    协调线程的主要工作，真正调用执行purge操作的函数
 |
 |-trx_purge()                       防止上次循环结束后又新的记录写入，此处不再使用worker线程
 |
 |-trx_purge()                       最后对history-list做一次清理，确保所有worker退出

srv_worker_thread()


最后一次做trx_purge()时，为了防止执行时间过程，批量操作时不再采用innodb_purge_batch_size(300)指定的值，而是采用20。


InnoDB的数据组织方式采用聚簇索引，也就是索引组织表，而二级索引采用(索引键值,主键键值)组合来唯一确定一条记录。
无论是聚簇索引，还是二级索引，每条记录都包含了一个DELETED-BIT位，用于标识该记录是否是删除记录；除此之外，聚簇索引还有两个系统列：DATA_TRX_ID，DATA_ROLL_PTR，分别表示产生当前记录项的事务ID以及指向当前记录的undo信息。



从聚簇索引行结构，与二级索引行结构可以看出，聚簇索引中包含版本信息(事务号+回滚指针)，二级索引不包含版本信息，二级索引项的可见性如何判断？？？？


InnoDB存储引擎在开始一个RR读之前，会创建一个Read View。Read View用于判断一条记录的可见性。Read View定义在read0read.h文件中，其中最主要的与可见性相关的属性如下：

class ReadView {
private:
  trx_id_t        m_low_limit_id;  //
};


mtr_commit(struct mtr_t*)                 提交一个mini-transaction，调用mtr_t::commit()
 |-mtr_t::Command::execute()              写redo-log，将脏页添加到flush-list，并释放占用资源
   |-mtr_t::Command::prepare_write()      准备写入日志
   | |-fil_names_write_if_was_clean()
   |-mtr_t::Command::finish_write()












妈的文件整理文件

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://www.cnblogs.com/liuhao/p/3714012.html


持续集成 https://www.zhihu.com/question/23444990




buf_flush_batch


<!--
注意这里，在函数结束时并没有将batch_running设置为FALSE，因为这里对数据文件做的是异步写，设置标记位的工作留给了IO线程来完成
io_handler_thread-> fil_aio_wait-> buf_page_io_complete->buf_flush_write_complete->buf_dblwr_update()：
-->










FAQ系列 | 如何避免ibdata1文件大小暴涨

0、导读

    遇到InnoDB的共享表空间文件ibdata1文件大小暴增时，应该如何处理？

1、问题背景

用MySQL/InnoDB的童鞋可能也会有过烦恼，不知道为什么原因，ibdata1文件莫名其妙的增大，不知道该如何让它缩回去，就跟30岁之后男人的肚腩一样，汗啊，可喜可贺的是我的肚腩还没长出来，hoho~

正式开始之前，我们要先知道ibdata1文件是干什么用的。

ibdata1文件是InnoDB存储引擎的共享表空间文件，该文件中主要存储着下面这些数据：

        data dictionary
        double write buffer
        insert buffer/change buffer
        rollback segments
        undo space
        Foreign key constraint system tables

另外，当选项 innodb_file_per_table = 0 时，在ibdata1文件中还需要存储 InnoDB 表数据&索引。ibdata1文件从5.6.7版本开始，默认大小是12MB，而在这之前默认大小是10MB，其相关选项是 innodb_data_file_path，比如我一般是这么设置的：

    innodb_data_file_path = ibdata1:1G:autoextend

当然了，无论是否启用了 innodb_file_per_table = 1，ibdata1文件都必须存在，因为它必须存储上述 InnoDB 引擎所依赖&必须的数据，尤其是上面加粗标识的 rollback segments 和 undo space，它俩是引起 ibdata1 文件大小增加的最大原因，我们下面会详细说。
2、原因分析

我们知道，InnoDB是支持MVCC的，它和ORACLE类似，采用 undo log、redo log来实现MVCC特性的。在事务中对一行数据进行修改时，InnoDB 会把这行数据的旧版本数据存储一份在undo log中，如果这时候有另一个事务又要修改这行数据，就又会把该事物最新可见的数据版本存储一份在undo log中，以此类推，如果该数据当前有N个事务要对其进行修改，就需要存储N份历史版本（和ORACLE略有不同的是，InnoDB的undo log不完全是物理block，主要是逻辑日志，这个可以查看 InnoDB 源码或其他相关资料）。这些 undo log 需要等待该事务结束后，并再次根据事务隔离级别所决定的对其他事务而言的可见性进行判断，确认是否可以将这些 undo log 删除掉，这个工作称为 purge（purge 工作不仅仅是删除过期不用的 undo log，还有其他，以后有机会再说）。

那么问题来了，如果当前有个事务中需要读取到大量数据的历史版本，而该事务因为某些原因无法今早提交或回滚，而该事务发起之后又有大量事务需要对这些数据进行修改，这些新事务产生的 undo log 就一直无法被删除掉，形成了堆积，这就是导致 ibdata1 文件大小增大最主要的原因之一。这种情况最经典的场景就是大量数据备份，因此我们建议把备份工作放在专用的 slave server 上，不要放在 master server 上。

另一种情况是，InnoDB的 purge 工作因为本次 file i/o 性能是在太差或其他的原因，一直无法及时把可以删除的 undo log 进行purge 从而形成堆积，这是导致 ibdata1 文件大小增大另一个最主要的原因。这种场景发生在服务器硬件配置比较弱，没有及时跟上业务发展而升级的情况。

比较少见的一种是在早期运行在32位系统的MySQL版本中存在bug，当发现待 purge 的 undo log 总量超过某个值时，purge 线程直接放弃抵抗，再也不进行 purge 了，这个问题在我们早期使用32位MySQL 5.0版本时遇到的比较多，我们曾经遇到这个文件涨到100多G的情况。后来我们费了很大功夫把这些实例都迁移到64位系统下，终于解决了这个问题。

最后一个是，选项 innodb_data_file_path 值一开始就没调整或者设置很小，这就必不可免导致 ibdata1 文件增大了。Percona官方提供的 my.cnf 参考文件中也一直没把这个值加大，让我百思不得其解，难道是为了像那个经常被我吐槽的xx那样，故意留个暗门，好方便后续帮客户进行优化吗？（我心理太阴暗了，不好不好~~）

稍微总结下，导致ibdata1文件大小暴涨的原因有下面几个：

        有大量并发事务，产生大量的undo log；
        有旧事务长时间未提交，产生大量旧undo log；
        file i/o性能差，purge进度慢；
        初始化设置太小不够用；
        32-bit系统下有bug。

稍微题外话补充下，另一个热门数据库 PostgreSQL 的做法是把各个历史版本的数据 和 原数据表空间 存储在一起，所以不存在本案例的问题，也因此 PostgreSQL 的事务回滚会非常快，并且还需要定期做 vaccum 工作（具体可参见PostgreSQL的MVCC实现机制，我可能说的不是完全正确哈）
3、解决方法建议

看到上面的这些问题原因描述，有些同学可能觉得这个好办啊，对 ibdata1 文件大小进行收缩，回收表空间不就结了吗。悲剧的是，截止目前，InnoDB 还没有办法对 ibdata1 文件表空间进行回收/收缩，一旦 ibdata1 文件的肚子被搞大了，只能把数据先备份后恢复再次重新初始化实例才能恢复原先的大小，或者把依次把各个独立表空间文件备份恢复到一个新实例中，除此外，没什么更好的办法了。

当然了，这个问题也并不是不能防范，根据上面提到的原因，相应的建议对策是：

        升级到5.6及以上（64-bit），采用独立undo表空间，5.6版本开始就支持独立的undo表空间了，再也不用担心会把 ibdata1 文件搞大；
        初始化设置时，把 ibdata1 文件至少设置为1GB以上；
        增加purge线程数，比如设置 innodb_purge_threads = 8；
        提高file i/o能力，该上SSD的赶紧上；
        事务及时提交，不要积压；
        默认打开autocommit = 1，避免忘了某个事务长时间未提交；
        检查开发框架，确认是否设置了 autocommit=0，记得在事务结束后都有显式提交或回滚。



关于MySQL的方方面面大家想了解什么，可以直接留言回复，我会从中选择一些热门话题进行分享。 同时希望大家多多转发，多一些阅读量是老叶继续努力分享的绝佳助力，谢谢大家 :)

最后打个广告，运维圈人士专属铁观音茶叶微店上线了，访问：http://yejinrong.com 获得专属优惠




MySQL-5.7.7引入的一个系统库sys-schema，包含了一系列视图、函数和存储过程，主要是一些帮助MySQL用户分析问题和定位问题，可以方便查看哪些语句使用了临时表，哪个用户请求了最多的io，哪个线程占用了最多的内存，哪些索引是无用索引等。

其数据均来自performance schema和information schema中的统计信息。

MySQL 5.7.7 and higher includes the sys schema, a set of objects that helps DBAs and developers interpret data collected by the Performance Schema. sys schema objects can be used for typical tuning and diagnosis use cases.

MySQL Server blog中有一个很好的比喻：

For Linux users I like to compare performance_schema to /proc, and SYS to vmstat.

也就是说，performance schema和information schema中提供了信息源，但是，没有很好的将这些信息组织成有用的信息，从而没有很好的发挥它们的作用。而sys schema使用performance schema和information schema中的信息，通过视图的方式给出解决实际问题的答案。

查看是否安装成功
select * from sys.version;
查看类型
select * from sys.schema_object_overview where db='sys';
当然，也可以通过如下命令查看
show full tables from sys
show function status where db = 'sys';
show procedure status where db = 'sys'

user/host资源占用情况
SHOW TABLES FROM `sys` WHERE
    `Tables_in_sys` LIKE 'user\_%' OR
 `Tables_in_sys` LIKE 'host\_%'
IO资源使用，包括最近IO使用情况latest_file_io
SHOW TABLES LIKE 'io\_%'
schema相关，包括表、索引使用统计
SHOW TABLES LIKE 'schema\_%'
等待事件统计
SHOW TABLES LIKE 'wait%'
语句查看，包括出错、全表扫描、创建临时表、排序、空闲超过95%
SHOW TABLES LIKE 'statement%'
当前正在执行链接，也就是processlist
其它还有一些厂家的帮助函数，PS设置。
https://www.slideshare.net/Leithal/the-mysql-sys-schema
http://mingxinglai.com/cn/2016/03/sys-schema/
http://www.itpub.net/thread-2083877-1-1.html

x$NAME保存的是原始数据，比较适合通过工具调用；而NAME表更适合阅读，比如使用命令行去查看。


select digest,digest_text from performance_schema.events_statements_summary_by_digest\G
CALL ps_trace_statement_digest('891ec6860f98ba46d89dd20b0c03652c', 10, 0.1, TRUE, TRUE);
CALL ps_trace_thread(25, CONCAT('/tmp/stack-', REPLACE(NOW(), ' ', '-'), '.dot'), NULL, NULL, TRUE, TRUE, TRUE);

优化器调优
https://dev.mysql.com/doc/internals/en/optimizer-tracing.html


MySQL performance schema instrumentation interface(PSI)

struct PFS_instr_class {}; 基类


通过class page_id_t区分页，

class page_id_t {
private:
    ib_uint32_t     m_space;     指定tablespace
    ib_uint32_t     m_page_no;   页的编号





buf_page_get_gen()          获取数据库中的页
 |-buf_pool_get()           所在buffer pool实例
 |-buf_page_hash_lock_get()
 |-buf_page_hash_get_low()  尝试从bp中获取页
 |-buf_read_page()
   |-buf_read_page_low()
     |-buf_page_init_for_read()  初始化bp
    |-buf_LRU_get_free_block() 如果没有压缩，则直接获取空闲页
    |-buf_LRU_add_block()
    |
    |-buf_buddy_alloc()        压缩页，使用buddy系统
  |-fil_io()
 |-buf_block_get_state()         根据页的类型，判断是否需要进一步处理，如ZIP
 |-buf_read_ahead_random()

buf_read_ahead_linear()

http://www.myexception.cn/database/511937.html
http://blog.csdn.net/taozhi20084525/article/details/17613785
http://blogread.cn/it/article/5367
http://mysqllover.com/?p=303
http://www.cnblogs.com/chenpingzhao/p/5107480.html ？？？
https://docs.oracle.com/cd/E17952_01/mysql-5.7-en/innodb-recovery-tablespace-discovery.html
http://mysqllover.com/?p=1214



[mysqld]
innodb_data_file_path            = ibdata1:12M;ibdata2:12M:autoextend



<br><br><br><h1>文件 IO 操作</h1><p>
在 InnoDB 中所有需要持久化的信息都需要文件操作，例如：表文件、重做日志文件、事务日志文件、备份归档文件等。InnoDB 对文件 IO 操作可以是煞费苦心，主要包括两方面：A) 对异步 IO 的实现；B) 对文件操作管理和 IO 调度的实现。<br><br>

其主要实现代码集中在 os_file.* + fil0fil.* 文件中，其中 os_file.* 是实现基本的文件操作、异步 IO 和模拟异步 IO；fil0fil.* 是对文件 IO 做系统的管理和 space 结构化。<br><br>

Innodb 的异步 IO 默认使用 libaio。
</p>

<!--
在innodb中，文件的操作是比较关键的，innodb封装了基本的文件操作，例如：文件打开与关闭、文件读写以及文件属性访问等。这些是基本的文件操作函数封装。在linux文件的读写方面，默认是采用pread/pwrite函数进行读写操作，如果系统部支持这两个函数，innodb用lseek和read、write函数联合使用来达到效果. 以下是innodb文件操作函数:
os_file_create_simple                        创建或者打开一个文件
os_file_create                                     创建或者打开一个文件，如果操作失败会重试，直到成功
os_file_close                                       关闭打开的文件
os_file_get_size                                   获得文件的大小
os_file_set_size                                   设置文件的大小并以0填充文件内容
os_file_flush                                        将写的内容fsync到磁盘
os_file_read                                        从文件中读取数据
os_file_write                                       将数据写入文件
innodb除了实现以上基本的操作以外，还实现了文件的异步IO模型，在Windows下采用的IOCP模型来进行处理（具
体可以见网上的资料），在linux下是采用aio来实现的，有种情况，一种是通过系统本身的aio机制来实现，还有一种是
通过多线程信号模拟来实现aio.这里我们重点来介绍，为了实现aio,innodb定义了slot和slot array,具体数据结构如下：

typedef struct os_aio_slot_struct
{
     ibool   is_read;                             /*是否是读操作*/
     ulint   pos;                                    /*slot array的索引位置*/
     ibool   reserved;                           /*这个slot是否被占用了*/
     ulint   len;                                     /*读写的块长度*/
     byte*   buf;                                   /*需要操作的数据缓冲区*/
     ulint   type;                                   /*操作类型：OS_FILE_READ OS_FILE_WRITE*/
     ulint   offset;                                 /*当前操作文件偏移位置，低32位*/
     ulint   offset_high;                        /*当前操作文件偏移位置，高32位*/
     os_file_t   file;                               /*文件句柄*/
     char*   name;                               /*文件名*/
     ibool   io_already_done;             /*在模拟aio的模式下使用，TODO*/
     void*   message1;
     void*   message2;
#ifdef POSIX_ASYNC_IO
     struct aiocb   control;                 /*posix 控制块*/
#endif
}os_aio_slot_t;

typedef struct os_aio_array_struct
{
 os_mutex_t  mutex;          /*slots array的互斥锁*/
 os_event_t  not_full;         /*可以插入数据的信号，一般在slot数据被aio操作后array_slot有空闲可利用的slot时发送*/
 os_event_t  is_empty;       /*array 被清空的信号，一般在slot数据被aio操作后array_slot里面没有slot时发送这个信号*/

 ulint   n_slots;                     /*slots总体单元个数*/
 ulint   n_segments;             /*segment个数，一般一个对应n个slot，n = n_slots/n_segments，一个segment作为aio一次的操作范围*/
 ulint   n_reserved;              /*有效的slots个数*/
 os_aio_slot_t* slots;         /*slots数组*/

 os_event_t*     events;         /*slots event array，暂时没弄明白做啥用的*/
}os_aio_array_t;

-->
其中数据刷盘的主要代码在 innodb/buf/buf0flu.c 中。
<pre style="font-size:0.8em; face:arial;">
buf_flush_batch()
 |-buf_do_LRU_batch()                         根据传入的type决定调用函数
 |-buf_do_flush_list_batch()
   |-buf_flush_page_and_try_neighbors()
     |-buf_flush_try_neighbors()
       |-buf_flush_page()                     刷写单个page
          |-buf_flush_write_block_low()       实际刷写单个page

    buf_flush_write_block_low调用buf_flush_post_to_doublewrite_buf （将page放到double write buffer中，并准备刷写）

    buf_flush_post_to_doublewrite_buf 调用 fil_io （ 文件IO的封装）

    fil_io 调用 os_aio （aio相关操作）

    os_aio 调用 os_file_write （实际写文件操作）

</pre>


其中buf_flush_batch 只有两种刷写方式： BUF_FLUSH_LIST 和 BUF_FLUSH_LRU 两种方式的方式和触发时机简介如下：

BUF_FLUSH_LIST: innodb master线程中 1_second / 10 second 循环中都会调用。触发条件较多（下文会分析）

BUF_FLUSH_LRU: 当Buffer Pool无空闲page且old list中没有足够的clean page时，调用。刷写脏页后可以空出一定的free page，供BP使用。

从触发频率可以看到 10 second 循环中对于 buf_flush_batch( BUF_FLUSH_LIST ) 的调用是10秒一次IO高负载的元凶所在。

我们再来看10秒循环中flush的逻辑：

    通过比较过去10秒的IO次数和常量的大小，以及pending的IO次数，来判断IO是否空闲，如果空闲则buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    如果脏页比例超过70，则 buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    否则  buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(10) );

可以看到由于SSD对于随机写的请求响应速度非常快，导致IO几乎没有堆积。也就让innodb误认为IO空闲，并决定全力刷写。

其中PCT_IO(N)  = innodb_io_capacity *N% ，单位是页。因此也就意味着每10秒，innodb都至少刷10000个page或者刷完当前所有脏页。

updated on 2013/10/31: 在5.6中官方的adaptive flush算法有所改变，但是空闲状态下innodb_io_capacity对于刷写page数量的影响仍然不改变。
UNIQUE 索引 IO 与聚簇索引 IO 完全一致，因为二者都必须读取页面，不能进行 Insert Buffer 优化。
<pre style="font-size:0.8em; face:arial;">
buf_page_get_gen()
 |-buf_page_hash_lock_get()                 # 判断所需的页是否在缓存中
 |-buf_read_page()                          # 如果不存在则直接从文件读取的buff_pool中
   |-buf_read_page_low()                    # 实际底层执行函数
     |-fil_io()
        |-os_aio()                          # 实际是一个宏定义，最终调用如下函数
        | |-os_aio_func()                   # 其入参包括了mode，标识同步/异步
        |   |-os_file_read_func()           # 同步读
        |   | |-os_file_pread()
        |   |   |-pread()
        |   |
        |   |-os_file_write_func()          # 同步写
        |   | |-os_file_pwrite()
        |   |   |-pwrite()
        |   |
        |   |-... ...                       # 对于异步操作，不同的mode其写入array会各不相同 #A
        |   |-os_aio_array_reserve_slot()   # 从相应队列中选取一个空闲slot，保存需要读写的信息
        |   | |
        |   | |-local_seg=... ...           # 1. 首先在任务队列中选择一个segment #B
        |   | |
        |   | |-os_mutex_enter()            # 2. 对队列加锁，遍历该segement，选择空闲的slot，如果没有则等待
        |   | |
        |   | |                             # 3. 如果array已经满了，根据是否使用AIO决定具体策略
        |   | |-os_aio_simulated_wake_handler_threads()    # 非native AIO，模拟唤醒
        |   | |-os_wait_event(array->not_full)             # native aio 则等待not_full信号
        |   | |
        |   | |-os_aio_array_get_nth_slot() # 4. 已经确定是有slot了，选择空闲的slot
        |   | |
        |   | |-slot... ...                 # 5. 将文件读写请求信息保存在slot，如目标文件、偏移量、数据等
        |   | |
        |   | |                             # 6. 对于Win AIO、Native AIO采取不同策略
        |   | |-ResetEvent(slot->handle)        # 对于Win调用该接口
        |   | |-io_prep_pread()                 # 而Linux AIO则根据传入的type，决定执行读或写
        |   | |-io_prep_pwrite()
        |   |
        |   |                               # 执行IO操作
        |   |-WriteFile()                       # 对于Win调用该函数
        |   |-os_aio_linux_dispatch()           # 对于LINUX_NATIVE_AIO需要执行该函数，将IO请求分发给内核层
        |   | |-io_submit()                 # 调用AIO接口函数发送
        |   |
        |   |-os_aio_windows_handle()       # Win下如果AIO_SYNC调用则通过该函数等待AIO结束
        |     |-... ...                     # 根据传入的array判断是否为sync_array
        |     |-WaitForSingleObject()           # 是则等待指定的slot aio操作完成
        |     |-WaitForMultipleObjects()        # 否则等待array中所有的aio操作完成
        |     |-GetOverlappedResult()       # 获取AIO的操作结果
        |     |-os_aio_array_free_slot()    # 最后释放当前slot
        |
 |      |-fil_node_complete_io()            # 如果是同步IO，则会等待完成，也就是确保调用os_aio()已经完成了IO操作
 |-buf_read_ahead_random()                  # 同时做预读

fil_aio_wait()
 |-os_aio_linux_handle()

os_aio_linux_handle

    分析完os_aio_windows_handle函数，接着分析Linux下同样功能的函数：os_aio_linux_handle
        无限循环，遍历array，直到定位到一个完成的I/O操作(slot->io_already_done)为止
        若当前没有完成的I/O，同时有I/O请求，则进入os_aio_linux_collect函数
            os_aio_linux_collect：从kernel中收集更多的I/O请求
                调用io_getevents函数，进入忙等，等待超时设置为OS_AIO_REAP_TIMEOUT

            /** timeout for each io_getevents() call = 500ms. */

            #define OS_AIO_REAP_TIMEOUT    (500000000UL)
                若io_getevents函数返回ret > 0，说明有完成的I/O，进行一些设置，最主要是将slot->io_already_done设置为TRUE

                slot->io_already_done = TRUE;
                若系统I/O处于空闲状态，那么io_thread线程的主要时间，都在io_getevents函数中消耗。


log_buffer_flush_to_disk()
 |-log_write_up_to()
</pre>



<ol type='A'><li>
<!--type = OS_FILE_READ; mode = OS_AIO_SYNC；-->
在这步中会选择不同的 array，包括了 os_aio_sync_array、os_aio_read_array、os_aio_write_array、os_aio_ibuf_array、os_aio_log_array。每个 aio array 在系统启动时调用 os0file.c::os_aio_init() 初始化。
<pre style="font-size:0.8em; face:arial;">
innobase_start_or_create_for_mysql() {
    ... ...
    os_aio_init(io_limit,            // 每个线程可并发处理pending IO的数量
        srv_n_read_io_threads,       // 处理异步read IO线程的数量
        srv_n_write_io_threads,      // 处理异步write IO线程的数量
        SRV_MAX_N_PENDING_SYNC_IOS); // 同步IO array的slots个数，
    ... ...
}

io_limit:
   windows = SRV_N_PENDING_IOS_PER_THREAD = 32
     linux = 8 * SRV_N_PENDING_IOS_PER_THREAD = 8 * 32 = 256

srv_n_read_io_threads:
    通过innobase_read_io_threads/innodb_read_io_threads参数控制
    因此可并发处理的异步read page请求为：io_limit * innodb_read_io_threads

srv_n_write_io_threads:
    通过innobase_write_io_threads/innodb_write_io_threads参数控制
    因此可并发处理的异步write请求为：io_limit * innodb_write_io_threads
    注意，当超过此限制时，必须将已有的异步IO部分写回磁盘，才能处理新的请求

SRV_MAX_N_PENDING_SYNC_IOS:
    同步IO不需要处理线程log thread、ibuf thread个数均为1
</pre>
接下来是创建 array 。
<pre style="font-size:0.8em; face:arial;">
os_aio_init()
 |-os_aio_array_create()
</pre>
异步 IO 主要包括两大类：A) 预读page，需要通过异步 IO 方式进行；B) 主动merge，Innodb 主线程对需要 merge 的 page 发出异步读操作，在read_thread 中进行实际 merge 处理。<!--
注：如何确定将哪些read io请求分配给哪些read thread？

    首先，每个read thread负责os_aio_read_array数组中的一部分。
    例如：thread0处理read_array[0, io_limit-1]；thread1处理read_array[io_limit, 2*io_limit – 1]，以此类推
    os_aio_array_reserve_slot函数中实现了array的分配策略(array未满时)。
    给定一个Aio read page，[space_id, page_no]，首先计算local_seg(local_thd):
    local_seg = (offset >> (UNIV_PAGE_SIZE_SHIFT + 6)) % array->n_segments;
    然后从read_array的local_seg * io_limit处开始向后遍历array，直到找到一个空闲slot。
    一来保证相邻的page，能够尽可能分配给同一个thread处理，提高aio(merge io request)性能；
    二来由于是循环分配，也基本上保证了每个thread处理的io基本一致。
--></li><br><li>



选择 segment 时，是根据偏移量来计算 segment 的，从而可以尽可能的将相邻的读写请求放到一起，从而有利于 IO 层的合并操作。
</li></ol>
<!--
http://blog.csdn.net/wudongxu/article/details/8647501  innodb学习（一）——innodb如何使用aio
http://blog.csdn.net/yuanrxdu/article/details/41418421  MySQL系列：innodb源码分析之文件IO
http://hedengcheng.com/?p=98   InnoDB AIO
http://mysqllover.com/?p=1444  InnoDB IO子系统介绍
-->
</p>






## 参考

XtraDB: The Top 10 enhancements
https://www.percona.com/blog/2009/08/13/xtradb-the-top-10-enhancements/

https://forums.cpanel.net/threads/innodb-corruption-repair-guide.418722/

http://www.itpub.net/thread-2083877-1-1.html




















innodb_adaptive_flushing
innodb_adaptive_flushing_lwm 百分比，配置自适应flush机制的低水位(low water mark)，超过该限制之后，即使没有通过上述参数开启AF，仍然执行AF
innodb_io_capacity
innodb_io_capacity_max redo 刷盘的最大值，如果刷盘落后很多，那么IO可能会超过innodb_io_capacity而小于max
innodb_max_dirty_pages_pct 刷脏时，需要保证没有超过该值；注意，该值是一个目标，并不会影响刷脏的速率。
innodb_max_dirty_pages_pct_lwm 脏页的低水位，用于决定什么时候开启pre-flush操作，从而保证不会超过上面配置的百分比
innodb_flushing_avg_loops 决定了利用上述的值循环多少次之后重新计算dirty page和LSN，次数越少对外部的动态变化就越敏感


要刷新多少page和lsn主要代码在af_get_pct_for_dirty()和af_get_pct_for_lsn()中，其中主要控制adaptive flush的代码位于后者函数中。

http://www.cnblogs.com/Amaranthus/p/4450840.html

1.先判断redo log的容量是否到了innodb_adaptive_flushing_lwm低水位阀值。
2.是否配置了adaptive flush或者age超过了异步刷新的阀值。
3.lsn_age_factor=age占异步刷新阀值的比例。
4.要被刷新的比率=innodb_io_capacity_max/innodb_io_capacity*lsn_age_factor* sqrt(innodb_io_capacity)/7.5


定义BP中的页。
class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{


https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***

page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 最近最少(the least recently used pages )使用的页将会从LRU_list上移除；
2. the oldest modified non-flushed pages从flush_list上移除；

https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive
https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in

MySQL 5.6.2引入了一个新的后台线程page_cleaner，

https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms
















当事务执行速度大于刷脏速度时，Ckp age和Buf age (innodb_flush_log_at_trx_commit!=1时) 都会逐步增长，当达到 async 点的时候，强制进行异步刷盘或者写 Checkpoint，如果这样做还是赶不上事务执行的速度，则为了避免数据丢失，到达 sync 点的时候，会阻塞其它所有的事务，专门进行刷盘或者写Checkpoint。

因此从理论上来说,只要事务执行速度大于脏页刷盘速度，最终都会触发日志保护机制，进而将事务阻塞，导致MySQL操作挂起。

class MVCC {
private:
    view_list_t             m_views;
};

buf_flush_wait_batch_end()


#define PCT_IO(p) ((ulong) (srv_io_capacity * ((double) (p) / 100.0)))



buf_flush_page_cleaner_coordinator() 该函数基本上由page_cleaner每隔1s调用一次
 |-buf_flush_page_cleaner_coordinator()
   |-page_cleaner_flush_pages_recommendation()
     |-af_get_pct_for_dirty() 需要刷新多个页
     | |-buf_get_modified_ratio_pct()
     |   |-buf_get_total_list_len()
  |
     |-af_get_pct_for_lsn() 计算是否需要进行异步刷redo log
       |-log_get_max_modified_age_async()



af_get_pct_for_lsn()计算方法涉及变量
srv_adaptive_flushing_lwm

srv_flushing_avg_loops

storage/innobase/log/log0log.cc

max_modified_age_sync

  |log_write_up_to()
    |-log_write_flush_to_disk_low()
      |-fil_flush()


#####FLUSH_LRU_LIST Checkpoint
srv_LRU_scan_depth


#####Async/Sync Flush Checkpoint
log_free_check() 用户线程调用
 |-log_check_margins()
   |-log_flush_margin()
   | |-log_write_up_to()
   |-log_checkpoint_margin() 执行sync操作，尝试空出足够的redo空间，避免checkpoint操作，可能会执行刷脏操作
     |-log_buf_pool_get_oldest_modification() 获取BP中最老的lsn，也就是LSN4
     | |-buf_pool_get_oldest_modification() 遍历各个BP实例，找出最大lsn，如果刚初始化完成则返回sys->lsn
     | 计算log->lsn-oldest_lsn，如果超过了max_modified_age_sync值，则执行sync操作

log_checkpoint_margin 核心函数，用于判断当前age情况，是否需要执行异步甚至是同步刷新。

buff async/sync是在前面，因为redo的刷新成本更低

buf_pool_resize() BP调整大小时的操作
 |-buf_pool_withdraw_blocks()


innodb_adaptive_flushing
innodb_adaptive_flushing_lwm 百分比，配置自适应flush机制的低水位(low water mark)，超过该限制之后，即使没有通过上述参数开启AF，仍然执行AF
innodb_io_capacity
innodb_io_capacity_max redo 刷盘的最大值，如果刷盘落后很多，那么IO可能会超过innodb_io_capacity而小于max
innodb_max_dirty_pages_pct 刷脏时，需要保证没有超过该值；注意，该值是一个目标，并不会影响刷脏的速率。
innodb_max_dirty_pages_pct_lwm 脏页的低水位，用于决定什么时候开启pre-flush操作，从而保证不会超过上面配置的百分比
innodb_flushing_avg_loops 决定了利用上述的值循环多少次之后重新计算dirty page和LSN，次数越少对外部的动态变化就越敏感


要刷新多少page和lsn主要代码在af_get_pct_for_dirty()和af_get_pct_for_lsn()中，其中主要控制adaptive flush的代码位于后者函数中。

http://www.cnblogs.com/Amaranthus/p/4450840.html

1.先判断redo log的容量是否到了innodb_adaptive_flushing_lwm低水位阀值。
2.是否配置了adaptive flush或者age超过了异步刷新的阀值。
3.lsn_age_factor=age占异步刷新阀值的比例。
4.要被刷新的比率=innodb_io_capacity_max/innodb_io_capacity*lsn_age_factor* sqrt(innodb_io_capacity)/7.5


定义BP中的页。
class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{



https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms
https://www.percona.com/blog/2014/11/18/mysqls-innodb_metrics-table-how-much-is-the-overhead/
https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb

http://www.cnblogs.com/digdeep/p/4947694.html ******
http://hedengcheng.com/?p=220 ***
http://blog.itpub.net/30496894/viewspace-2121517/


Purge 实际上就是一个垃圾回收策略，简单来说，对于类似 "DELETE FROM t WHERE c = 1;" 的 DML，InnoDB 实际上并不会直接就删除，主要原因是为了回滚以及MVCC机制，简述如下：
1. 在记录的控制标志位中，标记该行已经删除；
2. 将修改列的前镜像保存到UNDO log中；
3. 修改聚集索引中的DB_TRX_ID、DB_ROLL_PTR系统列，前者标示最近一次修改的事务信息，后者则指向undo log中的记录，而 undo log 可能会存在同样的两列指向其历史记录。
另外，B+Tree的合并操作比较耗时，通过后台的异步线程可以避免阻塞用户的事务。

当事务已经提交，而且其它事务也不再依赖该记录了，那么就可以删除掉相应的记录，当然，也包括了二级索引对应的记录；这也就是 purge 线程的工作。

接下来，看看 purge 是如何工作的？


trx_purge是purge任务调度的核心函数，包含三个参数：
* n_purge_threads —>使用到的worker线程数
* batch_size  —-> 由innodb_purge_batch_size控制，表示一次Purge的记录数
* truncate —>是否truncate history list

trx_purge()
 |-trx_purge_dml_delay() 计算是否需要对dml延迟
 | ### 持有purge_sys->latch的x锁
 |-clone_oldest_view() 复制当前的view，也就是Class MVCC:m_views链表的中尾部
 |-trx_purge_attach_undo_recs() 获取需要清理的undo记录
 |
 | ### 多线程
 |-que_fork_scheduler_round_robin() 根据是否是单线程
 |-srv_que_task_enqueue_low() 将线程添加到队列中
 |-que_run_threads() 协调线程也会运行执行一个任务
 |-trx_purge_wait_for_workers_to_complete() 等待任务执行完成
 |
 | ### 单线程
 |
 |-trx_purge_truncate() 如果需要删除
 http://mysqllover.com/?p=696

purge 会复制一份系统中最老的 view，通过这一结构体，可以断定哪些回滚段需要回收。


mysql> show variables like 'innodb%purge%';
+-----------------------------------------+-------+
| Variable_name                           | Value |
+-----------------------------------------+-------+
| innodb_max_purge_lag                    | 0     |   如果purge操作比较慢，可以通过该参数设置dml操作的延迟时间
| innodb_max_purge_lag_delay              | 0     |   最大延迟不会超过该参数
| innodb_purge_batch_size                 | 300   |   一次处理多少页
| innodb_purge_rseg_truncate_frequency    | 128   |
| innodb_purge_run_now                    | OFF   |
| innodb_purge_stop_now                   | OFF   |
| innodb_purge_threads                    | 4     |   并发线程数
| innodb_trx_purge_view_update_only_debug | OFF   |
+-----------------------------------------+-------+
8 rows in set (0.00 sec)
Changes in 5.5

In 5.5 there is an option innodb-purge-threads=[0,1] to create a dedicated thread that purges asynchronously if there are UNDO logs that need to be removed. We also introduced another option innodb-purge-batch-size that can be used to fine tune purge operations. The batch size determines how many UNDO log pages purge will parse and process in one pass.


 The default setting is 20, this is the same as the hard coded value that is in previous InnoDB releases. An interesting side effect of this value is that it also determines when purge will free the UNDO log pages after processing them. It is always after 128 passes, this magic value of 128  is the same as the number of UNDO logs in the system tablespace, now that 5.5 has 128 rollback segments. By increasing the innodb-purge-batch-size the freeing of the UNDO log pages behaviour changes, it will increase the number of UNDO log pages that it removes in a batch when the limit of 128 is reached. This change was seen as necessary so that we could reduce the cost of removing the UNDO log pages for the extra 127 rollback segments that were introduced in 5.5. Prior to this change iterating over the 128 rollback segments to find the segment to truncate had become expensive.

Changes in 5.6

In 5.6 we have the same parameters as 5.5 except that innodb-purge-threads can now be between 0 and 32. This introduces true multi threaded purging. If the value is greater than 1 then InnoDB will create that many purge worker threads and a dedicated purge coordinator thread. The responsibility of the purge coordinator thread is to parse the UNDO log records and parcel out the work to the worker threads. The coordinator thread also purges records, instead of just sitting around and waiting for the worker threads to complete. The coordinator thread will divide the innodb-purge-batch-size by innodb-purge-threads and hand that out as the unit of work for each worker thread.

对于单表来说，会阻塞在 dict_index_t::lock 中，除非使用分区；对于多表来说是可以并发的。





####### 崩溃恢复(Crash Recovery)
Crash Recovery的起点，Checkpoint LSN存储位置？
InnoDB如何完成Redo日志的重做？
InnoDB如何定位哪些事务需要Rollback？
Crash Recovery需要等待Rollbach完成吗？
InnoDB各版本，在Crash Recovery流程上做了哪些优化？
mysqld_safe是否存在自动重启功能？

ha_recover


af_get_pct_for_lsn()
 |-log_get_max_modified_age_async()
  
MySQL · 引擎特性 · InnoDB 崩溃恢复过程
http://mysql.taobao.org/monthly/2015/06/01/

Database was not shutdown normally!   # InnoDB开始Crash Recovery{recv_init_crash_recovery_spaces()}
Starting crash recovery.




Doing recovery: scanned up to log sequence number 0            # 扫描redolog日志recv_scan_log_recs()



Starting an apply batch of log records to the database...      # 开始应用redolog recv_apply_hashed_log_recs()   
InnoDB: Progress in percent: 2 3 4 5 ... 99
Apply batch completed


1. 读取Checkpoint LSN
2. 从Checkpoint LSN开始向前遍历Redo Log File
   重做从Checkpoint LSN开始的所有Redo日志
3. 重新构造系统崩溃时的事务
   Commit事务，等待Purge线程回收
   Prepare事务，由MySQL Server控制提交或者回滚(与Binlog 2PC相关)
   Active事务，回滚
4. 新建各种后台线程，Crash Recovery完成返回


正常关闭时，会在flush redo log和脏页后，做一次完全同步的checkpoint，并将checkpoint的LSN写到第一个ibdata文件的第一个page中，详细可以参考fil_write_flushed_lsn()。


innodb_counter_info[] 定义了监控计数器

http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

http://mysql.taobao.org/monthly/2015/05/01/

http://mysql.taobao.org/monthly/2016/05/01/

http://tech.uc.cn/?p=716

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=620

http://apprize.info/php/effective/6.html

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

数据库内核分享

https://www.slideshare.net/frogd/inno-db-15344119

检查保存到磁盘的最大checkpoint LSN与redo-log的LSN是否一致；


事务源码
崩溃恢复
ReadView
Undo-Redo


https://blogs.oracle.com/mysqlinnodb/entry/repeatable_read_isolation_level_in









#########################################################################


如果 InnoDB 没有正常关闭，会在服务器启动的时候执行崩溃恢复 (Crash Recovery)，这一流程比较复杂，涉及到了 redo log、undo log 甚至包括了 binlog 。

在此简单介绍下 InnoDB 崩溃恢复的流程。

<!-- more -->

## 崩溃恢复

InnoDB 的数据恢复是一个很复杂的过程，在其恢复过程中，需要 redolog、binlog、undolog 等参与，接下来具体了解下整个恢复的过程。

{% highlight text %}
innobase_init()
 |-innobase_start_or_create_for_mysql()
   |
   |-recv_sys_create()   创建崩溃恢复所需要的内存对象
   |-recv_sys_init()
   |
   |-srv_sys_space.check_file_spce()                检查系统表空间是否正常
   |-srv_sys_space.open_or_create()              1. 打开系统表空间，并获取flushed_lsn
   | |-read_lsn_and_check_flags()
   |   |-open_or_create()                           打开系统表空间
   |   |-read_first_page()                          读取第一个page
   |   |-buf_dblwr_init_or_load_pages()             将双写缓存加载到内存中，如果ibdata日志损坏，则通过dblwr恢复
   |   |-validate_first_page()                      校验第一个页是否正常，并读取flushed_lsn
   |   | |-mach_read_from_8()                       读取LSN，偏移为FIL_PAGE_FILE_FLUSH_LSN
   |   |-restore_from_doublewrite()                 如果有异常，则从dblwr恢复
   |
   |-log_group_init()                               redo log的结构初始化
   |-srv_undo_tablespaces_init()                    对于undo log表空间恢复
   |
   |-recv_recovery_from_checkpoint_start()       2. 从redo-log的checkpoint开始恢复；注意，正常启动也会调用
   | |-buf_flush_init_flush_rbt()                   创建一个红黑树，用于加速插入flush list
   | |                                              通过force_recovery判断是否大于SRV_FORCE_NO_LOG_REDO
   | |-recv_find_max_checkpoint()                   查找最新的checkpoint点，在此会校验redo log的头部信息
   | | |-log_group_header_read()                    读取512字节的头部信息
   | | |-mach_read_from_4()                         读取redo log的版本号LOG_HEADER_FORMAT
   | | |-recv_check_log_header_checksum()           版本1则校验页的完整性
   | | | |-log_block_get_checksum()                 获取页中的checksum，也就是页中的最后四个字节
   | | | |-log_block_calc_checksum_crc32()          并与计算后的checksum比较
   | | |-recv_find_max_checkpoint_0()
   | |   |-log_group_header_read()
   | |
   | |-recv_group_scan_log_recs()                3. 从checkpoint-lsn处开始查找MLOG_CHECKPOINT
   | | |-log_group_read_log_seg()
   | | |-recv_scan_log_recs()
   | |   |-recv_parse_log_recs()
   | |-recv_group_scan_log_recs()
   | |                                              ##如果flushed_lsn和checkponit lsn不同则恢复
   | |-recv_init_crash_recovery()
   | |-recv_init_crash_recovery_spaces()
   | |
   | |-recv_group_scan_log_recs()
   |
   |-trx_sys_init_at_db_start()
   |
   |-recv_apply_hashed_log_recs()                    当页LSN小于log-record中的LSN时，应用redo日志
   | |-recv_recover_page()                           实际调用recv_recover_page_func()
   |   |-recv_parse_or_apply_log_rec_body()
   |
   |-recv_recovery_from_checkpoint_finish()          完成崩溃恢复

fil_op_write_log() 些日志



fil_names_write()  写入MLOG_FILE_NAME
fil_name_write()
fil_op_write_log()

{% endhighlight %}

MLOG_FILE CHECKPOINT 如何写入？？？如何使用？？？kkkk

{% highlight text %}
flushed_lsn
  只有在系统表空间的第一页存在，偏移量为FIL_PAGE_FILE_FLUSH_LSN(26)，至少在此LSN之前的页已经刷型到磁盘；
  该LSN通过fil_write_flushed_lsn()函数写入；
{% endhighlight %}

1. 从系统表空间中读取flushed_lsn，每次刷脏时都会写入系统表空间的第一页，而且为了防止写入异常会使用Double Write Buffer；
2. 从redo log头部中读取两个checkpoint值，并比较获取最新的checkpoint信息；




mlog 如何记录的那些 ibd 时需要读取的。





InnoDB 在 MySQL 启动的时候，会对 redo-log 进行日志回放，通过 recv_sys_t 结构来进行数据恢复和控制的，它的结构如下：

{% highlight text %}
struct recv_sys_t {

mutex_t     mutex;                                 /*保护锁*/
ibool  apply_log_recs;                        /*正在应用log record到page中*/
ibool     apply_batch_on;                     /*批量应用log record标志*/

dulint  lsn;
ulint  last_log_buf_size;

byte*     last_block;                             /*恢复时最后的块内存缓冲区*/
byte*    last_block_buf_start;             /*最后块内存缓冲区的起始位置，因为last_block是512地址对齐的，需要这个变量记录free的地址位置*/
byte*   buf;                                        /*从日志块中读取的重做日志信息数据*/
ulint  len;    /*buf有效的日志数据长度*/

dulint    parse_start_lsn;                       /*开始parse的lsn*/
dulint   scanned_lsn;                           /*已经扫描过的lsn序号*/

ulint   scanned_checkpoint_no;          /*恢复日志的checkpoint 序号*/
ulint  recovered_offset;                       /*恢复位置的偏移量*/

dulint    recovered_lsn;                         /*恢复的lsn位置*/
dulint   limit_lsn;                                  /*日志恢复最大的lsn,暂时在日志重做的过程没有使用*/

ibool   found_corrupt_log;                   /*是否开启日志恢复诊断*/

log_group_t*  archive_group;

mem_heap_t*   heap;                             /*recv sys的内存分配堆,用来管理恢复过程的内存占用*/

  hash_table_t*   addr_hash;       // 以(space_id+page_no)为KEY的Hash表

ulint   n_addrs;                                        /*addr_hash中包含recv_addr的个数*/

};
{% endhighlight %}

在这个结构中，比较复杂的是addr_hash这个哈希表，这个哈希表是用sapce_id和page_no作为hash key,里面存储有恢复时对应的记录内容。

恢复日志在从日志文件中读出后，进行解析成若干个recv_t并存储在哈希表当中。在一个读取解析周期过后，日志恢复会对hash表中的recv_t中的数据写入到ibuf和page中。这里为什么要使用hash表呢？个人觉得是为了同一个page的数据批量进行恢复的缘故，这样可以page减少随机插入和修改。 以下是和这个过程相关的几个数据结构:


{% highlight text %}
/*对应页的数据恢复操作集合*/
struct recv_addr_t {
     ulint   state;          /*状态，RECV_NOT_PROCESSED、RECV_BEING_PROCESSED、RECV_PROCESSED*/
      ulint  space;         /*space的ID*/
       ulint     page_no;    /*页序号*/
  UT_LIST_BASE_NODE_T(recv_t) rec_list;  // 该页对应的log records地址
         hash_node_t     addr_hash;
};

/*当前的记录操作*/
struct recv_t {
     byte    type;             /*log类型*/
      ulint  len;               /*当前记录数据长度*/
       recv_data_t* data;    /*当前的记录数据list*/

  lsn_t  start_lsn;                 // mtr起始LSN
  lsn_t  end_lsn;                   // mtr结尾LSN
  UT_LIST_NODE_T(recv_t)  rec_list; // 该页对应的log records
};

struct recv_data_t {
  recv_data_t*   next;  // 指向下个结构体，该地址之后为一大块内存，用于存储log record消息体
};
{% endhighlight %}

他们的内存关系结构图如下： \
2.重做日志推演过程的LSN关系
除了这个恢复的哈希表以外，recv_sys_t中的各种LSN也是和日志恢复有非常紧密的关系。以下是各种lsn的解释： parse_start_lsn 本次日志重做恢复起始的lsn，如果是从checkpoint处开始恢复，等于checkpoint_lsn。 scanned_lsn 在恢复过程，将恢复日志从log_sys->buf解析块后存入recv_sys->buf的日志lsn. recovered_lsn 已经将数据恢复到page中或者已经将日志操作存储addr_hash当中的日志lsn; 在日志开始恢复时：
parse_start_lsn = scanned_lsn = recovered_lsn = 检查点的lsn。
在日志完成恢复时:
parse_start_lsn = 检查点的lsn
scanned_lsn = recovered_lsn = log_sys->lsn。
在日志推演过程中lsn大小关系如下：
\
3.日志恢复的主要接口和流程
恢复日志主要的接口函数: recv_recovery_from_checkpoint_start 从重做日志组内的最近的checkpoint开始恢复数据
recv_recovery_from_checkpoint_finish 结束从重做日志组内的checkpoint的数据恢复操作
recv_recovery_from_archive_start 从归档日志文件中进行数据恢复
recv_recovery_from_archive_finish 结束从归档日志中的数据恢复操作
recv_reset_logs 截取重做日志最后一段作为新的重做日志的起始位置，可能会丢失数据。
重做日志恢复数据的流程(checkpoint方式) 1.当MySQL启动的时候，先会从数据库文件中读取出上次保存最大的LSN。
2.然后调用recv_recovery_from_checkpoint_start，并将最大的LSN作为参数传入函数当中。
3.函数会先最近建立checkpoint的日志组，并读取出对应的checkpoint信息
4.通过checkpoint lsn和传入的最大LSN进行比较，如果相等，不进行日志恢复数据，如果不相等，进行日志恢复。
5.在启动恢复之前，先会同步各个日志组的archive归档状态
6.在开始恢复时，先会从日志文件中读取2M的日志数据到log_sys->buf，然后对这2M的数据进行scan,校验其合法性，而后将去掉block header的日志放入recv_sys->buf当中，这个过程称为scan,会改变scanned lsn.
7.在对2M的日志数据scan后，innodb会对日志进行mtr操作解析，并执行相关的mtr函数。如果mtr合法，会将对应的记录数据按space page_no作为KEY存入recv_sys->addr_hash当中。
8.当对scan的日志数据进行mtr解析后，innodb对会调用recv_apply_hashed_log_recs对整个recv_sys->addr_hash进行扫描，并按照日志相对应的操作进行对应page的数据恢复。这个过程会改变recovered_lsn。
9.如果完成第8步后，会再次从日志组文件中读取2M数据，跳到步骤6继续相对应的处理，直到日志文件没有需要恢复的日志数据。
10.innodb在恢复完成日志文件中的数据后，会调用recv_recovery_from_checkpoint_finish结束日志恢复操作，主要是释放一些开辟的内存。并进行事务和binlog的处理。
上面过程的示意图如下：




####### 崩溃恢复(Crash Recovery)
Crash Recovery的起点，Checkpoint LSN存储位置？
InnoDB如何完成Redo日志的重做？
InnoDB如何定位哪些事务需要Rollback？
Crash Recovery需要等待Rollbach完成吗？
InnoDB各版本，在Crash Recovery流程上做了哪些优化？
mysqld_safe是否存在自动重启功能？

ha_recover


af_get_pct_for_lsn()
 |-log_get_max_modified_age_async()
  
MySQL · 引擎特性 · InnoDB 崩溃恢复过程
http://mysql.taobao.org/monthly/2015/06/01/

Database was not shutdown normally!   # InnoDB开始Crash Recovery{recv_init_crash_recovery_spaces()}
Starting crash recovery.




Doing recovery: scanned up to log sequence number 0            # 扫描redolog日志recv_scan_log_recs()




Starting an apply batch of log records to the database...      # 开始应用redolog recv_apply_hashed_log_recs()   
InnoDB: Progress in percent: 2 3 4 5 ... 99
Apply batch completed


1. 读取Checkpoint LSN
2. 从Checkpoint LSN开始向前遍历Redo Log File
   重做从Checkpoint LSN开始的所有Redo日志
3. 重新构造系统崩溃时的事务
   Commit事务，等待Purge线程回收
   Prepare事务，由MySQL Server控制提交或者回滚(与Binlog 2PC相关)
   Active事务，回滚
4. 新建各种后台线程，Crash Recovery完成返回


正常关闭时，会在flush redo log和脏页后，做一次完全同步的checkpoint，并将checkpoint的LSN写到第一个ibdata文件的第一个page中，详细可以参考fil_write_flushed_lsn()。


innodb_counter_info[] 定义了监控计数器

http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

http://mysql.taobao.org/monthly/2015/05/01/

http://mysql.taobao.org/monthly/2016/05/01/

http://tech.uc.cn/?p=716

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=620

http://apprize.info/php/effective/6.html

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

数据库内核分享

https://www.slideshare.net/frogd/inno-db-15344119

检查保存到磁盘的最大checkpoint LSN与redo-log的LSN是否一致；


事务源码
崩溃恢复
ReadView
关闭过程
Undo-Redo


https://blogs.oracle.com/mysqlinnodb/entry/repeatable_read_isolation_level_in




GTID的全称为 global transaction identifier  ， 可以翻译为全局事务标示符，GTID在原始master上的事务提交时被创建。GTID需要在全局的主-备拓扑结构中保持唯一性，GTID由两部分组成：
GTID = source_id:transaction_id

source_id用于标示源服务器，用server_uuid来表示，这个值在第一次启动时生成，并写入到配置文件data/auto.cnf中
transaction_id则是根据在源服务器上第几个提交的事务来确定。

一个GTID的生命周期包括：
1.事务在主库上执行并提交
给事务分配一个gtid（由主库的uuid和该服务器上未使用的最小事务序列号），该GTID被写入到binlog中。
2.备库读取relaylog中的gtid，并设置session级别的gtid_next的值，以告诉备库下一个事务必须使用这个值
3.备库检查该gtid是否已经被其使用并记录到他自己的binlog中。slave需要担保之前的事务没有使用这个gtid，也要担保此时已分读取gtid，但未提交的事务也不恩呢过使用这个gtid.
4.由于gtid_next非空，slave不会去生成一个新的gtid，而是使用从主库获得的gtid。这可以保证在一个复制拓扑中的同一个事务gtid不变。

由于GTID在全局的唯一性，通过GTID，我们可以在自动切换时对一些复杂的复制拓扑很方便的提升新主库及新备库，例如通过指向特定的GTID来确定新备库复制坐标。

当然，使用GTID也有一些限制：
1.事务中的更新包含非事务性存储引擎，这可能导致多个GTID分配给同一个事务。
2. create table…select语句不被支持，因为该语句会被拆分成create table 和insert两个事务，并且这个两个事务被分配了同一个GTID，这会导致insert被备库忽略掉。
3.不支持CREATE/DROP临时表操作

可以看到，支持GTID的复制对一些语句都有一些限制，MySQL也提供了一个选项disable-gtid-unsafe-statements以禁止这些语句的执行。




### ibd页损坏
Page损坏的情况比较多：二级索引页损坏(可以通过OPTIMIZE TABLE恢复)；聚集索引页损坏；表字典损坏；
----- 使用VIM编译
vim -b titles.ibd
----- 通过外部xxd程序改变，注意修改完之后一定要通过-r选项恢复为二进制格式
:%!xxd
:%!xxd -r
当执行check table titles;检查表是否正常时，就会由于页面错误退出，报错信息如下。

buf_page_io_complete()
2017-03-09T08:58:34.750125Z 4 [ERROR] InnoDB: Database page corruption on disk or a failed file read of page [page id: space=NUM, page number=NUM]. You may have to recover from a backup.
len 16384; hex ... ...
InnoDB: End of page dump
InnoDB: Page may be an index page where index id is 48
2017-03-09T08:58:34.804632Z 4 [ERROR] [FATAL] InnoDB: Aborting because of a corrupt database page in the system tablespace. Or,  there was a failure in tagging the tablespace  as corrupt.
2017-03-09 16:58:34 0x7fe6e87b7700  InnoDB: Assertion failure in thread 140629719611136 in file ut0ut.cc line 916

实际上是可以正常重启的，但是一旦查询到该页时，仍然会报错，接下来看看如何恢复其中还完好的数据。

修改my.ini中的innodb_force_recovery参数，默认是0，此时可以修改为1-6，使mysqld在启动时跳过部分恢复步骤，在启动后将数据导出来然后重建数据库；当然，不同的情况可能恢复的数据会有所不同。

1. SRV_FORCE_IGNORE_CORRUPT: 忽略检查到的corrupt页；**
2. SRV_FORCE_NO_BACKGROUND): 阻止主线程的运行，在srv_master_thread()中处理；**

3. SRV_FORCE_NO_TRX_UNDO):不执行事务回滚操作。
4. SRV_FORCE_NO_IBUF_MERGE):不执行插入缓冲的合并操作。
5. SRV_FORCE_NO_UNDO_LOG_SCAN):不查看重做日志，InnoDB存储引擎会将未提交的事务视为已提交。
6. SRV_FORCE_NO_LOG_REDO):不执行redo log。

另外，需要注意，设置参数值大于0后，可以对表进行select,create,drop操作，但insert,update或者delete这类操作是不允许的。

再次执行check table titles;时，会报如下的错误。
+------------------+-------+----------+---------------------------------------------------+
| Table            | Op    | Msg_type | Msg_text                                          |
+------------------+-------+----------+---------------------------------------------------+
| employees.titles | check | Warning  | InnoDB: The B-tree of index PRIMARY is corrupted. |
| employees.titles | check | error    | Corrupt                                           |
+------------------+-------+----------+---------------------------------------------------+
2 rows in set (2.47 sec)

SELECT * FROM titles INTO OUTFILE '/tmp/titles.csv'
   FIELDS TERMINATED BY ',' ENCLOSED BY '"'
   LINES TERMINATED BY '\r\n';
注意上述在使用LOAD DATA INFILE或者SELECT INTO OUTFILE操作时，需要在配置文件中添加secure_file_priv=/tmp配置项，或者配置成secure_file_priv="/"不限制导入和导出路径。
如何确认哪些数据丢失了？？？？

http://www.runoob.com/mysql/mysql-database-export.html

如下是导致MySQL表毁坏的常见原因：
1、 服务器突然断电或者强制关机导致数据文件损坏；
2、 mysqld进程在修改表时被强制杀掉，例如kill -9；
3、 磁盘故障、服务器宕机等硬件问题无法恢复；
4、 使用myisamchk的同时，mysqld也在操作表；
5、 MySQL、操作系统、文件系统等软件的bug。




表损坏的典型症状：
 　　　　1 、当在从表中选择数据之时，你得到如下错误： 

　　　　　　Incorrect key file for table: '...'. Try to repair it

 　　　　2 、查询不能在表中找到行或返回不完全的数据。

　　　　 3 、Error: Table 'p' is marked as crashed and should be repaired 。

　　　　 4 、打开表失败： Can’t open file: ‘×××.MYI’ (errno: 145) 。

  ER_NOT_FORM_FILE  Incorrect information in file:
  ER_NOT_KEYFILE    Incorrect key file for table 'TTT'; try to repair it
  ER_OLD_KEYFILE    Old key file for table 'TTT'; repair it!
  ER_CANT_OPEN_FILE Can't open file: 'TTT' (errno: %d - %s)
 

　　3.预防 MySQL 表损坏

 　　可以采用以下手段预防mysql 表损坏： 

　　　　1 、定期使用myisamchk 检查MyISAM 表（注意要关闭mysqld ），推荐使用check table 来检查表（不用关闭mysqld ）。

　　　　2 、在做过大量的更新或删除操作后，推荐使用OPTIMIZE TABLE 来优化表，这样既减少了文件碎片，又减少了表损坏的概率。

　　　　3 、关闭服务器前，先关闭mysqld （正常关闭服务，不要使用kill -9 来杀进程）。

　　　　4 、使用ups 电源，避免出现突然断电的情况。

　　　　5 、使用最新的稳定发布版mysql ，减少mysql 本身的bug 导致表损坏。

　　　　6 、对于InnoDB 引擎，你可以使用innodb_tablespace_monitor来检查表空间文件内文件空间管理的完整性。

　　　　7 、对磁盘做raid ，减少磁盘出错并提高性能。

　　　　8 、数据库服务器最好只跑mysqld 和必要的其他服务，不要跑其他业务服务，这样减少死机导致表损坏的可能。

　　　　9 、不怕万一，只怕意外，平时做好备份是预防表损坏的有效手段。

　　4. MySQL 表损坏的修复

　　MyISAM 表可以采用以下步骤进行修复 ：

　　　　1、  使用 reapair table 或myisamchk 来修复。 

　　　　2、  如果上面的方法修复无效，采用备份恢复表。



　　具体可以参考如下做法：

　　阶段1 ：检查你的表

　　　　如果你有很多时间，运行myisamchk *.MYI 或myisamchk -e *.MYI 。使用-s （沉默）选项禁止不必要的信息。 

　　　　如果mysqld 服务器处于宕机状态，应使用--update-state 选项来告诉myisamchk 将表标记为' 检查过的' 。

　　　　你必须只修复那些myisamchk 报告有错误的表。对这样的表，继续到阶段2 。

　　　　如果在检查时，你得到奇怪的错误( 例如out of memory 错误) ，或如果myisamchk 崩溃，到阶段3 。

　　阶段2 ：简单安全的修复

　　　　注释：如果想更快地进行修复，当运行myisamchk 时，你应将sort_buffer_size 和Key_buffer_size 变量的值设置为可用内存的大约25% 。

　　　　首先，试试myisamchk -r -q tbl_name(-r -q 意味着“ 快速恢复模式”) 。这将试图不接触数据文件来修复索引文件。如果数据文件包含它应有的一切内容和指向数据文件内正确地点的删除连接，这应该管用并且表可被修复。开始修复下一张表。否则，执行下列过程：

　　　　在继续前对数据文件进行备份。

　　　　使用myisamchk -r tbl_name(-r 意味着“ 恢复模式”) 。这将从数据文件中删除不正确的记录和已被删除的记录并重建索引文件。

　　　　如果前面的步骤失败，使用myisamchk --safe-recover tbl_name 。安全恢复模式使用一个老的恢复方法，处理常规恢复模式不行的少数情况( 但是更慢) 。

　　　　如果在修复时，你得到奇怪的错误( 例如out of memory 错误) ，或如果myisamchk 崩溃，到阶段3 。 

　　阶段3 ：困难的修复

　　　　只有在索引文件的第一个16K 块被破坏，或包含不正确的信息，或如果索引文件丢失，你才应该到这个阶段。在这种情况下，需要创建一个新的索引文件。按如下步骤操做：

　　　　把数据文件移到安全的地方。

　　　　使用表描述文件创建新的( 空) 数据文件和索引文件：

　　　　shell> mysql db_name

　　　　mysql> SET AUTOCOMMIT=1;

　　　　mysql> TRUNCATE TABLE tbl_name;

　　　　mysql> quit 

　　　　如果你的MySQL 版本没有TRUNCATE TABLE ，则使用DELETE FROM tbl_name 。

　　　　将老的数据文件拷贝到新创建的数据文件之中。（不要只是将老文件移回新文件之中；你要保留一个副本以防某些东西出错。）

　　　　回到阶段2 。现在myisamchk -r -q 应该工作了。（这不应该是一个无限循环）。

　　　　你还可以使用REPAIR TABLE tbl_name USE_FRM ，将自动执行整个程序。

　　阶段4 ：非常困难的修复

　　　　只有.frm 描述文件也破坏了，你才应该到达这个阶段。这应该从未发生过，因为在表被创建以后，描述文件就不再改变了。

 　　　　从一个备份恢复描述文件然后回到阶段3 。你也可以恢复索引文件然后回到阶段2 。对后者，你应该用myisamchk -r 启动。

　　　　如果你没有进行备份但是确切地知道表是怎样创建的，在另一个数据库中创建表的一个拷贝。删除新的数据文件，然后从其他数据库将描述文件和索引文件移到破坏的数据库中。这样提供了新的描述和索引文件，但是让.MYD 数据文件独自留下来了。回到阶段2 并且尝试重建索引文件。
Log Sequence Number, LSN

Sharp Checkpoint 是一次性将 buffer pool 中的所有脏页都刷新到磁盘的数据文件，同时会保存最后一个提交的事务LSN。


fuzzy checkpoint就更加复杂了，它是在固定的时间点发生，除非他已经将所有的页信息刷新到了磁盘，或者是刚发生过一次sharp checkpoint，fuzzy checkpoint发生的时候会记录两次LSN，也就是检查点发生的时间和检查点结束的时间。但是呢，被刷新的页在并不一定在某一个时间点是一致的，这也就是它为什么叫fuzzy的原因。较早刷入磁盘的数据可能已经修改了，较晚刷新的数据可能会有一个比前面LSN更新更小的一个LSN。fuzzy checkpoint在某种意义上可以理解为fuzzy checkpoint从redo  log的第一个LSN执行到最后一个LSN。恢复以后的话，REDO LOG就会从最后一个检查点开始时候记录的LSN开始。

一般情况下大家可能fuzzy checkpoint的发生频率会远高于sharp checkpoint发生的频率，这个事毫无疑问的。不过当数据库关闭，切换redo日志文件的时候是会触发sharp checkpoint，一般情况是fuzzy checkpoint发生的更多一些。

一般情况下，执行普通操作的时候将不会发生检查点的操作，但是，fuzzy checkpoint却要根据时间推进而不停的发生。刷新脏页已经成为了数据库的一个普通的日常操作。

INNODB维护了一个大的缓冲区，以保证被修改的数据不会被立即写入磁盘。她会将这些修改过的数据先保留在buffer pool当中，这样在这些数据被写入磁盘以前可能会经过多次的修改，我们称之为写结合。这些数据页在buffer pool当中都是按照list来管理的，free list会记录那些空间是可用的，LRU list记录了那些数据页是最近被访问到的。flush list则记录了在LSN顺序当中的所有的dirty page信息，最近最少修改信息。

这里着重看一下flush list，我们知道innodb的缓存空间是有限的。如果buffer pool空间使用完毕，再次读取新数据就会发生磁盘读，也就是会发生flush操作，所以说就要释放一部分没有被使用的空间来保证buffer pool的可用性。由于这样的操作是很耗时的，所以说INNODB是会连续按照时间点去执行刷新操作，这样就保证了又足够的clean page来作为交换，而不必发生flush操作。每一次刷新都会将flush list的最老的信息驱逐，这样才能够保证数据库缓冲命中率是很高的一个值。这些老数据的选取是根据他们在磁盘的位置和LSN（最后一次修改的）号来确认数据新旧。

MySQL数据的日志都是混合循环使用的，但是如果这些事物记录的页信息还没有被刷新到磁盘当中的话是绝对不会被覆盖写入的。如果还没被刷新入磁盘的数据被覆盖了日志文件，那数据库宕机的话岂不是所有被覆盖写入的事物对应的数据都要丢失了呢。因此，数据修改也是有时间限制的，因为新的事物或者正在执行的事物也是需要日志空间的。日志越大，限制就越小。而且每次fuzzy checkpoint都会将最老最不被访问的数据驱逐出去，这也保证了每次驱逐的都是最老的数据，在下次日志被覆盖写入的时候都是已经被刷盘的数据的日志信息。最后一个老的,不被访问的数据的事物的LSN就是事务日志的 low-water标记，INNODB一直想提高这个LSN的值以保证buffer pool又足够的空间刷入新的数据，同时保证了数据库事务日志文件可以被覆盖写入的时候有足够的空间使用。将事务日志设置的大一些能够降低释放日志空间的紧迫性，从而可以大大的提高性能。

当innodb刷新 dirty page落盘的时候，他会找到最老的dirty  page对应的LSN并且将其标记为low-water，然后将这些信息记录到事物日志的头部，因此，每次刷新脏页都是要从flush  list的头部进行刷新的。在推进最老的LSN的标记位置的时候，本质上就是做了一次检查点。

当INNODB宕机的时候，他还要做一些额外的操作，第一：停止所有的数据更新等操作，第二：将dirty page in  buffer 的数据刷新落盘，第三：记录最后的LSN，因为我们上面也说到了，这次发生的是sharp checkpoint，并且，这个LSN会写入到没一个数据库文件的头部，以此来标记最后发生检查点的时候的LSN位置。

我们知道，刷新脏页数据的频率如果越高的话就代表整个数据库的负载很大，越小当然代表数据库的压力会小一点。将LOG 文件设置的很大能够再检查点发生期间减少磁盘的IO,总大小最好能够设置为和buffer pool大小相同，当然如果日志文件设置太大的话MySQL就会再crash recovery的时候花费更多的时间（5.5之前）。


http://mysqlmusings.blogspot.com/2011/04/crash-safe-replication.html

http://apprize.info/php/effective/6.html

http://mysqllover.com/?p=620

http://tech.uc.cn/?p=716

http://mysql.taobao.org/monthly/2016/05/01/

http://mysql.taobao.org/monthly/2015/06/01/

http://mysql.taobao.org/monthly/2015/05/01/

http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

https://gold.xitu.io/entry/5841225561ff4b00587ec651

https://yq.aliyun.com/articles/64677?utm_campaign=wenzhang&utm_medium=article&utm_source=QQ-qun&utm_content=m_7935

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=696

http://mysqllover.com/?p=213

http://mysql.taobao.org/monthly/2016/02/03/

http://mysql.taobao.org/monthly/2016/08/07/

http://mysql.taobao.org/monthly/2015/12/01/

http://mysqllover.com/?p=834

http://mysqllover.com/?p=1087

http://www.xuchunyang.com/2016/01/13/deak_lock/

http://mysqllover.com/?p=1119

http://www.askmaclean.com/archives/mysql-recover-innodb.html

http://www.askmaclean.com/archives/mysql%e4%b8%ad%e6%81%a2%e5%a4%8d%e4%bf%ae%e5%a4%8dinnodb%e6%95%b0%e6%8d%ae%e5%ad%97%e5%85%b8.html

http://www.thinkphp.cn/code/430.html

http://www.cnblogs.com/liuhao/p/3714012.html

http://louisyang.blog.51cto.com/8381303/1360394

http://mysql.taobao.org/monthly/2015/05/01/

http://hamilton.duapp.com/detail?articleId=34

https://twindb.com/undrop-tool-for-innodb/

https://twindb.com/tag/stream_parser/

http://jishu.y5y.com.cn/aeolus_pu/article/details/60143284

http://hedengcheng.com/?p=148

read_view

http://www.cnblogs.com/chenpingzhao/p/5065316.html

http://kabike.iteye.com/blog/1820553

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://mysqllover.com/?p=696

http://imysql.com/2014/08/13/mysql-faq-howto-shutdown-mysqld-fulgraceful.shtml

http://11879724.blog.51cto.com/11869724/1872928

http://coolnull.com/3145.html

http://mysqllover.com/?p=594

http://mysqllover.com/?p=87

http://mysqllover.com/?p=581

http://keithlan.github.io/2016/06/23/gtid/

http://mysqlmusings.blogspot.com/2011/04/crash-safe-replication.html

https://dbarobin.com/2015/08/29/mysql-optimization-under-ssd/

http://www.orczhou.com/index.php/2010/12/more-about-mysql-innodb-shutdown/

https://www.slideshare.net/frogd/inno-db-15344119

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://github.com/zhaiwx1987/innodb_ebook/blob/master/innodb_adaptive_hash.md




隔离级别

详细可以查看row_search_for_mysql()中的实现，实际上也就是row_search_mvcc()函数的实现。

row_search_for_mysql()   
 |-row_search_no_mvcc()       # 对于MySQL内部使用的表(用户不可见)，不需要MVCC机制
 |-row_search_mvcc()

row_search_no_mvcc()用于MySQL的内部表使用，通常是一些作为一个较大任务的中间结果存储，所以希望其可以尽快处理，因此不需要MVCC机制。

事务的隔离级别在trx->isolation_level中定义，其取值也就是如下的宏定义。

#define TRX_ISO_READ_UNCOMMITTED        0
#define TRX_ISO_READ_COMMITTED          1
#define TRX_ISO_REPEATABLE_READ         2
#define TRX_ISO_SERIALIZABLE            3


在不同的隔离级别下，可见性的判断有很大的不同。

READ-UNCOMMITTED
在该隔离级别下会读到未提交事务所产生的数据更改，这意味着可以读到脏数据，实际上你可以从函数row_search_mvcc中发现，当从btree读到一条记录后，如果隔离级别设置成READ-UNCOMMITTED，根本不会去检查可见性或是查看老版本。这意味着，即使在同一条SQL中，也可能读到不一致的数据。

    READ-COMMITTED
    在该隔离级别下，可以在SQL级别做到一致性读，当事务中的SQL执行完成时，ReadView被立刻释放了，在执行下一条SQL时再重建ReadView。这意味着如果两次查询之间有别的事务提交了，是可以读到不一致的数据的。

    REPEATABLE-READ
    可重复读和READ-COMMITTED的不同之处在于，当第一次创建ReadView后（例如事务内执行的第一条SEELCT语句），这个视图就会一直维持到事务结束。也就是说，在事务执行期间的可见性判断不会发生变化，从而实现了事务内的可重复读。

    SERIALIZABLE
    序列化的隔离是最高等级的隔离级别，当一个事务在对某个表做记录变更操作时，另外一个查询操作就会被该操作堵塞住。同样的，如果某个只读事务开启并查询了某些记录，那么另外一个session对这些记录的更改操作是被堵塞的。内部的实现其实很简单：
        对InnoDB表级别加LOCK_IS锁，防止表结构变更操作
        对查询得到的记录加LOCK_S共享锁，这意味着在该隔离级别下，读操作不会互相阻塞。而数据变更操作通常会对记录加LOCK_X锁，和LOCK_S锁相冲突，InnoDB通过给查询加记录锁的方式来保证了序列化的隔离级别。

注意不同的隔离级别下，数据具有不同的隔离性，甚至事务锁的加锁策略也不尽相同，你需要根据自己实际的业务情况来进行选择。





SELECT count_star, sum_timer_wait, avg_timer_wait, event_name FROM events_waits_summary_global_by_event_name WHERE count_star > 0 AND event_name LIKE "wait/synch/%" ORDER BY sum_timer_wait DESC LIMIT 20;




最新的事务ID通过trx_sys_get_new_trx_id()函数获取，每次超过了TRX_SYS_TRX_ID_WRITE_MARGIN次数后，都会调用trx_sys_flush_max_trx_id()函数刷新磁盘。






innodb_force_recovery变量对应源码中的srv_force_recovery变量，
 


当innodb_fast_shutdown设置为0时，会导致purge一直工作近两个小时。？？？？？

从5.5版本开始，purge任务从主线程中独立出来；5.6开始支持多个purge线程，可以通过innodb_purge_threads变量控制。

purge后台线程的最大数量可以有32个，包括了一个coordinator线程，以及多个worker线程。

在innobase_start_or_create_for_mysql()函数中，会创建srv_purge_coordinator_thread以及srv_worker_thread线程。


srv_purge_coordinator_thread()
 |-srv_purge_coordinator_suspend()   如果不需要purge或者上次purge记录数为0，则暂停
 |-srv_purge_should_exit()           判断是否需要退出；fast_shutdown=0则等待所有purge操作完成
 |-srv_do_purge()                    协调线程的主要工作，真正调用执行purge操作的函数
 |
 |-trx_purge()                       防止上次循环结束后又新的记录写入，此处不再使用worker线程
 |
 |-trx_purge()                       最后对history-list做一次清理，确保所有worker退出

srv_worker_thread()


最后一次做trx_purge()时，为了防止执行时间过程，批量操作时不再采用innodb_purge_batch_size(300)指定的值，而是采用20。


InnoDB的数据组织方式采用聚簇索引，也就是索引组织表，而二级索引采用(索引键值,主键键值)组合来唯一确定一条记录。
无论是聚簇索引，还是二级索引，每条记录都包含了一个DELETED-BIT位，用于标识该记录是否是删除记录；除此之外，聚簇索引还有两个系统列：DATA_TRX_ID，DATA_ROLL_PTR，分别表示产生当前记录项的事务ID以及指向当前记录的undo信息。



从聚簇索引行结构，与二级索引行结构可以看出，聚簇索引中包含版本信息(事务号+回滚指针)，二级索引不包含版本信息，二级索引项的可见性如何判断？？？？


InnoDB存储引擎在开始一个RR读之前，会创建一个Read View。Read View用于判断一条记录的可见性。Read View定义在read0read.h文件中，其中最主要的与可见性相关的属性如下：

class ReadView {
private:
  trx_id_t        m_low_limit_id;  //
};


ReadView::prepare()


copy_trx_ids
mtr_commit(struct mtr_t*)                 提交一个mini-transaction，调用mtr_t::commit()
 |-mtr_t::Command::execute()              写redo-log，将脏页添加到flush-list，并释放占用资源
   |-mtr_t::Command::prepare_write()      准备写入日志
   | |-fil_names_write_if_was_clean()
   |-mtr_t::Command::finish_write()   


测试场景#1 Drop Database (innodb_file_per_table=ON)
OFF代表MySQL是共享表空间，也就是所有库的数据都存放在一个ibdate1文件中；ON代表每个表的存储空间都是独立的。

ibd是MySQL数据文件、索引文件，二进制文件无法直接读取；frm是表结构文件，可以直接打开。如果innodb_file_per_table 无论是ON还是OFF，都会有这2个文件，区别只是innodb_file_per_table为ON的时候，数据时放在 .idb中，如果为OFF则放在ibdata1中。






 


























#######redo-log文件
redo-log保存在innodb_log_group_home_dir参数指定的目录下，文件名为ib_logfile*；undo保存在共享表空间ibdata*文件中。

InnoDB的redo log可控制文件大小以及文件个数，分别通过innodb_log_file_size和innodb_log_files_in_group控制，总大小为两者之积。日志顺序写入，而且文件循环使用。

简单来说，InnoDB中的两个核心参数innodb_buffer_pool_size、innodb_log_file_size，分别定义了数据缓存和redo-log的大小，而后者的大小也决定了可以允许buffer中可以有多少脏页。当然，也不能因此就增大redo-log文件的大小，如果这样，可能会导致系统启动时Crash Recovery时间增大。




LSN对应了日志文件的偏移量，为了减小故障恢复时间，引入了Checkpoint机制，

InnoDB在启动时会自动检测InnoDB数据和事务日志是否一致，是否需要执行相应的操作？？？保证数据一致性；当然，故障恢复时间与事务日志的大小相关。


checkpoint会将最近写入的LSN


主线程主要完成 purge、checkpoint、dirty pages flush 等操作。



Database was not shutdown normally!   # InnoDB开始Crash Recovery{recv_init_crash_recovery_spaces()}
Starting crash recovery.



1. 读取Checkpoint LSN
2. 从Checkpoint LSN开始向前遍历Redo Log File
   重做从Checkpoint LSN开始的所有Redo日志
3. 重新构造系统崩溃时的事务
   Commit事务，等待Purge线程回收
   Prepare事务，由MySQL Server控制提交或者回滚(与Binlog 2PC相关)
   Active事务，回滚
4. 新建各种后台线程，Crash Recovery完成返回


正常关闭时，会在flush redo log和脏页后，做一次完全同步的checkpoint，并将checkpoint的LSN写到第一个ibdata文件的第一个page中，详细可以参考fil_write_flushed_lsn()。





http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

http://mysql.taobao.org/monthly/2015/05/01/

http://mysql.taobao.org/monthly/2016/05/01/

http://tech.uc.cn/?p=716

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=620

http://apprize.info/php/effective/6.html

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

数据库内核分享

https://www.slideshare.net/frogd/inno-db-15344119

检查保存到磁盘的最大checkpoint LSN与redo-log的LSN是否一致；


MySQL · 引擎特性 · InnoDB 崩溃恢复过程

http://mysql.taobao.org/monthly/2015/06/01/













https://blogs.oracle.com/mysqlinnodb/entry/repeatable_read_isolation_level_in



http://mysql.taobao.org/monthly/2015/12/01/
http://hedengcheng.com/?p=148
read_view
http://www.cnblogs.com/chenpingzhao/p/5065316.html
http://kabike.iteye.com/blog/1820553
http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/
http://mysqllover.com/?p=696

隔离级别
详细可以查看row_search_mvcc()中的实现


row_search_for_mysql()
 |-row_search_no_mvcc()       # 对于MySQL内部使用的表(用户不可见)，不需要MVCC机制
 |-row_search_mvcc()

row_search_no_mvcc()用于MySQL的内部表使用，通常是一些作为一个较大任务的中间结果存储，所以希望其可以尽快处理，因此不需要MVCC机制。


innodb_force_recovery变量对应源码中的srv_force_recovery变量，




当innodb_fast_shutdown设置为0时，会导致purge一直工作近两个小时。？？？？？

从5.5版本开始，purge任务从主线程中独立出来；5.6开始支持多个purge线程，可以通过innodb_purge_threads变量控制。

purge后台线程的最大数量可以有32个，包括了一个coordinator线程，以及多个worker线程。

在innobase_start_or_create_for_mysql()函数中，会创建srv_purge_coordinator_thread以及srv_worker_thread线程。


srv_purge_coordinator_thread()
 |-srv_purge_coordinator_suspend()   如果不需要purge或者上次purge记录数为0，则暂停
 |-srv_purge_should_exit()           判断是否需要退出；fast_shutdown=0则等待所有purge操作完成
 |-srv_do_purge()                    协调线程的主要工作，真正调用执行purge操作的函数
 |
 |-trx_purge()                       防止上次循环结束后又新的记录写入，此处不再使用worker线程
 |
 |-trx_purge()                       最后对history-list做一次清理，确保所有worker退出

srv_worker_thread()


最后一次做trx_purge()时，为了防止执行时间过程，批量操作时不再采用innodb_purge_batch_size(300)指定的值，而是采用20。


InnoDB的数据组织方式采用聚簇索引，也就是索引组织表，而二级索引采用(索引键值,主键键值)组合来唯一确定一条记录。
无论是聚簇索引，还是二级索引，每条记录都包含了一个DELETED-BIT位，用于标识该记录是否是删除记录；除此之外，聚簇索引还有两个系统列：DATA_TRX_ID，DATA_ROLL_PTR，分别表示产生当前记录项的事务ID以及指向当前记录的undo信息。



从聚簇索引行结构，与二级索引行结构可以看出，聚簇索引中包含版本信息(事务号+回滚指针)，二级索引不包含版本信息，二级索引项的可见性如何判断？？？？


InnoDB存储引擎在开始一个RR读之前，会创建一个Read View。Read View用于判断一条记录的可见性。Read View定义在read0read.h文件中，其中最主要的与可见性相关的属性如下：

class ReadView {
private:
  trx_id_t        m_low_limit_id;  //
};


mtr_commit(struct mtr_t*)                 提交一个mini-transaction，调用mtr_t::commit()
 |-mtr_t::Command::execute()              写redo-log，将脏页添加到flush-list，并释放占用资源
   |-mtr_t::Command::prepare_write()      准备写入日志
   | |-fil_names_write_if_was_clean()
   |-mtr_t::Command::finish_write()












妈的文件整理文件

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://www.cnblogs.com/liuhao/p/3714012.html


持续集成 https://www.zhihu.com/question/23444990




buf_flush_batch









FAQ系列 | 如何避免ibdata1文件大小暴涨

0、导读

    遇到InnoDB的共享表空间文件ibdata1文件大小暴增时，应该如何处理？

1、问题背景

用MySQL/InnoDB的童鞋可能也会有过烦恼，不知道为什么原因，ibdata1文件莫名其妙的增大，不知道该如何让它缩回去，就跟30岁之后男人的肚腩一样，汗啊，可喜可贺的是我的肚腩还没长出来，hoho~

正式开始之前，我们要先知道ibdata1文件是干什么用的。

ibdata1文件是InnoDB存储引擎的共享表空间文件，该文件中主要存储着下面这些数据：

        data dictionary
        double write buffer
        insert buffer/change buffer
        rollback segments
        undo space
        Foreign key constraint system tables

另外，当选项 innodb_file_per_table = 0 时，在ibdata1文件中还需要存储 InnoDB 表数据&索引。ibdata1文件从5.6.7版本开始，默认大小是12MB，而在这之前默认大小是10MB，其相关选项是 innodb_data_file_path，比如我一般是这么设置的：

    innodb_data_file_path = ibdata1:1G:autoextend

当然了，无论是否启用了 innodb_file_per_table = 1，ibdata1文件都必须存在，因为它必须存储上述 InnoDB 引擎所依赖&必须的数据，尤其是上面加粗标识的 rollback segments 和 undo space，它俩是引起 ibdata1 文件大小增加的最大原因，我们下面会详细说。
2、原因分析

我们知道，InnoDB是支持MVCC的，它和ORACLE类似，采用 undo log、redo log来实现MVCC特性的。在事务中对一行数据进行修改时，InnoDB 会把这行数据的旧版本数据存储一份在undo log中，如果这时候有另一个事务又要修改这行数据，就又会把该事物最新可见的数据版本存储一份在undo log中，以此类推，如果该数据当前有N个事务要对其进行修改，就需要存储N份历史版本（和ORACLE略有不同的是，InnoDB的undo log不完全是物理block，主要是逻辑日志，这个可以查看 InnoDB 源码或其他相关资料）。这些 undo log 需要等待该事务结束后，并再次根据事务隔离级别所决定的对其他事务而言的可见性进行判断，确认是否可以将这些 undo log 删除掉，这个工作称为 purge（purge 工作不仅仅是删除过期不用的 undo log，还有其他，以后有机会再说）。

那么问题来了，如果当前有个事务中需要读取到大量数据的历史版本，而该事务因为某些原因无法今早提交或回滚，而该事务发起之后又有大量事务需要对这些数据进行修改，这些新事务产生的 undo log 就一直无法被删除掉，形成了堆积，这就是导致 ibdata1 文件大小增大最主要的原因之一。这种情况最经典的场景就是大量数据备份，因此我们建议把备份工作放在专用的 slave server 上，不要放在 master server 上。

另一种情况是，InnoDB的 purge 工作因为本次 file i/o 性能是在太差或其他的原因，一直无法及时把可以删除的 undo log 进行purge 从而形成堆积，这是导致 ibdata1 文件大小增大另一个最主要的原因。这种场景发生在服务器硬件配置比较弱，没有及时跟上业务发展而升级的情况。

比较少见的一种是在早期运行在32位系统的MySQL版本中存在bug，当发现待 purge 的 undo log 总量超过某个值时，purge 线程直接放弃抵抗，再也不进行 purge 了，这个问题在我们早期使用32位MySQL 5.0版本时遇到的比较多，我们曾经遇到这个文件涨到100多G的情况。后来我们费了很大功夫把这些实例都迁移到64位系统下，终于解决了这个问题。

最后一个是，选项 innodb_data_file_path 值一开始就没调整或者设置很小，这就必不可免导致 ibdata1 文件增大了。Percona官方提供的 my.cnf 参考文件中也一直没把这个值加大，让我百思不得其解，难道是为了像那个经常被我吐槽的xx那样，故意留个暗门，好方便后续帮客户进行优化吗？（我心理太阴暗了，不好不好~~）

稍微总结下，导致ibdata1文件大小暴涨的原因有下面几个：

        有大量并发事务，产生大量的undo log；
        有旧事务长时间未提交，产生大量旧undo log；
        file i/o性能差，purge进度慢；
        初始化设置太小不够用；
        32-bit系统下有bug。

稍微题外话补充下，另一个热门数据库 PostgreSQL 的做法是把各个历史版本的数据 和 原数据表空间 存储在一起，所以不存在本案例的问题，也因此 PostgreSQL 的事务回滚会非常快，并且还需要定期做 vaccum 工作（具体可参见PostgreSQL的MVCC实现机制，我可能说的不是完全正确哈）
3、解决方法建议

看到上面的这些问题原因描述，有些同学可能觉得这个好办啊，对 ibdata1 文件大小进行收缩，回收表空间不就结了吗。悲剧的是，截止目前，InnoDB 还没有办法对 ibdata1 文件表空间进行回收/收缩，一旦 ibdata1 文件的肚子被搞大了，只能把数据先备份后恢复再次重新初始化实例才能恢复原先的大小，或者把依次把各个独立表空间文件备份恢复到一个新实例中，除此外，没什么更好的办法了。

当然了，这个问题也并不是不能防范，根据上面提到的原因，相应的建议对策是：

        升级到5.6及以上（64-bit），采用独立undo表空间，5.6版本开始就支持独立的undo表空间了，再也不用担心会把 ibdata1 文件搞大；
        初始化设置时，把 ibdata1 文件至少设置为1GB以上；
        增加purge线程数，比如设置 innodb_purge_threads = 8；
        提高file i/o能力，该上SSD的赶紧上；
        事务及时提交，不要积压；
        默认打开autocommit = 1，避免忘了某个事务长时间未提交；
        检查开发框架，确认是否设置了 autocommit=0，记得在事务结束后都有显式提交或回滚。



关于MySQL的方方面面大家想了解什么，可以直接留言回复，我会从中选择一些热门话题进行分享。 同时希望大家多多转发，多一些阅读量是老叶继续努力分享的绝佳助力，谢谢大家 :)

最后打个广告，运维圈人士专属铁观音茶叶微店上线了，访问：http://yejinrong.com 获得专属优惠




MySQL-5.7.7引入的一个系统库sys-schema，包含了一系列视图、函数和存储过程，主要是一些帮助MySQL用户分析问题和定位问题，可以方便查看哪些语句使用了临时表，哪个用户请求了最多的io，哪个线程占用了最多的内存，哪些索引是无用索引等。

其数据均来自performance schema和information schema中的统计信息。

MySQL 5.7.7 and higher includes the sys schema, a set of objects that helps DBAs and developers interpret data collected by the Performance Schema. sys schema objects can be used for typical tuning and diagnosis use cases.

MySQL Server blog中有一个很好的比喻：

For Linux users I like to compare performance_schema to /proc, and SYS to vmstat.

也就是说，performance schema和information schema中提供了信息源，但是，没有很好的将这些信息组织成有用的信息，从而没有很好的发挥它们的作用。而sys schema使用performance schema和information schema中的信息，通过视图的方式给出解决实际问题的答案。

查看是否安装成功
select * from sys.version;
查看类型
select * from sys.schema_object_overview where db='sys';
当然，也可以通过如下命令查看
show full tables from sys
show function status where db = 'sys';
show procedure status where db = 'sys'

user/host资源占用情况
SHOW TABLES FROM `sys` WHERE
    `Tables_in_sys` LIKE 'user\_%' OR
 `Tables_in_sys` LIKE 'host\_%'
IO资源使用，包括最近IO使用情况latest_file_io
SHOW TABLES LIKE 'io\_%'
schema相关，包括表、索引使用统计
SHOW TABLES LIKE 'schema\_%'
等待事件统计
SHOW TABLES LIKE 'wait%'
语句查看，包括出错、全表扫描、创建临时表、排序、空闲超过95%
SHOW TABLES LIKE 'statement%'
当前正在执行链接，也就是processlist
其它还有一些厂家的帮助函数，PS设置。
https://www.slideshare.net/Leithal/the-mysql-sys-schema
http://mingxinglai.com/cn/2016/03/sys-schema/
http://www.itpub.net/thread-2083877-1-1.html

x$NAME保存的是原始数据，比较适合通过工具调用；而NAME表更适合阅读，比如使用命令行去查看。


select digest,digest_text from performance_schema.events_statements_summary_by_digest\G
CALL ps_trace_statement_digest('891ec6860f98ba46d89dd20b0c03652c', 10, 0.1, TRUE, TRUE);
CALL ps_trace_thread(25, CONCAT('/tmp/stack-', REPLACE(NOW(), ' ', '-'), '.dot'), NULL, NULL, TRUE, TRUE, TRUE);

优化器调优
https://dev.mysql.com/doc/internals/en/optimizer-tracing.html


MySQL performance schema instrumentation interface(PSI)

struct PFS_instr_class {}; 基类


通过class page_id_t区分页，

class page_id_t {
private:
    ib_uint32_t     m_space;     指定tablespace
    ib_uint32_t     m_page_no;   页的编号






http://www.myexception.cn/database/511937.html
http://blog.csdn.net/taozhi20084525/article/details/17613785
http://blogread.cn/it/article/5367
http://mysqllover.com/?p=303
http://www.cnblogs.com/chenpingzhao/p/5107480.html ？？？
https://docs.oracle.com/cd/E17952_01/mysql-5.7-en/innodb-recovery-tablespace-discovery.html
http://mysqllover.com/?p=1214



[mysqld]
innodb_data_file_path            = ibdata1:12M;ibdata2:12M:autoextend





<!--
Select_scan

http://wangyuanzju.blog.163.com/blog/static/13029200671942219943/

http://fengbin2005.iteye.com/blog/1738292

http://doc.haohtml.com/database/mysql/MySQL%CA%FD%BE%DD%BF%E2%D0%D4%C4%DC%BC%E0%BF%D8%D3%EB%D5%EF%B6%CF.pdf

http://www.docin.com/p-398501099.html

http://hidba.org/?p=170
-->



http://www.ruanyifeng.com/blog/2013/05/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm.html
http://www.maomao365.com/?p=3043

UNIX 域套接字用于在同一台机器上进程之间通信，与因特网域套接字类似，不过 UNIX Socket 更加高效，只需要复制数据，不执行协议处理，不需要添加或删除网络报头，无需计算检验和，不要产生顺序号，无需发送确认报文。

提供了流和数据报两种接口，可以使用面向网络的域套接字接口，也可使用 socketpair() 函数。
https://akaedu.github.io/book/ch37s04.html
http://www.cnblogs.com/nufangrensheng/p/3569416.html
http://blog.csdn.net/jnu_simba/article/details/9079359
http://liulixiaoyao.blog.51cto.com/1361095/533469/
http://www.infoq.com/cn/news/2012/12/twemproxy

typesdb 中如果不存在则会直接报 `Dataset not found` 错，然后退出，通过 type 做作为 key 保存在 AVL 中。

twemproxy后端支持多个server pool，为每个server pool分配一个监听端口用于接收客户端的连接。

客户端和 proxy 建立连接 (CLI_CONN)，然后发送请求，proxy 读取数据并放入到 req_msg 中，消息的 owner 为 (CLI_CONN)；
Proxy 根据策略从 server pool 中选取一个 server 并且建立连接 (SVR_CONN) 然后准备开始转发；
将 req_msg 的指针放入 CLI_CONN 的 output 队列中，同时放入 SVR_CONN 的 input 队列中，然后触发 SVR_CONN 的写事件；
SVR_CONN 的写回调函数从 input 队列中取出 req_msg 发送给对应的后端 server，并将 req_msg 放入 server_conn 的 output 队列；
收到从后端服务器发送的响应消息 rsp_msg 后，依次调用 rsp_filter(判断消息是否为空、是否消息可以不用回复等)和rsp_forward；
将 req_msg 从 SVR_CONN 的 output 队列中取出，建立 req_msg 和 rsp_msg 的对应关系(通过msg的peer字段)，通过req_msg的owner找到client_conn，然后启动client_conn的写事件；
client_conn的写回调函数从client_conn的output队列中取出req_msg，然后通过peer字段拿到对应的rsp_msg，将其发出去。
http://www.cnblogs.com/foxmailed/p/3623817.html
http://www.yeolar.com/note/topics/twemproxy/
http://blog.sina.com.cn/s/blog_4f8ea2ef0101iill.html
vim-minimal不需要删除  http://www.madirish.net/294  http://blog.itpub.net/28588485/viewspace-755403/


#include <stdio.h>
int main(int argc, char *argv[])
{
  unsigned long long x = 0;
  printf("g: %g,  f: %f\n", x, x);
  printf("sizeof: float(%d), double(%d), unsigned long long(%d)\n",
    sizeof(float), sizeof(double), sizeof(unsigned long long));
  return 0;
}
for (int i = 0; i < ds->ds_num; ++i) {
  INFO(">>>>>>> %s %s %s %f\n", vl->type, vl->type_instance, ds->ds[i].name, vl->values[i].gauge);
}




惊群问题讨论
http://www.voidcn.com/blog/liujiyong7/article/p-377809.html
Heap数据结构(栈)
http://www.cnblogs.com/gaochundong/p/binary_heap.html
http://www.cnblogs.com/skywang12345/p/3610187.html


AVL数
https://courses.cs.washington.edu/courses/cse373/06sp/handouts/lecture12.pdf
https://www.cise.ufl.edu/~nemo/cop3530/AVL-Tree-Rotations.pdf
http://www.cnblogs.com/zhoujinyi/p/6497231.html

https://dev.mysql.com/doc/refman/5.7/en/backup-policy.html
https://dev.mysql.com/doc/refman/5.7/en/point-in-time-recovery.html

https://www.unixhot.com/page/ops






非常经典的《Linux平台下的漏洞分析入门 》
https://github.com/1u4nx/Exploit-Exercises-Nebula
原文在这里
https://www.mattandreko.com/

http://hustcat.github.io/iostats/
http://ykrocku.github.io/blog/2014/04/11/diskstats/
http://www.udpwork.com/item/12931.html

FIXME:
  linux-monitor-io.html
/proc/diskstats 中包括了主设备号、次设备号和设备名称，剩余的各个字段的含义简单列举如下，详细可以查看内核文档 [I/O statistics fields](https://www.kernel.org/doc/Documentation/iostats.txt) 。

可以通过 grep diskstats 找到对应内核源码实现在 diskstats_show()@block/genhd.c 中。

获取源码 diskstats_show() + struct disk_stats 。

可以看到是通过 part_round_stats() 函数获取每个磁盘的最新统计信息，通过 struct hd_struct 中的 struct disk_stats *dkstats 结构体保存，然后利用 part_stat_read() 函数统计各个 CPU 的值 (如果是多核)。


在 Bash 编程时，经常需要切换目录，可以通过 pushd、popd、dirs 命令切换目录。

pushd  切换到参数指定的目录，并把原目录和当前目录压入到一个虚拟的堆栈中，不加参数则在最近两个目录间切换；
popd   弹出堆栈中最近的目录；
dirs   列出当前堆栈中保存的目录列表；
  -v 在目录前添加编号，每行显示一个目录；
  -c 清空栈；

切换目录时，会将上次目录保存在 $OLDPWD 变量中，与 "-" 相同，可以通过 cd - 切换回上次的目录。




安全渗透所需的工具
https://wizardforcel.gitbooks.io/daxueba-kali-linux-tutorial/content/2.html










http://fengyuzaitu.blog.51cto.com/5218690/1616268
http://www.runoob.com/python/os-statvfs.html
http://blog.csdn.net/papiping/article/details/6980573
http://blog.csdn.net/hepeng597/article/details/8925506








shell版本号比较
http://blog.topspeedsnail.com/archives/3999
https://www.netfilter.org/documentation/HOWTO/NAT-HOWTO-6.html
man 3 yum.conf 确认下YUM配置文件中的变量信息
https://unix.stackexchange.com/questions/19701/yum-how-can-i-view-variables-like-releasever-basearch-yum0


int lt_dlinit (void);
  初始化，在使用前调用，可以多次调用，正常返回 0 ；
const char * lt_dlerror (void);
  返回最近一次可读的错误原因，如果没有错误返回 NULL；
void * lt_dlsym (lt_dlhandle handle, const char *name);
  返回指向 name 模块的指针，如果没有找到则返回 NULL 。
lt_dlhandle lt_dlopen (const char *filename);
  加载失败返回 NULL，多次加载会返回相同的值；
int lt_dlclose (lt_dlhandle handle);
  模块的应用次数减一，当减到 0 时会自动卸载；成功返回 0 。

https://github.com/carpedm20/awesome-hacking
http://jamyy.us.to/blog/2014/01/5800.html




Page Cache
https://www.thomas-krenn.com/en/wiki/Linux_Page_Cache_Basics

Page Cache, the Affair Between Memory and Files

http://duartes.org/gustavo/blog/post/page-cache-the-affair-between-memory-and-files/

https://www.quora.com/What-is-the-major-difference-between-the-buffer-cache-and-the-page-cache

从free到page cache
http://www.cnblogs.com/hustcat/archive/2011/10/27/2226995.html



getaddrinfo()
http://www.cnblogs.com/cxz2009/archive/2010/11/19/1881693.html


#define unlikely(x) __builtin_expect((x),0)
http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html


libcurl使用
http://www.cnblogs.com/moodlxs/archive/2012/10/15/2724318.html




https://lwn.net/Articles/584225/
https://en.wikipedia.org/wiki/Stack_buffer_overflow#Stack_canaries
https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
https://outflux.net/blog/archives/2014/01/27/fstack-protector-strong/
-pipe
  从源码生成可执行文件一般需要四个步骤，并且还会产生中间文件，该参数用于配置实用PIPE，一些平台会失败，不过 GNU 不受影响。
-fexceptions
  打开异常处理，该选项会生成必要的代码来处理异常的抛出和捕获，对于 C++ 等会触发异常的语言来说，默认都会指定该选项。所生成的代码不会造成性能损失，但会造成尺寸上的损失。因此，如果想要编译不使用异常的 C++ 代码，可能需要指定选项 -fno-exceptions 。
-Wall -Werror -O2 -g --param=ssp-buffer-size=4 -grecord-gcc-switches -specs=/usr/lib/rpm/redhat/redhat-hardened-cc1  -m64 -mtune=generic -DLT_LAZY_OR_NOW="RTLD_LAZY|RTLD_GLOBAL"




关于Linux内核很不错的介绍
http://duartes.org/gustavo/blog/






编程时常有 Client 和 Server 需要各自得到对方 IP 和 Port 的需求，此时就可以通过 getsockname() 和 getpeername() 获取。

python -c '
import sys
import socket
s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("www.host.com", 80))
print s.getsockname()
s.close()'


Python判断IP有效性
https://gist.github.com/youngsterxyf/5088954



安全渗透工具集
https://wizardforcel.gitbooks.io/daxueba-kali-linux-tutorial/content/2.html

hostname获取方式，在启动时通过 1) global_option_get() 配置文件获取；2) gethostname()；3) getaddrinfo()。

#include <unistd.h>
int gethostname(char *name, size_t len);
  返回本地主机的标准主机名；正常返回 0 否则返回 -1，错误码保存在 errno 中。

#include <netdb.h>
#include <sys/socket.h>
struct hostent *gethostbyname(const char *name);
  用域名或主机名获取IP地址，注意只支持IPv4；正常返回一个 struct hostent 结构，否则返回 NULL。

#include<netdb.h>
int getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result);
  hostname: 一个主机名或者地址串，IPv4的点分十进制串或者IPv6的16进制串；
  service : 服务名可以是十进制的端口号，也可以是已定义的服务名称，如ftp、http等；
  hints   : 可以为空，用于指定返回的类型信息，例如，服务支持 TCP/UDP 那么，可以设置 ai_socktype 为 SOCK_DGRAM 只返回 UDP 信息；
  result  : 返回的结果。
  返回 0 成功。

struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;        // IP地址，需要通过inet_ntop()转换为IP字符串
    char            *ai_canonname;   // 返回的主机名
    struct addrinfo *ai_next;
};
http://blog.csdn.net/a_ran/article/details/41871437


const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
  将类型为af的网络地址结构src，转换成主机序的字符串形式，存放在长度为cnt的字符串中。返回指向dst的一个指针。如果函数调用错误，返回值是NULL。



对于不信任的组件建议使用后者，因为 ldd 可能会加载后显示依赖的库，从而导致安全问题。

----- 查看依赖的库
$ ldd /usr/bin/ssh
$ objdump -p /usr/bin/ssh | grep NEEDED

----- 运行程序加载的库
# pldd $(pidof mysqld)


# pldd $(pidof uagent)


VIRT, Virtual Memory Size @
  该任务的总的虚拟内存，包括了 code、data、shared libraries、换出到磁盘的页、已经映射但是没有使用的页。
USED, Memory in Use @
  包括了已使用的物理内存 RES ，以及换出到磁盘的内存 SWAP。
%MEM, Memory Usage(RES) @
  当前任务使用的内存与整个物理内存的占比。
CODE, Code Size
  可执行代码占用的物理内存数，也被称为 Text Resident Set, TRS。
DATA, Data+Stack Size
  除了代码之外的物理内存占用数，也就是 Data Resident Set, DRS 。
RES, Resident Memory Size @
  驻留在物理内存中的使用量。
SHR, Shared Memory Size @
  包括了共享内存以及共享库的数据。

SWAP, Swapped Size
  换出到磁盘的内存。
nMaj, nMin, nDRT


RES = CODE + DATA???? DATA太大了，为什么

====== ps
DRS, Data Resident Set <=> top(DATA) !!!
  除了代码之外的物理内存占用数。
RSS, Resident Set Size <=> top(RES)
  物理内存使用数。
TRS, Text Resident Set <=> top(CODE) !!!
  代码在内存中的占用数。
VSZ, Virtual Memory Size <=> top(VIRT) <=> pmap -d(mapped)
  虚拟内存的大小。

RES(top) 和 RSS(ps) 实际上读取的是 /proc/$(pidof process)/stat 或者 /proc/$(pidof process)/status statm。
pmap -d $(pidof uagent)
pmap -x $(pidof uagent)
ps -o pid,pmem,drs,trs,rss,vsz Hp `pidof uagent`

另外，cgtop 中显示的内存与什么相关？？？？
ps(TRS) 和 top(CODE) 的值不相同。

http://blog.csdn.net/u011547375/article/details/9851455
https://stackoverflow.com/questions/7594548/res-code-data-in-the-output-information-of-the-top-command-why
https://static.googleusercontent.com/media/research.google.com/zh-CN//pubs/archive/36669.pdf
https://landley.net/kdocs/ols/2010/ols2010-pages-245-254.pdf
LDD 跟 PMAP 加载的库不同？

awk 'BEGIN{sum=0};{if($3~/x/) {sum+=$2}};END{print sum}' /tmp/1

procs_refresh

Top用于查看Linux系统下进程信息，有时候需要选择显示那些列，以及按照某一列进行排序。查询整理如下：


top 除了默认的列之外，可以选择需要显示的列，操作如下：

----- 选择需要显示的列
1) 按 f 键进入选择界面；2) 方向键选择需要的列；3) 通过空格选择需要显示的列。

列显示位置调整：
执行top命令后，按 o 键，选择要调整位置的列（如K:CUP Usageage），按动一下大写K则显示位置往上调整，按动一下小写K则显示位置往下调整。

列排序：
执行top命令后，按 shift + f（小写），进入选择排序列页面，再按要排序的列的代表字母即可；

注册信号处理函数

setsid()
pidfile_create()

https://www.ibm.com/support/knowledgecenter/zh/ssw_aix_61/com.ibm.aix.genprogc/ie_prog_4lex_yacc.htm

flex 通过 yylval 将数据传递给 yacc；如果在 yacc 中使用了 ```%union``` ，那么各个条件的目的变量使用 yyval 。

https://www.howtoforge.com/storing-files-directories-in-memory-with-tmpfs



meminfo详解
https://lwn.net/Articles/28345/

ps的SIZE以及RSS不含部分的内存统计，所以要比pmap -d统计的RSS小。
The SIZE and RSS fields don't count some parts of a process including the page tables, kernel stack, struct thread_info
https://techtalk.intersec.com/2013/07/memory-part-2-understanding-process-memory/
http://tldp.org/LDP/tlk/mm/memory.html
http://tldp.org/LDP/khg/HyperNews/get/memory/linuxmm.html
https://lwn.net/Articles/230975/
https://gist.github.com/CMCDragonkai/10ab53654b2aa6ce55c11cfc5b2432a4
https://yq.aliyun.com/ziliao/75375
http://elinux.org/Runtime_Memory_Measurement
https://access.redhat.com/security/vulnerabilities/stackguard
http://events.linuxfoundation.org/sites/events/files/slides/elc_2016_mem_0.pdf
http://blog.csdn.net/lijzheng/article/details/23618365
https://yq.aliyun.com/articles/54405
https://stackoverflow.com/questions/31328349/stack-memory-management-in-linux
/post/mysql-parser.html
yyset_in() 设置入口


pmap -d $(pidof uagent)
pmap -x $(pidof uagent)

top -Hp $(pidof uagent)
ps -o pid,pmem,drs,trs,rss,vsz Hp `pidof uagent`
/proc/$(pidof uagent)/stat
/proc/$(pidof uagent)/status
/proc/$(pidof uagent)/maps
VmRSS、VmSize

$ ps aux|grep /usr/bin/X|grep -v grep | awk '{print $2}'   # 得出X server 的 pid   ...
1076
$ cat /proc/1076/stat | awk '{print $23 / 1024}'
139012
$ cat /proc/1076/status | grep -i vmsize
VmSize:      106516 kB
VmSize = memory + memory-mapped hardware (e.g. video card memory).

kmap 是用来建立映射的，映射后返回了被映射的高端内存在内核的线性地址
https://www.zhihu.com/question/30338816
http://blog.csdn.net/gatieme/article/details/52705142
http://www.cnblogs.com/zhiliao112/p/4251221.html
http://way4ever.com/?p=236
awk统计Linux最常用命令
http://www.ha97.com/3980.html
awk使用技巧
http://blog.csdn.net/ultrani/article/details/6750434
http://blog.csdn.net/u011204847/article/details/51205031 *****
http://ustb80.blog.51cto.com/6139482/1051310


关于smaps的详细介绍
https://jameshunt.us/writings/smaps.html
$ cat /proc/self/smaps  相比maps显示更详细信息
$ cat /proc/self/maps
address                  perms   offset   dev   inode       pathname
7f571af7a000-7f571af7d000 ---p 00000000 00:00 0
7f571af7d000-7f571b080000 rw-p 00000000 00:00 0             [stack:4714]
7f571b0ac000-7f571b0ad000 r--p 00021000 08:01 1838227       /usr/lib/ld-2.21.so
7ffe49dbd000-7ffe49dbf000 r-xp 00000000 00:00 0             [vdso]

列说明:
    starting address - ending address
    permissions
        r : read
        w : write
        x : execute
        s : shared
        p : private (copy on write)
    offset   : 如果不是file，则为0；
    device   : 如果是file，则是file所在device的major and monior device number，否则为00:00；
    inode    : 如果是file，则是file的inode number，否则为0；
    pathname : 有几种情况；
        file absolute path
        [stack]         the stack of the main process
        [stack:1001]    the stack of the thread with tid 1001
        [heap]
        [vdso] - virtual dynamic shared object, the kernel system call handler
        空白 -通常都是mmap创建的，用于其他一些用途的，比如共享内存


df -h
ls /dev/XXX -alh
echo $((0x0a))

print "Backed by file:\n";
print "  RO data                   r--  $mapped_rodata\n";
print "  Unreadable                ---  $mapped_unreadable\n"; 共享库同时存在？？？？
print "  Unknown                        $mapped_unknown\n";
print "Anonymous:\n";
print "  Writable code (stack)     rwx  $writable_code\n";
print "  Data (malloc, mmap)       rw-  $data\n";
print "  RO data                   r--  $rodata\n";
print "  Unreadable                ---  $unreadable\n";
print "  Unknown                        $unbacked_unknown\n";

16进制求和，都是16进制
awk --non-decimal-data  '{sum=($1 + $2); printf("0x%x %s\n", sum,$3)}'
strtonum("0x" $1)
echo $(( 16#a36b ))
echo "obase=2;256"|bc ibase base


print "Backed by file:\n";
print "  Unreadable                ---  $mapped_unreadable\n"; 共享库同时存在？？？？
print "  Unknown                        $mapped_unknown\n";
print "Anonymous:\n";
print "  Unreadable                ---  $unreadable\n";
print "  Unknown                        $unbacked_unknown\n";

代码
r-x 代码，包括程序(File)、共享库(File)、vdso(2Pages)、vsyscall(1Page)
rwx 没有，Backed by file: Write/Exec (jump tables); Anonymous: Writable code (stack)
r-- 程序中的只读数据，如字符串，包括程序(File)、共享库(File)
rw- 可读写变量，如全局变量；包括程序(File)、共享库(File)、stack、heap、匿名映射

静态数据、全局变量将保存在 ELF 的 .data 段中。
与smaps相关，以及一些实例
https://jameshunt.us/writings/smaps.html


各共享库的代码段，存放着二进制可执行的机器指令，是由kernel把该库ELF文件的代码段map到虚存空间；
各共享库的数据段，存放着程序执行所需的全局变量，是由kernel把ELF文件的数据段map到虚存空间；

用户代码段，存放着二进制形式的可执行的机器指令，是由kernel把ELF文件的代码段map到虚存空间；
用户数据段之上是代码段，存放着程序执行所需的全局变量，是由kernel把ELF文件的数据段map到虚存空间；

用户数据段之下是堆(heap)，当且仅当malloc调用时存在，是由kernel把匿名内存map到虚存空间，堆则在程序中没有调用malloc的情况下不存在；
用户数据段之下是栈(stack)，作为进程的临时数据区，是由kernel把匿名内存map到虚存空间，栈空间的增长方向是从高地址到低地址。

https://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto


另外，可以通过 ldd 查看对应的映射地址，在实际映射到物理内存时，会添加随机的变量，不过如上的各个共享库的地址是相同的。

可以通过 echo $(( 0x00007f194de48000 - 0x00007f194dc2c000)) 计算差值。


maps 文件对应了内核中的 show_map()

show_map()
 |-show_map_vma()

address                  perms   offset   dev   inode       pathname

http://duartes.org/gustavo/blog/post/how-the-kernel-manages-your-memory/


主要是anon中的rw属性导致
cat /proc/$(pidof uagent)/maps | grep stack | wc -l



Clean_pages 自从映射之后没有被修改的页；
Dirty_pages 反之；
RSS 包括了共享以及私有，Shared_Clean+Shared_Dirty、Private_Clean+Private_Dirty
PSS (Proportional set size) 包括了所有的私有页 (Private Pages) 以及共享页的平均值。例如，一个进程有100K的私有页，与一个进程有500K的共享页，与四个进程有500K的共享页，那么 PSS=100K+(500K/2)+(500K/5)=450K
USS (Unique set size) 私有页的和。

awk -f test.awk /proc/$(pidof uagent)/maps
#! /bin/awk -f
BEGIN {
    mapped_executable    = 0
    mapped_wrexec        = 0
    mapped_rodata        = 0
    mapped_rwdata        = 0
    mapped_unreadable    = 0
    mapped_unknown       = 0
    writable_code        = 0
    data                 = 0
    rodata               = 0
    unreadable           = 0
    vdso                 = 0
    unbacked_unknown     = 0
}

{
    split($1, addr, "-")
    pages = (strtonum("0x" addr[2]) - strtonum("0x" addr[1]))/4096
    if ( $4 == "00:00") {
        if      ( $2 ~ /rwx/ ) {     writable_code += pages }
        else if ( $2 ~ /rw-/ ) {              data += pages }
        else if ( $2 ~ /r-x/ ) {              vdso += pages }
        else if ( $2 ~ /r--/ ) {            rodata += pages }
        else if ( $2 ~ /---/ ) {        unreadable += pages }
        else                   {  unbacked_unknown += pages }
    } else {
        if      ( $2 ~ /rwx/ ) {     mapped_wrexec += pages }
        else if ( $2 ~ /rw-/ ) {     mapped_rwdata += pages }
        else if ( $2 ~ /r-x/ ) { mapped_executable += pages }
        else if ( $2 ~ /r--/ ) {     mapped_rodata += pages }
        else if ( $2 ~ /---/ ) { mapped_unreadable += pages }
        else                   {    mapped_unknown += pages }
    }
}
END {
    printf ("Backed by file:\n")
    printf ("  Write/Exec (jump tables)  rwx  %d\n", mapped_wrexec)
    printf ("  Data                      rw-  %d\n", mapped_rwdata)
    printf ("  Executable                r-x  %d\n", mapped_executable)
    printf ("  RO data                   r--  %d\n", mapped_rodata)
    printf ("  Unreadable                ---  %d\n", mapped_unreadable)
    printf ("  Unknown                        %d\n", mapped_unknown)
    printf ("Anonymous:\n")
    printf ("  Writable code (stack)     rwx  %d\n", writable_code)
    printf ("  Data (malloc, mmap)       rw-  %d\n", data)
    printf ("  vdso, vsyscall            r-x  %d\n", vdso)
    printf ("  RO data                   r--  %d\n", rodata)
    printf ("  Unreadable                ---  %d\n", unreadable)
    printf ("  Unknown                        %d\n", unbacked_unknown)
}

pmap -x $(pidof uagent) > /tmp/1
awk -f test.awk /tmp/1
#! /bin/awk -f
BEGIN {
    lib_dirty_rx         = 0
    lib_dirty_rw         = 0
    lib_dirty_r          = 0
    lib_dirty_unknown    = 0
    lib_rss_rx         = 0
    lib_rss_rw         = 0
    lib_rss_r          = 0
    lib_rss_unknown    = 0

    uagent_dirty_rx      = 0
    uagent_dirty_rw      = 0
    uagent_dirty_r       = 0
    uagent_dirty_unknown = 0
    uagent_rss_rx      = 0
    uagent_rss_rw      = 0
    uagent_rss_r       = 0
    uagent_rss_unknown = 0

    anon_dirty_rw        = 0
    anon_dirty_rx        = 0
    anon_dirty_unknown   = 0
    anon_dirty_r         = 0
    anon_rss_rw        = 0
    anon_rss_rx        = 0
    anon_rss_unknown   = 0
    anon_rss_r         = 0

    count          = 0
    actual          = 0
}

$NF ~ /(^lib|^ld)/ {
    if      ( $5 ~ /r-x/ ) {
        lib_rss_rx += $3
        lib_dirty_rx += $4
    } else if ( $5 ~ /rw-/ ) {
        lib_rss_rw += $3
        lib_dirty_rw += $4
    } else if ( $5 ~ /r--/ ) {
        lib_rss_r += $3
        lib_dirty_r += $4
    } else {
        lib_rss_unknown += $3
        lib_dirty_unknown += $4
    }
    count += $3
}
$NF ~ /([a-zA-Z]+\.so$|^uagent\>)/ {
    if      ( $5 ~ /r-x/ ) {
        uagent_rss_rx += $3
        uagent_dirty_rx += $4
    } else if ( $5 ~ /rw-/ ) {
        uagent_rss_rw += $3
        uagent_dirty_rw += $4
    } else if ( $5 ~ /r--/ ) {
        uagent_rss_r += $3
        uagent_dirty_r += $4
    } else {
        uagent_rss_unknown += $3
        uagent_dirty_unknown += $4
    }
    count += $3
}
$NF ~ /^]$/ {
    if      ( $5 ~ /r-x/ ) {
        anon_rss_rx += $3
        anon_dirty_rx += $4
    } else if ( $5 ~ /rw-/ ) {
        anon_rss_rw += $3
        anon_dirty_rw += $4
    } else if ( $5 ~ /r--/ ) {
        anon_rss_r += $3
        anon_dirty_r += $4
    } else {
        anon_rss_unknown += $3
        anon_dirty_unknown += $4
    }
    count += $3
}
$1 ~ /^total\>/ {
    actual = $4
}
END {
    printf ("Libraries info:\n")
    printf (" Perm        RSS   Dirty\n")
    printf ("  r-x        %5d     %5d\n", lib_rss_rx, lib_dirty_rx)
    printf ("  rw-        %5d     %5d\n", lib_rss_rw, lib_dirty_rw)
    printf ("  r--        %5d     %5d\n", lib_rss_r,  lib_dirty_r)
    printf ("  Unknown    %5d     %5d\n", lib_rss_unknown, lib_dirty_unknown)

    printf ("Uagent info:\n")
    printf (" Perm        RSS   Dirty\n")
    printf ("  r-x        %5d     %5d\n", uagent_rss_rx,      uagent_dirty_rx)
    printf ("  rw-        %5d     %5d\n", uagent_rss_rw,      uagent_dirty_rw)
    printf ("  r--        %5d     %5d\n", uagent_rss_r,       uagent_dirty_r)
    printf ("  Unknown    %5d     %5d\n", uagent_rss_unknown, uagent_dirty_unknown)

    printf ("Anon info:\n")
    printf (" Perm        RSS   Dirty\n")
    printf ("  r-x        %5d     %5d\n", anon_rss_rx,      anon_dirty_rx)
    printf ("  rw-        %5d     %5d\n", anon_rss_rw,      anon_dirty_rw)
    printf ("  r--        %5d     %5d\n", anon_rss_r,       anon_dirty_r)
    printf ("  Unknown    %5d     %5d\n", anon_rss_unknown, anon_dirty_unknown)

    printf ("\nCount: %d  Actual: %d\n", count, actual)
}






可以通过mprotect设置内存的属性
https://linux.die.net/man/2/mprotect
Memory protection keys
https://lwn.net/Articles/643797/
Memory Protection and ASLR on Linux
https://eklitzke.org/memory-protection-and-aslr

ES Collectd插件
https://www.elastic.co/guide/en/logstash/current/plugins-codecs-collectd.html




真随机数等生成
http://www.cnblogs.com/bigship/archive/2010/04/04/1704228.html

在打印时，如果使用了 size_t 类型，那么通过 ```%d``` 打印将会打印一个告警，可以通过如下方式修改，也就是添加 ```z``` 描述。

size_t x = ...;
ssize_t y = ...;
printf("%zu\n", x);  // prints as unsigned decimal
printf("%zx\n", x);  // prints as hex
printf("%zd\n", y);  // prints as signed decimal

/proc/iomem 保存物理地址的映射情况，每行代表一个资源 (地址范围和资源名)，其中可用物理内存的资源名为 "System RAM" ，在内核中通过 insert_resource() 这个API注册到 iomem_resource 这颗资源树上。

例如，如下的内容：

01200000-0188b446 : Kernel code
0188b447-01bae6ff : Kernel data
01c33000-01dbbfff : Kernel bss

这些地址范围都是基于物理地址的，在 ```setup_arch()@arch/x86/kernel/setup.c``` 中通过如下方式注册。

max_pfn = e820_end_of_ram_pfn();
        code_resource.start = __pa_symbol(_text);
        code_resource.end = __pa_symbol(_etext)-1;
        insert_resource(&iomem_resource, &code_resource);

linux虚拟地址转物理地址
http://luodw.cc/2016/02/17/address/
Linux内存管理
http://gityuan.com/2015/10/30/kernel-memory/
/proc/iomem和/proc/ioports
http://blog.csdn.net/ysbj123/article/details/51088644
port地址空间和memory地址空间是两个分别编址的空间，都是从0地址开始
port地址也可以映射到memory空间中来，前提是硬件必须支持MMIO
iomem—I/O映射方式的I/O端口和内存映射方式的I/O端口
http://www.cnblogs.com/b2tang/archive/2009/07/07/1518175.html


协程
https://github.com/Tencent/libco


#if FOO < BAR
#error "This section will only work on UNIX systems"
#endif
http://hbprotoss.github.io/posts/cyu-yan-hong-de-te-shu-yong-fa-he-ji-ge-keng.html
https://linuxtoy.org/archives/pass.html
https://en.m.wikipedia.org/wiki/Padding_(cryptography)





 sed -e 's/collectd/\1/' *
sed只取匹配部分
http://mosquito.blog.51cto.com/2973374/1072249
http://blog.sina.com.cn/s/blog_470ab86f010115kv.html

通过sed只显示匹配行或者某些行
----- 显示1,10行
$ sed -n '1,10p' filename
----- 显示第10行
$ sed -n '10p' filename
----- 显示某些匹配行
$ sed -n '/This/p' filename
sed -n '/\<collectd\>/p' *
sed -i 's/\<Collectd/Uagent/g' *




Proxy
简单的实现，通常用于嵌入式系统
https://github.com/kklis/proxy
Golang实现的代理，包括了0复制技术等 
https://github.com/funny/proxy
dnscrypt-proxy DNS客户端与服务端的加密传输，使用libevent库
https://github.com/jedisct1/dnscrypt-proxy
ProxyChains Sock以及HTTP代理，通常用在类似TOR上面
https://github.com/haad/proxychains
https://github.com/rofl0r/proxychains-ng
基于libevent的简单代理服务
https://github.com/wangchuan3533/proxy
支持多种协议的代理服务，包括FTP、DNS、TCP、UDP等等
https://github.com/z3APA3A/3proxy
一个加密的代理服务
https://github.com/proxytunnel/proxytunnel
Vanish缓存中使用的代理服务
https://github.com/varnish/hitch
360修改的MySQL代理服务
https://github.com/Qihoo360/Atlas
Twitter提供的Memchached代理服务
https://github.com/twitter/twemproxy

UDP可靠传输方案
http://blog.codingnow.com/2016/03/reliable_udp.html
http://blog.csdn.net/kennyrose/article/details/7557917
BitCoin中使用的可靠UDP传输方案
https://github.com/maidsafe-archive/MaidSafe-RUDP
UDP方案的优缺点
https://blog.wilddog.com/?p=668




export http_proxy="http://<username>:<password>@proxy.foobar.com:8080"
export https_proxy="http://<username>:<password>@proxy.foobar.com:8080"
export ftp_proxy="http://<username>:<password>@proxy.foobar.com:8080"
export no_proxy="xxxx,xxxx"


https://github.com/wglass/collectd-haproxy  Python
https://github.com/Fotolia/collectd-mod-haproxy  C
https://github.com/funzoneq/collectd-haproxy-nbproc
https://github.com/signalfx/collectd-haproxy  ***
https://github.com/mleinart/collectd-haproxy  *

很多collectd插件的组合，很多不错的监控指标梳理
https://github.com/signalfx/integrations
https://github.com/DataDog/the-monitor
https://www.librato.com/docs/kb/collect/integrations/haproxy/
https://www.datadoghq.com/blog/monitoring-haproxy-performance-metrics/
https://github.com/mleinart/collectd-haproxy/blob/master/haproxy.py
Request: (只对HTTP代理有效)
   request_rate(req_rate)      px->fe_req_per_sec           proxy_inc_fe_req_ctr()  请求速率
   req_rate_max 请求限制速率
   req_tot 目前为止总的请求数
Response: (只对HTTP代理有效)
  'hrsp_1xx': ('response_1xx', 'derive'),
  'hrsp_2xx': ('response_2xx', 'derive'),
  'hrsp_3xx': ('response_3xx', 'derive'),
  'hrsp_4xx': ('response_4xx', 'derive'),
  'hrsp_5xx': ('response_5xx', 'derive'),
  'hrsp_other': ('response_other', 'derive'),

>>>>>backend<<<<<
Time:
  qtime (v1.5+) 过去1024个请求在队里中的平均等待时间
  rtime (v1.5+) 过去1024个请求在队里中的平均响应时间









http://savannah.nongnu.org/projects/nss-mysql
https://github.com/NigelCunningham/pam-MySQL
http://lanxianting.blog.51cto.com/7394580/1767113
https://stackoverflow.com/questions/7271939/warning-ignoring-return-value-of-scanf-declared-with-attribute-warn-unused-r
https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
https://stackoverflow.com/questions/30813452/how-to-ignore-all-warnings-in-c
http://www.jianshu.com/p/7e84a33b46e9
https://github.com/flike/kingshard/blob/master/doc/KingDoc/kingshard_admin_api.md   微博
__attribute__((warn_unused_result))


协程
http://blog.163.com/shu_wang/blog/static/1721704112016114113331412
https://stackoverflow.com/questions/28977302/how-do-stackless-coroutines-differ-from-stackful-coroutines
http://www.infoq.com/cn/articles/CplusStyleCorourtine-At-Wechat
https://www.iamle.com/archives/1865.html
https://github.com/mcxiaoke/mqtt
http://www.infoq.com/cn/articles/fine-tuned-haproxy-to-achieve-concurrent-ssl-connections?utm_campaign=rightbar_v2&utm_source=infoq&utm_medium=articles_link&utm_content=link_text



JMX(Java Management Extensions) 是一个为应用程序植入管理功能的框架，是一套标准的代理和服务，实际上，用户可以在任何 Java 应用程序中使用这些代理和服务实现管理。
http://blog.csdn.net/derekjiang/article/details/4532375
http://tomcat.apache.org/tomcat-6.0-doc/monitoring.html#Enabling_JMX_Remote
http://comeonbabye.iteye.com/blog/1463104
https://visualvm.github.io/
http://blog.csdn.net/kingzone_2008/article/details/50865350


Buddy 和  Slub 是 Linux 内核中的内存管理算法。Buddy 防止内存的 "外碎片"，即防止内存块越分越小，而不能满足大块内存分配的需求。Slub 防止内存的 "内碎片"，即尽量按请求的大小分配内存块，防止内存块使用上的浪费。https://github.com/chenfangxin/buddy_slub

https://stackoverflow.com/questions/9873061/how-to-set-the-source-port-in-the-udp-socket-in-c
http://www.binarytides.com/programming-udp-sockets-c-linux/
https://www.cyberciti.biz/faq/linux-unix-open-ports/
https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c
https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
https://www.digitalocean.com/community/tutorials/how-to-use-netcat-to-establish-and-test-tcp-and-udp-connections-on-a-vps





http://my.fit.edu/~vkepuska/ece3551/ADI_Speedway_Golden/Blackfin%20Speedway%20Manuals/LwIP/socket-api/setsockopt_exp.html

socktop(systap使用)




Socket INTR的处理
http://blog.csdn.net/SUKHOI27SMK/article/details/43021081

http://www.tldp.org/HOWTO/html_single/C++-dlopen/

UDP-based Data Transfer Protocol
http://udt.sourceforge.net/
https://github.com/securesocketfunneling/udt
http://blog.leanote.com/post/caasi/Reliable-UDP-3

https://github.com/lsalzman/enet

https://askubuntu.com/questions/714503/regular-expressions-vs-filename-globbing


1.       #注释
2.       变量：使用set命令显式定义及赋值，在非if语句中，使用${}引用，if中直接使用变量名引用；后续的set命令会清理变量原来的值；
3.       command (args ...)  #命令不分大小写，参数使用空格分隔，使用双引号引起参数中空格
4.       set(var a;b;c) <=> set(var a b c)  #定义变量var并赋值为a;b;c这样一个string list
5.       Add_executable(${var}) <=> Add_executable(a b c)   #变量使用${xxx}引用

----- 条件语句
if(var)   # 其中空 0 N No OFF FALSE 视为假，NOT 后为真
else()/elseif()
endif(var)

7.       循环语句

Set(VAR a b c)

Foreach(f ${VAR})       …Endforeach(f)

8.       循环语句

WHILE() … ENDWHILE()



INCLUDE_DIRECTORIES(include)  # 本地的include目录设置
LINK_LIBRARIES('m')           # 添加库依赖，等价于命令行中的-lm参数



GTest 实际上不建议直接使用二进制文件，而是建议从源码开始编译。https://github.com/google/googletest/blob/master/googletest/docs/FAQ.md
如果要使用二进制包，那么可以使用如下方式进行配置。
find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED gtest>=1.7.0)
pkg_check_modules(GMOCK REQUIRED gmock>=1.7.0)

include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
)
http://www.yeolar.com/note/2014/12/16/cmake-how-to-find-libraries/
http://blog.csdn.net/netnote/article/details/4051620

find_package(Threads REQUIRED)   # 使用内置模块查找thread库支持


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(uagent)

ADD_SUBDIRECTORY(librudp )
INCLUDE_DIRECTORIES(include)

option(WITH_UNIT_TESTS "Compile with unit tests" OFF)



https://github.com/sohutv/cachecloud  Redis监控管理
https://github.com/apache/incubator-superset 牛掰的项目管理
https://github.com/huichen/wukong 悟空搜索引擎
https://github.com/sylnsfar/qrcode   动态二维码生成
https://github.com/hellysmile/fake-useragent 伪装浏览器身份
https://github.com/jwasham/coding-interview-university 谷歌面试题
https://github.com/Tencent/libco 腾讯协程库
https://github.com/xtaci/kcptun 最快的UDP传输
https://github.com/reorx/httpstat 图形显示http处理耗时
https://github.com/ajermakovics/jvm-mon JVM监控
https://github.com/stampery/mongoaudit MongoDB审计
https://github.com/alexazhou/VeryNginx
https://github.com/helloqingfeng/Awsome-Front-End-learning-resource
https://github.com/shimohq/chinese-programmer-wrong-pronunciation
https://github.com/egonelbre/gophers
https://github.com/dbcli/mycli
https://github.com/nextcloud/server
https://github.com/SpaceVim/SpaceVim
https://github.com/nlohmann/json
https://github.com/alisaifee/flask-limiter
https://github.com/nicolargo/glances
https://github.com/nonstriater/Learn-Algorithms
https://github.com/ZuzooVn/machine-learning-for-software-engineers
https://github.com/jumpserver/jumpserver
https://github.com/FredWe/How-To-Ask-Questions-The-Smart-Way/blob/master/README-zh_CN.md
https://github.com/drduh/macOS-Security-and-Privacy-Guide
https://github.com/chrislgarry/Apollo-11
https://github.com/taizilongxu/interview_python
https://github.com/FallibleInc/security-guide-for-developers
https://github.com/SamyPesse/How-to-Make-a-Computer-Operating-System
https://github.com/yiminghe/learning-react


FIXME:
  /post/python-modules.html
  Python 包管理
  http://www.jianshu.com/p/9acc85d0ff16
  http://xiaorui.cc/2014/09/20/%E4%BD%BF%E7%94%A8hashring%E5%AE%9E%E7%8E%B0python%E4%B8%8B%E7%9A%84%E4%B8%80%E8%87%B4%E6%80%A7hash/
360
https://github.com/flike/kingshard
https://github.com/Qihoo360/Atlas
https://tech.meituan.com/dbproxy-pr.html
http://www.cnblogs.com/wunaozai/p/3955156.html
https://tech.meituan.com/

http://blog.csdn.net/factor2000/article/details/3929816
http://www.tldp.org/HOWTO/html_single/C++-dlopen/


schema_integrity_check_failed
一般是由于 mnesia 数据库的问题导致，简单粗暴的方式是直接删除。



MTU
Maximum Transmission Unit
 ifconfig eth0 mtu number
/etc/sysconfig/network-scripts/ifcfg-eth0
MTU=1000
IPV6_MTU=1000
http://www.361way.com/linux-mtu-jumbo-frames/4055.html
http://www.microhowto.info/howto/change_the_mtu_of_a_network_interface.html
http://www.cnblogs.com/liu-yao/p/5678161.html
http://blog.csdn.net/anzhsoft/article/details/19563091





首先通过 crontab(crontab.c) 完成任务的编辑，然后通过 poke_daemon() 通知 crond 程序，实际上就是通过 utime() 修改 SPOOL_DIR 目录的访问和修改时间。而在 crond(cron.c) 程序中，会通过 inotify 机制接收，然后进行更新。

http://blog.csdn.net/rain_qingtian/article/details/11008779

https://github.com/DaveGamble/cJSON

语法规则可以参考 [JSON: The Fat-Free Alternative to XML](yuhttp://www.json.org/fatfree.html) 。

parse_value()  正式的语法解析

https://github.com/staticlibs/ccronexpr

American Fuzzy Lop, AFL 是一种开源的模糊测试器，由谷歌的 Michal Zalewski 开发。可以在源码编译时添加，或者使用 QEMU 模式，也就是 QEMU-(User Mode) ，在执行时注入部分代码进行测试。http://lcamtuf.coredump.cx/afl/
https://github.com/google/syzkaller
https://github.com/xxg1413/fuzzer/tree/master/iFuzz
https://stfpeak.github.io/2017/06/12/AFL-Cautions/
http://bobao.360.cn/news/detail/3354.html
http://www.jianshu.com/p/015c471f5a9d
http://ele7enxxh.com/Use-AFL-For-Stagefright-Fuzzing-On-Linux.html
http://www.freebuf.com/articles/system/133210.html
http://www.hackdig.com/07/hack-24522.htm
http://aes.jypc.org/?p=38207
https://fuzzing-project.org/tutorial3.html
afl-fuzz -i afl_in -o afl_out ./binutils/readelf -a @@
afl-gcc afl-clang-fast

http://blog.codingnow.com/2017/07/float_inconsistence.html#more


:verbose imap <tab>
http://www.cnblogs.com/acbingo/p/4757275.html
http://blog.guorongfei.com/2016/12/03/vim-ultisnipt-google-c-cpp-header-gurad/
http://vimzijun.net/2016/10/30/ultisnip/
http://www.linuxidc.com/Linux/2016-11/137665.htm
https://segmentfault.com/q/1010000000610373

http://liulixiaoyao.blog.51cto.com/1361095/814329













https://casatwy.com/pthreadde-ge-chong-tong-bu-ji-zhi.html
http://blog.csdn.net/willib/article/details/32942189

https://github.com/beego/beedoc/blob/master/zh-CN/module/grace.md
https://www.nginx.com/resources/wiki/start/topics/tutorials/commandline/
http://blog.csdn.net/brainkick/article/details/7192144
http://shzhangji.com/cnblogs/2012/12/23/nginx-live-upgrade/

http://www.freebuf.com/sectool/119680.html
http://tonybai.com/2011/04/21/apply-style-check-to-c-code/
https://github.com/dspinellis/cqmetrics

VGC、RATS、Source Insight


测试版本


core
gcov
CPP-Check
Flawfinder


静态安全扫描 flawfinder、RATS、ITS4、VCG、CPPLint、SPlint

Python: Pychecker、Pylint、RATS


python -m SimpleHTTPServer

## flawfinder

一个 Python 写的程序，用于扫描代码，然后在规则库 (c_ruleset) 中查找符合规则的场景。

源码可以直接从 [www.dwheeler.com](https://www.dwheeler.com/flawfinder/) 上下载，安装方式可以查看 README 文件，也就是如下命令。

$ tar xvzf FILENAME.tar.gz       # Uncompress distribution file
$ cd flawfinder-*                # cd into it.
# make prefix=/usr install       # Install in /usr

该工具只针对单个语句进行词法分析，不检查上下文，不分析数据类型和数据流；检查运行时可能存在的问题，比如内存泄露；然后会根据规则库给出代码建议。这也就意味着会有部分的误报，不过因为使用简单，仍不失为一个不错的静态检测工具。

检查可以直接指定文件或者目录，工具会自动查看所有的 C/C++ 文件，如果是 patch (diff -u、svn diff、git diff) 添加参数 --patch/-P 即可。严重等级从 0 到 5 依次增加，而且会标示出 [Common Weakness Enumeration, CWE](https://cwe.mitre.org/data/) 对应。

检查时会读取 ruleset 中的规则，然后如果匹配 (hit) 则将匹配数据保存到 hitlist 中，

### 常见操作

1. 重点检查与外部不可信用户的交互程序，先确保这部分程序无异常；
2. 如果已经审计的函数，可以通过 ```// Flawfinder: ignore``` 或者 ```/* Flawfinder: ignore */``` 减少异常输出，为了兼容，也可以使用 ```ITS4``` 或者 ```RATS``` 替换 ```Flawfinder```；


--inputs/-I
  只检查从外部用户(不可信)获取数据的函数；
--neverignore/-n
  默认可以通过上述的方式忽略标记的行，通过该参数用于强制检测所有的代码；
--savehitlist, --loadhitlist, --diffhitlist
  用于保存、加载、比较hitlist；
--minlevel=NUMBER
  指定最小的错误汇报级别；

--quiet/-Q
  默认会在检测时打印检查了哪些文件，通过该选项可以关闭，通常用于格式化输出检测；
--dataonly/-D
  不显示header和footer，可以配合--quiet参数只显示数据；
--singleline/-S
  检测结果默认会多行显示，该参数指定一行显示；
--immediate/-i
  默认在全部文件检测完之后，进行排序，然后显示最终的结果，该参数可以在监测到异常后立即显示；


----- 检查所有的代码，即使已经标记为ignore的代码
$ flawfinder --neverignore src
----- 可以通过如下命令输出，以供其它自动化工具使用
$ flawfinder -QD src
$ flawfinder -QDSC src
----- 检查代码只汇报CWE-120或者CWE-126
$ flawfinder --regex "CWE-120|CWE-126" src/


/* RATS: ignore */



uagent 调试，
export UAGENT_TRACE="yes"

flawfinder -Q --minlevel=5 src | less



int lcc_connect(const char *address, lcc_connection_t **ret_con);
功能：
  建立指向address的socket链接，通过ret_con返回链接信息；
入参:
  address socket地址，如/usr/var/run/uagent.sock、unix:/usr/var/run/uagent.sock；
  ret_con 返回的已经建立好的链接；
返回值：
  -1 入参异常，或者没有内存；
  0  正常返回；

int lcc_disconnect(lcc_connection_t *c);
功能：
  关闭链接，释放资源；

## coverage
http://blog.csdn.net/livelylittlefish/article/details/6448885
编译链接时需要修改配置选项。

* 编译的时候，增加 -fprofile-arcs -ftest-coverage 或者 –coverage；
* 链接的时候，增加 -fprofile-arcs 或者 –lgcov；
* 打开–g3 选项，去掉-O2以上级别的代码优化选项，否则编译器会对代码做一些优化，例如行合并，从而影响行覆盖率结果。

ifeq ($(coverage), yes)
CFLAGS   +=  -fprofile-arcs -ftest-coverage -g3
LDFLAGS  +=  -fprofile-arcs -ftest-coverage
endif

如下是测试步骤。
----- 1. 编译源码，此时每个文件都会生成一个*.gcno文件
$ make coverage=yes
----- 2. 运行，运行之后会生成 *.gcda 文件
$ ./helloworld
----- 3.1 可以通过如下命令生成单个文件的覆盖率，生成的是文本文件*.gcov
$ gcov helloworld.c

除了使用 gcov 之外，还可以通过 lcov 查看覆盖率，简单说下 *.gcov 的文件格式。

    -:    2:#include <assert.h>   非有效行
    -:    3:#include <stdlib.h>
 ... ...
  148:   71:  if (n == NULL)      调用次数
#####:   72:    return (0);       未调用


简单介绍下代码覆盖率的常见术语。


主要是基本块（Basic Block），基本块图（Basic Block Graph），行覆盖率（line coverage）, 分支覆盖率（branch coverage）等。

##### 基本块
这里可以把基本块看成一行整体的代码，基本块内的代码是线性的，要不全部运行，要不都不运行，其详细解释如下：
A basic block is a sequence of instructions with only entry and only one exit. If any one of the instructions are executed, they will all be executed, and in sequence from first to last.





    基本块图（Basic Block Graph），基本块的最后一条语句一般都要跳转，否则后面一条语句也会被计算为基本块的一部分。 如果跳转语句是有条件的，就产生了一个分支(arc)，该基本块就有两个基本块作为目的地。如果把每个基本块当作一个节点，那么一个函数中的所有基本块就构成了一个有向图，称之为基本块图(Basic Block Graph)。且只要知道图中部分BB或arc的执行次数就可以推算出所有的BB和所有的arc的执行次数；
    打桩，意思是在有效的基本块之间增加计数器，计算该基本块被运行的次数；打桩的位置都是在基本块图的有效边上；

##### 行覆盖率
就是源代码有效行数与被执行的代码行的比率；

##### 分支覆盖率
有判定语句的地方都会出现 2 个分支，整个程序经过的分支与所有分支的比率是分支覆盖率。注意，与条件覆盖率(condition coverage)有细微差别，条件覆盖率在判定语句的组合上有更细的划分。

### gcc/g++ 编译选项

如上所述，在编译完成后会生成 *.gcno 文件，在运行正常结束后生成 *.gcda 数据文件，然后通过 gcov 工具查看结果。

--ftest-coverage
  让编译器生成与源代码同名的*.gcno文件 (note file)，含有重建基本块依赖图和将源代码关联至基本块的必要信息；
--fprofile-arcs
  让编译器静态注入对每个源代码行关联的计数器进行操作的代码，并在链接阶段链入静态库libgcov.a，其中包含在程序正常结束时生成*.gcda文件的逻辑；

可以通过源码解析来说明到底这 2 个选项做了什么，命令如下：
g++ -c -o hello.s hello.c -g -Wall -S
g++ -c -o hello_c.s hello.c -g -Wall –coverage -S
vimdiff hello.s hello_c.s


1. 覆盖率的结果只有被测试到的文件会被显示，并非所有被编译的代码都被作为覆盖率的分母

实际上，可以看到整个覆盖率的产生的过程是4个步骤的流程，一般都通过外围脚本，或者makefile/shell/python来把整个过程自动化。2个思路去解决这个问题，都是通过外围的伪装。第一个，就是修改lcov的 app.info ，中间文件，找到其他的文件与覆盖率信息的地方，结合makefile，把所有被编译过的源程序检查是否存于 app.info 中，如果没有，增加进去。第二个伪装，是伪装 *.gcda，没有一些源码覆盖率信息的原因就是该文件没有被调用到，没有响应的gcda文件产生。


2. 后台进程的覆盖率数据收集；


其实上述覆盖率信息的产生，不仅可以针对单元测试，对于功能测试同样适用。但功能测试，一般linux下c/c++都是实现了某个Daemon进程，而覆盖率产生的条件是程序需要正常退出，即用户代码调用 exit 正常结束时，gcov_exit 函数才得到调用，其继续调用 __gcov_flush 函数输出统计数据到 *.gcda 文件中。同样2个思路可以解决这个问题，

第一，给被测程序增加一个 signal handler，拦截 SIGHUP、SIGINT、SIGQUIT、SIGTERM 等常见强制退出信号，并在 signal handler 中主动调用 exit 或 __gcov_flush 函数输出统计结果。但这个需要修改被测程序。这个也是我们之前的通用做法。但参加过清无同学的一个讲座后，发现了下面第二种更好的方法。

第二，借用动态库预加载技术和 gcc 扩展的 constructor 属性，我们可以将 signalhandler 和其注册过程都封装到一个独立的动态库中，并在预加载动态库时实现信号拦截注册。这样，就可以简单地通过如下命令行来实现异常退出时的统计结果输出了。


### lcov

用于生成 html 格式的报告。

yum install --enablerepo=epel lcov perl-GD

----- 1. 生成*.info文件
$ lcov -d . -o 'hello_test.info' -t ‘Hello test’ -b . -c
参数解析：
   -d 指定目录
----- 2. 生成html，-o指定输出目录，可以通过HTTP服务器查看了
$ genhtml -o result hello_test.info



## 静态检查

http://www.freebuf.com/sectool/119680.html

cppcheck、Splint(Secure Programming Lint)

### cppcheck

直接从 [github cppcheck](https://github.com/danmar/cppcheck) 下载，然后通过 make && make install 编译安装即可。

cppcheck -j 3 --force --enable=all src/*

--force
  如果#ifdef的宏定义过多，则cppcheck只检查部分
-j
  检查线程的个数，用于并发检查；
--enable
  指定当前的检查级别，可选的参数有all，style，information等；
--inconclusive
  默认只会打印一些确认的错误，通过该参数配置异常的都打印；



### Splint

http://www.cnblogs.com/bangerlee/archive/2011/09/07/2166593.html


## 圈复杂度 (Cyclomatic complexity)

OCLint(Mac) cppncss SourceMonitor(Windows)

常用概念介绍如下：

* Non Commenting Source Statements, NCSS 去除注释的有效代码行；
* Cyclomatic Complexity Number, CCN 圈复杂度。

同样，一个函数的 CCN 意味着需要多少个测试案例来覆盖其不同的路径，当 CCN 发生很大波动或者 CCN 很高的代码片段被变更时，意味改动引入缺陷风险高，一般最好小于 10 。



Findbugs (compiled code analysis)
PMD (static code analysis)

### SourceMonitor

http://www.campwoodsw.com/sourcemonitor.html

### cppncss

很简单的计算圈复杂度的工具，java。


## 内存检测

Valgrind


HAVE_LIBGEN_H 1
HAVE_FNMATCH_H 1

----- 当前目录下生成buildbot_master目录，以及配置文件master.cfg.sample
$ buildbot create-master buildbot_master


$ cd buildbot_master && mv master.cfg.sample master.cfg
$ buildbot checkconfig master.cfg
c['buildbotNetUsageData'] = None

----- 运行
$ buildbot start buildbot_master
# 查看日志
tail -f master/twistd.log




https://github.com/nodejs/http-parser
https://github.com/shellphish/how2heap
https://github.com/DhavalKapil/heap-exploitation

https://github.com/maidsafe-archive/MaidSafe-RUDP/wiki
RTP  https://tools.ietf.org/html/rfc3550
UDT http://udt.sourceforge.net/
https://github.com/dorkbox/UDT

https://github.com/greensky00/avltree
http://blog.csdn.net/q376420785/article/details/8286292
http://xstarcd.github.io/wiki/shell/UDP_Hole_Punching.html



http://www.wowotech.net/timer_subsystem/tick-device-layer.html
https://www.w3.org/CGI/
https://github.com/HardySimpson/zlog











main()
 |-handle_read()
   |-httpd_start_request()
     |-really_start_request()
       |-cgi()
         |-cgi_child()
           |-make_envp()
             |-build_env()



onion_http_new() 会将onion_http_read_ready()赋值给read_ready

onion_http_read_ready()
 |-onion_request_process()


onion_url_new()
 |-onion_url_handler()

onion_listen_point_accept() 在listen端口上出现时调用

onion_listen_point_read_ready()









cJSON_CreateObject()    创建新的对象，设置对应的type类型
 |-cJSON_New_Item()     新申请cJSON结构内存，并初始化为0
cJSON_CreateString()    和cJSON_CreateRaw()函数调用相同，只是设置的类型不同
 |-cJSON_New_Item()
 |-cJSON_strdup()       创建对象后会将字符串复制一份
cJSON_Print()
 |-print()
   |-print_value()


typedef struct cJSON {
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;    对于Array类型，则会作为链表头
    int type;               类型，包括了String、Number、Raw等
    char *valuestring;      如果是String或者Raw时使用
    int valueint;           这个已经取消，使用valuedouble替换，为了兼容未删除
    double valuedouble;     如果是Number时使用

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;

Invalid、True、False、NULL、Object、Array 通过 type 区分，

cJSON_Parse()
 |-cJSON_ParseWithOpts()
   |-cJSON_New_Item()
   |-parse_value()  根据不同的字段进行解析

cJSON_Duplicate()
cJSON_Minify()
???cJSON_Compare()

cJSON_Parse()
cJSON_Print()  按照可阅读的格式打印，方便阅读，一般用于交互
cJSON_PrintUnformatted() 最小化打印，一般用于格式发送
cJSON_PrintBuffered()
cJSON_PrintPreallocated()

string trim
https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way


scanf 中一种很少见但很有用的转换字符 `[...]` 和 `[ ^...]` 。

#include <stdio.h>
int main()
{
    char strings[100];
    scanf("%[1234567890]", strings);
    printf("%s", strings);
    return 0;
}

运行，输入 `1234werew` 后，结果是 `1234` ，也就是说，如果输入的字符属于方括号内字符串中某个字符，那么就提取该字符；如果一经发现不属于就结束提取。

这就是 ANSI C 增加的一种新特性，称为扫描集 (scanset)，由一对方括号中的一串字符定义，左方括号前必须缀以百分号，通过 `^` 表示补集。

注意，其中必须至少包含一个字符，否则非法，如 `%[]` 和 `%[^]` 是非法的。

%[a-z]  读取在 a-z 之间的字符串
    char s[]="hello, my friend";   // 注意: 逗号在不a-z之间
    sscanf(s, "%[a-z]", string);   // string=hello
%[^a-z] 读取不在 a-z 之间的字符串，如果碰到a-z之间的字符则停止
    char s[]="HELLOkitty";
    sscanf( s, "%[^a-z]", string); // string=HELLO
%*[^=]  前面带 * 号表示不保存变量，跳过符合条件的字符串。
    char s[]="notepad=1.0.0.1001" ;
    char szfilename [32] = "" ;
    int i = sscanf( s, "%*[^=]", szfilename ) ;
// szfilename=NULL,因为没保存

int i = sscanf( s, "%*[^=]=%s", szfilename ) ;
// szfilename=1.0.0.1001


%40c 读取40个字符


%[^=] 读取字符串直到碰到’=’号，’^’后面可以带更多字符,如：
              char s[]="notepad=1.0.0.1001" ;
              char szfilename [32] = "" ;
             int i = sscanf( s, "%[^=]", szfilename ) ;
           // szfilename=notepad
       如果参数格式是：%[^=:] ，那么也可以从 notepad:1.0.0.1001读取notepad
http://www.cnblogs.com/mafly/p/postman.html

http://www.quartz-scheduler.org/documentation/quartz-2.x/tutorials/crontrigger.html
http://www.blogjava.net/javagrass/archive/2011/07/12/354134.html
https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/


https://en.wikipedia.org/wiki/Network_Time_Protocol

Linux 内核通过 adjtime() 或者 ntp_adjtime() 来进行时钟的同步，ntptime
http://jfcarter.net/~jimc/documents/bugfix/12-ntp-wont-sync.html
http://libev.schmorp.de/bench.c
https://stackoverflow.com/questions/14621261/using-libev-with-multiple-threads
https://curl.haxx.se/libcurl/c/evhiperfifo.html
https://github.com/HardySimpson/zlog
http://kagachipg.blogspot.com/2013/10/multi-thread-in-libev.html














conn_config()
 |-port_collect_listening  如果配置了ListeningPorts变量则设置为1
 |-conn_get_port_entry()   对于LocalPort和RemotePort参数，如果存在则设置否则创建
 |-port_collect_total      如果配置了AllPortsSummary变量则设置为1
conn_read()
 |-
####

$ cat << EOF > request.txt
GET / HTTP/1.1
Host: 127.1
EOF
$ cat request.txt | openssl s_client -connect 127.1:443


printf 'GET / HTTP/1.1\r\nHost: github.com\r\n\r\n' | ncat --ssl github.com 443

----- 发送系统日志内容
$ ncat -l -p 6500 | tee -a copy.out | tar -zx -C $(mktemp -d)
$ (tar -zc -C /var/log; tail -f /var/log/syslog) | ncat 127.1 6500

----- 使用SSL传输
$ ncat -l -p 6500 --ssl --ssl-cert /etc/ssl/host.crt \
    --ssl-key /etc/ssl/host.key > out.tgz
$ tar -zc ~ | ncat --ssl 127.1 6500

----- 使用UDP协议
$ ncat -l -p 6500 --udp > out.tgz
$ tar -zc ~ | ncat --udp machineb 6500

----- 使用SCTP
$ ncat --sctp -l -p 6500 > out.tgz
$ tar -zc ~ | ncat --sctp machineb 6500
给系统添加根证书
http://manuals.gfi.com/en/kerio/connect/content/server-configuration/ssl-certificates/adding-trusted-root-certificates-to-the-server-1605.html
https://segmentfault.com/a/1190000002569859
CentOS 会保存在 /etc/ssl/certs/ 目录下，

--ssl                  Connect or listen with SSL
--ssl-cert             Specify SSL certificate file (PEM) for listening
--ssl-key              Specify SSL private key (PEM) for listening
--ssl-verify           Verify trust and domain name of certificates
--ssl-trustfile        PEM file containing trusted SSL certificates

http://blog.csdn.net/ljy1988123/article/details/51424162
http://blog.csdn.net/younger_china/article/details/72081779
http://blog.csdn.net/yusiguyuan/article/details/48265205

SSL Certificate File 文件中包含了一个 X.509 证书，实际上也就是加密用的公钥，而 SSL Certificate Key File 文件中是公钥对应的私钥，在进行安全传输时就需要这对密钥。有的程序是将两者放在一起，如一些 Java 程序；有的则会分开存储，如 Apache 。

一般在申请了证书之后，如通过 GoDaddy，会提供上述的两个文件。

如果服务端只使用了上述的两个文件，那么实际上客户端并不知道这个证书是谁颁发的；不过一般来说没有太大问题，因为客户端会保存很多的 CA 证书，包括中间证书以及根证书。如果要直接指定证书的依赖关系，可以通过 SSLCertificateChainFile 参数指定。

Nginx https配置
https://fatesinger.com/75967
https://imququ.com/post/my-nginx-conf-for-security.html


tail  -> coreutils
tailf -> util-linux

Linux Shell man 命令详细介绍
http://blog.jobbole.com/93404/
http://www.lai18.com/content/1010397.html

网络监控
https://stackoverflow.com/questions/614795/simulate-delayed-and-dropped-packets-on-linux

The f_frsize value is the actual minimum allocation unit of the
filesystem, while the f_bsize is the block size that would lead to
most efficient use of the disk with io calls.  All of the block counts
are in terms of f_frsize, since it is the actual allocation unit size.
 The BSD manpages are a bit more informative on this function than the
POSIX ones.

https://blog.blahgeek.com/glibc-and-symbol-versioning/
http://www.runoob.com/linux/linux-comm-indent.html
http://riemann.io/quickstart.html
http://blog.csdn.net/c80486/article/details/45066439
http://www.hzrcj.org.cn/personnel/pd01/findda_qc01
https://github.com/mkirchner/tcping/blob/master/tcping.c

C格式化检查
sparse

indent                                \
 --ignore-profile                  \    不读取indent的配置文件
 --k-and-r-style                   \    指定使用Kernighan&Ritchie的格式
 --indent-level8                   \    缩进多少字符，如果为tab的整数倍，用tab来缩进，否则用空格填充
 --tab-size8                       \    tab大小为8
 --swallow-optional-blank-lines    \    删除多余的空白行
 --line-length130                  \    设置每行的长度
 --no-space-after-casts            \    不要在cast后添加一个空格
 --space-special-semicolon         \    若for或while区段只有一行时，在分号前加上空格
 --else-endif-column1              \    将注释置于else与elseif右侧
    --use-tabs                        \    使用tab做缩进
 --blank-lines-after-procedures    \    函数结束后加空行
 --blank-lines-after-declarations  \    声明结束后加空行
    load.c

find -type f -regextype posix-egrep -regex ".*(~|\.bak)$" -exec ls -alh {} \;

NAN 一种是 <math.h> 中提供的默认值，也可以自定义宏，如下

#define NAN          (0.0 / 0.0)
#define isnan(f)     ((f) != (f))
#define isfinite(f)  (((f) - (f)) == 0.0)
#define isinf(f)     (!isfinite(f) && !isnan(f))

http://zh.cppreference.com/w/c/numeric/math/fpclassify

其中使用 `isnan()` 时，`FLT_EVAL_METHOD` 将被忽略。

FIXME:
https://jin-yang.github.io/post/collectd-source-code.html
 |   | | | | |-FORMAT_VL()                    ← 实际上是调用format_name()将vl中的值生成标示符
             |-pthread_mutex_lock()
 |   | | | | |-c_avl_get()                    ← 利用上述标示符获取cache_entry_t，在此会缓存最近的一次采集数据
             |-uc_insert() 如果不存在则插入，直接解锁退出
    |-合法性检查，上次的采集时间应该小于本次时间
    |-根据不同的类型进行检查<<<DS-TYPE>>>，计算方法详见如下
 |   | | | | |... ...                         ← 会根据不同的类型进行处理，例如DS_TYPE_GAUGE
 |   | | | | |-uc_check_range()               ← 检查是否在指定的范围内

values_raw   保存原始数据的值
values_gauge 会按照不同的类型进行计算，其中计算规则如下







git diff <local branch> <remote>/<remote branch>
----- 查看stage中的diff
git diff --cached/--staged
http://perthcharles.github.io/2015/08/25/clean-commit-log-before-push/
https://github.com/chenzhiwei/linux/tree/master/git
https://ddnode.com/2015/04/14/git-modify-remote-responsity-url.html

通过amend修改之后，需要使用--force强制推送该分支。
git push --force origin feature/ping:feature/ping

etcd
https://tonydeng.github.io/2015/10/19/etcd-application-scenarios/
https://tonydeng.github.io/2015/11/24/etcd-the-first-using/
http://cizixs.com/2016/08/02/intro-to-etcd
http://debugo.com/using-etcd/
curl
http://blog.likewise.org/2011/08/brushing-up-on-curl/
http://www.ruanyifeng.com/blog/2011/09/curl.html
http://www.cnblogs.com/gbyukg/p/3326825.html
https://stackoverflow.com/questions/27368952/linux-best-way-in-two-way-ipc-in-c
http://www.cnblogs.com/zhang-shijie/p/5439210.html
正则表达式
https://www.zhihu.com/question/27434493


以 `CHECK_SYMBOL_EXISTS()` 宏为例，对于 CentOS，在 `/usr/share/cmakeX/Modules` 中存在 `CheckSymbolExists.cmake` 模板，可以直接查看相关宏的定义；其它类似模板同样可以进行相应的检查。

ss -tan |awk 'NR>1{++S[$1]}END{for (a in S) print a,S[a]}' && ./tcpstatus

https://github.com/schweikert/fping
https://github.com/octo/liboping


https://www.typora.io/#windows
json-handle markdown edit chrome浏览器
http://blog.csdn.net/fandroid/article/details/45787423
进程通讯
http://blog.csdn.net/21aspnet/article/details/7479469

https://github.com/TeamStuQ/skill-map

## 网络监控

Interface /proc/net/dev
TCPConns
PowerDNS
IPtables
netlink

命令
ethtool
   ethtool -i eth0
netstat

libev压测
http://libev.schmorp.de/bench.c
spark经典
https://aiyanbo.gitbooks.io/spark-programming-guide-zh-cn/content/index.html
https://github.com/lw-lin/CoolplaySpark/blob/master/Spark%20Streaming%20%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%90%E7%B3%BB%E5%88%97/readme.md
小型数据库
lmdb
cdb  https://github.com/rmind/libcdb
Kyoto Cabinet   http://fallabs.com/kyotocabinet/
https://github.com/symisc/unqlite
https://github.com/numetriclabz/awesome-db
https://github.com/gstrauss/mcdb
https://github.com/tklauser/inotail/blob/master/inotail.c
https://github.com/adamierymenko/kissdb



Futex
http://www.cnblogs.com/bbqzsl/p/6814031.html
http://www.jianshu.com/p/570a61f08e27
http://blog.csdn.net/Javadino/article/details/2891385
http://blog-kingshaohua.rhcloud.com/archives/84
http://blog.csdn.net/michael_r_chang/article/details/30717763
http://www.anscen.cn/article1.html
http://kouucocu.lofter.com/post/1cdb8c4b_50f62fe
http://blog.sina.com.cn/s/blog_e59371cc0102v29b.html
https://bg2bkk.github.io/post/futex%E5%92%8Clinux%E7%9A%84%E7%BA%BF%E7%A8%8B%E5%90%8C%E6%AD%A5%E6%9C%BA%E5%88%B6/
http://kexianda.info/2017/08/17/%E5%B9%B6%E5%8F%91%E7%B3%BB%E5%88%97-5-%E4%BB%8EAQS%E5%88%B0futex%E4%B8%89-glibc-NPTL-%E7%9A%84mutex-cond%E5%AE%9E%E7%8E%B0/

https://github.com/Hack-with-Github/Awesome-Hacking
NTPL生产者消费者模型
http://cis.poly.edu/cs3224a/Code/ProducerConsumerUsingPthreads.c


多线程编程，其中有很多DESIGN_XXX.txt的文档，甚至包括了Systemtap的使用，其底层用到的是系统提供的 futex_XXX() 调用。
https://github.com/lattera/glibc/tree/master/nptl
https://en.wikipedia.org/wiki/List_of_C%2B%2B_multi-threading_libraries

浅谈C++ Multithreading Programming
http://dreamrunner.org/blog/2014/08/07/C-multithreading-programming/
Introduction to Parallel Computing
https://computing.llnl.gov/tutorials/parallel_comp/
解剖 Mutex
https://techsingular.org/2012/01/05/%E8%A7%A3%E5%89%96-mutex/
Pthreads并行编程之spin lock与mutex性能对比分析
http://www.parallellabs.com/2010/01/31/pthreads-programming-spin-lock-vs-mutex-performance-analysis/
Linux线程浅析
http://blog.csdn.net/qq_29924041/article/details/69213248

LinuxThreads VS. NPTL
https://www.ibm.com/developerworks/cn/linux/l-threading.html
http://pauillac.inria.fr/~xleroy/linuxthreads/

FUTEX简析，也可以通过man 7 futex man 2 futex 查看
http://blog.sina.com.cn/s/blog_e59371cc0102v29b.html
futex and userspace thread syncronization (gnu/linux glibc/nptl) analysis
http://cottidianus.livejournal.com/325955.html
原始论文
https://www.kernel.org/doc/ols/2002/ols2002-pages-479-495.pdf
进程资源
http://liaoph.com/inux-process-management/

http://www.cnblogs.com/big-xuyue/p/4098578.html

BMON Ncurse编程
tcpconns
conn_read()
 |-conn_reset_port_entry()
 |-conn_read_netlink()
 | |-conn_handle_ports()
 |-conn_handle_line()
   |-conn_handle_ports()

ping
Host    会新建一个hostlist_t对象
SourceAddress   ping_source
Device          ping_device，要求OPING库的版本大于1.3
TTL             ping_ttl，要求0<ttl<255
Interval        ping_interval，采集时间间隔，需要大于0
Size            ping_data，指定报文的大小
Timeout         ping_timeout，超时时间设置
MaxMissed       ping_max_missed

Timeout 不能超过 Interval ，否则会将 Timeout 调整为 0.9 * Interval 。
  
ping_init()
 |-start_thread()
   |-pthread_mutex_lock() 会做条件判断，防止重复创建线程
   |-plugin_thread_create()  创建ping_thread()线程
会有一个线程ping_thread()一直采集数据，

./configure --prefix=/usr
  
https://cmake.org/Wiki/CMake_FAQ
cmake最终打印信息
https://stackoverflow.com/questions/25240105/how-to-print-messages-after-make-done-with-cmake





http://jialeicui.github.io/blog/file_atomic_operations.html

当多个进程通过 write/read 调用同时访问一个文件时，无法保证操作的原子性，因为在两个函数调用间，内核可能会将进程挂起执行另外的进程。

如果想要避免这种情况的话，则需要使用 pread/pwrite 函数。

ssize_t pread(int fd ，void *buffer ，size_t size，off_t offset);
  返回读取到的字节数，offset是指的从文件开始位置起的offset个字节数开始读。
  其定位和读取操作为原子操作，而且读取过后的文件偏移量不会发生改变。

https://github.com/Garik-/ev_httpclient
https://github.com/zhaojh329/evmongoose
https://github.com/dexgeh/webserver-libev-httpparser
https://github.com/bachan/ugh
https://github.com/novator24/libeva
https://github.com/titilambert/packaging-efl
https://github.com/hyperblast/libevhtp
https://github.com/tailhook/libwebsite
https://github.com/cinsk/evhttpserv
https://github.com/erikarn/libevhtp-http
http://beej.us/guide/bgnet/output/html/multipage/recvman.html

LMDB 中所有的读写都是通过事务来执行，其事务具备以下特点：

支持事务嵌套(??)
读写事务可以并发执行，但写写事务需要被串行化
因此，在lmdb实现中，为了保证写事务的串行化执行，事务执行之前首先要获取全局的写锁。

底层读写的实现

lmdb 使用 mmap 访问存储，不管这个存储是在内存上还是在持久存储上，都是将需要访问的文件只读地装载到宿主进程的地址空间，直接访问相应的地址。

减少了硬盘、内核地址控件和用户地址空间之间的拷贝，也简化了平坦的“索引空间”上的实现，因为使用了read-only的mmap，规避了因为宿主程序错误将存储结构写坏的风险。IO的调度由操作系统的页调度机制完成。

而写操作，则是通过write系统调用进行的，这主要是为了利用操作系统的文件系统一致性，避免在被访问的地址上进行同步。

https://hackage.haskell.org/package/lmdb
http://wiki.dreamrunner.org/public_html/C-C++/Library-Notes/LMDB.html
http://www.jianshu.com/p/yzFf8j
http://www.d-kai.me/lmdb%E8%B0%83%E7%A0%94/
https://github.com/exabytes18/mmap-benchmark
https://symas.com/understanding-lmdb-database-file-sizes-and-memory-utilization/
https://github.com/pmwkaa/sophia

http://www.zkt.name/skip-list/

FIXMAP 是什么意思
MDB_FIXEDMAP

MDB_DUPFIXED
MDB_DUPSORT
内存减小时的优化场景
http://www.openldap.org/conf/odd-sandiego-2004/Jonghyuk.pdf
https://symas.com/performance-tradeoffs-in-lmdb/
http://gridmix.blog.51cto.com/4764051/1731411

## 事务

LMDB 每个事务都会分配唯一的事务编号 (txid)，而且会被持久化。

### 事务表

初始化时会创建一张读事务表，该表记录了当前所有的读事务以及读事务的执行者 (持有该事务的进程与线程 id )；读事务表不仅会在内存中维护，同时会将该信息持久化到磁盘上，也就是位与数据库文件相同目录下的 lock.mdb 文件。

事务表的文件存储格式如下图所示：
http://www.d-kai.me/lmdb%E8%B0%83%E7%A0%94/

该事务表会在 LMDB 打开时初始化，也就是在 `mdb_env_setup_locks()` 函数中，其调用流程如下：

mdb_env_open(MDB_env *env, const char *path, unsigned int flags, mdb_mode_t mode) {
    ......
    /* For RDONLY, get lockfile after we know datafile exists */
    if (!(flags & (MDB_RDONLY|MDB_NOLOCK))) {
        rc = mdb_env_setup_locks(env, lpath, mode, &excl);
        if (rc)
             goto leave;
    }
    ......
}

### 页管理

这应该是 lmdb 中最核心的数据结构了，所有的数据和元数据都存储在page内。

#### Meta Page

Meta Page 使用了起始的两个 Page，在第一次创建的时候会初始化页面，并将相同的内容写入到 Page0 和 Page1 中，也就是说开始时两个页的内容一致；写入是通过 `mdb_env_init_meta()` 函数完成。

因为存在两个页，每次写入或者读取时需要选一个页，其计算规则很简单：

meta = env->me_metas[txn->mt_txnid & 1];

也就是用本次操作的事务 id 取最低位，后面解释这么使用的原因。

main()
 |-mdb_env_create()
 |-mdb_env_set_maxreaders()
 |-mdb_env_set_mapsize()
 |-mdb_env_open()
 | |-mdb_fname_init()
 | |-pthread_mutex_init()
 | |-mdb_fopen()
 | |-mdb_env_setup_locks()
 | | |-mdb_fopen()
 | | |-mdb_env_excl_lock()
 | |-mdb_env_open2()
 |   |-mdb_env_read_header()   尝试从头部读取信息，如果是第一次创建，则会调用如下函数
 |   |-mdb_env_init_meta0()  第一次创建
 |   |-mdb_env_init_meta()   第一次创建时同样需要初始化
 |   |-mdb_env_map()  调用mmap进行映射
 |-mdb_txn_begin()  开启事务，允许嵌套
   |-mdb_txn_renew0()
 |-mdb_dbi_open()
 |-mdb_put()
   |-mdb_cursor_put() 最复杂的函数处理
 |-mdb_txn_commit()
 |-mdb_env_stat()
mdb_get

#ifndef LOG_ERR
/* NOTE: please keep consistent with <syslog.h> */
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* (used) critical conditions, print stack message and quit, nomemory */
#define LOG_ERR         3       /* (used) error conditions */
#define LOG_WARNING     4       /* (used) warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* (used) informational */
#define LOG_DEBUG       7       /* (used) debug-level messages */
#define LOG_DEBUG0      8       /* (used) print more debug-level messages */
#else
/* Others check the <syslog.h> file. */
#define LOG_DEBUG0      8       /* (used) print more debug-level messages */
#endif
int main(int argc, char *argv[])
{
        char time_str[16]; /* 08-28 12:07:11 */
        time_t tt;
        struct tm local_time;
        time(&tt);
        localtime_r(&tt, &local_time);
        strftime(time_str, sizeof(time_str), "%m-%d %T", &local_time);
        puts(time_str);
        static char level_info[] = {' ', ' ', 'C', 'E', 'W', ' ', 'I', 'D', '0'};
        return 0;
}

FIXME:
/post/linux-monitor-cpu.html
  ps -Lf 60010 查看线程

CMAKE教程，很不错
https://blog.gmem.cc/cmake-study-note


https://github.com/jamesroutley/write-a-hash-table

使用场景：
1. 不判断内存是否申请失败

cJSON *tag = cJSON_CreateObject() 不判断是否为NULL
cJSON_AddStringToObject(tag, "key", "value"); 可能会导致内存泄露。场景tag=NULL时，会通过"value"创建一个对象，当tag为NULL时，则会直接退出，那么通过 "value" 创建的对象将无法释放。

cJSON_AddItemToArray(data, metric = cJSON_CreateObject());
cJSON_AddStringToObject(metric, "mi_n", m->string);

比如 Facebook 的 wdt (https://github.com/facebook/wdt)，Twitter 的 ( https://github.com/lg/murder )，百度的 Ginko 等等，还有包括亚马逊 Apollo 里面的文件分发系统，它们那个和我们的有点不太一样，他们的是基于 S3 做的。
蜻蜓 - P2P文件分发
AOL - 集中配置管理

插件

PING_OPT_QOS:
  通过setsockopt()配置IP_TOS，实际上是设置IP头的Type-of-Service字段，用于描述IP包的优先级和QoS选项，例如IPTOS_LOWDELAY、IPTOS_THROUGHPUT等。

会议内容：
1. 监控需求。通过常驻进程记录上次历史数据，用于计数类型指标统计，目前方案可能会丢失部分监控数据。该问题CloudAgent方的郑力、王一力认可。
2. CloudAgent实现。目前针对日志采集普罗米修斯已经开始开发常驻监控端口，细节尚未讨论，该功能点基本功能满足指标上报的需求。
3. 对接CloudAgent方案。具体细节需要接下来讨论，包括对接方式、保活检测、常驻进程的管理等等。
4. 监控Uagent需求。需要提供动态修改功能，目前来看开发量还比较大；监控插件部分需要做少量修改，之前实现的中间件如haproxy、nginx可以继续使用。

参数修改
PING_OPT_TIMEOUT:
  设置超时时间。
PING_OPT_TTL:
  同样是通过setsockopt()配置IP_TTL。
PING_OPT_SOURCE:
  需要通过getaddrinfo()获取。

ENOTSUP

main()
 |-ping_construct()
 |-ping_setopt()
 |-ping_host_add()
 | |-ping_alloc()
 | |-getaddrinfo() reverse查找
 |-ping_send()
   |-ping_open_socket() 如果还没有打开
   |-select() 执行异步接口
   |-ping_receive_one()
   |-ping_send_one()


https://stackoverflow.com/questions/16010622/reasoning-behind-c-sockets-sockaddr-and-sockaddr-storage?answertab=votes

FIXME:
获取CPU核数
http://blog.csdn.net/turkeyzhou/article/details/5962041
浮点数比较
http://www.cnblogs.com/youxin/p/3306136.html
MemAvailable
https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=34e431b0ae398fc54ea69ff85ec700722c9da773

https://stackoverflow.com/questions/1631450/c-regular-expression-howto

Share your terminal over the web
https://github.com/tsl0922/ttyd
http-parser
https://github.com/nodejs/http-parser
picohttpparser
https://github.com/h2o/picohttpparser
为什么会使用http-parser
https://stackoverflow.com/questions/28891806/what-is-http-parser-where-it-is-used-what-does-it-do
http://mendsley.github.io/2012/12/19/tinyhttp.html
https://github.com/tinylcy/tinyhttpd




pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;
http://www.cnblogs.com/renxinyuan/p/3875659.html
http://blog-kingshaohua.rhcloud.com/archives/54

设置线程名称
http://blog.csdn.net/jasonchen_gbd/article/details/51308638
https://gxnotes.com/article/78417.html
http://www.cprogramming.com/debugging/segfaults.html
http://cering.github.io/2015/11/10/%E8%BD%AC-%E8%AF%A6%E8%A7%A3coredump/
https://www.ibm.com/developerworks/cn/linux/l-cn-deadlock/
http://blog.jobbole.com/106738/
http://blog.csdn.net/gebushuaidanhenhuai/article/details/73799824
http://www.cnblogs.com/yuuyuu/p/5103744.html
http://blog.csdn.net/sunshixingh/article/details/50988109
https://michaelyou.github.io/2015/03/07/epoll%E7%9A%84%E4%BA%8B%E4%BB%B6%E8%A7%A6%E5%8F%91%E6%96%B9%E5%BC%8F/
http://kimi.it/515.html
https://jeff-linux.gitbooks.io/muduo-/chapter2.html
https://www.zhihu.com/question/20502870
http://www.firefoxbug.com/index.php/archives/1942/
http://cr.yp.to/daemontools.html
http://www.voidcn.com/article/p-vfwivasm-ru.html

如果没有将线程设置为 detached ，而且没有显示的 pthread_exit()，那么在通过 valgrind 进行测试时会发现出现了内存泄露。

在通过 pthread_key_create() 创建私有变量时，只有调用 pthread_exit() 后才会调用上述函数注册的 destructor ；例如主进程实际上不会调用 destructors，此时可以通过 atexit() 注册回调函数。



################################
# Core Dump
################################
http://happyseeker.github.io/kernel/2016/03/04/core-dump-mechanism.html
http://blog.csdn.net/work_msh/article/details/8470277


←





O_NONBLOCK VS. O_NDELAY
http://blog.csdn.net/ww2000e/article/details/4497349

SO_ACCEPTFILTER
http://blog.csdn.net/hbhhww/article/details/8237309



Address already in use

该错误信息是由于返回了 EADDRINUSE 错误码，通常是由 TCP 套接字的 TIME_WAIT 状态引起，该状态在套接字关闭后会保留约 2~4 分钟，只有在该状态 TIME_WAIT 退出之后，套接字被删除，该地址才能被重新绑定而不出问题。

在 C 中，可以通过如下方式设置端口允许重用。

int opt = 1;
setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

/linux-monitor-cpu.html
ps -ax -o uid,pid,ppid,tpgid,pgrp,session,lstart,cmd

Linux查看某个文件被谁占用
当用户卸载某个目录的时候，因其他用户在当前目录或者当前目录在运行一个程序，卸载时报busy的处理办法：

1：fuser -av /mnt
查看占用mnt的程序以及pid，根据pid去kill -9

2：fuser -km /mnt
查看占用mnt的程序并自动kill
-m是指定被占用的目录，-k是kill

3：losf /mnt
查看正在使用的某个文件
4：递归查看某个目录的文件信息
lsof +D /mnt/fat

5:列出某个用户打开的文件信息

lsof -u student

6：列出某个程序打开的文件信息

lsof -c mysql
http://blog.csdn.net/kozazyh/article/details/5495532


System V IPC 分三类：消息队列、信号量、共享内存区，都采用 `key_t` 作为其内部使用的标示，该类型在 `<sys/types.h>` 中定义，一般是 32 位整数。

通常可以使用 ftok() 函数，也就是 file to key，把一个已存在的路径名和一个整数标识符转换成一个 `key_t` 值，该函数声明如下：

#include <sys/ipc.h>
key_t ftok (const char *pathname, int proj_id);

pathname 通常是跟本应用相关的路径；proj_id 指的是本应用所用到的 IPC 的一个序列号，通常约定好，这样可以获取相同的 `key_t` 值。

注意，需要保证该路径应用程序可以访问，并且在运行期间不能删除。

#include <stdio.h>        
#include <stdlib.h>   
#include <sys/stat.h> 
 
int main() 
{ 
        char filename[50]; 
        struct stat buf; 
        int ret; 
        strcpy(filename, "/home/satellite/" ); 
        ret = stat( filename, &buf ); 
        if(ret) { 
                printf( "stat error\n" ); 
                return -1; 
        } 
 
        printf( "the file info: ftok( filename, 0x27 ) = %x, st_ino = %x, st_dev= %x\n", ftok( filename, 0x27 ), buf.st_ino, buf.st_dev );

        return 0; 
}

通过执行结果可看出，ftok获取的键值是由ftok()函数的第二个参数的后8个bit，st_dev的后两位，st_ino的后四位构成的

### semget

创建一个新的信号量或获取一个已经存在的信号量的键值。

#include <sys/sem.h>
int semget(key_t key, int nsems, int semflg);

key: 为整型值，可以自己设定，有两种场景
   1. IPC_PRIVATE 通常为 0，创建一个仅能被本进程给我的信号量。
   2. 非 0 的值，可以自己手动指定，或者通过 ftok() 函数获取一个唯一的键值。
nsems: 初始化信号量的个数。
semflg: 信号量创建方式或权限，包括了 IPC_CREAT(不存在则创建，存在则获取)；IPC_EXCL(不存在则建立，否则报错)。

#include <stdio.h>
#include <sys/sem.h>

int main()
{
 int semid;
 semid = semget(666, 1, IPC_CREAT | 0666); // 创建了一个权限为666的信号量
 printf("semid=%d\n", semid);
 return 0;
}

可以用 ipcs –s 来查看是否创建成功。
用 ipcrm -s semid 号来删除指定的信号量。



################################
# CMake
################################


针对特定对象，可以通过如下方式指定特定的编译选项、头文件路径、宏定义。

target_compile_definitions(audio_decoder_unittests
 PRIVATE "AUDIO_DECODER_UNITTEST"
 PRIVATE "WEBRTC_CODEC_PCM16")
target_include_directories(audio_decoder_unittests
 PRIVATE "interface"
 PRIVATE "test"
 PRIVATE "../codecs/g711/include")
target_compile_options(RTPencode PRIVATE "/wd4267")


## 配置文件
CheckSymbolExists.cmake   宏定义检查


################################
# Curl
################################

详细可以查看 http://php.net/manual/zh/function.curl-setopt.php
https://moz.com/devblog/high-performance-libcurl-tips/

CURLOPT_NOSIGNAL

CURLOPT_WRITEFUNCTION 用于设置数据读取之后的回调函数，通过该函数可以保存结果，其函数声明如下。
    size_t function( char *ptr, size_t size, size_t nmemb, void *userdata);
CURLOPT_WRITEDATA 定义了上述函数声明中userdata的值。

#include <stdio.h>
#include <curl/curl.h>

size_t save_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    fp = fopen("index.html", "wb");

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "www.baidu.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }

    return 0;
}

CURLOPT_USERNAME
CURLOPT_PASSWORD 分别设置用户名密码，低版本可以通过CURLOPT_USERPWD选项设置，其值为"user:password" 。

CURLOPT_TIMEOUT_MS 设置超时时间。

CURLOPT_MAXREDIRS
CURLOPT_FOLLOWLOCATION 允许重定向，以及设置重定向的跳转次数。


CURLOPT_SSL_VERIFYPEER 验证证书，证书信息可以通过 CURLOPT_CAINFO 设置，或在 CURLOPT_CAPATH 中设置证书目录。

CURLOPT_SSL_VERIFYHOST 设置为 1 是检查服务器SSL证书中是否存在一个公用名(common name)。译者注：公用名(Common Name)一般来讲就是填写你将要申请SSL证书的域名 (domain)或子域名(sub domain)。 设置成 2，会检查公用名是否存在，并且是否与提供的主机名匹配。 0 为不检查名称。 在生产环境中，这个值应该是 2（默认值）。


./configure  \
    --disable-shared --enable-static                     不使用动态库，而是静态编译
    --without-libidn2 --without-winidn                   忽略国际化库
 --disable-ipv6 --disable-unix-sockets                关闭IPV6以及Unix Socket
 --without-ssl --without-gnutls --without-nss         关闭安全配置项1
 --without-libssh2 --disable-tls-srp --without-gssapi 关闭安全配置项2
    --without-zlib                                       不支持压缩
 --disable-ares --disable-threaded-resolver      
 --without-librtmp  --disable-rtsp                    关闭不需要的协议1
 --disable-ldap --disable-ldaps                       关闭不需要的协议2
 --disable-dict --disable-file --disable-gopher
 --disable-ftp --disable-imap --disable-pop3
 --disable-smtp --disable-telnet --disable-tftp
 --disable-sspi                                       Windows选项
 --without-libpsl --without-libmetalink
    --with-nghttp2
 
resolver:         ${curl_res_msg}
Built-in manual:  ${curl_manual_msg}
--libcurl option: ${curl_libcurl_msg}
Verbose errors:   ${curl_verbose_msg}
ca cert bundle:   ${ca}${ca_warning}
ca cert path:     ${capath}${capath_warning}
ca fallback:      ${with_ca_fallback}
HTTP2 support:    ${curl_h2_msg}                                                                               
Protocols:        ${SUPPORT_PROTOCOLS}


curl_code = curl_easy_perform (session);
long http_code = 0;
curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_code);  /* 获取返回码 */



http://187.0.0.1:8080/status



使用 curl 测量 Web 站点的响应时间。

curl -o /dev/null -s -w '%{http_code}-%{time_namelookup}:%{time_connect}:%{time_appconnect}:%{time_pretransfer}:%{time_redirect}:%{time_starttransfer}:%{time_total}\n' 'http://187.0.0.1:8080/status'

time_namelookup     DNS解析时间，从请求开始到DNS解析完毕所用时间
time_connect     建立到服务器的 TCP 连接所用的时间
time_appconnect     连接建立完成时间，如SSL/SSH等建立连接或者完成三次握手时间
time_pretransfer    准备传输的时间，对于一些协议需要做一些初始化操作
time_redirect       重定向时间，包括到最后一次传输前的几次重定向的DNS解析、连接、预传输、传输时间
time_starttransfer 传输时间，在发出请求之后，服务器返回数据的第一个字节所用的时间
time_total          完成请求所用的时间
speed_download      下载速度，单位是字节/秒
http_code           返回码

注意，如果某一步失败了，该步骤对应的值实际上显示的是 0 ，此时需要通过总时间减去上一步的消耗时间。

上述的执行，是在执行 curl_easy_perform() 函数的时候开始的，在 docs/examples 目录下，有很多的参考实例。


const char *optstr;
char *endptr = NULL;
double v;
long value;

/* NOTE: if optstr = NULL, strtol() will raise 'Segmentation fault' */
optstr = "  not a number"; /* errno=0, optstr == endptr */
errno = 0;
value = strtol(optstr, &endptr, /* base = */ 0);
assert(value == 0);
assert(errno == 0);
assert(optstr == endptr);
printf("errno=%d, optstr=%p, endptr=%p, endchar='%c'/0x%02x, value=%ld\n", errno, optstr, endptr, *endptr, *endptr, value);

optstr = "  12part number";
errno = 0;
value = strtol(optstr, &endptr, /* base = */ 0);
printf("errno=%d, optstr=%p, endptr=%p, endchar='%c'/0x%02x, value=%ld\n", errno, optstr, endptr, *endptr, *endptr, value);

optstr = "  12";
errno = 0;
value = strtol(optstr, &endptr, /* base = */ 0);
printf("errno=%d, optstr=%p, endptr=%p, endchar='%c'/0x%02x, value=%ld\n", errno, optstr, endptr, *endptr, *endptr, value);


memory-barriers
https://www.kernel.org/doc/Documentation/memory-barriers.txt
http://ifeve.com/linux-memory-barriers/
https://dirtysalt.github.io/memory-barrier.html
http://preshing.com/20120625/memory-ordering-at-compile-time/
http://events.linuxfoundation.org/sites/events/files/slides/dbueso-elc2016-membarriers-final.pdf
http://larmbr.com/2014/02/14/the-memory-barriers-in-linux-kernel(1)/
https://www.zhihu.com/question/47990356
http://www.wowotech.net/kernel_synchronization/Why-Memory-Barriers.html
http://blog.csdn.net/qb_2008/article/details/6840570

网卡缓存
https://zrj.me/archives/1102



http://www.cnblogs.com/bodhitree/p/6018369.html
sed高级用法
https://www.zhukun.net/archives/6975
http://gohom.win/2015/06/20/shell-symbol/
mysql core dump
http://xiezhenye.com/2015/05/%E8%8E%B7%E5%8F%96-mysql-%E5%B4%A9%E6%BA%83%E6%97%B6%E7%9A%84-core-file.html
文件句柄数
http://blog.sina.com.cn/s/blog_919f173b01014vol.html
http://www.opstool.com/article/166
rpm 升级到旧的版本
http://ftp.rpm.org/max-rpm/s1-rpm-upgrade-nearly-identical.html#S2-RPM-UPGRADE-OLDPACKAGE-OPTION
https://stackoverflow.com/questions/2452226/master-branch-and-origin-master-have-diverged-how-to-undiverge-branches
C hash算法
http://troydhanson.github.io/uthash/index.html

https://blog.zengrong.net/post/1746.html
https://stackoverflow.com/questions/9537392/git-fetch-remote-branch

http://www.cnblogs.com/yuuyuu/p/5103744.html
https://codeascraft.com/2011/02/15/measure-anything-measure-everything/


http://wkevin.github.io/2014/05/05/git-submodule/
http://xstarcd.github.io/wiki/sysadmin/ntpd.html
ps -ax -o lstart,cmd


可以通过如下命令指定分支，提交后会修改原数据中的 `Subproject commit` 。
cd submodule_directory
git checkout v1.0
cd ..
git add submodule_directory
git commit -m "moved submodule to v1.0"
git push

http://docs.python-requests.org/zh_CN/latest/user/quickstart.html
https://liam0205.me/2016/02/27/The-requests-library-in-Python/

















https://github.com/MarkDickinson/scheduler
https://github.com/Meituan-Dianping/DBProxy
https://github.com/greensky00/avltree
http://www.freebuf.com/sectool/151426.html
http://www.freebuf.com/sectool/150367.html



tar 打包可以通过 --exclude=dir 排除。
通过 `--transform` 参数可以根据 `sed` 语法进行一些转换，例如增加前缀 `'s,^,prefix/,'` 或者 `s%^%prefix/%`。



支持数据类型float, int, str, text, log

支持函数：
    abschange 计算最新值和上次采集值相减的绝对值，对于字符串0/相同、1/不同，1->5=4, 3->1=2

https://www.zabbix.com/documentation/3.0/manual/appendix/triggers/functions


支持数据类型：
   Numeric (unsigned) - 64位无符号整数；
   Numeric (float) - 浮点数，可以存储负值，范围是 [-999999999999.9999, 999999999999.9999]，同时可以支持科学计算 1e+7、1e-4；
   Character - 短文本数据，最大255字节；
  
  
Log - 具有可选日志相关属性的长文本数据(timestamp, source, severity, logeventid)
Text - 长文本数据


Tim O’Reilly and Crew [5, p.726]
The load average tries to measure the number of active processes at any time. As a measure of CPU utilization, the load average is simplistic, poorly defined, but far from useless.

Adrian Cockcroft [6, p. 229]
The load average is the sum of the run queue length and the number of jobs currently running on the CPUs.


默认没有移动平均值计算，只是针对单个值进行计算。

threshold_tree 仍然通过format_name格式化名称。
 
目前分为了三种类型，分别为 Host > Plugin > Type ，需要按照层级进行排列，例如 Host 下面可以有 Plugin 和 Type 段；Plugin 下可以有 Type 但是不能有 Host 。

其它的配置项用于一些类似阈值的判断等，只能在 Type 下面配置。

FailureMax Value
WarningMax Value
 设置报警的上限值，如果没有配置则是正无穷。告警发送规则如下：
    A) (FailureMax, +infty) 发送 FAILURE 通知；
    B) (WarningMax, FailureMax] 发送 WARNING 通知；

FailureMin Value
WarningMin Value
 设置报警的下限值，如果没有配置则是负无穷。告警发送规则如下：
    A) (-infty, FailureMin) 发送 FAILURE 通知；
    B) [FailureMin, WarningMin) 发送 WARNING 通知；

Persist true|false(default)
 多久发送一次报警，设置规则如下：
    true) 每次超过阈值之后都会发送一次报警通知；
    false) 只有在状态发生转换且前一次状态是OKAY时才会发送一次通知。

PersistOK true|false(default)
 定义如何发送OKAY通知，设置规则如下：
    true) 每次在正常范围内都会发送通知；
    false) 当本次状态正常而且之前状态不正常时才发送一次OK通知。

Hysteresis Value
 迟滞作用，用于处理在一个状态内重复变换，在该阈值范围内不会告警。

Hits Value
    告警条件必须连续满足多少次之后才会发送告警。

Interesting true(default)|false
 发现数据未更新时是否发送告警，会根据插件的采集时间间隔以及 Timeout 参数判断是否有事件发生。
    true) 发送FAILURE报警；
    false) 忽略该事件。

DataSource  <-> Types.db 中字段，例如midterm
Host <-> host
Plugin <-> plugin
Type <-> type
Instance <-> type_instance

1. 根据value list中的参数获取到具体的配置。
2.

Invert true|false
Percentage true|false

以 loadavg 为例，其实现在 fs/proc/loadavg.c 中。
calc_global_load()
监控指标，其中监控插件包括了

https://en.wikipedia.org/wiki/Moving_average
http://www.perfdynamics.com/CMG/CMGslides4up.pdf
https://zh.wikipedia.org/wiki/%E7%A7%BB%E5%8B%95%E5%B9%B3%E5%9D%87

移动平均 (Moving Average) 可以处理短期波动，反映长期趋势或周期，从数学上看做是卷积。


## 简单移动平均

Simple Moving Average, SMA 将变量的前 N 值做平均。

SMA = (V1 + V2 + ... + Vn) / n

当有新值之后，无需重复计算，只需要将最老的旧值删除，然后加入新值。

SMAn = SMAn-1 - V1/n + V/n

这样需要保存 N 个值。

## 加权移动平均

Weighted Moving Average, WMA 也就是在计算平均值时将部分数据乘以不同数值，

## 指数移动平均

Exponential Moving Average, EMA


----- 锁定用户该用户不能再次登录
ALTER USER username ACCOUNT LOCK;
----- 解锁用户
ALTER USER username ACCOUNT UNLOCK;

aussdb plugin: Connect to database failed: FATAL:  The account has been locked.
FATAL:  The account has been locked.

Zabbix上报数据格式
http://www.ttlsa.com/zabbix/zabbix-active-and-passive-checks/
Open-falcon 上报数据
http://blog.niean.name/2015/08/06/falcon-intro
main_timer_loop() 周期计算定义的触发值，如果有事件发生，那么就直接写入到数据库中。


timer_thread()     main_timer_loop时间相关的处理
 |-process_time_functions()
 | |-DCconfig_get_time_based_triggers() 从缓存中获取trigger表达式
 | |-evaluate_expressions()       触发器表达式的主要处理函数，同时会产生事件
 | | |-substitute_simple_macros() 宏分为两类，分别是{...} {$...}
 | | |-substitute_functions()
 | | | |-zbx_evaluate_item_functions()
 | | |   |-evaluate_function()
 | | |-evaluate()
 | |
 | |-DBbegin()
 | |-process_triggers() T:triggers
 | | |-process_trigger()
 | |   |-add_event()              会保存到内存的events数组中
 | |-process_events()             处理事件，主要是将新事件插入数据库
 | | |-save_events() T:events
 | | |-process_actions() T:actions
 | | |-clean_events()
 | |-DBcommit()
 |
 |-process_maintenance()


源码解析
https://jackywu.github.io/articles/zabbix_server%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90/

## 表结构

{<server>:<key>.<function>(<parameter>)}<operator><constant>

hosts  包含了主机以及模板信息。
    select hostid, host from hosts where status = 0;
    hostid  主机ID
 host    主机、模板名
 status  0->主机 3->模板
goups   主机的逻辑分组
hosts_grous 分组和主机之间的关联关系
items 监控项，保存了每个主机的监控项，该监控项来自的模板ID
    select itemid, name, key_, templateid, delay, status, units from items where hostid = 10107;
triggers 触发器，其中表达式中使用的是function id
 select triggerid, expression, status, value, description from triggers;
 select * from functions where functionid = 13302;
functions

Paxos协议解析
https://zhuanlan.zhihu.com/p/21438357?refer=lynncui

https://github.com/hanc00l/wooyun_public
https://github.com/niezhiyang/open_source_team
http://www.jianshu.com/p/43c604177c08
http://kenwheeler.github.io/slick/

http://lovestblog.cn/blog/2016/07/20/jstat/
http://blog.phpdr.net/java-visualvm%E8%AE%BE%E7%BD%AEjstat%E5%92%8Cjmx.html


http://metrics20.org/spec/
Stack Overflow 的架构
https://zhuanlan.zhihu.com/p/22353191
GCC部分文件取消告警
http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
http://gcc.gnu.org/onlinedocs/gcc-4.0.4/gcc/Warning-Options.html









SHELL反弹
https://segmentfault.com/a/1190000010975294
各种环境下反弹shell的方法
http://www.zerokeeper.com/experience/a-variety-of-environmental-rebound-shell-method.html

libev信号处理
http://blog.csdn.net/gqtcgq/article/details/49688027
信号相关参考
http://www.cnblogs.com/mickole/p/3191281.html

## Joinable VS. Detached

默认创建的线程是 Joinable 的，也就是可以通过 `pthread_join()` 函数在任何其它线程中等待它的终止。

各个线程是独立运行的，也就意味着在调用 join 之前，目标线程可能已经终止了，那么如果一个线程是 Joinable ，POSIX 标准要求必须保持、维护一些信息，至少是线程 ID 以及返回值。

实际上，对于大多数系统，如 Linux、Sloaris 等，在实现时通过 Thread Control Block, TCB 保存一些相关的信息，包括线程 ID、属性、入口函数、参数、返回值；调度策略、优先级；信号掩码、信号栈等等；而且通常是一次性分配堆栈和 TCB，例如单次 mmap() 调用，并把 TCB 放在栈的开始位置处。

也就是说，不能立即释放对应的资源，这样就会造成浪费。

### Detached

当设置为 Detached 时，表明对于开启的线程，如果一旦执行结束，则会立即清理资源。

可以在创建线程时设置该属性，或者调用 pthread_detach() 函数；当设置了该属性后，如果再次调用 pthread_join() 则会返回 EINVAL 错误。
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void *thread(void *dummy)
{
        (void) dummy;
        sleep(1);
        return NULL;
}

void detach_state(pthread_t tid, const char *tname)
{
        int rc;

        rc = pthread_join(tid, NULL);
        if (rc == EINVAL)
                printf("%s is detached\n", tname);
        else if (rc == 0)
                printf("%s was joinable\n", tname);
        else
                printf("ERROR: %s rc=%d, %s\n", tname, rc, strerror(rc));
}

int main(void)
{
        /* TODO: Check the return value */

        /* normal thread creation */
        pthread_t tid1;
        pthread_create(&tid1, NULL, thread, NULL);
        detach_state(tid1, "thread1"); /* joinable */

        /* detach thread from main thread */
        pthread_t tid2;
        pthread_create(&tid2, NULL, thread, NULL);
        pthread_detach(tid2);
        detach_state(tid2, "thread2"); /* detached */

        /* create detached thread */
        pthread_attr_t attr;
        pthread_t tid3;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&tid3, &attr, thread, NULL);
        detach_state(tid3, "thread3");

        return EXIT_SUCCESS;
}


对于该特性，需要注意如下的规则：
1. 不要重复 join 一个线程，已经 join 线程的栈空间已经被回收，再次调用无法获取对应的信息；
2. 不要 join 一个是 detach 的线程，分离的线程栈空间是由系统内部来做回收的；

sigprocmask() 函数能够根据

根据参数 how 实现对信号集的操作，主要包括如下三种：
* SIG_BLOCK 在进程当前阻塞信号集中添加set指向信号集中的信号，相当于 mask=mask|set；
* SIG_UNBLOCK 如果进程阻塞信号集中包含set指向信号集中的信号，则解除对该信号的阻塞，相当于 mask=mask|~set；
* SIG_SETMASK 更新进程阻塞信号集为set指向的信号集，相当于mask=set。

EPOLLIN 对应的文件描述符可读，包括对端 Socket 正常关闭(返回字节为 0)；
EPOLLOUT 对应的文件描述符可写；
EPOLLPRI 所谓的带外数据，也就是在通过 send() 发送时指定 MSG_OOB 参数；
EPOLLERR 发生本地错误；
EPOLLHUP 表示客户端套接字已经断开连接；
EPOLLET	使用边沿触发，默认是水平触发；
EPOLLONESHOT 只监听一次事件，事件触发后会自动关闭，如果需要再次监听则需要重新设置。

1、listen fd，有新连接请求，触发EPOLLIN。
2、对端发送普通数据，触发EPOLLIN。
3、带外数据，只触发EPOLLPRI。

1. 当对端正常关闭时，会触发 EPOLLIN 和 EPOLLRDHUP 事件，而非 EPOLLERR 和 EPOLLHUP，此时读缓冲区大小为 0 。

注意，如果是对端发生错误，不会主动触发 EPOLLERR 错误，只有在下次调用读写时才会触发。

#### 文件描述符异常

一般来说，不需要手动从 epoll() 中删除，系统会自动删除掉，不过按照 man epoll(7) 的 Q6 介绍，需要保证文件描述符没有被 dup() 复制过。

#### 同时注册两次

如果将相同的 fd 添加到 epoll_set 两次，接口会返回 EEXIST 报错，不过可以通过 dup 接口复制一份并添加，例如多线程中的使用。

#### 关于 file descriptors and file descriptions
https://idea.popcount.org/2017-03-20-epoll-is-fundamentally-broken-22/
https://blog.codingnow.com/2017/05/epoll_close_without_del.html


### 边缘触发 VS. 水平触发

简单来说，两者区别如下：

1. Level Triggered 水平触发：有事件触发时会通过 epoll_wait() 去处理，如果一次处理不完会持续通知，直到处理完成，当系统中有大量不需要的描述符时会大大降低处理效率。
2. Edge Triggered 边缘触发：如果本次没有处理完成，那么即使下次调用 epoll_wait() 也不会再次通知，知道出现下次的读写事件时才会再次触发。

其中 select() poll() 的模型都是水平触发模式，信号驱动 IO 是边缘触发模式，epoll() 模型即支持水平触发，也支持边缘触发，默认是水平触发。

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
        glob_t globbuf;
        struct stat statbuf;
        int rc;
        size_t pathc;
        char **pathv;

        rc = glob("/var/run/haproxy[0-9]*.sock", GLOB_ERR | GLOB_NOSORT, NULL, &globbuf);
        if (rc != 0)
                return -1;

        pathv = globbuf.gl_pathv;
        pathc = globbuf.gl_pathc;
        printf("Got #%d matches\n", pathc);

        for (; pathc-- > 0; pathv++) {
                rc = lstat(*pathv, &statbuf);
                if (rc < 0 || !S_ISSOCK(statbuf.st_mode))
                        continue;
                printf("Match path: %s\n", *pathv);
        }

        globfree(&globbuf);
        return 0;
}

mobius








aspire



介绍TCP流
http://kaiyuan.me/2015/09/04/TCP%E7%9A%84%E6%94%B6%E5%8F%91%E5%8C%85%E6%9C%BA%E5%88%B6%E8%A7%A3%E6%9E%90/















返回值：如果执行成功则返回子进程识别码(PID), 如果有错误发生则返回-1. 失败原因存于errno 中.

https://github.com/krallin/tini
https://github.com/Yelp/dumb-init

## tini

其功能类似于 init 进程，一般用于容器中，

在编译静态二进制文件时会依赖 glibc 的静态库，对于 CentOS 来说，需要通过 `yum install glibc-static` 安装。

可以通过该工程查看 CMake 的编写，以及编写类似 init 进程的注意事项。


在 /post/linux-kernel-process 中有关于孤儿进程和僵尸进程的介绍，简单来说：

* 孤儿进程。当父进程被 kill 掉，其子进程就会成为孤儿进程 (Orphaned Process)，并被 init(PID=1) 所接管。


### 孤儿进程如何被接管

在 Linux 内核中，有如下的代码 [Kernel find_new_reaper()](https://github.com/torvalds/linux/blob/eae21770b4fed5597623aad0d618190fa60426ff/kernel/exit.c#L479) ，其开头的注释摘抄如下：

/*
 * When we die, we re-parent all our children, and try to:
 * 1. give them to another thread in our thread group, if such a member exists
 * 2. give it to the first ancestor process which prctl'd itself as a
 *    child_subreaper for its children (like a service manager)
 * 3. give it to the init process (PID 1) in our pid namespace
 */

也就是说，接管分三步：A) 找到相同线程组里其他可用的线程；B) 如果没有找到则进行第二步C) 最后交由 PID=1 的进程管理。



### SubReaper

当一个进程被标记为 child_subreaper 后，这个进程所创建的所有子进程，包括子进程的子进程，都将被标记拥有一个 subreaper。

那么当某个进程成为了孤儿进程时，会沿着它的进程树向祖先进程找一个最近的是 child_subreaper 且运行着的进程，这个进程将会接管这个孤儿进程。

http://adoyle.me/blog/orphaned-process-and-zombie-process-and-docker.html


https://stackoverflow.com/questions/9305992/if-threads-share-the-same-pid-how-can-they-be-identified/9306150#9306150

## RBASH

restricted bash, rhash 也就是受限制的 bash，实际上这只是指向 bash 的软连接，也可以通过 `bash -r` 参数启动，作用相同。

此时，启动的这个 BASH 会在某些功能上受限制，包括：



* 通过 cd 来改变工作目录
* 设置或取消环境变量 SHELL、PATH、ENV、BASH_ENV
* 命令名中不能包含目录分隔符 '/'


包含有 ‘/’ 的文件名作为内置命令 ‘.’ 的参数
hash 内置命令有 -p 选项时的文件名参数包含 ‘/’
在启动时通过 shell 环境导入函数定义
在启动时通过 shell 环境解析 SHELLOPTS 的值
使用 >，>|， <>， >&， &>， >> 等重定向操作符
使用 exec 内置命令
通过 enable 内置命令的 -f 和 -d 选项增加或删除内置命令
使用 enable 内置命令来禁用或启用 shell 内置命令
执行 command 内置命令时加上 -p 选项
通过 set +r 或 set +o restricted 关闭受限模式

## 逃逸

rbash 提供的受限环境的安全程度取决于用户能执行的命令，很多命令都能调用外部命令，从而导致逃逸出受限环境。

例如用 vim 打开一个文件，然后通过 `!bash` 执行外部命令，那么就可以启动一个不受限的 bash，这对 more，less，man 等命令同样有效。如果还能执行脚本，如 python、perl 等，则有更过的方式来启动一个不受限的 shell 。

也可以通过如下方式执行：

$ BASH_CMDS[a]=/bin/sh;a
$ /bin/bash
$ export PATH=$PATH:/bin:/usr/bin


要让 rbash 更安全，可以限制用户能够执行的命令，如我们让用户执行执行 ssh 命令。一种方法是，修改 PATH 环境变量。

例如我们创建一个 ruser 用户，让他只能执行 ssh 命令：

$ ls -s /bin/bash /bin/rbash

$ useradd -s /bin/rbash ruser

$ chown -R root:ruser /home/ruser/.bashrc /home/ruser/.bash_profile

$ chmod 640 /home/ruser/.bashrc /home/ruser/.bash_profile

$ mkdir /home/ruser/bin
然后修改 PATH 环境变量的值为 /home/ruser/bin，并将允许执行的命令放到这个目录下。：

$ echo "export PATH=/home/ruser/bin" >> /home/ruser/.bash_profile
把用户可执行的命令链接到用户 PATH 路径下：

$ ln -s /user/bin/ssh /home/ruser/bin/ssh
这样就可以只让登录的用户执行 ssh 命令。

## 限制用户执行命令

git比较两个分支
https://blog.csdn.net/u011240877/article/details/52586664







一个简单的时序数据库
https://github.com/Cistern/catena

https://www.byvoid.com/zhs/blog/string-hash-compare
http://www.open-open.com/lib/view/open1451882746667.html
https://hitzhangjie.github.io/jekyll/update/2018/05/19/golang-select-case%E5%AE%9E%E7%8E%B0%E6%9C%BA%E5%88%B6.html





HTTP平滑升级
https://segmentfault.com/a/1190000004445975

### WithCancel

func WithCancel(parent Context) (ctx Context, cancel CancelFunc)

当调用 Cencel() 函数时，相关的子协程可以通过 `ctx.Done()` 手动相关的请求。

###

https://deepzz.com/post/golang-context-package-notes.html
https://juejin.im/post/5a6873fef265da3e317e55b6

## 常见问题

### 成员变量

cannot refer to unexported field or method ver

在 GoLang 中要提供给外面访问的方法或是结构体必须是首字母大写，否则会报错。

exec.Command() 新建一个对象，但是没有执行

Output() 会等待任务执行完成并收集输出。

## GoReman 进程管理
https://github.com/polaris1119/The-Golang-Standard-Library-by-Example/blob/master/chapter10/10.1.md
https://www.jianshu.com/p/49e83c39cffc

/post/linux-commands-text.html

AWK统计操作


grep 'Got result' /tmp/foobar.log | awk '{s[$2]++} END{ for(i in s){print i, s[i]} }' | sort
http://blog.51cto.com/6226001001/1659824
https://shaohualee.com/article/691


使用较多的是网络编程中，假设存在 A 调用 B 的 API，然后 B 再调用 C 的 API，如果要取消 `A->B` 的调用，按照正常的逻辑也应该要取消 `B->C` 的调用，那么此时就可以通过传递 Context 以及正常的逻辑判断来实现。



Linux内核的DEBUG方法
https://medium.com/square-corner-blog/a-short-guide-to-kernel-debugging-e6fdbe7bfcdf




#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define log_it(fmt, args...)  do { printf(fmt, ## args); putchar('\n'); } while(0)

int main()
{
        pid_t pid;
        int fd[2], rc, i;
        char buffer[1024];

        rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
        if (rc < 0) {
                log_it("create sockpaire failed, %s", strerror(errno));
                return -1;
        }

        pid = fork();
        if (pid < 0) {
                log_it("fork failed, %s", strerror(errno));
                return -1;
        } else if (pid == 0) {
                close(fd[0]);
                strcpy(buffer, "hello socketpair");
                for (i = 0; i < 10; i++) {
                        write(fd[1], buffer, strlen(buffer));
                        usleep(200000);
                }
                strcpy(buffer, "exit");
                write(fd[1], buffer, strlen(buffer));
                close(fd[1]);
        } else {
                close(fd[1]);
                while (1) {
                        rc = read(fd[0], buffer, sizeof(buffer) - 1);
                        if (rc > 0) {
                                buffer[rc] = 0;
                                if (strcmp(buffer, "exit") == 0)
                                        break;
                                log_it("father: %s", buffer);
                        }
                }
                close(fd[0]);
        }

        return 0;
}

查找 socketpair 的对端。


其中 `/proc/<PID>/fd/<FD>` 中显示的数字是虚拟套接字文件系统中套接字的 `inode` 编号，创建管道或套接字对时，每个端口都会连续接收一个 inode 编号

如下命令实际上很难查找到，并没有将内核的信息暴露出来。

lsof -c progname
lsof -c parent -c child1
ls -l /proc/$(pidof server)/fd
cat /proc/net/unix

$ ss -xp | grep foobar


HTTP服务
http://fuxiaohei.me/2016/9/20/go-and-http-server.html



musl libc一个安全用户嵌入式设备的库
http://www.musl-libc.org/
http://www.etalabs.net/compare_libcs.html






Rootkit 是一套由入侵者留在系统中的后门程序，通常只有在系统被入侵后被安装进系统，用于长期控制，主要特征为：隐藏、操纵、收集数据。

Linux RooKit 可以简单地分为用户态和内核级，一些新的技术可能支持 BIOS、PIC、EFI 。

其中用户态通常是替换一些二进制文件，如 ps、netstat、ls 等，从而实现进程隐藏、网络连接信息隐藏、文件隐藏等功能。内核态由于隐蔽性好、攻击能力强，逐渐成为了主流，分为了 LKM 和 非LKM 类型。

一般其包含的功能有：远程指令执行、信息收集、文件隐藏、进程隐藏、网络连接隐藏、内核模块隐藏 。




https://github.com/mempodippy/vlany
https://github.com/maK-/maK_it-Linux-Rootkit




http://www.freebuf.com/articles/network/185324.html
https://github.com/iagox86/dnscat2
Netfilter RootKit
https://github.com/zionlion67/rootkit

Wildpwn：Unix通配符攻击工具
http://www.freebuf.com/sectool/185276.html

anti-rootkit
https://github.com/dgoulet/kjackal
http://www.ywnds.com/?p=6905
http://rkhunter.sourceforge.net/
http://www.cis.syr.edu/~wedu/Teaching/cis643/LectureNotes_New/Set_UID.pdf

https://www.ibm.com/developerworks/cn/linux/l-overflow/index.html

SQLite
http://huili.github.io/
http://www.iteye.com/blogs/subjects/deepfuture
http://www.cnblogs.com/hustcat/archive/2009/02/26/1398896.html
unix_dgram unix_stream unix_seqpacket










http://www.sqlite.org/howtocorrupt.html


### Atomic Operation

所谓的原子操作就是 "不可中断的一个或一系列操作"，这里说的是硬件的原子操作能力。

在单处理器系统(UniProcessor)来说，能够在单条指令中完成的操作都可以认为是 "原子操作"，因为中断只能发生于指令之间。

而对称多处理器(Symmetric Multi-Processor)来说则会有很大区别，由于多个处理器同时在独立运行，即使能在单条指令中完成的操作也有可能受到干扰。

但是多核之间通常由于在共享内存空间会导致异常，为此在 x86 平台上，CPU 提供了在指令执行期间对总线加锁的手段。

简单来说，增加了 HLOCK 用来对总线进行加锁，从而在使用某个或者某段指令之间其它 CPU 无法访问内存，这样对于单个 CPU 来说，内存中的资源对于这个 CPU 就是独有的。

一般来说，CPU 使用较多的是针对单个数据类型的原子操作，最常见的是整形。

https://blog.csdn.net/qq100440110/article/details/51194563
https://my.oschina.net/jcseg/blog/316726
https://software.intel.com/zh-cn/blogs/2010/01/14/cpucpu


1. 读取PIDFile，获取进程PID
    1.1 检查进程是否存在 /proc/<PID> 目录
	    不存在(通过kill -9强制杀死导致PIDFile未被清除)。
		     1.1.1 再次通过pidof检查所有进程，以防止PIDFile更新异常，或者被人为误更新PIDFile。
			       不存在，程序确实未启动，直接退出。
		存在。发送kill信号，等待一个安全时间进程自动退出。
	1.2 再次检查上次获取的PID及其子进程。
	    1.2.1 先向子进程组发送kill -9信号。
		1.2.2 再向父进程组发送kill -9信号。
	1.3 最后检查确保进程正常退出，如果未被清理输出告警信息。



/post/python-modules.html
搜索路径
一般来说，顺序为当前路径(`' '`)，环境变量 `PYTHONPATH` 指定路径，通过 `site` 模块生成的路径。

/post/python-tips.html
带 * 参数

注意，在进行参数传递时，也可以通过 `**` 进行字典的传参，示例如下。

#!/bin/python

def foobar(name, phone, addr):
    print name, phone, addr
#foobar("foobar", "137-0123", "US")
arg = dict(phone="137-0123", addr="US")
foobar("foobar", **arg)

https://docs.python.org/2/library/sys.html#sys.path


















https://matplotlib.org/faq/usage_faq.html

load 用来表示系统的负载，通过 `top` `uptime` `w` 命令或者 `cat /proc/loadavg` 查看当前系统前 1min 5min 15min 的负载平均值。

实际上在计算时采用的是指数平滑法，只是 Linux 内核中不允许直接做浮点运算，而且有多个 CPU 核，考虑到效率问题，从而导致计算的代码比较复杂。

$$s_{t}=\alpha x_t + (1 - \alpha) s_{t-1}$$

简单来说，算法比较简单，但是


We take a distributed and async approach to calculating the global load-avg
in order to minimize overhead.

The global load average is an exponentially decaying average of nr_running +
nr_uninterruptible.

Once every LOAD_FREQ:

  nr_active = 0;
  for_each_possible_cpu(cpu)
     nr_active += cpu_of(cpu)->nr_running + cpu_of(cpu)->nr_uninterruptible;

  avenrun[n] = avenrun[0] * exp_n + nr_active * (1 - exp_n)

也就是说，核心的是：A) 多 CPU 中如何获取到整个系统的 `nr_active`；B) 定点计算指数平滑法。

## CPU 核数

其实对于一台机器来说有几种类型：A) Multi Processor 也就是多个物理 CPU；B) Multi-Core Processor 一个物理 CPU 中有多个核。

另外，为了提高并行计算的性能，Intel 还引入了 Hyper-Threading 特性，此时，一个物理 CPU 对于操作系统来说表现为两个。


我们以 `/proc/loadavg` 为准，实际上实现在
fs/proc/loadavg.c

http://ilinuxkernel.com/?p=869
http://brytonlee.github.io/blog/2014/05/07/linux-kernel-load-average-calc/
https://www.teamquest.com/files/9214/2049/9761/ldavg1.pdf

内核计算负载的公式如下：

$$load_t=load_{t-1}e^{\frac{-5}{60R}}+queue(1-e^{\frac{-5}{60R}})$$

其中 $R$ 为 1 5 15 分别对应了 1min 5min 15min 的负载计算方法；$queue$ 对应了当前的运行队列长度。

通过 `unsigned long avenrun[3];` 存放负载情况，因为无法进行浮点运算，所以低 11 位用来保存负载的小数部分，其余的高位用来保存整数部分。

内核在进程调度时会调用 `calc_global_load()` 函数来计算负载，一般是每 5s 更新一次。


unsigned long avenrun[3]; // 用于存放

#define FSHIFT          11              /* nr of bits of precision */
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_FREQ       (5*HZ+1)        /* 5 sec intervals */
#define EXP_1           1884            /* 1/exp(5sec/1min) as fixed-point */
#define EXP_5           2014            /* 1/exp(5sec/5min) */
#define EXP_15          2037            /* 1/exp(5sec/15min) */

#define CALC_LOAD(load,exp,n) \
        load *= exp; \
        load += n*(FIXED_1-exp); \
        load >>= FSHIFT;



the CXX compiler identification is unknown

也就是 CMake 找不到 C++ 对应的编译器，在 CentOS 可以通过 `yum install gcc-c++` 来安装。









/post/git-tips.html

----- 对一些修改增加着色
git config --global color.ui true


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#define THREAD_NAME_MAX 16

//#define HAVE_SETNAME_NP 1

static void *threadfunc(void __attribute__((unused)) *args)
{
#ifndef HAVE_SETNAME_NP
        if (prctl(PR_SET_NAME, "FOOOOO") < 0)
                fprintf(stderr, "set thread name failed, %s.\n", strerror(errno));
#endif
        sleep(60);

        return NULL;
}

int main(void)
{
        int rc;
        pthread_t thread;

        rc = pthread_create(&thread, NULL, threadfunc, NULL);
        if (rc != 0) {
                fprintf(stderr, "create thread failed, %s.\n", strerror(rc));
                return -1;
        }

#ifdef HAVE_SETNAME_NP
        char thdname[THREAD_NAME_MAX];

        rc = pthread_getname_np(thread, thdname, sizeof(thdname));
        if (rc != 0)
                fprintf(stderr, "get thread_np name failed, %s.\n", strerror(rc));
        fprintf(stdout, "current thread name is '%s'.\n", thdname);

        strncpy(thdname, "FOOBAR", sizeof(thdname) - 1);
        rc = pthread_setname_np(thread, thdname);
        if (rc != 0)
                fprintf(stderr, "set thread_np name failed, %s.\n", strerror(rc));
#endif

        rc = pthread_join(thread, NULL);
        if (rc != 0) {
                fprintf(stderr, "join thread failed, %s.\n", strerror(rc));
                return -1;
        }
}

## SendFile

sendfile:Linux中的"零拷贝"
https://blog.csdn.net/caianye/article/details/7576198
http://fred-zone.blogspot.com/2011/03/linux-kernel-sendfile-server.html


一般来说通过 SendFile 优化之后至少会有 2 倍的效率提升。

kHTTPd 一个内核中的HTTP服务器
http://www.fenrus.demon.nl/

Life of a HTTP request, as seen by my toy web server 极限优化WEB服务器的性能
https://tia.mat.br/posts/2014/10/06/life_of_a_http_request.html
https://jvns.ca/blog/2016/01/23/sendfile-a-new-to-me-system-call/
https://blog.plenz.com/2014-04/so-you-want-to-write-to-a-file-real-fast.html

可以通过 `man 2 sendfile` 查看相关的帮助文档。

#include <sys/sendfile.h>
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

其中 in_fd 必须是一个支持 mmap 的文件描述符，而 out_fd 在 2.6.33 之前只支持 socket ，在此之后支持任意的文件描述符。

        size_t count = 0;
        int filefd;
        struct stat filestat;

        filefd = open(argv[3], O_RDONLY);
        fstat(filefd, &filestat);

        char buff[1024];
        int sock, connfd;
        int rc;
        struct sockaddr_in address, cliaddr;
        socklen_t addrlen = sizeof(cliaddr);

        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        inet_pton(AF_INET, argv[1], &address.sin_addr);
        address.sin_port = htons(atoi(argv[2]));

        sock = socket(PF_INET, SOCK_STREAM, 0);
        bind(sock, (struct sockaddr *)&address, sizeof(address));
        listen(sock, 5);

        connfd = accept(sock, (struct sockaddr *)&cliaddr, &addrlen);
        while (1) {
                rc = read(filefd, buff, sizeof(buff));
                count += rc;
                write(connfd, buff, rc);
        }

        close(connfd);
        close(sock);

https://blog.csdn.net/hnlyyk/article/details/50856268



灰度发布
全链路压测

在 Nginx 中，通过 sticky 模块根据 Cookie 中的 mode 字段进行转发，包括了：A) 空 (真正业务流量)；B) gray 灰度流量，用来发布测试；C) shadow 影子流量。

进行全链路压测时，实际上流量应该与真正用户相同，只是使用的是影子表，所以在 Nginx 做负载均衡时，实际上也可以将 mode 置空，那么此时就不会转发到不同的服务器。

https://www.kancloud.cn/kancloud/xorm-manual-zh-cn/56003

# Casbin

Casbin 一个支持 Access Control List, ACL RBAC ABAC 的开源方案。

PERM stands for Policy, Effect, Request, Matchers.

用户 (User, Subject) 发起操作的主体。
对象 (Object) 操作所针对的目标，例如订单、文件等。
操作 (Action) 执行的动作，例如创建用户、文章等。

## ACL

全称为 Access Control List 。

最早也是最基本的一种访问控制机制，原理非常简单：每项资源都配有一个列表，这个列表记录的就是哪些用户可以对这项资源执行 CRUD 中的那些操作。

当访问某项资源时，会先检查这个列表中是否有关于当前用户的访问权限，从而确定当前用户可否执行相应的操作。总得来说，ACL 是一种面向资源的访问控制模型，它的机制是围绕资源展开的。

ACL 简单，但缺点也是很明显。由于需要维护大量的访问权限列表，ACL 在性能上有明显的缺陷。另外，对于拥有大量用户与众多资源的应用，管理访问控制列表本身就变成非常繁重的工作。


自主访问控制 Discretionary Access Control, DAC

当获得授权之后，可以将相关的权限转移给他人使用。

强制访问控制 Mandatory Access Control, MAC

## RBAC

也就是基于角色的访问控制，其全称为 Role-Based Access Control 。简单来说，就是每个用户关联一个或多个角色，每个角色关联一个或多个权限，从而可以实现了非常灵活的权限管理。

## ABAC

基于属性的权限验证，全称为 Attribute-Based Access Control ，被认为是权限系统设计的未来。

通过动态计算一个或一组属性来是否满足某种条件来进行授权判断，属性通常来说分为四类：用户属性 (如用户年龄)、环境属性 (如当前时间)、操作属性 (如读取) 和对象属性 (如一篇文章)，理论上能够实现非常灵活的权限控制，几乎能满足所有类型的需求。

不过 ABAC 的管理太过复杂，实际使用较多的还是 RBAC 。

https://dinolai.com/notes/others/authorization-models-acl-dac-mac-rbac-abac.html
https://callistaenterprise.se/blogg/teknik/2017/09/11/go-blog-series-part11/
https://marcus.se.net/hystrix-go-intro/
https://github.com/callistaenterprise/goblog
http://www.ru-rocker.com/2017/04/24/micro-services-using-go-kit-hystrix-circuit-breaker/
https://dzone.com/articles/go-microservices-part-11-hystrix-and-resilience
https://github.com/tcnksm-sample/hystrix-go


## N 的最小 2 次幂

其英文解释为 `Smallest power of 2 greater than or equal to N.` 。

## Method 1

通过纯数学的计算方法。

1. 计算 N 底为 2 的对数，并取其 ceil 值。
2. 再计算其指数。

## Method 2

获取数字中的最高位信息。

1. 判断是否已经是 0 或者 2 的 N 次幂。
2. 向右移位并获取最高位的值，同时需要统计移位次数。
3. 按照移位次数向左移动。

## Method 3

与方法 2 类似，不需要再统计位移的次数。

https://jameshfisher.com/2018/03/30/round-up-power-2.html
https://jeffreystedfast.blogspot.com/2008/06/calculating-nearest-power-of-2.html

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

unsigned int next_pow2_1(unsigned int v)
{
        return pow(2.0, ceil((log(v) / log(2.0)) + 0.5));
}

unsigned int next_pow2_2(unsigned int v)
{
        int c = 0;

        if (v && !(v & (v - 1)))
                return v;

        for (c = 0; v; c++)
                v >>= 1;

        return 1 << c;
}

unsigned int next_pow2_3(unsigned int v)
{
        unsigned int p = 1;

        if (v && !(v & (v - 1)))
                return v;

        while (p < v)
                p <<= 1;

        return p;
}

int main(void)
{
        printf("xxxxx %d\n", next_pow2_1(5));

        return 0;
}



applyAll()

## Snapshot

设置


applyAll()
 |-triggerSnapshot() 通过apply的次数判断是否需要执行Snapshot

在执行备份的时候，可以通过如下命令主动生成 snapshot 。


ENDPOINTS='127.0.0.1:15379,127.0.0.1:25379,127.0.0.1:35379'
ETCDCTL_API=3 ./etcdctl --endpoints=${ENDPOINTS} snapshot save snap.db


## 测试

为了方便进行测试，在启动时添加 `--snapshot-count '100'` 参数，并通过如下脚本生成测试用的数据。

#!/bin/sh -e

ENDPOINTS='127.0.0.1:15379,127.0.0.1:25379,127.0.0.1:35379'
for ((i=1; i<=150; i ++)); do
        uuid=`uuidgen`
        echo "ETCDCTL_API=3 ./etcdctl --endpoints=${ENDPOINTS} put ${uuid} '${uuid}-hello'"
done

## Progress

Leader 会通过 Progress 维护各个 Follower 的状态，会根据该状态向 Follower 发送日志信息(msgApp)。

对应的实现在 `raft/progress.go` 中，其中包括了两个重要的属性：

* match


ins *inflights 发送数据的滑动窗口，最大值为MaxSizePerMsg

becomeProbe

resetState()

https://github.com/metametaclass/libev-aliasing-warning/blob/master/test_alias.c




Protobuf 如果连续发包会导致粘包。

首先会判断是否为新的接口，也就是 `XXX_Unmarshal()` 的定义。

pb.Reset() 重置报文

SA_RESTART
http://www.cnblogs.com/mickole/p/3191832.html
commitC 的数据从何而来？

通过 `publishEntries()[raft.go]` 将已经提交的数据添加到 commitC 管道中，而已经提交数据是从 Ready() 中获取。

初步判断，在 publishEntries() 函数中会添加

注意这里的处理方式有问题？

`pthread_setname_np()` 函数是在 glibc 的 2.12 版本之后添加的，可以查看其源码实际上如果是本线程也是通过 `prctl()` 添加，而非本线程则实际修改的 `/proc/self/task/%u/comm` 文件的内容。




在 C 中，可以通过 `goto` 跳转到同一个函数的某个 label 处，但是不能在不同的函数之间跳转。实际上，C 另外提供了 `setjmp()` 和 `longjmp()` 来完成这种类型的分支跳转。

实现这种类型的跳转，有点类似于操作系统中任务的上下文切换，这里只需要恢复 label 标签所处函数的上下文即可，一般函数上下文包括：

* 函数栈帧，主要是栈帧指针 BP 和栈顶指针 SP；
* 程序指针 PC，也就是修改为指向 label 语句处的地址；
* 其它寄存器，这和 CPU 的体系相关，例如在 x86 体系下需要保存 `AX` `BX` `CX` 等等。

在执行跳转语句时，直接恢复 label 处的上下文，即完成跳转到 label 处的功能。

在 C 语言中 `setjmp()` 和 `longjmp()` 就提供了完成保存上下文和切换上下文的工作。

http://www.cnblogs.com/hazir/p/c_setjmp_longjmp.html
gcc 关键字 __thread
https://blog.csdn.net/liuxuejiang158blog/article/details/14100897







类似于top命令，但是监控的是进程的网络带宽
https://github.com/raboof/nethogs
https://zhoujianshi.github.io/articles/2017/Linux%20%E8%8E%B7%E5%8F%96TCP%E8%BF%9E%E6%8E%A5%E4%B8%8Epid%E7%9A%84%E6%98%A0%E5%B0%84%EF%BC%8C%E5%8F%8A%E7%9B%91%E6%B5%8B%E8%BF%9B%E7%A8%8B%E7%9A%84TCP%E6%B5%81%E9%87%8F/index.html

其中 `/proc/net/dev` 包含了网络设备的相关统计，而 `/proc/<pid>/net/dev` 包含了进程相关的统计信息。注意，如果进程添加到了网络的 namespace 中 (man ip-netns) ，那么进程中的文件就只有指定的网络设备。


关于CacheLine的介绍
http://cenalulu.github.io/linux/all-about-cpu-cache/


https://blog.csdn.net/muxiqingyang/article/details/6615199
http://www.pandan.xyz/2016/09/23/mesi%20%E7%BC%93%E5%AD%98%E4%B8%80%E8%87%B4%E6%80%A7%E5%8D%8F%E8%AE%AE/

### False Sharing
http://blog.yufeng.info/archives/783

## 参考

详细可以查看 Intel 的文档 [Avoiding and Identifying False Sharing Among Threads](https://software.intel.com/en-us/articles/avoiding-and-identifying-false-sharing-among-threads/) 中相关介绍，或者本地文档。


虽然目前 Python 在 AI 领域使用的越来越多，不过其在 DevOps、安全领域使用的还比较多，例如可以通过 Python 脚本来简化日常的运维工作。

在 Linux 中可以通过多个命令查看当前系统的信息，例如 `ps`、`top`、`free` 等等，如果这样就需要通过类似 `subprocess` 的模块进行调用，并解析其返回的结果。

实际上，存在一个 psutil (process and system utilities) 可用来获取当前系统的信息，支持 Linux Unix OSX Windows 等多个平台。

这里简单介绍，并提供一个 C 库。

## psutil

如果 pip 源配置好之后，可以直接通过 `pip install psutil` 命令安装，提供的接口详细可以查看 [GitHub psutil](https://github.com/giampaolo/psutil/) 中的介绍。

## libsysx

主要是参考 Python 中的 [psutil](https://github.com/giampaolo/psutil/) 包使用以及实现方式，提供一个 C 实现的 API 接口，实现一个信号安全、可重入的库。


Linux安全审计
https://cisofy.com/downloads/lynis/
https://dacat.cc/1984.html



1. Cron
2. PS
3. lsof
4. Pipe

Jaeger源码相关的解析
https://github.com/jukylin/blog

Interface Description Language, IDL 接口描述语言


PSUtils 支持 Windows Mac Linux 等
https://www.jianshu.com/p/64e265f663f6



Keeping Redundancy Effective, KRE 保持冗余有效性

源于多年的运维实践，因为每次回溯故障时，都会痛心地发现几乎所有的故障都是可以避免的，只要我们的多道防线中的任何一道发挥阻拦效用，这起故障就不可能引起服务中断。

假设每一道防护的失效概率是0.01%，同时失效的概率就降到了0.0001%。如果三层冗余，同时失效率就是0.000001%，几乎就是百年不遇的了。

1. 这个系统有没有采用两道以上的冗余保护？
2. 是否有机制确保每一道保护在任何时候都是有效的，一旦失效能够及时发现（自动检测、管理手段等）？
3. 是否有方法和工具用最短的时间恢复？





glibc 提供了内置的位运算符。

int __builtin_ffs (unsigned int x)
返回x的最后一位1的是从后向前第几位，比如7368（1110011001000）返回4。

//----- 前导的0的个数
int __builtin_clz(unsigned int x);


int __builtin_ctz (unsigned int x)
返回后面的0个个数，和__builtin_clz相对。
int __builtin_popcount (unsigned int x)
返回二进制表示中1的个数。
int __builtin_parity (unsigned int x)
返回x的奇偶校验位，也就是x的1的个数模2的结果。

此外，这些函数都有相应的usigned long和usigned long long版本，只需要在函数名后面加上l或ll就可以了，比如int __builtin_clzll。















https://www.zfl9.com/c-regex-pcre.html

其中 `regex.h` 是一个正则表达式实现的头文件，提供的常用函数有 `regcomp()`、`regexec()`、`regfree()` 和 `regerror()` 。

注意，`regexec()` 函数在一次匹配成功之后就会返回，例如对于 IP 地址 `192.168.9.100` 来说，如果使用 `[0-9]+` 进行匹配会返回 `192` ，如果要匹配所有的数值，则需要使用 `([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)` 。

当使用最后的匹配时，返回的第一条是整个完整的匹配字符串，然后是返回的匹配子串，也就是依次返回 `192.168.9.100` `192` `168` `9` `100` 五个字符串。

/var/log/message 常见异常

\<segfault at\> 段错误
\<Out of memory:\> OOM

#include <stdio.h>
#include <string.h>
#include <regex.h>

#define MATCH_ITEMS_MAX    32

int main(void)
{
        int rc, i;
        regex_t regex;
        regmatch_t matches[MATCH_ITEMS_MAX];

        //const char *pattern = "([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)";
        //char buff[] = "192.168.9.100";

        //const char *pattern = "([0-9]+)";
        //char buff[] = "192.168.9.100";

        const char *pattern = "\\<size=([0-9]+)B\\>";
        char buff[] = "Got package size=100B.";

        rc = regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
        if (rc != 0) {
                fprintf(stderr, "compile pattern '%s' failed, rc %d.\n", pattern, rc);
                return -1;
        }

        rc = regexec(&regex, buff, MATCH_ITEMS_MAX, matches, 0);
        if (rc == REG_NOMATCH) {
                fprintf(stderr, "no match, pattern '%s' string '%s'.\n", pattern, buff);
                regfree(&regex);
                return -1;
        }

        //assert(rc == 0);
        for (i = 0; i < MATCH_ITEMS_MAX; i++) {
                if (matches[i].rm_eo < 0 || matches[i].rm_so < 0)
                        break;
                fprintf(stderr, "got match string '%.*s'.\n", matches[i].rm_eo - matches[i].rm_so,
                        buff + matches[i].rm_so);
        }

        regfree(&regex);

        return 0;
}

https://segmentfault.com/a/1190000008125359
https://www.cnblogs.com/charlieroro/p/10180827.html
https://github.com/digoal/blog/blob/master/201701/20170111_02.md

/post/golang-syntax-interface-introduce.html

接口继承

一个接口可以继承多个其它接口，如果要实现这个接口，那么就必须要其所继承接口中的方法都实现。

type Saying interface {
        Hi() error
}

type Notifier interface {
        Saying
        Notify() error
}

func (u *User) Hi() error {
        log.Printf("Hi %s\n", u.Name)
        return nil
}

上述报错的大致意思是说 -->

这里的关键是 User 的 Notify() 方法实现的是 Pointer Receiver ，而实际需要的是 Value Receiver 。


在官方文档 [golang.org/doc](https://golang.org/doc/effective_go.html#pointers_vs_values) 中有相关的介绍。

> The rule about pointers vs. values for receivers is that value methods can be invoked on pointers and values, but pointer methods can only be invoked on pointers.
>
> This rule arises because pointer methods can modify the receiver; invoking them on a value would cause the method to receive a copy of the value, so any modifications would be discarded. The language therefore disallows this mistake.

通过 Pointer Method 可以直接修改对象的值，而 Value Method 在执行前会复制一份对应的对象，并在复制后的对象上执行相关操作，而不会修改原对象的值。

原则上来说，这样定义也正常，但是很容易误用而且很难发现，所以 GoLang 放弃了这一特性。


## 使用

接口对于 GoLang 来说关键是其实现了泛型，类似于 C++ 中的多态特性，对于函数可以根据不同类型的入参生成不同的对象。

注意，GoLang 是静态编程语言，会在编译过程中检查对应的类型，包括了函数、变量等，同时又有一定的灵活性。实际上处于纯动态语言 (例如 Python) 以及静态语言之间 (例如 C)，可以在一定程度上进行语法检查，同时又提供了高阶功能。

通过 Go 的接口，可以使用 Duck Typing 方式编程。

Duck typing in computer programming is an application of the duck test—"If it walks like a duck and it quacks like a duck, then it must be a duck"—to determine if an object can be used for a particular purpose.  With normal typing, suitability is determined by an object's type. In duck typing, an object's suitability is determined by the presence of certain methods and properties, rather than the type of the object itself.

### 标准库

比较典型的示例可以参考 `io/io.go` 中的读写接口。

type Reader interface {
        Read(p []byte) (n int, err error)
}

type Writer interface {
        Write(p []byte) (n int, err error)
}

很多的标准库会使用这一接口，包括了网络、编码等类型，这里简单介绍 `encoding/binary` 的使用，其中 `Read()` 函数的声明为。

func Read(r io.Reader, order ByteOrder, data interface{}) error

其中的 `r` 参数可以是任意一个支持 `type Reader interface` 的实现，例如，使用示例如下。

package main

import (
        "bytes"
        "encoding/binary"
        "log"
)

func main() {
        var pi float64

        buff := bytes.NewBuffer([]byte{0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40})
        if err := binary.Read(buff, binary.LittleEndian, &pi); err != nil {
                log.Fatalln("binary.Read failed:", err)
        }
        log.Println(pi)
}

如上，从新建的一个内存缓存中读取，并格式化，也可以是文件或者网络。也就是说，只要支持 `Read()` 函数即可 (包括入参等，一般称为签名 Signature ) ，对于 Python 来说编译阶段就会报错。

## 源码解析

在 `runtime/runtime2.go` 文件中定义了 `type iface struct` 以及 `type eface struct` 两个结构体。

type iface struct {
        tab  *itab
        data unsafe.Pointer
}

type eface struct {
        _type *_type
        data  unsafe.Pointer
}

分别表示包含方法以及不包含方法的接口。

// iface 含方法的接口
type Person interface {
	Print()
}

// eface 不含方法的接口
type Person interface {}
var person interface{} = xxxx实体

https://segmentfault.com/a/1190000017389782

## eface

由两个属性组成：`_type` 类型信息；`data` 数据信息。

type eface struct {
        _type *_type
        data  unsafe.Pointer
}

其中 `_type` 是所有类型的公共描述，几乎所有的数据都可以抽象成 `_type` 。

## iface

type iface struct {
        tab  *itab
        data unsafe.Pointer
}

https://draveness.me/golang/docs/part2-foundation/ch04-basic/golang-reflect/

SetDeadline
SetReadDeadline
SetWriteDeadline

在 GoLang 提供的 net.Conn 结构中，提供了 Deadline 方法，包括了

其中 Deadline是一个绝对时间值，当到达这个时间的时候，所有的 I/O 操作都会失败，返回超时(timeout)错误。

https://colobu.com/2016/07/01/the-complete-guide-to-golang-net-http-timeouts/


Answer to the Ultimate Question of Life, The Universe, and Everything. 42


## Reference

* [miniz](https://github.com/richgel999/miniz) Single C source file zlib-replacement library.




malloc产生SEGV问题排查方法
https://blog.csdn.net/win_lin/article/details/7822762

https://eklitzke.org/memory-protection-and-aslr

TIME_WAIT和端口复用
https://www.cnblogs.com/kex1n/p/7437290.html
https://blog.csdn.net/u010585120/article/details/80826999

内存数据提取
https://github.com/rek7/mXtract
https://github.com/hephaest0s/usbkill

查找敏感信息
https://www.freebuf.com/articles/system/23993.html
c++ pitfall


## 指针

### 数组指针

`int (*arr)[3]` 这定义了一个指向数组的指针，数组的元素必须是 3 。

#include <stdio.h>

int main(void)
{
        int (*ptr)[3], i, *data;
        int array[3] = {1, 2, 3}; // size MUSTBE 3.

        ptr = &array; // ptr is a pointer to array.
        for (i = 0; i < 3; i++)
                printf("%d\n", (*ptr)[i]); // got the array first

        data = array;
        for (i = 0; i < 3; i++)
                printf("%d\n", data[i]);

        return 0;
}

如上是容易出错的三个点：

1. 数组的大小必须与声明的数组指针变量大小相同；
2. 因为ptr是一个数组指针，所以必须对数组取地址；
3. 由于ptr是数组指针，那么在获取数组中的元素时，需要先取地址，而且要加括号保证优先级。

后面是比较常用的使用方法，如果要传递给一个函数，那么数组的大小同样需要传递。







https://stackoverflow.com/questions/11167907/compression-in-openssl
https://blog.csdn.net/liujiayu2/article/details/51860184

SSH-Key的选择
https://medium.com/@honglong/%E9%81%B8%E6%93%87-ssh-key-%E7%9A%84%E5%8A%A0%E5%AF%86%E6%BC%94%E7%AE%97%E6%B3%95-70ca45c94d8e

很多不错的网络开发介绍
http://www.52im.net/thread-50-1-1.html

## 文件格式

假设下载的是一个 [CentOS 8](http://mirrors.163.com/centos/8/isos/x86_64/) 的镜像，可以直接下载。

协议简介，官方以及非官方
https://wiki.theory.org/index.php/Main_Page
http://bittorrent.org/beps/bep_0003.html

https://github.com/skeeto/bencode-c
https://github.com/amwales-888/ambencode
https://github.com/janneku/bencode-tools
https://github.com/willemt/heapless-bencode
https://github.com/somemetricprefix/tbl
https://segmentfault.com/a/1190000000681331
https://github.com/Rudde/mktorrent

其中比较关键的是 `announce` URL 以及 `info` 字典，

MP3格式解析
https://github.com/lieff/minimp3
https://blog.csdn.net/u010650845/article/details/53520426
https://www.cnblogs.com/ranson7zop/p/7655474.html

GO客户端
https://github.com/anacrolix/torrent
Tracker
https://github.com/chihaya/chihaya
https://github.com/masroore/opentracker
https://github.com/xaiki/opentracker
https://github.com/danielfm/bttracker
https://github.com/willemt/tracker-client
http://erdgeist.org/arts/software/opentracker/
https://github.com/crosbymichael/tracker

http://www.kristenwidman.com/blog/33/how-to-write-a-bittorrent-client-part-1/
https://www.cnblogs.com/hnrainll/archive/2011/07/26/2117423.html

https://blog.jse.li/posts/torrent/
https://www.jianshu.com/p/22205fa24c9b
https://skerritt.blog/bit-torrent/
µTorrent Vuze Deluge Transmission

DFS非stack模式
https://segmentfault.com/a/1190000010632749
安全编译选项
https://firmianay.gitbooks.io/ctf-all-in-one/doc/4.4_gcc_sec.html
https://blog.lao-yuan.com/2018/06/09/Linux-GCC%E5%AE%89%E5%85%A8%E4%BF%9D%E6%8A%A4%E6%9C%BA%E5%88%B6.html
https://blog.lao-yuan.com/2018/05/29/Linux%E4%B8%8B%E5%A0%86%E6%A0%88%E7%BB%93%E6%9E%84%E5%88%86%E6%9E%90.html

* 不会存在环，即使存在不能存在总和为负值的环；
* 对于有 V 的节点的图，最多经过 V - 1 个边，此时退化成了链表；
* 最短路径上的较小段 (subpath) 也是最短路径。


/post/program-c-gcc-security-options.html
VSDO随机化
https://zhuanlan.zhihu.com/p/58419878


为了方便调试，GDB 会自动关闭随机选项，可以通过 `set disable-randomization off` 打开该选项。

https://blog.csdn.net/Plus_RE/article/details/79199772
https://yifengyou.gitbooks.io/learn-linux_exploit/

反ptrace
http://eternalsakura13.com/2018/02/01/ptrace/
/proc/<PID>/environ

/post/kernel-memory-virtual-physical-map
/post/kernel-memory-management-from-userspace-view


文件 `/proc/<PID>/maps` 显示了进程映射的内存区域和访问权限，通过 `proc_pid_maps_op` 实现，对应的函数为 `show_map()` ，对应内核中的 `task->mm->mmap` 链表。

https://blog.csdn.net/lijzheng/article/details/23618365

/post/charsets-encoding.html
https://upload.wikimedia.org/wikipedia/commons/d/dd/ASCII-Table.svg

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <sys/ptrace.h>

int strsplit(char *string, char **fields, size_t size)
{
        size_t i = 0;
        char *ptr = string, *saveptr = NULL;

        while ((fields[i] = strtok_r(ptr, ", \t\r\n", &saveptr)) != NULL) {
                ptr = NULL;
                i++;

                if (i >= size)
                        break;
        }

        return ((int)i);
}

static void *read_data_range(int pid, void *start, void *end)
{
        long word;
        void *data;
        size_t len, offset;

        len = end - start;
        if ((len % sizeof(void *)) != 0) {
                fprintf(stderr, "malformed memory address, length %d.", len);
                return NULL;
        }
        if (len > 1024 * 1024)
                return NULL;
        //fprintf(stdout, "read data from %p to %p, length %ld.\n", start, end, len);

        data = malloc(len);
        if (data == NULL) {
                fprintf(stderr, "malformed memory address, length %d.", len);
                return NULL;
        }

        errno = 0;
        for (offset = 0; offset < len; offset += sizeof(long)) {
                word = ptrace(PTRACE_PEEKTEXT, pid, start + offset, NULL);
                if (word < 0 && errno != 0) {
                        fprintf(stderr, "peek text from %p failed, %d:%s.",
                                start + offset, errno, strerror(errno));
                        free(data);
                        return NULL;
                }
                memcpy((uint8_t *)data + offset, &word, sizeof(word));
        }

        return data;
}
int main(void)
{
        FILE *maps;
        int pid = 27898, rc, idx, len, i;
        char path[128], line[1024], *fields[32], *end, *ptr, *data;
        unsigned long long addr_start, addr_end;

        if (geteuid() != 0) {
                fprintf(stdout, "Running as root is recommended.");
                return -1;
        }

        rc = snprintf(path, sizeof(path), "/proc/%d/maps", pid);
        if (rc < 0 || rc >= (int)sizeof(path)) {
                fprintf(stderr, "format maps filepath failed, rc %d.", rc);
                return -1;
        }

        maps = fopen(path, "r");
        if (maps == NULL) {
                fprintf(stderr, "open map file '%s' failed, %d:%s.", path, errno, strerror(errno));
                return -1;
        }

        // /proc/<PID>/environ
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
                fprintf(stderr, "Attach to PID %d failed, %d:%s.", pid, errno, strerror(errno));
                fclose(maps);
                return -1;
        }
        wait(NULL);

        while (feof(maps) == 0) {
                if (fgets(line, sizeof(line), maps) == NULL)
                        break;

                rc = strsplit(line, fields, (sizeof(fields)/sizeof(fields[0])));
                if (rc < 2) {
                        fprintf(stderr, "invalid line '%s', at least 2 fields expect.", line);
                        break;
                }

                if (strchr(fields[1], 'r') == 0)
                        continue;
                fprintf(stderr, "======= %s\n", line);

                end = strchr(fields[0], '-');
                if (end == NULL)
                        continue;
                *end = 0;
                end++;

                errno = 0;
                addr_start = strtoull(line, &ptr, 16);
                if (line == ptr || errno != 0) {
                        fprintf(stderr, "convert start address '%s' failed, %d:%s.\n",
                                        line, errno, strerror(errno));
                        continue;
                }
                addr_end = strtoull(end, &ptr, 16);
                if (end == ptr || errno != 0) {
                        fprintf(stderr, "convert end address '%s' failed, %d:%s.\n",
                                        end, errno, strerror(errno));
                        continue;
                }

                data = read_data_range(pid, (void *)addr_start, (void *)addr_end);
                if (data == NULL)
                        continue;

                len = addr_end - addr_start;
                for (idx = 0, i = 0; i < len; i++) {
                        if (data[i] < ' ' || data[i] > '~')
                                continue;
                        data[idx++] = data[i];
                }
                data[idx] = 0;
                fprintf(stdout, "got data: %s\n", data);

                free(data);
                //fprintf(stderr, "%p %p 0x%llx   0x%llx\n", line, ptr, addr_start, addr_end);
        }

        fclose(maps);

        if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
                fprintf(stderr, "Attach to PID %d failed, %d:%s.", pid, errno, strerror(errno));
                return -1;
        }

        return 0;
}

## 内存保护

简单来说，就是针对不同的场景设置内存的读写权限。

https://www.gnu.org/software/libc/manual/html_node/Memory-Protection.html
https://www.informit.com/articles/article.aspx?p=23618&seqNum=10
https://www.cnblogs.com/rim99/p/5523289.html
https://unix.stackexchange.com/questions/211951/how-does-the-kernel-prevent-a-malicious-program-from-reading-all-of-physical-ram

ASLR实现以及漏洞分析
https://www.cnblogs.com/wangaohui/p/7122653.html
https://www.freebuf.com/articles/system/228731.html

## 参考

* [Linux Kernel Memory Protection](http://ijcsit.com/docs/Volume%205/vol5issue04/ijcsit20140504225.pdf)

检测在什么样的虚拟机里的脚本
https://www.freebuf.com/articles/network/229040.html


Non-Deterministic Polynomial Complete Problem, NPC 问题
Non-Deterministic Polynomially, NP 是指一个问题不能确定是否在多项式时间内找到答案，但是可以在多项式时间内验证答案是否正确，如完全子图问题、图着色问题、旅行商(TSP)问题等。

之所以要定义 NP 问题，是因为通常只有 NP 问题才可能找到多项式的算法，不能指望一个连多项式验证一个解都不行的问题存在一个解决它的多项式级的算法。


http://halobates.de/memorywaste.pdf
-->


{% highlight text %}
{% endhighlight %}
