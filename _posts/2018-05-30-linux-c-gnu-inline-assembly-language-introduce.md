---
title: Linux GNU 内联汇编
layout: post
comments: true
language: chinese
category: [misc]
keywords: linux,inline,assembly
description: 在通过 C 做上层的开发时，实际上很少会用到汇编语言，不过对于 Linux 内核开发来说，经常会遇到与体系架构相关的功能或优化代码，此时可能需要通过将汇编语言指令插入到 C 语句的中间来执行这些任务。这里简单介绍下 GNU 中与 Linux 内联汇编相关的用法。
---

在通过 C 做上层的开发时，实际上很少会用到汇编语言，不过对于 Linux 内核开发来说，经常会遇到与体系架构相关的功能或优化代码，此时可能需要通过将汇编语言指令插入到 C 语句的中间来执行这些任务。

这里简单介绍下 GNU 中与 Linux 内联汇编相关的用法。

<!-- more -->

![gnu logo]({{ site.url }}/images/gnu-logo.png "gnu logo"){: .pull-center width="50%" }

## 示例

如下是 GNU 内联汇编的基本语法。

{% highlight text %}
asm asm-qualifiers (assembler template
    : output operands               (optional)
    : input operands                (optional)
    : list of clobbered registers   (optional)
);
{% endhighlight %}

下面的代码是一个简单的赋值操作，采用的是 `AT&T` 的语法，而非常见的 `Intel` 语法。

{% highlight c %}
#include <stdio.h>

int main(void)
{
	int foo = 0; bar = 10;

	__asm__ __volatile__ (
		"movl %1, %%eax;"
		"movl %%eax, %0;"
		:"=r"(foo)  /* %0 output */
		:"r"(bar)   /* %1 input */
		:"%eax");   /* clobbered register */

        printf("foo = bar; foo = %d\n", foo);

        return 0;
}
{% endhighlight %}

直接通过 `gcc foobar.c -o foobar` 进行编译，然后运行即可，简单解析如下：

* `foo` 是输出操作数，内联汇编中通过 `%0` 引用；`bar` 是输入操作数，在内联中通过 `%1` 引用。
* `r` 是操作数的约束，指明这两个变量存放到通用寄存器中，详见如下关于约束的介绍。
* 最后的修饰寄存器 `%eax` 用于告知编译器，在内联汇编中会修改该寄存器的值，这样 GCC 就不会使用该寄存器存储任何其它的值。

那么上述的内联汇编中，通过 `movl %1, %%eax` 将 `bar` 的值移到 `%eax` 中，`movl %%eax, %0` 再将 `%eax` 的内容移到 `foo` 中。

而 `foo` 被指定为输出操作数，那么当上述的内联汇编执行完之后，在 C 中对应的 `foo` 变量同时会更新，也就是说在内对 `foo` 的修改会直接在外面体现出来。

注意，内联汇编中使用 `%0` `%1` 来标示变量，任何带有一个 `%` 的都被看成是输入、输出操作数，而非寄存器，如果要使用寄存器需要两个 `%` 也就是类似 `%%eax` 这种方式。

## 规范

参照上述的语法，简单列举一下使用方法。

### 关键字

主要是 `asm` 和 `volatile` ，前者显然是用来标示这是一段汇编代码，后者表示不需要 gcc 对下面的汇编代码做任何优化。

为避免 `asm` 的命名冲突，gcc 还支持 `__asm__` 关键字，两者的作用等价；`volatile` 也同样存在一个 `__volatile__` 关键字。

### 通用规范

其中的每条指令以分界符结尾，分界符可以是换行符 `\n` 和分号 `;` ，为了排版可以使用 `\n\t` 作为分隔。如下是编写内联汇编时一些 `AT&T` 语法的常见规范：

* 寄存器命名。都添加了 `%` 前缀，例如 `%eax` 。
* 由左到右的顺序为 源操作数 和 目的操作数，这与 Intel 的语法有所区别。
* 操作数大小。非强制(编译器会根据操作数推测)，但是可以增加可读性，指令后添加 `b` `w` `l` 标示 `Byte` `Word` `Long` 类型。
* 立即操作数。通过 `$` 符号指定。
* 间接内存引用。通过 `()` 来完成。

简单的示例。

{% highlight text %}
----- 指令后显示指定操作数大小
movb %al, %bl   -- Byte Move 8-bits
movw %ax, %bx   -- Word Move 16-bits
movl %eax, %ebx -- Long Word Move 32-bits
movq %rax, %rbx -- Long Long Word Move 64-bits

----- 寄存器寻址，将eax的数据放到edx中
movl %eax, %edx

----- 操作立即数，将0xffff添加到eax寄存器中
movl $0xffff, %eax

----- 直接寻址，也就是访问的0x123地址处的数据，等价于edx = *(uint32_t *)0x123
movl 0x123, %edx

----- 间接内存引用，将esi所指向内存地址的内容添加到edx寄存器中
movl (%esi), %edx

----- 变址寻址，将ebx所指向内存地址+4的内容添加到edx寄存器中，等价于edx = *(int32_t*)(ebx+4)
movl 4(%ebx), %edx
{% endhighlight %}

### 操作数

在汇编程序模板中，每个操作数用数字引用，如果总共有 n 个操作数 (包括输入和输出操作数)，那么第一个输出操作数编号为 0 ，逐项递增。

## 约束

简单来说，就是告知 gcc 在编译时如何使用一些参数，例如：A) 操作数是否能放到寄存器，应该放到那种寄存器；B) 操作数是否可以为一个内存引用和哪种地址；操作数是否可以为一个立即数和它可能的取值范围（即值的范围），等等。

### 寄存器约束

用于存储到通用寄存器中，当指定 `r` 时，gcc 可以将变量保存在任何可用的 GPR 中，除非使用具体的名称用来指定所需的寄存器类型，其中类型如下：

{% highlight text %}
a    %eax, %ax, %al
b    %ebx, %bx, %bl
c    %ecx, %cx, %cl
d    %edx, %dx, %dl
S    %esi, %si
D    %edi, %di
{% endhighlight %}

<!--
### 内存操作数约束

当操作数位于内存时，任何对它们的操作将直接发生在内存位置，而寄存器约束需要先将值存储在要修改的寄存器中，然后将它写回到内存位置。

使用寄存器可以提高访问速度，如果只需要在 asm 内更新一个 C 变量，而又不想使用寄存器去保存它的值时，使用内存最为有效。

asm("sidt %0\n" : :"m"(loc));
-->

## 示例代码

### 加法操作

{% highlight c %}
#include <stdio.h>

int main(void)
{
        int foo = 10, bar = 15;

        __asm__ __volatile__ (
                "addl %%ebx, %%eax;"
                : "=a"(foo)         /* ouput */
                : "a"(foo), "b"(bar)/* input */
        );

        printf("foo + bar = %d\n", foo);

	return 0;
}
{% endhighlight %}

### 内存屏障

{% highlight text %}
__asm__ __volatile__ ("" ::: "memory");
{% endhighlight %}

用于防止编译器将读写操作乱序执行，这里再重新讨论下其中的参数：

* `__volatile__` 告诉编译器，严禁将此处的汇编语句与其它的语句重组合优化，防止编译器的乱序执行；
* `memory` 强制编译器假设 RAM 所有内存单元均被汇编指令修改，这样寄存器、Cache 中已缓存数据将会重新从内存中加载。

## 其它

### 生成汇编

在编写完内联汇编之后，如果想要查看代码是否正确，可以先编译生成汇编代码查看。

可以直接使用 `-S` 参数生成汇编，默认是 `AT&T` 风格，当然也可以指定为 `Intel` 方式。

{% highlight text %}
$ gcc -S -o target.s source.c
$ gcc -masm=intel -S -o target.s source.c
{% endhighlight %}

## 参考

* [GNU Assembler Examples](http://cs.lmu.edu/~ray/notes/gasexamples/) 通过非常详细的示例介绍如何在 GNU 下编写汇编。
* [GCC Inline Assembly HOWTO](http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html) GNU 中的帮助文档。
* [Using Assembly Language in Linux](http://asm.sourceforge.net/articles/linasm.html) 有介绍系统调用的使用方式，包括入参。

<!--
https://www.linuxprobe.com/gcc-how-to.html

几种基本汇编指令详解
http://blog.luoyuanhang.com/2015/07/07/%E5%87%A0%E7%A7%8D%E5%9F%BA%E6%9C%AC%E6%B1%87%E7%BC%96%E6%8C%87%E4%BB%A4%E8%AF%A6%E8%A7%A3/

关于x86_64汇编的一些详细介绍
https://github.com/0xAX/asm
https://0xax.github.io/asm_1/
-->

{% highlight text %}
{% endhighlight %}
