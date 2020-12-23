---
title: 详细介绍 glibc 特性宏的使用
layout: post
comments: true
language: chinese
tag: [Linux, Program, C/C++]
keywords: glibc,GNU,C,C++,_DEFAULT_SOURCE,_BSD_SOURCE
description: glibc 是目前 Linux 上最为基本的 C 语言库，不过其实现的不只是标准的 C 定义，还包括了其它的一些标准协议，可以通过一些特性宏来打开不同的实现。
---

GNU C Library 也就是 glibc 库，是目前 Linux 上最为基本的 C 语言库，不过其实现的不只是标准的 C 定义，还包括了其它的一些标准协议，不同场景下用户可以使用不同的接口。

这些特性的开启都是通过宏来实现的，这里详细介绍其使用方式。

<!-- more -->

## 简介

在 glibc 中，实现了很多标准，其中部分规范有：

* `ISO C` The international standard for the C programming language.
* `POSIX` The ISO/IEC ArrayArray45 (aka IEEE 1003) standards for operating systems.
* `Berkeley Unix` BSD and SunOS.
* `SVID` The System V Interface Description.
* `XPG` The X/Open Portability Guide.

如果只需要使用 ISO C 功能，那么在编译时可以使用 `-ansi` 选项，glibc 实现了全部的 ISO C 功能，而 POSIX 是 ISO C 的超集，还包括了文件系统、终端设备、进程相关的函数。

Berkeley Unix 实现了前两者多数功能，源于 4.2 BSD、4.3 BSD、4.4 BSD Unix 系统 (一般称为 Berkeley Unix) 以及 SunOS (基于 4.2 BSD 但是又实现了 System V 的部分功能)，额外包括了符号链接、select IO 复用函数、BSD 信号函数、Socket 接口。

SVID 的全称是 System V Interface Description 描述 AT&T Unix System V operating system 的一份文档，某种意义上是 POSIX 的超集，但是没有已有的 Unix 实现了其全部功能，glibc 也仅仅实现了其部分功能，额外实现的功能包括 IPC、共享内存、hsearch、drand48、fmtmsg 以及一些其它的数学函数。

最后的 The X/Open Portability Guide, XPG 描述了什么样的系统满足类 Unix 系统的基本需要，glibc 服从该要求。

### 使用

在编写程序时，为了使用上述实现的功能，需要定义对应的宏，例如要使用 POSIX 的接口，需要应定义 `_POSIX_SOURCE` ，类似的还有 `_BSD_SOURCE`、`_SVID_SOURCE`、`_XOPEN_SOURCE` 等。

如前所述，可以通过 `-ansi` 参数仅使用 ISO C 内的功能，其它特性使用时可以在所有的头文件前通过 `#define _BSD_SOURCE` 定义宏，或者使用 gcc 编译时通过 `-D_BSD_SOURCE` 参数定义一个宏。

<!--
#define _XOPEN_SOURCE
是为了可以使用 The X/Open Portability Guide 的功能。
-->


<!--
为了处理大文件产生的问题，应定义 _LARGEFILE_SOURCE，又比如 64 位文件系统应定义 _LARGEFILE64_SOURCE。_ISOCArrayArray_SOURCE 允许使用 ISO C ArrayArray 的标准。_GNU_SOURCE 将允许使用全部的 glibc 的功能。

#define _GNU_SOURCE
#include 
#include 

int
main( int argc, char *argv[] )
{
  printf( "%s＼n%s＼n%s＼n", argv[0],
          program_invocation_short_name,
          program_invocation_name ) ;

  return 0 ;
}

该例子给出了如何使用非 ISO C 的 program_invocation_(short_)name 变量（程序名带或者不带路径）的例子。使用 _REENTRANT 和 _THREAD_SAFE 时保证使用对应的函数。
一些笔记：注意的点

1、 头文件里面某些函数的实现使用 macro 做到的，理由是一般这样会更快。但是不需要区别他们，因为一般定义了同名的函数，如 abs(int) 既有函数定义也有 macro 定义，使用 & 操作时，由于 macro 调用时不能匹配到 () 因此仍然能取到函数的地址。如果需要显式的调用函数可以用 () 将函数名包围，如 (abs)(3)，又或者使用 #undef abs 将定义过的 macro 取消。

2、 ISO C 允许重复 #include 头文件，但是 glibc 使用了 #define #ifndef 等预处理器命令避免了同一头文件重复装载，因此可以放心的 #include glibc 的头文件。

3、 除了 ISO C 定义的一些保留字，由于 glibc 实现了一些额外的功能，建议不要使用可能与之冲突的一些命名方式。

4、. error handling 模型，这种依赖于全局变量 errno 的做法 ms 被 BS bs 了 -.-b 指责的原因是不能处理多线程倒是文档中解释了如何处理 signal handling 时 errno 的处理（先备份，再修改以免丢失），但是如果忘了这样做可能会把一些潜在的错误覆盖掉，其错误的定义在 errno.h 文件中。可使用 string.h 里面的 strerror( int errno ) 获得该 errno 下的一个字符串解释，但是有更简单的函数，如 perror( const char * ) 输出指定字符串后加上一个冒号和空格再输出错误解释（errno.h）。有一些功能更强大的函数 strerror_r 将字符串写入指定的缓存（多线程时需要），另外为了使得输出更统一（符合 GNU Coding Standard），在 error.h 中定义了一些更加方便的函数，如 error 系列 err 系列，在解析文件时可以用到的 error_at_line 系列、warn 系列。注意 error 函数可以指定一个调用 exit 的值，
这样就不会返回了，但是可以让其返回增大 error_message_count。
-->

## 其它

### _DEFAULT_SOURCE

在比较新的版本中 (严格来说是大于 2.10 版本)，如果继续使用 `_BSD_SOURCE` 或者 `_SVID_SOURCE` 会有如下的报错 `warning "_BSD_SOURCE and _SVID_SOURCE are deprecated, use _DEFAULT_SOURCE"` 。

也就是说，在新版本中建议使用 `_DEFAULT_SOURCE` 而不是 `_BSD_SOURCE` 宏，不过这样会有一个问题，同一份代码，在新旧 glibc 版本中如何进行适配。

目前来看，没有找到太好的办法，需要在 CMake 类似的工具里进行判断。

<!--
如下的方法是错误的，没有起到想要的效果。

简单来说，就是需要根据 glibc 的版本，来定义不同的宏，代码如下。

``` c
#include <features.h>

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 10
	#ifndef _DEFAULT_SOURCE
	#define _DEFAULT_SOURCE
	#endif
#else
	#ifndef _BSD_SOURCE
	#define _BSD_SOURCE
	#endif
#endif
```

在 `<features.h>` 头文件中，通过 `__GLIBC__` 和 `__GLIBC_MINOR__` 两个宏定义了当前 glibc 的版本号。
-->


{% highlight text %}
{% endhighlight %}
