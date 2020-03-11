---
title: LLVM
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

LLVM 计划启动于 2000 年，由 UIUC 大学的 Chris Lattner 博士主持，后来入职 Apple 并继续推广，所以 Apple 也就成了主要的赞助商。

最初是 Low Level Virtual Machine 的缩写，不过随着发展，逐渐整合成了一整套的编译工具，所以官方也就放弃了原有的缩写，而修改成 The LLVM Compiler Infrastructure 。

其强大之处在于模块化，可以很方便适配不同的语言以及硬件平台。

<!-- more -->

## LLVM

LLVM 是一个模块化、可重用的编译器及其工具链相关的技术。

![llvm arch]({{ site.url }}/images/compiler/llvm-arch.png "llvm arch"){: .pull-center }

一般编译器分成了三部分：A) Frontend 前端，包括了词法分析、语法分析、语义分析、生成中间代码；B) Optimizer 优化器，中间代码优化；C) Backend 后端，生成机器码。

不同的前端后端使用统一中间代码，如果需要支持一种新的变成语言，那么只需要实现一个新的前端；同样，如果支持一种新的硬件设备，只需要实现一个新的后端。

优化阶段不论是支持新的变成语言，还是支持新的硬件设备，都不需要对优化阶段做修改。

<!--
http://llvm.org/
-->

## clang

Clang 是 LLVM 项目的一个子项目，用作 C C++ Objective-C 编译器的前端。

{% highlight text %}
# yum install clang
{% endhighlight %}

如下是常见的使用命令。

{% highlight text %}
$ cat main.c
int main(void)
{
	return 0;
}

----- 打印各个阶段
$ clang -ccc-print-phases main.c

----- 预处理结果
$ clang -E main.c

----- 词法分析
$ clang -fmodules -E -Xclang -dump-tokens main.c

----- 打印语法树
$ clang -fmodules -fsyntax-only -Xclang -ast-dump main.c
{% endhighlight %}

也以生成保存中间码。

<!--
http://clang.llvm.org/
-->


{% highlight text %}
{% endhighlight %}
