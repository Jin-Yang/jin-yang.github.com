---
title: ELF 符号表
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,cpp,c++,program
description: 整理下 C 语言中调试、发布的流程。
---


<!-- more -->

## -rdynamic VS. -g

如果通过 `readelf -s` 查看，可以发现，使用 `-rdynamic` 选项后，在 `.dynsym` 段中增加了很多符号，包括本程序内定义的符号。

* `-g` 会添加调试信息(.debug_xxx)，通常被 gdb 使用，可以通过 strip 删除。
* `-rdynamic`  是添加动态连接符号信息，用于动态连接功能，比如 `dlopen()`、`backtrace()` 系列函数使用，不能被 strip 掉。

在 ELF 文件中包含了 `.symtab` 和 `.dynsym` 符号表，前者包含的内容更多，通过 strip 会去掉 `.symtab` 而不会去掉 `.dynsym`。

<!--
https://linuxtools-rst.readthedocs.io/zh_CN/latest/advance/02_program_debug.html
-->

## addr2line

addr2line 常见的用处就是根据程序崩溃时候打印的堆栈地址，用来查看出错的源代码的行数。

其原理是，在通过 `-g` 参数编译生成 ELF 对象时，最终的生成文件包含有几个 `.debug` 段，例如，`.debug_line` 包含了每个汇编指令对应的地址和源代码行数源代码名字等信息。

`objdump`、`gdb` 都是可以解析这个 `.debug_line` 段，可以通过如下命令查看。

{% highlight text %}
$ objdump -S -d test
{% endhighlight %}

如果通过 `strip -d` 把对应 ELF 文件的几个 debug 段都删除掉，那么 addr2line、objdump、gdb 都不能显示代码行数和汇编的关系了。

{% highlight text %}
----- 查看ELF文件各个段的名称
$ readelf --sections test
$ objdump -x test

----- 查看调试段的内容
$ readelf -wl test
$ readelf --debug-dump test
{% endhighlight %}

其中 `.symtab` 段只保存函数名和变量名等基本的符号的地址和长度等信息的，所以 `backtrace()` 可以根据地址得到出错的是哪个函数和在函数中的偏移，但具体到源码的哪一行还是要靠 `.debug_line` 这里面的信息的。

gdb 里面可以通过 `file` 或者 `add-symbol-file` 来加载符号表的，如果需要源码行数信息就需要用 `-g` 重新编译程序以生成 `.debug_line` 段才能查看。

### dmesg

在系统 `dmesg` 中可以发现系统日志的错误信息：

{% highlight text %}
[54106.016179] test1[8352] trap divide error ip:400506 sp:7fff2add87e0 error:0 in test1[400000+1000]
{% endhighlight %}

这里的 IP 字段后面的数字就是程序出错时所程序执行的位置，可以使用 `addr2line` 将地址 `400506` 转换成出错程序的位置：

{% highlight text %}
$ addr2line -e test 400506
/code/test/addr2line/test1.c:5
{% endhighlight %}

### 注意

1. 通常在使用 `-O1` 或者 `-O2` 编译时会对代码进行优化，包括了 `inline` 方式，此时可能会忽略部分的函数。

## 总结

简单来说，如果需要在 Backtrace 中打印函数，需要添加 `-rdynamic` 参数，如果要通过 `addr2line` 显示行数则需要添加 `-g` 参数。


{% highlight text %}
{% endhighlight %}
