---
title: C 整数介绍
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords: align
description: 在 C 中与整数类型相关的内容。
---

在 C 中与整数类型相关的内容。

<!-- more -->

## 整型大小

基本类型数据所占字节数与机器的字长及编译器有关，也就是说，`int` `long int` `short int` 的宽度都会随编译器、机器字长而有所区别，例如同样是 32 位操作系统，VC++ 中的 `int` 类型是 4 字节，而 TuborC 中的 `int` 类型则是 2 字节。

不过同时几条 ANSI/ISO 制定的规定必须要遵守，一般在 Numerical limits 相关的部分中定义：

* `sizeof(short int)<=sizeof(int)` ；
* `sizeof(int)<=sizeof(long int)` ；
* `short int` 至少应为 2 字节；
* `long int` 至少应为 4 字节；
* `int` 建议为一个机器字长，其中 32 位环境为 4 字节，64 位环境为 8 字节。

与基本类型相关的主要包含了几个基本的头文件：

* `<stdint.h>` 一些固定大小的类型，例如 `uint32_t` `int654_t` `uintptr_t` 等类型；
* `<limits.h>` 基本类型的最大最小值限制，例如 `SHORT_MAX` `INT_MAX` `LONG_MAX` 等；

在机器上具体类型的大小，可以通过如下的代码查看。

{% highlight c %}
#include <float.h>
#include <stdio.h>
#include <limits.h>

void main(void)
{
        printf("Type                Bytes   Range\n");
        printf("              char  %5ld    [%d, %d]\n", sizeof(char), CHAR_MIN, CHAR_MAX);
        printf("     unsigned char  %5ld    [%u, %u]\n", sizeof(unsigned char), 0, UCHAR_MAX);
        printf("             short  %5ld    [%d, %d]\n", sizeof(short), SHRT_MIN, SHRT_MAX);
        printf("    unsigned short  %5ld    [%u, %u]\n", sizeof(unsigned short), 0, USHRT_MAX);
        printf("               int  %5ld    [%d, %d]\n", sizeof(int), INT_MIN, INT_MAX);
        printf("      unsigned int  %5ld    [%u, %u]\n", sizeof(unsigned int), 0U, UINT_MAX);
        printf("              long  %5ld    [%ld, %ld]\n", sizeof(long), LONG_MIN, LONG_MAX);
        printf("     unsigned long  %5ld    [%lu, %lu]\n", sizeof(unsigned long), 0UL, ULONG_MAX);
        printf("         long long  %5ld    [%lld, %lld]\n", sizeof(long long), LLONG_MIN, LLONG_MAX);
        printf("unsigned long long  %5ld    [%llu, %llu]\n", sizeof(unsigned long long), 0ULL, ULLONG_MAX);
        puts("\n");
        printf("             float  %5ld    [%g, %g]\n", sizeof(float), FLT_MIN, FLT_MAX);
        printf("            double  %5ld    [%g, %g]\n", sizeof(double), DBL_MIN, DBL_MAX);
        printf("       long double  %5ld    [%lg, %lg]\n", sizeof(long double), LDBL_MIN, LDBL_MAX);
}
{% endhighlight %}

<!--
C11 标准
http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf

## 字面常量 Literal Constant

这是 C/C++ 词法上的概念，也就是源码中一些表示固定值的符号，通常是一些基本类型，用来直接以值得形式来操作。

### 类型后缀

对于一些类型，


Data Type                    Constant Examples
int                          1 123 21000 -234
long int                     35123L -34L
unsigned int                 10000U 987u 40000U
float                        123.23F 4.34e-3f
double                       123.23 1.0 -0.98763241
long double                  1001.2L
-->



## 类型转换

计算机中的数值通过补码进行表示，对于有符号类型，最高有效位代表了符号类型；而无符号类型，最高位仍然是数据位；同时在转换时，还会涉及到不同类型的大小。

所以总共会有八种类型转换。

{% highlight text %}
// <1> 有符号小 --> 有符号大 无损按照符号位扩展
// <2> 无符号小 --> 无符号大 无损零扩展，高位填充零
// <3> 有符号小 --> 无符号大 无损按照符号位扩展
// <4> 无符号小 --> 有符号大 无损零扩展，高位填充零
{% endhighlight %}

上述仅列举了部分，规则可以总结为。

* 在由短类型到长类型存储时，不会发生丢失数据，而扩展方式是通过原有的数据类型定义。
* 在由短类型到长类型存储时，如果符号不同，会先进行类型扩展，然后原封不动赋值给对方。
* 当从长类型转换为短类型时，如果高位不都一样，也就意味着高位不只是表示符号位，那么就可能会造成数据丢失。
* 对于同一长度类型，则直接赋值，如果符号不同，那么数值的大小就会发生改变。
* 当整型转换为浮点时，会先转换为 long 类型，再转换为浮点。

另外，注意，当无符号和有符号类型比较时，会先将有符号转换为无符号进行比较，当有符号为负数时，转换之后实际为一个很大的值。

{% highlight c %}
#include <stdio.h>

void main(void)
{
        char varnchar = 0xff;                  // -1
        char varuchar = 1;                     //  1

        short varnshort = varnchar;            // <1> 0xff -> 0xffff
        short varushort = varuchar;            // <1> 0x01 -> 0x0001
        printf("small   signed -> large   singned  %5d %5d\n", varnshort, varushort);


        unsigned char uvarnchar = 0xff;        // -1
        unsigned char uvaruchar = 1;           //  1

        unsigned short uvarnshort = uvarnchar; // <2> 0xff -> 0x00ff
        unsigned short uvarushort = uvaruchar; // <2> 0x01 -> 0x0001
        printf("small unsigned -> large unsingned  %5u %5u\n", uvarnshort, uvarushort);


        unsigned short uvarshort = varnchar;   // <3> 0xff -> 0xffff
        printf("small   signed -> large unsingned  %5u\n", uvarshort);

        short varshort = uvarnchar;            // <4> 0xff -> 0x00ff
        printf("small   signed -> large unsingned  %5d\n", varshort);
}
{% endhighlight %}

上面只是基本的概念介绍，下面搞几个题看看。

### 隐式类型转换

{% highlight c %}
#include <stdio.h>

int main(void)
{
        int a = -1;
        unsigned int b = 1;

        if (a > b)
                printf("a > b\n");
        else
                printf("a <= b\n");

        return 0;
}
{% endhighlight %}

输出为 `a > b` ，在比较时，如果没有强制转换，那么默认会转换为无符号类型，而 `-1` 转换后是一个很大的正整数。

如下是一个比较复杂的部分。

{% highlight c %}
#include <stdio.h>

int main(void)
{
        unsigned int a = 20;
        int b = 13;
        int k = b - a;

        printf("k = %d, %u\n", k, b - a);
        if (k < (unsigned int)b + a)
                printf("false A 4294967289 < 13 + 20\n");
        if (k < (int)(b + a))
                printf("true  B -7 < 13 + 20\n");
        if (k < b + (int)a)
                printf("true  C -7 < 13 + 20\n");
        if (k < (b + a))
                printf("false D 4294967289 < 13 + 20\n");

        return 0;
}
{% endhighlight %}

首先 `k = b - a` 的计算实际上等价于 `k = (int)((unsigned int)b - (unsigned int)a) = (int)(4294967289) = -7` ，也就是实际计算是无符号类型，最后将结果强转为有符号类型。

## 整型溢出

简单来说，就是在计算的时候超过了整数表示的最大范围，分为了有符号和无符号类型，两者在定义是略有区别。

* 有符号类型，C 语言规范未定义，不同的编译器实现会略有区别。
* 无符号类型，约束为对溢出结果按照变量类型大小取模。

如下是在 GCC 上的简单示例。

{% highlight c %}
#include <stdio.h>

int main(void)
{
        unsigned char a = 0xff; // 255 UCHAR_MAX
        printf("%d\n", ++a);    // 256 mod 2^8 = 0

        signed char b = 0x7f;   // 127 SCHAR_MAX
        printf("%d\n", ++b);    // 0x80 -> -128

        return 0;
}
{% endhighlight %}

注意，如果是乘法，例如 `0x7f * 0x05` 那么结果就是 `123` 了。

<!--
https://wiki.x10sec.org/
https://coolshell.cn/articles/11466.html
https://wiki.x10sec.org/pwn/integeroverflow/intof/
-->

### 示例

下面那些函数可能会出现整型溢出。

{% highlight text %}
int funcA(short a, short b)
{
        return (a + b);
}

int funcB(signed char a, signed char b)
{
        return (a + b);
}

long long int funcC(int a, int b)
{
        return (a + b);
}

long long int funcD(int a, int b)
{
        return ((long long int)a + b);
}
{% endhighlight %}

`short` 相加时会自动转换为 `int` 类型计算，而 `int` 相加是不会自动扩展为 `long long int` 的，需要显示转换。


{% highlight text %}
{% endhighlight %}
