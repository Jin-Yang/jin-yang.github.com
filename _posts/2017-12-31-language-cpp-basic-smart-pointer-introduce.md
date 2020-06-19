---
title: C++ 智能指针
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->



类似于 C 语言，C++ 中申请的对象也需要手动释放，不像 Java GoLang 这类的语言，有自己的垃圾回收机制 (C++中有相关的实现，但非标准) ，这样 C++ 程序员需要重点关注内存的申请和释放，否则可能会导致内存泄漏。

为了解决 C++ 内存泄漏问题，在 C++11 中引入了智能指针 (Smart Pointer)，在一定程度上减轻了对内存的管理。

## 简介

例如对于如下的函数，如果没有显示的调用 `delete(p)` 函数，那么就会造成内存泄漏。

{% highlight cpp %}
#include <unistd.h>
#include <iostream>

class Rectangle {
private:
        double width, height;
};

void foobar(void)
{
        Rectangle *p = new Rectangle();
        //delete(p);
}

int main(void)
{
        while (1) {
                foobar();
                sleep(1);
        }
}
{% endhighlight %}

而智能指针实际上就是为了解决上述的问题。

### 原理

一个栈上创建的对象，在退出栈的作用域之后，该对象会自动销毁，而智能指针就是使用的这一原理。创建一个智能指针保存申请好的内存地址，当程序退出作用域的时候，对应的智能指针被自动销毁，同时会释放其指向的内存。

在 C++11 中提供了三种智能指针 `std::shared_ptr` `std::unique_ptr` `std::weak_ptr` ，定义在头文件 `<memory>` 中，分别用于不同的场景中。

#### unique_ptr

指向一个唯一的对象，该对象可以 `move` 但是不能进行赋值。

{% highlight cpp %}
#include <memory>
#include <iostream>

class Rectangle {
private:
        double width, height;
public:
        Rectangle(double w, double h) {
                width = w;
                height = h;
        }

        double Area(void) {
                return width * height;
        }
};

int main(void)
{
        std::unique_ptr<Rectangle> P1(new Rectangle(10, 5));
        //std::unique_ptr<Rectangle> P2(P1);    // delete copy constructor
        std::cout << P1->Area() << std::endl;   // this'll print 50

        std::unique_ptr<Rectangle> P2;
        P2 = move(P1);
        std::cout << P2->Area() << std::endl;   // this'll print 50
        //std::cout << P1->Area() << std::endl; // cause 'Segmentation fault'

        return 0;
}
{% endhighlight %}

#### shared_ptr

会维护一个引用计数 (可以通过 `use_count()` 查看)，每个 `shared_ptr` 指向相同的内存，使用引用计数加一，析构时则减一，到 0 时会删除所指向的堆内存。

{% highlight cpp %}
#include <memory>
#include <iostream>

class Rectangle {
private:
        double width, height;
public:
        Rectangle(double w, double h) {
                width = w;
                height = h;
        }

        double Area(void) {
                return width * height;
        }
};

int main(void)
{
        std::shared_ptr<Rectangle> P1(new Rectangle(10, 5));
        std::cout << P1->Area() << std::endl;   // this'll print 50

        std::shared_ptr<Rectangle> P2;
        P2 = P1;
        std::cout << P2->Area() << std::endl;   // this'll print 50
        std::cout << P1->Area() << std::endl;   // this'll print 50
        std::cout << P1.use_count() << std::endl;

        return 0;
}
{% endhighlight %}

<!--
#### weak_ptr

类似于 `shared_ptr` 但是没有维护计数器。
https://www.internalpointers.com/post/beginner-s-look-smart-pointers-modern-c
https://www.geeksforgeeks.org/auto_ptr-unique_ptr-shared_ptr-weak_ptr-2/
-->

{% highlight text %}
{% endhighlight %}
