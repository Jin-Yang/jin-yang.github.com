---
title: C++ 内存模型
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->


## 内存结构

在介绍 C++ 对象在内存分布之前，先讨论下 C++ 中的对象是如何创建、初始化、删除的。

简单来说，对于一个普通对象，会先执行内存申请函数，然后执行构造函数，当用户使用完准备释放时，会接着调用析构函数，最后内存释放，那么对于有继承关系的是如何处理的。

``` cpp
#include <iostream>

class Base {
public:
        void *operator new(std::size_t size) {
                std::cout << "Allocate Base "<< size <<  "Bytes" << std::endl;
                ::operator new(size);
        }
        void operator delete(void *ptr, std::size_t size) {
                std::cout << "Deallocate Base" << std::endl;
        }
        Base() {
                std::cout << "Constructor Base" << std::endl;
        }
        ~Base() {
                std::cout << "Deconstructor Base" << std::endl;
        }
};

class Derive: public Base {
public:
        void *operator new(std::size_t size) {
                std::cout << "Allocate Derive " << size << "Bytes" << std::endl;
                ::operator new(size);
        }
        void operator delete(void *ptr,std::size_t size) {
                std::cout << "Deallocate Derive" << std::endl;
        }
        Derive() {
                std::cout << "Constructor Derive" << std::endl;
        }
        ~Derive() {
                std::cout << "Deconstructor Derive" << std::endl;
        }
};

int main(void)
{
        //Base *pb = new Base();
        //delete pb;

        Derive *pd = new Derive();
        delete pd;

        return 0;
}
```

Allocate Derive 1Bytes
Constructor Base
Constructor Derive
Deconstructor Derive
Deconstructor Base
Deallocate Derive






C++ 允许在同一作用域中定义多个函数和运算符，分别称为函数重载和运算符重载。

### 函数重载

在同一作用域中，可以声明几个功能类似的同名函数，但是这些同名函数的形式参数 (参数的个数、类型或者顺序) 必须不同，在编译阶段会决策使用哪个函数的定义。

### 运算符重载

可以重定义或重载大部分 C++ 内置的运算符，这样就可以使用自定义类型的运算符，重载的运算符是带有特殊名称的函数，函数名是由关键字 `operator` 和其后要重载的运算符符号构成的。

## 隐式类型转换

当构造函数只有一个参数时，除了作为显示的构造函数外，还会作为隐式类型转换。

#include <iostream>

class Array
{
public:
        int size;
        Array() {}
        //explicit Array(int s) {
        Array(int s) {
                size = s;
        }
};

int main(void)
{
        Array A0;

        A0 = 10;  // equal to A0 = Array(10);
        std::cout << A0.size << std::endl;

        return 0;
}

如果不希望使用隐式转换，可以通过 `explicit` 关键字进行描述，该关键字用来修饰类的构造函数，被修饰的构造函数的类，不能发生相应的隐式类型转换，只能以显示的方式进行类型转换，否则编译阶段会报错。

在使用时需要注意：A) 只能用在类内部的构造函数声明上；B) 用于单个参数的构造函数。

### 默认参数

上述只有一个参数的构造函数会同时做为隐式转换，但是当设置了默认值之后，其作用其实是类似的。

#include <iostream>

class Rectangle {
public:
        double width, height;
        Rectangle(double w = 0, double h = 0): height(h), width(w) {}
};

int main(void)
{
        Rectangle r(1.2, 1.3); // OR Rectangle r = Rectangle(1.2, 1.3);
        //Rectangle r = (1.2, 1.3); // equal to Rectangle r = 1.3;
        std::cout << r.height << std::endl << r.width << std::endl;

        return 0;
}

上述的结果是 `0` `1.3` ，因为 `(1.2, 1.3)` 中的逗号是合法的运算，结果是 `1.3` ，那么实际最终的构造函数调用的是 `Rectangle(1.3, 0)` 。

## 多态


？？？？？？实测不对
在构造函数中，是不能使用多态特性的，直接调用使用的是基类中定义的函数。这主要是因为多态是通过 `vptr` 实现，真正指向派生类是在完成构造函数之后。

#include <iostream>

using namespace std;

class Shape {
public:
        int state;
        virtual void reset(void) {
                state = -1;
        }
};

class Rectangle: public Shape {
public:
        double width, height;
        Rectangle(void) {
                reset();
        }
        void reset(void) {
                std::cout << "xxxx" << std::endl;
                Shape::reset();
                width = 1;
                height = 1;
        }
};

int main(void)
{
        Rectangle r;
        std::cout << r.state << std::endl << r.width << std::endl;

        return 0;
}


## 资源申请

int *data = new int(10); // 申请一个int的内存，初始化为10
int *data = new int[10]; // 申请数组


内存模型实际上包含了两部分的含义，而且很容易混淆：A) 对象在内存中的分布；B) 内存操作时的语义控制。











https://www.horstmann.com/cpp/pitfalls.html
https://lucumr.pocoo.org/2009/4/8/c++-pitfalls/
http://blog.davidecoppola.com/2013/09/cpp-pitfalls/
https://scotchi.net/cpp-pitfalls/
https://www.typemock.com/pitfalls-c/
https://www.toptal.com/c-plus-plus/top-10-common-c-plus-plus-developer-mistakes

## RTTI 机制

Runtime Type Information, RTTI 也就是运行时类型信息，它提供了一种在运行时确定对象类型的方法。
https://www.jianshu.com/p/3b4a80adffa7
https://zhuanlan.zhihu.com/p/32182911
https://www.cnblogs.com/zhuyf87/archive/2013/03/15/2960899.html

{% highlight text %}
{% endhighlight %}
