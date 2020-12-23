---
title: 可执行文件解析
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: elf,objdump,readelf
description: 主要是在如何解析 ELF 格式的文件。
---

主要是在如何解析 ELF 格式的文件。

<!-- more -->

## objdump

详细参考 `man objdump` 。

{% highlight text %}
-h, --section-headers, --headers
  查看目标文件的头部信息。
-x, --all-headers
  显示所有的头部信息，包括了符号表和重定位表，等价于 -a -f -h -p -r -t 。
-s, --full-contents
  显示所请求段的全部信息，通常用十六进制表示，默认只会显示非空段。
-d, --disassemble
  反汇编，一般只反汇编含有指令的段。
-t, --syms
  显示符号表，与nm类似，只是显示的格式不同，当然显示与文件的格式相关，对于ELF如下所示。
  00000000 l    d  .bss   00000000 .bss
  00000000 g       .text  00000000 fred
{% endhighlight %}

<!--
第一列为符号的值，有时是地址；下一个是用字符表示的标志位；接着是与符号相关的段，*ABS* 表示段是绝对的（没和任何段相关联）， *UND* 表示未定义；对于普通符号(Common Symbols)表示对齐，其它的表示大小；最后是符号的名字。<br><br>

对于标志组的字符被分为如下的 7 组。
<ol type="A"><li>
    "l(local)" "g(global)" "u(unique global)" " (neither global nor local)" "!(both global and local)"<br>
    通常一个符号应该是 local 或 global ，但还有其他的一些原因，如用于调试、"!"表示一个bug、"u"是 ELF 的扩展，表示整个进程中只有一个同类型同名的变量。</li><br><li>

    "w(weak)" " (strong)"<br>
    表示强或弱符号。</li><br><li>

    "C(constructor)" " (ordinary)"<br>
    为构造函数还是普通符号。</li><br><li>

    "W(warning)" " (normal symbol)"<br>
    如果一个含有警告标志的符号被引用时，将会输出警告信息。</li><br><li>

    "I"
   "i" The symbol is an indirect reference to another symbol (I), a function to be evaluated
       during reloc processing (i) or a normal symbol (a space).

   "d(debugging symbol)" "D(dynamic symbol)" " (normal symbol)"<br>
    表示调试符号、动态符号还是普通的符号。</li><br><li>

   "F(function)" "f(file)" "O(object)" " (normal)"<br>
    表示函数、文件、对象或只是一个普通的符号。
-->

## readelf

用于读取 ELF 格式文件，包括可执行程序和动态库，常用参数如下。

{% highlight text %}
-a --all
  显示所有信息，等价于-h -l -S -s -r -d -V -A -I
-h --file-header
  文件头信息；
-l --program-headers
  程序的头部信息；
-S --section-headers
  各个段的头部信息；
-s --syms
  显示符号表，也就是.symtab段；
-e --headers
  全部头信息，等价于-h -l -S；
-x, --hex-dump=<number or name>
  十六进制方式打印某个段；
{% endhighlight %}

示例用法：

{% highlight text %}
----- 读取dynstr段，包含了很多需要加载的符号，每个动态库后跟着需要加载函数
$ readelf -p .dynstr hello

----- 以十六进制方式读取dynstr段
$ readelf -x .dynstr hello

----- 查看是否含有调试信息
$ readelf -S hello | grep debug
{% endhighlight %}



<!--
readelf  -S hello
readelf -d hello

  --sections
An alias for –section-headers
--symbols
An alias for –syms
-n –notes 内核注释 Display the core notes (if present)
-r –relocs 重定位 Display the relocations (if present)
-u –unwind Display the unwind info (if present)
-d --dynamic
  显示动态段的内容；
-V –version-info 版本 Display the version sections (if present)
-A –arch-specific CPU构架 Display architecture specific information (if any).
-D –use-dynamic 动态段 Use the dynamic section info when displaying symbols
-x –hex-dump=<number> 显示 段内内容Dump the contents of section <number>
-w[liaprmfFso] or
-I –histogram Display histogram of bucket list lengths
-W –wide 宽行输出 Allow output width to exceed 80 characters
-H –help Display this information
-v –version Display the version number of readelf
-->


## objcopy

用于转换目标文件。

{% highlight text %}
常用参数：
  -S / --strip-all
    不从源文件中拷贝重定位信息和符号信息到输出文件(目的文件)中去。
  -I bfdname/--input-target=bfdname
    明确告诉程序源文件的格式是什么，bfdname是BFD库中描述的标准格式名。
  -O bfdname/--output-target=bfdname
    使用指定的格式来写输出文件(即目标文件)，bfdname是BFD库中描述的标准格式名，
    如binary(raw binary 格式)、srec(s-record 文件)。
  -R sectionname/--remove-section=sectionname
    从输出文件中删掉所有名为section-name的段。
{% endhighlight %}

上一步的 strip 命令只能拿掉一般 symbol table，有些信息还是沒拿掉，而这些信息对于程序的最终执行没有影响，如: `.comment` `.note.ABI-tag` `.gnu.version` 就是完全可以去掉的。

所以说程序还有简化的余地，我们可以使用 objcopy 命令把它们抽取掉。

{% highlight text %}
$ objcopy -R .comment -R .note.ABI-tag -R .gnu.version hello hello1
{% endhighlight %}

## nm

用来显示指定文件中的符号信息，可以是对象文件、可执行文件、动态库等。

### 符号

第二列标示了符号的类型，大写表示为全局变量，小写则表示为局部的变量。

* I 对另一个符号的间接引用，一般为动态库。
* T 位于代码区。

当出现了 `I` 指定的符号时，另外比较常见的是通过 `@` 指定版本号，例如 `memcpy@@GLIBC_2.14` 或者 `memcpy@GLIBC_2.2.5` ，

## 其它

### strip

我们知道二进制的程序中包含了大量的符号表格(symbol table)，有一部分是用来 gdb 调试提供必要信息的，可以通过如下命令查看这些符号信息。

{% highlight text %}
$ readelf -S hello
{% endhighlight %}

其中类似与 `.debug_xxxx` 的就是 gdb 调试用的。去掉它们不会影响程序的执行。

{% highlight text %}
$ strip hello
{% endhighlight %}

### hexdump

用来查看二进制文件。


{% highlight text %}
-n, --length length  指定显示的长度
-s, --skip offset    跳过的字节数
{% endhighlight %}

### nm VS. readelf -s

对于一个执行文件，当执行了 `strip` 之后，如果通过 `nm` 命令查看，会直接报 `no symbols` 的错误，而通过 `readelf -s` 却可以查到，也可以通过 `strings` 查看。

<!--
在 ELF 文件中存在着两类符号表：A)
    why nm give no result for striped libtest.so

    There are two symbol tables in the original libtest.so: a "regular" one (in .symtab and .strtab sections) and a dynamic one (in .dynsym and .dynstr sections).

    If strip removed both symbol tables, you library would be completely useless: the dynamic loader couldn't resolve any symbols in it. So strip does the only thing that makes sense: removes the "regular" symbol table, leaving the dynamic one intact.

    You can see symbols in the dynamic symbol table with nm -D or readelf -s.

    The "regular" symbol table is useful only for debugging (for example, it contains entries for static functions, which are not exported by the library, and do not show up in the dynamic symbol table).

    But the dynamic loader never looks at the "regular" symbol table (which is not in a format suitable for fast symbol lookups); only at the dynamic one. So the "regular" symbol table is not needed for correct program operation, but the dynamic one is.
-->



{% highlight text %}
{% endhighlight %}
