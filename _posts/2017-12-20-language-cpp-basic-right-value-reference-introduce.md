---
title: C++ 右值引用
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

在 C++11 的新特性中增加了所谓的右值引用的支持，其主要目的是为了解决两个问题：A) 临时对象非必要的拷贝操作；B) 在模板函数中如何按照参数的实际类型进行转发。

同时，和右值引用相关的概念比较多，包括了纯右值、将亡值、Universal References、引用折叠、移动语义、Move语义和完美转发等等，这里简单介绍。

<!-- more -->

## 简介

引用本质上是一个隐式指针，是某个对象的一个别名，在 C++ 中通过操作符 `&` 实现，实际上是左值引用，C++11 又提出了右值引用。

### 左值 VS. 右值

当执行 `int i = foobar();` 语句时，通过函数获取一个整形值，但实际上返回了两种类型：A) 左值 `i` 会一直存在；B) 临时值，在表达式结束时被销毁，这个也就是右值。最简单的区分方法就是能否取地址，能就是左值，否则为右值，可以执行 `&i` 但是无法执行 `&foobar()` 。

另外一个示例是 `int a = b + c;` ，其中 `a` 就是左值，可以通过 `&a` 获取其地址，而表达 `b + c` 是右值，在赋值给某个具体的变量前，不能直接通过变量名获取，例如 `&(b + c)` 会直接报错。

在 C++11 中，所有的值必属于左值、将亡值、纯右值三类，纯右值包括了：非引用返回临时变量、运算表达式产生的临时变量、原始常量值、lambda 表达式等。

### 左值引用

在 C++ 中声明了一个引用变量之后必须初始化，否则编译就会报错，改引用与复制的变量相同，包括了地址以及类型，所以可以理解为别名。与指针的区别是，指针是一种数据类型，而引用则不是。

当作为函数参数时，传递的就是变量的左值，也就是地址。

{% highlight cpp %}
#include <iostream>

void update(int &v)
{
        v = 12;
}
int main(void)
{
        int A = 3;
        int &Aref = A;

        std::cout << "A addr " << &A << ", A-Ref addr " << &Aref << std::endl;
        update(A);
        std::cout << "A " << A << std::endl;

        return 0;
}
{% endhighlight %}

### 右值引用

上述的左值引用是对左值进行绑定，而对右值进行绑定的引用就是右值引用，其语法为 `&&` ，如下绑定了一个右值 `0` 。

{% highlight text %}
int &&i = 0;
{% endhighlight %}

再看下获取某个函数的返回值。

{% highlight text %}
int i = foobar();
int &&i = foobar();
{% endhighlight %}

虽然两者的写法基本类似，但语义却相差很大，第一行 `foobar()` 产生的临时值在表达式结束时就已经被销毁了，而第二行 `foobar()` 返回的值则会随着 `i` 变量的生命周期结束，而非立即结束。

注意，在最后一行中，变量 `i` 是右值引用类型 `int &&`，但如果从左右值角度看，实际上是个左值，可以取地址，是一个已经赋值的右值。

## 绑定规则

使用时，左值引用只能绑定左值，右值引用只能绑定右值，如果绑定的不对，那么在编译期间就会直接报错。

注意，常量左值引用比较特殊，可以作为一个 "万能" 引用类型，可以绑定非常量左值、常量左值、右值，而且在绑定右值的时候，常量左值引用还可以像右值引用一样将右值的生命期延长，缺点是，只能读不能改。

常量左值经常会在不经意的场景下使用，例如复制构造函数中。

{% highlight cpp %}
#include <iostream>

class Copyable {
public:
        Copyable(){}
        Copyable(const Copyable &c) {
                std::cout << "Copyied" << std::endl;
        }
};

Copyable CreateRValue()
{
        return Copyable(); //
}
void AcceptValue(Copyable c) { }
void AcceptReference(const Copyable &c) { }

int main(void)
{
        std::cout << "Passed by value:" << std::endl;
        AcceptValue(CreateRValue());

        std::cout << "Passed by reference:" << std::endl;
        AcceptReference(CreateRValue());

        return 0;
}
{% endhighlight %}

正常 `AcceptValue()` 会有调用两次复制构造函数，而 `AcceptReference()` 会调用一次，实际上任何一个函数都没有调用复制构造函数。

这是因为，编译器默认会开启 RVO/NRVO 优化，例如，当编译器发现 `CreateRValue()` 内部生成了一个对象，返回时要生成一个临时对象，而该对象又赋值给了一个形参，那么这三个变量完全可以用一个变量替代，这样就不需要调用拷贝构造函数了。

测试时可以通过 `-fno-elide-constructors` 参数取消优化。

### 总结

如下的 T 是一个具体类型。

* 左值引用，通过 `T&` 定义，只能绑定左值；
* 右值引用，通过 `T&&` 定义，只能绑定右值；
* 常量左值，通过 `const T&` 定义，既可以绑定左值又可以绑定右值；
* 已命名的右值引用，指向右值，但本身是一个左值；
* 编译器有返回值优化，但不要过于依赖。


<!--
https://www.cnblogs.com/qicosmos/p/4283455.html
https://liam.page/2016/12/11/rvalue-reference-in-Cpp/
https://www.cnblogs.com/Braveliu/p/6220906.html
https://juejin.im/post/59c3932d6fb9a00a4b0c4f5b
https://www.ibm.com/developerworks/cn/aix/library/1307_lisl_c11/index.html
https://blog.csdn.net/xiaolewennofollow/article/details/52559306
-->

{% highlight text %}
{% endhighlight %}
