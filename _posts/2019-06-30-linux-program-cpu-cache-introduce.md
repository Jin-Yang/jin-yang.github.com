---
title: CPU Cache
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords:
description:
---

CPU 的发展因为受制于制造工艺和成本限制，CPU 的频率、内存的访问速度都没有太多质的突破，但是两者直接的访问处理速度相差越来越大。

所以，目前在 CPU 和内存之间包含了多级缓存，用来提高 CPU 的处理速度。

<!-- more -->

![intel cpu chips]({{ site.url }}/images/hardware/intel_cpu_chips.jpg "intel cpu chips"){: .pull-center width="70%" }

## 简介

在 Linux 中可以通过 `lscpu` 命令查看 CPU 相关信息。


## Cache Line

所谓的 Cache Line 可以简单理解为 CPU Cache 中的最小缓存单位，目前主流的 CPU 的 Cache Line 大小都是 64Bytes 。



例如，假设有一个 1M 字节的一级缓存，那么按照64B的缓存单位大小来算，这个一级缓存所能存放的缓存个数就是512/64 = 8个。具体参见下图：

## 参考

* [CPU 性能天梯图](http://www.mydrivers.com/zhuanti/tianti/cpu/) 以及 [ZOL CPU 天梯图](https://cpu.zol.com.cn/soc/)。

{% highlight text %}
{% endhighlight %}
