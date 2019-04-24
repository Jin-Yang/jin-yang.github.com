---
title: 【专题】Linux 内存
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---



<!-- more -->


* [Kernel 物理映射](/post/kernel-memory-virtual-physical-map.html)，x86 中逻辑地址到物理地址的映射关系，包括了具体的实验。
* [Kernel 内存-用户空间](/post/kernel-memory-management-from-userspace-view.html)，用户空间的内存管理，包括了内存的布局、内存申请等操作。
* [Kernel 内存-内核空间](/post/kernel-memory-management-from-kernel-view.html)，包括了内核中与内存相关内容，包括了初始化、内存分配等。
* [Linux 共享内存简介](/post/linux-program-shared-memory.html)，Linux 中与共享内存相关的内容。
* [Linux 通用内存监控](/post/linux-monitor-memory.html)，简单介绍下 Linux 中与 Memory 监控相关的内容。
* [Linux 映射文件](/post/kernel-memory-mmap-introduce.html)，也就是 mmap() 函数的使用方法。
* [Kernel 内存杂项](/post/kernel-memory-tips.html)，简单介绍下内核中与内存相关的内容，以及常见的故障处理。

* [Kernel Cache VS. Buffer](/post/linux-memory-buffer-vs-cache-details.html)，Buffer 和 Cache 相关的概念比较难理解，这里仅简单介绍其皮毛。
* [Kernel CGroup Memory](/post/kernel-cgroup-memory-introduce.html)
* [Kernel OOM Killer](/post/kernel-memory-oom-killer-introduce.html)

<!--
Golang语法
https://blog.csdn.net/u013210620/article/details/78404805?locationNum=8&fps=1


内存使用详解



### /proc/meminfo

这里面的 Buffers 就是 BufferCache，而 Cached 就是 PageCache 。

第二个问题，What is the difference between Buffers and Cached columns in /proc/meminfo output?

/proc/meminfo输出的Buffers与Cached这两列有什么区别？
cat /proc/meminfo
MemTotal:      8162388 kB
MemFree:         86004 kB
Buffers:         56432 kB
Cached:        1141924 kB
SwapCached:     800992 kB
Active:        6090024 kB
Inactive:      1857208 kB
HighTotal:           0 kB
HighFree:            0 kB
LowTotal:      8162388 kB
LowFree:         86004 kB
SwapTotal:     2096472 kB
SwapFree:      1048264 kB
...
Robert Love：

短话长说，Cached等于Linux page cache的大小减去swap cache的大小，swap cache的大小是SwapCached那一列（因此全部page cache的大小就等于Cached+SwapCached）。Linux通过page cache执行所有的I/O操作。写的实现很简单，只要将page cache中相应的页标记为脏页即可；负责刷盘的线程会周期性的将脏页写回磁盘。读就是直接读取page cache的数据，如果数据还没被缓存，就先读进来。在现代的Linux系统中，Cached很容易就会达到几个G，当内存有压力时它才会缩小。只要需要系统就会清理page cache并将数据swap到磁盘以获取更多可用的内存。

Buffers是内存中块I/O的缓冲区。相对来说，它们是比较短暂的。在Linux内核2.4版本之前，page cache跟buffer cache是分开的。从2.4开始，page cache跟buffer cache统一了。Buffers就只缓存raw disk block了，这一部分不在page cache—也就是非文件数据。Buffers这个指标也就不那么重要了。大部分系统中，Buffers经常也就几十M。
在监控中开始对着两个Cache有点搞不清楚，后来查了下，弄清楚了它们的区别，都是Cache但完全不是缓存一种东西，很好区分。

我们通过三个测试例子，发现Linux系统内存中的cache并不是在所有情况下都能被释放当做空闲空间用的。并且也也明确了，即使可以释放cache，也并不是对系统来说没有成本的。总结一下要点，我们应该记得这样几点：

当cache作为文件缓存被释放的时候会引发IO变高，这是cache加快文件访问速度所要付出的成本。

当理解了这些的时候，希望大家对free命令的理解可以达到我们说的第三个层次。我们应该明白，内存的使用并不是简单的概念，cache也并不是真的可以当成空闲空间用的。如果我们要真正深刻理解你的系统上的内存到底使用的是否合理，是需要理解清楚很多更细节知识，并且对相关业务的实现做更细节判断的。我们当前实验场景是Centos 6的环境，不同版本的Linux的free现实的状态可能不一样，大家可以自己去找出不同的原因。
https://xuxinkun.github.io/2016/05/16/memory-monitor-with-cgroup/
http://blog.51cto.com/alanwu/1122077
http://blog.yufeng.info/archives/tag/fincore
https://blog.csdn.net/icycode/article/details/80200437
http://blog.yufeng.info/archives/2456
https://blog.csdn.net/liuxiao723846/article/details/72628847
http://blog.51cto.com/shanker/1787378

https://segmentfault.com/a/1190000005601925
https://www.cnblogs.com/wangchenxicool/articles/2172035.html
https://blog.csdn.net/windeal3203/article/details/52864994
https://blog.csdn.net/windeal3203/article/details/52849236
https://blog.csdn.net/feitianxuxue/article/details/9386843

grep hang死阻塞到pipe_wait
https://www.cnblogs.com/embedded-linux/p/6986525.html
https://www.chenyudong.com/archives/python-subprocess-popen-block.html
http://xstarcd.github.io/wiki/Python/python_subprocess_study.html

steal guest guest_nice

关于CPU使用率的计算方法(Devops还有很多不错的文章) account_guest_time
https://github.com/Leo-G/DevopsWiki/wiki/How-Linux-CPU-Usage-Time-and-Percentage-is-calculated
https://blog.csdn.net/jessysong/article/details/73571878
http://blog.scoutapp.com/articles/2013/07/25/understanding-cpu-steal-time-when-should-you-be-worried
Linux进程网络流量统计
http://www.freebuf.com/articles/system/182158.html
DEVOPS工具集
http://blog.jobbole.com/80879/
buildbot


















buffer 全称是 buffer cache 内存，块设备的读写缓冲区，一般是由于没有使用文件系统直接对磁盘操作。
cache 全称为 page cache 内存，作为文件系统的缓冲。

dentries 目录的数据结构
inodes 文件的数据结构

在 [ELC: How much memory are applications really using?](https://lwn.net/Articles/230975/) 中，有介绍如何将虚拟地址映射到物理地址空间，其中主要涉及了如下的几个文件。

  /proc/kpagecount — A binary array of 64-bit words, one for each page of physical RAM, containing the current count of mappings for that page.
  /proc/kpageflags — A binary array of 64-bit words, one for each page of physical RAM, containing a set of flag bits for that page.
  /proc/pid/pagemap — A binary array of 64-bit words, one for each page in process pid's virtual address space, containing the physical address of the mapped page.

然后再结合 `/proc/PID/maps` 文件，基本上就可以确认进程内存映射到了那些物理内存上。不过难点是，因为需要读取多个文件，实际很难获取到一个一致的内存镜像。

[Capturing Process Memory Usage Under Linux](http://www.eqware.net/articles/CapturingProcessMemoryUsageUnderLinux/index.html) 介绍如何将虚拟地址映射到物理地址空间。

不同的 Region 会有多个属性，包括了 Read-Only、Read-Write、Execute-Only 等。

一片很经典的介绍内存相关的内容 [What Every Programmer Should Know About Memory](https://akkadia.org/drepper/cpumemory.pdf)

介绍了申请内存时的四种分类 [Memory Types](https://techtalk.intersec.com/2013/07/memory-part-1-memory-types/)


----- 查看系统中与共享内存相关的信息
# ipcs -m
# ipcrm -m|-q|-s shm_id

共享内存使用
https://blog.csdn.net/guoping16/article/details/6584058


brk(), sbrk() 用法详解 
https://blog.csdn.net/sgbfblog/article/details/7772153

http://huqunxing.site/2017/03/31/linux%E5%86%85%E5%AD%98%E5%8D%A0%E7%94%A8%E5%88%86%E6%9E%90/
http://senlinzhan.github.io/2017/07/02/linux-memory/

内存的限制提供了多个粒度，包括了 RSS(`memory.limit_in_bytes`)、RSS+SWAP(`memory.memsw.limit_in_bytes`)，另外还有 kmem 以及 kmem.tcp 等配置，如果已经打开了 swap 那么可能看到的值要大很多。

注意，如果只设置了 RSS，那么实际上可能还可以分配内存，只是部分转储到了 swap 上。

Sysbench压测使用简介
https://wiki.gentoo.org/wiki/Sysbench
Perf测试
http://donghao.org/2016/11/30/using-sysbench-to-test-memory-performance/


Linux上通过top查看进程的RES和SHR的值很高，表示进程（独）占用的内存很多吗？ 
http://filwmm1314.blog.163.com/blog/static/2182591920121016541582/


CGroup内存与进程内存的区别
http://hustcat.github.io/memory-usage-in-process-and-cgroup/

https://lwn.net/Articles/529927/
https://github.com/torvalds/linux/blob/master/Documentation/cgroup-v1/memcg_test.txt
http://www.wowotech.net/memory_management/meminfo_1.html
http://www.361way.com/memory-analysis/5018.html
http://linuxperf.com/?p=142

与进程内存相关的文件包括了：

### /proc/PID/stat

其中包括了 `vsize` 以及 `rss` ，可以通过 `man proc` 查看相关的解释。

#### RSS

该文件在内核中的入口为 `do_task_stat()` 函数，其中 RSS 通过 `get_mm_rss()` 获取大小。

static inline unsigned long get_mm_rss(struct mm_struct *mm)
{
        return get_mm_counter(mm, MM_FILEPAGES) + get_mm_counter(mm, MM_ANONPAGES);
}

那么这两个值分别代表了什么含义呢？

内核中与进程相关的结构体为 `struct task_struct` ，其中包含了与内存相关的结构体 `struct mm_struct` ，而与 RSS 使用情况相关的为 `struct mm_rss_stat` 。

struct mm_rss_stat {
	atomic_long_t count[NR_MM_COUNTERS];
};

与该统计相关的会在创建子进程时初始化为 0 ，然后在申请内存时，也就是在 `mm/memory.c` 文件中进行累加计数。

## cgroup

对应了 cgroup 中的 `memory.stat` 文件，详细的含义可以参考 [Documentation/cgroups/memory.txt](https://lwn.net/Articles/529927/) 中的介绍。

memcg_stat_show()

mem_cgroup_stat_index

usage_in_bytes

并非准确的实时信息，如果要获取完全准确的信息需要通过 `memory.stats` 中的 RSS+CACHE(+SWAP) 获取。

注意，实测是 Cache+RSS 估计是 RSS 中同时已经包含了 SWAP 内存。

cat memory.usage_in_bytes && cat memory.stat

在 `$GOPATH` 目录下会保存三个子目录 `bin` `pkg` `src` 。

#### bin目录

用于通过 `go install` 安装二进制程序，如果设置了 `GOBIN` 环境变量则以此为准，如果 `GOPATH` 有多个目录，同样需要设置 `GOBIN` 。

#### pkg目录

在编译过程中生成的 `.a` 归档文件，一般都是代码包的名字，所有文件都会被存放到该目录下平台相关的目录中，例如 Linux 平台上为 `linux_amd64` 。

其中平台目录与 `GOOS` 和 `GOARCH` 这两个环境变量相关，系统默认自带不需要进行设置，分别表示操作系统类型和计算架构，而平台相关目录是以 `$GOOS_$GOARCH` 格式命名，对于 Linux 平台上这个目录名就是 `linux_amd64` 。

#### src目录

以代码包的形式组织并保存 Go 源码文件的，每个代码包都和 src 目录下的文件夹一一对应，每个子目录都是一个代码包。

源码包括了命令源码文件(为 `main` 包，含有 `main()` 入口函数)，库源码文件(需要保存在 `src` 目录下)，测试源码文件。

## 编译过程

与编译相关的子命令包括了 `build`、`get`、`install`、`run` 四个，其中有些通用的命令参数简介如下。

### 编译选项

* `-a` 强制重新编译所有涉及的代码包(含标准包)，即使它们已经是最新的了。
* `-n` 仅打印执行过程中所使用到的命令，而不是真正执行。
* `-race` 检测并报告程序中存在的数据竞争问题，尤其是在使用并发编程时非常重要。
* `-v` 打印命令执行过程中所涉及的代码包，包括了直接或者间接依赖的那些代码包。
* `-work` 打印命令执行过程中生成和使用的临时工作目录，而且命令执行完后不会删除。
* `-x` 打印在执行过程中所有使用到的命令，同时执行它们。

### go run

专门用来运行命令源码文件的命令，注意，**不是所有 Go 的源码文件** ！只能接受一个命令源码文件以及若干个 `main` 库源码文件作为参数，且不能接受测试源码文件。

详细示例
https://halfrost.com/go_command/
关于动态链接的示例
http://wiki.jikexueyuan.com/project/go-command-tutorial/0.1.html
从源码开始分析
https://blog.csdn.net/free2o/article/details/38417293




-->


{% highlight text %}
{% endhighlight %}
