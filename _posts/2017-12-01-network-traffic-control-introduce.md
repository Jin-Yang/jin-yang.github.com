---
title: TC 使用简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

网络的 IP 服务模型是尽力而为的，这样的模型不能体现某些流量的重要性，所以诞生了 QoS 技术，Linux 很早就提供了流量控制接口，命令行工具是 tc (Traffic Control)。

<!-- more -->

## 简介

协议栈的 QoS 由三部分组成：A) qdisc 队列规则；B) class 控制策略；C) filter 过滤选择策略。内核中使用 Hierarchical Token Bucket, HTB 方式实现。

网络报文会先发送到一个发送队列，也就是 Qdisc ，然后由该队列决定何时以及如何发送报文，而这里的所谓策略制定，就是通过 Class 和 Filter 实现。

注意，入口的流量比较难控制，一般会通过出口控制流量，如果要控制入口流量，一般是添加一个虚拟的网络设备。

### 使用

在 Linux 中，流量控制是通过 tc 完成，通产对网卡进行流量控制的配置，需要进行如下的步骤。

1. 为网卡配置一个队列；
2. 在该队列上建立分类；
3. 根据需要建立子队列和子分类；
4. 为每个分类建立过滤器。

一般使用的队列有 CBQ 和 HTB，后者是用来替换前者的，相比来说 Hierarchical Token Bucket, HTB 配置简单、容易理解。

### 标识

使用 `major:minor` 这样的句柄来标识队列和类别，其中 major 和 minor 都是数字。

队列的 minor 总是 0 ，也就是 `major:0` ，一般简写时会把后面的 0 直接省略，而 major 需要保持在某个网卡中的唯一。

对于类别来说，其 major 必须和它的父类别或父队列的 major 相同，而 minor 在一个队列内部则必须是惟一。

## 实践

### 1. 流量设置

假设 eth0 出口带宽为 100Mb ，分配给 WWW E-Mail Telnet 的带宽分别为 40:40:20 ，那么可以通过如下方式设置。

相关的详细使用方法可以通过 `man 8 tc` 查看，这里仅针对示例测试。

#### 配置队列

也就是为网卡 eth0 配置一个 HTB 队列。

{% highlight text %}
# tc qdisc add dev eth0 root handle 1:htb default 11
{% endhighlight %}

这里，root 表示为网卡 eth0 添加的是一个根队列；`handle 1:htb` 表示队列的句柄为 `1:` 队列类型为 HTB 队列；后面的 `default 11` 是 HTB 的特有队列参数，意味着将未分类的流量分配给类别 `1:11` 。

#### 创建类别

分别创建三个类别 `1:11` `1:12` `1:13` ，分别占用 40M:40M:20M 的带宽。

{% highlight text %}
# tc class add dev eth0 parent 1: classid 1:11 htb rate 40mbit ceil 40mbit
# tc class add dev eth0 parent 1: classid 1:12 htb rate 40mbit ceil 40mbit
# tc class add dev eth0 parent 1: classid 1:13 htb rate 20mbit ceil 20mbit
{% endhighlight %}

其中，`parent 1:` 表示类别的父亲为根队列 `1:`；`classid 1:11` 创建一个标识为 `1:11` 的类别； `rate 40mbit ceil 40mbit` 表示系统为该类别确保带宽 40mbit 最大 40mbit 。

#### 设置过滤器

因为要将 WWW、E-Mail、Telnet 三种流量分配到三个类别，需要创建三个过滤器。

{% highlight text %}
# tc filter add dev eth0 protocol ip parent 1:0 prio 1 u32 match ip dport 80 0xffff flowid 1:11
# tc filter add dev eth0 protocol ip parent 1:0 prio 1 u32 match ip dport 25 0xffff flowid 1:12
# tc filter add dev eth0 protocol ip parent 1:0 prio 1 u32 match ip dport 23 oxffff flowid 1:13
{% endhighlight %}

这里，`protocol ip` 表示检查的是报文分组协议字段；`prio 1` 标示相同的优先级。

这几个过滤器还用到了 u32 选择器来匹配不同的数据流。

以第一个命令为例，如果 dport 与 0xffff 进行与操作的结果是 80，那么就把该数据流分配给类别 `1:11` 。

<!--
融合了NS的配置
https://xiaoweiqian.github.io/note/macvlan-qos/
https://guanjunjian.github.io/2017/11/29/study-14-cgroup-network-control-group/
https://ggaaooppeenngg.github.io/zh-CN/2017/05/19/cgroup-%E5%AD%90%E7%B3%BB%E7%BB%9F%E4%B9%8B-net-cls-%E5%92%8C-net-prio/
https://mirrors.deepspace6.net/Linux+IPv6-HOWTO/x2769.html

## 网络

cgroup 的网络隔离提供了两种机制，分别为 net_cls net_prio ，这两个模块分别用来打标以及设置网口的优先级。

### net_cls

用来给改分组中的网络流量打上 classid 的标签，然后通过 TC 进行流控处理，也就是说 classid 需要符合上述的规范，例如 0xAAAABBBB 分别对应了 major 和 minor 。
-->


{% highlight text %}
{% endhighlight %}
