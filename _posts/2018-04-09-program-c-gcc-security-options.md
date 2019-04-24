---
title: GCC 安全编译选项
layout: post
comments: true
language: chinese
category: [linux]
keywords: gcc,c,language
description: 操作系统提供了许多安全机制来尝试降低或阻止缓冲区溢出攻击带来的安全风险，例如 ASLR、NX 等等，这里简单介绍一些常见的使用项。
---

操作系统提供了许多安全机制来尝试降低或阻止缓冲区溢出攻击带来的安全风险，例如 ASLR、NX 等等，这里简单介绍一些常见的使用项。

<!-- more -->

## 简介

操作系统提供了许多安全机制来尝试降低或阻止缓冲区溢出攻击带来的安全风险，例如 NX ASLR PIE CANARY FORTIFY RELRO 等手段，存在 NX 的话就不能直接执行栈上的数据，存在 ASLR 的话各个系统调用的地址就是随机化的等等。

GCC 在生成代码时，实际上已经提供了一些针对安全相关的编译选项，在介绍详细内容前，首先介绍下基本概念。

### Linux 程序段介绍

在 Linux 中大致会有如下的 5 种数据区：

* BSS Segment 用来存放程序中未初始化的全局/静态变量的一块内存区域，属于静态内存分配，其全称为 Block Started by Symbol 。
* DATA Segment 保存已经初始化的全局变量，属于静态内存分配。
* TEXT Segment 用来存放真正执行的代码，大小在编译后已经确定，一般是只读。
* HEAP Segment 保存堆的内容，一般是通过 malloc() 动态分配的内存。
* STACK Segment 栈空间，一般是存放程序临时创建的局部变量。

可以通过如下程序查看进程各个段的地址。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

static char bss[1024];
static int data = 1234;
static const char *text = "foobar";

static void foobar(void)
{
        printf("Hi foobar\n");
}

int main(void)
{
        int stack = 0;
        char *heap = (char *)malloc(1000);

        printf("Address of various segments:\n");
        printf("     Text Segment: %p\n", foobar);
        printf("       RO Segment: %p\n", text);
        printf("     Data Segment: %p\n", &data);
        printf("              BSS: %p\n", bss);
        printf("    Stack Segment: %p\n", &stack);
        printf("     Heap Segment: %p\n", heap);

        return 0;
}
{% endhighlight %}

注意，目前的程序一般都会有多个这样的段，所以基本上无法确认每个段的边界。

## ASLR 地址随机

Address Space Layout Randomization, ASLR 地址空间布局随机化，该技术在 2005 年的 Kernel 2.6.12 版本中引入，会将进程的某些内存空间地址进行随机化来增大入侵者预测目的地址的难度，从而降低进程被成功入侵的风险。

当前 Linux、Windows 等主流操作系统都已经采用该项技术。

### Linux ASLR

在 Linux 中会通过 `randomize_va_space` 文件配置相关的 ASLR 级别，总共包含了三个：

* 0 没有随机化，也就是关闭 ASLR 。
* 1 保留的随机化，其中共享库、栈、mmap 以及 VDSO 将被随机化。
* 2 完全的随机化，在 1 的基础上，通过 brk() 分配的内存空间也将被随机化。

可以通过如下方式查看或者修改。

{% highlight text %}
# sysctl -n kernel.randomize_va_space
# cat /proc/sys/kernel/randomize_va_space

# sysctl -w kernel.randomize_va_space=0
# echo 0 > /proc/sys/kernel/randomize_va_space
{% endhighlight %}

看到这里列出的几项内存空间，很自然有两个地方十分值得注意，

那么对于 代码段(text) 和 数据段(data bss) 是否被随机化了？堆是否被随机化了？

### PIE

多次运行上述程序会发现，随机化的只有堆和栈空间，而代码段和数据段是没有被随机化的。

> Position Independent Executable, PIE 是 gcc 提供的功能，需要连接到 `scrt1.o` ，然后可以像共享库一样在内存任何位置装载。而标准可执行程序需要固定的地址，并且只有被装载到这个地址时，程序才能正确执行。

实际山，在 Linux 中是通过 PIE 机制来负责代码段和数据段的随机化工作，而不是 ASLR ，当然也同时需要 ASLR 同时开启才可以。

要开启 PIE 需要在使用 gcc 进行编译链接时添加 `-fpie` `-pie` 选项，这样使得在利用缓冲溢出和移动操作系统中存在的其它内存崩溃缺陷时采用面向返回的编程 (Return Oriented Programming) 方法变得难得多。

#### PIE VS. PIC

GCC 中的参数 PIE 和 PIC 都可以用来生成跟位置没有关系的 Symbol ，其中 A) PIE 用在可执行文件；B) PIC 用在共享库文件 。使用示例如下：

{% highlight text %}
$ gcc -o test test.c                // 默认不开启PIE
$ gcc -fpie -pie -o test test.c     // 开启PIE 强度为1
$ gcc -fPIE -pie -o test test.c     // 开启PIE 最高强度2
$ gcc -fpic -o test test.c          // 开启PIC 强度为1 不会开启PIE
$ gcc -fPIC -o test test.c          // 开启PIC 最高强度2 不会开启PIE
{% endhighlight %}

### 堆随机化

这里会有些复杂，因为 Linux 中堆空间可以通过 `mmap()` 以及 `brk()` 这两个系统调用完成的，而在不同的等级上面可能会只有部分接口被随机化。

一般谁使用 glibc 中的 `malloc()` 类接口分配内存，通过 `man 3 malloc` 可以发现其中有相关的介绍，也就是当超过了 `MMAP_THRESHOLD` 大小后会使用 `mmap()`，否则使用 `brk()` 申请。

如果当前 ASLR 等级为 1，那么当申请空间大于 128K 时，系统通过 `mmap()` 分配空间，得到的地址是随机的；而当申请空间小于 128K 时，系统是通过 `brk()` 进行分配的，得到的地址是静止的。

## 栈保护

针对的是一种很常见的栈溢出攻击，

### 不执行数据

Linux 和 Windows 平台都支持对非可执行代码的保护，在 Linux 平台中被称为 NX (No-eXecute protect) ，在 Windows 中叫做 DEP (Data Execution Prevention) 。

其基本原理是将数据所在内存页标识为不可执行，当程序溢出成功转入 ShellCode 时，程序会尝试在数据页面上执行指令，此时 CPU 就会抛出异常，而不是去执行恶意指令。

![nx dep]({{ site.url }}/images/security/gcc-nx-dep.jpg "nx dep"){: .pull-center }

GCC 编译器默认开启了 NX 选项，可以通过添加 `-z execstack` 编译参数关闭 NX 选项。

{% highlight text %}
$ gcc -o test test.c                    // 默认开启 NX 保护
$ gcc -z execstack -o test test.c       // 禁用 NX 保护
$ gcc -z noexecstack -o test test.c     // 开启 NX 保护
{% endhighlight %}

### 栈溢出保护

当启用栈保护后，函数开始执行的时候会先往栈里插入 Cookie 信息，函数返回时会验证 Cookie 信息是否合法，非法则停止运行。

攻击者在覆盖返回地址的时候往往也会将 Cookie 信息给覆盖掉，导致栈保护检查失败进而阻止 ShellCode 的执行，在 Linux 中将 Cookie 信息称为 Canary 。

GCC 在 4.2 版本中添加了 `-fstack-protector` 和 `-fstack-protector-all` 编译参数以支持栈保护功能，4.9 新增了 `-fstack-protector-strong` 编译参数让保护的范围更广，在编译时可以控制是否开启栈保护以及程度，例如：

{% highlight text %}
$ gcc -o test test.c                        // 默认不开启 Canary 保护
$ gcc -fno-stack-protector -o test test.c   // 禁用栈保护
$ gcc -fstack-protector -o test test.c      // 启用堆栈保护，只为局部变量中含有 char 数组的函数插入保护代码
$ gcc -fstack-protector-all -o test test.c  // 启用堆栈保护，为所有函数插入保护代码
{% endhighlight %}

### FORTIFY

用于检查是否存在缓冲区溢出的错误，针对的是字符串、内存操作函数，例如 memcpy memset strcpy strcats snprintf 等等。

可以通过 `_FORTIFY_SOURCE` 宏定义检查的级别：

* `_FORTIFY_SOURCE=1` 仅在编译时检查。
* `_FORTIFY_SOURCE=2` 在程序运行时也会检查，如果判断到缓冲区溢出则会直接终止程序。

实际上 GCC 会到生成了一些附加代码，通过对数组大小的大小进行判断，从而达到防止缓冲区溢出的作用，使用示例如下：

{% highlight text %}
$ gcc -o test test.c                          // 默认不会开启检查
$ gcc -D_FORTIFY_SOURCE=1 -o test test.c      // 较弱的检查
$ gcc -D_FORTIFY_SOURCE=2 -o test test.c      // 较强的检查
{% endhighlight %}


## RELRO

在 Linux 系统安全领域，数据可写的存储区就会是攻击的目标，尤其是存储函数指针的区域，所以在安全防护的角度来说尽量减少可写的存储区域对安全会有极大的好处。

GCC 提供了一种 Read Only Relocation 的方法，其原理为是由 linker 指定 binary 的一块经过 dynamic linker 处理过 relocation 之后的区域为只读.

设置符号重定向表格为只读或在程序启动时就解析并绑定所有动态符号，从而减少对 Global Offset Table, GOT 攻击。

{% highlight text %}
gcc -o test test.c                     // 默认是 Partial RELRO
gcc -z norelro -o test test.c          // 关闭
gcc -z lazy -o test test.c             // 部分开启 即Partial RELRO
gcc -z now -o test test.c              // 全部开启
{% endhighlight %}

如果 RELRO 为 "Partial RELRO"，说明对 GOT 表具有写权限。


## 其它

### checksec

checksec 是一个 Bash 脚本，可以用来检查可执行文件属性，例如 `PIE` RELRO PaX Canaries, ASLR, Fortify Source等等属性。

详细可以查看官网 [TrapKit CheckSec](http://www.trapkit.de/tools/checksec.html) 或者 [Github CheckSec](https://github.com/slimm609/checksec.sh/) ，也可以直接使用 [本地保存](/reference/security/checksec) 。

## 总结

各种安全选择的编译参数如下：

* NX `-z execstack` 关闭 `-z noexecstack` 开启
* Canary `-fno-stack-protector` 关闭 `-fstack-protector` 开启 `-fstack-protector-all` 全开启
* PIE `-no-pie` 关闭 `-pie` 开启
* RELRO `-z norelro` 关闭 `-z lazy` 部分开启 `-z now` 完全开启

## 参考

[Memory Protection and ASLR on Linux](https://eklitzke.org/memory-protection-and-aslr) 关于 Linux 中 ASLR 机制的介绍。


<!--
ASLR 级别为2
堆栈保护 -fstack-protector-all
堆栈不可执行 -Wl,-z,noexecstack
位置无关随机化 PIC/PIE
禁用RPATH/RUNPATH
GOT表保护 -Wl,-z,relro
立即加载 -z now

Windows
数据执行保护 /DEP
缓冲区安全监察 /GS
安全异常处理 /SafeSEH
随机化 /DYNAMICBASE


AnC攻击的实现--bypass ASLR by MMU
https://paper.seebug.org/226/
-->


{% highlight text %}
{% endhighlight %}
