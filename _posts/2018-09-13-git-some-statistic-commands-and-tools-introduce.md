---
title: Git 统计工具
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: git,cloc
description: git 应该是目前使用最为广泛的代码管理仓库了，提供了非常简单的命令行工具，通过这些命令行可以进行一些常见的代码统计。 这里简单介绍一些常用的命令。
---

git 应该是目前使用最为广泛的代码管理仓库了，提供了非常简单的命令行工具，通过这些命令行可以进行一些常见的代码统计。

这里简单介绍一些常用的命令。

<!-- more -->

## 基本命令

#### 查看git上的个人代码量

注意，这里统计的是从代码库创建以来所有的代码提交情况。

{% highlight text %}
$ git log --author="username" --pretty=tformat: --numstat |  \
    awk '{ add += $1; subs += $2; loc += $1 - $2 } END {   \
    printf "added lines: %s, removed lines: %s, total lines: %s\n", add, subs, loc }' -
{% endhighlight %}

#### 统计所有人提交代码量

{% highlight text %}
$ git log --format='%aN' | sort -u | while read name; do     \
    echo -en "$name\t";                                    \
	git log --author="$name" --pretty=tformat: --numstat | \
	awk '{ add += $1; subs += $2; loc += $1 - $2 } END {   \
	printf "added lines: %s, removed lines: %s, total lines: %s\n", add, subs, loc }' -; done
{% endhighlight %}

#### 仓库提交者排名前五

{% highlight text %}
$ git log --pretty='%aN' | sort | uniq -c | sort -k1 -n -r | head -n 5
{% endhighlight %}

#### 总的提交人数

{% highlight text %}
$ git log --pretty='%aN' | sort -u | wc -l
{% endhighlight %}

#### 总提交次数统计

{% highlight text %}
$ git log --oneline | wc -l
{% endhighlight %}

#### 统计总的代码行数

{% highlight text %}
$ find . -name "*.m" -or -name "*.h" -or -name "*.c" | xargs grep -v "^$" | wc -l
{% endhighlight %}

## 工具

有一个可视化的 git 统计工具，使用 Python 编写，可以参考 [github.com gitstats](https://github.com/hoxu/gitstats) 。

### cloc

对于当前代码库，有一个开源的 Perl 工具 `cloc` 统计不同语言的代码使用情况。

在 CentOS 中可以直接通过 `yum install cloc` 安装，然后在代码库中直接执行 `cloc .` 即可。


{% highlight text %}
{% endhighlight %}
