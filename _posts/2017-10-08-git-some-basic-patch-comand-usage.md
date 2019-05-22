---
title: Git Patch 相关
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: git,patch,diff
description: 在 Linux 中可以通过 diff patch 命令使用 Patch ，git 实际上提供了很简单的命令直接生成 Patch 文件，然后再结合 patch 命令使用即可。 这里简单介绍其使用方法。
---

在 Linux 中可以通过 diff patch 命令使用 Patch ，git 实际上提供了很简单的命令直接生成 Patch 文件，然后再结合 patch 命令使用即可。

这里简单介绍其使用方法。

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

假设文件 A 和文件 B ，其中 A 为原文件， B 为修改后的文件，经过 diff 之后生成了补丁文件 C ，那么着个过程相当于 `A - B = C` ，那么 patch 的过程就是 `B + C = A` 或 `A - C = B` 。

也就是说，如果简单使用 `diff A B > C`，那么只能用 `patch A C`，使用 `patch B C` 将出现错误，如果想恢复则应该使用 `patch -RE A C`。

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

另外需要通过 `-pn` 来处理补丁中的路径问题，如 `dir/dirA/A` 、 `dir/dirB/B` ，在 dir 目录下执行 `diff -rc dirA dirB > C`，此时 C 中记录的路径为 `dirA/A` ；此时如果在 `dirA/` 目录下，那么应该使用 `patch -p1` ，即忽略 `dirA/` 。

补丁失败的文件会以 `.rej` 结尾，下面命令可以找出所有 `rej` 文件， `find . -name '*.rej'` 。

{% highlight text %}
dir# diff -uNr from-dir to-dir > dir.patch
dir# patch -p0 < dir.patch                       # from-dir 将会被删除
dir/from-dir# patch -p1 < ../dir.patch           # 正确的处理方法
{% endhighlight %}

## git

我们可以通过 `git log` 获取提交时的 Commit ID ，然后再通过如下命令获取提交的差异。

{% highlight text %}
----- 当前HEAD到2e4bb3提交的差异
$ git diff 2e4bb3 > /tmp/foobar.patch

----- 然后开始打补丁
$ cd /your/project/path
$ patch -R -p1 < /tmp/foobar.patch
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
