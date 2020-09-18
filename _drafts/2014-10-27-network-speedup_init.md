---
Date: October 19, 2013
title: Linux 网络加速
layout: post
comments: true
language: chinese
category: [linux, network]
---

硬件与 CPU 进行信息沟通有两种方式，一种是中断，另一种是轮询。中断是硬件主动产生中断信号，中断控制器将信号传递给 CPU，此时 CPU 会停下手中的工作，执行中断任务；轮询则是 CPU 主动，定时查询硬件设备的状态，是否处理硬件请求。

随着网络带宽越来越大，实际由中断带来的 CPU 负载越来越大，这也就导致了 NAPI 的出现。

不过这不是本文要讲的重点，本文着重看下网络针对 SMP 所做的优化。

<!-- more -->


# 简介

内核的加速方式有几种，包括了网卡硬件加速、多队列、对 SMP 的支持等。

对于针对 SMP 的优化后面介绍，首先介绍通过网卡硬件方式的加速方法，包括了 LSO、LRO、GSO、GRO、TSO、USO，可以通过如下的方式查看网卡是否支持，其中 fixed 表示不能修改，其它可以通过 -K 参数修改：


{% highlight text %}
# ethtool -k eth0                           # 查看网口的offload特性
Offload parameters for eth0:
rx-checksumming: on
tx-checksumming: on
        tx-checksum-ipv4: off [fixed]
        tx-checksum-ip-generic: on
        tx-checksum-ipv6: off [fixed]
        tx-checksum-fcoe-crc: off [fixed]
        tx-checksum-sctp: off [fixed]
scatter-gather: on
tcp-segmentation-offload: on
        tx-tcp-segmentation: on
        tx-tcp-ecn-segmentation: off [fixed]
        tx-tcp6-segmentation: on
udp-fragmentation-offload: off [fixed]
generic-segmentation-offload: on
generic-receive-offload: on
large-receive-offload: off [fixed]
rx-vlan-offload: on
tx-vlan-offload: on
ntuple-filters: off [fixed]
receive-hashing: on
highdma: on [fixed]
rx-vlan-filter: off [fixed]
vlan-challenged: off [fixed]
tx-lockless: off [fixed]
netns-local: off [fixed]
tx-gso-robust: off [fixed]
tx-fcoe-segmentation: off [fixed]
tx-gre-segmentation: off [fixed]
tx-ipip-segmentation: off [fixed]
tx-sit-segmentation: off [fixed]
tx-udp_tnl-segmentation: off [fixed]
tx-mpls-segmentation: off [fixed]
fcoe-mtu: off [fixed]
tx-nocache-copy: off
loopback: off [fixed]
rx-fcs: off
rx-all: off
tx-vlan-stag-hw-insert: off [fixed]
rx-vlan-stag-hw-parse: off [fixed]
rx-vlan-stag-filter: off [fixed]
busy-poll: off [fixed]
{% endhighlight %}

其中 offload 的作用就是将一个本来由软件实现的功能现在放到硬件上来实现，也就是说这些 offload 特性都是为了提升网络收/发性能，其中 TSO、UFO 和 GSO 是对应网络发送，在接收方向上对应的是 LRO、GRO 。

当数据包传输时，需要按照标准分割成一个一个小于 MTU 的小包进行传输，然后传输到另一边后还得将这些数据包拼装回去。本来这些分割、拼装的事情都是软件实现的，于是就有人想将这些事情 offload 到硬件上实现，于是就产生了如下的技术。






### TSO (TCP Segmentation Offload)、USO (UDP Fragmentation Offload)、LSO (Large Segment Offload)

一种利用网卡对 TCP 数据包分片，减轻 CPU 负载的一种技术，有时也被叫做 LSO，TSO 是针对 TCP 报文的，UFO 是针对 UDP 报文的。如果硬件支持 TSO 功能，同时也需要硬件支持的 TCP 校验计算和 Scatter Gather 功能。


### GSO (Generic Segmentation Offload)

比 TSO 更通用，基本思想就是尽可能的推迟数据分片直至发送到网卡驱动之前，此时会检查网卡是否支持分片功能（如TSO、UFO）, 如果支持直接发送到网卡，如果不支持就进行分片后再发往网卡。这样大数据包只需走一次协议栈，而不是被分割成几个数据包分别走，这就提高了效率。


### LRO (Large Receive Offload)

将接收到的多个 TCP 数据聚合成一个大的数据包，然后传递给网络协议栈处理，以减少上层协议栈处理开销，提高系统接收 TCP 数据包的能力。

### GRO(Generic Receive Offload)

基本思想跟 LRO 类似，克服了 LRO 的一些缺点，更通用。



### Scatter Gather

这种方式是与传统的 block DMA 相对应的一种 DMA 方式，对应的 dev features 为 NETIF_F_SG 。

在 DMA 传输数据的过程中，要求源物理地址和目标物理地址必须是连续的，如果物理地址不连续，那么在传输完一块连续的物理内存后，会发起一次中断，然后进行下一块连续物理内存的数据传输，这种方式为 block DMA 方式。

Scatter/Gather 是通过一个链表描述物理不连续的存储器，然后把链表首地址告诉 DMA master，每次传输完一块物理连续的数据后，不用再发中断了，而是根据链表传输下一块物理连续的数据，最后发起一次中断。
<!--
struct scatterlist {
    unsigned long   page_link;
    unsigned int    offset;
    unsigned int    length;
    dma_addr_t  dma_address;
};
-->




# SMP 优化

目前在内核中采用了多种技术提高多处理器系统的并行性并改善性能：

* RSS: Receive Side Scaling (接收侧的缩放)
* RPS: Receive Packet Steering (接收端包的控制)
* RFS: Receive Flow Steering (接收端流的控制)
* Accelerated Receive Flow Steering (加速的接收端流的控制)
* XPS: Transmit Packet Steering(发送端包的控制)

## RSS (Receive Side Scaling)

是一项网卡的新特性，也就是多队列，具备 RSS 功能的网卡，可以将不同的网络流分成不同的队列，再分别将这些队列分配到多个 CPU 核心上进行处理，从而将负荷分散，充分利用多核处理器的能力。


![rss-struct]{: .pull-center}

通常的处理流程如下：

1. NIC 通过指定的区域（可以不连续，通常为源IP、目的IP、源Port、目的Port）计算一个 hash 值；
2. 通常选择 hash 的低位 (LSB) 作为索引，在 indirection table 中对应的就是 CPU 序号，而这些值是可以进行配置的；
3. 通过 MSI，网卡的中断也可以和相应的 CPU 绑定。

对于 RSS 特性需要硬件的支持：A) 网卡需要支持 RSS 和 MSI-X，可以参考相应网卡的产品文档，一般都会有 RSS 或者 multiple queue；B) 内核需要支持 MSI/MSI-X 。

当网卡驱动加载时，获取网卡型号得到网卡的硬件 queue 数量，并结合 CPU 核的数量，选择两者的最小值作为所要激活的网卡 queue 数量，并申请相应数目的中断号，分配给激活的各个 queue。

当某个 queue 收到报文时，触发相应的中断，收到中断的 CPU 将该任务加入到协议栈负责收包的该核的 NET_RX_SOFTIRQ 队列中 (NET_RX_SOFTIRQ 在每个核上都有一个实例)，在 NET_RX_SOFTIRQ 中，会调用 NAPI 的收包接口，将报文收到有多个 netdev_queue 的 net_device 数据结构中。

可以通过如下方式查看内核中的相关信息：

{% highlight text %}
----- 查看当前网卡 eth0 注册的多队列数目
# grep -iE "msi.*eth0" /proc/interrupts

----- 通过lspci查看网卡相关信息
# lspci -vvvv | less
... ...
01:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
        ... ...
        Capabilities: [50] MSI: Enable- Count=1/1 Maskable+ 64bit+
                Address: 0000000000000000  Data: 0000
                Masking: 00000000  Pending: 00000000
        Capabilities: [70] MSI-X: Enable+ Count=10 Masked-
                Vector table: BAR=3 offset=00000000
                PBA: BAR=3 offset=00002000
        ... ...
        Kernel driver in use: igb
        Kernel modules: igb

----- 通过modinfo查看内核加载信息
# modinfo igb | grep  -E "QueuePairs|RSS"
parm: RSS:Number of Receive-Side Scaling Descriptor Queues (0-8), default 0, 0=number of cpus (array of int)
parm: QueuePairs:Enable Tx/Rx queue pairs for interrupt handling (0,1), default 1=on (array of int)
{% endhighlight %}

可以在载入模块时指定参数，如 modprobe igb RSS=8，或者直接在配置文件中指定：

{% highlight text %}
$ cat /etc/modprobe.conf
options igb RSS=8,8
{% endhighlight %}

下图是网卡的处理流程，可供参考。

![rss-process]{: .pull-center width="700"}


另外，网卡绑定时，最好和同一个物理 CPU 的核挨个绑定，从而可以避免 L1、L2、L3 践踏，那些 CPU 属于同一个物理核可以通过 lscpu 查看。









## RPS (Receive Packet Steering)

也就是接收端包的控制，实际上类似于 RSS，只是 RPS 是在软件层面实现的，它将软中断均摊到多个 CPU core 上，相比来说也更加灵活。

其中 RPS/RFS 是 Google 的 Tom Herbert 向 Linux 内核提交的一个 patch 。没有 RPS 时，一个网卡队列，硬中断和软中断都是相同的 CPU 在处理，而 RPS 可以将软中断均摊到多个 CPU 上。

之所以有 RPS/RFS 是因为：

1. 如果网卡不支持 RSS+MSI-X，那么只能使用一个 CPU core 处理中断，此时性能较差。

2. 网卡中队列的选择，很多仍然是基于 4 元组 hash 出来的，如果两台机器间的流量较高，那么响应的队列就非常忙。有些 Intel 的网卡可以支持 [Flow Director][intel-flow-director] 的，可以提供更加精细的调优。

3. RSS 虽然可以使同一个连接的流量都走相同的队列，减少排序，但处理中断的 CPU 和上层应用的 CPU 仍有可能不是同一个，从而会造成额外的开销。


内核中的实现是在收包的链路 netif_receive_skb_internal() 函数中，存在一个函数 get_rps_cpu()，根据 skb，找到该哪个 CPU 处理。简单得说，是建立一个 hash 表（一般使用 Toeplitz hash 函数），那个连接的流量应该在哪个 CPU 上处理，避免相同连接的流量被不同的 CPU 处理。

{% highlight c %}
static int netif_receive_skb_internal(struct sk_buff *skb)
{
    ... ...
    if (static_key_false(&rps_needed)) {
        ... ...
        cpu = get_rps_cpu(skb->dev, skb, &rflow);

        if (cpu >= 0) {  // 如果没有开启RPS，CPU返回-1，走原来的流程
            ret = enqueue_to_backlog(skb, cpu, &rflow->last_qtail);
            rcu_read_unlock();
            return ret;
        }
        rcu_read_unlock();
    }
    return __netif_receive_skb(skb);
}
{% endhighlight %}

在调整时需要对每块网卡每个队列分别进行设置，仍以 eth0 的 0 号队列进行设置：

{% highlight text %}
# echo ff > /sys/class/net/eth0/queues/rx-0/rps_cpus
{% endhighlight %}

RPS 的设置方式和中断亲和力设置的方法类似，采用的是掩码的方式，但通常要将所有的 CPU 设置进去。



## RFS (Receive Flow Steering)

主要是解决网卡数据流 CPU 与接受该数据流的应用程序所在 CPU 不同，导致 CPU 切换，RFS 就是为了保证程序运行的 CPU 和软中断处理 CPU 相同，从而提升 CPU 缓存命中率，降低网络延迟。

要开启 RFS 需要修改两个参数：

{% highlight text %}
/proc/sys/net/core/rps_sock_flow_entries
/sys/class/net/${DEV}/queues/rx-${NUM}/rps_flow_cnt
{% endhighlight %}

前者为期望获得的最大并发连接数，该值必须为 2 的幂，如果不是，系统设置为向上最接近的 2 的幂；后者的值为 rps_sock_flow_entries/N，N 表示设备接收队列的数量，对于单队列两者设置为同一值。






## XPS (Transmit Packet Steering)

同样是 Tom Herbert 提交的 patch，主要是针对多队列的网卡发送时的优化，对于单队列几乎没有效果。其中 RFS 是根据包接收的队列来选择 CPU，而 XPS 是根据 CPU 来选择要发送的网卡队列。

CPU MAP 通过如下的方式设置，这里的 xps_cpus 是一个 CPU 的掩码，表示当前队列对应的 CPU。

{% highlight text %}
/sys/class/net/${DEV}/queues/tx-${NUM}/xps_cpus
{% endhighlight %}

XPS 就是提高多对列下的数据包发送吞吐量，具体来说就是提高了发送的局部性。



原理很简单，就是根据当前skb对应的hash值(如果当前socket有hash，那么就使用当前socket的)来散列到xps_cpus这个掩码所设置的cpu上，也就是cpu和队列是一个1对1，或者1对多的关系，这样一个队列只可能对应一个cpu，从而提高了传输结构的局部性。

没有xps之前的做法是这样的，当前的cpu根据一个skb的4元组hash来选择队列发送数据，也就是说cpu和队列是一个多对多的关系，而这样自然就会导致传输结构的cache line bouncing。

这里还有一个引起cache line bouncing的原因，不过这段看不太懂：

点击(此处)折叠或打开

    Also when sending from one CPU to a queue whose
    transmit interrupt is on a CPU in another cache domain cause more
    cache line bouncing with transmit completion.

接下来来看代码，我这里看得代码是net-next分支，这个分支已经将xps合并进去了。

先来看相关的数据结构,首先是xps_map,这个数据结构保存了对应的cpu掩码对应的发送队列，其中queues队列就保存了发送对列.这里一个xps_map有可能会映射到多个队列。

点击(此处)折叠或打开

    struct xps_map {
        //队列长度
        unsigned int len;
        unsigned int alloc_len;
        struct rcu_head rcu;
        //对应的队列序列号数组
        u16 queues[0];
    };

而下面这个结构保存了设备的所有的cpu map，比如一个设备 16个队列，然后这里这个设备的xps_dev_maps就会保存这16个队列的xps map(sysctl中设置的xps_map),而每个就是一个xps_map结构。

点击(此处)折叠或打开

    struct xps_dev_maps {
        //rcu锁
        struct rcu_head rcu;
        //所有对列的cpu map数组
        struct xps_map __rcu *cpu_map[0];
    };

然后就是net_device结构增加了一个xps_dev_maps的域来保存这个设备所有的cpu map。

点击(此处)折叠或打开

    struct net_device {
        ................................
    #ifdef CONFIG_XPS
        //保存当前设备的所有xps map.
        struct xps_dev_maps __rcu *xps_maps;
    #endif
        ..........................
    }

内核发送数据包从 IP 层到驱动层会调用 dev_queue_xmit()，该函数会调用 dev_pick_tx() 选择一个队列。

先来分析下这个函数的主要流程，首先，如果设备只有一个队列，那么就选择这唯一的队列。

点击(此处)折叠或打开

    if (dev->real_num_tx_queues == 1)
            queue_index = 0;

然后如果设备设置了回调函数ndo_select_queue，则调用ndo_select_queue来选择队列号，这里要注意，当编写驱动时，如果设置了回调函数ndo_select_queue，此时如果需要xps特性，则最好通过get_xps_queue来取得队列号。

点击(此处)折叠或打开

    else if (ops->ndo_select_queue) {
            queue_index = ops->ndo_select_queue(dev, skb);
            queue_index = dev_cap_txqueue(dev, queue_index);

然后进入主要的处理流程，首先从skb从属的sk中取得缓存的队列索引，如果有缓存，则直接返回这个索引，否则开始计算索引，这里就通过调用xps patch最重要的一个函数get_xps_queue来计算queue_index.

点击(此处)折叠或打开

    static struct netdev_queue *dev_pick_tx(struct net_device *dev,
                        struct sk_buff *skb)
    {
    ....................................
    else {
            struct sock *sk = skb->sk;
            queue_index = sk_tx_queue_get(sk);

            if (queue_index < 0 || skb->ooo_okay ||
                queue_index >= dev->real_num_tx_queues) {
                int old_index = queue_index;
                //开始计算队列索引
                queue_index = get_xps_queue(dev, skb);
                if (queue_index < 0)
                    //调用老的计算方法来计算queue index.
                    queue_index = skb_tx_hash(dev, skb);
                    ......................................................
            }
        }
        //存储队列索引
        skb_set_queue_mapping(skb, queue_index);
        //返回对应的queue
        return netdev_get_tx_queue(dev, queue_index);
    }

接下来我们来看get_xps_queue，这个函数是这个patch的核心，它的流程也很简单，就是通过当前的cpu id获得对应的xps_maps,然后如果当前的cpu和队列是1:1对应则返回对应的队列id，否则计算skb的hash值，根据这个hash来得到在xps_maps 中的queue的位置，从而返回queue id.




{% highlight c %}
static inline int get_xps_queue(struct net_device *dev, struct sk_buff *skb)
{
    struct xps_dev_maps *dev_maps;
    struct xps_map *map;
    int queue_index = -1;

    rcu_read_lock();
    dev_maps = rcu_dereference(dev->xps_maps);
    if (dev_maps) {
        // 根据cpu id得到当前cpu对应的队列集合
        map = rcu_dereference(
            dev_maps->cpu_map[raw_smp_processor_id()]);
        if (map) {
            if (map->len == 1)       // 如果队列集合长度为1，则说明是1:1对应
                queue_index = map->queues[0];
            else
                queue_index = map->queues[reciprocal_scale(skb_get_hash(skb),
                                       map->len)];
            if (unlikely(queue_index >= dev->real_num_tx_queues))
                queue_index = -1;
        }
    }
    rcu_read_unlock();

    return queue_index;
}
{% endhighlight %}



    static inline int get_xps_queue(struct net_device *dev, struct sk_buff *skb)
    {
    #ifdef CONFIG_XPS
        struct xps_dev_maps *dev_maps;
        struct xps_map *map;
        int queue_index = -1;

        rcu_read_lock();
        dev_maps = rcu_dereference(dev->xps_maps);
        if (dev_maps) {
            //根据cpu id得到当前cpu对应的队列集合
            map = rcu_dereference(
                dev_maps->cpu_map[raw_smp_processor_id()]);
            if (map) {
                //如果队列集合长度为1，则说明是1:1对应
                if (map->len == 1)
                    queue_index = map->queues[0];
                else {
                    //否则开始计算hash值，接下来和老的计算hash方法一致。
                    u32 hash;
                    //如果sk_hash存在，则取得sk_hash(这个hash，在我们rps和rfs的时候计算过的,也就是四元组的hash值)
                    if (skb->sk && skb->sk->sk_hash)
                        hash = skb->sk->sk_hash;
                    else
                        //否则开始重新计算
                        hash = (__force u16) skb->protocol ^
                            skb->rxhash;
                    hash = jhash_1word(hash, hashrnd);
                    //根据hash值来选择对应的队列
                    queue_index = map->queues[
                        ((u64)hash * map->len) >> 32];
                }
                if (unlikely(queue_index >= dev->real_num_tx_queues))
                    queue_index = -1;
            }
        }
        rcu_read_unlock();

        return queue_index;
    #else
        return -1;
    #endif
    }




# 参考

相关内容可以参考内核文档 [Scaling in the Linux Networking Stack][network-scaling] ，实际上也就是内核中的 Documentation/networking/scaling.txt 。



[network-scaling]:       https://www.kernel.org/doc/Documentation/networking/scaling.txt      "关于网络针对SMP的优化"
[intel-flow-director]:   http://www.intel.com/content/www/us/en/ethernet-products/ethernet-flow-director-video.html      "关于 Intel 的 Flow Director 介绍视频"
[xps-mailist]:           http://lwn.net/Articles/412062/                                      "Tom Herbert 提交的 XPS patch"
[rps-mailist]:           http://lwn.net/Articles/328339/                                      "Tom Herbert 提交的 RPS patch"
[rfs-mailist]:           http://lwn.net/Articles/381955/                                      "Tom Herbert 提交的 RFS patch"


[rss-struct]:            /images/linux/network-rss.png
[rss-process]:           /images/linux/network-speedup-rss.png


<!--
http://blog.yufeng.info/archives/2037
-->
