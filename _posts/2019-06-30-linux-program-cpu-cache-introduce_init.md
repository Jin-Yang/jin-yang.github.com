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

那么 CPU Cache 也就成了理解计算机体系架构的重要知识点，同样也是并发编程设计中的技术难点。

<!-- more -->

![intel cpu chips]({{ site.url }}/images/hardware/intel_cpu_chips.jpg "intel cpu chips"){: .pull-center width="70%" }

## 简介

在 Linux 中可以通过 `lscpu` 命令查看 CPU 相关信息。

## 实例测试

如下通过一些实例展示一些内存相关特性。

### 访问速度


### Cache Line

所谓的 Cache Line 可以简单理解为 CPU Cache 中的最小缓存单位，目前主流的 CPU 的 Cache Line 大小都是 64Bytes 。

例如，假设有一个 32KB 的一级数据缓存，那么按照 64B 的缓存行来计算，这个一级缓存能够存放 32K/8=4K 。

## 参考

* [Gallery of Processor Cache Effects](http://igoro.com/archive/gallery-of-processor-cache-effects/) 通过代码介绍 CPU Cache 的一些特性。
* [CPU 性能天梯图](http://www.mydrivers.com/zhuanti/tianti/cpu/) 以及 [ZOL CPU 天梯图](https://cpu.zol.com.cn/soc/)。

<!--

https://coolshell.cn/articles/10249.html
-->

{% highlight text %}
{% endhighlight %}
