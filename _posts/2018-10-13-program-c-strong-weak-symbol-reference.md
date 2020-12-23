---
title: GCC 强弱符号、引用
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: weak,strong,symbol,reference
description: 在编程时经常会碰到一种情况叫符号重复定义，一般意味着多个目标文件中含有相同名字全局符号的定义，而有时又不会报错，为什么？ 在 glibc 中定义了很多类似 `read()` `open()` 的函数，但是又可以自己定义相同的函数？ 这就涉及到了强符号、弱符号以及强引用和弱引用了。
---

在编程时经常会碰到一种情况叫符号重复定义，一般意味着多个目标文件中含有相同名字全局符号的定义，而有时又不会报错，为什么？

在 glibc 中定义了很多类似 `read()` `open()` 的函数，但是又可以自己定义相同的函数？

这就涉及到了强符号、弱符号以及强引用和弱引用了。

<!-- more -->

## 强弱符号

在编程时经常会碰到一种情况叫符号重复定义，一般意味着多个目标文件中含有相同名字全局符号 (变量或者函数) 的定义，这种符号的定义被称为强符号 (Strong Symbol)，有些符号的定义被称为弱符号 (Weak Symbol)。

例如，如下的示例，两个 C 代码文件 `main.c` 以及 `other.c` 。

{% highlight c %}
/* main.c */
#include <stdio.h>

int foobar(void)
{
        return 0;
}

int data = 100;
int main(void)
{
        printf("Data is %d\n", data);
        return 0;
}
{% endhighlight %}

{% highlight c %}
/* other.c */
int data = 1;
int foobar(void)
{
        return 0;
}
{% endhighlight %}

{% highlight text %}
$ gcc main.c other.c
/tmp/ccJ2vhHv.o:(.data+0x0): multiple definition of `data'
/tmp/cch1fzSM.o:(.data+0x0): first defined here
/tmp/ccJ2vhHv.o: In function `foobar':
other.c:(.text+0x0): multiple definition of `foobar'
/tmp/cch1fzSM.o:main.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
{% endhighlight %}

上述会发现两个地方出现了重复定义问题，分别是一个全局变量 `int data` 以及全局函数 `int foobar(void)` 。

对于 C/C++ 来说，编译器默认函数和初始化了的全局变量为强符号，未初始化的全局变量为弱符号。所以，如果将 `other.c` 文件中的第一行修改为 `int data` 实际上就不会再报错了，但是对于函数来说没有太好的办法。

### weak

实际上 GCC 提供了一个 `__attribute__((weak))` 来将任何一个强符号定义为弱符号。注意，强符号和弱符号都是针对定义来说的，不是针对符号的引用。

{% highlight c %}
/* main.c */
#include <stdio.h>

extern int ext;
int weak1;
int strong = 1;
int __attribute__((weak)) weak2 = 2;

int main(void)
{
        printf("Data is %d, %d, %d\n", weak1, strong, weak2);
        return 0;
}
{% endhighlight %}

{% highlight c %}
int weak1 = 100;
//int strong = 200; /* multiple definition of `strong' */
int weak2 = 200;
{% endhighlight %}

### 总结

针对强弱符号的概念，链接器就会按照如下规则处理与选择被多次定义的全局符号：

* 不允许强符号被多次定义 (即不同的目标文件中不能有同名的强符号)，如果有多个强符号定义，则链接器报符号重复定义错误。
* 如果一个符号在某个目标文件中是强符号，在其他文件中都是弱符号，那么选择强符号。
* 如果一个符号在所有目标文件中都是弱符号，那么选择其中占用空间最大的一个。

对于最后一点，如果两个文件同时定义了 `global_var` 变量，类型分别为 `int` 和 `double` ，也就是分别占用 4 和 8 字节，那么 `global_var` 符号最终占用的是 8 字节。

注意，尽量不要使用多个不同类型的弱符号，否则容易导致很难发现的程序错误。

## 强弱引用

一些对外部文件的符号引用，如果在被链接成可执行文件时，没有找到相关的符号，那么链接器就会报符号未定义错误，这种被称为强应用 (Strong Reference)。

与之相对应还有一种弱引用 (Weak Reference)，如果可以找到符号则连接，否则连接器会默认设置为 0 或者一个特殊值。

### weakref

在 GCC 中，可以使用 `__attribute__((weakref))` 来声明对一个外部函数的引用为弱引用。

{% highlight c %}
/* main.c */
#include <stdio.h>

static void foo() __attribute__((weakref("bar")));

int main(void)
{
        if (foo)
                foo();
        return 0;
}
{% endhighlight %}

{% highlight c %}
/* other.c */
#include <stdio.h>

void bar()
{
        printf("Hello World!!!\n");
}
{% endhighlight %}

注意，在声明弱引用时需要使用 `static` 关键字。

如果通过 `gcc main.c` 进行编译则不会打印任何东西，而通过 `gcc main.c other.c` 编译会看到相关的 `Hello World!!!` 输出。

## weak_alias

在 glibc 代码中，可以看到很多对函数的属性修饰，包括了各种 alias 的定义，这里简单看下 weak_alias 的相关内容。

函数的属性定义在 `include/libc-symbols.h` 头文件中，其中 `weak_alias` 的定义如下。

{% highlight c %}
/* Define ALIASNAME as a weak alias for NAME.
   If weak aliases are not available, this defines a strong alias.  */
# define weak_alias(name, aliasname) _weak_alias (name, aliasname)
# define _weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));
{% endhighlight %}

可以参考 glibc 中的一个实际应用，也就是 `gettimeofday()` 。

{% highlight c %}
# include <sysdep.h>
# include <errno.h>

int __gettimeofday (struct timeval *tv, struct timezone *tz)
{
	return INLINE_SYSCALL (gettimeofday, 2, tv, tz);
}
libc_hidden_def (__gettimeofday)

weak_alias (__gettimeofday, gettimeofday)
libc_hidden_weak (gettimeofday)
{% endhighlight %}

### 示例

{% highlight c %}
#include <stdio.h>

void foobar() __attribute__ ((weak));
void foobar(void)
{
        printf("libfoobar test\n");
}
{% endhighlight %}

{% highlight c %}
#include <stdio.h>

void foobar(void)
{
        printf("app test\n");
}
{% endhighlight %}

{% highlight c %}
#include <stdio.h>

void foobar(void);

int main(void)
{
        foobar();
        return 0;
}
{% endhighlight %}

{% highlight text %}
all:
        gcc -c foobar.c
        ar crv libfoobar.a foobar.o
        #gcc main.c app.c libfoobar.a -o foobar
        gcc main.c libfoobar.a -o foobar
{% endhighlight %}

首先生成一个静态库，可以通过 `nm libfoobar.a` 看到定义了一个 `Weak` 类型的 `foobar()` 函数。

{% highlight text %}
----- 直接使用app.c中的代码
$ gcc libfoobar.a main.c app.c -o foobar
$ ./foobar
app test

----- 如果不使用app.c中的代码
$ gcc libfoobar.a main.c -o foobar
$ ./foobar
libfoobar test
{% endhighlight %}

在使用时需要注意顺序，如果 `libfoobar.a` 中包含了 `weak` 函数，那么应该放在最后，否则会报错 `undefined reference` 。

<!--
http://www.lenky.info/archives/2013/01/2195
-->


## 总结

这种弱符号和弱引用对于库来说十分有用，比如库中定义的弱符号可以被用户定义的强符号所覆盖，从而使得程序可以使用自定义版本的库函数。

或者程序可以对某些扩展功能模块的引用定义为弱引用，当用户将扩展模块与程序链接在一起时，功能模块就可以正常使用；如果我们去掉了某些功能模块，那么程序也可以正常链接，只是缺少了相应的功能，这使得程序的功能更加容易裁剪和组合。

{% highlight text %}
{% endhighlight %}
