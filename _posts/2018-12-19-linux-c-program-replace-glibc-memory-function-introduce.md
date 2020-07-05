---
title: 替换 glibc malloc
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: glibc,malloc
description: 在进行系统优化、内存泄漏测试时，经常需要对 glibc 的一些 API 进行替换，例如内存管理的接口。 如果代码量很大，或者使用了三方的静态库时，此时就无法直接替换相关的函数，例如 `malloc()` `realloc()` `calloc()` 等，需要直接替换掉系统的相关函数。这里简单介绍几种方法。
---

在进行系统优化、内存泄漏测试时，经常需要对 glibc 的一些 API 进行替换，例如比较常见的是内存管理接口。

如果代码量很大，或者使用了三方的静态库时，此时就无法直接替换相关的函数，例如 `malloc()` `realloc()` `calloc()` 等，需要直接替换掉系统的相关函数。

这里简单介绍几种方法。

<!-- more -->

## LD_PRELOAD

使用动态库时，环境变量 `LD_PRELOAD` 所指定程序在运行时会优先加载，这个动态库中的符号优先级最高，该库中的函数将会替换掉 glibc 中的相关函数，例如 `malloc()` 。

例如，在使用 MySQL 时可以通过该变量，将内存管理库替换为 `jemalloc` 或者 `tcmalloc` 。

## hook

glibc 提供了 `__malloc_hook`、`__realloc_hook`、`__free_hook`、`__memalign_hook` 四个钩子函数，当相关的 API 调用时会同时调用这四个指针指定的函数。

不过最新版本的 glibc 中已经取消掉了这些钩子函数，使用方法可以查看 `man 3 malloc_hook` 。

## Weak Alias

glibc 里面有很大的一部分 API 都是 Weak Alias 类型的，可以通过 `nm /usr/lib64/libc-2.17.so  | grep ' W '` 命令查看，例如 `wait` `write` 等。

不过 `malloc()` 函数不是，但如果使用的动态链接，仍然可以自己单独定义一个相同的 `malloc()` 函数，例如如下的示例。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

extern void *__libc_malloc(size_t size);
void *malloc(size_t size)
{
        printf("Try to malloc %ld bytes\n", size);
        return __libc_malloc(size);
}

int main(void)
{
        void *ptr;
        ptr = malloc(1);
        free(ptr);

	return 0;
}
{% endhighlight %}

如果要使用 glibc 原生的接口，可以通过 `dlsym(RTLD_NEXT, "malloc")` 或者 `__libc_malloc()` 函数都可以，不过后者不建议。

注意，不能通过 `-static` 参数使用静态链接，否则会报 `multiple definition of 'malloc'` 的错误，至于原因后面讨论。

### 静态链接

如果函数对应的是 `Weak Alias` ，那么可以兼容静态链接方式，例如 `getuid()` 函数。

{% highlight c %}
#include <stdio.h>
#include <sys/types.h>

uid_t getuid(void)
{
        return 0;
}

int main(void)
{
        return printf("uid is %d\n", getuid());
}
{% endhighlight %}

此时直接通过 `gcc -Wall -static main.c -o main` 编译不会报错。

### 区别

对于动态库，会在进程加载执行时通过动态链接器 (例如 dlopen) 来确定未定义符号的地址，上述的代码中，因为程序中已经定义了 `malloc` 函数，所以不会再查找该函数。

而通过 `-static` 表示要包含 glibc 的静态库，原库中已经定义了 `malloc()` 函数，同时在自己的源文件中也定义了该函数，显然重复定义。

对静态链接而言，目前没有找到太有效的解决方法。

## alias

使用 gcc 的 `__attribute__` 关键字来为函数创建别名。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

extern void *__libc_malloc(size_t size);
void *malloc(size_t) __attribute__((alias("my_malloc")));
void *my_malloc(size_t size)
{
        printf("Try to malloc %ld bytes\n", size);
        return __libc_malloc(size);
}

int main ()
{
        void *ptr;

        ptr = malloc(1);
        free(ptr);
}
{% endhighlight %}

上述代码的含义是，为 `my_malloc()` 函数创建了一个函数别名 `malloc`，这样即使是 gcc 内建的 `malloc()` 也就不再可用，所有调用到 `malloc()` 的地方都将调用 `my_malloc()`，也就达到了覆盖的目的。

## wrap

还有一种方式是使用 gcc 提供的 wrap 功能，详细参考 [Linux C Mock Wrap]({{ production_url }}/post/linux-c-mock-wrap-unit-test.html) 中的相关介绍。

{% highlight text %}
{% endhighlight %}
