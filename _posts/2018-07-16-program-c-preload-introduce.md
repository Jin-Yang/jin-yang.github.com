---
title: C Preload 简介
layout: post
comments: true
language: chinese
category: [program,misc]
keywords: c,preload
description: Linux C 中有一个很不错的特性，可以在不改变程序的前提下，修改动态库所调用的函数，也就是 Preload 功能。这里简单介绍其使用方法。
---

Linux C 中有一个很不错的特性，可以在不改变程序的前提下，修改动态库所调用的函数，也就是 Preload 功能。

这里简单介绍其使用方法。

<!-- more -->

## 简介

在 `*NIX` 系统中，`LD_PRELOAD` 是一个环境变量，可以影响程序在运行时的动态链接 (Runtime Linker)，它允许在程序运行前优先加载的动态链接库，也可以在 `/etc/ld.so.preload` 文件中添加。

其实现的功能和 Windows 下通过修改 `import table` 来 `hook API` 很类似，只是更简单一些。

最常见的使用场景是不修改程序，而直接修改动态库中函数的实现，例如重新实现 `malloc()` 和 `free()` 函数。

## 示例

{% highlight c %}
#include <stdio.h>

int main(void)
{
	FILE *fd;

	printf("Calling the fopen() function...\n");

	fd = fopen("test.txt","r");
	if (fd == NULL) {
		printf("fopen() returned NULL\n");
		return 1;
	}
	printf("fopen() succeeded\n");

	return 0;
}
{% endhighlight %}

然后可以通过如下方式编译、执行。

{% highlight text %}
$ gcc foobar.c -o foobar

$ ./foobar
Calling the fopen() function...
fopen() succeeded
{% endhighlight %}

接着我们生成自己定义 `fopen()` 函数。

{% highlight c %}
#include <stdio.h>

FILE *fopen(const char *path, const char *mode)
{
	printf("Always failing fopen\n");
	return NULL;
}
{% endhighlight %}

然后，编译生成动态库。

{% highlight text %}
$ gcc -Wall -fPIC -shared -o libawrap.so awrap.c

$ LD_PRELOAD=./libawrap.so ./foobar
Calling the fopen() function...
Always failing fopen
fopen() returned NULL
{% endhighlight %}

也可以通过如下命令查看符号的查找过程。

{% highlight text %}
LD_DEBUG=symbols ./foobar
{% endhighlight %}

以及其真正依赖的库。

{% highlight text %}
$ ldd ./foobar
        linux-vdso.so.1 =>  (0x00007fffaffe7000)
        libc.so.6 => /lib64/libc.so.6 (0x00007f0c22128000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f0c224f5000)
$ LD_PRELOAD=./libawrap.so ldd ./foobar
        linux-vdso.so.1 =>  (0x00007fff023fe000)
        ./libawrap.so (0x00007fbfa3e08000)
        libc.so.6 => /lib64/libc.so.6 (0x00007fbfa3a3b000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fbfa400a000)
{% endhighlight %}

## 高级用法

假设我们仍然需要调用系统提供的函数，可以使用如下的方法。

{% highlight c %}
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>

FILE *fopen(const char *path, const char *mode)
{
	FILE *(*oopen)(const char *, const char*);

	printf("A wrapped fopen\n");
	oopen = dlsym(RTLD_NEXT, "fopen");
	if (oopen == NULL) {
		fprintf(stderr, "Failed to find fopen\n");
		return NULL;
	}

	return oopen(path, mode);
}
{% endhighlight %}

也就是通过 `dlsym()` 查找下个 `fopen()` 符号。

## 任务白名单

如上所述，`execXXX` 系列函数底层都是通过 `execve()` 系统调用实现，可以通过 preload 替换动态库中的 `execXXX` 函数。

{% highlight text %}
exec.c       用来覆盖execXXX
main.c       主函数
testwrap.sh  测试主脚本
test.sh      测试脚本，被testwrap.sh调用
{% endhighlight %}

实际上，如 `execle()` 的实现可以参考 glibc 代码中 `posix/execle.c` 的实现。

1. Bash脚本支持。大部分脚本在执行时都会调用 `execXXX` 接口，其中部分 (如echo) 直接调用的是系统函数。
2. Python、Perl、Java 等程序，由于这里的程序可以直接调用系统接口，那么很难直接进行限制。

对于调用的接口可以通过 `strace` 或者 `ltrace` 命令查看，关于 `ltrace` 可以查看 [github.com](https://github.com/ice799/ltrace) 。

简单来说，就是需要设置一个环境变量，可以在执行命令之前设置，或者在程序中添加。两者的区别在于，前者会检查本程序内执行的命令，而后者只影响到后续的命令执行，例如 Bash 脚本中的。

{% highlight text %}
$ LD_PRELOAD=./libawrap.so ./test
$ LD_PRELOAD=./libawrap.so strace -ff ./test
{% endhighlight %}

### 其它

一个程序调用了那些函数，可以通过 `nm -u binary` 查看符号表，或者通过 `strace/ltrace` 查看。

<!--
除了通过strace查看程序的系统调用外，还可以通过ltrace查看库的调用(ltrace -S ./hello 同样系统调用)
http://linxiaohui.github.io/2014/08/06/2014-08-06-ltrace-on-linux/

两者都可以统计系统调用耗时(`-c, -T`)、跟踪现在进程(`-p`)、跟踪子进程(`-f`) 。
-->

#### 安全相关

这一方式对于静态编译无效，因为不需要在执行时链接动态库里的函数；如果文件设置了 `SUID` `SGID`，出于安全考虑，在加载时会忽略 `LD_PRELOAD` 变量。

**注意** 设置的 `SUID/SGID` 对应的用户应该与真正运行的用户不同，例如文件用户为 monitor ，如果进程以 monitor 用户运行那么实际上还是有效的。

也可以替换掉默认的 ld 加载器，直接忽略 `LD_PRELOAD` 变量。

#### Permission denied

在脚本执行时，可能会遇到上述的报错，通常来说需要确保路径到执行文件有访问权限。

如果是通过 `./script.sh` 执行，还需要确保文件有执行权限，或者通过 `/bin/bash script.sh` 执行，此时只需要有读取权限就可以了。



<!--
每个进程包含了 `effective uid/gid` ，默认是直接从父进程中继承的，而最开始的父进程一般对应了 `/etc/passwd` 中的字段，当设置了 `SUID/SGID` 之后，对应的 `effective uid/gid` 则是从文件那里继承。

----- 设置/取消SUID SGID
$ chmod u+s filename
$ chmod u-s filename
$ chmod g+s filename
$ chmod g-s filename

chmod +s test
-->



## 使用链接参数

其中 GUN 的连接器提供了 `ld --wrap=symbol` 选项，解释如下 `man 1 ld`。

> --wrap=symbol
> Use a wrapper function for symbol. Any undefined reference to symbol will be resolved to
> __wrap_symbol. Any undefined reference to __real_symbol will be resolved to symbol.

也即是说，GUN 链接器会将未定义的符号解析成 `__wrap_symbol`，然后真正调用的函数封装成 `__real_symbol`，那么可以通过这一特性将系统调用包裹起来，不过只适用于该程序中。

其中示例如下。

{% highlight c %}
#include <stdio.h>

ssize_t __real_write(int fd, const void *buf, size_t count);

ssize_t __wrap_write (int fd, const void *buf, size_t count)
{
	printf("<<< write >>> %lu\n", count);
	return __real_write(fd, buf, count);
}
{% endhighlight %}

{% highlight c %}
#include <stdio.h>
#include <unistd.h>

int main(void)
{
        write(0, "Hello, Kernel!\n", 15);
        return 0;
}
{% endhighlight %}

{% highlight text %}
$ gcc -Wl,-wrap,write write.c main.c -o test
{% endhighlight %}



## 参考

详细可以参考 [Dynamic linker tricks: Using LD_PRELOAD to cheat, inject features and investigate programs](https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/) 或者 [本地文档](/reference/programs/linux-c-preload.html) 。

{% highlight text %}
{% endhighlight %}
