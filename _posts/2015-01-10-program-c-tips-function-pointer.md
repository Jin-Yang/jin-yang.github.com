---
title: C 函数指针
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: c,function,pointer
description: 通过函数指针，允许程序在不同的状态时，调用不同的函数，而又不需要改变当前的处理逻辑。 那么函数指针如何声明、使用？
---

通过函数指针，允许程序在不同的状态时，调用不同的函数，而又不需要改变当前的处理逻辑。

那么函数指针如何声明、使用？

<!-- more -->

## 简介

其中函数的调用方式如下。

{% highlight c %}
#include <stdio.h>

void foobar(int);

int main(void)
{
        foobar(100);
        return 0;
}

void foobar(int v)
{
        printf("got %d\n", v);
}
{% endhighlight %}

如上，是一个简单的函数声明及其实现，其中 `foobar()` 是一个函数，代表了一段代码的实现。那么函数名的作用是什么？

一个数据变量的内存地址可以存储在相应的指针变量中，函数的首地址也以存储在某个函数指针变量中，然后就可以通过这个函数指针变量来调用所指向的函数了。

在 C 语言中，任何一个变量需要先声明，然后才能使用，函数指针变量同样也应该要先声明。

{% highlight c %}
//----- 函数指针变量声明，函数实现的参数、返回值需要相同
void (*foobar)(int);
{% endhighlight %}

与函数声明基本类似，只是将 `foobar` 替换成了 `(*foobar)` 而已，这样它就成为了一个指针。

## 使用方法

函数指针有如下的几种使用方法。

{% highlight c %}
#include <stdio.h>

void foobar(int);

int main(void)
{
        void (*foo)(int);

        foo = foobar;
        foo(1);
        (*foo)(2);

        foo = &foobar;
        foo(1);
        (*foo)(2);

        (*foobar)(3);

        return 0;
}

void foobar(int v)
{
        printf("got %d\n", v);
}
{% endhighlight %}

总结如下。

* `foobar` 函数名与 `foo` 函数指针一样，都是函数指针；只是前者为常量，后者为变量。
* 函数调用可以以 `(*foobar)(10)` 方式使用，但不方便，所以允许以 `foobar(1)` 的方式调用；这也就意味着函数指针的操作相同。
* 函数变量赋值时，可以写成 `foo=foobar` 的形式，也可以写成 `foo=&foobar` 的形式。
* 在声明时 `void foobar(int)` 和 `void (*foobar)(int)` 不能互换，因为这是两种不同的方式。
* 函数指针变量也可以存入一个数组内，数组的声明方法为 `int (*foobars[10])(int)` 。


<!--
函数指针
https://www.cnblogs.com/windlaughing/archive/2013/04/10/3012012.html
-->



{% highlight text %}
{% endhighlight %}
