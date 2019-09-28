---
title: Linux C 位域和大小端
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords:  bit field,endian
description: 在处理通讯协议时，经常需要按照字节甚至是位进行处理，那么 C 语言中如何处理呢？另外，网络通讯时采用的是大端，到底是啥意思？
---

在处理通讯协议时，经常需要按照字节甚至是位进行处理，例如 MySQL 协议，那么对于 C 而言如何方便的进行处理呢？ 另外，网络通讯时采用的是大端，到底是啥意思？

<!-- more -->

## 简介

简单来说，可以通过 C 中的位段 (或者称为 "位域") 进行处理。

所谓 "位域" 是把一个字节中的二进位划分为几个不同的区域，并标明每个区域的位数，每个域有一个域名，允许在程序中按域名进行操作。

为了支持跨平台，建议使用类似 `uint32_t` 这类固定长度的类型，在 C 中一般定义在 `stdint.h` 头文件中。

## 位域

在使用时需要注意如下的内容。

* 其中类型必须为整形，不能是浮点类型。
* 可以使用空的位域，此时可以占用空间但是不能直接引用。

### 简单使用

可以参考如下的示例。

{% highlight c %}
#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct header {
        uint32_t length:16;
        uint32_t :8; /* reserved */
        uint32_t sequence:8;
};

int main(void)
{
        int i;
        uint8_t *ptr;
        struct header hdr;

        memset(&hdr, 0, sizeof(struct header));
        hdr.length = 0x10;
        hdr.sequence = 0x22;

        ptr = (uint8_t *)&hdr;
        printf("header size: %ld\n", sizeof(struct header));
        for (i = 0; i < (int)sizeof(struct header); i++)
                printf(" 0x%02X", ptr[i]);
        puts("\n");

        return 0;
}
{% endhighlight %}

## 大小端

当数据类型大于一个字节时，其所占用的字节在内存中的顺序存在两种模式：小端模式 (little endian) 和大端模式 (big endian)，其中 MSB(Most Significant Bit) 最高有效位，LSB(Least Significant Bit) 最低有效位.

例如，`0x1234` 使用两个字节储存，其中 高位字节(MSB) 是 `0x12`，低位字节(LSB) 是 `0x34` 。

{% highlight text %}
小端模式
MSB                             LSB
+-------------------------------+
|   1   |   2   |   3   |   4   | int 0x01020304
+-------------------------------+
  0x03    0x02    0x01    0x00   Address

大端模式
MSB                             LSB
+-------------------------------+
|   1   |   2   |   3   |   4   | int 0x01020304
+-------------------------------+
  0x00    0x01    0x02    0x03   Address
{% endhighlight %}

两种类型的字节序介绍如下：

* 大端字节序。高位字节在前，低位字节在后，也是人类读写数值的方法。
* 小端字节序：低位字节在前，高位字节在后，更适合计算机。

在计算都是从低位开始的，那么计算机先处理低位字节时效率比较高，计算机的内部处理都是小端字节序。但是，人类还是习惯读写大端字节序。

所以，除了计算机的内部处理，其他的场合几乎都是大端字节序，比如网络传输和文件储存。

### 测试程序

如下是一个测试程序。

{% highlight c %}
#include <stdio.h>

void main(void)
{
	int test = 0x41424344;
	char *ptr = (char*)&test;

#ifdef DEBUG
	printf("int  Address:%x Value:%x\n", (unsigned int)&test, test);
	printf("\n------------------------------------\n");

	int j;
	for(j = 0; j <= 3; j++) {
		printf("char Address:%x Value:%c\n", (unsigned int)ptr, *ptr);
		ptr++;
	}
	printf("------------------------------------\n\n");
	pAddress = (char*)&test;
#endif
	if(*ptr == 0x44)
		printf("Little-Endian\n");
	else if(*ptr == 0x41)
		printf("Big-Endian\n");
	else
		printf("Something Error!\n");
}
{% endhighlight %}

如果采用大端模式，则在向某一个函数通过向下类型装换来传递参数时可能会出错。如一个变量为 ```int i=1;``` 经过函数 ```void foo(short *j);``` 的调用，即 ```foo((short*)&i);```，在 foo() 中将 i 修改为 3 则最后得到的 i 为 0x301 。

#### 转换方式

假设，在保存一个二进制文件时采用的是大端字节序，那么读取一个 32bits 的数据方式如下。

{% highlight c %}
uint32_t length = (data[3]<<0) | (data[2]<<8) | (data[1]<<16) | (data[0]<<24);
{% endhighlight %}

对于小端字节序的读取方式如下。

{% highlight c %}
uint32_t length = (data[0]<<0) | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);
{% endhighlight %}

### GCC 使用

实际上 GCC 已经提供了大小端判断和数据转换的函数，可以直接使用。

对于字节序来说，GCC 提供了 `__BYTE_ORDER__` `__ORDER_LITTLE_ENDIAN__` `__ORDER_BIG_ENDIAN__` 这三种预定义的宏，在代码中可以直接使用。

> GCC 会内置了很多的宏定义，可以通过 `gcc -posix -E -dM - </dev/null` 命令进行查看，内置的函数可以查看 [Other Built-in Functions Provided by GCC](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html) 中的介绍。


## 参考

可以参考 [How to teach endian](https://blog.erratasec.com/2016/11/how-to-teach-endian.html) 中的介绍。

<!--
## 实践

综合上述的内容，假设在通讯时采用与网络字节序相同的策略，也就是大端字节序。

endian.h

如果是跨平台的，可以参考
https://github.com/lichray/endian2
https://github.com/rdpoor/endian
https://github.com/hubenchang0515/Endian

https://blog.csdn.net/10km/article/details/49021499
-->

{% highlight text %}
{% endhighlight %}
