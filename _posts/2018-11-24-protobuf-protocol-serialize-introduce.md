---
title: Protobuf 序列化详解
layout: post
comments: true
language: chinese
category: [misc]
keywords: protobuf
description:
---

类似于 JSON、XML 格式，Protocol Buffer 是 Google 出品的一种轻量而且高效的结构化数据存储格式，性能比 JSON、XML 要强很多，包括其序列化、反序列化速度，数据压缩效果。

当然，带来的问题是可读性不高。

<!-- more -->

![protobuf introduce]({{ site.url }}/images/programs/protobuf-format.png "protobuf introduce"){: .pull-center width="70%" }

## 简介

ProtoBuf 实际上是通过 TLV 进行保存，也就是 `Tag Length Value` 方式进行存储，通过标识、长度、字段值来表示单个数据，然后将所有数据拼接成一个字节流，从而实现数据的序列化功能。

当然，其中的 Length 是可选的。

## 参考

其它相关的 C 实现可以参考 [Nanopb protocol buffers with small code size](http://jpa.kapsi.fi/nanopb/) 中的实现，另外还有 minipb 。

{% highlight text %}
{% endhighlight %}
