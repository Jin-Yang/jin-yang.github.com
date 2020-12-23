---
title: 浮点数的前世今生
layout: post
usemath: true
comments: true
language: chinese
tag: [Linux, Program, Others]
keywords: Linux,GMP,浮点数
description: 在计算机中计算 0.1 + 0.2 时，结果并不是想象中的 0.3 ，而是一个非常近似的但又略有区别的值，也就是 0.30000000000000004 。为什么？这其实跟计算机的二进制表示方式有关，这里就详细介绍到底是什么关系，为什么会出现这种情况。

---

在计算机中，所有的信息都是通过二进制进行保存的，对于浮点数也不例外，但是，因为字节长度有限，这样就很难保证能表示所有的实数。

这也就是造成了 `0.1 + 0.2` 的结果不太符合预期，这里一步步介绍这一问题产生的原因。

<!-- more -->

## 简介

如上，对于 `0.1 + 0.2` 如果在 Python 中计算会输出如下的内容。

```
>>> 0.1 + 0.2
0.30000000000000004
```

正常应该是 `0.3` ，但是后面莫名其妙的多了很多的 0 ，显然不符合我们的预期。其它的还有：

```
>>> 1.4 - 1.1                    # 又是一堆的近似，与上面的类似，0.3无法被以2为底在有限位表示出来
0.2999999999999998
>>> (0.1 + 0.7) * 10
7.999999999999999
>>> 100 - 99.98
0.01999999999999602
>>> 100 * 0.58
57.99999999999999
>>> 0.7 / 0.1
6.999999999999999
>>> 4.0 + 1e+17 - 1e+17          # WTF 不应是4.0吗，实际上是溢出
0.0
>>> 4.0 + 1e+16 - 1e+16          # 这才是对的啊
4.0
```

## 浮点数的来源

计算机采用的晶体管电路，通常只会标识 0/1 ，也就是所谓的二进制表示，也就意味着所有的数据存储、使用、计算等都是以二进制方式进行的。

> 实际上早期的设计是允许单个晶体管标识多个的，也就是划分不同的电压范围，对应了不同的取值，但是在实际使用时容错率很低，而且随着老化可能会导致功能异常，所以，最终就只剩下了二进制的表示方式。

例如整数 `uint16_t data = 123;` 实际保存的是 `0x007b` ，对应二进制为 `00000000 01111011` 。

### 二进制定点表示

那么对于小数来说，同样需要通过二进制来表示，与整数不同的是需要确定小数点具体的位置，假设有长度是一个字节，小数点位于 2 和 3 之间，例如 `10011.110` 对应的值为：

$$
1 \times 2^4 + 0 \times 2^3 + 0 \times 2^2 + 1 \times 2^1 + 1 \times 2^0 + 1 \times 2^{-1} + 1 \times 2^{-2} + 0 \times 2^{-3} = 16 + 2 + 1 + 0.5 + 0.25 = 19.75
$$

因为有很多实数是无法通过有限位表示的，那么这种固定小数点带来的缺点是，如果比较靠近高位，那么精度会增加，但表示的范围会减小；而靠近低位时，精度会降低，而范围会增加。

所以，对于这种小数点固定的方案，范围和精度只能二选一。

### 小数浮点表示

所谓的浮点数，其实是用科学记数法表示，例如对于十进制来说，例如 3000 表示成科学计数法就是 $3 \times 10^4$ ，也就是以 10 为底数 4 为指数，在 IEEE754 中类似，只是底数是 2 。

所以，一般来说会通过如下方式表示。

$$
V=(-1)^S \times M \times 2^E
$$

各个位分别为：

* $(-1)^S$ 表示符号位，当 $S=0$ 时对应的值 $V$ 是正数；当 $S=1$ 时对应的值 $V$ 是负数。
* $M$ 表示有效数字，其范围是大于等于 1 且小于 2 。
* $2^E$ 中的 E 表示指数位。

例如，十进制的 $5.0$ 写成二进制就是 $101.0$ 对应的科学表示是 $1.01 \times 2^2$ ，于是按照上述的表示格式，可以得到 $S=0$、$M=1.01$、$E=2$ ；同样，对于十进制的 $-5.0$ 写成二进制是 $-101.0$ 相当于 $-1.01 \times 2^2$ ，于是，$S=1$、$M=1.01$、$E=2$ 。

{% include ads_content01.html %}

## 标准规范

其中在 IEEE754 中最常用的浮点数值表示法是单精确度 (32-bits) 以及双精确度 (64-bits)，不过有些语言是不区分的，例如 JavaScript ，实际使用的就只有双精度浮点数。

单精度的划分如下：

![floating point IEEE754 32bits format](/{{ site.imgdir }}/linux/floating-point-IEEE-754-32bits-format.png "floating point IEEE754 32bits format")

双精度的划分如下：

![floating point IEEE754 64bits format](/{{ site.imgdir }}/linux/floating-point-IEEE-754-64bits-format.png "floating point IEEE754 64bits format")

按照如上的区分，直接查看 $0.5$ 时，对应的二进制是 `01000000 10100000 00000000 00000000` ，显然不符合上面的介绍。

其实 IEEE754 对上述的表示方法稍微做了调整。

### IEEE-754 规范

在 IEEE-754 的规范中，对于有效数字 M 和指数 E 有一些特殊的优化。

对于 M 来说，如前所述，其中 $1 \leqslant M \lt 2 $ ，也就是说整数部分的 1 是必然存在的，那么就可以将 1 省略，只保留小数部分。例如上述的 $5.0$ 对应 M 为 $1.01$ ，可以只保留 $01$ ，这样可以节省一个有效位。

而指数 E (无符号整数) 会复杂一些，对于单精度取值范围是 `0~255` 而双精度是 `0~2047`，而在科学计数法中会出现负数的，所以，转换为真实值时需要减去一个中间值 (保存时需要加上)，分别是 127 和 1023 。

对于上面的 $5.0$ 其指数是 2 ，会被表示为 $2 + 127 = 129$ ，也就是 $10000001$ ，这也就是上面二进制的表示方式。

另外，对于指数 E 还有两种特殊情况：

* 全 0 ，在转换为真实值时，有效数字 M 将不再加上第一位的 1 ，而是还原为 `0.XXX` 的小数，从而可以表示零或者接近零的很小数字。
* 全 1 ，如果有效数字 M 全为 0 ，表示正负无穷大 (取决于符号位)；如果 M 不全为 0，表示这个数不是一个数，也就是 `NaN` 。

也就是说，除了正常的浮点数之外，还有两个比较特殊的值，无穷大 inf(infinity) 以及非数值 NaN(Not a Number) 。例如 `1.0/0.0=inf` `-1.0/0.0=-inf` `0.0+inf=inf` `0.0/0.0=NaN` ，当然还有其它的一些异常场景。

### 根本原因

回到之前的问题，之所以会有精度的差异，实际就是因为要用有限位表示小数带来的误差，可以一步步进行分析，不过实在是太过枯燥了，所以，暂时放弃。

{% include ads_content02.html %}

## 其它

### 高精度计算

上面已经讨论了浮点数所带来的精度问题，那么对于一些需要高精度的科学计算应该如何处理？PS. 其实金融里面完全可以用整数表示，无非是将小数点后移两位即可。

其实有很多高精度的数学计算库，其中比较常用而且速度比较快的可以使用 [GMP](https://gmplib.org/) 库。

在 CentOS 中直接安装 `gmp-devel` 包即可，如下是一个简单示例，编译时添加 `-lgmp` 参数即可。

<!-- https://github.com/brick/math PHP的 -->

``` c
#include <gmp.h>

int main(void)
{
        mpf_t a, b, c;

        mpf_init(a);
        mpf_init(b);
        mpf_init(c);

        mpf_set_str(a, "0.1", 10);
        mpf_set_str(b, "0.2", 10);

        mpf_add(c, a, b); // c = a + b
        gmp_printf("got result %Ff\n", c);

        mpf_clear(c);
        mpf_clear(b);
        mpf_clear(a);

        return 0;
}
```

### 测试代码

如下是 C 实现的代码，可以将指定的数值转换为二进制表示。

``` c
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

// including NULL termination
int binary_format(const unsigned char *in, int ins, unsigned char *out, int outs)
{
        int i, j;
        unsigned char ch;
        int idx = ins * 9 - 1 + 1;

        if (idx >= outs)
                return -EINVAL;

        idx = 0;
        for (i = ins - 1; i >= 0; i--) {
                ch = in[i];
                for (j = 0; j < 8; j++, idx++) {
                        if (ch & 0x80)
                                out[idx] = '1';
                        else
                                out[idx] = '0';
                        ch <<= 1;
                }
                printf("%d\n", idx);
                out[idx++] = ' ';
        }
        out[idx - 1] = 0;

        return 0;
}

int main(void)
{
        char buff[1024];

        //char data = 'A';     // 0x41 01000001
        //uint16_t data = 123; // 0x7b 00000000 01111011

        float data = 5.0; //      01000000 10100000 00000000 00000000
        data = 1.0/0.0;   // inf  01111111 10000000 00000000 00000000
        data = -1.0/0.0;  // inf  11111111 10000000 00000000 00000000
        data = 0.0/0.0;   // NaN  11111111 11000000 00000000 00000000
        data = 0.0;       //      00000000 00000000 00000000 00000000
        data = 1.4;

        binary_format((char *)&data, sizeof(data), buff, sizeof(buff));
        printf("original %.50f\ngot '%s'\n", data, buff);

        return 0;
}
```

可以查看整数、浮点数的二进制保存内容。

<!--
### 浮点数压缩算法
/post/beringei-memory-database-introduce.html
https://github.com/MrBean818/gorilla-paper-encode


https://zhuanlan.zhihu.com/p/58731780
https://www.barretlee.com/blog/2016/09/28/ieee754-operation-in-js/
https://www.ruanyifeng.com/blog/2010/06/ieee_floating-point_representation.html

FLT_MAX DBL_MAX
https://timroderick.com/floating-point-introduction/
-->

{% highlight text %}
{% endhighlight %}
