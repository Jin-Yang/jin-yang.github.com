---
title: Linux Random
layout: post
comments: true
language: chinese
category: [linux, misc]
keywords: linux,dev,random,urandom
description: 在 Linux 中，有两类用于生成随机数的设备，分别是 `/dev/random` 以及 `/dev/urandom` ，其中前者可能会导致阻塞，后者的安全性则较低。后者是 unblocked random 的简称，会重用内部池中的数据以产生伪随机数据，可用于安全性较低的应用。
---

在 Linux 中，有两类用于生成随机数的设备，分别是 `/dev/random` 以及 `/dev/urandom` ，其中前者可能会导致阻塞，后者的安全性则较低。

后者是 unblocked random 的简称，会重用内部池中的数据以产生伪随机数据，可用于安全性较低的应用。

<!-- more -->

## 简介

通过 `/dev/random` 生成随机数时，依赖熵池，如果熵池空了或不够用，那么读取 `/dev/random` 时就会堵塞，直到熵池够用为止。

而读取 `/dev/urandom` 不会堵塞，不过此时 `urandom` 的随机性弱于 `random` 。

### 熵池

熵池本质上是若干字节，在 `/proc/sys/kernel/random/entropy_avail` 中存储了熵池现在的大小，而在 `/proc/sys/kernel/random/poolsize` 是熵池的最大容量，单位都是 bit。

如果 `entropy_avail` 的值小于要产生的随机数 bit 数，那么 `/dev/random` 就会堵塞。

熵池实际上是从各种噪声源 (noice source) 中获取数据，包括了键盘事件、鼠标事件、设备时钟中断等等。而内核从 2.4 升级到 2.6 时，出于安全性的考虑，废弃了一些噪声源，从而导致熵池补给的速度也变慢，进而不够用。

### 问题排查

对一些安全性要求比较高的场景，如果由于熵池不够，那么就会导致程序阻塞，此时可以通过 `strace -p PID` 查看。

一般来说，会阻塞到读取 `/dev/random` 设备上。


## 补充熵池

服务器在运行时，既没有键盘事件，也没有鼠标事件，那么就会导致噪声源减少。在很多发行版本中存在一个 `rngd` 程序，用来增加熵池。

在 CentOS 中，该工具包含在 `rng-tools` 工具中。

可以通过如下命令查看。

{% highlight text %}
----- 查看当前熵池的大小
$ watch cat /proc/sys/kernel/random/entropy_avail

----- 从另外的终端启动服务
# rngd -r /dev/urandom -o /dev/random -f -t 1
{% endhighlight %}

可以看到，很快就会接近上限。


{% highlight text %}
{% endhighlight %}
