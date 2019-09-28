---
Date: Auguest 05, 2015
title: Linux 网络超时与重传
layout: post
comments: true
language: chinese
category: [linux, network]
---

在此介绍重传，为了保证可靠性，在 TCP 的三次握手、数据传输、链接关闭阶段都有响应的重传机制。那么，重传的次数都是有那些参数指定？tcp_retries1 和 tcp_retries2 到底有什么区别？什么是 orphan socket ？


<!-- more -->

首先我们查看一下网络中与重传相关的参数有那些。

# 重传简介

关于重传的控制参数，可以查看内核中的 Documentation/networking/ip-sysctl.txt 文件，内核中的控制参数总共有五个相关参数。

{% highlight text %}
tcp_syn_retries - INTEGER
    Number of times initial SYNs for an active TCP connection attempt
    will be retransmitted. Should not be higher than 255. Default value
    is 6, which corresponds to 63seconds till the last retransmission
    with the current initial RTO of 1second. With this the final timeout
    for an active TCP connection attempt will happen after 127seconds.

tcp_synack_retries - INTEGER
    Number of times SYNACKs for a passive TCP connection attempt will
    be retransmitted. Should not be higher than 255. Default value
    is 5, which corresponds to 31seconds till the last retransmission
    with the current initial RTO of 1second. With this the final timeout
    for a passive TCP connection will happen after 63seconds.

tcp_orphan_retries - INTEGER
    This value influences the timeout of a locally closed TCP connection,
    when RTO retransmissions remain unacknowledged.
    See tcp_retries2 for more details.

    The default value is 8.
    If your machine is a loaded WEB server,
    you should think about lowering this value, such sockets
    may consume significant resources. Cf. tcp_max_orphans.

tcp_retries1 - INTEGER
    This value influences the time, after which TCP decides, that
    something is wrong due to unacknowledged RTO retransmissions,
    and reports this suspicion to the network layer.
    See tcp_retries2 for more details.

    RFC 1122 recommends at least 3 retransmissions, which is the
    default.

tcp_retries2 - INTEGER
    This value influences the timeout of an alive TCP connection,
    when RTO retransmissions remain unacknowledged.
    Given a value of N, a hypothetical TCP connection following
    exponential backoff with an initial RTO of TCP_RTO_MIN would
    retransmit N times before killing the connection at the (N+1)th RTO.

    The default value of 15 yields a hypothetical timeout of 924.6
    seconds and is a lower bound for the effective timeout.
    TCP will effectively time out at the first RTO which exceeds the
    hypothetical timeout.

    RFC 1122 recommends at least 100 seconds for the timeout,
    which corresponds to a value of at least 8.
{% endhighlight %}

简单来说，五个重试参数的含义为：

* tcp_syn_retries：在三次握手阶段，客户端发起链接时，发送 SYN 报文的重试次数。

* tcp_synack_retries：在三次握手阶段，服务端回应 SYN+ACK 时，发送报文的重试次数。

* tcp_orphan_retries：在关闭阶段，会影响到主动关闭链接的孤儿 socket 重传。

* tcp_retries1：在数据传输阶段。

* tcp_retries2：在数据传输阶段。

在此重点关注一下 tcp_retries1、tcp_retries2 两个参数，以及 tcp_orphan_retries 。

# tcp_retries1、tcp_retries2

这两个参数在上述的介绍中有些模糊，可能由于过于概括，会令人产生很多疑问，甚至产生一些误解。

1. 当重试超过 tcp_retries1 这个阈值后，到底向网络层报告了什么 suspicion ？

2. tcp_retries1 和 tcp_retries2 对应的是重传次数？其中的时间是怎么计算出来的？

3. tcp_retries2 应该是重传的上限吧，其中的 lower bound for the effective timeout 又是几个意思？不应该是 upper bound 吗？effective timeout 又是做什么的？

## 源码解析

定时器的初始化是在 tcp_init_xmit_timers() 函数中完成，超时重传相关的是 tcp_write_timer()，实际最终调用的是 tcp_write_timer_handler() 。

{% highlight c %}
void tcp_write_timer_handler(struct sock *sk)
{
    ... ...
    switch (event) {
    case ICSK_TIME_EARLY_RETRANS:
        tcp_resume_early_retransmit(sk);
        break;
    case ICSK_TIME_LOSS_PROBE:
        tcp_send_loss_probe(sk);
        break;
    case ICSK_TIME_RETRANS:
        icsk->icsk_pending = 0;
        tcp_retransmit_timer(sk);
        break;
    case ICSK_TIME_PROBE0:
        icsk->icsk_pending = 0;
        tcp_probe_timer(sk);
        break;
    }
    ... ...
}
{% endhighlight %}

如果是超时重传定时器触发的，就会调用 tcp_retransmit_timer() 进行处理，其中与 tcp_retries 相关的代码调用逻辑如下。

{% highlight text %}
tcp_retransmit_timer()
  |-tcp_write_timeout()              # 判断是否重传了足够的久
  | |-retransmit_timed_out()         # 判断是否超过了阈值
  |-tcp_retransmit_skb()             # 如果没有超过了重传阈值，则直接重传报文
{% endhighlight %}

接下来看一下 tcp_write_timeout() 的具体实现。

{% highlight c %}
static int tcp_write_timeout(struct sock *sk)
{
    struct inet_connection_sock *icsk = inet_csk(sk);
    struct tcp_sock *tp = tcp_sk(sk);
    int retry_until;
    bool do_reset, syn_set = false;

    if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) {
        /* 超时发生在三次握手期间，重传次数通过tcp_syn_retries指定 */
        ... ...
        retry_until = icsk->icsk_syn_retries ? : sysctl_tcp_syn_retries;
        syn_set = true;
    } else { /* 超时发生在数据发送期间，也就是我们现在讨论的问题 */
        /* 下面的函数负责判断重传是否超过阈值，返回真表示超过 */
        if (retransmits_timed_out(sk, sysctl_tcp_retries1, 0, 0)) {
            /* Black hole detection */
            tcp_mtu_probing(icsk, sk); /* 如果开启了tcp_mtu_probing，则执行PMTU */
            /* 更新路由缓存，用以避免由于路由选路变化带来的问题 */
            dst_negative_advice(sk);
        }

        retry_until = sysctl_tcp_retries2;
        if (sock_flag(sk, SOCK_DEAD)) {
            /* 表示是没有应用在使用的一个孤立的socket */
            const int alive = icsk->icsk_rto < TCP_RTO_MAX;

            retry_until = tcp_orphan_retries(sk, alive);
            do_reset = alive ||
                !retransmits_timed_out(sk, retry_until, 0, 0);

            if (tcp_out_of_resources(sk, do_reset))
                return 1;
        }
    }

    /* 一般来说，如果没有孤儿socket，那么一般重传次数是tcp_retries2 */
    if (retransmits_timed_out(sk, retry_until,
                  syn_set ? 0 : icsk->icsk_user_timeout, syn_set)) {
        /* Has it gone just too far? */
        tcp_write_err(sk); /* 最终会调用tcp_done关闭TCP流 */
        return 1;
    }
    return 0;
}
{% endhighlight %}

在如上的函数中，两次超时时间的计算都是通过 retransmits_timed_out() 函数计算的，那么也就是该函数实际会判断是否已经超时。

{% highlight c %}
#define tcp_time_stamp      ((__u32)(jiffies))

static bool retransmits_timed_out(struct sock *sk,
                  unsigned int boundary,
                  unsigned int timeout,
                  bool syn_set)
{
    unsigned int linear_backoff_thresh, start_ts;
    /* 如果是在三次握手阶段，syn_set为真 */
    unsigned int rto_base = syn_set ? TCP_TIMEOUT_INIT : TCP_RTO_MIN;

    /* 如果设置的重传次数为0，那么就直接退出 */
    if (!inet_csk(sk)->icsk_retransmits)
        return false;

    /* 获取数据包第一次发送的时间，在tcp_retransmit_skb()中设置 */
    start_ts = tcp_sk(sk)->retrans_stamp;
    if (unlikely(!start_ts))
        start_ts = tcp_skb_timestamp(tcp_write_queue_head(sk));

    /* 如果用户态没有指定timeout，则自动计算一个出来 */
    if (likely(timeout == 0)) {
        /* 计算一下以rto_base为第一次重传间隔，重传boundary次所需要的时间 */
        linear_backoff_thresh = ilog2(TCP_RTO_MAX/rto_base);

        if (boundary <= linear_backoff_thresh)
            timeout = ((2 << boundary) - 1) * rto_base;
        else
            timeout = ((2 << linear_backoff_thresh) - 1) * rto_base +
                (boundary - linear_backoff_thresh) * TCP_RTO_MAX;
    }

    /* 如果数据包第一次发送的时间距离现在的时间间隔，超过了timeout值，则认为重传超于阈值了 */
    return (tcp_time_stamp - start_ts) >= timeout;
}
{% endhighlight %}

从以上的代码分析可以看到，如果用户没有设置超时时间，那么真正起到限制重传次数的并不是真正的重传次数，而是与此相关的一个超时时间。

如上，是以 tcp_retries1 或 tcp_retries2 为 boundary，以 rto_base (如 TCP_RTO_MIN=200ms) 为初始的 RTO，计算出来一个 timeout 值，当重传间隔超过这个 timeout 后，则认为超过了阈值。

如文档中所说的，当 tcp_retries2=15 时，那么计算得到的 timeout 是 924600ms，这个值是如何计算到的？简单来说，还是在 retransmits_timed_out() 函数计算完成，计算过程如下：

{% highlight text %}
----- 首先计算thresh
linear_backoff_thresh = ilog2(TCP_RTO_MAX/rto_base) = ilog2((120*HZ)/(HZ/5)) = ilog2(600) = 9

if (boundary <= linear_backoff_thresh)
    timeout = ((2 << boundary) - 1) * rto_base;
else
    timeout = ((2 << linear_backoff_thresh) - 1) * rto_base +
        (boundary - linear_backoff_thresh) * TCP_RTO_MAX;

----- 此时选择的是第二个分支
timeout = (2<<9 - 1) * (HZ/5) + (15 - 9) * 120 * HZ = 924.6s
{% endhighlight %}

如果用户没有指定超时时间，那么默认就是 924.6s，而对于具体重试了多少次，是与 RTT 相关的。

1. 如果 RTT 比较小，那么 RTO 初始值就约等于下限 200ms，这时表现出来的现象基本就是刚好重传了 15 次。

2. 如果 RTT 较大，比如 RTO 初始值计算得到的是 1000ms，那么根本不需要重传 15 次，重传总间隔就会超过 924.6s 。

对于上述的问题，基本也就有结论了：

1. 重试超过了 tcp_retries1 之后，怀疑路由有问题，直接更新路由缓存。

2. 通过 tcp_retries 计算出来的是时间，而具体重传了多少次是与实际的 RTT 相关的。

3. effective timeout 实际就是 retransmits_timed_out() 函数计算得到的 timeout 值。所谓的 lower / upper bound 是指超时时间的范围，如果是 15 那么实际的重传范围是 [924.6s, 1044.6s) 。


# tcp_orphan_retries

首先介绍一下什么是 orphan socket，简单来说就是该 socket 不与任何一个文件描述符相关联。例如，当应用主动调用 close() 关闭一个链接时，此时该 socket 就成为了 orphan，但是该 sock 仍然会保留一段时间，直到最后根据 TCP 协议结束。


![fourway]{: .pull-center}

如上是 TCP 关闭链接时的握手过程。

主动关闭的一方发出 FIN，同时进入 FIN_WAIT1 状态，被动关闭的一方响应 ACK，从而使主动关闭的一方迁移至 FIN_WAIT2 状态，接着被动关闭的一方同样会发出 FIN，主动关闭的一方响应 ACK，同时链接的状态迁移至 TIME_WAIT 。

那么这与 tcp_orphan_retries 参数有什么关系？

正常来说，服务器间的 ACK 确认是非常快的，通常不会有 FIN_WAIT1 状态存在，如果被动关闭的一段很长时间没有响应，此时的 TCP 协议会如何处理呢。

实际上这个参数决定了 FIN_WAIT1 状态的持续时间，其计算方式与 tcp_retries1 的相同。

内核中还有一个容易混淆的参数 net.ipv4.tcp_fin_timeout，它实际上它控制的是 FIN_WAIT2 的超时时间，其定义如下：

{% highlight text %}
tcp_fin_timeout - INTEGER
    The length of time an orphaned (no longer referenced by any
    application) connection will remain in the FIN_WAIT_2 state
    before it is aborted at the local end.  While a perfectly
    valid "receive only" state for an un-orphaned connection, an
    orphaned connection in FIN_WAIT_2 state could otherwise wait
    forever for the remote to close its end of the connection.
    Cf. tcp_max_orphans
    Default: 60 seconds
{% endhighlight %}

需要注意的是，在 tcp_orphan_retries 的定义中，如果要设置成一个比较小的值，该值应该至少大于0，如果是 0 那么实际等同于 8 ，对应的代码如下：

{% highlight c %}
/* Calculate maximal number or retries on an orphaned socket. */
static int tcp_orphan_retries(struct sock *sk, int alive)
{
    int retries = sysctl_tcp_orphan_retries; /* May be zero. */

    /* We know from an ICMP that something is wrong. */
    if (sk->sk_err_soft && !alive)
        retries = 0;

    /* However, if socket sent something recently, select some safe
     * number of retries. 8 corresponds to >100 seconds with minimal
     * RTO of 200msec. */
    if (retries == 0 && alive)
        retries = 8;
    return retries;
}
{% endhighlight %}

可以得出结论，如果系统负载较重，有很多处于 FIN_WAIT1 的链接，那么可以通过降低 tcp_orphan_retries 来解决问题，具体设置多少视网络条件而定。

另外，在遇到 DoS 攻击时，可以通过设置 tcp_max_orphans 减小。但是这和用来控制 TIME_WAIT 的最大值的 tcp_max_tw_buckets 参数一样，除非你遇到了 DoS 攻击，否则最好不要降低它。


## 如何关闭一个链接

如果要关闭一个 TCP 连接，那么需要知道相应的 ACK 和 SEQ ，然后才可以 RESET 连接。

为了获取 ACK 和 SEQ 可以有主动与被动两种，分别有 tcpkill 以及 killcx，其中后者是 Perl 写的脚本。












[fourway]:         /images/linux/four-way-handshake.png

<!--
http://perthcharles.github.io/2015/09/06/wiki-rtt-estimator/            RTO的计算方法
-->
