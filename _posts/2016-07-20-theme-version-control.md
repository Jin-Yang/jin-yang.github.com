---
title: 【专题】版本管理
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![Git Logo]({{ site.url }}/images/misc/git-logo.jpg "Git Logo"){: .pull-center width="230" }

Git 是一免费、开源的分布式版本控制系统，可有效、高速的处理从很小到非常大的项目版本管理，该工具是 Linus Torvalds 为了帮助管理 Linux 内核开发而开发，其本意是为了替换 BitKeeper 。

除了 GitHub 外，开源的工具还有 [GitLab](https://about.gitlab.com/)、[Gogs](https://gogs.io/)，以及 C 的相关库 [libgit2](https://github.com/libgit2/libgit2) 。

这里简单介绍一下常见概念及其操作。

* [Git 简明教程](/post/git-simple-guide.html) 简单介绍常见操作。
* [Git 分支管理](/post/git-branch-model.html) 主要介绍 git 的分支处理常用操作，以及比较经典的版本分支管理方式。
* [Git 使用杂项](/post/git-tips.html) 记录 git 常见的示例，可以用来作为参考使用。

### 内部原理

* [Git 对象简介](/post/git-internal-object-introduce.html) 内部使用一组极为简洁的数据结构来维护，也就是对象。

### 常用技巧

* [Git 多个远程库](/post/git-multi-remote-repos.html) 也就是同时配置多个远端仓库地址。
* [Git Cherry Pick 使用](/post/git-cherry-pick-introduce.html) 将某个分值的提交到其它的分支上。
* [Git 清理空间](/post/git-cleanup-method-introduce.html) 常见的是如何清理大文件、做开源的准备等。



{% highlight text %}
{% endhighlight %}
