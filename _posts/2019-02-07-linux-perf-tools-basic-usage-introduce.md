---
title: Perf 使用简介
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: program,linux,memory reordering
description:
---

Perf 是一款随 Linux 内核代码一同发布和维护的性能诊断工具，由内核社区维护和发展，不仅可以用于应用程序的性能统计分析，也可以应用于内核代码的性能统计和分析。

可以分析程序运行期间发生的硬件事件，比如 instructions retired ，processor clock cycles 等；也可以分析软件事件，比如 Page Fault 和进程切换。

这里简单介绍。

<!-- more -->

## 简介

性能调优工具如 perf，Oprofile 等的基本原理都是对被监测对象进行采样，最简单的情形是根据 tick 中断进行采样，即在 tick 中断内触发采样点，在采样点里判断程序当时的上下文。

假如一个程序 90% 的时间都花费在函数 `foo()` 上，那么 90% 的采样点都应该落在函数 `foo()` 的上下文中。

### 安装

在 CentOS 中可以直接通过 `yum install perf` 安装，或者在内核源码 `tools/perf` 目录下通过 `make && make install` 进行编译安装，安装时不需要 root，将安装在 home 目录下。

### 使用

所支持的功能包括了 Hardware Event (硬件事件，需要类似 Performance Monitoring Unit, PMU 的硬件支持)、Software Event(内核软件事件，如进程切换)、Tracepoint Event(内核静态跟踪点) 等。

perf 相关命令通过 `perf <subcmd>` 执行，大概有二十多个子命令，可以直接输入 `pref help [subcmd]` 查看，其中最常用的有 list、stat、top、record、report。

## 常用命令

通过 list 子命令查看时，默认会打印所有，可以通过添加子集过滤所需要的事件，如 `[hw|sw|cache|tracepoint|pmu|event_glob]` 。






<!--

http://wiki.csie.ncku.edu.tw/embedded/perf-tutorial
----- 

$ cat "/boot/config-`uname -r`" | grep "PERF_EVENT"
OProfile GProf
#### not supported

很多情况下虚机上是无法支持的。

   <not supported>      cycles                                                      
   <not supported>      instructions                                                
   <not supported>      branches                                                    
   <not supported>      branch-misses    

   
$ perf list | grep stalled
  stalled-cycles-frontend OR idle-cycles-frontend    [Hardware event]
  stalled-cycles-frontend OR cpu/stalled-cycles-frontend/ [Kernel PMU event]

$ ls /sys/devices/cpu/events/
branch-instructions  bus-cycles    cache-references  instructions  mem-stores
branch-misses        cache-misses  cpu-cycles        mem-loads     stalled-cycles-frontend

$ cat /sys/bus/event_source/devices/cpu/events/stalled-cycles-frontend
event=0x0e,umask=0x01,inv,cmask=0x01

----- 查看当前所支持的事件
$ perf list

----- 整理的统计指标，可能会出现not supported
$ perf stat program args
$ perf stat dd if=/dev/zero of=test.iso bs=10M count=1
输出指标:
  task-clock       运行时占用CPU的时钟周期，可以判定CPU/IO Bound；
  context-switches 上下文切换次数，包括了进程间切换以及内核态和用户态的切换；
  cpu-migrations   运行期间发生CPU迁移次数，也即从一个CPU运行切换到另外的CPU；
  page-faults      程序发生了缺页异常的次数。

----- 实时显示当前系统TopN进程、函数，通过-e指定关注的指标，默认是CPU
$ perf top -e cache-miss

----- 通过record+report统计程序性能TOP指标
$ perf record -e cpu-clock -ag program args
$ perf stat dd if=/dev/zero of=test.iso bs=10M count=1
参数：
  -a           统计所有的CPU；
  -g           记录函数的调用关系，也即调用栈；
  -e cpu-clock 监控的指标为CPU调用周期；
$ perf report -i perf.data

perf record ls
perf annotate -d ls
https://github.com/brendangregg/perf-tools

<br><h2>perf list</h2><p>
使用 perf list 命令可以列出所有能够触发 perf 采样点的事件，包括软件以及硬件，root 用户会有更多的采样点事件，如 hw (Hardware Events)、sw (Software Events)、cache (Hardware Cache Events)、tracepoint (Tracepoint Events)。<br><br>

不同的系统会列出不同的结果，大致可以将它们划分为三类：<ol><li>
Hardware Event<br>
由 PMU 硬件产生的事件，比如 cache 命中，当需要了解程序对硬件特性的使用情况时，便需要对这些事件进行采样；</li><br><li>

Software Event<br>
是内核软件产生的事件，与硬件无关，比如进程切换，tick 数等;</li><br><li>

Tracepoint event<br>
内核中的静态 tracepoint 所触发的事件，基于 ftrace，这些 tracepoint 用来判断程序运行期间内核的行为细节，比如 slab 分配器的分配次数等。</li></ol>
(2) 指定性能事件(以它的属性)
-e <event> : u // userspace
-e <event> : k // kernel
-e <event> : h // hypervisor
-e <event> : G // guest counting (in KVM guests)
-e <event> : H // host counting (not in KVM guests)

(3) 使用例子
显示内核和模块中，消耗最多CPU周期的函数：
# perf top -e cycles:k
显示分配高速缓存最多的函数：
# perf top -e kmem:kmem_cache_alloc



<h2>perf stat</h2><p>
统计一个应用程序的运行性能指标，也即可以直接跟一个程序，当程序退出后会打印统计的结果。
<pre style="font-size:0.8em; face:arial;">
$ perf stat ./simple            // simple编译的时候必须使用-g
$ perf stat -r 5 ls             // 查看ls，通过-r执行多次
</pre>

缺省情况下，除了 task-clock-msecs 之外，perf stat 还给出了其他几个最常用的统计信息：<ul><li>
task-clock<br>
CPU 利用率，该值高，说明程序的多数时间花费在 CPU 计算上而非 IO。</li><li>

context-switches<br>
进程切换次数，记录了程序运行过程中发生了多少次进程切换，频繁的进程切换是应该避免的。</li><li>

cpu-migrations<br>
表示进程运行过程中发生了多少次 CPU 迁移，即被调度器从一个 CPU 转移到另外一个 CPU 上运行。</li><li>

page-faults<br>
程序在运行过程中发生的缺页中断次数。</li><li>

cycles<br>
执行程序所发费的处理器周期数。</li><li>

instructions<br>
执行程序所发费的指令数。</li><li>

branches<br>
程序在运行过程中的分支指令数。</li><li>

branch-misses<br>
程序在运行过程中的分支预测失败次数。</li><li>

cache-references<br>
记录程序在运行过程中的cache命中次数。</li><li>

cache-misses<br>
记录程序在运行过程中的cache失效次数。
</li></ul>
</p>


<br><br><h2>perf top</h2><p>
对整个系统直接进行采样，可以直接看到消耗最多的函数，也可以通过 -e 指定关注的事件，默认是 cycles 。
<pre style="font-size:0.8em; face:arial;">
# perf top -e cache-misses              # 造成cache miss最多的函数
</pre>
</p>



<br><br><h2>perf record</h2><p>
可以用 -F count 来指定采样频率，防止由于频率过低导致部分数据没有采集到。
</p>



<br><br><h2>perf report</h2><p>
-->

## 源码解析

如上所述，源码在内核的 `tools/perf` 目录下，其中入口在 `main()@perf.c`，会根据子命令调用不同的函数，如 `cmd_list()@builtin-list.c`、`cmd_stat()@builtin-stat.c` 等。

### list

该指令对应 `cmd_list()` 函数，该函数依赖 debugfs，在 `tracing/events` 目录下。

## 参考

* 详细可以查看 [perf wiki](https://perf.wiki.kernel.org/index.php/Main_Page) 以及 [Brendan Gregg's perf examples](http://www.brendangregg.com/perf.html) 中的示例。

<!--
http://wiki.csie.ncku.edu.tw/embedded/perf-tutorial
-->

{% highlight text %}
{% endhighlight %}
