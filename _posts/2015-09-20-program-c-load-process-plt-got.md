---
title: C 加载过程
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,加载过程
description: 利用动态库，可以节省磁盘、内存空间，而且可以提高程序运行效率；不过同时也导致调试比较困难，而且可能存在潜在的安全威胁。这里主要讨论符号的动态链接过程，即程序在执行过程中，对其中包含的一些未确定地址的符号进行重定位的过程。
---

利用动态库，可以节省磁盘、内存空间，而且可以提高程序运行效率；不过同时也导致调试比较困难，而且可能存在潜在的安全威胁。

这里主要讨论符号的动态链接过程，即程序在执行过程中，对其中包含的一些未确定地址的符号进行重定位的过程。

<!-- more -->

## 简介

假设有如下的示例。

{% highlight c %}
#include <stdio.h>

int main(void)
{
	puts("Hello World.");
	return 0;
}
{% endhighlight %}

可以通过 `gcc -o main main.c` 编译生成二进制文件，然后通过 `readelf -S main` 查看当前的段信息，其中比较关键的有。

{% highlight text %}
$ readelf -S main | grep -E '(plt|got)'
  [10] .rela.plt         RELA             0000000000400448  00000448
  [12] .plt              PROGBITS         0000000000400480  00000480
  [21] .got              PROGBITS         0000000000600fe0  00000fe0
  [22] .got.plt          PROGBITS         0000000000601000  00001000
{% endhighlight %}

各个段的用途如下。

* `.got` Global Offset Table 会通过链接器将实际函数的地址填充。
* `.plt` Procedure Linkage Table 编辑器生成的代码，如果已经填充了 `.got.plt` 那么会直接跳转，否则会调用链接器查找对应的函数。
* `.got.plt` 如果链接器已经完成了查找，那么就包含了实际函数地址，或值跳转到 `.plt` 地址执行查找。

### 符号表

函数和变量作为符号被存在可执行文件中，同时会将不同类型的符号又放到一块，称为符号表，包括了两类：A) 常规 `.symtab` `.strtab`；B) 动态 `.dynsym` `.dynstr` 。

{% highlight text %}
$ readelf -S main | grep -E '\.(dyns|symtab|strtab)'
  [ 5] .dynsym           DYNSYM           00000000004002b8  000002b8
  [ 6] .dynstr           STRTAB           0000000000400348  00000348
  [27] .symtab           SYMTAB           0000000000000000  000016d0
  [28] .strtab           STRTAB           0000000000000000  00001eb0
{% endhighlight %}

常规的符号表通常只在调试时使用，正常发布的二进制文件会 `strip` 调，而动态符号表则是程序执行时候真正会查找的目标。

{% highlight text %}
$ readelf -S /bin/bash | grep -E '\.(dyns|symtab|strtab)'
  [ 6] .dynsym           DYNSYM           0000000000004ca8  00004ca8
  [ 7] .dynstr           STRTAB           0000000000012af0  00012af0
{% endhighlight %}

另外，生成的目标文件是没有 `.dyn*` 类型的符号表的。

除此之外，还有一个 `.shstrtab` 保存的是段表字符串表 (Section Header String Table)，也就是通过 `readelf -S` 读出的段名称。

每个段的地址可以通过 `ld --verbose` 查看。

#### 符号地址

在目标文件中符号是没有地址的，只有在生成可执行文件后会指定。

{% highlight text %}
$ gcc -c main.c
$ nm main.o
0000000000000000 T main
                 U puts

$ gcc -o main main.c
$ nm main | grep -E "(\<main|puts)"
0000000000400586 T main
                 U puts@@GLIBC_2.2.5
{% endhighlight %}

在链接后，函数 `main()` 的地址已经确定，但是，动态库中的 `puts()` 函数是没有指定的，而该函数在标准 C 库中实现，可以通过如下方法查看。

{% highlight text %}
$ nm -D /usr/lib64/libc-2.28.so  | grep -E '\<puts\>'
0000000000073010 W puts
{% endhighlight %}

也可以通过 `readelf -s` 查看，上述表明，这是一个若引用，其它的库可以将其覆盖。

<!--
关于.strtab，.symtab，.shstrtab
`.strtab` 是字符串表（STRING TABLE）
.symtab是符号表，一般是变量、函数
-->

### 重定位

在编译时，只知道外部符号的类型，而不知道具体函数或变量的地址，地址是通过重定位的方式获取，有两种重定位方式：A) 链接时重定位；B) 运行时重定位。

在链接阶段会将中间文件通过链接器链接成一个可执行文件，主要做的事情有：A) 对各个中间文件的同名段进行合并；B) 对代码段、数据段等进行地址分配；C) 链接时重定位。

在重定位时，如果在其它中间文件中已经定义，链接阶段可以直接重定位到函数地址；如果是在动态库中定义了的函数，链接阶段无法直接重定位到函数地址，就需要使用如下介绍的方式。

### 基本概念

在动态库中的函数以及变量会在运行时动态确定，编译链接后的二进制文件中包含了几个与之相关的段，运行时加载到内存中，然后以此确定真实的地址。

* `.got` Global Offset Table 全局偏移表，这是链接器为外部符号填充的实际偏移表。
* `.plt` Procedure Linkage Table 程序链接表，链接器生成的代码片段，实现懒加载功能。

<!--
* `.got.plt`
这个是 GOT 专门为 PLT 专门准备的节。说白了，.got.plt 中的值是 GOT 的一部分。它包含上述 PLT 表所需地址（已经找到的和需要去触发的）
-->

注意，GOT PLT 只是 Linux 中实现的一种动态链接的方式。

### 位置无关代码

在执行程序时，需要先将磁盘上的文件读取到内存中, 然后再执行，而每个进程都有自己的虚拟内存空间，32 位程序的寻址空间为 `2^32` 大小，而 64 位当前为 `2^48` ，虚拟内存最终会通过页表映射到物理内存中。

<!--
https://manybutfinite.com/post/anatomy-of-a-program-in-memory/
-->

按照 `System V ABI` 的规定，32 位程序会加载到 `0x08048000` 这个地址中，而 64 位则会加载到 `0x00400000` 中，那么所写的程序，实际就是以这个地址为基础，对变量进行绝对地址寻址。

<!--
https://www.cnblogs.com/l2017/p/11879310.html
-->

例如，如下程序



.data部分在可执行文件中的偏移量为0x1014, 那么加载到虚拟内存中的地址应该是
0x8048000+0x1014=0x804a14, 正好和显示的结果一样. 再看看main函数的汇编代码:

{% highlight text %}
$ readelf -S main | grep -A 1 -E "\.data"
  [Nr] Name              Type             Address           Offset
	Size              EntSize          Flags  Link  Info  Align
  [23] .data             PROGBITS         0000000000601020  00001020
       0000000000000004  0000000000000000  WA       0     0     1
{% endhighlight %}





## 动态解析

控制权先是提交到解释器，由解释器加载动态库，然后控制权才会到用户程序。动态库加载的大致过程就是将每一个依赖的动态库都加载到内存，并形成一个链表，后面的符号解析过程主要就是在这个链表中搜索符号的定义。

<!--
对于静态文件通常只有一个文件要被映射，而动态则还有所依赖的共享目标文件，通过 /proc/PID/maps 可以查看在内存中的分布。
地址随机实际上包括了动态链接库、堆、栈，而对于程序本身的函数，其地址是固定的。
-->

{% highlight text %}
$ cat test.c
#include <stdio.h>

int main(void)
{
	puts("Hello World");
	return 0;
}

----- 编译连接
$ gcc test.c -o test -g
----- 打印程序的反汇编
$ objdump -S test

----- 使用gdb调式
$ gdb test -q
(gdb) break main
(gdb) run
(gdb) disassemble
Dump of assembler code for function main:
   0x0000000000400586 <+0>:     push   %rbp
   0x0000000000400587 <+1>:     mov    %rsp,%rbp
=> 0x000000000040058a <+4>:     mov    $0x400638,%edi
   0x000000000040058f <+9>:     callq  0x400490 <puts@plt> 此处调用的地址是固定的
   0x0000000000400594 <+14>:    mov    $0x0,%eax
   0x0000000000400599 <+19>:    pop    %rbp
   0x000000000040059a <+20>:    retq
End of assembler dump.
{% endhighlight %}

从上面反汇编代码可以看出，调用 `puts()` 函数时，实际上调用的是 `puts@plt` 这个符号，也就是位于 `0x400490` 地址处，实际上这是一个 PLT 条目，可以通过反汇编查看相应的代码，不过它代表什么意思呢？

### PLT

上述会跳转到 `puts@plt` 中，可以直接通过 `objdump -S test` 命令查看反汇编，其中的 `.plt` 内容如下。

{% highlight text %}
Disassembly of section .plt:

0000000000400480 <.plt>:
  400480:       ff 35 82 0b 20 00       pushq  0x200b82(%rip)        # 601008 <_GLOBAL_OFFSET_TABLE_+0x8>
  400486:       ff 25 84 0b 20 00       jmpq   *0x200b84(%rip)        # 601010 <_GLOBAL_OFFSET_TABLE_+0x10>
  40048c:       0f 1f 40 00             nopl   0x0(%rax)

0000000000400490 <puts@plt>:
  400490:       ff 25 82 0b 20 00       jmpq   *0x200b82(%rip)        # 601018 <puts@GLIBC_2.2.5>
  400496:       68 00 00 00 00          pushq  $0x0
  40049b:       e9 e0 ff ff ff          jmpq   400480 <.plt>
{% endhighlight %}

当然，也可以通过 `gdb` 命令进行反汇编。

{% highlight text %}
(gdb) disassemble 0x400490
Dump of assembler code for function puts@plt:
=> 0x0000000000400490 <+0>:     jmpq   *0x200b82(%rip)        # 0x601018 <puts@got.plt>
   0x0000000000400496 <+6>:     pushq  $0x0
   0x000000000040049b <+11>:    jmpq   0x400480
End of assembler dump.
{% endhighlight %}

可以看到 `puts@plt` 中包含三条指令，而且可以看出，除 `PLT0(__gmon_start__@plt-0x10)` 所标记的内容，其它的所有 `PLT` 项的形式都是一样的，而且最后的 `jmp` 指令都是 `0x400480`，即 `PLT0` 为目标的；所不同的只是第一条 `jmp` 指令的目标和 `push` 指令中的数据。

`PLT0` 则与之不同，但是包括 `PLT0` 在内的每个表项都占 16 个字节，所以整个 PLT 就像个数组。

另外，需要注意，每个 PLT 表项中的第一条 `jmp` 指令是间接寻址的，比如的 `puts()` 函数是以地址 `0x601018` 处的内容为目标地址进行中跳转的。

### GOT

也就是说，上述执行的是 `jmpq *0x601018`，而 `*0x601018` 就是 `0x00400496`，就是会调转到 `0x400496` 所在的地址执行。

{% highlight text %}
----- 实际是顺序执行，最终会调转到0x400400
(gdb) x/w 0x601018
0x601018 <puts@got.plt>:        0x00400496
{% endhighlight %}

也就是在 `puts@plt` 的代码中，没有直接执行下一条指令，而是通过一次跳转后再继续执行下一条指令，那么，为什么要多此一举？这个问题后面解释，这里接着向下看。

最终会跳转到 `0x400480` 地址处，也就是 `.plt` 的第一个。

### Resolve

看看第一个 `.plt` 中的内容是什么。

{% highlight text %}
(gdb) b *0x400406               设置断点
(gdb) c
Breakpoint 2, 0x0000000000400406 in ?? ()
(gdb) ni
_dl_runtime_resolve () at ../sysdeps/x86_64/dl-trampoline.S:58
58              subq $REGISTER_SAVE_AREA,%rsp
(gdb) i r rip
rip            0x7ffff7df0290   0x7ffff7df0290 <_dl_runtime_resolve>
{% endhighlight %}

如上，在 `jmpq` 中设置一个断点，观察到，实际调转到了 `_dl_runtime_resolve()` 这个函数，这里，实际上就是真正链接器查找函数所在的地址，也就是 `_dl_runtime_resolve` 函数。

### 验证

实际上，上面所谓的多此一举是实现动态加载的关键操作，第一次的时候，是跳转到下一条，如果引用的地址已经被替换成了需要的地址，那么就可以直接跳转了。

在执行完 `callq  0x400490 <puts@plt>` 指令之后，那么 `0x601018` 地址中保存的应该是最新的 `puts` 函数地址了。

{% highlight text %}
(gdb) break *0x400594
Breakpoint 3 at 0x400594: file main.c, line 6.
(gdb) continue
Continuing.
Hello World.

Breakpoint 3, main () at main.c:6
6               return 0;
(gdb) x/w 0x601018
0x601018 <puts@got.plt>:        0xf7a84010
{% endhighlight %}

也就是 `0xf7a84010` 地址就是真实的 `puts` 函数地址。

<!--
![elf load]({{ site.url }}/images/linux/elf-load-process.png "elf load"){: .pull-center }

上图中的红线是解析过程，蓝线则是后面的调用流程。
-->

## 地址解析

在 gdb 中，可以通过 `disassemble _dl_runtime_resolve` 查看该函数的反汇编，感兴趣的话可以看看其调用流程，这里简单介绍其功能。

从调用 `puts@plt` 到 `_dl_runtime_resolve` ，总共有两次压栈操作，一次是 `pushq  $0x0`，另外一次是 `pushq  0x200c02(%rip) # 601008`，分别表示了 `puts` 函数在 `GOT` 中的偏移以及 `GOT` 的起始地址。

在 `_dl_runtime_resolve()` 函数中，会解析到 `puts()` 函数的绝对地址，并保存到 `GOT` 相应的地址处，这样后续调用时则会直接调用 `puts()` 函数，而不用再次解析。

## 安全风险

如上，在 `.got.plt` 中保存的是实际已经查找到函数的地址，那么只需要修改这个段，就可以完成程序执行的跳转，也就是常见的攻击手段。

> 注意，一般主持 NX 的系统中，不会同时设置 Write 和 eXecute 两个权限，也就是说，我们无法覆盖执行的节。

<!--
https://cs155.stanford.edu/papers/formatstring-1.2.pdf
-->

### 防范措施

也就是所谓的 `Relocations Read-only` ，一般简称为 `RELRO` ，包括了两种。


* `Partial RELRO` 编译时使用 `-Wl,-z,relro` 参数，

<!--
    Maps the .got section as read-only (but not .got.plt)
    Rearranges sections to reduce the likelihood of global variables overflowing into control structures.

Full RELRO (enabled with -Wl,-z,relro,-z,now):

    Does the steps of Partial RELRO, plus:
    Causes the linker to resolve all symbols at link time (before starting execution) and then remove write permissions from .got.
    .got.plt is merged into .got with full RELRO, so you won’t see this section name.

Only full RELRO protects against overwriting function pointers in .got.plt. It works by causing the linker to immediately look up every symbol in the PLT and update the addresses, then mprotect the page to no longer be writable.

dl-resolve浅析
https://xz.aliyun.com/t/6364
-->

## 参考

关于动态库的加载过程，可以参考 [动态符号链接的细节](https://github.com/tinyclub/open-c-book/blob/master/zh/chapters/02-chapter4.markdown)。

<!--

https://www.jianshu.com/p/57f6474fe4c6
https://www.iteye.com/blog/jzhihui-1447570
http://blog.chinaunix.net/uid/21471835/cid--1-list-3.html
https://blog.csdn.net/conansonic/article/details/54634142
https://delcoding.github.io/2018/12/dl_runtime_resolve/
 Linux Pwn学习之ret2_dl_resolve
https://www.por7er.com/linux-ret2-dl-resolve.html


ELF文件的加载和动态链接过程
http://jzhihui.iteye.com/blog/1447570

linux是如何加载ELF格式的文件的，包括patchelf
http://cn.windyland.me/2014/10/24/how-to-load-a-elf-file/


readelf -h /usr/bin/uptime
可以找到 Entry point address ，也即程序的入口地址。

http://michalmalik.github.io/elf-dynamic-segment-struggles
https://greek0.net/elf.html
https://lwn.net/Articles/631631/
https://www.haiku-os.org/blog/lucian/2010-07-08_anatomy_elf/


很多不错的介绍
https://delcoding.github.io/


GOT and PLT for pwning.
https://systemoverlord.com/2017/03/19/got-and-plt-for-pwning.html


https://www.cnblogs.com/pannengzhi/p/2018-04-09-about-got-plt.html


https://github.com/tinyclub/open-c-book
https://tinylab.gitbooks.io/cbook/zh/chapters/02-chapter4.html
 -->

{% highlight text %}
{% endhighlight %}
