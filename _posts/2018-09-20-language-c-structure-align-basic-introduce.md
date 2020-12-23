---
title: Linux C 数据对齐
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords: align
description: C 语言中的结构体包含了各种成员，编译器会将结构体中的每个成员按其自然边界分配空间，合理的设置对齐规则，可以提高 CPU 的访问速度 (有些 CPU 会禁止非对齐，会抛出硬件异常)，降低结构体的占用空间。
---

C 语言中的结构体包含了各种成员，编译器会将结构体中的每个成员按其自然边界分配空间，合理的设置对齐规则，可以提高 CPU 的访问速度 (有些 CPU 会禁止非对齐，会抛出硬件异常)，降低结构体的占用空间。

<!-- more -->

## 简介

所谓的对齐，指的是内存中位置的对齐，通常是出于对性能的考虑，一般来说，如果一个变量的内存地址正好位于它长度的整数倍，就被称做自然对齐。

{% highlight c %}
#include <stdio.h>

void main(void)
{
        char data[] = {0x05, 0x04, 0x03, 0x02, 0x01};
        printf("%x\n", *(int *)(data + 1));
}
{% endhighlight %}

因为没有对齐，访问一个 `int` 类型的变量，实际会分成两次读取，导致效率低下。

对于标准数据类型，只要保证地址是它的长度的整数倍就可以了；数组按照元素类型对齐即可，当第一个元素对齐之后，后面元素自然对齐；联合会按照最大元素对齐。

剩下比较复杂的就是结构体，第一个变量的地址等同于数据结构的地址，另外，结构体中的成员也要对齐，同时，结构体也同样需要对齐。

**自然对齐** 对于基础类型来说，就是具体类型的长度，例如 `short` 为 2 字节，`int` 为 4 字节等 (与编译器相关)；而结构体则是最大的那个值。

### 指定对齐

每个平台上的编译器会有默认的对齐系数，一般 32 位系统是 4 ，而 64 位系统为 8 ，也可以通过如下方式修改某个代码范围的默认对齐方式。

* 通过伪指令 `#pragma pack(n)` 将编译器将按照 `n` 个字节对齐，可以是 `1` `2` `4` `8` `16` 。
* 结束时通过 `#pragma pack()` 取消自定义字节对齐方式。

另外，对于结构体，还可以通过如下方式进行定义。

* `__attribute((aligned(n)))` 结构成员对齐在 `n` 字节自然边界上，如果有成员长度大于 `n` ，则按照最大成员的长度来对齐。
* `__attribute__((packed))` 取消编译过程中的优化对齐，按照实际占用字节数进行对齐。

<!--
注意，`__attribute((aligned))` 实际上使用的默认规则，如果两者都使用，那么 `#pragma` 生效。
-->

### 对齐规则

使用的对齐规则，会选择指定对齐和自然对齐的最小值，包括了数据成员以及整体的数据结构都需要进行对齐：

* 数据成员对齐，第一个数据成员放在 `offset` 为 `0` 的地方，以后每个数据成员的对齐按照 `#pragma pack` 指定的数值 `n` 和这个数据成员长度中较小的那个进行对齐。
* 整体对齐，将按照 `#pragma pack` 指定数值 `n` 和结构体中最大数据成员长度较小的那个进行。

也就是说，当 `#pragma pack` 的 `n` 值大于等于所有数据成员长度的时候，这个 `n` 值的大小将不产生任何效果。

### 位域对齐

位域尽量按照位进行填充，超过了当前类型的大小，会按照对齐规则从新的存储位置开始。

{% highlight c %}
#include <stdio.h>

void main(void)
{
        struct bitfield {
                unsigned char a:5;
                unsigned char b:3;
                unsigned char c:4;
        };

        // sizeof 2
        printf("sizeof %d\n", sizeof(struct bitfield));
}
{% endhighlight %}

如果相邻的类型不同，那么不同的编译器实现略有不同，VC 不会压缩，而 GCC 则会压缩。

{% highlight c %}
#include <stdio.h>

void main(void)
{
        struct bitfield {
                unsigned int a:5;
                unsigned char b:3;
        };

        // sizeof 4
        printf("sizeof %d\n", sizeof(struct bitfield));
}
{% endhighlight %}

另外，当中间穿插了其它类型时，将不再进行压缩；而整个结构体同样是以最大类型进行对齐。

## 示例

基于 64 位平台，使用 `#pragma pack` 和 `__attribute((aligned(n)))` 等效，但是后者会更方便一些，所以直接使用后者。

### 示例代码

可以通过如下代码查看具体的对齐规则。

{% highlight c %}
#include <stdio.h>

void main(void)
{
        struct align {
                int    a;
                char   b;
                short  c;
                char   d;
        } __attribute((aligned(8)));

        printf("__attribute((aligned)): %zd\n"
                "                     a: %d\n"
                "                     b: %d\n"
                "                     c: %d\n"
                "                     d: %d\n",
                sizeof(struct align),
                (&((struct align*)0)->a),
                (&((struct align*)0)->b),
                (&((struct align*)0)->c),
                (&((struct align*)0)->d));
}
{% endhighlight %}

### 1字节对齐

{% highlight text %}
#pragma pack(1)
struct align {
    int    a;   // 长度4 > 1 按1对齐；起始offset=0 0%1=0；存放位置区间[0,3]
    char   b;   // 长度1 = 1 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 > 1 按1对齐；起始offset=5 5%1=0；存放位置区间[5,6]
    char   d;   // 长度1 = 1 按1对齐；起始offset=7 7%1=0；存放位置区间[7]
} __attribute__((packed));
#pragma pack()
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct align) = 8
整体对齐系数 min((max(int,short,char), 1) = 1
整体大小(size)=$(成员总大小8) 按 $(整体对齐系数) 圆整 = 8
{% endhighlight %}

### 2字节对齐

{% highlight text %}
#pragma pack(2)
struct align {
    int    a;   // 长度4 > 2 按2对齐；起始offset=0 0%2=0；存放位置区间[0,3]
    char   b;   // 长度1 < 2 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 = 2 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 2 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
} __attribute((aligned(2)));
#pragma pack()  // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct align) = 10
整体对齐系数 = min((max(int,short,char), 2) = 2
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 10
{% endhighlight %}

### 4字节对齐

{% highlight text %}
#pragma pack(4)
struct align {
    int    a;   // 长度4 = 4 按4对齐；起始offset=0 0%4=0；存放位置区间[0,3]
    char   b;   // 长度1 < 4 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 < 4 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 4 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
} __attribute((aligned(4)));
#pragma pack() // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct align) = 12
整体对齐系数 = min((max(int,short,char), 4) = 4
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 12
{% endhighlight %}

### 8字节对齐

{% highlight text %}
#pragma pack(8)
struct align {
    int    a;   // 长度4 < 8 按4对齐；起始offset=0 0%4=0；存放位置区间[0,3]
    char   b;   // 长度1 < 8 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 < 8 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 8 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
} __attribute((aligned(8)));
#pragma pack()  // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct align) = 12
整体对齐系数 = min((max(int,short,char), 8) = 4
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 12
{% endhighlight %}

<!--
`__attribute((aligned(1)))` 和 `__attribute__((packed))` 不等价，而且 `#pragma pack(8)` 和 `__attribute((aligned(8)))` 计算的整个结构体大小也不相同。
-->


<!--
采用宏进行对齐操作，计算a以size为倍数的上下界数，
#define alignment_down(a, size)  (a & (~(size - 1)))
#define alignment_up(a, size)    ((a + size - 1) & (~(size - 1)))
详见参考程序 [github align.c]({{ site.example_repository }}/c_cpp/c/align.c) 。

-->


{% highlight text %}
{% endhighlight %}
