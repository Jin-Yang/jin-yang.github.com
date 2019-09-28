---
Date: October 19, 2013
title: Linux 路由表
layout: post
comments: true
language: chinese
category: [linux, network]
---


<!-- more -->


linux在进行路由查找时，先查找local，再查找main：


实际上，如果内核认为目标地址是本机IP，就会将包的出口设备设置为loopback_dev（不管路由表将出口设备设置成什么）。
{% highlight c lineno %}
static inline int fib_lookup(struct net *net, const struct flowi4 *flp,
                 struct fib_result *res)
{
    struct fib_table *table;

    table = fib_get_table(net, RT_TABLE_LOCAL);
    if (!fib_table_lookup(table, flp, res, FIB_LOOKUP_NOREF))
        return 0;

    table = fib_get_table(net, RT_TABLE_MAIN);
    if (!fib_table_lookup(table, flp, res, FIB_LOOKUP_NOREF))
        return 0;
    return -ENETUNREACH;
}
{% endhighlight %}




ip route list table local
ip route list table main

