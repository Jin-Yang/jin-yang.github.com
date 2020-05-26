---
title: Linux C 数据对齐
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords: align
description:
---


<!-- more -->


## 对齐操作

为了性能上的考虑，很多的平台都会从某一个特定的地址开始读取数据，比如偶地址。

数据结构中的数据变量都是按照定义的顺序来定义，第一个变量的地址等同于数据结构的地址，结构体中的成员也要对齐，最后结构体也同样需要对齐。对齐是指 **起始地址对齐**，其中对齐规则如下:

1. 数据成员对齐规则<br>结构体(struct)或联合(union)的数据成员，第一个数据成员放在offset为0的地方，以后每个数据成员的对齐按照`#pragma pack`指定的数值n和这个数据成员自身长度中，比较小的那个进行。

2. 结构体(或联合)的整体对齐规则<br>在数据成员完成各自对齐之后，结构体(或联合)本身也要进行对齐，对齐将按照`#pragma pack`指定的数值n和结构体(或联合)最大数据成员长度中，比较小的那个进行。

3. 当`#pragma pack`的n值等于或超过所有数据成员长度的时候，这个n值的大小将不生任何效果。

现举例如下：

#### 1字节对齐

{% highlight c %}
#pragma pack(1)
struct test_t {
    int    a;   // 长度4 > 1 按1对齐；起始offset=0 0%1=0；存放位置区间[0,3]
    char   b;   // 长度1 = 1 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 > 1 按1对齐；起始offset=5 5%1=0；存放位置区间[5,6]
    char   d;   // 长度1 = 1 按1对齐；起始offset=7 7%1=0；存放位置区间[7]
};
#pragma pack()  // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct test_t) = 8
整体对齐系数 min((max(int,short,char), 1) = 1
整体大小(size)=$(成员总大小8) 按 $(整体对齐系数) 圆整 = 8
{% endhighlight %}

#### 2字节对齐

{% highlight c %}
#pragma pack(2)
struct test_t {
    int    a;   // 长度4 > 2 按2对齐；起始offset=0 0%2=0；存放位置区间[0,3]
    char   b;   // 长度1 < 2 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 = 2 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 2 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
};
#pragma pack()  // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct test_t) = 10
整体对齐系数 = min((max(int,short,char), 2) = 2
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 10
{% endhighlight %}

#### 4字节对齐

{% highlight c %}
#pragma pack(4)
struct test_t {
    int    a;   // 长度4 = 4 按4对齐；起始offset=0 0%4=0；存放位置区间[0,3]
    char   b;   // 长度1 < 4 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 < 4 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 4 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
};
#pragma pack() // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct test_t) = 12
整体对齐系数 = min((max(int,short,char), 4) = 4
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 12
{% endhighlight %}

#### 8字节对齐

{% highlight c %}
#pragma pack(8)
struct test_t {
    int    a;   // 长度4 < 8 按4对齐；起始offset=0 0%4=0；存放位置区间[0,3]
    char   b;   // 长度1 < 8 按1对齐；起始offset=4 4%1=0；存放位置区间[4]
    short  c;   // 长度2 < 8 按2对齐；起始offset=6 6%2=0；存放位置区间[6,7]
    char   d;   // 长度1 < 8 按1对齐；起始offset=8 8%1=0；存放位置区间[8]
};
#pragma pack()  // 取消对齐
{% endhighlight %}

{% highlight text %}
输出结果 sizeof(struct test_t) = 12
整体对齐系数 = min((max(int,short,char), 8) = 4
整体大小(size)=$(成员总大小9) 按 $(整体对齐系数) 圆整 = 12
{% endhighlight %}

### \_\_attribute((aligned(n)))

另一种方式是 `__attribute((aligned(n)))` 让所作用的结构成员对齐在 `n` 字节自然边界上，如果结构中有成员长度大于 `n `，则按照最大的成员的长度对齐。

示例如下：

{% highlight text %}
struct test_t {
    int    a;
    char   b;
    short  c;
    char   d;
} __attribute((aligned(n)));
{% endhighlight %}

`__attribute__((packed))` 取消编译过程中的优化对齐，按照实际占用字节数进行对齐。


<!--
采用宏进行对齐操作，计算a以size为倍数的上下界数，
#define alignment_down(a, size)  (a & (~(size - 1)))
#define alignment_up(a, size)    ((a + size - 1) & (~(size - 1)))
-->

详见参考程序 [github align.c]({{ site.example_repository }}/c_cpp/c/align.c) 。


{% highlight text %}
{% endhighlight %}
