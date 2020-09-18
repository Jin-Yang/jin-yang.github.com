---
Date: October 19, 2013
title: Linux 中的 loopback 设备
layout: post
comments: true
language: chinese
category: [linux,network]
---

我们知道，在 Linux 设备中有一个 lo 设备，在此稍微介绍下。

<!-- more -->

# 简介

对于 loopback 设备，我们可以直接通过 ifconfig 查看 lo 设备的配置。

{% highlight text %}
$ ifconfig lo
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
    inet 127.0.0.1  netmask 255.0.0.0
    inet6 ::1  prefixlen 128  scopeid 0x10<host>
    loop  txqueuelen 0  (Local Loopback)
    RX packets 442824  bytes 208834219 (199.1 MiB)
    RX errors 0  dropped 0  overruns 0  frame 0
    TX packets 442824  bytes 208834219 (199.1 MiB)
    TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
{% endhighlight %}

如上，可以看出 lo 的 mask 仅为 8bit，也就是说，只要 127 开头，任何网络主机号都可以，也就是说 lo 接口可以理解为一个网络号。

可以 ping 该网段中的 IP 地址，例如 127.0.0.1、127.255.255.200 等。

另外常用的是，通过 ifconfig 设置一个浮动 IP 地址，该地址在 127 网段，从而可以在本地使用非 127.1 进行测试，可以通过如下命令设置。

{% highlight text %}
# ifconfig lo:1 127.168.1.1 netmask 255.0.0.0
{% endhighlight %}

然后，像 MySQL 就可以绑定该 IP 即可。

# 源码解析

在内核初始化的时候，会调用 net_dev_init() 函数，其中通过 register_pernet_device() 注册 lo 设备。
{% highlight c %}
/* Registered in net/core/dev.c */
struct pernet_operations __net_initdata loopback_net_ops = {
       .init = loopback_net_init,
};

static int __init net_dev_init(void)
{
    ... ...
    if (register_pernet_device(&loopback_net_ops))
        goto out;
    ... ...
}
subsys_initcall(net_dev_init);
{% endhighlight %}

在进行注册的过程中会调用 loopback_net_init() 做初始化操作，也就是 loopback NIC 的驱动程序。

{% highlight c %}
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
    alloc_netdev_mqs(sizeof_priv, name, name_assign_type, setup, 1, 1)

static __net_init int loopback_net_init(struct net *net)
{
    ... ...
    /* 申请一个net_device实例，并进行初始化 */
    dev = alloc_netdev(0, "lo", NET_NAME_UNKNOWN, loopback_setup);
    if (!dev)
        goto out;

    dev_net_set(dev, net);
    err = register_netdev(dev); /* 注册loopback NIC设备 */
    if (err)
        goto out_free_netdev;

    BUG_ON(dev->ifindex != LOOPBACK_IFINDEX);
    net->loopback_dev = dev;
    return 0;
    ... ...
}

static const struct net_device_ops loopback_ops = {
    .ndo_init            = loopback_dev_init,
    .ndo_start_xmit      = loopback_xmit,
    .ndo_get_stats64     = loopback_get_stats64,
    .ndo_set_mac_address = eth_mac_addr,
};

static void loopback_setup(struct net_device *dev)
{
    ... ...
    dev->netdev_ops     = &loopback_ops;
    ... ...
}
{% endhighlight %}

在调用 alloc_netdev() 初始化设备时，该函数同时会初始化接收和发送队列，该函数在初始化完数据结构之后，同时会调用回调函数 loopback_setup()，该函数会初始化一些与 lo 设备相关的参数。

其中，最重要的是 dev->netdev_ops，其中发消息的方法就是调用 loopback_xmit() 函数即可，该函数的声明如下。

{% highlight c %}
netdev_tx_t loopback_xmit(struct sk_buff *skb, struct net_device *dev);
{% endhighlight %}

其中，skb 是待发送的数据缓冲区，dev 是网络设备的一个指针。lo 设备是要把数据报文发给本机，所以其发送数据报文的函数比较特殊，它把 skb 稍加处理后，又转回给协议栈的数据报接收函数 netif_rx()。

{% highlight c %}
netdev_tx_t loopback_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct pcpu_lstats *lb_stats;
    int len;

    skb_orphan(skb);

    /* Before queueing this packet to netif_rx(),
     * make sure dst is refcounted.
     */
    skb_dst_force(skb);

    skb->protocol = eth_type_trans(skb, dev);

    /* it's OK to use per_cpu_ptr() because BHs are off */
    lb_stats = this_cpu_ptr(dev->lstats);

    len = skb->len;
    if (likely(netif_rx(skb) == NET_RX_SUCCESS)) { // 调用该函数发送
        u64_stats_update_begin(&lb_stats->syncp);
        lb_stats->bytes += len;
        lb_stats->packets++;
        u64_stats_update_end(&lb_stats->syncp);
    }

    return NETDEV_TX_OK;
}
{% endhighlight %}

首先会调用 skb_orphan() 把 skb 孤立，使它跟发送 socket 和协议栈不再有任何联系，也即对本机来说，这个 skb 的数据内容已经发送出去了，而 skb 相当于已经被释放掉了。



<!--
skb_orphan所做的实际事情是，首先从skb->sk(发送这个skb的那个socket)的sk_wmem_alloc减去skb->truesize，也即从socket的已提交发送队列的字节数中减去这个skb，表示这个skb已经发送出去了， 同时，如果有进程在这个socket上写等待，则唤醒这些进程继续发送数据报，然后把socket的引用计数减1，最后，令 sk->destructor和skb->sk都为NULL，使skb完全孤立。实际上，对于环回设备接口来说，数据的发送工作至此已经全部完成，接下来，只要把这个实际上还未被释放的skb传回给协议栈的接收
-->

{% highlight text %}
{% endhighlight %}
