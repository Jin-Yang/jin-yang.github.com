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

{% highlight text %}
{% endhighlight %}
