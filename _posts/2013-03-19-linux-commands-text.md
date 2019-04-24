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

## diff & patch

该命令通常用于文本文件的区别查看。

{% highlight text %}
----- diff常用选项
    -r :  递归；
    -N :  确保能正确处理已创建或删除的文件；
    -u :  以统一格式创建，默认输出上下文前后 3 行，-u5 显示 5 行；
    -E :  忽略 tab 的改变；
    -b :  忽略 white space；
    -w :  忽略所有空白，当使用不同的对齐方式时非常方便，注意空白行仍然认为有差别；
    -B :  忽略空白行。

----- patch常用选项
    -R :  新旧版本互换；
    -E :  直接删除空文件。
{% endhighlight %}

假设文件 A 和文件 B ，其中 A 为原文件， B 为修改后的文件，经过 diff 之后生成了补丁文件 C ，那么着个过程相当于 A - B = C ，那么 patch 的过程就是 B + C = A 或 A - C = B。

也就是说，如果简单使用 diff A B &gt; C，那么只能用 patch A C，使用 patch B C 将出现错误，如果想恢复则应该使用 patch -RE A C。

#### 格式

其中简单示例如下：

{% highlight text %}
$ diff -uN A B
--- A   2013-02-17 11:20:08.926661164 +0800         # 原文件
+++ B   2013-02-17 11:20:38.666854034 +0800         # 修改后的文件
@@ -1 +1,2 @@                  # 原文件的第一行，以及改后的1~2行
 Hello World                   # 两者之差的内容即为修改的内容
 +foo bar                      # 其中减号('-')表示删除，加号('+')表示添加
{% endhighlight %}

#### 常用操作

{% highlight text %}
----- 单个文件
# diff -uN from-file to-file > to-file.patch     # 产生补丁
# patch -p0 < to-file.patch                      # 打补丁，针对目录下所有文件
# patch -p0 from-file to-file.patch              # 同上，但是指定文件
# patch -RE -p0 < to-file.patch                  # 恢复原文件

----- 多个文件
# diff -uNr from-docu to-docu > to-docu.patch
# patch -p1 < to-docu.patch
# patch -R -p1 < to-docu.patch
{% endhighlight %}

patch 使用时不用指定文件，在补丁文件中已经记载了原文件的路径和名称。

另外需要通过 -pn 来处理补丁中的路径问题，如 dir/dirA/A 、 dir/dirB/B ，在 dir 目录下执行 $ diff -rc dirA dirB > C，此时 C 中记录的路径为 dirA/A ；此时如果在 dirA/ 目录下，那么应该使用 patch -p1 ，即忽略 dirA/ 。

补丁失败的文件会以 .rej 结尾，下面命令可以找出所有 rej 文件， find . -name '*.rej' 。

{% highlight text %}
dir# diff -uNr from-dir to-dir > dir.patch
dir# patch -p0 < dir.patch                       # from-dir 将会被删除
dir/from-dir# patch -p1 < ../dir.patch           # 正确的处理方法
{% endhighlight %}



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
