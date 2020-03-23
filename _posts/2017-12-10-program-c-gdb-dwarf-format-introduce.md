---
title: DWARF 格式简介
layout: post
comments: true
language: chinese
category: [program]
keywords:  program,c,deadlock
description: 也就是 Debugging With Attributed RecordFormats, DWARF ，到目前为止，应该包括了 `V1~5` 共五个版本，其中 dwarf2 对 dwarf1 的改变很大，而后续的版本基本上是对前者的扩展。这一格式独立于语言，不过使用比较多的是 C/C++ ，这里简单介绍在 GDB 中，如何对这些调试信息进行组织、实现，并且如何利用调试信息进行 C 语言级别的调试。
---

也就是 Debugging With Attributed RecordFormats, DWARF ，到目前为止，应该包括了 `V1~5` 共五个版本，其中 dwarf2 对 dwarf1 的改变很大，而后续的版本基本上是对前者的扩展。

这一格式独立于语言，不过使用比较多的是 C/C++ ，这里简单介绍在 GDB 中，如何对这些调试信息进行组织、实现，并且如何利用调试信息进行 C 语言级别的调试。

<!-- more -->

## 简介

一般来说，代码的调试信息要远大于二进制程序本身，包括了可执行程序、动态库等等，一般的 Linux 发行版本会使用类似 debuginfo 的包，作为独立的调试信息。

对应的 debuginfo 包可以从 [debuginfo.centos.org](http://debuginfo.centos.org/7/x86_64/) 中获取，如果在使用 gdb 时没有加载，除了路径、权限等问题外，可能是版本不匹配。

{% highlight text %}
----- 设置全局的DebugInfo保存目录
(gdb) set debug-file-directory <directory>

----- 查看当前的全局配置目录
(gdb) show debug-file-directory
{% endhighlight %}

对于 CentOS 来说，一般保存在 `/usr/lib/debug` 目录下，

<!--
symbol-file myprogram.debug

https://blog.csdn.net/JS072110/article/details/44153303

https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Debugging_with_gdb/separate-debug-files.html
http://abcdxyzk.github.io/blog/2014/02/21/debug-base-objdump-apply/

在使用 gdb 进行调试时，需要在机器码和源代码之间建立映射关系，这样的话

yum install yum-utils
debuginfo-install glibc

DebugInfo
https://blog.csdn.net/chinainvent/article/details/24129311
https://blog.csdn.net/testcs_dn/article/details/19565411

https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
https://stackoverflow.com/questions/866721/how-to-generate-gcc-debug-symbol-outside-the-build-target

详细介绍gdb是如何查找符号的
http://blog.yajun.info/?p=7394




gcc -o thread thread.c -lpthread -g

使用readelf和objdump解析目标文件
https://www.jianshu.com/p/863b279c941e
readelf -S -W thread
gcc -o thread thread.c -lpthread -O2 会删除掉 `.debug_XXX` 字段，包括了 `aranges` `info` `abbrev` `line` `str` 。
strip thread  去除了symtab、strtab

(gdb) bt     # 二进制文件中含有调试信息，会显示thread2对应的代码行信息
#0  __lll_lock_wait () at ../nptl/sysdeps/unix/sysv/linux/x86_64/lowlevellock.S:135
#1  0x00007fbb2222ddcb in _L_lock_883 () from /lib64/libpthread.so.0
#2  0x00007fbb2222dc98 in __GI___pthread_mutex_lock (mutex=0x7ffed182d540) at ../nptl/pthread_mutex_lock.c:78
#3  0x00000000004007e2 in thread2 (arg=0x7ffed182d540) at thread.c:32
#4  0x00007fbb2222bdd5 in start_thread (arg=0x7fbb21655700) at pthread_create.c:307
#5  0x00007fbb21f54ead in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:111

(gdb) bt     # 当只有debuginfo时，只有公共库会显示具体的地址
#0  __lll_lock_wait () at ../nptl/sysdeps/unix/sysv/linux/x86_64/lowlevellock.S:135
#1  0x00007fa224700dcb in _L_lock_883 () from /lib64/libpthread.so.0
#2  0x00007fa224700c98 in __GI___pthread_mutex_lock (mutex=0x7ffc94fdb610) at ../nptl/pthread_mutex_lock.c:78
#3  0x000000000040081a in thread2 ()
#4  0x00007fa2246fedd5 in start_thread (arg=0x7fa223b28700) at pthread_create.c:307
#5  0x00007fa224427ead in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:111

(gdb) bt     # 否则只能显示函数所在的库
#0  0x00007fa2247054ed in __lll_lock_wait () from /lib64/libpthread.so.0
#1  0x00007fa224700dcb in _L_lock_883 () from /lib64/libpthread.so.0
#2  0x00007fa224700c98 in pthread_mutex_lock () from /lib64/libpthread.so.0
#3  0x000000000040081a in thread2 ()
#4  0x00007fa2246fedd5 in start_thread () from /lib64/libpthread.so.0
#5  0x00007fa224427ead in clone () from /lib64/libc.so.6


https://deepzz.com/post/gdb-debug.html

addr2line 能够将地址转换为文件名和行号，可以通过可执行文件的地址或者可重定位目标的目标偏移，然后根据 debug 信息来计算出与该地址关联的文件名和行号。

int divide(int a, int b)
{
        return a / b;
}

int main(void)
{
        return divide(10, 0);
}

通过 `dmesg` 或者 `/var/log/messges` 看到如下的内容。

traps: divide[31417] trap divide error ip:4004fe sp:7ffcf0104230 error:0 in divide[400000+1000]

其中 IP 指向的地址就是出问题的地方，那么就可以通过该地址获取到代码的位置。

addr2line -e divide 4004fe

注意，上述代码编译的时候需要添加 `-g` 参数。

通过 `readelf -w divide` 可以看到如下的内容，保存在 `.debug_line` 中，那么对应的 `4004fe` 就对应了第 3 行。

 Line Number Statements:
  Extended opcode 2: set Address to 0x4004f0
  Special opcode 6: advance Address by 0 to 0x4004f0 and Line by 1 to 2
  Special opcode 146: advance Address by 10 to 0x4004fa and Line by 1 to 3
  Special opcode 104: advance Address by 7 to 0x400501 and Line by 1 to 4

对于动态库一般可以使用如下的方式。

addr2line -j .text -e libtst.so 0x26887

https://stackoverflow.com/questions/7556045/how-to-map-function-address-to-function-in-so-files/7557756
https://blog.csdn.net/force_eagle/article/details/51980558
https://blog.csdn.net/tenfyguo/article/details/6623967


https://blog.csdn.net/chenyijun/article/details/85284867
-->


## 参考

* DWARF 格式的详细信息可以参考 [www.dwarfstd.org](http://www.dwarfstd.org/) 官网中的介绍，包括了一个基础的教程。
* [Articles in tag "Debuggers"](https://eli.thegreenplace.net/tag/debuggers) 一个简单的调试器，包括了 ptrace 以及 DWARF 的使用方式。

{% highlight python %}
{% endhighlight %}
