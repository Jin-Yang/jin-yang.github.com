---
title: 关于 C++ 基本概念介绍
layout: post
comments: true
language: chinese
tag: [C/C++, Program]
keywords:
description: C++ 从底层 C 发展而来，提供了面向对象的编程机制，包括了一些其它更丰富的特性，尤其是随着 C++11 以及 C++0x 标准的发布，这里会介绍一些基本的概念。
---

一般会将 C++ 划分为所谓的中级语言，从底层的 C 语言发展而来，提供了比 C 更丰富的特性，而又没有提供类似 Java、Python 等语言的高级特性，所以，介于中间，称之为中级语言。

随着 C++11 标准的发布，以及更新的 C++0x 标准，C++ 提供的功能越来越完善。

这里仅简单介绍下基本的概念。

<!-- more -->

## 简介

C++ 是由 Bjarne Stroustrup 于 1979 年在贝尔实验室开始设计开发的，进一步扩充和完善了 C 语言，是一种面向对象的程序设计语言，可以行于多种平台上，如 Windows、MAC、Unix、Linxu 等各种版本。

不过不像 Java、Python 这类语言，其运行依赖于底层的虚拟机进行解释，编译后的代码无需重新编译，而 C++ 在不同的平台上需要重新编译，以生成不同的二进制文件。

这里，主要以 Linux 平台上的 gcc 为例。

### 示例

如下是一个最简单的 Hello World 示例。

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

## 面向对象

在面向对象的编程 (OOP) 中包含了比较重要的三大特性：封装、继承和多态。

### 封装

使代码模块化，通过对象封装了其特性和行为，分别称之为成员变量以及成员函数。其中成员访问管理分成了三类：

1. public 类、类对象、派生类、派生类对象 均可访问。
2. protected 类、派生类可以访问；类对象、派生类对象不可以访问。
3. private 只有类内部可以访问。

### 继承

扩展已经存在的代码，可以通过已有的数据类型定义新的类型，继承老成员，并定义新成员。

包括了公有 Public、保护 Protected、私有 Private 继承，共三种方式，主要是对其父类成员中的访问权限设置。

### 多态 Polymorphisn

C++ 支持**编译时**以及**运行时**的多态性，分别通过函数重载以及虚函数来实现，注意，运行时的多态是通过虚拟函数指针实现的，也就是为什么是运行时动态绑定。

可以查看如下的 `double GetArea(Shape &s)` 函数，其入参是基类 `Shape` ，允许用户传入其派生类对象，例如 `Rectangle`、`Circle`，也就是调用成员函数时，会根据调用函数的对象的类型来执行不同的函数。

### 示例

如下是一个简单的示例，展示一些基础的特性，其中 `class Shape` 包含了纯虚函数，也就是一个接口类，无法进行实例化，而派生类包含了四边形、圆形的实现。

{% highlight cpp %}
#include <iostream>

using namespace std;

// Abstract Base Class
class Shape {
protected:
private:
        int pri;

public:
        // Pure Virtual Function
        virtual double Area(void) = 0;
};

class Rectangle: public Shape {
//protected:
private:
        double width, height;

public:
        // Simple Constructor
        Rectangle(double w, double h) {
                width = w;
                height = h;
        }

        // Destructor
        ~Rectangle() { } // Do nothing

        double Area(void) {
                return width * height;
        }
};

class Circle: public Shape {
private:
        static constexpr double PI = 3.1415926;
        double radius;

public:
        Circle(double r) {
                radius = r;
        }
        double Area(void);
        double GetDiameter(void);
        friend void DumpInfo(Circle &c);
};

void DumpInfo(Circle &c)
{
        cout << "radius is " << c.radius << endl;
}

double Circle::Area(void)
{
        return PI * radius * radius;
}

double Circle::GetDiameter(void)
{
        return radius * 2;
}

double GetArea(Shape &s)
{
        return s.Area();
}

int main(void)
{
        Rectangle rec(10, 100);
        Circle c(10);

        DumpInfo(c); // friend function

        cout << "Area Rect=" << GetArea(rec) << " Circle=" << GetArea(c) << endl;

        return 0;
}
{% endhighlight %}

类中函数的。

## 其它

### 初始化列表

对于简单的对象可以使用初始化列表，但是需要注意，初始化列表是按照声明的顺序，而非在初始化列表中的顺序。

``` cpp
#include <iostream>

class Rectangle {
public:
        double width, height;
        Rectangle(double w): height(w), width(height) {}
};

int main(void)
{
        Rectangle r(1.2);
        std::cout << r.height << std::endl << r.width << std::endl;

        return 0;
}
```

上述的代码初始化时，实际上会先将 `width` 初始化为 `height`，而此时的 `height` 变量没有初始化，是个随机值，通常会很大；接着，才会通过入参 `w` 初始化 `height` 变量。

所以，最终的结果导致 `width` 变量值随机，`height` 的值是入参。

另外，在初始化列表中可以使用 `new` 关键字，例如 `Array(int s): size(s), data(new int[s])` 。

## 模板

泛型编程的基础，一种独立于任何特定类型的编码方式，常见的库容器都是采用的泛型编程。

### 函数模板

{% highlight cpp %}
#include <string>
#include <iostream>

template <typename T>
inline T const &Max(T const &a, T const &b)
{
        return a < b ? b : a;
}
int main(void)
{
        std::cout << "Max(int)   : " << Max((int)10, (int)20) << std::endl;
        std::cout << "Max(double): " << Max((double)10, (double)20) << std::endl;

        return 0;
}
{% endhighlight %}

<!--
C++11
https://blog.csdn.net/FX677588/article/details/70157088
-->


通过 `static` 关键字描述的成员函数或者变量，所有的类实例只会保存一份实现，不能通过

## 静态数据成员

静态数据成员可以实现多个对象之间的数据共享，但是不能直接在类内定义处初始化，需要单独定义，而且必须要初始化，同时，在初始化时，为了防止与普通的静态变量混淆，初始化时无需添加 `static` 关键字。

对于 `static const` 的整形变量 (可以是 `char` `int` `long` `enum` 等)，可以直接在类的定义处初始化，在 C++11 引入了 `constexpr` 关键字，可以用作其它类型的初始化。

{% highlight cpp %}
#include <iostream>

class Circle {
private:
        //static const double PIA = 3.1415926;   // NOT OK
        static constexpr double PIB = 3.1415926;  // OK for C++11
        const double PIC = 3.1415926;

        int countA = 1;
        const int countB = 2;
        //static int countC = 3; // MUSTBE const
        static const int countD = 4;
};

int main(void)
{
        return 0;
}
{% endhighlight %}

<!--
https://zhuanlan.zhihu.com/p/20206577
https://blog.csdn.net/tobefxz/article/details/14109697
-->







### 其它

#### 友元

可以是一个函数或者类，这样通过友元函数可以直接访问类中的 `private` 以及 `protected` 的成员，需要注意，友元不是类的成员，也就不能使用 `this` 指针。





#### 实现

当类中存在虚函数 (不一定是纯虚函数) 或者是派生类，编译器会在类中自动生成一个虚函数表，用来保存类成员函数，通过 `virtual` 修饰的成员函数会被编译器放入虚函数表中，在对象中同时保存了一个指向虚拟函数表的指针 `vptr` 。

那么在执行的时候会执行查找。




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


{% highlight text %}
{% endhighlight %}
