---
title: 容器之 CGroup
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,cgroup,container
description:
---

在 CentOS 7 中，已经通过 systemd 替换了之前的 cgroup-tools 工具，为了防止两者冲突，建议只使用 systemd ，只有对于一些不支持的，例如 net_prio ，才使用 cgroup-tools 工具。

在此，简单介绍其使用。

<!-- more -->

## 简介

在系统设计时，经常会遇到类似的需求，就是希望能限制某个或者某些进程的分配资源。

由此，就有了容器的概念，在容器中，有分配好的特定比例的 CPU、IO、内存、网络等资源，这就是 controller group ，简称为 cgroup ，最初由 Google 工程师提出，后来被整合进 Linux 内核中。

cgroup 本身提供了将进程进行分组化管理的功能和接口的基础结构。

## 使用简介

在 CentOS 7 中需要通过 ```yum install libcgroup libcgroup-tools``` 安装额外的 cgroup 工具，对系统来说，默认会挂载到 ```/sys/fs/cgroup/``` 目录下。

{% highlight text %}
----- 查看系统已经存在cgroup子系统及其挂载点
# lssubsys -am
----- 或者通过mount查看cgroup类型的挂载点
# mount -t cgroup

----- 可以命令行挂载和卸载子系统，此时再次执行上述命令将看不到memory挂载点
# umount /sys/fs/cgroup/memory/
----- 挂载cgroup的memory子系统，其中最后的cgroup参数是在/proc/mounts中显示的名称
# mount -t cgroup -o memory cgroup /sys/fs/cgroup/memory/
# mount -t cgroup -o memory none /sys/fs/cgroup/memory/
{% endhighlight %}

另外，在 CentOS 中有 ```/etc/cgconfig.conf``` 配置文件，该文件中可用来配置开机自动启动时挂载的条目：

{% highlight text %}
mount {
    net_prio = /sys/fs/cgroup/net_prio;
}
{% endhighlight %}

然后，通过 ```systemctl restart cgconfig.service``` 重启服务即可，然后可以通过如下方式使用。

### 使用步骤

简单介绍如何通过 ```libcgroup-tools``` 创建分组并设置资源配置参数。

#### 1. 创建控制组群

可以通过如下方式创建以及删除群组，创建后会在 cpu 挂载目录下 ```/sys/fs/cgroup/cpu/``` 目录下看到一个新的目录 test，这个就是新创建的 cpu 子控制组群。

{% highlight text %}
# cgcreate -g cpu:/test
# cgdelete -g cpu:/test
{% endhighlight %}

#### 2. 设置组群参数

```cpu.shares``` 是控制 CPU 的一个属性，更多的属性可以到 ```/sys/fs/cgroup/cpu``` 目录下查看，默认值是 1024，值越大，能获得更多的 CPU 时间。

{% highlight text %}
# cgset -r cpu.shares=512 test
{% endhighlight %}


#### 3. 将进程添加到控制组群

可以直接将需要执行的命令添加到分组中。

{% highlight text %}
----- 直接在cgroup中执行
# cgexec -g cpu:small some-program
----- 将现有的进程添加到cgroup中
# cgclassify -g subsystems:path_to_cgroups pidlist
{% endhighlight %}

例如，想把 sshd 添加到一个分组中，可以通过如下方式操作。

{% highlight text %}
# cgclassify -g cpu:/test `pidof sshd`
# cat /sys/fs/cgroup/cpu/test/tasks
{% endhighlight %}

就会看到相应的进程在这个文件中。

### CPU

在 CGroup 中，与 CPU 相关的子系统有 cpusets、cpuacct 和 cpu 。

* cpuset 用于设置 CPU 的亲和性，可以限制该组中的进程只在(或不在)指定的 CPU 上运行，同时还能设置内存的亲和性，一般只会在一些高性能场景使用。
* 另外两个，cpuaccct 和 cpu 保存在相同的目录下，其中前者用来统计当前组的 CPU 统计信息。

这里简单介绍 cpu 子系统，包括怎么限制 cgroup 的 CPU 使用上限及与其它 cgroup 的相对值。

<!--
根据不同的CPU调度策略进行的讨论
https://yq.aliyun.com/articles/54483
如何采集容器的指标
https://www.datadoghq.com/blog/how-to-collect-docker-metrics/
-->

#### cpu.cfs_period_us & cpu.cfs_quota_us

其中 `cfs_period_us` 用来配置时间周期长度；`cfs_quota_us` 用来配置当前 cgroup 在设置的周期长度内所能使用的 CPU 时间数，两个文件配合起来设置 CPU 的使用上限。

两个文件单位是微秒，`cfs_period_us` 的取值范围为 `[1ms, 1s]`，默认 100ms ；`cfs_quota_us` 的取值大于 1ms 即可，如果 `cfs_quota_us` 的值为 `-1`(默认值)，表示不受 cpu 时间的限制。

下面是几个例子：

{% highlight text %}
----- 1.限制只能使用1个CPU，每100ms能使用100ms的CPU时间
# echo 100000 > cpu.cfs_quota_us
# echo 100000 > cpu.cfs_period_us

------ 2.限制使用2个CPU核，每100ms能使用200ms的CPU时间，即使用两个内核
# echo 200000 > cpu.cfs_quota_us
# echo 100000 > cpu.cfs_period_us

------ 3.限制使用1个CPU的50%，每100ms能使用50ms的CPU时间，即使用一个CPU核心的50%
# echo 50000 > cpu.cfs_quota_us
# echo 100000 > cpu.cfs_period_us
{% endhighlight %}

#### cpu.shares

用于设置相对值，这里针对的是所有 CPU (多核)，默认是 1024，假如系统中有两个 A(1024) 和 B(512)，那么 A 将获得 `1024/(1204+512)=66.67%` 的 CPU 资源，而 B 将获得 33% 的 CPU 资源。

对于 shares 有两个特点：

* 如果A不忙，没有使用到66%的CPU时间，那么剩余的CPU时间将会被系统分配给B，即B的CPU使用率可以超过33%；
* 添加了一个新的C，它的shares值是1024，那么A和C的限额变为`1024/(1204+512+1024)=40%`，B的资源变成了20%；

也就是说，在空闲时 shares 基本上不起作用，只有在 CPU 忙的时候起作用。但是这里设置的值是需要与其它系统进行比较，而非设置了一个绝对值。

<!--
### cpu.stat

包含了下面三项统计结果

nr_periods： 表示过去了多少个cpu.cfs_period_us里面配置的时间周期
nr_throttled： 在上面的这些周期中，有多少次是受到了限制（即cgroup中的进程在指定的时间周期中用光了它的配额）
throttled_time: cgroup中的进程被限制使用CPU持续了多长时间(纳秒)
-->

#### 示例

演示一下如何控制CPU的使用率。

{% highlight text %}
----- 创建并查看当前的分组
# cgcreate -g cpu:/small
# ls /sys/fs/cgroup/cpu/small

----- 查看当前值，默认是1024
# cat /sys/fs/cgroup/cpu/small/cpu.shares
# cgset -r cpu.shares=512 small

----- 执行需要运行的程序，或者将正在运行程序添加到分组
# cgexec -g cpu:small ./foobar
# cgclassify -g cpu:small <PID>

----- 设置只能使用1个cpu的20%的时间
# echo 50000 > cpu.cfs_period_us
# echo 10000 > cpu.cfs_quota_us

----- 将当前bash加入到该cgroup
# echo $$
5456
# echo 5456 > cgroup.procs

----- 启动一个bash内的死循环，正常情况下应该使用100%的cpu，消耗一个核
# while :; do echo test > /dev/null; done
{% endhighlight %}

注意，如果是在启动进程之后添加的，实际上 CPU 资源限制的速度会比较慢，不是立即就会限制死的，而且不是严格准确。如果起了多个子进程，那么各个进程之间的资源是共享的。

#### 其它

可以通过如下命令查看进程属于哪个 cgroup 。

{% highlight text %}
# ps -O cgroup
# cat /proc/PID/cgroup
{% endhighlight %}

### 内存

相比来说，内存控制要简单的多，只需要注意物理内存和 SWAP 即可。

{% highlight text %}
----- 创建并查看当前的分组
# cgcreate -g memory:/small
# ls /sys/fs/cgroup/memory/small

----- 查看当前值，默认是一个很大很大的值，设置为1M
# cat /sys/fs/cgroup/memory/small/memory.limit_in_bytes
# cgset -r memory.limit_in_bytes=10485760 small

----- 如果开启了swap之后，会发现实际上内存只限制了RSS，设置时需要确保没有进程在使用
# cgset -r memory.memsw.limit_in_bytes=104857600 small

----- 启动测试程序
# cgexec -g cpu:small -g memory:small ./foobar
# cgexec -g cpu,memory:small ./foobar
{% endhighlight %}

可以使用 `x="a"; while [ true ];do x=$x$x; done;` 命令进行测试。

#### 内存回收

在通过 `memory.usage_in_bytes` 查询当前 cgroup 的内存资源使用情况时，如果对比当前组中进程的 RSS 资源时，可能会发现，前者要远大于后者。甚至于，当前 cgroup 中已经没有了进程，但是其内存使用量仍然很大。

构造的测试场景如下，其中 cgroup 仍然使用上述创建的组。

{% highlight text %}
$ cat memory.usage_in_bytes
0
$ cgexec -g cpu:small -g memory:small dd if=/dev/sda1 of=/dev/null count=10M
10485760+0 records in
10485760+0 records out
5368709120 bytes (5.4 GB) copied, 33.8483 s, 159 MB/s
$ cat memory.usage_in_bytes
4930969600
{% endhighlight %}

实际这与内核处理系统内存时的机制一样，在内存足够的情况下，默认不会自动释放内存。所以，看到的内存使用情况与实际不符。

这样带来的问题时，如果开始设置的内存空间为 1G ，当前使用了 700M (实际 300M)，当前如果想限制到 500M ，如果内存不能被回收那么可能会报错。

{% highlight text %}
$ echo 524288000 > memory.limit_in_bytes
{% endhighlight %}

上述占用空间较大的是 buffer ，通过上述方式设置是可以被回收掉。

对于上述的场景，如果要回收所有的内存，有两种方式。

{% highlight text %}
----- 释放的是系统的Buffer和Cache
# echo 3 > /proc/sys/vm/drop_caches
----- 需要保证该cgroup组下面没有进程，否者会失败
# echo 0 > memory.force_empty
{% endhighlight %}

#### OOM

当进程试图占用的内存超过了 cgroups 的限制时，会触发 out of memory 导致进程被强制 kill 掉。

{% highlight text %}
----- 关闭默认的OOM
# echo 1 > memory.oom_control
# cgset -r memory.oom_control=1 small
{% endhighlight %}

注意，及时关闭了 OOM，对应的进程会处于 uninterruptible sleep 状态。

<!--
memory 子系统中还有一个很重要的设置是 memory.use_hierarchy 这是个布尔开关，默认为 0。此时不同层次间的资源限制和使用值都是独立的。当设为 1 时，子控制组进程的内存占用也会计入父控制组，并上溯到所有 memory.use_hierarchy = 1 的祖先控制组。这样一来，所有子孙控制组的进程的资源占用都无法超过父控制组设置的资源限制。同时，在整个树中的进程的内存占用达到这个限制时，内存回收也会影响到所有子孙控制组的进程。这个值只有在还没有子控制组时才能设置。之后在其中新建的子控制组默认的 memory.use_hierarchy 也会继承父控制组的设置。

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main ()
{
        pid_t fpid;
        int count=0;
        void *mem;

        fpid = fork();
        if (fpid < 0) {
                fprintf(stderr, "[ERROR] failed to fork!, %s\n", strerror(errno));
                return -1;
        } else if (fpid == 0) { /* child */
                //while(1) count++;
                while(1) {
                        usleep(1000);
                        mem = calloc(1, 10000);
                        if (mem == NULL) {
                                fprintf(stderr, "[ERROR] failed to malloc\n");
                                usleep(100000);
                        }
                }
        } else { /* parent */
                fprintf(stderr, "[INFO] Parent %d started, child %d\n", getpid(), fpid);
                //while(1) count++;
                wait(NULL);
        }
        return 0;
}
-->

### 磁盘

可以通过 blkio 进行设置，不过只能针对设备限速，例如可以设置 `/dev/sda` 而无法设置具体的分区。

如下是一个示例。

{% highlight text %}
----- 查看并选择其中的一个
# df -h
----- 直接读取某个磁盘分区
# dd if=/dev/sda1 of=/dev/null
{% endhighlight %}

可以通过 `iotop` 查看，因为是单纯的读取，其速度一般可以达到 `100M/s` 以上。

{% highlight text %}
----- 新建一个cgroup的分组
# mkdir /sys/fs/cgroup/blkio/foobar
# cd /sys/fs/cgroup/blkio/foobar
----- 配置读取某个磁盘的最大速度，也就是1M
# echo '8:0 1048576' > blkio.throttle.read_bps_device
{% endhighlight %}

上述配置中的 `8:0` 为设备的主次设备号，可以通过 `ls -l /dev/sda` 查看。

在 blkio 中，除了设置，还可以监控 IO 的使用情况，所以大部分的文件都是只读的，可以配置的只有如下的几个。

{% highlight text %}
blkio.throttle.read_bps_device
blkio.throttle.read_iops_device
blkio.throttle.write_bps_device
blkio.throttle.write_iops_device
blkio.weight
blkio.weight_device
{% endhighlight %}

其中 `throttle` 是用来配置流量限速的，而 `weight` 则是配置权重信息。blkio 子系统里还有很多统计项，用来监控当前 cgroup 的使用情况。

## systemd

CentOS 7 中默认的资源隔离是通过 systemd 进行资源控制的，systemd 内部使用 cgroups 对其下的单元进行资源管理，包括 CPU、BlcokIO 以及 MEM，通过 cgroup 可以 。

systemd 的资源管理主要基于三个单元 service、scope 以及 slice：

* service<br>通过 unit 配置文件定义，包括了一个或者多个进程，可以作为整体启停。
* scope<br>任意进程可以通过 ```fork()``` 方式创建进程，常见的有 session、container 等。
* slice<br>按照层级对 service、scope 组织的运行单元，不单独包含进程资源，进程包含在 service 和 scope 中。

常用的 slice 有 A) ```system.slice```，系统服务进程可能是开机启动或者登陆后手动启动的服务，例如crond、mysqld、nginx等服务；B) ```user.slice``` 用户登陆后的管理，一般为 session；C) ```machine.slice``` 虚机或者容器的管理。

对于 cgroup 默认相关的参数会保存在 ```/sys/fs/cgroup/``` 目录下，当前系统支持的 subsys 可以通过 ```cat /proc/cgroups``` 或者 ```lssubsys``` 查看。

<!--
服务进程限制
    PrivateNetwork=[BOOL] :若服务不需要网络连接可开启本选项，更加安全。
    PrivateTmp=[BOOl] :由于传统/tmp目录是所有本地用户和服务共用，会带来很多安全性问题，开启本选项后，服务将有一个私有的tmp，可防止攻击。
    InaccessibleDirectories= :限制服务进程访问某些目录。
    ReadOnlyDirectories= :设置服务进程对某些目录只读，保证目录下数据不被服务意外撰改。
    OOMScoreAdjust= :调整服务OOM值，从-1000（对该服务进程关闭OOM）到1000(严格)。
    IOSchedulingClass= ：IO调度类型，可设置为0，1,2,3中的某个数值,分配对应none，realtime，betst-effort和idle。
    IOSchedulingPriority= :IO调度优先级，0～7（高到低）。
    CPUSchedulingPriority= :CPU调度优先级，99～1(高到低)
    Nice= :进程调度等级。
-->

### 常见命令

常用命令可以参考如下。

{% highlight text %}
----- 查看slice、scope、service层级关系
# systemd-cgls

----- 当前资源使用情况
# systemd-cgtop

----- 启动一个服务
# systemd-run --unit=name --scope --slice=slice_name command
   unit   用于标示，如果不使用会自动生成一个，通过systemctl会输出；
   scope  默认使用service，该参数指定使用scope ；
   slice  将新启动的service或者scope添加到slice中，默认添加到system.slice，
          也可以添加到已有slice(systemctl -t slice)或者新建一个。
# systemd-run --unit=toptest --slice=test top -b
# systemctl stop toptest

----- 查看当前资源使用状态
$ systemctl show toptest
{% endhighlight %}

各服务配置保存在 ```/usr/lib/systemd/system/``` 目录下，可以通过如下命令设置各个服务的参数。

{% highlight text %}
----- 会自动保存到配置文件中做持久化
# systemctl set-property name parameter=value

----- 只临时修改不做持久化
# systemctl set-property --runtime name property=value

----- 设置CPU和内存使用率
# systemctl set-property httpd.service CPUShares=600 MemoryLimit=500M
{% endhighlight %}

另外，在 213 版本之后才开始支持 `CPUQuota` 参数，可直接修改 `cpu.cfs_{quota,period}_us` 参数，也就是在 `/sys/fs/cgroup/cpu/` 目录下。

## libcgroup

基于 [libcgroup](http://libcg.sourceforge.net/) 实现一套容器的管理，详细的文档可以参考 [libcg Documentation](http://libcg.sourceforge.net/html/index.html) 中的相关介绍。

可以参考 [Github - cgfy](https://github.com/geokat/cgfy) 中的实现，该程序是通过 `libcgroup` 实现，功能类似于 cgexec 。

另外，也可以参考 [Github - clique](https://github.com/vodik/clique)，是直接利用 DBus 与 Systemd 进行通讯。

<!--
int cgroup_init(void);
  初始化函数，会自动获取当前的cgroup挂载点，注意，如果unmount、remount等操作，不会自动刷新；

cgroup_walk_tree_begin()
  遍历某个subsys，可以通过 cgroup_walk_tree_set_flags() 设置遍历的方式。
-->

## 其它

一个自动挂载的脚本。

{% highlight bash %}
#!/bin/bash

CGROUP_PATH="/tmp/cgroup"
CGROUP_SUBSYS="cpu memory cpuacct cpuset freezer net_cls blkio devices"
MOUNTS_PATH="/proc/mounts"
FILESYSTEMS_PATH="/proc/filesystems"

CreateCGroup() {
        for sys in ${CGROUP_SUBSYS}; do
                if [ ! -d "${CGROUP_PATH}/${sys}" ]; then
                        mkdir -p "${CGROUP_PATH}/${sys}"
                        if [ $? -ne 0 ]; then
                                echo "Create directory '${CGROUP_PATH}/${sys}' failed." 2>&1
                                exit 1
                        fi
                        echo "Create directory '${CGROUP_PATH}/${sys}' done."
                fi
                mount -t cgroup -o "${sys},relatime" cgroup "${CGROUP_PATH}/${sys}"
                if [ $? -ne 0 ]; then
                        echo "Mount subsys ${sys} to '${CGROUP_PATH}/${sys}' failed."
                        exit 1
                fi
        done
}

if [ ! -f "${FILESYSTEMS_PATH}" ]; then
        echo "File '${FILESYSTEMS_PATH}' doesn't exist." 2>&1
        exit 1
fi

if [ `grep -c '\<cgroup$' ${FILESYSTEMS_PATH}` -eq '0' ]; then
        echo "Filesystem 'cgroup' doesn't support on current platform." 2>&1
        exit 1
fi

if [ ! -f "${MOUNTS_PATH}" ]; then
        echo "File '${MOUNTS_PATH}' doesn't exist." 2>&1
        exit 1
fi

if [ `grep -c '^cgroup\>' ${MOUNTS_PATH}` -eq '0' ]; then
        #echo "Filesystem 'cgroup' hasn't mounted."
        CreateCGroup
else
        #echo "Filesystem 'cgroup' has mounted."
        if [ `grep -cE '^cgroup\>.*(\<cpu\>|\<memory\>)' ${MOUNTS_PATH}` -ne '2' ]; then
                echo "Invalid cgroup subsys 'cpu' 'memory'." 2>&1
                exit 1
        fi
        info=`grep -E '^cgroup\>.*\<memory\>' /proc/mounts | awk '{ print $2 }'`
        CGROUP_PATH=`dirname ${info}`
fi
echo ${CGROUP_PATH}
{% endhighlight %}



## 参考

关于 systemd 的资源控制，详细可以通过 ```man 5 systemd.resource-control``` 命令查看帮助，或者查看 [systemd.resource-control 中文手册](http://www.jinbuguo.com/systemd/systemd.resource-control.html)；详细的内容可以参考 [Resource Management Guide](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/7/html/Resource_Management_Guide/index.html) 。

<!--
Systemd 进程管理相关
http://fangpeishi.com/systemd_chapter2.html

https://yq.aliyun.com/articles/54458
http://www.cnblogs.com/yanghuahui/p/3751826.html
https://yq.aliyun.com/articles/54483
systemd-cgls


Python使用
https://github.com/francisbouvier/cgroups

cgred


CGroup使用
https://tech.meituan.com/cgroups.html
如何通过配置文件限制资源使用
https://blog.csdn.net/jesseyoung/article/details/39077829
https://ggaaooppeenngg.github.io/zh-CN/2017/05/07/cgroups-%E5%88%86%E6%9E%90%E4%B9%8B%E5%86%85%E5%AD%98%E5%92%8CCPU/
https://github.com/coredump/cgrouptool

在某个容器内运行一个线程
https://github.com/geokat/cgfy/blob/master/cgfy.c
https://github.com/chr15p/cgshares
https://github.com/bmwcarit/cgroup-tests
https://github.com/erebe/collectd_cgroup_memory_pressure/blob/master/collectd_cgroup_memory_pressure.c
https://github.com/enukane/cgpstree
https://github.com/dveeden/udf_cgroup
https://github.com/poelzi/ulatencyd



其中关于 cpuacct 返回的内容实现在内核的 `kernel/sched/cpuacct.c` 文件中，可以看到起对应返回值的类型。

注意，这里的统计包含了，当前的 cgroup 组以及该组下的子组中的 CPU 使用。

cpuacct.usage 统计 CPU 消耗时间(单位是纳秒)，在计算使用率时应该再除以采集间隔。
cpuacct.stat 统计了包括 user system 空间的 CPU 使用率，其对应的单位是 USER_HZ 。

如果设置了 CPUQuota 值，

在获取系统时间时建议优先采用 `gettimeofday()` 获取微秒级的时间，如果非必须建议不要调用 `clock_gettime()` 获取纳秒级的时间，因为前者会通过 VDSO 做系统调用，相比来说要快很多。







内核调度的调优参数
http://yupeng0921.blogspot.com/2012/08/cfstuning.html


## cgroup 进程调度

Linux 中对于已经就绪的进程，会添加到就绪队列 `struct rq` 中，当 CPU 需要加载新任务时，那么调度器就会从就绪队列中选择一个最优的进程加载执行。

选择进程时依赖的是优先级，Linux 将进程分为普通和实时两种类型，用 `0~139` 表示优先级，值越小优先级越高，其中 `0~99` 为实时进程，`100~139` (对应用户空间 nice 值范围 `-20~19`) 表示普通进程，显然，实时进程总是高于普通进程。

普通和实时进程使用两套不同的算法，分别为完全公平调度和实时调度，内核为此抽象了一个结构体 `struct sched_class`，来实现类似面向对象的功能，分别对应了 fair_sched_class 和 rt_sched_class 。

那么就绪队列里也分为了两个子队列：A) 维护普通进程的 CFS 队列 cfs_rq；B) 维护实时进程的 RT 队列 rt_rq 。每个 CPU 会对应一个就绪队列。

### 调度实体

调度器最终是以线程为粒度，但实际上是通过调度实体 `struct sched_entity` 和 `struct sched_rt_entity` 表示一个要被调度的对象。

对于普通的进程调度来说，调度实体对象内嵌在 `struct task_struct` 中；对于 cgroup 则内嵌在 `struct task_group` 中。

其中 CFS 队列采用红黑树组织调度实体，总是选择最左边的调度实体；而 RT 队列则使用类似 `rt_queue[100][list]` 这样的结构，100 个数组对应实时进程的 `0~99` 个优先级，而二维则是链表，每次都选择优先级最高的进程。

## 代码详解

scheduler_tick()
  |-update_rq_clock()                 更新运行队列的时钟rq->clock
  |-curr->sched_class->task_tick()    执行调度器的周期性函数，以CFS为例
  | |-task_tick_fair()                CFS对应的task_tick()
  |   |-cfs_rq_of()
  |   |-entity_tick()
  |     |-update_curr()               更新当前进程的vruntime值
  |     | |-rq_clock_task()           获取当前的实际时间，计算执行时间差值
  |     | |-account_cfs_rq_runtime()
  |     |-update_cfs_shares()         对SMP有效
  |     |-check_preempt_tick()        检查当前进程是否运行了足够长的时间，是否需要抢占
  |-update_cpu_load_active()          更新运行队列load，将数组中先前存储的load向后移动一个位置，并插入新值

上面讨论时一直针对的是调度实体，一个调度实体可能对应一个 `struct task_struct`，也可能对应一个 `struct task_group` 。


因为实时进程优先级总是高于普通进程,又不使用完全公平算法,极端情况下实时进程一直占着cpu,普通进程等不到cpu资源.
因此实现用rt_time,rt_runtime用来限制实时进程占用cpu资源,例如rt_time = 100 rt_runtime = 95,这两个变量对应cgroup下的cpu.rt_period_us, cpu.rt_runtime_us.
那么该rq下,所有的实时进程只能占用cpu资源的95%,剩下的%5的资源留给普通进程使用.

https://www.cnblogs.com/acool/p/6882644.html
https://segmentfault.com/a/1190000010543907
https://blog.csdn.net/yin262/article/details/46730261
http://m.blog.chinaunix.net/uid-20543183-id-1930840.html
https://www.cnblogs.com/acool/p/6882644.html
https://www.cnblogs.com/acool/p/6852250.html
https://www.twblogs.net/a/5b8165522b71772165ac6322
https://www.cnblogs.com/honpey/p/7816386.html
https://blog.csdn.net/eleven_xiy/article/details/76602245
https://www.cnblogs.com/lisperl/archive/2012/04/28/2474872.html



cgroup详解含源码
https://www.cnblogs.com/lisperl/archive/2012/04/28/2474872.html
https://blog.csdn.net/eleven_xiy/article/details/76602245
https://www.cnblogs.com/honpey/p/7816386.html
https://www.twblogs.net/a/5b8165522b71772165ac6322

cgroup简介
https://segmentfault.com/a/1190000006917884

如果出现了多个子系统的挂载点，会产生什么影响？此时的资源是如何限制的？

在用户空间的 cgroup 管理是通过 cgroup 文件系统实现的，所有的操作与文件系统操作类似，例如，对于子系统需要通过 mount 进行挂载，修改配置可以实现参数的修改。

## 简介

因为是一个虚拟的文件系统，所以，系统是否支持 cgroup 可以通过 `/proc/filesystems` 查看，比如要挂在一个 cgroup 可以执行如下命令：

mount -t cgroup -o cpu,relatime cgroup /tmp/cgroup/cpu

其中的 `-t` 参数必须指定为 cgroup ；`-o` 指定了挂载的子类以及参数，可以指定多个子类；接着是设备路径，因为是虚拟文件系统，可以任意指定，一般是 cgroup；最后是挂载点。

在上述的挂载点中，会自动生成相关的控制文件。

### 内核实现

Linux 中的文件系统实现都是基于 VFS ，这是一个内核的软件层，定义了一些标准接口的实现，用来处理与 Unix 标准文件系统的所有系统调用。

具体而言，文件在内核空间中通过 `struct file` 表示，其中包含了一个 `const struct file_operations *f_op` 字段，字段中包含了一组指向特定文件系统实现的函数指针。

例如，当用户执行 `read()` 操作时，内核会通过系统调用 `sys_read()` 查找到指向该文件属于的文件系统的读函数指针，并调用它，也就是 `file->f_op->read()` 。

## 实现细节

Linux 中的大部分功能都是以模块的方式实现的，cgroup 也是相同的方式，其实现的源码在 `kernel/cgroup.c` 文件中。

### 数据结构

#### 文件系统

cgroup_fork() 用于 fork() 进程时调用，主要是为了复制父进程的 cgroup 配置信息。
### 初始化

初始化分成了两部分，分别对应了 `cgroup_init_early()` 和 `cgroup_init()` ，前者主要是在系统启动的时候做一些初始化操作。

cgroup_init()
 |-bdi_init() 初始化DBI设备，猜测主要是为了防止一些回写之类的操作
 |-cgroup_init_subsys() 针对各个子系统做初始化
 |-css_set_hash() 其中的CSS主要是为了优化进程的fork速度
 |-kobject_create_and_add() 创建一个cgroup的kobject对象
 |-register_filesystem() 注册cgroup文件系统
 |-proc_create() 创建/proc/cgroups文件，子系统是否支持可以通过该文件判断

### 挂载

mount -t cgroup -o cpu,relatime cgroup /tmp/cgroup/cpu

在 `cgroup_init()` 初始化的时候，会注册 cgroup 文件系统，其对应的接口如下。

static struct file_system_type cgroup_fs_type = {
        .name = "cgroup",
        .mount = cgroup_mount,
        .kill_sb = cgroup_kill_sb,
};

上述结构体定义了 cgroup 文件系统相关的一些操作 API，会通过 `register_filesystem()` 函数进行注册，对于用户空间的 mount 命令来说，最终会调用 `cgroup_mount()` 函数。

在分析该函数时，有几个点需要关注 A) 部分 subsystem 可以挂载多次，例如 memory ，而有些则不行，例如 cpu ；B) 挂载时可以选择多个 subsystem 使用。

cgroup_mount()
 |-parse_cgroupfs_options() 解析传入的参数
 |-cgroup_root_from_opts() 会新建一个struct cgroupfs_root对象
 | |-init_root_id() 初始化层级Unique ID
 | |-init_cgroup_root() 创建根cgroup
 |-sget() 获取一个super block
 |-rebind_subsystems()
 |-cgroup_populate_dir() 创建跟cgroup下的各种文件
   |-cgroup_addrm_files() 添加基本的控制文件，也就是struct cftype files
   | |-cgroup_add_file()
   |   |-cgroup_create_file() 创建文件或者目录
   |     |-cgroup_new_inode() 包含了文件以及inode操作方法
   |-for_each_subsys() 然后遍历每个subsys

真正的文件操作对应了 `struct file_operations` 和 `struct inode_operations` 结构体，分别对应了读写、属性设置等。

## 创建子目录

## 进程关系

使用 cgroup 的最终目的是为了管理进程组、控制进程组的资源，所以需要明确：A) cgroup 需要管理哪些进程；B) 管理进程的哪些资源。

进程如果需要与 cgroup 以及 subsystem 建立联系，其关键在于 `struct css_set` 结构体。

内核中的进程对象通过 `struct task_struct` 标示，与 cgroup 相关的包含了如下的两个成员对象。

struct task_struct {
	... ...
#ifdef CONFIG_CGROUPS
    /* Control Group info protected by css_set_lock */
    struct css_set __rcu *cgroups;
    /* cg_list protected by css_set_lock and tsk->alloc_lock */
    struct list_head cg_list;
#endif
	... ...
}

`struct css_set` 是一个辅助的结构体，其主要的目的是为了保存一系列的 `struct cgroup_subsys_state` 结构体，并使用引用计数以降低 `fork()` 进程时的开销。

struct css_set {
	struct cgroup_subsys_state *subsys[CGROUP_SUBSYS_COUNT]; // 保存了进程所对应的subsystem信息
};

struct cgroup_subsys_state {
	struct cgroup_subsys *ss; // 指向了对应的subsyste系统
};

如上，对应了一个进程与 subsystem 的映射关系，也就是 `B)` 问题的解答。

https://blog.csdn.net/yin262/article/details/46730261

很经典的介绍
https://www.cnblogs.com/acool/p/6852250.html

https://www.kernel.org/doc/Documentation/cgroup-v1/blkio-controller.txt
-->

{% highlight text %}
{% endhighlight %}
