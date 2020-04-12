---
title: ELF 详解
layout: post
comments: true
language: chinese
category: [misc]
keywords: 
description:
---

ELF 的全称为 Executable and Linkable Format 用于存储 Linux 程序，可以从运行以及链接的两个视角查看，分别通过 Program Header Table 以及 Section Header Table 查看。

<!-- more -->

## 简介

ELF 文件的结构类似如下，包含了三个关键的索引表 `ELF Header`、`Program Header Table` 以及 `Section Header Table` 。

{% highlight text %}
文件头部(ELF Header)
程序头部表(Program Header Table)
节区1(Section1)
节区2(Section2)
节区3(Section3)
...
节区头部表(Section Header Table)
{% endhighlight %}

<!--
它们都对应着 C 语言里头的一些结构体（elf.h 中定义）。文件头部主要描述 ELF 文件的类型，大小，运行平台，以及和程序头部表和节区头部表相关的信息。节区头部表则用于可重定位文件，以便描述各个节区的信息，这些信息包括节区的名字、类型、大小等。程序头部表则用于描述可执行文件或者动态链接库，以便系统加载和执行它们。而节区主要存放各种特定类型的信息，比如程序的正文区（代码）、数据区（初始化和未初始化的数据）、调试信息、以及用于动态链接的一些节区，比如解释器（.interp）节区将指定程序动态装载 / 链接器 ld-linux.so 的位置，而过程链接表（plt）、全局偏移表（got）、重定位表则用于辅助动态链接过程。



* ELF header
* Program header table：告诉系统如何创建进程映像。用来构造进程映像的目标文件必须具有程序头部表，可重定位文件不需要这个表。
* Section Header Table 描述文件Section的信息，包括了名称、大小等信息；
-->

在头文件 `/usr/include/elf.h` 中定义了相关的结构体。

### 保存格式

其中 `Section Header Table` 可以通过 `readelf -S /bin/bash` 查看，`Program Header Table` 通过 `readelf -l /bin/bash` 查看，两者会通过 `ELF Header` 结合起来。

{% highlight text %}
$ readelf -h main
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x4004a0
  Start of program headers:          64 (bytes into file)
  Start of section headers:          9200 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         30
  Section header string table index: 29
{% endhighlight %}

可以看到其长度为 64 字节。

{% highlight text %}
$ hexdump -x main -n 64
0000000    457f    464c    0102    0001    0000    0000    0000    0000
0000010    0002    003e    0001    0000    04a0    0040    0000    0000
0000020    0040    0000    0000    0000    23f0    0000    0000    0000
0000030    0000    0000    0040    0038    0009    0040    001e    001d
0000040
{% endhighlight %}


<!--
libelf的使用
https://www.zybuluo.com/devilogic/note/139554

The ELF format - how programs look from the inside
https://greek0.net/elf.html


http://michalmalik.github.io/elf-dynamic-segment-struggles
-->

## 参考

* [System V Application Binary Interface](http://sco.com/developers/gabi/latest/contents.html)

{% highlight text %}
{% endhighlight %}
