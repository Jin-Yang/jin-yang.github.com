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
-->


## 参考

DWARF 格式的详细信息可以参考 [www.dwarfstd.org](http://www.dwarfstd.org/) 官网中的介绍吧，包括了一个基础的教程 [DWARF Tutorial](http://www.dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf) 。

{% highlight python %}
{% endhighlight %}
