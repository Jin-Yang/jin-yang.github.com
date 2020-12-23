---
title: 详细介绍 C++ 的强制转换方法
layout: post
comments: true
language: chinese
tag: [C/C++, Program]
keywords: const_cast,static_cast,dynamic_cast,reinterpret_cast,类型转换
description: C++ 针对不同的场景对强制类型转换进行了分类，同时相比 C 来说也可以更加安全，这里详细介绍其使用场景。
---

在 C 中提供了强制类型转换，不过其实现很简单，而且没有区分具体的使用场景，所以，在编译或者运行阶段可能会出现一些异常，尤其是一旦不注意极易发生异常。

为此，C++ 对于不同的场景进行了划分，由于 C++ 支持面向对象编程，也因此对这一场景进行了优化。

本文详细介绍其使用场景。

<!-- more -->

## 简介

C++ 支持 C 风格的强制类型转换，无论是什么类型，都可以通过 `TypeName VarB = (TypeName)VarA;` 进行装换，因为不区分具体的场景，所以在某些场景下可能会存在隐患。

所以 C++ 提供了一组可以在不同场合下执行强制转换的函数，主要分成了四种：

* `const_cast` 去除 `const` 属性，将之前无法修改的变量变为可修改。
* `static_cast` 静态类型转换，常用于基本类型转换，例如将 `int` 转换成 `char` 。
* `dynamic_cast` 动态类型转换，多态类之间的类型转换，如子类和父类之间的多态类型转换。
* `reinterpret_cast` 重新解释类型，不同类型指针和整型之间的相互转换，没有进行二进制的转换。

在转换时可以通过 `TypeName VarB = XXX_cast<TypeName>(VarA);` 这种方式进行转换，下面详细介绍四种方法的区别。

## const_cast

通过 `const` 限定该变量的值不能被修改，而 `const_cast` 则是强制去掉这种属性。注意，这里去除的不是变量的常量性，而是去除的是**指向常量的指针或者引用**。

如下，其中有两个错误：1) `ptr` 指向的是常量，不允许对常量进行修改；2) 通过 `const_cast` 强制转换对象必须是指针或者引用，不允许变量。

``` cpp
#include <iostream>

int main(void)
{
        const int var = 10;
        const int *ptr = &var;
        int *data;

        //*ptr = 20; // ERROR <1>
        //int val = const_cast<int>(var); // ERROR <2>

        data = const_cast<int *>(ptr);
        *data = 20; // OK

        std::cout << var << " " << *ptr << " " << *data << std::endl;
        std::cout << &var << " " << ptr << " " << data << std::endl;

        return 0;
}
```

如果通过 `g++ -std=c++11 -o main main.cc` 编译，在 `x86_64` 机器上的执行结果为。

```
10 20 20
0x7ffcf79ef4ac 0x7ffcf79ef4ac 0x7ffcf79ef4ac
```

也就是说，从执行结果上看，首先没有修改常量；其次，所有的地址信息是相同的。原则上来说，常量不修改应该是正常的，虽然我们在使用该方法尝试将其修改。

但是，为什么变量 `var` 没有被修改？？？

### Why !?

在介绍其原因之前，先看看可能的修改策略。可以在声明变量时添加 `volatile` 关键字，也就是将声明修改为如下。

```
const volatile int var = 100;
const volatile int *ptr = &var;
```

编译后执行，可以看到对应 `var` 的值也被修改为了 `20` 。

> 另外，在输出地址时，会看到 `var` 和 `ptr` 的地址都为 `1` ，这主要是因为 `volatile int *` 并没有重载运算符，导致默认输出的是 bool 类型，而当值大于 0 时输出的就是 1 。

其实之前输出 `10` 的原因是因为 C++ 在编译阶段的优化策略，也就是常量折叠，对于常量来说，其值放在编译器的符号表中，在计算时编译器直接从表中取值，省去了访问内存的时间，从而达到了优化。

在添加了 `volatile` 关键字之后，编译器会认为该变量是易变的，所以会直接去内存中读取数据。

如果通过 `g++ -std=c++11 -S -o main.s main.cc` 生成汇编代码，如果没有使用 `volatile` 可以看到如下内容。

```
movq    -32(%rbp), %rax
movl    (%rax), %ebx
movq    -24(%rbp), %rax
movl    (%rax), %r12d
movl    $100, %esi
```

而使用了之后会有如下内容。

```
movq    -32(%rbp), %rax
movl    (%rax), %ebx
movq    -24(%rbp), %rax
movl    (%rax), %r12d
movl    -36(%rbp), %eax
movl    %eax, %esi
```

也就是说，内存中的数据已经修改，但是因为编译器的优化，实际使用的是常量，只是看起来没有修改而已。

### 使用场景

如果无法使用上述的方式，那么最常用的场景是什么？

``` cpp
#include <iostream>

const int *search(const int *arr, int size, int val)
{
        int i;

        for (i = 0; i < size; i++)
                if (arr[i] == val)
                        return &arr[i];
        return NULL;
}

int main(void)
{
        int arr[] = {0, 1, 2, 3, 4};
        int val = 3;
        int *ptr;

        ptr = const_cast<int *>(search(arr, 5, val));
        if(ptr == NULL)
                return -1;
        std::cout << "Before change " << arr[3] << std::endl;
        *ptr = 10;
        std::cout << "After change " << arr[3] << std::endl;

        return 0;
}
```

编译后执行，其输出结果如下。

```
Before change 3
After change 10
```

也即是说，对应的值通过指针被修改了。

这是 `const_cast` 最常见的使用场景，也即是说，原始的变量实际是可以修改的，但是在中间处理过程中，因为某些原因将其转换为了 `const` ，所以，强制修改是没有问题的。

### 总结

简单来说，C++ 的标准没有对通过指针或引用修改常量的这一场景进行定义，其具体的行为完全取决于编译器的实现，也就算是，对于 `volatile` 与 `const` 的优先级并没有明确定义。

所以，强烈不建议对声明为 `const` 的变量进行修改，尤其对于跨平台的软件来说。

总结如下。

* 如果变量开始定义为 `const` 也就意味着不想修改，那么就不要尝试去修改了。
* 原始变量可以修改，中间指针或者引用传递过程引入了 `const` ，那么可以通过 `const_cast` 进行转换。

## static_cast

与 C 语言的强制转换效果相同，也是默认的隐式转换方式，因为没有运行时类型检查，所以在转换时会存在安全风险。使用时需要注意：

* 从派生类转换为基类是安全的，反之因为没有动态类型检查，不安全；
* 基本类型转换需要开发者保证其安全性，例如 `int` 转换为 `char` 或者 `enum` 等；
* 不能去掉原类型的 `const`、`volatile` `__unaligned` 属性，前两者可以使用 `const_cast` 转换。

如果可以用其它类型的转换，那么就不要用改类型，除非迫不得已。

``` cpp
#include <iostream>

int main(void)
{
        int a = 10, b = 3;
        double result = static_cast<double>(a) / static_cast<double>(b);

        std::cout << a / b << "   " << result << std::endl;

        return 0;
}
```

直接将整型转换为了浮点型进行计算。

## dynamic_cast

这应该是最特殊的一个，会涉及到面向对象的多态性和程序运行时的状态，也是面向对象编程时最常有用的一种。使用时需要注意：

* 其它三种是在编译阶段完成，而 `dynamic_cast` 需要在运行时检查类型，而且不能用于内置基本数据类型的强制转换。
* 转换成功返回的是指向类的指针或引用，失败则会返回 NULL 。
* 通过 `dynamic_cast` 进行转换时，基类中一定要有虚函数，否则编译不通过。
* 向下转换 (派生类指针指向基类对象) 时，使用 `dynamic_cast` 更安全；而向上转换 (基类指针指向派生类对象) 时，`dynamic_cast` 和 `static_cast` 两者效果相同。

之所以要求有虚函数，是因为，只有存在虚函数，才有可能会出现让基类指针/引用指向派生类对象的情况，也就才有转换的意义。在运行时，该信息会存储在类的虚函数表中，以供运行时检查。

### 异常转换

如下，通过 `static_cast` 进行了向下转换，也就是将派生类指针指向基类对象。

``` cpp
#include <iostream>

class Shape {
public:
        const char *ToString(void) {
                return "empty";
        }
};

class Rectangle: public Shape {
private:
        double width, height;
public:
        Rectangle(double w, double h): width(w), height(h) { }
        double Area(void) {
                return width * height;
        }
};

int main(void)
{
        Rectangle *ptr;

        //ptr = new Shape; // ERROR
        ptr = static_cast<Rectangle *>(new Shape);
        std::cout << ptr->ToString() << std::endl;
        std::cout << ptr->Area() << std::endl;

        return 0;
}
```

这里强制将一个基类转换为了派生类，两者分别定义了两个不同的成员函数，如果直接使用上述的 `ptr = new Shape;` 方式，编译阶段会有 `invalid conversion from ‘Shape*’ to ‘Rectangle*’ [-fpermissive]` 的报错。

当然，后面的话进行了强制转换，虽然可以编译成功，但实际上将派生类指针指向基类是不安全的。

因为基类中定义了 `ToString()` 函数，直接通过 `ptr->ToString()` 调用是合法的，而在执行 `ptr->Area()` 时，虽然没有语法错误，严格来说会产生一个运行是错误 (不过执行时没有报错，不知道为啥)。

在调用 `ToString()` 函数时，调用了基类的函数，返回的是 `empty` 字符串，符合预期；而在调用 `Area()` 时，返回 0 ，估计是因为成员变量默认初始化为了 0 。（暂时不确定为啥基类中没有改函数，竟然可以调用，而且这里的成员变量是什么时候申请，为什么会初始化，而非对内存地址的非法访问）

### 使用

简单来说，向下转换 (派生类指针指向基类对象) 是否成功，需要保证被转换指针所指向的对象实际类型与转换以后的对象类型一定要相同，否则转换失败。也就是说，基类指向的对象原来就是那个派生类，使用基类只是为了可以兼容所有派生类。

``` cpp
#include <iostream>

class Shape {
public:
        virtual double Area(void) {
                return 0;
        }
};

class Rectangle: public Shape {
private:
        double width, height;
public:
        Rectangle(double w, double h): width(w), height(h) { }

        double Area(void) {
                return width * height;
        }
};

int main(void)
{
        Shape *S1 = new Shape;
        Shape *S2 = new Rectangle(5, 10);

        Rectangle *R1;

        R1 = dynamic_cast<Rectangle *>(S2); // OK <1>
        std::cout << R1->Area() << std::endl;

        R1 = dynamic_cast<Rectangle *>(S1); // FAIL <2>
        if (R1 == nullptr)
                std::cout << "got nullptr" << std::endl;

        return 0;
}
```

在 `<1>` 中，原来的 S2 指向的就是一个 Rectangle 对象，所以可以直接转换成功；而 `<2>` 因为 `S1` 是基类，转换会失败，也就是返回 `nullptr` 。

### 总结

基类的一个用途就是通过多态承载不同的派生类，而 `dynamic_cast` 的作用只是将之前的派生类从基类指针或者引用中 **安全地** 还原回去。

## reinterpret_cast

该转换方式仅仅只是比特位的拷贝，在使用过程中需要特别谨慎，常用的场景有：A) 指针/引用的相互转换，例如函数指针、类型指针等；B) 整型和指针的相互转换，需要确保整型可以容纳指针。

因为可以在任意的指针和整型之间相互转换，所以，需要用户确保相互转换的安全性。

``` cpp
#include <iostream>

int main(void)
{
        int num = 0x00434241; // 0x41 <-> 'A'
        int *pint = &num;
        char *pstr = reinterpret_cast<char *>(pint);

        std::cout << pint << "   " << static_cast<void *>(pstr) << std::endl;
        std::cout << pstr << "   " << std::hex << *pint <<std::endl;

        return 0;
}
```

其中 `static_cast<void *>(pstr)` 是为了输出指针，否则会直接输出字符串的内容，在 `x86_64` 上的输出如下。

```
0x7fffa772b9c0   0x7fffa772b9c0
ABC   434241
```

如果仔细查看，其实 `pstr` 和 `*pint` 输出的值是一致的，也就是说 `reinterpret_cast` 运算符并不会改变括号中运算对象的值，只是将该对象从位模式上进行重新解释。

但是为什么之前说输出值相等？实际输出的是 `ABC` 和 `434241` 啊！

实际上，在内存中的数据如下。

```
+-----------+
|  0x41(A)  | 0x7fffa772b9c0
+-----------+
|  0x42(B)  | 0x7fffa772b9c1
+-----------+
|  0x43(C)  | 0x7fffa772b9c2
+-----------+
|  0x00     | 0x7fffa772b9c3
+-----------+
```

对于字符串来说，会按照地址会按照单字节递增的顺序输出，直到遇到终止符 `NULL` 或者 `\0` 为止，这也就是为什么 `pstr` 输出的是 `ABC` 字符串；而对于 `*pint` 来说，返回的是整型，因为 `x86_64` 是小端字节序，也就是高位在低地址，那么十六进制输出就是 `434241` 了。

### 总结

用于指针和整型的转换，转换时一定要明确转换后的具体含义，否则就不要随便转换。


{% highlight text %}
{% endhighlight %}
