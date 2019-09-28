---
title: VIM Tags 相关
layout: post
comments: true
language: chinese
category: [misc]
keywords: vim,编辑器
description:
---

符号索引是个重要功能，不论阅读新项目，还是开发复杂点的大项目，符号索引都能帮你迅速掌握项目脉络，加快开发进度。

在 VIM 中，大部分的代码浏览都要依赖 tags 的使用。

<!-- more -->

## 简介

传统 ctags 系统虽和 vim 结合紧密，但只能查定义无法查引用，cscope 能查引用，但只支持 C 语言，C++ 都不支持，况且常年不更新。

ctags 由于使用文本格式存储数据，虽用了二分查找，但打开 Linux Kernel 这样的大项目时，查询会有卡顿的感觉。

相比来说，gtags 由如下的几个特点：

* 不但能查定义，还能查引用；
* 原生支持 6 种语言 C C++ Java PHP4 ASM Yacc，扩展支持 50+ 种语言；
* 使用性能更好的本地数据库存储符号，而非文本；
* 支持增量更新，每次只索引改变过的文件；
* 多种输出格式，能更好的同编辑器相集成。

## gtags

实际上，gtags 是 GNU Global 软件包中的一个程序，也是最为常用的一个，类似于 ctags 的代码导航工具，可以为 vim emacs 等添加代码调转功能；另外还有 gtags-cscope 代替 cscope 。

GNU Global 内置的解析器支持 C C++ Java PHP ASM YACC 六种语言，通过 exuberant-ctags 扩展可以支持 ctags 支持的所有语言。

在 CentOS 中可以通过如下命令安装。

{% highlight text %}
# yum install --enablerepo=epel global
{% endhighlight %}

### gtags-cscope

一些常用的快捷键：

* `<leader>cg` 查看光标下符号的定义；
* `<leader>cs` 查看光标下符号的引用；
* `<leader>cc` 查看有哪些函数调用了该函数；
* `<leader>cf` 查找光标下的文件；
* `<leader>ci` 查找哪些文件 include 了本文件。

## Gutentags

其中 gutentags 可以用来动态生成对应的 tags 数据库，而 gutentags_plus 则提供了无缝切换 tags 的功能。

相关内容可以参考官网 [bolt80.com](https://bolt80.com/gutentags/) 。

### 问题排查

一般的报错信息为 `gutentags: gutentags: gtags-cscope job failed, returned: 1` ，也就是说在生成 tag 数据时报错。

判断 gtags 为何失败，需进一步打开日志，查看 gtags 的错误输出：

{% highlight text %}
let g:gutentags_define_advanced_commands = 1
{% endhighlight %}

在 vimrc 中添加上面的配置，允许 gutentags 打开一些高级命令和选项，打开出错文件，执行 `:GutentagsToggleTrace` 打开日志信息，保存文件，然后触发 gtags 数据库更新。

也可以通过 `:messages` 命令列出所有消息记录，即可看到 gtags 的错误输出，方便定位。

最简单的方式，如果开始是正常的，可以直接删除掉 cache 信息。

<!--
https://github.com/skywind3000/gutentags_plus
https://github.com/ludovicchabant/vim-gutentags
-->


<!--
http://ju.outofmemory.cn/entry/112383

非常经典的介绍
https://zhuanlan.zhihu.com/p/36279445
-->

{% highlight text %}
{% endhighlight %}
