---
title: C 宏使用技巧
layout: post
comments: true
language: chinese
usemath: true
category: [program]
keywords: align
description:
---


<!-- more -->

## 拼接宏

其中 `#` 把宏的参数直接替换为字符串，不进行替换；而 `##` 会把两个宏参数贴在一起。注意，当宏参数是另一个宏时，只要宏定义用到了 `#` 或 `##` 的地方，其宏参数都不会再展开。

{% highlight c %}
#include <stdio.h>

#define INT_MAX      0x7FFFFFFF
#define STRING(s)    #s

#define A            2
#define CONCAT(a, b) (int)(a##e##b)

int main(void)
{
        int AeA = 55;

        puts(STRING(INT_MAX));        // INT_MAX
        printf("%d\n", CONCAT(A, A)); // 55

        return 0;
}
{% endhighlight %}

如果将 `CONCAT` 宏定义修改为 `(int)((a##e##b) + a)` ，那么结果为 `57` 。

其中 `A` 和 `INT_MAX` 都是宏，而且做为了 `STRING` 和 `CONCAT` 的参数，但是因为存在 `#` 或者 `##` 符号，所以其中的 `A` 和 `INT_MAX` 都没有展开。

如果本意不是如此，而对宏定义的内容进行处理，那么可以在中间再加一层宏定义，如下。

{% highlight c %}
#include <stdio.h>

#define INT_MAX       0x7FFFFFFF
#define _STRING(s)    #s
#define STRING(s)     _STRING(s)

#define A             2
#define _CONCAT(a, b) (int)(a##e##b)
#define CONCAT(a, b)  _CONCAT(a, b)

int main(void)
{
        int AeA = 55;

        // STRING(INT_MAX) -> _STRING(0x7FFFFFFF)
        puts(STRING(INT_MAX));        // 0x7FFFFFFF

        // CONCAT(A, A) -> CONCAT((2), (2)) -> int((2)e(2))
        printf("%d\n", CONCAT(A, A)); // 200

        return 0;
}
{% endhighlight %}

## do-while

如果通过 `define` 定义一个含有多个语句的宏，通常我们使用 `do{...} while(0);` 进行定义，具体原因，如下详细介绍。

如果想在宏中包含多个语句，可能会如下这样写。

{% highlight c %}
#define do_something() \
	do_a();        \
	do_b();
{% endhighlight %}

通常，这样就可以用 `do_somethin()` 来执行一系列操作，但这样会有个问题：如果通过如下的方式用这个宏，将会出错。

{% highlight c %}
if (...)
	do_something();

// 宏被展开后
if (...)
	do_a();
	do_b();
{% endhighlight %}

原代码的目的是想在 if 为真的时候执行 `do_a()` 和 `do_b()`, 但现在，实际上只有 `do_a()` 在条件语句中，而 `do_b()` 任何时候都会执行。

当然这时可以通过如下的方式将那个宏改进一下。

{% highlight c %}
#define do_something() { \
	do_a();          \
	do_b();          \
}
{% endhighlight %}

然而，即使是这样，仍然有错。假设有一个宏是这个样子的，

{% highlight c %}
#define do_something() { \
	if (a)           \
		do_a();  \
	else             \
		do_b();  \
{% endhighlight %}

在使用如下情况时，仍会出错。

{% highlight c %}
if (...)
	do_something();
else {
	...
}

// 宏展开后
if (...)
{
	if (a)
		do_a();
	else
		do_b();
}; else {
	...
}
{% endhighlight %}

此时第二个 else 前边会有一个分号，那么编译时就会出错。

因此对于含有多条语句的宏我们使用 `do{...} while(0);` ，do-while 语句是需要分号来结束的，另外，现代编译器的优化模块能够足够聪明地注意到这个循环只会执行一次而将其优化掉。

综上所述，`do{...} while(0);` 这个技术就是为了类似的宏可以在任何时候使用。

## 其它

### 宏定义顺序

当使用一个静态函数时，当 `A()` 调用 `B()` 时，需要保证 `B()` 先定义 (声明也可以) ，但是对于宏定义，只需要在使用时能够查找到即可，也就是使用与宏定义的顺序无关。

{% highlight c %}
// outer.h
#define OUTER(x) INNER(x)
#include "inner.h"

// inner.h
#define INNER(x) puts(x)

// main.c
#include <stdio.h>
#include "outer.h"

int main(void)
{
        OUTER("Hi");
        return 0;
}
{% endhighlight %}

虽然宏 `OUTER()` 定义在 `INNER()` 之前，但是在使用时，仍然可以正常展开。

利用这个特性，在多平台上，可以将 `outer.h` 文件作为一个平台相关的文件，然后 `inner.h` 作为默认的操作。

#### 内核实例

在内核的 `barrier.h` 实现中，其中与 x86 相关的定义在 `arch/x86/include/asm/barrier.h` 头文件中定义，例如 `dma_rmb()` 宏实际上使用的是 `asm-generic/barrier.h` 中的定义，也就是 `include/linux/compiler.h` 默认的定义。

#### undef

另外，可以通过 `#undef` 取消宏的定义，实际上也就是定义宏的作用区间。

{% highlight c %}
#include <stdio.h>

#define DUMP()  puts(MSG)

int main(void)
{
#define MSG "message #1"
        DUMP();
#undef MSG

#define MSG "message #2"
        DUMP();
#undef MSG

        return 0;
}
{% endhighlight %}

在取消之后，会输出不同的信息。


{% highlight text %}
{% endhighlight %}
