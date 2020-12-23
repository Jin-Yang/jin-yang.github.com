---
title: Git LFS 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,git,lfs
description: GIT 代码管理是基于文本文件行的，那么，对于二进制文件来说，当保存每次提交的改动时，会保存整个文件，会导致仓库快速增大，包括了网络带宽。在 2015.04 GitHub 推出了 Large File Storage, LFS 将标记的大文件保存在另外的仓库，而主仓库仅保留其轻量级指针。

---

GIT 代码管理是基于文本文件行的，那么，对于二进制文件来说，当保存每次提交的改动时，会保存整个文件，会导致仓库快速增大，包括了网络带宽。

在 2015.04 GitHub 推出了 Large File Storage, LFS 将标记的大文件保存在另外的仓库，而主仓库仅保留其轻量级指针。

<!-- more -->

## 简介

主要是为了处理大文件问题，在 git-lfs 之前，已经存在一个 [git-annex](https://git-annex.branchable.com/) 提供类似的大文件解决方案，使用时需要保证 git 大于 1.8.2 版本。

### 安装

不同的平台可以参考 [Installation](https://github.com/git-lfs/git-lfs/wiki/Installation) 中介绍的安装方法，也可以直接从 [github release](https://github.com/git-lfs/git-lfs/releases) 上下载对应的安装包，以 CentOS 为例，直接下载 RPM 包安装即可。

### Server

有很多 LFS 服务器的实现，实际上就是提供一些简单的 HTTP 接口，可以直接搜索 git lfs server 关键字，也可以参考 [Server Implementations](https://github.com/git-lfs/git-lfs/wiki/Implementations) 中的相关链接。

### 示例

简单介绍其使用流程。

{% highlight text %}
----- 每个用户只需要运行一次，通过--local仅本地，uninstall取消
$ git lfs install

----- 使用通配符设置跟踪模式，默认保存在.gitattributes文件中
$ git lfs track '*.bin'
$ cat .gitattributes 
*.bin filter=lfs diff=lfs merge=lfs -text
$ git lfs track
Listing tracked patterns
    *.bin (.gitattributes)
Listing excluded patterns

----- 生成并添加一个二进制文件
$ dd if=/dev/urandom of=dog.bin bs=1048576 count=1
$ git add -A .
$ git commit -m 'add binary file'
$ git lfs ls-files

----- Clone仓库，但是不更新LFS文件(默认更新)
$ GIT_LFS_SKIP_SMUDGE=1 git clone
{% endhighlight %}

现在的 git 貌似是直接能够拉取所有文件，包括 lfs 文件，如果不想拉取 lfs 文件，可以使用 

其它常用命令。

{% highlight text %}
----- 查看LFS对象的当前状态
$ git lfs status
{% endhighlight %}

## 原理

在 git 库中，存在一个 `.gitattributes` 管理文件，用来保存改库中相关文件的属性，包括了如何过滤、合并、diff 等等。其中 git-lfs 就是利用 `.gitattributes` 完成偷梁换柱的，详细可以参考 [Git LFS Specification](https://github.com/git-lfs/git-lfs/blob/master/docs/spec.md) 中的介绍。

在提交时，假设添加了一个 `cat.bin` 文件，此时会计算该文件的 hash 值，文件保存在 `.git/lfs/objects` 目录下 (子目录为 hash 值的映射)，上传仓库的是一个 Pointer 文本文件，文件中包含了 hash 以及大小等信息，类似如下。

{% highlight text %}
version https://git-lfs.github.com/spec/v1
oid sha256:e46f752aac0a87edc6f148def9d82b1a5f7f3fe9724bc535b98926795395c44f
size 1048576
{% endhighlight %}

同时会调用 `pre-push` 中的钩子，通过 `git-lfs` 命令将文件发送到远程 LFS 服务器中。

## 其它

### 问题排查

在执行一些操作时，如果因为某些原因被卡住，那么可以通过如下方式，打印详细的执行步骤。

{% highlight text %}
GIT_TRACE=2 git clone https://www.foobar.com/foobar.git
GIT_CURL_VERBOSE=1 git clone https://www.foobar.com/foobar.git
{% endhighlight %}

其中后者会打印更多的信息。

## 参考

* [git-lfs.github.com](https://git-lfs.github.com/) 官方网站、[Git LFS Tutorial](https://github.com/git-lfs/git-lfs/wiki/Tutorial) 简单教程。

<!--
Git LFS 使用总结
http://www.ikouz.com/2018/05/git-lfs-%E4%BD%BF%E7%94%A8%E6%80%BB%E7%BB%93/
-->

{% highlight text %}
{% endhighlight %}
