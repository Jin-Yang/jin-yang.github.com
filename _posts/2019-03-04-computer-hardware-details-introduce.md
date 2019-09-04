---
title: CPU 硬件介绍
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords:
description:
---

经常能够看到 2-ways 4-ways Cache，它是什么意思呢？Cache 和内存地址是怎么对应起来的呢？

<!-- more -->

![cache hierarchy]({{ site.url }}/images/hardware/cache-hierarchy-intel-i5-m450.svg "cache hierarchy"){: .pull-center }






## Cache

目前大多数处理器的 Cache 被分成多行，也就是 Cache Line ，这是 Cache 操作的基本单位，从 16B 到 256B 不等，一般大小是 64B ，如果 L1D 的缓存大小为 32KB ，那么就有 512 个 Cache Line 。

当程序访问某个地址时，需要经过虚拟地址到物理地址的映射，那么这就涉及到了页表以及 TLB ，只在转换到了物理地址之后才会涉及到 Cache 相关的内容。

### 映射关系

也就是 Cache 到物理内存的映射。

#### 直接映射

Direct Mapped Cache 这也是最简单粗暴的例子，也就是每个地址可以直接且只能映射到某个 Cache Line 上。

假设内存大小为 16K ，每四个字节作为一个 Block ，Cache 的大小为 256 字节，每个 Block 大小同样为四个字节；如果要对 16K 内存进行寻址，那么需要 14 位地址。

通过 Index 定位到 Cache 中的偏移，Offset 对应了 Cache Line 中的偏移，而 Tag 则对应了内存地址的高位，也就是每个 Cache Line 可能会缓存不同的 64 个地址中的一个。

https://upload.wikimedia.org/wikipedia/commons/a/ab/Direct-Mapped_Cache_Snehal_Img.png

#### 全映射

Fully Associative Cache 也就是每个 Cache 可以映射到内存中的任意一个内存中地址。

https://upload.wikimedia.org/wikipedia/commons/9/9c/Fully-Associative_Cache_Snehal_Img.png

#### Set 映射

Set Associative Cache 将直接映射又做了逻辑分组。



https://upload.wikimedia.org/wikipedia/commons/7/71/Set-Associative_Cache_Snehal_Img.png


## 其它

### Linux 查看

可以通过如下的方式查看。

{% highlight text %}
----- 如果启动时间比较短，可以通过如下方式查看
# dmesg | grep cache

----- 比较详细的硬件信息，包括了Cache的详细信息
# dmidecode

----- 查看硬件信息，例如CPU、内存等
# lshw

----- 查看CPU相关信息，两者比较类似
# lscpu
# cat /proc/cpuinfo
{% endhighlight %}

另外，由专门针对 x86 信息的程序，也就是 `x86info` ，可以直接安装对应的包。

注意，现在多数的 CPU 采用的是超线程，也就是说对于一个物理核来说，对于内核看到的是两个，而实际的物理核是一个。


<!--
另外还有个库 libosinfo

包含了一个逻辑图
https://diego.assencio.com/?index=614d73283d49e939ebfb648cfb86819d
-->



https://zhuanlan.zhihu.com/p/31859105
https://en.wikipedia.org/wiki/Cache_placement_policies

关于计算机不错的介绍
http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.115.1881&rep=rep1&type=pdf




{% highlight text %}
{% endhighlight %}
