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
        Rectangle(void): width(0), height(0) { }

        // Simple Constructor
        Rectangle(double w, double h) {
                width = w;
                height = h;
        }

        // Convert Constructor
        Rectangle(int v) {
                width = v;
                height = v;
        }

        // Copy Constructor
        Rectangle(const Rectangle &r) {
                width = r.width;
                height = r.height;
        }

        void String(void) {
                std::cout << "Width " << width << " Height " << height << std::endl;
        }
};

int main(void)
{
        Rectangle r1;           // Default Constructor
        Rectangle r2(3.0, 4.0); // Simple Constructor
        Rectangle r3 = 3;       // Convert Constructor OR r3(3)
        Rectangle r4 = r2;      // Copy Constructor

        r1.String();
        r2.String();
        r3.String();
        r4.String();

        return 0;
}
{% endhighlight %}

<!--
https://blog.csdn.net/zxc024000/article/details/51153743
-->


{% highlight text %}
{% endhighlight %}
