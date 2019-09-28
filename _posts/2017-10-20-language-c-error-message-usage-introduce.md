---
title: Linux C 错误信息
layout: post
comments: true
language: chinese
category: [misc]
keywords: language,c,errno,strerror
description: 在 C 代码中，当发生错误时，一般会设置 errno ，然后再通过 strerror 打印错误信息，以方便定位问题。但是有些错误打印函数标示为安全的？这是什么意思？使用时应该注意什么？
---

在 C 代码中，当发生错误时，一般会在库函数中设置 errno ，然后应用可以通过 strerror 打印详细的错误信息，以方便定位问题。

但是有些错误打印函数标示为安全的？这是什么意思？使用时应该注意什么？

<!-- more -->

## 简介

详细可以查看 `man 3 errno` 中的介绍，这里简单整理一些常见的注意事项。

#### 使用时机

只有当系统调用或者 C 库函数出错时，才会设置 `errno`，异常值为非零，但是 **不会设置为 0** 。也就意味着，**必须当确定异常** 时才可以使用 `errno` ，否则 `errno` 可能是无效的。

#### 异常判断

一般来说，异常时的返回值为 `-1` 或 `NULL` ，但是有些函数，如 `getpriority()`，返回 `-1` 仍然也被认为是正常。

这是可以在调用该函数之前将 `errno` 设置为 0 ，然后在调用后判断 `errno` 是否为非零。

#### 线程安全

当前的 `errno` 一般是线程安全的，也就意味着，当程序在一个线程内修改 `errno` 不会影响到其它线程。也就是说，不能私自定义 `<errno.h>` 或者通过 `extern int errno` 重新定义 `errno` 。

#### 错误保存

`printf()` `perror()` 甚至 `strerror()` 也可能会修改 `errno`，所以，如果需要，应该在调用该函数之前，存储临时值。

## 错误打印

为了方便排查问题，通常会将 `errno` 转换为可读的错误信息，一般使用的是 `strerror()` 函数，对应的函数声明如下。

{% highlight c %}
#include <string.h>

char *strerror(int errnum);
char *strerror_r(int errnum, char *buf, size_t buflen);
{% endhighlight %}

其中，`strerror_r` 是一个安全版本，根据返回值不同，其实还有另外的一个版本，不过建议使用这个吧，如果超过了 `buf` 指定的长度，同样会在末尾添加一个 `\0` 终止符。

基础版本的使用示例如下。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
        int fd;

        fd = open("/tmp/test.txt", O_RDONLY);
        if (fd < 0) {
                fprintf(stderr, "open '/tmp/test.txt' failed, %d:%s.\n",
                                errno, strerror(errno));
                return -1;
        }

        close(fd);

        return 0;
}
{% endhighlight %}

正常来说，直接通过错误码，然后返回一个指向静态错误信息的字符串指针即可，有啥安全不安全的！！！

### 源码解析

`strerror()` 是否是安全的？简单来说，某些情况下是安全的。

如果参数 `errnum` 是一个已知的 `errno`，那么该函数是绝对安全的，使用的是静态的字符串，也就是会正常返回，不会出现乱码。

在 `__strerror_r()` 函数中，如果满足上述条件，会直接通过如下语句返回，此时就是安全的。

{% highlight c %}
return (char *) _(_sys_errlist_internal[errnum]);
{% endhighlight %}

那所谓的不安全实际上是指，返回的结果可能会出现乱码。

如果 `errnum` 可能是用户传入的任意一个值，那么此时就可能会在 `strerror()` 函数中申请一个全局的内存，而且会返回这个内存指向的指针，这就可能会导致不是信号安全。

### 安全版本

所谓的安全版本，实际上就是调用的上述 `__strerror_r()` 函数，而通过用户控制的缓存来保证安全性。

但是，这样操作起来会比较麻烦，要么需要占用栈的空间或者堆的空间。

## 其它

### GNU C 扩展

在通过 `printf` 打印时，GNU C 支持 `%m` 格式，用于打印 `errno` 错误码对应的字符串，效果与 `strerror_r` 相同。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
        int fd;

        fd = open("/tmp/test.txt", O_RDONLY);
        if (fd < 0) {
                fprintf(stderr, "open '/tmp/test.txt' failed, %d:%m.\n",
                                errno);
                return -1;
        }

        close(fd);

        return 0;
}
{% endhighlight %}

### errno

上面说 `errno` 是线程安全的，指的是大多数的发布版本，如果是用户自己编译的版本，那么就有可能会是不安全的。

{% highlight c %}
#  if !defined _LIBC || defined _LIBC_REENTRANT
/* When using threads, errno is a per-thread value.  */
#   define errno (*__errno_location ())
#  endif
{% endhighlight %}

也就是说，需要确保上述的宏定义才可以。

## 总结

所以，如果确认给的都是合法的 `errno`，可以认为是安全的，可以直接定义如下的宏。

{% highlight c %}
#define strerror(x) strerror_r((x),NULL,0)
{% endhighlight %}

注意，编译时需要添加 `-D_GNU_SOURCE` 参数。

那么可以通过如下的宏确定打印的函数是否为预期的，也就是保证 A) 确保 `strerror_r()` 返回字符串；B) `errno` 是线程安全。

{% highlight c %}
#include <stdio.h>

int main(void)
{
#ifdef _GNU_SOURCE
        printf("_GNU_SOURCE defined\n");
#endif
#if !defined _LIBC || defined _LIBC_REENTRANT
        printf("per-thread errno\n");
#endif
        return 0;
}
{% endhighlight %}

为了防止异常错误码出现，输出错误的时候，同时打印 `errno` 。

{% highlight text %}
{% endhighlight %}
