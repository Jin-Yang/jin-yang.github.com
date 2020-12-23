---
title: LVM 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

在最初对磁盘分区的大小进行规划时，很难确定其真正使用的磁盘空间，那在后续的管理时就非常麻烦，实际上类似的问题可以通过 LVM 进行管理。

Logical Volume Manager, LVM 一般翻译为 "逻辑卷管理"，它是 Linux 下对磁盘分区进行管理的一种机制，在磁盘分区和文件系统之间的一个逻辑层，可以灵活的调整分区大小。

这里简单介绍。

<!-- more -->

## 简介

LVM 的优点有：

* 文件系统可以跨多个磁盘，大小不再受物理磁盘的限制；
* 在系统运行时扔可以动态扩展文件系统大小；
* 可以以镜像的方式冗余重要数据到多个物理磁盘上；
* 可以很方便地导出整个卷组，并导入到另外一台机器上。

LVM 也有一些缺点：

* 在从卷组中移除一个磁盘的需要使用 `reducevg` 命令；
* 当卷组中的一个磁盘损坏时，整个卷组都会受影响；
* 因为中间增加了一个逻辑层，存储的性能会略受影响。

对服务器的管理非常有用，但对于桌面系统的帮助则没有那么显著。

### 基本概念

一些常见的基本概念：

* Physical Media, PM 系统中的存储设备，例如 `/dev/sda` `/dev/sdb` 等。
* Physical Volume, PV 一般是指硬盘分区 (也可以是RAID设备)，例如 `/dev/sda1` `/dev/sdb2` 。
* Volume Group, VG 卷组 有点类似于非 LVM 系统中的物理硬盘，一个 LVM 卷组由一个或者多个 PV 组成。
* Logical Volume, LV 逻辑卷 类似于非 LVM 系统上的磁盘分区，LV 建立在 VG 上，可以在 LV 上建立文件系统。
* Physical Extent, PE PV 中可以分配的最小存储单元，该大小可以指定。
* Logical Extent, LE LV 中可以分配的最小存储单元，在同一个卷组中，LE 的大小和 PE 的大小是一样的，并且一一对应。

<!--
https://linux.cn/article-3218-1.html
https://segmentfault.com/a/1190000014035832
https://www.cnblogs.com/sparkdev/p/10130934.html
https://zhangchenchen.github.io/2017/03/10/LVM-command/
https://www.howtogeek.com/howto/40702/how-to-manage-and-use-lvm-logical-volume-management-in-ubuntu/


Device Mapper 是 Linux 2.6 内核中提供的一种从逻辑设备到物理设备的映射框架机制，在该机制下，用户可以很方便的根据自己的需要制定实现存储资源的管理策略。例如，当前比较流行的逻辑卷管理器 Linux Volume Manager, LVM 管理器，都是基于该机制实现的。

在 Linux 系统中，一般是通过 `dm-XX` 方式表示。
-->

{% highlight text %}
{% endhighlight %}
