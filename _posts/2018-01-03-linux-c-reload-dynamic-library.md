---
title: Linux C 动态库加载
layout: post
comments: true
language: chinese
category: [misc]
keywords: linux,c,dynamic,library,reload
description: 简单来说，对于正在运行的程序，当尝试重新加载动态库时，可能会导致程序 CoreDump 。这里简单解释下其原因，以及规避方案。

---

简单来说，对于正在运行的程序，当尝试重新加载动态库时，可能会导致程序 CoreDump 。

这里简单解释下其原因，以及规避方案。

<!-- more -->

## 简介

在加载动态库时，主要通过如下方式进行测试。

1. 加载执行关闭，cp覆盖文件，加载执行，正常。
2. 加载执行不关闭，cp覆盖文件，加载执行，CoreDump。
3. 加载执行不关闭，install覆盖文件，关闭加载执行，正常。
4. 加载执行不关闭，install覆盖文件，加载执行关闭关闭，第二次加载的是同一个文件。

测试程序如下。


{% highlight c %}
/* cat foobar.c */
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>

typedef int (*CAC_FUNC)(int, int);
int main(void)
{
        void *handle1, *handle2;
        char *err;
        CAC_FUNC func = NULL;

        if ((handle1 = dlopen("./libcaculate.so", RTLD_LAZY)) == NULL) {
                fprintf(stderr, "!!! %s\n", dlerror());
                exit(EXIT_FAILURE);
        }

        func = (CAC_FUNC)dlsym(handle1, "add");
        if ((err = dlerror()) != NULL)  {
                fprintf(stderr, "!!! %s\n", err);
                exit(EXIT_FAILURE);
        }
        printf("add: %d\n\n", (func)(2,7));
        dlclose(handle1);

        sleep(10);

        if ((handle2 = dlopen("./libcaculate.so", RTLD_LAZY)) == NULL) {
                fprintf(stderr, "!!! %s\n", dlerror());
                exit(EXIT_FAILURE);
        }

        func = (CAC_FUNC)dlsym(handle2, "add");
        if ((err = dlerror()) != NULL)  {
                fprintf(stderr, "!!! %s\n", err);
                exit(EXIT_FAILURE);
        }
        printf("add: %d\n\n", (func)(2,7));
        dlclose(handle2);

        exit(EXIT_SUCCESS);
}
{% endhighlight %}

{% highlight c %}
/* cat caculate.c */
#include <stdio.h>

static int gint = 0;
int add(int a, int b)
{
        printf("static global member address is %p %p\n", &gint, add);
        return (a + b + 5);
        return (a + b);
}
{% endhighlight %}

{% highlight makefile %}
# Makefile
all:
        gcc -fPIC -shared caculate.c -o libcaculate.so
        gcc -o foobar foobar.c -ldl
{% endhighlight %}

## CoreDump

对于第二个问题产生的原因如下。

### 产生原因

直接复制文件过去，实际上对应的 inode 信息是不变的，可以简单通过 `strace cp new old` 核心内容如下：

{% highlight text %}
open("../libcaculate.so", O_RDONLY)     = 3
fstat(3, {st_mode=S_IFREG|0700, st_size=8064, ...}) = 0
open("libcaculate.so", O_WRONLY|O_TRUNC) = 4
{% endhighlight %}

也即是说，老的动态库文件被 trunc 了，这个过程发生的具体的事情是：

1. 应用程序通过 dlopen 打开 so 的时候，kernel 通过 mmap 把 so 加载到进程地址空间，对应于 vma 里的几个 page；
2. 在该过程中 loader 会把 so 里引用的外部符号，如 malloc printf 等解析成真正的虚存地址；
3. 当 so 被 cp 覆盖时，确切地说是被 trunc 时，kernel 会把 so 文件在虚拟内的页 purge 掉；
4. 当运行到 so 里面的代码时，因为物理内存中不再有实际的数据，此时仅存在于虚存空间内，会产生一次缺页中断；
5. 内核此时会从 so 文件中 copy 一份到内存中去，此时可能会出现：A) 全局符号表并没有经过解析，当调用到时就产生 segment fault；B) 如果需要的文件偏移大于新的 so 的地址范围，就会产生 bus error。

所以，如果用相同的 so 去覆盖，那么 A) 如果 so 里面依赖了外部符号，直接导致 coredump；B) 如果 so 里面没有依赖外部符号，运气不错，不会 coredump 。

所有问题的产生都是因为 so 被 trunc 了一把，所以如果不用 turnc 的方式就避免这个问题。

### 解决方案

可以使用 install 命令，可以通过 `strace install new old` 查看，核心内容如下：

{% highlight text %}
unlink("libcaculate.so")                = 0
open("../libcaculate.so", O_RDONLY)     = 3
fstat(3, {st_mode=S_IFREG|0700, st_size=8064, ...}) = 0
open("libcaculate.so", O_WRONLY|O_CREAT|O_EXCL, 0600) = 4
{% endhighlight %}

可以看到 install 的方式跟 cp 不同，先 unlink 再 creat，当 unlink 的时候，已经 map 的虚拟空间 vma 中的 inode 结点没有变，只有当 inode 结点的引用计数为 0 是，内核才会把它干掉。

也就是新的 so 和旧的 so 用的不是同一个 inode 结点，所以不会相互影响。

## 引用相同文件

对于第四种情况，实际上通过 strace 查看时，不再加载新的动态库，而是直接引用之前的文件。

通过 dlclose() 关闭时，会减少动态库的引用计数，当为 0 的时候才会真正卸载该动态库；可以在执行前添加 `LD_DEBUG=bindings` 环境变量，并查看如下的信息：

{% highlight text %}
binding file <some.so> [0] to libdynamicTest.so.1 [0]: normal symbol `<symbol>'
{% endhighlight %}

实际上，第一次没有关闭，下次重新打开时直接使用上次结果，导致引用函数不变。

<!--
LD_DEBUG=all
-->

## 结论

替换动态库文件时使用 install 命令而非 cp ，在重新加载动态库前先关闭所有的引用，确保动态库被正常卸载。



{% highlight text %}
{% endhighlight %}
