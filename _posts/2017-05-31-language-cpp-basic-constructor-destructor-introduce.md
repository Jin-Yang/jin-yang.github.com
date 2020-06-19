---
title: C++ 构造析构
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---


<!-- more -->

## 构造函数

在 C++ 中的构造函数分成了 4 类，如下以 `class Shape` 为例：

* 默认构造函数，`Shape()` 没有任何参数，生成的对象是默认的参数。
* 初始化构造函数，`Shape(int heigh, int width)` 可以在创建类对象时对类变量进行设置。
* 复制(拷贝)构造函数，`Shape(Shape &s)` 参数时本类对象的引用。
* 转换构造函数，`Shape(int w)` 形参是其它类型的变量，而且只有一个。

### 转换构造函数

用于将其它类型的变量隐式转换为本类对象，一般用于将基本的类型转换成类对象使用。

{% highlight cpp %}
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
{% endhighlight %}

<!--
https://blog.csdn.net/zxc024000/article/details/51153743
-->


{% highlight text %}
{% endhighlight %}
