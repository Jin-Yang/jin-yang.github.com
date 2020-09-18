---
Date: October 19, 2013
title: Linux 网卡驱动的工作原理
layout: post
comments: true
category: [linux, network]
language: chinese
---

如题所示，介绍网卡是如何接收数据，然后又是如何交给上层处理。因为鄙人的网卡驱动用的是 e1000e，所以就以此为例了。

Just enjoy it.

<!-- more -->

网络设备有很多，但它们的工作原理基本相同，可以分为两个层次，分别为 MAC (Media Access Control) 对应于 OSI 的数据链路层，PHY (Physical Layer) 对应于物理层。

同时为了减小 CPU 的压力，底层很多采用 DMA 方式。


# 查看驱动

对于网卡驱动，通常是以内核模块的方式提供，本机网卡对应的内核驱动可以通过如下方式查看。

{% highlight text %}
----- 可以查看Ethernet、Wireless字样，以及Kernel driver in use:（也就是所使用的驱动)
# lspci -vvv | less
... ...
00:19.0 Ethernet controller: Intel Corporation Ethernet Connection I218-LM (rev 04)
        Subsystem: Dell Device 05cb
        Control: I/O+ Mem+ BusMaster+ SpecCycle- MemWINV- VGASnoop- ParErr- Stepping- SERR- FastB2B- DisINTx+
        Status: Cap+ 66MHz- UDF- FastB2B- ParErr- DEVSEL=fast >TAbort- <TAbort- <MAbort- >SERR- <PERR- INTx-
        Latency: 0
        Interrupt: pin A routed to IRQ 46
        Region 0: Memory at f7e00000 (32-bit, non-prefetchable) [size=128K]
        Region 1: Memory at f7e3c000 (32-bit, non-prefetchable) [size=4K]
        Region 2: I/O ports at f080 [size=32]
        Capabilities: [c8] Power Management version 2
                Flags: PMEClk- DSI+ D1- D2- AuxCurrent=0mA PME(D0+,D1-,D2-,D3hot+,D3cold+)
                Status: D0 NoSoftRst- PME-Enable- DSel=0 DScale=1 PME-
        Capabilities: [d0] MSI: Enable+ Count=1/1 Maskable- 64bit+
                Address: 00000000fee0f00c  Data: 41a4
        Capabilities: [e0] PCI Advanced Features
                AFCap: TP+ FLR+
                AFCtrl: FLR-
                AFStatus: TP-
        Kernel driver in use: e1000e
... ...

----- 查看驱动相关的信息
# modinfo e1000e
filename:       .../kernel/drivers/net/ethernet/intel/e1000e/e1000e.ko
version:        3.2.5-k
license:        GPL
description:    Intel(R) PRO/1000 Network Driver
author:         Intel Corporation, <linux.nics@intel.com>
... ...

----- 查看对应的中断信息
$ cat /proc/interrupts
           CPU0       CPU1       CPU2       CPU3
... ...
 46:        853          0        930          0   PCI-MSI-edge      eth0
... ...
{% endhighlight %}

如上可以看到硬件的信息，以及所使用的驱动信息。


# 源码解析

接下来查看下 e1000e 相关的驱动。

## 网络初始化

网络设备相关的驱动在内核的 drivers/net 目录下，如上的 e1000e 驱动在 net/ethernet/intel/e1000e 目录下，接下来我们就以此为例。

首先是模块初始化函数，也就是 module_init() 宏指定的相关函数。
{% highlight c %}
char e1000e_driver_name[] = "e1000e";
static struct pci_driver e1000_driver = {
    .name     = e1000e_driver_name,             // 驱动名称
    .id_table = e1000_pci_tbl,             // 指定了那些设备可以使用该驱动
    .probe    = e1000_probe,
    .remove   = e1000_remove,
    .driver   = {
        .pm = &e1000_pm_ops,
    },
    .shutdown = e1000_shutdown,
    .err_handler = &e1000_err_handler
};

static int __init e1000_init_module(void)
{
    int ret;

    pr_info("Intel(R) PRO/1000 Network Driver - %s\n",
        e1000e_driver_version);
    pr_info("Copyright(c) 1999 - 2014 Intel Corporation.\n");
    ret = pci_register_driver(&e1000_driver);

    return ret;
}
module_init(e1000_init_module);
{% endhighlight %}
如上，实际的初始化，只是通过 pci_register_driver() 注册的 PCI 驱动。当然其它的驱动也类似，同样会在加载的时候都会通过该函数注册，然后，在该函数中会通过驱动对应的 probe() 函数探测设备，如果找到则加载驱动，注册添加一个网络设备。

现在已经定义一个 PCI 驱动，但是哪些 PCI 设备可以使用此驱动？ 实际上与 e1000_driver.id_table 中的定义相关，对于 e1000e 也就是通过 struct pci_device_id e1000_pci_tbl[] 定义。

接下来看看是如何匹配的。

首先介绍下 PCI 的相关内容，PCI 有 3 种地址空间：IO 空间、内存地址空间、配置空间。一个 PCI 配置空间至少有 256 字节，如下：

![pci-config]{: .pull-center}

id_table 是 struct pci_device_id 类型的一个数组，每个元素就对应一条使用的 PCI 硬件信息，如果符合就可以使用这个驱动，例如我的网卡，其类型为：

{% highlight text %}
pci -nn | grep -E 'Ethernet controller.*Intel'
00:19.0 Ethernet controller [0200]: Intel Corporation Ethernet Connection I218-LM [8086:155a] (rev 04)
{% endhighlight %}

刚好可以匹配上如下的记录。
{% highlight c %}
#define E1000_DEV_ID_PCH_LPTLP_I218_LM      0x155A

static const struct pci_device_id e1000_pci_tbl[] = {
    ... ...
    { PCI_VDEVICE(INTEL, E1000_DEV_ID_PCH_LPTLP_I218_LM), board_pch_lpt },
    ... ...
};
{% endhighlight %}

在 pci_driver 中的 probe 成员（也就是 e1000_probe() 函数）用来对设置进行一系列的初始化操作。在驱动和设备的初始化阶段，包括在总线上驱动注册初始化；驱动探测设备注册初始化；开启设备，初始化接收缓存。

网络设备通过 struct net_device 标示，包括硬件网络设备接口，如以太网；软件网络设备接口，如 loopback 。通过 dev_base 头指针将设备链接起来集体管理，每个节点代表一个网络设备接口。

另外，也个比较重要的是中断相关内容，也就是在  e1000_open() 函数中调用 e1000_request_irq() 函数。

{% highlight c %}
static int e1000_request_irq(struct e1000_adapter *adapter)
{
    ... ...
    /* E1000E_INT_MODE_MSIX模式
     * # cat /proc/interrupts | grep eth0
     *   86          0          0          0  IR-PCI-MSI-edge      eth0-rx-0
     *    0          0          0          0  IR-PCI-MSI-edge      eth0-tx-0
     *   10          0          0          0  IR-PCI-MSI-edge      eth0
     */
    if (adapter->flags & FLAG_MSI_ENABLED) { // 如果是MSI
        err = request_irq(adapter->pdev->irq, e1000_intr_msi, 0,
                  netdev->name, netdev);
        if (!err)
            return err;

        /* fall back to legacy interrupt */
        e1000e_reset_interrupt_capability(adapter);
        adapter->int_mode = E1000E_INT_MODE_LEGACY;
    }

    /* E1000E_INT_MODE_MSI模式
     * # cat /proc/interrupts | grep eth0
     * 853          0        930          0   PCI-MSI-edge      eth0
     */
    err = request_irq(adapter->pdev->irq, e1000_intr, IRQF_SHARED,
              netdev->name, netdev);
    ... ...
}
{% endhighlight %}

如上，在注册完设备之后，会通过 request_irq() 申请中断，不同的网卡类型会注册不同的中断处理函数，下面以 e1000_intr() 为例。


## NAPI (New API)

报文接收是整个协议栈的入口，负责从网卡中把报文接收并送往内核协议栈相应协议处理模块处理。一般有中断和轮询两种方式，最开始，网络流量小，采用中断方式。

当流量增大时，有此引起的中断将会影响到系统的整体效率。例如，我们使用标准的 100M 网卡，可能实际达到的接收速率为 80MBits/s，而此时数据包平均长度为 1500Bytes，则每秒产生的中断数目为：

{% highlight text %}
80M bits/s / (8 Bits/Byte * 1500 Byte) = 6667 个中断 /s
{% endhighlight %}

当每秒数千个中断请求时，那么很大一部分时间会消耗在中断上下文中；因此，当流量较大时，最好采用轮询的方式。而轮询，在请求较少时仍需要定时捞取数据，从而也会导致资源浪费。

而 NAPI 就是为了解决此问题。???????


除了中断次数之外，当系统压力很大，不得不丢数据报文时，最好的方式是在最底层直接丢弃。采用 NAPI 后，可以直接在网络驱动中丢弃，而不需要再通知到内核。

在网卡中断中，首先处理的是关闭中断，因为我们已经知道了现在有很多的报文需要去处理，关闭中断，从而防止其它的中断请求再次到来。接着就是通知 Network Subsystem 来处理现在已经接收的报文。


### 结构体
{% highlight c %}
//----- 每个CPU会分配一个结构体
DEFINE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);

struct softnet_data {                         // 保存接收报文，每个CPU一个队列
    struct Qdisc        *output_queue;        // 输出帧的控制
    struct Qdisc        **output_queue_tailp;
    struct list_head    poll_list;            // 有输入帧待处理的设备链表
    struct sk_buff      *completion_queue;    // 已经成功被传递出的帧的链表
    struct sk_buff_head process_queue;

    unsigned int        dropped;
    struct sk_buff_head input_pkt_queue;      // 接收到数据会分配一个skb，并保存在该链表中，注意只针对非NAPI
                                              // 而NAPI有自己的私有队列
    struct napi_struct  backlog;              // 用来兼容非NAPI的驱动
};

struct napi_struct {
    struct list_head    poll_list;            // 等待被执行的设备链表，链表的头就是softnet_data.poll_list
    unsigned long       state;
    int                 weight;               // 标示设备的权重
    unsigned int        gro_count;
    int (*poll)(struct napi_struct *, int);   // NAPI都有poll虚函数，而非NAPI没有，初始化会赋值process_backlog
    struct net_device   *dev;                 // 指向具体的网络设备
    struct sk_buff      *gro_list;
    struct sk_buff      *skb;
    struct list_head    dev_list;             // 指向设备的NAPI链表
    struct hlist_node   napi_hash_node;
    unsigned int        napi_id;
};
{% endhighlight %}
上述结构体中有 Qdisc (Queueing Discipline)，即排队规则，也就是我们经常说的 QoS 。


### 与设备关联

在网卡驱动中，同时会创建轮询函数，一般在网卡初始化的时候完成，通过 nefif_napi_add() 函数添加。

{% highlight c %}
void netif_napi_add(struct net_device *dev, struct napi_struct *napi,
            int (*poll)(struct napi_struct *, int), int weight);
{% endhighlight %}

也就是用于将轮询函数与实际的网络设备 struct net_device 关联起来。上述函数的入参中包括了一个权重，该值通常是一个经验数据，一般 10Mb 的网卡设置为 16，而更快的网卡则设置为 64 。


### 通知软中断
通知是通过 napi_schedule() 函数进行，有如下的两种方式；其中 napi_schedule_prep() 是为了判定现在是否已经进入了轮询模式。

{% highlight c %}
//----- 将网卡预定为轮询模式
void napi_schedule(struct napi_struct *n);

//----- 或者
if (napi_schedule_prep(n))        // 返回0表示已经在做poll操作了
    __napi_schedule(n);
{% endhighlight %}

接下来查看下源码的具体实现。
{% highlight c %}
static inline bool napi_schedule_prep(struct napi_struct *n)
{
    return !napi_disable_pending(n) &&
        !test_and_set_bit(NAPI_STATE_SCHED, &n->state);
}
static inline void napi_schedule(struct napi_struct *n)
{
    if (napi_schedule_prep(n))
        __napi_schedule(n);
}
static inline void ____napi_schedule(struct softnet_data *sd,
                     struct napi_struct *napi)
{
    // 把自己挂到per cpu的softnet_data上，触发NET_RX_SOFTIRQ软中断
    list_add_tail(&napi->poll_list, &sd->poll_list);
    __raise_softirq_irqoff(NET_RX_SOFTIRQ);
}
void __napi_schedule(struct napi_struct *n)
{
    unsigned long flags;

    local_irq_save(flags);
    ____napi_schedule(this_cpu_ptr(&softnet_data), n);
    local_irq_restore(flags);
}
{% endhighlight %}
可以看到 napi_schedule() 基本操作是添加到链表中，然后触发软中断。

### 软中断

软中断包括了读写中断，均在 net_dev_init() 函数中初始化。
{% highlight c %}
static int __init net_dev_init(void)
{
    ... ...
    open_softirq(NET_TX_SOFTIRQ, net_tx_action);
    open_softirq(NET_RX_SOFTIRQ, net_rx_action);
    ... ...
}
subsys_initcall(net_dev_init);
{% endhighlight %}


### 轮询函数

驱动中创建轮询函数，它的工作是从网卡获取数据包并将其送入到网络子系统，函数声明如下。

{% highlight c %}
int (*poll)(struct napi_struct *napi, int weight);
{% endhighlight %}

该函数在将网卡切换为轮询模式之后，用 poll() 方法处理接收队列中的数据包，如队列为空，则重新切换为中断模式。在切换回中断模式前，需要先通过 netif_rx_completer() 关闭轮询模式，然后开启网卡接收中断。

{% highlight c %}
//----- 退出轮询模式
void __napi_complete(struct napi_struct *n)
{
    list_del(&n->poll_list);
    smp_mb__before_atomic();
    clear_bit(NAPI_STATE_SCHED, &n->state);
}
void napi_complete(struct napi_struct *n)
{
    unsigned long flags;

    /*
     * don't let napi dequeue from the cpu poll list
     * just in case its running on a different cpu
     */
    if (unlikely(test_bit(NAPI_STATE_NPSVC, &n->state)))
        return;

    napi_gro_flush(n, false);
    local_irq_save(flags);
    __napi_complete(n);
    local_irq_restore(flags);
}
{% endhighlight %}

关于驱动需要做的修改，可以参考 [Driver porting: Network drivers](http://lwn.net/Articles/30107/)，这篇是 2.6 时的文章，稍微有点老。

## 接收报文

在 e1000e 驱动中，只有 NAPI，也就是在收到中断后，然后调用 NAPI 轮询。

{% highlight c %}
static irqreturn_t e1000_intr(int __always_unused irq, void *data)
{
    ... ...
    if (napi_schedule_prep(&adapter->napi)) {
        adapter->total_tx_bytes = 0;
        adapter->total_tx_packets = 0;
        adapter->total_rx_bytes = 0;
        adapter->total_rx_packets = 0;
        __napi_schedule(&adapter->napi);
    }

    return IRQ_HANDLED;
}

static int e1000e_poll(struct napi_struct *napi, int weight)
{
    ... ...
    if (!adapter->msix_entries ||
        (adapter->rx_ring->ims_val & adapter->tx_ring->ims_val))
        tx_cleaned = e1000_clean_tx_irq(adapter->tx_ring);        // 查看并回收tx slot

    adapter->clean_rx(adapter->rx_ring, &work_done, weight);

    if (!tx_cleaned)
        work_done = weight;

    /* If weight not fully consumed, exit the polling mode */
    if (work_done < weight) {
        if (adapter->itr_setting & 3)
            e1000_set_itr(adapter);
        napi_complete(napi);
        if (!test_bit(__E1000_DOWN, &adapter->state)) {
            if (adapter->msix_entries)
                ew32(IMS, adapter->rx_ring->ims_val);
            else
                e1000_irq_enable(adapter);
        }
    }

    return work_done;
}

{% endhighlight %}



对于接收过程，我们仅大致疏理一下其执行过程。
{% highlight text %}
napi_gro_receive()
 |-skb_gro_reset_offset()
 |-dev_gro_receive()
 |-napi_skb_finish()
   |-netif_receive_skb_internal()
     |-__netif_receive_skb()            # 进入上层的接收函数
{% endhighlight %}


## rx_ring索引





所以，next_to_clean指示的位置，一定要next_to_use先初始化过。

空 代表没有分配内存

0  代表分配了内存但是没有数据

1  代表数据准备好


接收到的数据通过 struct e1000_ring 结构体保存，其初始化在 e1000_sw_init() 中设置。
{% highlight c %}
struct e1000_ring {
    struct e1000_adapter *adapter;  /* back pointer to adapter */
    void *desc;         /* pointer to ring memory  */
    dma_addr_t dma;         /* phys address of ring    */
    unsigned int size;      /* length of ring in bytes */
    unsigned int count;     /* number of desc. in ring */

    u16 next_to_use;               // 内存分配完毕，网卡可以将数据写入其描述的最后下标
    u16 next_to_clean;             // 网卡已经将数据写入后，驱动从描述符中将数据取出的开始下标

    void __iomem *head;            // 头尾指针，为闭区间索引，假设slot个数为16
    void __iomem *tail;

    /* array of buffer information structs */
    struct e1000_buffer *buffer_info;

    char name[IFNAMSIZ + 5];
    u32 ims_val;
    u32 itr_val;
    void __iomem *itr_register;
    int set_itr;

    struct sk_buff *rx_skb_top;
};
{% endhighlight %}

{% highlight c %}
{% endhighlight %}

在 ISR 中通常会通过 netif_rx() 传递给上层。另外，内核采用 NAPI 机制来处理高流量时的并发，减小由于中断引起的负载。

在无线设备中 beacon 往往通过 tasklet 实现，将其传递给软中断处理，当然这也意味着会通过 tasklet_init 类似的函数进行初始化，由此可以查找到具体的处理函数。


[e1000e-driver-analysis]:    "http://mnstory.net/2014/12/e1000e-driver-source-analysis/"     "e1000e 驱动源码分析"

[pci-config]:                /images/linux/pci-config.png                                    "PCI 配置空间格式"



<!--
http://www.linuxfoundation.org/collaborate/workgroups/networking/napi
http://blog.chinaunix.net/uid-20786208-id-3420546.html
http://blog.chinaunix.net/uid-10915175-id-3367864.html
http://www.atatech.org/articles/51019


http://mnstory.net/2014/12/e1000e-driver-source-analysis/       e1000e 驱动源码分析
OR reference/linux/network/e1000e-driver-source-analysis.pdf
http://www.ibm.com/developerworks/cn/linux/l-cn-network-pt/     对GRO、GSO的介绍
-->
