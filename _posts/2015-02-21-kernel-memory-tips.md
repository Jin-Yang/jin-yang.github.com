---
title: Kernel 内存杂项
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,内存,kernel,overcommit,dentry cache,buffer,cache
description: 简单介绍下内核中与内存相关的内容。
---

简单介绍下内核中与内存相关的内容。

<!-- more -->

## OverCommit

内核中内存的 OverCommit 就是操作系统承诺给所有进程的内存大小超过了实际可用的内存，主要是由于物理内存页的分配发生在使用时，而非申请时，也就是所谓的 COW 机制。

例如，程序通过 ```malloc()``` 申请了 200MB 内存，不过实际只使用了 100MB 的内存，也就是说，只要你来申请内存我就给你，寄希望于进程实际上用不到那么多内存。

如果申请了内存之后，确实会有大量进程使用内存，那么就会发生类似银行的 "挤兑"，此时就会使用 Linux 的 OOM killer 机制。

<!--
(OOM = out-of-memory)来处理这种危机：挑选一个进程出来杀死，以腾出部分内存，如果还不够就继续杀…也可通过设置内核参数 vm.panic_on_oom 使得发生OOM时自动重启系统。这都是有风险的机制，重启有可能造成业务中断，杀死进程也有可能导致业务中断，我自己的这个小网站就碰到过这种问题，参见前文。所以Linux 2.6之后允许通过内核参数 vm.overcommit_memory 禁止memory overcommit。
-->

其行为可以通过 ```vm.overcommit_memory``` 参数修改，包括了三种取值：

* 0 (Heuristic Overcommit Handling)，默认值<br>允许 OC ，不过如果申请的内存过大，那么系统会拒绝请求，而判断的算法在后面介绍。
* 1 (Always Overcommit)<br>允许 OC，对内存申请来者不拒。
* 2 (Don't Overcommit)<br>禁止 OC 。

当上述的值为 2 时，会禁止 OC，那么怎么才算是 OC 呢？实际上，Kernel 设有一个阈值，申请的内存总数超过这个阈值就算 OC，阀值可以通过 ```/proc/meminfo``` 文件查看：

{% highlight text %}
# grep -i commit /proc/meminfo
CommitLimit:    12162784 kB
Committed_AS:    6810268 kB
{% endhighlight %}

其中 ```CommitLimit``` 就是对应的阈值，超过了该值就算 Overcommit ，该值通过 ```vm.overcommit_ratio``` 或 ```vm.overcommit_kbytes``` 间接设置的，公式如下：

{% highlight text %}
CommitLimit = (Physical RAM * vm.overcommit_ratio / 100) + Swap
{% endhighlight %}

其中 ```vm.overcommit_ratio``` 的默认值是 50，也就是表示物理内存的 50% ，当然，也可以直接通过 ```vm.overcommit_kbytes``` 即可。注意，如果 huge pages 那么就需要从物理内存中减去，公式变成：

{% highlight text %}
CommitLimit = ([total RAM] – [total huge TLB RAM]) * vm.overcommit_ratio / 100 + swap
{% endhighlight %}

另外，```/proc/meminfo``` 中的 ```Committed_AS``` 表示所有进程已经申请的内存总大小；注意，是已经申请的，不是已经分配的。如果 ```Committed_AS``` 超过 ```CommitLimit``` 就表示发生了 OC 。


<!--
“sar -r”是查看内存使用状况的常用工具，它的输出结果中有两个与overcommit有关，kbcommit 和 %commit：
kbcommit对应/proc/meminfo中的 Committed_AS；
%commit的计算公式并没有采用 CommitLimit作分母，而是Committed_AS/(MemTotal+SwapTotal)，意思是_内存申请_占_物理内存与交换区之和_的百分比。
$ sar -r
05:00:01 PM kbmemfree kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
05:10:01 PM    160576   3648460     95.78         0   1846212   4939368     62.74   1390292   1854880         4

附：对Heuristic overcommit算法的解释

内核参数 vm.overcommit_memory 的值0，1，2对应的源代码如下，其中heuristic overcommit对应的是OVERCOMMIT_GUESS：

源文件：source/include/linux/mman.h

#define OVERCOMMIT_GUESS                0
#define OVERCOMMIT_ALWAYS               1
#define OVERCOMMIT_NEVER                2

Heuristic overcommit算法在以下函数中实现，基本上可以这么理解：
单次申请的内存大小不能超过 【free memory + free swap + pagecache的大小 + SLAB中可回收的部分】，否则本次申请就会失败。
源文件：source/mm/mmap.c 以RHEL内核2.6.32-642为例

0120 /*
0121  * Check that a process has enough memory to allocate a new virtual
0122  * mapping. 0 means there is enough memory for the allocation to
0123  * succeed and -ENOMEM implies there is not.
0124  *
0125  * We currently support three overcommit policies, which are set via the
0126  * vm.overcommit_memory sysctl.  See Documentation/vm/overcommit-accounting
0127  *
0128  * Strict overcommit modes added 2002 Feb 26 by Alan Cox.
0129  * Additional code 2002 Jul 20 by Robert Love.
0130  *
0131  * cap_sys_admin is 1 if the process has admin privileges, 0 otherwise.
0132  *
0133  * Note this is a helper function intended to be used by LSMs which
0134  * wish to use this logic.
0135  */
0136 int __vm_enough_memory(struct mm_struct *mm, long pages, int cap_sys_admin)
0137 {
0138         unsigned long free, allowed;
0139
0140         vm_acct_memory(pages);
0141
0142         /*
0143          * Sometimes we want to use more memory than we have
0144          */
0145         if (sysctl_overcommit_memory == OVERCOMMIT_ALWAYS)
0146                 return 0;
0147
0148         if (sysctl_overcommit_memory == OVERCOMMIT_GUESS) { //Heuristic overcommit算法开始
0149                 unsigned long n;
0150
0151                 free = global_page_state(NR_FILE_PAGES); //pagecache汇总的页面数量
0152                 free += get_nr_swap_pages(); //free swap的页面数
0153
0154                 /*
0155                  * Any slabs which are created with the
0156                  * SLAB_RECLAIM_ACCOUNT flag claim to have contents
0157                  * which are reclaimable, under pressure.  The dentry
0158                  * cache and most inode caches should fall into this
0159                  */
0160                 free += global_page_state(NR_SLAB_RECLAIMABLE); //SLAB可回收的页面数
0161
0162                 /*
0163                  * Reserve some for root
0164                  */
0165                 if (!cap_sys_admin)
0166                         free -= sysctl_admin_reserve_kbytes >> (PAGE_SHIFT - 10); //给root用户保留的页面数
0167
0168                 if (free > pages)
0169                         return 0;
0170
0171                 /*
0172                  * nr_free_pages() is very expensive on large systems,
0173                  * only call if we're about to fail.
0174                  */
0175                 n = nr_free_pages(); //当前free memory页面数
0176
0177                 /*
0178                  * Leave reserved pages. The pages are not for anonymous pages.
0179                  */
0180                 if (n <= totalreserve_pages)
0181                         goto error;
0182                 else
0183                         n -= totalreserve_pages;
0184
0185                 /*
0186                  * Leave the last 3% for root
0187                  */
0188                 if (!cap_sys_admin)
0189                         n -= n / 32;
0190                 free += n;
0191
0192                 if (free > pages)
0193                         return 0;
0194
0195                 goto error;
0196         }
0197
0198         allowed = vm_commit_limit();
0199         /*
0200          * Reserve some for root
0201          */
0202         if (!cap_sys_admin)
0203                 allowed -= sysctl_admin_reserve_kbytes >> (PAGE_SHIFT - 10);
0204
0205         /* Don't let a single process grow too big:
0206            leave 3% of the size of this process for other processes */
0207         if (mm)
0208                 allowed -= mm->total_vm / 32;
0209
0210         if (percpu_counter_read_positive(&vm_committed_as) < allowed)
0211                 return 0;
0212 error:
0213         vm_unacct_memory(pages);
0214
0215         return -ENOMEM;
0216 }
-->

<!--
https://unix.stackexchange.com/questions/60474/atop-shows-red-line-vmcom-and-vmlim-what-does-it-mean
-->

## Dentry Cache

free 命令主要显示的用户的内存使用 (新版available可用)，包括使用 top 命令 (可以通过 Shift+M 内存排序)，对于 slab 实际上没有统计 (可以使用 slabtop 查看)，真实环境中，经常会出现 dentry(slab) 的内存消耗过大的情况。

dentry cache 是目录项高速缓存，记录了目录项到 inode 的映射关系，用于提高目录项对象的处理效率，当通过 stat() 查看文件信息、open() 打开文件时，都会创建 dentry cache 。

{% highlight text %}
----- 查看当前进程的系统调用
# strace -f -e trace=open,stat,close,unlink -p $(ps aux | grep 'cmd' | awk '{print $2}')
{% endhighlight %}

不过这里有个比较奇怪的现象，从 ```/proc/meminfo``` 可以看出，slab 中的内存分为 SReclaimable 和 SUnreclaim 两部分；而占比比较大的是 SReclaimable 的内存，也就是可以回收的内存，但是，当通过 slabtop 命令查看 dentry 对应的使用率时，基本处于 100% 的使用状态。

那么，到底是什么意思？这部分内存能不能回收？

首先，```/proc/meminfo``` 对应到源码中为 ```fs/proc/meminfo.c```，在内核中与这两个状态相关的代码在 ```mm/slub.c``` 中 (如果是新版本的内核)；简单来说就是对应到了 ```allocate_slab()``` 和 ```__free_slab()``` 函数中，实际上源码中直接搜索 ```NR_SLAB_RECLAIMABLE``` 即可。

对应到源码码中，实际只要申请 cache 时使用了 ```SLAB_RECLAIM_ACCOUNT``` 标示，那么在进行统计时，就会被标记为 SReclaimable 。

<!-- 而从 /proc/slabinfo 中读取对应源码 mm/slab_common.c 中的 cache_show() 函数。 -->

也就是说 meminfo 中的 SReclaimable 标识的是可以回收的 cache，但是真正的回收操作是由各个申请模块控制的，对于 dentry 来说，真正的回收操作还是 dentry 模块自己负责。

### 解决方法

对于上述的 slab 内存占用过多的场景，在 Linux 2.6.16 之后提供了 [drop caches](https://linux-mm.org/Drop_Caches) 机制，用于释放 page cache、inode/dentry caches ，方法如下。

{% highlight text %}
----- 刷新文件系统缓存
# sync
----- 释放Page Cache
# echo 1 > /proc/sys/vm/drop_caches
----- 释放dentries+inodes
# echo 2 > /proc/sys/vm/drop_caches
----- 释放pagecache, dentries, inodes
# echo 3 > /proc/sys/vm/drop_caches
----- 如果没有root权限，但是有sudo权限
$ sudo sysctl -w vm.drop_caches=3
{% endhighlight %}

注意，修改 drop_caches 的操作是无损的，不会释放脏页，所以需要在执行前执行 sync 以确保确实有可以释放的内存。

另外，除了上述的手动处理方式之外，还可以通过修改 ```/proc/sys/vm/vfs_cache_pressure``` 的参数，来调整自动清理 inode/dentry caches 的优先级，默认为 100 此时需要调大；关于该参数可以参考内核文档 [vm.txt]( {{ site.kernel_docs_url }}/Documentation/sysctl/vm.txt )，简单摘抄如下：

{% highlight text %}
This percentage value controls the tendency of the kernel to reclaim
the memory which is used for caching of directory and inode objects.

At the default value of vfs_cache_pressure=100 the kernel will attempt to
reclaim dentries and inodes at a "fair" rate with respect to pagecache and
swapcache reclaim.  Decreasing vfs_cache_pressure causes the kernel to prefer
to retain dentry and inode caches. When vfs_cache_pressure=0, the kernel will
never reclaim dentries and inodes due to memory pressure and this can easily
lead to out-of-memory conditions. Increasing vfs_cache_pressure beyond 100
causes the kernel to prefer to reclaim dentries and inodes.

Increasing vfs_cache_pressure significantly beyond 100 may have negative
performance impact. Reclaim code needs to take various locks to find freeable
directory and inode objects. With vfs_cache_pressure=1000, it will look for
ten times more freeable objects than there are.
{% endhighlight %}

也就是说，当该文件对应的值越大时，系统的回收操作的优先级也就越高，不过内核对该值好像不敏感，一般会设置为 10000 。

<!-- https://major.io/2008/12/03/reducing-inode-and-dentry-caches-to-keep-oom-killer-at-bay/ -->

<!--
### 自动回收 dcache

查了一些关于Linux dcache的相关资料，发现操作系统会在到了内存临界阈值后，触发kswapd内核进程工作才进行释放，这个阈值的计算方法如下：

1. 首先，grep low /proc/zoneinfo，得到如下结果：

        low      1
        low      380
        low      12067

2. 将以上3列加起来，乘以4KB，就是这个阈值，通过这个方法计算后发现当前服务器的回收阈值只有48MB，因此很难看到这一现象，实际中可能等不到回收，操作系统就会hang住没响应了。

3. 可以通过以下方法调大这个阈值：将vm.extra_free_kbytes设置为vm.min_free_kbytes和一样大，则/proc/zoneinfo中对应的low阈值就会增大一倍，同时high阈值也会随之增长，以此类推。

$ sudo sysctl -a | grep free_kbytes
vm.min_free_kbytes = 39847
vm.extra_free_kbytes = 0
$ sudo sysctl -w vm.extra_free_kbytes=836787  ######1GB


 4. 举个例子，当low阈值被设置为1GB的时候，当系统free的内存小于1GB时，观察到kswapd进程开始工作（进程状态从Sleeping变为Running），同时dcache开始被系统回收，直到系统free的内存介于low阈值和high阈值之间，停止回收。

vm.overcommit_ratio=2
vm.dirty_background_ratio=5
vm.dirty_ratio=20
extra_free_kbytes
-->



接下来，看看如何保留部分内存。

### min_free_kbytes

该参数在内核文档 [vm.txt]( {{ site.kernel_docs_url }}/Documentation/sysctl/vm.txt )，简单摘抄如下：

{% highlight text %}
This is used to force the Linux VM to keep a minimum number
of kilobytes free.  The VM uses this number to compute a
watermark[WMARK_MIN] value for each lowmem zone in the system.
Each lowmem zone gets a number of reserved free pages based
proportionally on its size.

Some minimal amount of memory is needed to satisfy PF_MEMALLOC
allocations; if you set this to lower than 1024KB, your system will
become subtly broken, and prone to deadlock under high loads.

Setting this too high will OOM your machine instantly.
{% endhighlight %}


该参数值在 init_per_zone_wmark_min() 函数中设置，


<!--
1. 代表系统所保留空闲内存的最低限。
在系统初始化时会根据内存大小计算一个默认值，计算规则是：

  min_free_kbytes = sqrt(lowmem_kbytes * 16) = 4 * sqrt(lowmem_kbytes)(注：lowmem_kbytes即可认为是系统内存大小）

另外，计算出来的值有最小最大限制，最小为128K，最大为64M。
可以看出，min_free_kbytes随着内存的增大不是线性增长，comments里提到了原因“because network bandwidth does not increase linearly with machine size”。随着内存的增大，没有必要也线性的预留出过多的内存，能保证紧急时刻的使用量便足矣。
-->

该参数的主要用途是计算影响内存回收的三个参数 ```watermark[min/low/high]```，每个 Zone 都保存了各自的参数值，不同的范围操作如下。

{% highlight text %}
< watermark[low]
  触发内核线程kswapd回收内存，直到该Zone的空闲内存数达到watermark[high]；
< watermark[min]
  进行direct reclaim (直接回收)，即直接在应用程序的进程上下文中进行回收，再用
  回收上来的空闲页满足内存申请，因此实际会阻塞应用程序，带来一定的响应延迟，
  而且可能会触发系统OOM。
{% endhighlight %}

实际上，可以认为 ```watermark[min]``` 以下的内存属于系统的自留内存，用以满足特殊使用，所以不会给用户态的普通申请来用。


<!--
3）三个watermark的计算方法：

 watermark[min] = min_free_kbytes换算为page单位即可，假设为min_free_pages。（因为是每个zone各有一套watermark参数，实际计算效果是根据各个zone大小所占内存总大小的比例，而算出来的per zone min_free_pages）

 watermark[low] = watermark[min] * 5 / 4
 watermark[high] = watermark[min] * 3 / 2

所以中间的buffer量为 high - low = low - min = per_zone_min_free_pages * 1/4。因为min_free_kbytes = 4* sqrt(lowmem_kbytes），也可以看出中间的buffer量也是跟内存的增长速度成开方关系。
4）可以通过/proc/zoneinfo查看每个zone的watermark
例如：

Node 0, zone      DMA
pages free     3960
       min      65
       low      81
       high     97


3.min_free_kbytes大小的影响
min_free_kbytes设的越大，watermark的线越高，同时三个线之间的buffer量也相应会增加。这意味着会较早的启动kswapd进行回收，且会回收上来较多的内存（直至watermark[high]才会停止），这会使得系统预留过多的空闲内存，从而在一定程度上降低了应用程序可使用的内存量。极端情况下设置min_free_kbytes接近内存大小时，留给应用程序的内存就会太少而可能会频繁地导致OOM的发生。
min_free_kbytes设的过小，则会导致系统预留内存过小。kswapd回收的过程中也会有少量的内存分配行为（会设上PF_MEMALLOC）标志，这个标志会允许kswapd使用预留内存；另外一种情况是被OOM选中杀死的进程在退出过程中，如果需要申请内存也可以使用预留部分。这两种情况下让他们使用预留内存可以避免系统进入deadlock状态。
http://kernel.taobao.org/index.php?title=Kernel_Documents/mm_sysctl
-->

<!--
一个Laravel队列引发的报警
https://huoding.com/2015/06/10/444
Linux服务器Cache占用过多内存导致系统内存不足问题的排查解决（续）
http://www.cnblogs.com/panfeng412/p/drop-caches-under-linux-system-2.html
Where is the memory going? 
https://www.halobates.de/memory.pdf
 一次linux内存问题排查-slab 
https://bhsc881114.github.io/2015/04/19/%E4%B8%80%E6%AC%A1linux%E5%86%85%E5%AD%98%E9%97%AE%E9%A2%98%E6%8E%92%E6%9F%A5-slab/
怎样诊断slab泄露问题
http://linuxperf.com/?p=148
-->


## 参考

关于内存的 OverCommit 可以参考内核文档 [vm/overcommit-accounting]( {{ site.kernel_docs_url }}/Documentation/vm/overcommit-accounting ) 。

<!-- https://www.win.tue.nl/~aeb/linux/lk/lk-9.html -->



{% highlight text %}
{% endhighlight %}
