---
title: Linux Package 管理
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,package
description: 可以通过 `pkg-config` 命令来检索系统中安装库文件的信息，通常用于库的编译和连接。
---

可以通过 `pkg-config` 命令来检索系统中安装库文件的信息，通常用于库的编译和连接。

<!-- more -->

如果库的头文件不在 `/usr/include` 目录中，那么在编译的时候需要用 `-I` 参数指定其路径；同样，链接时可以通过 `-L` 参数指定库，这样就导致了编译命令界面的不统一。

为了保证统一，通过库提供的一个 `.pc` 文件获得库的各种必要信息的，包括版本信息、编译和连接需要的参数等，在需要的时候可以通过提供的参数，如 `--cflags`、`--libs`，将所需信息提取出来供编译和连接使用。

主要包括了：A) 检查库的版本号；B) 获得编译预处理参数，如宏定义，头文件的路径；C) 获得编译参数，如库及其依赖的其他库的位置，文件名及其他一些连接参数；D) 自动加入所依赖的其他库的设置。

在 CentOS 中默认会保存在 `/usr/lib64/pkgconfig`、`/usr/share/pkgconfig` 目录下。

{% highlight text %}
----- 查看所有.pc
$ pkg-config --list-all

----- 编译
$ gcc -c `pkg-config --cflags glib-2.0` sample.c

----- 链接
$ gcc sample.o -o sample `pkg-config --libs glib-2.0`

----- 编译链接合并
$ gcc sample.c -o sample `pkg-config --cflags --libs glib-2.0`
{% endhighlight %}

另外，可以通过环境变量 `PKG_CONFIG_PATH` 指定搜索路径。

{% highlight text %}
{% endhighlight %}
