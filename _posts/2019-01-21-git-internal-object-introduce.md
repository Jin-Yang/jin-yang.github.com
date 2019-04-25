---
title: git 对象简介
layout: post
comments: true
language: chinese
category: [misc,linux]
keywords: git,blob
description: git 作为当前最流行的版本管理工具，简单、易用，而且功能强大，不过其依托的是一组极为简洁的数据结构。这里简单介绍其核心概念。
---

git 作为当前最流行的版本管理工具，简单、易用，而且功能强大，不过其依托的是一组极为简洁的数据结构。

这里简单介绍其核心概念。

<!-- more -->

## 简介

git 维护着一个微型的文件系统，其中的文件也被称作数据对象，所有的数据对象均存储于项目下面的 `.git/objects` 目录中。

{% highlight text %}
$ git init foobar && cd foobar
$ echo 'Hello World !!!' > README.md
$ git add README.md
{% endhighlight %}

此时已经将文件添加到了 `.git/objects` 目录下了，可以通过如下命令查找。

{% highlight text %}
$ find .git/objects -type f
.git/objects/2a/c7fb025641058bed0a8ebaa7a862d90bbb9522
{% endhighlight %}

该对象被称为 Blob 对象，可以通过下面的命令把文件内容打印出来，或者查看文件的类型。

{% highlight text %}
$ git cat-file -p 2ac7fb
Hello World !!!
$ git cat-file -t 2ac7fb
blob
{% endhighlight %}

版本库中的每个文件，包括了图片、文本、二进制文件等，都会被映射为一个 Blob 对象。

除了 Blob 对象，还存储着另外三种数据对象：Tree、Commit、Tag 。

### 常用操作

{% highlight text %}
----- 查看对象的值以及内容，可以指定具体的类型
$ git cat-file -t <SHA1>
$ git cat-file -p <SHA1>
$ git cat-file blob <SHA1>
$ git cat-file commit <SHA1>
{% endhighlight %}

可以通过如下命令查看所有对象的类型。

{% highlight text %}
$ find .git/objects -type f -a ! -name "*pack*" | \
	awk -F "/" '{printf("%s%s ", $3, $4); system("git cat-file -t "$3$4)}'
{% endhighlight %}

查看 `blog` 对象。

{% highlight text %}
----- 将一个文件生成对应的SHA1值，也就是对象的ID
$ git hash-object filename
----- 查看对象的内容，如果文件不存在则会报错
$ git show <SHA1>
{% endhighlight %}

查看 `commit` 对象。

{% highlight text %}
----- 列出所有的commit信息
$ git log --pretty=oneline
----- 查看commit的内容
$ git show <SHA1>
----- 该commit对象所对应的tree
$ git cat-file -p <SHA1>
{% endhighlight %}




## Blob 对象

其全称为 Binary Large Object ，一个 Blob 对象就是一段二进制数据。

为了把文件映射为 Blob 对象，Git 会执行如下的步骤：

1. 读取文件内容，并添加一段特殊标记到头部 `"blob %u\0%s", len, content` ，得到新的内容；
2. 对上述的输出内容执行 SHA-1 哈希加密，此时，会得到一个长度为 40 字符的 hash 值，例如上述的 `2ac7fb025641058bed0a8ebaa7a862d90bbb9522`；
3. 取该 hash 值的前两位作为子目录，剩下的 38 位作为文件名；
4. 然后对第一步输出的内容进行 zip 压缩，得到新的二进制内容，存入文件中。

## 查找过程

如果要查看某个分支的历史提交记录，那么使用过程如下。

首先会找到 HEAD 中对应的索引文件，该文件记录了最近一次 commit 对象的 hash 值。

{% highlight text %}
$ cat .git/HEAD
ref: refs/heads/feature/resource
$ cat .git/refs/heads/feature/resource
8e9c66dca71bd8e6452cf7534d1271e1fc60d54e
$ git log --pretty=oneline
8e9c66dca71bd8e6452cf7534d1271e1fc60d54e Your Commit
... ...
$ git cat-file -p 8e9c66dca71bd8e6452cf7534d1271e1fc60d54e
tree 71a4f5f84f9fd37004204a243c4f1f37aefbe15f
parent 9dfe4f99303977ff25db9880cc457bdad1ad91e7
author foobar <foobar@foobar.com> 1555426113 +0800
committer foobar <foobar@foobar.com> 1555426113 +0800

Your Commit
$ git ls-tree 71a4f5f8          # 当前版本包含的数据
100644 blob 9e2b32adba25a2a1c6de532dc7ab004717c2171e    CMakeLists.txt
100644 blob 311e372615b8926986e6a5e4a34f1c58a04ba003    README.md
040000 tree 9d0d57f47b4aaccb220fe8139f92c4bda5c8f54d    include
$ git cat-file blob 311e3726    # 查看某个文件对应的内容
Hello World
$ git cat-file -p 9d0d57f4      # 查看某个tree对象对应的内容，或者git ls-tree 9d0d57f4
120000 blob d4715f1f09960daa402fc1b9fdbc0afb2b0927a1    common.h
100644 blob 5c99a3054cc022ad981260b18d1dddebc7c749b6    sockets.h
{% endhighlight %}

一个 tree 对象和 parent 对象是关键，tree 表示了当前 commit 对象下的所有内容，而 parent 对象指向了前一个 commit 对象。

{% highlight text %}
$ git rev-parse HEAD
8e9c66dca71bd8e6452cf7534d1271e1fc60d54e
$ git rev-parse HEAD~
9dfe4f99303977ff25db9880cc457bdad1ad91e7
{% endhighlight %}

可以看到当前 tree 对象的 parent 对象与前一次的 commit 对应的 hash 值是相同的，这样就可以按照这一关联关系依次查找。

> git 中绝大部分对象都是通过 SHA1 来标识，通过 `rev-parse` 可以将一些习惯用的标识转换为 SHA1 内部 ID ，例如 `HEAD^` `origin/master` 。
> 除此之外，还有一些其它的用法，例如 `git rev-parse --symbolic --tags` 查看所有的 tag 信息，`git rev-parse --symbolic --branches` 本地所有的分支信息。



## 参考

<!--
https://git-scm.com/book/zh/v1/Git-%E5%86%85%E9%83%A8%E5%8E%9F%E7%90%86-Git-%E5%AF%B9%E8%B1%A1

https://www.jianshu.com/p/fa31ef8814d2
https://jingsam.github.io/2018/06/03/git-objects.html
https://www.jianshu.com/p/5c865a50b375
http://gitbook.liuhui998.com/1_2.html
-->


{% highlight text %}
{% endhighlight %}
