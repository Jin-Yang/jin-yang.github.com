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

在编写代码的时候，很大一部分工作是在不同的格式之间进行转换，从外部的数据结构转换成内部使用的结构，例如网络包 (TCP/IP、MySQL协议等)、磁盘文件 (GIF、JPEG等图片格式) 等等。

其中很重要的一部分就是整数的字节顺序问题，也就是当整数的大小超过了一个字节之后，如何进行表示，这就是所谓的字节序的问题。

### CPU

不同的 CPU 对应的字节序略有区别：

* 大端，PowerPC、IBM、Sun、51
* 小端，x86、DEC

其中 ARM 两种模式都可以支持，另外，网络协议中大部分使用的是大端字节序，所以就有一系列的 API 对整数进行转换。

{% highlight text %}
# if __BYTE_ORDER == __BIG_ENDIAN
# define ntohl(x)     (x)
# define ntohs(x)     (x)
# define htonl(x)     (x)
# define htons(x)     (x)
# else
# define ntohl(x)     __bswap_32(x)
# define ntohs(x)     __bswap_16(x)
# define htonl(x)     __bswap_32(x)
# define htons(x)     __bswap_16(x)
# endif
{% endhighlight %}

其中 `ntohs` 为 `network to host short` 的简写，这些函数一般在头文件 `<arpa/inet.h>` 中进行定义，也就是，当前 CPU 为大端则直接使用，为小端时才会进行转换。

如下，详细介绍大小端的概念。

### 大小端

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

如果采用大端模式，则在向某一个函数通过向下类型装换来传递参数时可能会出错。如一个变量为 `int i=1;` 经过函数 `void foo(short *j);` 的调用，即 `foo((short*)&i);`，在 `foo()` 中将 `i` 修改为 `3` 则最后得到的 `i` 为 `0x301` 。

为了支持跨平台，建议使用类似 `uint32_t` 这类固定长度的类型，在 C 中一般定义在 `stdint.h` 头文件中。

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

GCC 内置了 `__BYTE_ORDER__` `__ORDER_LITTLE_ENDIAN__` `__ORDER_BIG_ENDIAN__` 这三种与字节序相关的宏定义，在代码中可以直接使用。

> GCC 会内置了很多的宏定义，可以通过 `gcc -posix -E -dM - </dev/null` 命令进行查看，内置的函数可以查看 [Other Built-in Functions Provided by GCC](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html) 中的介绍。

## 位域

在保存一些信息的时候，并不需要占用一个完整的字节，可能只需要几个二进制位即可，例如一个开关量。这时候，就可以通过 C 语言中的位段 (或者称为 "位域") 进行处理。

所谓 "位域" 是把一个字节中的二进位划分为几个不同的区域，并标明每个区域的位数，每个域有一个域名，允许在程序中按域名进行操作。

{% highlight c %}
struct bitfield {
	int a:8;
	int b:2;
	int c:6;
};
{% endhighlight %}

如下简单介绍其使用方法。

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

### 注意事项

在使用时需要注意如下的内容。

* 其中类型必须为整形，也就是 `int` `unsigned int` `signed int` 三种之一，不能是浮点类型或者 `char` 类型。
* 二进制位数不能超过基本类型表示的最大位数，例如 x64 上的最多不能超过 64 位。
* 可以使用空的位域，此时可以占用空间但是不能直接引用，下个位域从新存储单元开始存放。
* 不能对位域进行取地址操作。

<!--
    6)若位段出现在表达式中，则会自动进行整型升级，自动转换为int型或者unsigned int。
    7)对位段赋值时，最好不要超过位段所能表示的最大范围，否则可能会造成意想不到的结果。
    8)位段不能出现数组的形式。
-->

### 其它

在头文件 `<netinet/ip.h>` 中，有类似如下的结构体定义，将大小端和位域进行耦合适配。

{% highlight c %}
struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;		/* header length */
    unsigned int ip_v:4;		/* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_v:4;		/* version */
    unsigned int ip_hl:4;		/* header length */
#endif
    uint8_t ip_tos;			/* type of service */
    unsigned short ip_len;		/* total length */
    unsigned short ip_id;		/* identification */
    unsigned short ip_off;		/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
    uint8_t ip_ttl;			/* time to live */
    uint8_t ip_p;			/* protocol */
    unsigned short ip_sum;		/* checksum */
    struct in_addr ip_src, ip_dst;	/* source and dest address */
};
{% endhighlight %}

在定义结构体时，有关于大下端的直接优化，可以忽略字节序的转换。其实这种转换对于 CPU 来说开销很小，相比来说分支预测、执行依赖反而会占用更多的 CPU 。

## 参考

可以参考 [How to teach endian](https://blog.erratasec.com/2016/11/how-to-teach-endian.html) 中的介绍。

<!--
综合上述的内容，假设在通讯时采用与网络字节序相同的策略，也就是大端字节序。

如果是跨平台的，可以参考
https://github.com/lichray/endian2
https://github.com/rdpoor/endian
https://github.com/hubenchang0515/Endian
-->

{% highlight text %}
{% endhighlight %}
