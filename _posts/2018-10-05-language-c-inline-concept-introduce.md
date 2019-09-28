---
title: C 语言 inline 简介
layout: post
comments: true
language: chinese
category: [program]
keywords: c, inline
description: 在 C 语言中，调用函数时会在栈空间生成一个函数调用栈，如果函数嵌套过深则会导致大量的栈空间消耗，甚至是溢出。
---

在 C 语言中，调用函数时会在栈空间生成一个函数调用栈，如果函数嵌套过深则会导致大量的栈空间消耗，甚至是溢出。

<!-- more -->

## 简介

可以通过 `inline` 修饰符来解决这一问题，将函数标示为内联函数，有些类似于宏。

{% highlight c %}
#include <stdio.h>

inline const char *num_check(int v)
{
        return (v % 2 > 0) ? "odd" : "even";
}

int main(void)
{
        int i;

        for (i = 0; i < 100; i++)
                printf("%02d   %s\n", i, num_check(i));

        return 0;
}
{% endhighlight %}

上面的示例就是标准的内联函数，当运行调用 `for` 循环时，其中的 `num_check()` 函数会被替换为 `(i % 2) ? "odd" : "even"` 直接使用，而非通过函数栈进行调用。

简单来说，`inline` 的函数一般都很小，适合在调用处被替换。

### 使用方法

在 gcc 的文档中，关键字 `inline` 必须与函数定义放在一起才能使函数成为内联，仅将 `inline` 放在函数声明前不起任何作用。

例如，如下方式的 `foobar` 函数不能成为内联函数。

{% highlight c %}
inline void foobar(int x, int y);
void foobar(int x, int y)
{
	// ... ...
}
{% endhighlight %}

而如下风格的函数 `foobar()` 则会成为内联函数。

{% highlight c %}
void foobar(int x, int y);
inline void foobar(int x, int y)
{
	// ... ...
}
{% endhighlight %}

虽然内联函数能提高执行效率，其代价是更多的代码量，编译生成的二进制相比来说会更大。

## 注意事项

`inline` 关键字仅仅是 **建议** 编译器做内联展开处理，而不是强制，在 gcc 编译器中，如果编译优化设置为 `-O0`，即使是 `inline` 函数也不会被内联展开。

除非设置了强制内联 `__attribute__((always_inline))` 属性，例如：

{% highlight c %}
void func_test() __attribute__((always_inline));
{% endhighlight %}

相比宏来说，内联函数有如下的优点：

* 因为是函数，存在入参的检查。
* 即使多次调用，也不会存在风险。
* 有自己的变量空间，可以返回一个值。


{% highlight text %}
{% endhighlight %}
