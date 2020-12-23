---
title: C++ 可调用对象
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

在 C++11 之前，可调用对象包括了函数、函数指针以及重载 `operator()` 运算符的对象；而在 C++11 之后，新增了 `lambda` 表达式以及 `bind()` 返回值，同时，引入了新的 `function` 类型，用来统一可调用对象。

<!-- more -->

## 简介

在 C++11 之前，可调用对象包括了函数、函数指针以及重载 `operator()` 运算符的对象；而在 C++11 之后，新增了 `lambda` 表达式以及 `bind()` 返回值，同时，引入了新的 `function` 类型，用来统一可调用对象。

> bind() 函数可以把已有的变量绑定到指定函数的参数，从而产生一个新的函数。

如下是一个使用可调用对象的示例。

{% highlight cpp %}
#include <string>
#include <iostream>
#include <functional>

class Hey {
public:
        void operator() (const std::string &msg) {
                std::cout << "Hey " << msg << "!" << std::endl;
        }
};

void Hi(const std::string &msg)
{
        std::cout << "Hi " << msg << "!" << std::endl;
}

void Message(const std::string &pre, const std::string &msg)
{
        std::cout << pre << msg << "!" << std::endl;
}

int main(void)
{
        Hey hey;
        void (*Hello)(const std::string &);

        /* before C++11 */
        Hi("Andy");                // function
        Hello = Hi, Hello("Andy"); // function pointer
        hey("Andy");               // override operator()

        /* after C++11 */
        std::function<void(const std::string &)> callit;

        callit = Hi;
        callit = hey;
        callit = [](const std::string &msg) {
                std::cout << "Hi " << msg << "!" << std::endl;
        };
        callit = std::bind(Message, "Hello ", std::placeholders::_1);
        callit("Bruce");

        return 0;
}
{% endhighlight %}

### ref cref

在 C++ 语法中，已经存在了引用，那么为什么在 C++11 中还要引入 `std::ref` 呢？

其实，主要是考虑到函数式编程 (例如 `std::bind()` ) ，默认是对参数的直接拷贝而非引用，通过 `std::ref` 和 `std::cref` 可以分别表示引用以及 `const` 引用。

{% highlight cpp %}
#include <iostream>
#include <functional>

void foobar(int &a, int &b, const int &c)
{
        std::cout << "foobar " << a << ", " << b << ", " << c << std::endl;
        ++a; // increments the copy of a stored in the function object
        ++b; // increments the main()'s variable.
        // ++c; // compile error
}

int main(void)
{
        int a = 1, b = 2, c = 3;
        std::function<void()> func = std::bind(foobar, a, std::ref(b), std::cref(c));
        func();
        std::cout << " After " << a << ", " << b << ", " << c << std::endl;

        return 0;
}
{% endhighlight %}

其输出的结果如下。

{% highlight text %}
foobar 1, 2, 3
 After 1, 3, 3
{% endhighlight %}

也就是说，只有变量 `b` 在被调用的函数中被修改了。

{% highlight text %}
{% endhighlight %}
