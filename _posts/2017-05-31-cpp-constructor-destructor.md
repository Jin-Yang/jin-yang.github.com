---
title: C++ 构造析构函数详细分析
layout: post
comments: true
language: chinese
tag: [C/C++, Program]
keywords:
description: 在 C++ 中，当类的内存申请完之后，会通过构造函数进行初始化，而构造函数在不同的使用场景下又分成了不同的类型，例如默认构造函数、复制构造函数、移动构造函数等等，而赋值操作也同样会与之耦合。
---

在 C++ 中，当类的内存申请完之后，会通过构造函数进行初始化，而构造函数在不同的使用场景下又分成了不同的类型，例如默认构造函数、简单构造函数、复制构造函数、转换构造函数等等。

同时，在涉及到赋值操作时，最好将赋值操作运算符也重载掉，尤其是会动态申请内存的类；另外，随着 C++11 标准的发布，有引入了移动构造函数以及移动赋值运算符。

这就导致构造函数与赋值重载不断重叠，很容易引起混淆，所以，这里就详细介绍其使用方式。

<!-- more -->

## 构造函数

C++ 中类内存空间创建完之后，会通过构造函数初始化，可以分成 4 类，如下以 `class Rectangle` 为例：

* 默认构造函数，`Rectangle()` 没有任何参数，生成的对象是默认的参数。
* 简单构造函数，`Rectangle(int heigh, int width)` 可以在创建类对象时对类变量进行设置。
* 复制(拷贝)构造函数，`Rectangle(Rectangle &s)` 参数时本类对象的引用。
* 转换构造函数，`Rectangle(int w)` 形参是其它类型的变量，而且只有一个。

除了构造函数之外，还包括了赋值操作符。

析构函数是类的一种特殊的成员函数，其名称与类的名称是完全相同，只是在前面加了个波浪号 `~` 前缀，但不会返回任何值，也不能带有任何参数。该函数会在删除对象时执行，通常用来释放类申请的资源。

### 三法则

Rule of Three 三法则，是一个实际使用时的指导方法，简单来说，就是，如果用户显示定义了析构函数、复制构造函数、赋值运算符中的一个，那么也需要定义其它两个。

这一三法则其主要目的是为了避免一些常见的陷阱，如果没有定义这三个函数，编译器会自动创建，而当用户自己定义了其中某一个时，而其它仍然使用编译器默认，那么就可能会引入一些异常。

另外，如果有使用到 RAII ，那么可以不定义析构函数，也被称为二法则；在 C++11 新增了移动构造函数以及移动赋值运算符，也被称为五法则 (之前的三法则严格来说应该是复制赋值运算符)。

也就是说，在需要管理类的资源分配时使用，也就是需要确定如何分配、赋值时是否需要深拷贝、如何释放资源等。

### 实例

如下是一个简单示例。

``` cpp
#include <iostream>

class Rectangle {
private:
        double width, height;

public:
        // Default Constructor
        Rectangle(void): width(0), height(0) {
                std::cout << "Default Constructor" << std::endl;
        }

        // Simple Constructor
        Rectangle(double w, double h) {
                std::cout << "Simple Constructor" << std::endl;
                width = w;
                height = h;
        }

        // Convert Constructor
        Rectangle(int v) {
                std::cout << "Convert Constructor" << std::endl;
                width = v;
                height = v;
        }

        // Copy Constructor
        Rectangle(const Rectangle &r) {
                std::cout << "Copy Constructor" << std::endl;
                width = r.width;
                height = r.height;
        }

        // Copy-Assignment Operator
        Rectangle& operator=(const Rectangle &r) {
                std::cout << "Copy Assignment Operator" << std::endl;
                if (this == &r) /* self assignment */
                        return *this;
                width = r.width;
                height = r.height;
                return *this;
        }

        // Move Constructor
        Rectangle(Rectangle &&r) {
                std::cout << "Move Constructor" << std::endl;
                width = r.width;
                height = r.height;
        }

        // Move-Assignment Operator
        Rectangle& operator=(const Rectangle &&r) {
                std::cout << "Move Assignment Operator" << std::endl;
                if (this == &r) /* self assignment */
                        return *this;
                width = r.width;
                height = r.height;
                return *this;
        }

        void String(void) {
                std::cout << "Width " << width << " Height " << height << std::endl;
        }
};

Rectangle CreateRectangle(void)
{
        return Rectangle(5, 6);
}

int main(void)
{
        Rectangle r1;           // Default Constructor
        Rectangle r2(3, 4);     // Simple Constructor
        Rectangle r3(3);        // Convert Constructor
        Rectangle r4 = r2;      // Copy Constructor
        Rectangle r5;           // Default Constructor
        r5 = r2;                // Copy-Assignment Operator

        Rectangle&& r6 = CreateRectangle(); // Move Constructor
        Rectangle r7;           // Default Constructor
        r7 = Rectangle(3, 4);   // Simple Constructor + Move Assignment Operator
        Rectangle r8 = 3;       // Convert Constructor + Move Constructor

        r1.String();
        r2.String();
        r3.String();
        r4.String();
        r5.String();
        r6.String();
        r7.String();
        r8.String();

        return 0;
}
```


## 简单、转换构造函数

先介绍两个相对来说比较简单的构造函数，也就是简单构造函数和转换构造函数。

### 简单构造函数

通过入参构造一个类对象。

### 转换构造函数

用于将其它类型的变量隐式转换为本类对象，一般用于将基本的类型转换成类对象使用，例如上述的 `Rectangle(int v)` 函数，会将基本 `int` 类型的变量转换为 `Rectangle` 类对象。

例如，类重载了 `+` 运算符，使得两个类对象可以相加，那么如果使用是一个类对象和一个 `int` 类型对象，那么在相加之前会先将 `int` 类型转换为类对象，这里用到的就是转换构造函数。

## 默认构造函数

一般来说，就是没有任何参数传入，例如如下示例。

``` cpp
Rectangle r;
```

调用该构造函数时不需要传入任何参数，可以是却是没有入参，所有的成员变量初始化为默认值；也可以是有参数列表，但是都指定的默认值，允许调用时不传入任何参数。

``` cpp
Rectangle(void): width(0), height(0) {
	std::cout << "Default Constructor" << std::endl;
}

Rectangle(int w = 0, int h = 0): width(w), height(h) {
	std::cout << "Default Constructor" << std::endl;
}
```

如果没有定义默认构造函数，编译器会自动生成默认构造函数，注意，并不是所有场景都会生成的，只有需要时才会。而关键时，什么时候才需要默认构造函数。

### 编译器生成

总共有四种场景，编译器需要通过创建默认构造函数来完成一些工作。

<!--
#### 1. 含有类对象数据成员，该类对象类型有默认构造函数

编译器自动生成一个默认构造函数，来调用其成员对象的构造函数，完成该成员的初始化构造；如果这个成员的类也没有默认构造函数，那么编译器也不会自动生成该类的默认构造函数。

第二种情况是这个类的基类有默认构造函数。那么C++编译器也会帮你生成该派生类的默认构造函数，以调用基类的默认构造函数，完成基类的初始化。另外还得强调一下的是，如果基类没有提供这个默认构造的函数，那么C++编译器也不会为派生类生成默认的构造函数（这里包括两层意思，第一，基类没有任何形式构造函数；第二，基类存在其他形式的非默认构造函数，当然了，这种类型就是编译不过的，道理很明显）。

第三种情况是类中存在虚函数，那么C++编译器会为你生成默认构造函数，以初始化虚表(虚函数表vftable)。

第四种情况是存在虚基类，那么C++编译器会为你生成默认构造函数，以初始化虚基类表(vbtable)。

https://blog.csdn.net/songsong2017/article/details/92082423
https://blog.csdn.net/weixin_43174419/article/details/84678873
https://www.cnblogs.com/xiaodi914/p/5715862.html
-->

## 复制构造函数

在介绍之前，可以先尝试回答几个问题：什么场景会调用复制构造函数？如果没有定义会如何处理？

以如下的 `class Rectangle` 为例，赋值构造函数声明为 `Rectangle(const Rectangle &r)` ，注意，拷贝构造函数 **必须以引用的方式传参**，否在在传值时会再次调用一个拷贝构造函数生成对象，以此往复直至栈溢出。

如果实现时没有定义，编译器会自动生成一个拷贝构造函数和拷贝赋值运算符，当然，如果不需要，可以通过 `delete` 关键字显示指定不自动生成，这样会导致对象在函数传参时不能通过值传递，而且不能执行赋值运算。

### 使用场景

拷贝构造函数和赋值运算符比较类似，都是将一个对象的值复制给另一个对象，其区别是：A) 拷贝构造函数使用传入对象的值生成一个新的对象的实例；B) 赋值运算符是将对象的值复制给一个已经存在的实例。关键是是否要生成新的对象。

调用拷贝构造函数主要有以下场景：

* 对象作为函数的参数，以值传递的方式传给函数。
* 对象作为函数的返回值，以值的方式从函数返回。
* 使用一个对象给另一个对象初始化。

可以参考如下的示例，也就是其中的如下几个语句。

``` cpp
Rectangle r3 = r1; // Copy Constructor
Rectangle r4(r1);  // Copy Constructor
FuncFoo(r1);       // Copy Constructor
```

其中比较容易混淆的是 `p3 = p1` ，如果 `p3` 已经创建完成，那么会调用赋值运算符，如果还未创建，就会调用复制构造函数。

### 深拷贝 VS. 浅拷贝

默认生成的拷贝构造函数和赋值运算符，只进行简单的值复制，对于像 `int` `string` 这类的基本类型是不影响的，两个对象包含了各自的成员。

但是，如果使用的是指针，如果只是值复制，那么会导致两个对象操作的是一个相同的对象。

### 示例

在调用函数时，编译器会进行部分的优化。

{% highlight cpp %}
#include <iostream>

class Rectangle {
private:
        int width, height;
public:
        Rectangle() { }
        Rectangle(int w, int h): width(w), height(h) {
                std::cout << "Constructor" << std::endl;
        }
        ~Rectangle() {
                std::cout << "Destructor" << std::endl;
        }
        Rectangle(const Rectangle &r) {
                std::cout << "Copy Constructor" << std::endl;
        }
        Rectangle& operator=(const Rectangle &r) {
                std::cout << "Assignment Operator" << std::endl;
                return *this;
        }
        //Rectangle(const Rectangle &p) = delete;
        //Rectangle& operator=(const Rectangle &p) = delete;
};

void FuncFoo(Rectangle r)
{
}

Rectangle FuncBar(void)
{
        Rectangle r(20, 40);
        return r;
}

int main(void)
{
        Rectangle r1(10, 20), r2;

        Rectangle r3 = r1; // Copy Constructor
        Rectangle r4(r1);  // Copy Constructor
        r2 = r1;           // Assignment Operator
        FuncFoo(r1);       // Copy Constructor
        r2 = FuncBar();    // Constructor + Assignment Operator
        Rectangle r5 = FuncBar(); // Constructor

        return 0;
}
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
