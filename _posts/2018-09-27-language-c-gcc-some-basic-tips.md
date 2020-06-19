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

<!--
## 多版本支持

可以通过 `gcc -v 2>&1 | grep 'gcc version'` 命令查看当前默认 GCC 的版本号。
https://learnku.com/articles/39849
https://www.cnblogs.com/alisonzhu/p/4506722.html

-Wimplicit-fallthrough 处理
https://developers.redhat.com/blog/2017/03/10/wimplicit-fallthrough-in-gcc-7/
-->


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

## 检测工具

在 GCC 4.8 之后，提供了很多有用的测试工具，包括了：

* `Address Sanitizer` 可以用来检查内存访问的错误，编译时通过 `-fsanitize=address` 指定；
* `Thread Sanitizer` 检查数据竞争问题，编译时通过 `-fsanitize=thread -fPIE -pie` 指定。




## 其它

### 打印详细命令

通常在引入了一些环境变量时，希望查看具体的命令参数，可以使用 `gcc -### main.c` 类似的方式，与 `-v` 参数的区别是，只打印执行的命令，并不真正执行。

包括了具体的搜索路径信息。

### 打印链接库信息

可以用来查看具体链接到的是哪一个库，通过类似 `gcc -print-file-name=libc.a` 方式查看。

### 优化选项

GCC 提供了很多优化方法，而且不同的优先级对应的优化选项也不同，可以通过 `-Q --help=optimizers` 查看所有的选项，包括默认是否打开，不同优先级可以通过如下方式查看：

{% highlight text %}
$ gcc -Q --help=optimizers -O
$ gcc -Q --help=optimizers -O1
$ gcc -Q --help=optimizers -O2
$ gcc -Q --help=optimizers -O3
$ gcc -Q --help=optimizers -Og
$ gcc -Q --help=optimizers -Os
$ gcc -Q --help=optimizers -Ofast
{% endhighlight %}

### 其它

* `-DDEBUG` 命令行中定义宏，中间可以添加空格，例如 `-D DEBUG` ；通过 `-U` 参数取消定义。
* `-Wall` 打开所有的警告信息。
* `-Werror` 将所有的编译警告信息作为错误，也可以通过 `-Wno-error` 关闭。
* `-fsyntax-only` 只作语法检查，不编译。
* `-fdiagnostics-color` 彩色打印异常信息，也可以通过 `GCC_COLORS` 环境变量设置，在 4.9 之后支持。


{% highlight text %}
{% endhighlight %}
