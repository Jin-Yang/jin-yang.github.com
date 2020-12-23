---
title:
layout: post
comments: true
language: chinese
tag: [Program, C/C++]
keywords: C/C++,gmock
description:
---


<!-- more -->

## 内存布局

可以通过 `sizeof()` 获取整个类对象的大小，使用 `offsetof()` 查看成员变量的偏移 (必须是 `public` 类型)，例如如下示例。

``` cpp
#include <iostream>

class Rectangle {
public:
        int width, height;
};

int main(void)
{
	std::cout << "sizeof(): " << sizeof(Rectangle) << std::endl;
	std::cout << "offset(): " << offsetof(Rectangle, width) << ", " <<
		offsetof(Rectangle, height) << std::endl;

        return 0;
}
```

其便以后执行的输出为。

```
sizeof(): 8
offset(): 0, 4
```

也就是说成员变量是按照定义时的顺序保存，而类对象的大小可以理解为所有成员变量大小之和。

只要不包含虚函数，意味着不会发生运行态的动态绑定，那么函数之间的调用关系在编译期间就可以确定下来，这样，就不会对内存的结构造成影响。






   都知道C++中的多态是用虚函数实现的： 子类覆盖父类的虚函数， 然后声明一个指向子类对象的父类指针， 如Base *b = new Derive();
当调用b->f()时， 调用的是子类的Derive::f()。 
这种机制内部由虚函数表实现，下面对虚函数表结构进行分析，并且用GDB验证。



虚继承和虚函数是完全无相关的两个概念。

虚继承是解决C++多重继承问题的一种手段，从不同途径继承来的同一基类，会在子类中存在多份拷贝。这将存在两个问题：其一，浪费存储空间；第二，存在二义性问题，通常可以将派生类对象的地址赋值给基类对象，实现的具体方式是，将基类指针指向继承类（继承类有基类的拷贝）中的基类对象的地址，但是多重继承可能存在一个基类的多份拷贝，这就出现了二义性。


虚继承可以解决多种继承前面提到的两个问题：

虚继承底层实现原理与编译器相关，一般通过虚基类指针和虚基类表实现，每个虚继承的子类都有一个虚基类指针（占用一个指针的存储空间，4字节）和虚基类表（不占用类对象的存储空间）（需要强调的是，虚基类依旧会在子类里面存在拷贝，只是仅仅最多存在一份而已，并不是不在子类里面了）；当虚继承的子类被当做父类继承时，虚基类指针也会被继承。


实际上，vbptr指的是虚基类表指针（virtual base table pointer），该指针指向了一个虚基类表（virtual table），虚表中记录了虚基类与本类的偏移地址；通过偏移地址，这样就找到了虚基类成员，而虚继承也不用像普通多继承那样维持着公共基类（虚基类）的两份同样的拷贝，节省了存储空间。


在这里我们可以对比虚函数的实现原理：他们有相似之处，都利用了虚指针（均占用类的存储空间）和虚表（均不占用类的存储空间）。

虚基类依旧存在继承类中，只占用存储空间；虚函数不占用存储空间。

虚基类表存储的是虚基类相对直接继承类的偏移；而虚函数表存储的是虚函数地址。

 

此篇博客有关于虚继承详细的内存分布情况 

{% highlight text %}
{% endhighlight %}