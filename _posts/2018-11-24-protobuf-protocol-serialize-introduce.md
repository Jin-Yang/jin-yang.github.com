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

![protobuf tlv]({{ site.url }}/images/programs/protobuf-tlv-format.png "protobuf tlv"){: .pull-center width="60%" }

支持的格式有。

{% highlight text %}
WireType          Encoding     Length(Bytes)   Method             Type
       0            Varint              1~10      T-V   int32 int64 uint32 uint64
       1            Varint              1~10      T-V   int32 int64 uint32 uint64
       2  Length Delimited            Length    T-L-V   string bytes embeded repeated

       5    32-Bits     4-Bytes          TV      fixed32,sfixed32,float
{% endhighlight %}

### Varint

这是一种变长的编码方式，值越小的数字，使用的字节越少。

例如，对于 int32 类型的数字，一般需要 4 字节，当采用 Varint 编码，对于很小的数值只需要一个字节表示即可，当然，如果很大的值可能需要 5 个字节来表示，不会一般很少出现。

通过每个字节的最高位标示是否有后续的字节：0) 这是最后一个字节，并用剩余 7 位来表示数字；1) 后续的字节也是该数字一部分。

#### Zigzag

不过有个问题是，如果是负数，一般会被表示为很大的整数，为此提供了一种 Zigzag 的编码方式。

### Tag

ProtoBuf 中的 Tag 保存了 WireType 和 FieldNumber 两类信息，例如如下的示例。

{% highlight text %}
message person {
	// wire type = 0，field_number =1
	required int32     id = 1;

	// wire type = 2，field_number =2
	required string    name = 2;
}
{% endhighlight %}

而 Tag 则是通过 `(field_number << 3) | wire_type` 这种方式计算，并通过 Varint 和 Zigzag 进行编码。

## 参考

其它相关的 C 实现可以参考 [Nanopb protocol buffers with small code size](http://jpa.kapsi.fi/nanopb/) 中的实现，另外还有 minipb 。

<!--
https://github.com/squidfunk/protobluff

对应了protobuf-c/protobuf-c.c
https://github.com/protobuf-c/protobuf-c
-->

{% highlight text %}
{% endhighlight %}
