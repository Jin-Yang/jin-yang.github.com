---
title: C++ 基本概念
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

## 简介

{% highlight cpp %}
#include <iostream>

using namespace std;

int main(void)
{
        cout << "Hello World" << endl;
        return 0;
}
{% endhighlight %}

通过 `using namespace std` 标示使用标准的命名空间，可以选择使用多个，只要不发生命名空间的冲突即可。

在编译的时候这里使用最新的 C++11 标准，可以使用 `g++ -std=c++11 -o hello hello.cpp` 命令进行编译。

## 多线程

C++11 提供了标准的线程库 `std::thread` ，可以不再显式依赖 pthread 提供的线程库，实际上底层仍然使用的是该库，这也就是为什么链接时需要添加 `-lpthread` 或者 `-pthread` 选项。

{% highlight text %}
#include <thread>
#include <unistd.h>
#include <iostream>

using namespace std;

void hello(void)
{
        usleep(1000);
        cout << "Hello Concurrent World" << endl;
        cout << "Current thread ID " << this_thread::get_id() << endl;
}

int main(void)
{
        thread thd(hello);

        cout << "Hello World" << endl;
        thd.join();
        cout << "All threads joined!" << endl;

        return 0;
}
{% endhighlight %}

## 面向对象

在面向对象的编程 (OOP) 中包含了三大特性：封装、继承和多态。

### 封装性

使代码模块化，通过对象封装了其特性和行为，分别称之为成员变量以及成员函数。

其中成员访问管理分成了三类：

1. public 类、类对象、派生类、派生类对象 均可访问。
2. protected 类、派生类可以访问；类对象、派生类对象不可以访问。
3. private 只有类内部可以访问。

### 继承

扩展已经存在的代码，可以通过已有的数据类型定义新的类型，继承老成员，并定义新成员。

包括了公有 Public、保护 Protected、私有 Private 继承，共三种方式，主要是对其父类成员中的访问权限设置。

### 多态 Polymorphisn

主要为了接口重用，通过父类指针引用或者指针来实现运行时的一个动态绑定，不同于重载的静态绑定。

C++ 支持 编译时 以及 运行时 的多态性，分别通过函数重载以及虚函数来实现。

{% highlight cpp %}
#include <thread>
#include <unistd.h>
#include <iostream>

using namespace std;

// Abstract Base Class
class Shape {
protected:

private:
        int pri;

public:
        // Pure Virtual Function
        virtual double area(void) = 0;
};

class Rectangle: public Shape {
//protected:
private:
        double width, height;

public:
        Rectangle(double w, double h)
        {
                width = w;
                height = h;
        }

        double area(void)
        {
                return width * height;
        }
};

class Circle: public Shape {

};

int main(void)
{
        Rectangle rec(10, 100);

        cout << "got area " << rec.area() << endl;

        return 0;
}
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
