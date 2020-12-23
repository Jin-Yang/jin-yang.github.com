---
title: SSE 指令集
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: stan
description:
---


<!-- more -->

## 简介

其全称是 Sreaming SIMD Extensions, SSE，是一组 CPU 指令集 (AMD 和 Intel 均支持)，常用于像信号处理、科学计算、3D 图形计算类似的场景，相比来说其处理速度会更快。

在 Linux 中可以通过 `/proc/cpuinfo` 查看是否支持该指令集，该指令集是 x86 所特有 ARM 不支持，跨版本需要考虑其兼容性。

<!--
关于SSE指令集很不错的介绍
https://www.jianshu.com/p/d718c1ea5f22


int posix_memalign(void **memptr, size_t alignment, size_t size);

成功会返回 `size` 字节的动态内存，内存的地址是 `alignment` 的倍数，也就是对齐；类似于 `malloc()` 函数，使用后通过 `free()` 函数释放。
-->

## 示例

通过一个函数累加一个数组，然后逐级进行优化。

### 基本

### 循环展开

也就是将循环展开，提高指令的并行性能。

### SSE

使用SSE指令，首先要了解这一类用于进行初始化加载数据以及将暂存器的数据保存到内存相关的指令，大多数SSE指令是使用的xmm0到xmm8的暂存器，那么使用之前，就需要将数据从内存加载到这些暂存器。

load(set)系列，用于加载数据，从内存到暂存器。


<!--
//----- 从内存加载到SSE的寄存器中
__m128i _mm_load_si128(__m128i *p);
__m128i _mm_loadu_si128(__m128i *p);

//----- 从寄存器加载到内存中
void _mm_store_si128(__m128i *p, __m128i a);
void _mm_storeu_si128(__m128i *p, __m128i a);

返回的是一个设置好的 SSE 寄存器。

https://www.linuxjournal.com/content/introduction-gcc-compiler-intrinsics-vector-processing
https://blog.csdn.net/Reformatsky/article/details/69388772
http://sci.tuomastonteri.fi/programming/sse
## Vector Processing

有一类通用的需求，就是计算一个数组中的成员的累加，可能是整数或者浮点数。对于通用的 CPU ，会依次循环整个数组然后累加，效率很低。

VP 实际会将数组中的多个元素添加到寄存器中进行计算，一般被称为 Single Instruction Multiple Data, SIMD 。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <xmmintrin.h> // x86 SSE1
#include <mmintrin.h>
//#include <pmmintrin.h>
//#include <tmmintrin.h>
#include <emmintrin.h>

int addv1(int *arr, int size)
{
        int sum = 0, i;

        for (i = 0; i < size; i++)
                sum += arr[i];
        return sum;
}

int addv2(int *arr, int size)
{
        int sum = 0, i, block, remain;

        block = size / 4;
        remain = size % 4;
        for (i = 0; i < block; i++, arr += 4) {
                sum += *(arr + 0);
                sum += *(arr + 1);
                sum += *(arr + 2);
                sum += *(arr + 3);
        }
        for (i = 0; i < remain; i++)
                sum += arr[i];
        return sum;
}

#define SIZE 10000000
int main(void)
{
        int *array, rc, i;
        struct timeval beg, end, diff;

        rc = posix_memalign((void **)&array, 16, sizeof(int) * SIZE);
        if (rc != 0) {
                fprintf(stderr, "create align memory failed, %d:%s.",
                                rc, strerror(rc));
                return -1;
        }

        printf("xxxx %d\n", sizeof(int));

        for (i = 0; i < SIZE; i++)
                array[i] = 5;

        gettimeofday(&beg, NULL);
        //rc = addv1(array, SIZE);  // 31042us
        rc = addv2(array, SIZE);  // 18621us
        gettimeofday(&end, NULL);
        timersub(&end, &beg, &diff);

        printf("result %d cosume %ldus\n", rc, diff.tv_sec * (int)10e6 + diff.tv_usec);
        free(array);

        return 0;
}
-->


{% highlight text %}
{% endhighlight %}
