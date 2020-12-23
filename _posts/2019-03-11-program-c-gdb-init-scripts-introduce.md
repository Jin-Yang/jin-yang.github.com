---
title: GDB INIT 使用
layout: post
comments: true
language: chinese
category: [program]
keywords: gdb
description:
---

在启动时，会在当前用户目录下寻找文件名为 `.gdbinit` 的文件，如果存在，会执行该文件中的所有命令，通常用于简单的配置命令。

<!-- more -->

## 简介

通常使用该文件用于简单的配置，如设置所需的默认汇编程序格式、输出数据默认基数，还可以读取宏编码语言，从而实现更强大的自定义函数。

<!--
define <command>
<code>
end
document <command>
<help text>
end

    .gdbinit 的配置繁琐, 因此某些大神想到了用插件的方式来实现, 通过 Python 的脚本可以很方便的实现我们需要的功能
-->

<!--
https://blog.csdn.net/gatieme/article/details/63254211
-->

## pwngdb

## 参考

* 一些常见的 init 项目，包括了 [gdbinit](https://github.com/gdbinit/Gdbinit)、[PEDA](https://github.com/longld/peda)、[GDF](https://github.com/hugsy/gef)、[pwngdb](https://github.com/pwndbg/pwndbg) ，后两者相对新一些。

{% highlight text %}
{% endhighlight %}
