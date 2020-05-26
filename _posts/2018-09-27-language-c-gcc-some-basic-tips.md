---
title: Linux GCC 常用技巧
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords: align
description:
---


<!-- more -->

## 预定义宏

在 GCC 编译其内部定义了一些通用的宏定义，用户可以据此对不同的平台进行适配，例如大小端机器等，所有支持的宏可以通过入选方式查看。

{% highlight text %}
$ gcc -dM -E - < /dev/null
#define __DBL_MIN_EXP__ (-1021)
#define __FLT32X_MAX_EXP__ 1024
#define __UINT_LEAST16_MAX__ 0xffff
#define __ATOMIC_ACQUIRE 2
#define __FLT128_MAX_10_EXP__ 4932
#define __FLT_MIN__ 1.17549435082228750796873653722224568e-38F
#define __GCC_IEC_559_COMPLEX 2
#define __UINT_LEAST8_TYPE__ unsigned char
... ...
{% endhighlight %}

其中 `-dM` 可以生成预定义的宏信息，"-E" 表示预处理操作完成后就停止，不再进行下面的操作，这与命令 `echo | gcc -dM -E -` 的作用相同。


## 64bits 编译运行 32bits

需要安装 32 位基础的 glibc 库，例如 CentOS 中的 `glibc-devel.i386` 、Ubuntu 中的 `libc6-dev-i386`、OpenSUSE 中的 `glibc-devel-32bit` 等等，否则在编译的时候会报 `gnu/stubs-32.h` 文件不存在。

然后在编译的时候添加 `-m32` 参数，链接时使用 `-m elf_i386` 参数。

{% highlight text %}
$ cat main.c
#include <stdio.h>

void main(void)
{
	printf("sizeof(long) = %ld\n", sizeof(long));
}
$ gcc main.c -o main
$ file main
main: ELF 64-bit LSB executable, Intel 80386, version 1 (SYSV), dynamically linked... ...
$ ./main
sizeof(long) = 8

$ gcc -m32 main.c -o main
$ file main
main: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), dynamically linked... ...
$ ./main
sizeof(long) = 4
{% endhighlight %}



{% highlight text %}
{% endhighlight %}
