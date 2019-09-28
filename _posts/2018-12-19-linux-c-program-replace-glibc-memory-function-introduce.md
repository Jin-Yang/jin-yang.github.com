---
title: 替换 glibc malloc
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: glibc,malloc
description: 在进行系统优化、内存泄漏测试时，经常需要对 glibc 的一些 API 进行替换，例如内存管理的接口。 如果代码量很大，或者使用了三方的静态库时，此时就无法直接替换相关的函数，例如 `malloc()` `realloc()` `calloc()` 等，需要直接替换掉系统的相关函数。这里简单介绍几种方法。
---

在进行系统优化、内存泄漏测试时，经常需要对 glibc 的一些 API 进行替换，例如内存管理的接口。

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

glibc 里面的大部分 API 实际上都是 Weak Alias ，可以直接实现一个相同的接口即可。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

extern void *__libc_malloc(size_t size);
void *malloc(size_t size)
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

如果要使用 glibc 原生的接口，那么可以通过 `dlsym(RTLD_NEXT, "malloc")` 或者 `__libc_malloc()` 都可以。

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
