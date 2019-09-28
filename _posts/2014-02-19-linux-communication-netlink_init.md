---
Date: December 12, 2015
title: Linux 内核与用户态通讯机制 -- Netlink
layout: post
comments: true
language: chinese
category: [linux]
---

Netlink 机制是在 Linux 中作为内核态与用户态的一种通讯机制，它是基于 socket 的！！！怎么样，没有想到，除了 TCP/IP 协议外，这样也可以 ^_^

另外的一个特性是，面向数据报文的无连接消息子系统，有点类似于 UDP 协议。

<!-- more -->

# 简介

如果要为新特性添加系统调用，那么相比传统的 ioctl 和 procfs 来说，netlink 要简单的多，只需要将一个常量以及协议类型加入到 netlink.h 文件中即可。然后，内核模块和用户程序可以通过 socket 类型的 API 进行通信。

与其它基于 socket API 实现的通讯协议一样，Netlink 也是异步的，它通过一个 socket 队列来缓存突发的消息。







# 内核模块和用户测试程序

内核中的 Netlink 相关接口实际上一直在变化，最好的方式是参考内核中相关的实现，例如 TCP/IP 协议相关的诊断模块 sock_diag ，对应应用程序为 iproute2 包。

对于 Netlink，不仅可以实现用户-内核空间的通信还可使现实用户空间两个进程之间，或内核空间两个进程之间的通信。

## Netlink 消息格式

消息体包括了两部分：消息头和有效数据载荷；其中，消息头固定为 16 字节，消息体长度可变。另外，整个 Netlink 消息是 4 字节对齐，很多与消息体相关的宏操作。

{% highlight text %}
|<====== 16B ======>|<======================= 2^32-16B =============================>|
+-------------------+----------------------------------------------------------------+
|    msg header     |                         msg body                               |
+-------------------+----------------------------------------------------------------+
|                   |                                                                |
|                   |<--------------------- NLMSG_SPACE() -------------------------->|
|                                                                                    |
|<------------------------------- NLMSG_LENGTH() ----------------------------------->|
{% endhighlight %}

对于上述的 Netlink 消息，可以通过几个宏的进行操作：

* NLMSG_SPACE(MAX_PAYLOAD)、NLMSG_LENGTH(LEN)：通常在申请内存时使用，两个宏都会返回 4 字节对齐的最小长度值，前者的入参为 body 大小，而后者为 header+body 的长度。

* NLMSG_DATA(nlh)：返回 Netlink 消息中数据部分的首地址，写入和读取消息数据部分时会使用。

其中消息头定义通过 struct nlmsghdr 表示：
{% highlight c %}
struct nlmsghdr {
    __u32        nlmsg_len;     /* Length of message including header */
    __u16        nlmsg_type;    /* Message content */
    __u16        nlmsg_flags    /* Additional flags */
    __u32        nlmsg_seq;     /* Sequence number */
    __u32        nlmsg_pid;     /* Sending process PID */
};
{% endhighlight %}

消息头结构体中各成员详细解释如下：

* nlmsg_len： 包括消息头在内的整个消息长度，按字节计算。

* nlmsg_type：消息的类型，即是数据还是控制消息。目前类型有 4 中：A) NLMSG_NOOP 空操作；B) NLMSG_ERROR 错误消息；C) NLMSG_DONE 若内核返回多个消息，最后一条为该类型，其它的消息的 nlmsg_flags 属性被设置 NLM_F_MULTI 位有效；D) NLMSG_OVERRUN 数据被覆盖。

* nlmsg_flags：消息中的额外说明信息，通常用 NLM_F_XXX 表示，其中常用的如下：

    * NLM_F_MULTI，消息从用户到内核是同步的立刻完成，而从内核用户则需要排队。如果用户发送的请求中有NLM_F_DUMP标志位，那么内核就会向用户发送多个 Netlink 消息，除了最后的消息外，其余都设置了该位。

    * NLM_F_REQUEST，标示请求消息。所有从用户到内核的消息都要设置该位，否则内核将返回一个 EINVAL 无效参数的错误。

    * NLM_F_ACK，内核对来自用户空间的 NLM_F_REQUEST 消息的响应。

    * NLM_F_ECHO，用户发送内核发送时置位，则说明用户要求内核以单播的方式再发送给用户，也就是 "回显" 。

* nlmsg_seq：消息序号。类似于 UDP，Netlink 也可能会丢失数据，如果用户要保证其发送的每条消息都能成功被内核收到，那么在发送请求时需要设置序号，然后检查返回结果的该值是否相同。对于内核向用户发送的广播消息，该字段为 0 。

* nlmsg_pid：Netlink 会为用户空间和内核空间建立的链接分配一个唯一标示，用于确保内核返回给用户时的进程 ID 。



















## Netlink 地址结构体

和 TCP/IP 协议中的地址结构体和标准结构体相似，Netlink 的结构体如下：

{% highlight text %}
-----   sock标准结构体                    TCP/IP结构体                   Netlink结构体

        struct sockaddr;             struct sockaddr_in;            struct sockaddr_nl;
      +------------------+          +------------------+           +------------------+
   2B |    sa_family     |       2B |    sin_family    |        2B |    nl_family     |
      +------------------+          +------------------+           +------------------+
  14B |   sa_data[14]    |       2B |     sin_port     |        2B |      nl_pad      |
      +------------------+          +------------------+           +------------------+
                                 4B |     sin_addr     |        4B |      nl_pid      |
                                    +------------------+           +------------------+
                                 8B |      __pad       |        4B |    nl_groups     |
                                    +------------------+           +------------------+
{% endhighlight %}

对于 Netlink 地址结构体，详细内容如下：
{% highlight c %}
struct sockaddr_nl {
    sa_family_t    nl_family;    // 总是为AF_NETLINK
    unsigned short    nl_pad;    // 目前未用到，填充为0
    __u32        nl_pid;         // process pid
    __u32        nl_groups;      // multicast groups mask
};
{% endhighlight %}

其中，各个成员的详细内容如下：

* nl_family：对应 AF_NETLINK ，不用多说了。

* nl_pad：暂未使用，填充为 0 。

* nl_pid：PID 全称为 Port-ID，用于唯一标识一个基于 Netlink 的 socket 通道。通常为当前进程的进程号，对于多线程的应用可以设置为 `pthread_self() << 16 | getpid()`。如果为 0 通常有两种情况：A) 用户空间发往内核空间；B)
<!--
   第二，从内核发出的多播报文到用户空间时，如果用户空间的进程处在该多播组中，那么其地址结构体中nl_pid也设置为0，同时还要结合下面介绍到的另一个属性。
-->

* nl_groups：对于多播，该字段标明了调用者希望加入的多播组号的掩码；0 表示调用者不希望加入任何多播组。

如果用户空间的进程想加入某个多播组，则必须执行 bind() 系统调用；对于每个 Netlink 协议域中的协议，最多可支持 32 个多播组，每个多播组用一个比特来表示。




# 内核实现

其中内核实现在 net/netlink/af_netlink.c 文件中。

## 初始化

Netlink 相关的内容通过 core_initcall() 指定，其初始化函数为 netlink_proto_init()，简化后的内容如下。
{% highlight c %}
static const struct net_proto_family netlink_family_ops = {
    .family = PF_NETLINK,
    .create = netlink_create,
    .owner  = THIS_MODULE,  /* for consistency 8) */
};

static struct pernet_operations __net_initdata netlink_net_ops = {
    .init = netlink_net_init,
    .exit = netlink_net_exit,
};

static int __init netlink_proto_init(void)
{
    ... ...
    sock_register(&netlink_family_ops);
    register_pernet_subsys(&netlink_net_ops);
    ... ...
}
core_initcall(netlink_proto_init);
{% endhighlight %}

其中在创建命名空间时，会调用 netlink_net_init() 函数，而 netlink_family_ops 变量则定义了 sock 相关的函数，也就是如何创建 Netlink socket 。

而 netlink_net_init() 函数就非常简单了，就是在 procfs 中创建 /proc/net/netlink 文件。

## 创建 Socket

如上述 netlink_family_ops 变量中的定义，创建 Socket 时最终会调用到 netlink_create() 函数。

{% highlight c %}
static const struct proto_ops netlink_ops = {
    .family =       PF_NETLINK,
    .owner =        THIS_MODULE,
    .release =      netlink_release,
    .bind =         netlink_bind,
    .connect =      netlink_connect,
    .socketpair =   sock_no_socketpair,
    .accept =       sock_no_accept,
    .getname =      netlink_getname,
    .poll =         netlink_poll,
    .ioctl =        sock_no_ioctl,
    .listen =       sock_no_listen,
    .shutdown =     sock_no_shutdown,
    .setsockopt =   netlink_setsockopt,
    .getsockopt =   netlink_getsockopt,
    .sendmsg =      netlink_sendmsg,
    .recvmsg =      netlink_recvmsg,
    .mmap =         netlink_mmap,
    .sendpage =     sock_no_sendpage,
};

static struct proto netlink_proto = {
    .name     = "NETLINK",
    .owner    = THIS_MODULE,
    .obj_size = sizeof(struct netlink_sock),
};

static int __netlink_create(struct net *net, struct socket *sock,
                struct mutex *cb_mutex, int protocol)
{
    ... ...
    sock->ops = &netlink_ops;
    sk = sk_alloc(net, PF_NETLINK, GFP_KERNEL, &netlink_proto);
    sock_init_data(sock, sk);
    init_waitqueue_head(&nlk->wait);
    ... ..
}
{% endhighlight %}

也就是创建 struct sock 并初始化，其中最重要的是 netlink_ops 变量，定义了 sock 相关的操作函数，如常见的 sendmsg()、connect() 等操作。

也就是说，接下来我们可以针对不同的接口进行分析。





## 其它
{% highlight text %}
{% endhighlight %}

netlink_rcv_skb();
{% highlight c %}
int netlink_rcv_skb(struct sk_buff *skb, int (*cb)(struct sk_buff *, struct nlmsghdr *));
{% endhighlight %}
该函数提供了一个通用的接收函数，会先做一些头文件的检查，判断是否有 NLM_F_ACK(回显) 标示，如果正常或者不需要 Netlink 处理，则会调用其传入的函数。




# Generic Netlink

该模块通过 subsys_initcall(genl_init) 定义初始化函数，也就是在系统启动的时候会调用 genl_init() 初始化函数。而在 genl_init() 函数中会调用 register_pernet_subsys()，该函数主要作用是将一个网络协议模块添加到每一个网络命令空间中。

{% highlight c %}
static struct pernet_operations genl_pernet_ops = {
    .init = genl_pernet_init,
    .exit = genl_pernet_exit,
};

static int __init genl_init(void) {
    ... ...
    err = register_pernet_subsys(&genl_pernet_ops);
    ... ...
}
subsys_initcall(genl_init);
{% endhighlight %}

也就是说实际的初始化函数为 genl_pernet_init()，该函数中调用 netlink_kernel_create() 创建回掉函数，也就是对应 genl_rcv() 。
{% highlight c %}
static int __net_init genl_pernet_init(struct net *net)
{
    struct netlink_kernel_cfg cfg = {
        .input      = genl_rcv,
        .flags      = NL_CFG_F_NONROOT_RECV,
    };
    net->genl_sock = netlink_kernel_create(net, NETLINK_GENERIC, &cfg);
    ... ...
}

static void genl_rcv(struct sk_buff *skb)
{
    down_read(&cb_lock);
    netlink_rcv_skb(skb, &genl_rcv_msg);
    up_read(&cb_lock);
}
{% endhighlight %}
如上，netlink_rcv_skb() 是 Netlink 提供的通用函数，如果正常则调用传入的回掉函数 genl_rcv_msg() 。

{% highlight c %}
static int genl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    struct genl_family *family;
    int err;

    family = genl_family_find_byid(nlh->nlmsg_type);
    if (family == NULL)
        return -ENOENT;

    if (!family->parallel_ops)
        genl_lock();

    err = genl_family_rcv_msg(family, skb, nlh);

    if (!family->parallel_ops)
        genl_unlock();

    return err;
}
{% endhighlight %}






# 参考

Netlink 相关内容可以参考内核中的文档，以及 [www.linuxfoundation.org][generic-netlink-howto] 中关于 netlink 协议的基本介绍，或者 [本地保存][netlink-local] 信息。

用户端的实现还可以参考 [Netlink Protocol Library Suite (libnl)][libnl-offical]，一个不错的实现，还包括了 Python 的接口，偶然发现，没有仔细研究








[generic-netlink-howto]:    http://www.linuxfoundation.org/collaborate/workgroups/networking/generic_netlink_howto    "netlink的相关帮助信息"
[netlink-local]:            /reference/linux/communication/generic_netlink_howto.mht
[libnl-offical]:            http://www.infradead.org/~tgr/libnl/                                                      "Netlink Protocol Library Suite (libnl)"

<!--
http://blog.chinaunix.net/uid-23069658-id-3400761.html    用户空间和内核空间通讯之【Netlink 上】
http://www.linuxfoundation.org/collaborate/workgroups/networking/generic_netlink_howto      generic_netlink_howto


http://www.linuxidc.com/linux/2011-09/42936.htm
http://www.linuxidc.com/Linux/2011-07/39085p2.htm
http://www.ibm.com/developerworks/cn/linux/l-netlink/index.html
http://blog.chinaunix.net/uid-20788636-id-1841429.html
http://www.tuicool.com/articles/fEFzY3a
http://www.360doc.com/content/13/1214/19/8744436_337155545.shtml
http://www.360doc.com/content/13/1214/19/8744436_337155240.shtml
http://blog.csdn.net/daydring/article/details/24000081
http://blog.chinaunix.net/uid-14753126-id-2983915.html
http://blog.chinaunix.net/uid-9950859-id-98548.html
http://blog.chinaunix.net/uid-26675482-id-3255770.html
http://blog.chinaunix.net/uid-23069658-id-3400761.html             #####
http://blog.chinaunix.net/uid-20788636-id-2980152.html
http://blog.chinaunix.net/uid-26675482-id-3255770.html
http://blog.csdn.net/love_life2011/article/details/7596190
http://blog.csdn.net/zcabcd123/article/details/8275891
http://blog.csdn.net/zcabcd123/article/details/8272423
http://blog.csdn.net/wangjingfei/article/details/5288460
http://blog.csdn.net/liumang_D/article/details/5413042


http://blog.chinaunix.net/uid-20788636-id-2980152.html       IP路由


http://bbs.chinaunix.net/thread-2162796-1-1.html




http://www.linuxjournal.com/article/7356
http://bbs.chinaunix.net/thread-2029813-1-1.html


netlink是linux内核提供的一种用户空间与内核通讯基础组件，基于网络实现，可实现多播、单播、组播复杂的通讯功能。内核驱动建立服务端，用户程序通过socket绑定服务端，可发送消息与接收消息，实现监听、系统调用等功能。其中generic netlink(genetlink)是基于netlink机制实现的一种通用协议，可直接使用到一般用户程序环境中。

本文实现的是一种通知机制，用户程序监听内核netlink创建的端口，不能通过端口发消息到内核。这种机制适用于报告系统硬件状态，如电池电量、温度，SIM卡移除等
1. NETLINK内核编码相关

本框架是基于netlink的一种通用协议generic netlink(genetlink)，并没有单独占用新的协议号，内核服务端部分注册genetlink接口、相关操作函数和指定数据传输格式。

注册接口：用于通知内核有一个family添加，可提供服务


ret = genl_register_family(&detect_family);     //注册family
if (ret != 0)
        return ret;

ret = genl_register_ops(&detect_family, &user_pid_ops); //family相关的操作函数
if (ret != 0)
        goto unreg_fam;

接口数据结构定义：family类型对应的操作接口，可定义多个，但cmd字段不同


static struct genl_ops user_pid_ops = {
        .cmd = 0x01,
        .flags = 0,
        .policy = user_msg_policy,
        .doit = set_user_pid,
        .dumpit = NULL,
};

操作接口用于响应消息，用户给内核发送命令时需指定命令号cmd，发送的内容格式为policy指定，其他字段程序中没用到。同一family可注册多个命令，不同命令对应各自的处理函数doit。


static struct genl_family detect_family = {
        .id = GENL_ID_GENERATE,
        .hdrsize = 0,
        .name = "DETECT_USB",
        .version = 0x01,
        .maxattr = DETECT_A_MAX,
};

协议结构，使用genl接口的id统一为GENL_ID_GENERATE，name字段用于标识特定的family，用户程序通过比较该字段连接到family。 此处用于响应用户消息的接口只接收用户进程的pid，之后内核会将消息发送到该pid进程

内核消息发送接口：将指定消息发送到用户进程，消息是一个32位整数，消息的定义内核与用户程序要一致


skb = genlmsg_new(size, GFP_KERNEL);    //申请发送数据缓冲
if (skb == NULL)
        goto end;

//使用family协议发送数据，填充协议头
msg_head = genlmsg_put(skb, 0, 0, &detect_family,
                                           0x01);
if (msg_head == NULL) {
        nlmsg_free(skb);
        goto end;
}

if (nla_put_u32(skb, DETECT_A_UINT32, event->event) <</span> 0) {
        nlmsg_free(skb);
        goto end;
}

if (genlmsg_end(skb, msg_head) <</span> 0) {
        nlmsg_free(skb);
        goto end;
}

genlmsg_unicast(&init_net, skb, g_detect->user_pid);

在建立socket连接后内核可随时向用户空间发送消息，用户程序调用recv接收。框架暂时没有使用多播、组播功能。
2. NETLINK用户程序框架

基于NETLINK通讯的用户程序类似SOCKET程序，都是创建socket,绑定端口号，发送和接收数据等操作。框架中用户守护进程阻塞接收内核消息，再调用消息处理函数分发消息。

创建socket并绑定： 创建一个netlink类型的socket


struct sockaddr_nl local;

fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
if (fd <</span> 0)
        return -1;

memset(&local, 0, sizeof(local));
local.nl_family = AF_NETLINK;
local.nl_groups = 0;
if (bind(fd, (struct sockaddr *)&local, sizeof(local)) <</span> 0)
        goto error;

return fd;

创建NETLINK_GENERIC类型socket，绑定端口。

查找DETECT_USB服务端，这部分属于genetlink公用部分。



family_req.n.nlmsg_type = GENL_ID_CTRL;
family_req.n.nlmsg_flags = NLM_F_REQUEST;
family_req.n.nlmsg_seq = 0;
family_req.n.nlmsg_pid = getpid();
family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
family_req.g.cmd = CTRL_CMD_GETFAMILY;
family_req.g.version = 0x1;

na = (struct nlattr *)GENLMSG_DATA(&family_req);
na->nla_type = CTRL_ATTR_FAMILY_NAME;
na->nla_len = strlen("DETECT_USB") + 1 + NLA_HDRLEN;
strcpy(NLA_DATA(na), "DETECT_USB");

family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

if (sendto_fd(sd, (char *)&family_req, family_req.n.nlmsg_len) <</span> 0)
        return -1;

rep_len = recv(sd, &ans, sizeof(ans), 0);
if (rep_len <</span> 0)
        return -1;

na = (struct nlattr *)GENLMSG_DATA(&ans);

na = (struct nlattr *)((char *)na + NLA_ALIGN(na->nla_len));
if (na->nla_type == CTRL_ATTR_FAMILY_ID)
        id = *(__u16 *) NLA_DATA(na);

这里查找使用的字符串必须与内核中注册接口结构中定义的字符串相同，用于绑定到我们注册的接口。

发送消息相关程序：用户程序初始化时运行一次，用于将自己的pid通知到内核



req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
req.n.nlmsg_type = id;
req.n.nlmsg_flags = NLM_F_REQUEST;
req.n.nlmsg_seq = 0;
req.n.nlmsg_pid = getpid();
req.g.cmd = 1;

na = (struct nlattr *)GENLMSG_DATA(&req);
na->nla_type = 1;                        //DETECT_A_MSG，消息格式类型
snprintf(message, 63, "usb detect deamon setup with pid %d", getpid());
na->nla_len = 64 + NLA_HDRLEN;
memcpy(NLA_DATA(na), message, 64);
req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

memset(&nladdr, 0, sizeof(nladdr));
nladdr.nl_family = AF_NETLINK;

sendto(sd, (char *)&req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr));

接收消息相关接口：这里放在一个循环里来做，也可以用poll实现


rep_len = recv(sd, &ans, sizeof(ans), 0);  //阻塞接收内核消息
if (ans.n.nlmsg_type == NLMSG_ERROR)
        return -1;

if (rep_len <</span> 0)
        return -1;

if (!NLMSG_OK((&ans.n), rep_len))
        return -1;

na = (struct nlattr *)GENLMSG_DATA(&ans);   //验证正确后做消息解析。

总结

上面实现了基于genetlink内核组件的内核消息单播框架，并不是一个完成的应用程序，这也是我在做netlink程序时认为最主要的内容，其实将内核与用户进程互发消息调试通过之后的事情就不与netlink相关了，知道这个框架能提供些什么服务，应用程序也好做扩展。

-->
