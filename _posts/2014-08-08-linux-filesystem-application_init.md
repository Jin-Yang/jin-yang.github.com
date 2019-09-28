---
Date: August 8, 2014
title: Linux 文件系统
layout: post
comments: true
category: [linux]
language: chinese
---

Linux 或者 Unix 的设计理念是：一切都是文件！

可以看到包括常用的文件，其它的包括设备，虚拟文件系统，甚至是网络 socket 都是通过文件表示。而且与 Windows 不同，在 Linux 中没有分区的概念，只有目录以及文件。
<!-- more -->

# 简介
在 Linux 中，所有的文件以及目录组成了一个目录树结构，在根目录下包括 bin、boot、etc、home、lib、mnt、proc 等目录，而实际上存在一个标准 [Filesystem Hierarchy Standard (FHS)][fhs] 规定了目录的结构，当然不同的发行版本只是略有不同。

当然，在 Linux 中我们也可以对磁盘进行分区，然后挂载到目录上。当拿到一个磁盘后，需要做的是 A) 对磁盘分区；B) 针对分区安装文件系统，如 ext4、xfs、ntfs 等；C) 将分区挂载到目录上，如 /mnt/foobar，然后就可以对磁盘上的文件进行操作了。


# 文件属性


# inode 介绍
inode 用来唯一标示 Linux 中的文件，包括设备文件、符号链接、目录等，可以通过 stat 命令查看对应文件的 inode 号。在此只针对磁盘文件系统，如 ext4 ，进行介绍。

对于一个文件来说，有唯一的 inode 与之对应，而对于一个 inode ，由于 Linux 支持链接，从而可以有多个文件名与之对应，也就是说，在磁盘上的同一个文件可以通过不同的路径去访问该文件。

另外，Linux 与其他类 UNIX 系统一样并不区分文件与目录，目录是记录了其它文件名的文件。这也意味着，当使用命令 mkdir 创建目录时，若期望创建的目录名称与现有的文件名 (或目录名) 重复，则会创建失败。






[fhs]:          http://www.pathname.com/fhs/                 "Filesystem Hierarchy Standard"
 
<!-- pictures -->
[garbage-logo]:            /images/python/garbages/garbage-logo.jpg               "ASCII 字符集"
