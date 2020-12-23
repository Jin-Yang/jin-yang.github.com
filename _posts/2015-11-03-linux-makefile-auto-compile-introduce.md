---
title: Linux 自动编译 Makefile
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,automake,configure,cmake
description: 在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。这里简单介绍下这种构建方式的细节，以及开发者如何利用 autotools 创建兼容 GNU 构建系统的项目。
---

在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。

这里简单介绍下最基本的 Makefile 。

<!-- more -->

## 简介

在 Linux 平台下，在编译项目时通常使用 Makefile，简单来说，Makefile 是用来定义整个工程的编译规则，也就是文件的编译、链接顺序，如何生成可执行文件或者动态加载库等等。

Makefile 带来的最大好处就是 "自动化编译"，一但编辑好文件，只需要一个 make 命令，整个工程完全自动编译，极大的提高了软件开发的效率。

如果项目比较小，那么可以直接手动编辑该文件；不过一但项目比较大时，手动维护将变得极其复杂，为此，就可以使用 Autotools 或者 CMake 生成 Makefile 文件。

假设项目工程的使用源码结构如下。

{% highlight text %}
src/
   子目录，存放工程源码；
doc/
   子目录，用来存放工程文档；
bin/
   子目录，最后生成的二进制可执行文件；
COPYRIGHT
   版权信息；
README.md
   使用 markdown 编写的自述文件；
{% endhighlight %}

默认，会将可执行文件安装在 /usr/bin 目录下，doc 安装到 /usr/share/doc 目录下。


## Makefile

详细内容可以参考陈皓编写的 [跟我一起写 Makefile](/reference/linux/Makefile.pdf)，在此就不做过多介绍了，仅简单记录下。

### 变量

如下是 Makefile 中内置的变量。

{% highlight text %}
$@:  规则中的目标名（也就是规则名）；
$<:  规则中的依赖项目,只代表规则所有依赖项目中的第一项；
$^:  规则中所有的依赖项目；
$?:  规则中时间新于目标的依赖项目。
{% endhighlight %}

关于变量通配符，与 shell 相同，如 1) `?` 任意单个字符；2) `*` 任意字符的字符串；3) `[set]` 任何在 set 里的字符；4) `[!set]` 任何不在 set 里的字符。

以如下为例 `touch {a,b,c}.c Makefile` 。

{% highlight makefile %}
.PHONY: all

src1=$(wildcard *.c)
src2=%.c
src3=*.c
objs:=$(patsubst $(src2),%.o,$(wildcard *.c))

all: $(objs)
    @echo $^
    @echo $(src1)
    @echo $(src2)
    @echo $(src3)
    @echo *.c

%o:%.c
    @echo $?

#cc    -c -o a.o a.c
#cc    -c -o c.o c.c
#cc    -c -o b.o b.c
#a.o c.o b.o
#a.c c.c b.c
#%.c
#a.c b.c c.c
#a.c b.c c.c
{% endhighlight %}


### PHONY 伪目标

通常来说 Makefile 会检测 `:` 左侧的目标是不是最新的，如果是最新的则不会更新，对应规则不会执行。假设目标为 clean，本意是做编译后的清理，但是当目录下有 clean 文件时，则对应的规则将不会执行，为了解决这一问题，定义了伪目标。

当定义了伪目标之后，make 在执行规则时不会去试图去查找隐含规则来创建它，而是直接执行，这样就提高了 make 的执行效率，也不用担心由于目标和文件名重名了。

伪目标的另一种使用场合时在 make 的并行和递归执行过程中，第一个实际上时串行执行的；第二个会并行执行。

{% highlight text %}
### 1
SUBDIRS=foo bar baz
subdirs:
    for dir in $(SUBDIRS)
    do
    $(MAKE) –C $$dir
    done

### 2
.PHONY:subdirs $(SUBDIRS)
SUBDIRS=foo bar baz
subdirs: $(SUBDIRS)
$(SUBDIRS):
    $(MAKE) –C $@
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
