---
title: C++ 基本概念
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

简单介绍一些基本的概念。

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

C++ 支持 编译时 以及 运行时 的多态性，分别通过函数重载以及虚函数来实现。运行时的多态通过指针来实现运行时的一个动态绑定。

也就是调用成员函数时，会根据调用函数的对象的类型来执行不同的函数。

#### 实现

当类中存在虚函数 (不一定是纯虚函数) 或者是派生类，编译器会在类中自动生成一个虚函数表，用来保存类成员函数，通过 `virtual` 修饰的成员函数会被编译器放入虚函数表中，在对象中同时保存了一个指向虚拟函数表的指针 `vptr` 。

那么在执行的时候会执行查找。

### 其它

#### 友元

可以是一个函数或者类，这样通过友元函数可以直接访问类中的 `private` 以及 `protected` 的成员，需要注意，友元不是类的成员，也就不能使用 `this` 指针。



### 示例

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
        Rectangle(double w, double h)
        {
                width = w;
                height = h;
        }

        // Destructor
        ~Rectangle() { } // Do nothing

        double Area(void)
        {
                return width * height;
        }
};

class Circle: public Shape {
private:
        static constexpr double PI = 3.1415926;
        double radius;

public:
        Circle(double r)
        {
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

## 复制构造函数

什么场景会调用复制构造函数？如果没有定义会如何处理？

默认编译器会自动生成一个拷贝构造函数和赋值运算符，可以通过 `delete` 关键字指定不生成，这样会导致对象不能通过值传递以及执行赋值运算。

注意，拷贝构造函数必须以引用的方式传参，否在在传值时会再次调用一个拷贝构造函数生成对象，以此往复直至栈溢出。

### 使用场景

拷贝构造函数和赋值运算符比较类似，都是将一个对象的值复制给另一个对象，其区别是：A) 拷贝构造函数使用传入对象的值生成一个新的对象的实例；B) 赋值运算符是将对象的值复制给一个已经存在的实例。关键是是否要生成新的对象。

调用拷贝构造函数主要有以下场景：

* 对象作为函数的参数，以值传递的方式传给函数。
* 对象作为函数的返回值，以值的方式从函数返回。
* 使用一个对象给另一个对象初始化。

在调用函数时，编译器会进行部分的优化。

{% highlight cpp %}
#include <iostream>

class Person {
private:
        int age;
public:
        Person() { }
        Person(int age): age(age) {
                std::cout << "Constructor" << std::endl;
        }
        ~Person() {
                std::cout << "Destructor" << std::endl;
        }
        Person(const Person &p) {
                std::cout << "Copy Constructor" << std::endl;
        }
        Person& operator=(const Person &p) {
                std::cout << "Assignment Operator" << std::endl;
                return *this;
        }
        //Person(const Person &p) = delete;
        //Person& operator=(const Person &p) = delete;
};

void FuncFoo(Person p)
{
}

Person FuncBar(void)
{
        Person p(20);
        return p;
}

int main(void)
{
        Person p1(10), p2;

        Person p3 = p1; // Copy Constructor
        Person p4(p1);  // Copy Constructor
        p2 = p1;        // Assignment Operator
        FuncFoo(p1);    // Copy Constructor
        p2 = FuncBar(); // Constructor + Assignment Operator
        Person p5 = FuncBar(); // Constructor

        return 0;
}
{% endhighlight %}

### 深拷贝 VS. 浅拷贝

默认生成的拷贝构造函数和赋值运算符，只进行简单的值复制，对于像 `int` `string` 这类的基本类型是不影响的，两个对象包含了各自的成员。

但是，如果使用的是指针，如果只是值复制，那么会导致两个对象操作的是一个相同的对象。

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

{% highlight text %}
{% endhighlight %}
