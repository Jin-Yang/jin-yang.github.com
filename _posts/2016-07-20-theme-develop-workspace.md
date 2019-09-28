---
title: 【专题】开发环境
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## GIT

![Git Logo]({{ site.url }}/images/misc/git-logo.jpg "Git Logo"){: .pull-center width="28%" }

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
* [Git 撤销操作](/post/git-cancel-reset-operation-introduce.html) 对于不同的场景，一些常见的撤销处理方法。
* [Git 统计工具](/post/git-some-statistic-commands-and-tools-introduce.html) 对于一些常见指标的统计。
* [Git Patch 使用](/post/git-some-basic-patch-comand-usage.html) 通过 git 可以很容易将 Patch 提取出来。

### GOGS

Gogs 的功能类似于 GitHub 或者 GitLab ，不过相比来说是一款极易搭建的 Git 服务，其目标是打造一个最简单、最快速和最轻松的方式搭建自助 Git 服务。

* [Gogs 仓库使用](/post/linux-git-gogs-introduce.html) 类似 GitLab 或者 GitHub，但是搭建起来更容易搭建。

## 词法语法解析

Lex (A Lexical Analyzer Generator) 和 Yacc(Yet Another Compiler Compiler) 是 UNIX 中两个非常重要的、功能强大的工具，分别用来做词法扫描以及语法分析。

在 Linux 上就是 flex-bison，其中在使用 bison 时，采用的语法必须是上下文无关文法 (context-free grammar)。

这里简单介绍，包括常规表达式、声明、匹配模式、变量、Yacc 语法和解析器代码。

* [基本概念](/post/program-lexical-basic-introduce.html) 关于词法语法分析的基本概念，例如 BNF、上下文无关等。
* [Flex 使用简介](/post/program-concept-lexical-flex-introduce.html) 词法解析器的介绍，在 Linux 中的实现。
* [Bison 使用简介](/post/program-concept-syntax-bison-introduce.html) 语法解析器的介绍，Linux 中对应 Yacc 的实现。
* [MySQL 语法解析](/post/mysql-parser.html) MySQL 中使用方式，词法解析独立实现，语法分析则使用 Bison 。

## VIM

![vim logo]({{ site.url }}/images/misc/vim_logo.png "vim logo"){: .pull-center width="25%" }

Vim 是一个功能强大、高度可定制的文本编辑器，与其相匹敌的是 Emacs ，这两个都是不错的编辑器，在此不再比较两者的优劣，仅介绍 Vim 相关的内容。

### 功能介绍

* [VIM 基本功能](/post/linux-vim-introduce.html) 一功能强大、高度可定制的文本编辑器，介绍其安装和基本用法。
* [VIM 插件使用](/post/linux-vim-third-plugins-introduce.html) 一些常用的三方组件安装、配置、使用方法。
* [VIM Tags 相关](/post/linux-vim-third-plugins-tags-stuff-introduce.html)
* [VIM LaTeX 使用](/post/linux-vim-latex-snippets-introduce.html)

## 其它

一些在开发阶段常见的使用工具。

* [TMUX 简介](/post/tmux-introduce.html) 终端复用工具，类似 screen 但是更加方便使用，不过更加高端。

{% highlight text %}
{% endhighlight %}
