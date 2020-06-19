---
title: 程序简介
layout: post
comments: true
language: chinese
category: [program,misc]
keywords: elf
description:
---

可执行二进制文件在编译以及加载过程中都有统一的格式，不同的平台其格式略有不同，这还只是在磁盘上的保存方式，而加载到内存中又不相同。

这里简单介绍。

<!-- more -->

## 目标文件

所谓的目标文件，严格来说是编译器编译完后，还没有进行链接的文件，但通常采用和可执行文件相同的存储格式，也包括了动态链接库 `(Dynamic Linking Library)` 和静态链接库 `(Statci Linking Library)` 。

通常来说，在 Windows 平台下会采用 `Portable Executable, PE` 格式，而在 Linux 平台下则会采用 `Executable Linkale Format, ELF`，它们都是 `Common File Format, COFF` 的变种。

对于文件的类型可以通过 `file` 命令进行查看，常见的类型包括了：

* 可重定位文件(Relocatable File)<br>主要包含了代码和数据，主要用来链接成可执行文件或共享目标文件，如 `.o` 文件。
* 可执行文件(Executable File)<br>主要是可以直接执行的程序，如 `/bin/bash` 。
* 共享目标文件(Shared Object File)<br>包含了代码和数据，常见的有动态和静态链接库，如 `/lib64/libc-2.17.so` 。
* 核心转储文件(Core Dump File)<br>进程意外终止时，系统将该进程的地址空间的内容及终止时的一些其他信息转储到该文件中。

如下介绍时使用 ELF 格式。

### 示例

假设有如下的程序。

{% highlight c %}
#include <stdio.h>

int main(void)
{
	puts("Hello World.");
	return 0;
}
{% endhighlight %}

首先是可重定向文件或者目标文件。

{% highlight text %}
$ gcc -c main.c
$ file main.o
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
{% endhighlight %}

然后，编译成二进制文件，如下的部分内容省略。

{% highlight text %}
$ gcc -o main main.c
$ file main
main: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, ... ...
{% endhighlight %}

也可以生成动态库，一般不会将 `main()` 函数放到动态库中使用。

{% highlight text %}
$ gcc -c -fPIC main.c
$ gcc -fpic -shared -Wl,-soname,libtest.so.0 -o libtest.so.0.0 main.o
$ file libtest.so.0.0
libtest.so.0.0: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, ... ...
{% endhighlight %}

关于 ELF 的文件格式，详细可以参考 [ELF 详解](/post/program-c-elf-details.html) 中的介绍。

### 查看符号

所谓的符号 (Symbols) 其实包含了函数以及变量的名称。

有时候可能需要查看一个库中到底有哪些函数，`nm` 命令可以打印出库中的涉及到的所有符号，库既可以是静态的也可以是动态的。

`nm` 列出的符号有很多，常见的有三种：

* 在库中被调用，但并没有在库中定义(表明需要其他库支持)，用U表示；
* 库中定义的函数，用T表示，这是最常见的；
* 所谓的“弱态”符号，它们虽然在库中被定义，但是可能被其他库中的同名符号覆盖，用W表示。

例如，希望知道上文提到的 `hello` 库中是否定义了 `printf()` 。

{% highlight text %}
$ nm libhello.so
{% endhighlight %}

发现其中没有 `printf()` 的定义，取而代之的是 `puts()` 函数，而且为 `U` ，表示符号 `puts` 被引用，但是并没有在函数内定义，由此可以推断，要正常使用 `hello` 库，必须有其它库支持，再使用 `ldd` 命令查看 `hello` 依赖于哪些库：

{% highlight text %}
$ ldd -v hello
$ readelf -d hello     直接使用readelf
{% endhighlight %}

每行 `=>` 前面的，为动态链接程序所需的动态链接库的名字；而 `=>` 后面的，则是运行时系统实际调用的动态链接库的名字。所需的动态链接库在系统中不存在时，`=>` 后面将显示 `"not found"`，括号所括的数字为虚拟的执行地址。

## 段 Segment

目标文件通过段进行存储，在 Windows 中可以通过 `Process Explorer` 查看相关信息，Linux 可以通过 `objdump` 查看，如下以Linux 为例。

在 Linux 中大致会有如下的 5 种数据区：

* BSS 用来存放程序中未初始化的全局/静态变量的一块内存区域，属于静态内存分配。
* DATA 保存已经初始化的全局变量，属于静态内存分配。
* TEXT 用来存放真正执行的代码，大小在编译后已经确定，一般是只读，也包含了一些只读常量，例如常量字符串。
* HEAP 保存堆的内容，一般是通过 `malloc()` 动态分配的内存。
* STACK 栈空间，包括了函数的调用栈，以及存放程序临时创建的局部变量。

其中 BSS 是 Block Started by Symbol 的简称，由操作系统初始化清零，而 DATA 则是由程序初始化，从而造成了上述的差别。

{% highlight text %}
addr       contents                   comments

High  +-----------------+  ---
 |    |    arguments    |   |   command-line arguments and
 |    |   environments  |   |   environments variables
 |    +-----------------+  ---
 |    |      stack      |   |
 |    +-----------------+   |
 |    |      v v v      |   |
 |    ~                 ~   |
 |    |      ^ ^ ^      |   |
 |    +-----------------+   |
 |    |      heap       |   |
 |    +-----------------+  ---
 |    |      bss        |   |   initialized to zero by exec
 |    +-----------------+  ---
 |    |      data       |   |
 |    +-----------------+   |   initialized from program file by exec
 V    |      text       |   |
Low   +-----------------+  ---
{% endhighlight %}

如上只是基本分类，一般是在运行时的状态，在存储时，一般称为 Section ，而且在存储时，还会存在其它的段，如 `.rodata` `.comment` 等。

可以通过如下程序查看进程在运行时各个段的地址。

{% highlight c %}
/* main.c */
#include <stdio.h>
#include <stdlib.h>

static char bss[1024];     // 未初始化全局变量，在BSS段
//int data = 0;            // 初始化为0全局变量，在BSS段
//static int data = 0;     // 初始化为0静态全局变量，在BSS段
//int data = 1234;         // 已初始化全局变量，在DATA段
static int data = 1234;    // 已初始化静态全局变量，在DATA段
static const char *text = "foobar";

static void foobar(void)
{
        printf("Hi foobar\n");
}

int main(void)
{
        //static int bss = 0;          // 已初始化为0的静态局部变量，在BSS段
        //static int bss;              // 未初始化的静态局部变量，在BSS段
        //static int data = 123;       // 已初始化静态局部变量，在DATA段
        //char *ptr = "Hello World!";  // 局部变量，ptr存在在栈中，而指向的字符串则是在字符常量区
        int stack = 0;                 // 局部变量，存在在栈中
        char *heap = (char *)malloc(1000);

        printf("Address of various segments:\n");
        printf("     Text Segment: %p\n", foobar);
        printf("       RO Segment: %p\n", text);
        printf("     Data Segment: %p\n", &data);
        printf("              BSS: %p\n", bss);
        printf("    Stack Segment: %p\n", &stack);
        printf("     Heap Segment: %p\n", heap);

        return 0;
}
{% endhighlight %}

注意，目前的程序一般都会有多个这样的段，所以基本上无法确认每个段的边界。

另外一个比较有意思的场景是，通过 `int array[1024 * 1024] = {0}` 编译的二进制文件会较小，而 `int array[1024 * 1024] = {0x55}`  至少包含 4M 的初始化数据，即使除了第一之外都是 0 。

### 示例程序

如上示例通过 `gcc -c main.c` 编译生成目标文件 `main.o`，然后通过 `objdump -h` 查看头部信息，也可以通过 `-x` 参数查看更详细的信息。

{% highlight text %}
$ objdump -h main.o

main.o:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         000000c0  0000000000000000  0000000000000000  00000040  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000010  0000000000000000  0000000000000000  00000100  2**3
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
  2 .bss          00000400  0000000000000000  0000000000000000  00000120  2**5
                  ALLOC
  3 .rodata       000000b8  0000000000000000  0000000000000000  00000120  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .comment      0000002d  0000000000000000  0000000000000000  000001d8  2**0
                  CONTENTS, READONLY
  5 .note.GNU-stack 00000000  0000000000000000  0000000000000000  00000205  2**0
                  CONTENTS, READONLY
  6 .eh_frame     00000058  0000000000000000  0000000000000000  00000208  2**3
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
{% endhighlight %}

数据段 `.data` 用来保存已经初始化了的全局变量和局部静态变量，如上述的 `data` 。

只读数据段 `.rodata` 主要用于保存常量，如 `printf()` 中的字符串和 `const` 类型的变量，该段在加载时也会将其设置为只读。

`BSS` 段保存了未初始化的全局变量和局部静态变量，如上述 `bss` 。

`.text` 为代码段，`.data` 保存含初始值的变量，`.bss` 只保存了变量的符号。


<!--
比较重要的是 `File off` 和 `Size` 信息，一般头部信息的大小为 `0x34` ，因此第一个段的地址就会从 `0x34` 开始 (地址从 0 开始计数)，另外，由于需要 4bytes 对齐，因此会从 `54(0x36)` 开始。也可以通过 size 查看，采用的是十进制，最后会用十进制和十六进制表示总的大小。

正常应该是 8 字节，但是查看时只有 4 字节，通过符号表(Symbol Table)可以看到，只有 static_var2 保存在 .bss 段，而 global_uninit_var 未存放在任何段，只是一个未定义的 COMMON 符号。这与不同的语言和编译器实现有关，有些编译器会将全局的为初始化变量存放在目标文件 .bss 段，有些则不存放，只是预留一个未定义的全局变量符号，等到最终链接成可执行文件时再在 .bss 段分配空间。
-->

### 添加一个段

将以个二进制文件，如图片、MP3 音乐等作为目标文件的一个段。如下所示，此时可以直接声明 `_binary_example_png_start` 和 `_binary_example_png_end` 并使用。

{% highlight text %}
$ objcopy -I binary -O elf32-i386 -B i386 example.png image.o
$ objdump -ht image.o

image.o:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .data         000293d6  00000000  00000000  00000034  2**0
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
00000000 l    d  .data	00000000 .data
00000000 g       .data	00000000 _binary_example_png_start
000293d6 g       .data	00000000 _binary_example_png_end
000293d6 g       *ABS*	00000000 _binary_example_png_size
{% endhighlight %}

如果在编译时想将某个函数或者变量放置在一个段里，可以通过如下的方式进行。

{% highlight c %}
__attribute__((section("FOO"))) int global = 42;
__attribute__((section("BAR"))) void foo() { }
{% endhighlight %}

## 其它

容易出错的几个知识点。

### 栈变量地址

关键是当函数调用返回时，对应的内存地址是否被销毁，其中静态内存的生命周期与程序运行声明周期相同。

{% highlight c %}
#include <stdio.h>

char *get_string(void)
{
	//char str[] = "Hello World!";      // 栈地址，完成函数调用会释放，结果无法预测
	//char *str = "Hello World!";       // 指向字符串常量(静态内存)，函数返回仍可访问
	static char str[] = "Hello World!"; // 静态内存，函数退出通用可以访问
	return str;
}

int main(void)
{
	return puts(get_string());
}
{% endhighlight %}

还有一种是判断地址是否相同，使用 `char []` 时，定义的是一个保存在栈中的局部变量，地址不同；而使用 `char *` 指向的实际是一个常量地址，所以相等。

{% highlight text %}
#include <stdio.h>

int main(void)
{
	//char str1[] = "Hi!"; // 两个不同的栈地址
	//char str2[] = "Hi!";

	char *str1 = "Hi!";
	char *str2 = "Hi!";

	return str1 == str2;
}
{% endhighlight %}



{% highlight text %}
{% endhighlight %}
