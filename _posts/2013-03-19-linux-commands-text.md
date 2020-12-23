---
title: Linux 常用命令 -- 文本处理
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,命令
description: 主要介绍下在 Linux 中，与文本处理相关的命令。
---

主要介绍下在 Linux 中，与文本处理相关的命令。

<!-- more -->

## cmp

二进制文件的比较。

cmp 命令会逐字节比较两个文件内容，如果两个文件内容完全，则 cmp 命令不显示任何内容。若两个文件内容有差异，会显示第一个不同之处的字节数和行数编号。

如果文件是 '-' 或没给出，则从标准输入读入内容。

{% highlight text %}
cmp [options] file1 file2
常用选项：
  -l, --verbose
    显示每一个不同点的字节号（10进制）和不同点的字节内容（8进制）；会显示所有不同字节。
  -b, --print-bytes
    以子符的形式显示不同的字节。
  -i NUM, --ignore-initial=NUM
    两个文件均越过开始的NUM个字节开始比较。
  -i NUM1:NUM2, --ignore-initial=NUM1:NUM2
    第一个文件越过开始的NUM1个字节，第二个文件越过开始的NUM2个字节，开始比较。
  -n NUM, --bytes=NUM
    设定比较的上限，最多比较 NUM 个字节。
{% endhighlight %}

文件相同只返回0；文件不同返回1；发生错误返回2。

{% highlight text %}
$ cmp file1 file2
file1 file2 differ: char 23, line 6
cmp: EOF on file1
{% endhighlight %}

第一行的结果表示 file1 与 file2 内容在第 6 行的第 23 个字符开始有差异。第二行的结果表示 file2 前半部分与 file1 相同，但在 file2 中还有其他数据。



<!--
awk -F : '$1=="root" {print $0}' /etc/passwd

/usr/sbin/useradd -M -N -g test -o -r -s /bin/false -c "Uagent Server" -u 66 test
/usr/sbin/groupadd -g 66 -o -r test
-->

## 常用技巧

如果其中的部分参数需要动态获取，而 ```''``` 则会原样输出字符内容，那么可以通过类似如下的方式使用。

{% highlight text %}
$ echo "'$(hostname)'" | xargs sed filename -e
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
