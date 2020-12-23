---
title: Linux umask 使用
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: umask,open
description: 在使用 umask 设置问价的掩码时遇到了一个坑，本来以为设置会很简单，但是开始设置时一直不是预期的结果。简单整理下踩坑的过程。
---

在使用 umask 设置问价的掩码时遇到了一个坑，本来以为设置会很简单，但是开始设置时一直不是预期的结果。

简单整理下踩坑的过程。

<!-- more -->

## 简介

在 Linux 中可以通过 umask 命令设置创建文件的默认权限，当用户创建文件夹以及文件时，就会通过该命令的入参计算出来，文件或者文件夹的真正权限。

通过 `umask` 的值可以设置用户创建文件时的默认文件权限，总共有 4 组数字，与文件的属性相对应，第一个数字比较特殊一般可以忽略。

对于目录最大权限是 `777` ，普通文件是 `666` ，创建时的默认权限就是最大权限减去 `umask` 指定的权限，也就是需要去掉的权限。

例如，默认设置的 `022` ，当创建目录和文件的时候计算规则如下。

{% highlight text %}
mkdir 777 - 022 = 755  -rwxr-xr-x
touch 666 - 022 = 644  -rw-r--r--
{% endhighlight %}

也就是说，当用户的 umask 是 `022`，创建目录时的权限就是所有者不去掉任何权限，属组和其它用户去掉 w 权限，所以目录默认是 `755`，而文件默认是 `644` 。

如下是 Python 中的实现方式。

{% highlight text %}
import os
os.umask(022)
os.mkdir("foobar")
open("foobar.txt", "w").close()
{% endhighlight %}

## C 语言

在 C 中，同样有一个 `umask()` 函数来设置对应的值，开始设置为 `umask(022)` ，那么在通过 `open(..., O_CREAT)` 创建文件时，得到的结果竟然不是预期的 `-rw-rw-rw-` ，而是一个很奇葩的结果。

{% highlight text %}
# ./foobar
... ...
---S-----T  1 root    root   534 May 27 17:23 FooBar.log
{% endhighlight %}

当通过 `strace` 命令查看系统 API 的调用参数时，可以看到如下的结果。

{% highlight text %}
... ...
umask(022)                              = 022
... ...
open("FooBar.log", O_WRONLY|O_CREAT|O_APPEND, 03776721252605000) = 3
{% endhighlight %}

WTF !!!

`umask()` 设置时正确的，但是代码里 `open()` 使用的是两个参数，但是第三个参数是怎么来的，看着显然是个乱码，这也就是为什么出现了上述问题的原因。

## 原因

简单来说，这里设置的 `umask()` 没有问题，但是这需要配合 `open()` 函数一块使用。

### 计算规则

一般默认的 `umask()` 应该是 `S_IWGRP | S_IWOTH` 也就是八进制的 `022` ，那么此时通过 `open()` 打开文件时，通过第三个参数指定为如下：

{% highlight text %}
S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
{% endhighlight %}

也就是 `-rw-rw-rw-` ，那么实际上生成的文件权限为 `0666 & ~022 = 0644` 。

这也就意味着，设置文件的权限需要 `umask()` 和 `open()` 函数配合使用，通过 `umask()` 限制权限，利用 `open()` 指定默认的权限。

### Open 参数

另外需要注意，在通过 `man 2 open` 查看帮助文档时，会发现 `open()` 函数实际上有两种声明，分别有两个或者三个入参。

实际上，默认的第三个参数是不使用的，除非 `open()` 函数中存在 `O_CREAT` 选项，那么对应的 `mode` 应该包含有效的入参，否则就可能会出现上述的问题。

## 总结

1. `umask()` 需要配合 `open()` 函数一块使用；
2. `open()` 函数中如果允许创建文件，那么就需要配置有效的 `mode` 参数。


{% highlight text %}
{% endhighlight %}
