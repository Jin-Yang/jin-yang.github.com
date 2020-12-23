---
title: C++ 右值引用
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

在 C++11 的新特性中增加了所谓的右值引用的支持，其主要目的是为了解决两个问题：A) 临时对象非必要的拷贝操作；B) 在模板函数中如何按照参数的实际类型进行转发。

同时，和右值引用相关的概念比较多，包括了纯右值、将亡值、Universal References、引用折叠、移动语义、Move语义和完美转发等等，这里简单介绍。

<!-- more -->

## 引用介绍

在介绍之前，先看下 C++ 中最原始引用变量的定义。引用变量可以看做是另外一个变量的别名，可以通过引用来访问和修改原变量，而这，严格来说是左值引用。

引用与指针类似，很多地方通过引用替换指针会更容易阅读和维护，当然，两者使用方式和场景略有区别：

* 不存在空引用，在声明时必须初始化，需要指向一块合法内存；指针可以为空，允许后续指定。
* 一旦指定引用对象，不能修改；指针可以随时指向另外的对象。

如下是一个简单示例。

``` cpp
#include <iostream>

int main(void)
{
        int var;
        int &ref = var;
        int *ptr;

        /* ERROR: declared as reference but not initialized
        int &ref;
        ref = var;
        */
        var = 10;
        ptr = &var;
        std::cout << "Value " << var << ", Reference " << ref << ", Pointer " << *ptr << std::endl;

        return 0;
}
```

其输出为。

```
Value 10, Reference 10, Pointer 10
```

除了对变量的使用，最常见的是在函数参数以及函数返回值中使用。

### 函数参数

传递参数时包含了值传递、指针传递、引用传递三种方式，其中值传递会将原变量复制一份，会导致函数中的修改无法影响到原变量，后两者则可以影响，可以根据场景使用。

如下是一个简单的交换值的示例。

``` cpp
#include <iostream>

void swap(int &x, int &y)
{
        int tmp;

        tmp = x;
        x = y;
        y = tmp;
}

int main(void)
{
        int x = 1, y = 2;

        std::cout << "Before: x=" << x << ", y=" << y << std::endl;
        swap(x, y);
        std::cout << "After : x=" << x << ", y=" << y << std::endl;

        return 0;
}
```

该函数的执行结果为。

```
Before: x=1, y=2
After : x=2, y=1
```

可以看到两个值已经交换，如果将函数定义为 `void swap(int x, int y)` ，实际上是不会交换的。

### 返回值

当引用作为返回值时，类似于返回一个隐式指针，这样，除了获取该元素外，甚至可以将其放到赋值语句的左边，也就是所谓的左值。

``` cpp
#include <iostream>

int vals[] = {1, 2, 3, 4, 5};

int &GetValue(int idx)
{
        return vals[idx];
}

int main(void)
{
        std::cout << "vals[1]=" << GetValue(1) << std::endl;

        std::cout << "Before:";
        for (int i = 0; i < 5; i++)
                std::cout << " vals[" << i << "]=" << vals[i];
        std::cout << std::endl;

        GetValue(1) = 4;
        GetValue(3) = 8;

        std::cout << "After :";
        for (int i = 0; i < 5; i++)
                std::cout << " vals[" << i << "]=" << vals[i];
        std::cout << std::endl;

        return 0;
}
```

其输出结果如下。

```
vals[1]=2
Before: vals[0]=1 vals[1]=2 vals[2]=3 vals[3]=4 vals[4]=5
After : vals[0]=1 vals[1]=4 vals[2]=3 vals[3]=8 vals[4]=5
```

也就是，通过左值完成了原数组元素的修改。

另外，需要注意，当返回一个引用时，需要注意被引用对象的作用域，返回一个局部变量的引用是不合法的，但允许返回一个对静态变量的引用。

```
#include <iostream>

int &FooBar(void)
{
        /* warning: reference to local variable ‘bar’ returned [-Wreturn-local-addr]
        int bar = 10;
        return bar;
        */
        static int foo = 10;
        return foo;
}

int main(void)
{
        std::cout << "FooBar " << FooBar() << std::endl;
        return 0;
}
```

返回局部变量时会有一个 warning 信息，如果栈被销毁后被覆盖，那么指向的值就会被修改，所以，严格来说不能这么使用。

## 左值 VS. 右值

最开始实际很少会区分所谓的左值(lvalue)、右值(rvalue)，只是在 C/C++11 版本之后，在做一些性能优化时引入了一些新的概念，也同时导致了这两者也很容易混淆。

首先，可以这么简单定义 (后面会补充细节)，左值是可标识的内存位置，可以通过 `&` 符号取地址；而右值定义用的是排除法，一个表达式，不是左值就是右值。

### 示例

例如，当执行 `int i = foobar();` 语句时，通过函数获取一个整形值，但实际上返回了两种类型：A) 左值 `i` 会一直存在；B) 临时值，在表达式结束时被销毁，这个也就是右值，可以执行 `&i` 取其地址，但是无法执行 `&foobar()` 获得地址。

另外一个示例是 `int a = b + c;` ，其中 `a` 就是左值，可以通过 `&a` 获取其地址，而表达 `b + c` 是右值，在赋值给某个具体的变量前，不能直接通过变量名获取，例如 `&(b + c)` 会直接报错。

另外，还有个区别，左值可以出现在赋值操作的左边或者右边，而右值只能出现在左边。

如下是一些常见的示例。

``` cpp
#include <iostream>

int global = 10;

int GetRValue(void)
{
        return global;
}

int &GetLValue(void)
{
        return global;
}

int main(void)
{
        int var;
        int *ptr;

        var = 8;    // OK var is a l-value, and 8 is a r-value
        ptr = &var; // OK both ptr and var are l-value
        8 = var;    // ERROR r-value on the left
        (var + 1) = 8; // ERROR r-value on the left

        ptr = &GetLValue(); // OK return a l-value
        GetLValue() = 20;   // OK return a l-value
        ptr = &GetRValue(); // ERROR return a r-value
        GetRValue() = 20;   // ERROR return a r-value

        return 0;
}
```

简单来说，常见的如常量 (如上的 `8`)、临时值 (通过寄存器而非地址保存数据，如上的 `var + 1`) 都会被看做右值，临时值也包括了函数返回的临时值。

上述的 `GetLValue()` 返回左值对一些重载运算符很有用，例如常见的 `[]` 操作符重载，可以用来实现访问的查找操作，例如 `std::map` 中的使用：

``` cpp
std::map<int, float> amap;
amap[10] = 3.1415926;
```

之所以可以赋值给 `amap[10]` ，就是因为 `std::map::operator[]` 重载后返回的是一个可赋值的引用。

### 相互转换

大多数情况下，对象之间的运算是右值形式，例如常见的加减乘除二元运算，两边的参数都是右值传入，计算结果也是右值。

```
int a = 1; // l-value
int b = 2; // l-value
int c = a + b; // a and b converted to r-values and an r-value returned
```

其中，数组、函数和非完整 (incomplete types) 的左值是不能转换为右值的，所谓的非完整类型，是指用指针声明的类型。

注意，右值是不能直接转换为左值的，但是可以通过一些方法显示转换，例如 `*` 就需要一个右值参数，然后返回一个左值结果，示例如下。

```
int arr[] = {1, 2};
int *ptr = &arr[0];
*(ptr + 1) = 20;   // OK ptr + 1 is an rvalue, but *(ptr + 1) is an lvalue
```

与之相反，取地址操作符 `&` 需要一个左值参数，然后返回一个右值。

```
int var = 10;
int *ptr = &(var + 1); // ERROR lvalue required as unary '&' operand
int *addr = &var;      // OK var is an lvalue
&var = 40;  // ERROR lvalue is required, &var is rvalue
```

其中 `&` 符号还可以定义引用类型，也被称为左值引用，赋值的时候一般是一个左值，不过也允许是右值，但是有一定的约束。

#### 常量引用

如果要将一个右值赋值给左值引用，左值引用的类型必须定义为常量。

```
const std::string &name = "Andy"; // OK
std::string &name = "Andy"; // ERROR use a left reference for a rvalue
```

也就是说，会强制要求使用常量引用来指向右值；或者说，常量左值引用可以使用右值赋值。因为无法通过常量的引用修改变量的值，所以，也就不会出现修改右值的情况。

另外，在作为参数的时候，有个很有意思的场景。

``` cpp
#include <iostream>

void DumpInfo(const std::string &name)
{
        std::cout << "got an rvalue " << name << std::endl;
}

void DumpInfo(std::string &name)
{
        std::cout << "got an lvalue " << name << std::endl;
}

int main(void)
{
        std::string name = "LValue";

        DumpInfo(name);
        DumpInfo("RValue");

        return 0;
}
```

运行结果为。

```
got an lvalue LValue
got an rvalue RValue
```

也就是说，这个差异足以让编译器重载该函数，而其，这也是 C++ 一个比较常见的使用习惯，通过常量左值引用减少不必要的复制。注意，这里实际上存在一个隐式转换，右值转换为常量引用。

### 其它

#### 可修改左值

最开始在 C 语言中，左值被定义为 "可以出现在赋值操作左边的值"，但是当引入 `const` 关键字之后，该定义就不再成立，例如下面的示例。

```
const int a = 10; // a is an l-value
a = 20;           // but it can't be assigned here!
```

也就是说，并不是所有的左值都可以被赋值的，可被赋值的被称为可修改左值 (modifiable lvalues)，C99 的定义为：

> [...] an lvalue that does not have array type, does not have an incomplete type, does not have a const-qualified type, and if it is a structure or union, does not have any member (including, recursively, any member or element of all contained aggregates or unions) with a const-qualified type.

#### CV 限定右值

在 C++ 标准中，其中左值转换为右值有如下的内容。

> An lvalue (3.10) of a non-function, non-array type T can be converted to an rvalue. [...] If T is a non-class type, the type of the rvalue is the cv-unqualified version of T. Otherwise, the type of the rvalue is T.

也就是说，如果是类，那么从左值转换为右值的时候，其中的 CV 限定符会被保留。所谓的 CV 是指 `const` 和 `volatile` 两个类型限定符，会有三种组合：`const`、`volatile` 和 `const volatile` 。

``` cpp
#include <iostream>

class FooBar {
public:
        void Foo(void) const {
                std::cout << "FooBar::Foo() const" << std::endl;
        }
        void Foo(void) {
                std::cout << "FooBar::Foo()" << std::endl;
        }
};

FooBar Bar(void) {
        return FooBar();
}
const FooBar CBar(void) {
        return FooBar();
}

int main(void)
{
        Bar().Foo();
        CBar().Foo();
        return 0;
}
```

执行的输出为。

```
FooBar::Foo()
FooBar::Foo() const
```

在 C 语言中，只有左值会有 CV 限定，而右值绝对不可能存在；而在 C++ 中，类的右值是可以有 CV 限定的，基础类型 (例如 `int`、`double`、`char` 等) 则没有。

## 右值引用

终于进入正题了，C++11 引入的右值引用以及延伸的 move 语义，应该是最强大的特性之一。如前所述，左值 (非 `const`) 可以通过赋值操作修改，但是右值不能，而通过右值引用，可以突破这一限制，允许获取并修改右值。

如上，增加右值引用的主要目的是为了减少非必要的拷贝以及所谓的完美转发。

### 简介

上述的左值引用是对左值进行绑定，而对右值进行绑定的引用就是右值引用，其语法为 `&&` ，如下绑定了一个右值 `0` ，注意，右值引用只能绑定右值，不能绑定左值。

{% highlight text %}
int &&i = 0;
{% endhighlight %}

再看下获取某个函数的返回值。

{% highlight text %}
int i = foobar();
int &&i = foobar();
{% endhighlight %}

虽然两者的写法基本类似，但语义却相差很大，第一行 `foobar()` 产生的临时值在表达式结束时就已经被销毁了，而第二行 `foobar()` 返回的值则会随着 `i` 变量的生命周期结束，而非立即结束。

注意，在最后一行中，变量 `i` 是右值引用类型 `int &&`，但如果从左右值角度看，实际上是个左值，可以取地址，是一个已经赋值的右值。














在 C++11 中，所有的值必属于左值、将亡值、纯右值三类，纯右值包括了：非引用返回临时变量、运算表达式产生的临时变量、原始常量值、lambda 表达式等。





















































## 简介

















### 左值 VS. 右值



## 绑定规则

使用时，左值引用只能绑定左值，右值引用只能绑定右值，如果绑定的不对，那么在编译期间就会直接报错。

注意，常量左值引用比较特殊，可以作为一个 "万能" 引用类型，可以绑定非常量左值、常量左值、右值，而且在绑定右值的时候，常量左值引用还可以像右值引用一样将右值的生命期延长，缺点是，只能读不能改。

常量左值经常会在不经意的场景下使用，例如复制构造函数中。

{% highlight cpp %}
#include <iostream>

class Copyable {
public:
        Copyable(){}
        Copyable(const Copyable &c) {
                std::cout << "Copyied" << std::endl;
        }
};

Copyable CreateRValue()
{
        return Copyable(); //
}
void AcceptValue(Copyable c) { }
void AcceptReference(const Copyable &c) { }

int main(void)
{
        std::cout << "Passed by value:" << std::endl;
        AcceptValue(CreateRValue());

        std::cout << "Passed by reference:" << std::endl;
        AcceptReference(CreateRValue());

        return 0;
}
{% endhighlight %}

正常 `AcceptValue()` 会有调用两次复制构造函数，而 `AcceptReference()` 会调用一次，实际上任何一个函数都没有调用复制构造函数。

这是因为，编译器默认会开启 RVO/NRVO 优化，例如，当编译器发现 `CreateRValue()` 内部生成了一个对象，返回时要生成一个临时对象，而该对象又赋值给了一个形参，那么这三个变量完全可以用一个变量替代，这样就不需要调用拷贝构造函数了。

测试时可以通过 `-fno-elide-constructors` 参数取消优化。

### 总结

如下的 T 是一个具体类型。

* 左值引用，通过 `T&` 定义，只能绑定左值；
* 右值引用，通过 `T&&` 定义，只能绑定右值；
* 常量左值，通过 `const T&` 定义，既可以绑定左值又可以绑定右值；
* 已命名的右值引用，指向右值，但本身是一个左值；
* 编译器有返回值优化，但不要过于依赖。

``` cpp
#include <cstdlib>
#include <cstring>
#include <iostream>

class Person {
private:
        int id;
        char *name;

public:
        // Destructor
        ~Person() {
                std::cout << "Destructor" << std::endl;
                if (name == NULL)
                        return;
                free(name);
        }

        // Default Constructor
        Person(): id(0), name(NULL) {
                std::cout << "Default Constructor" << std::endl;
        }

        // Simple Constructor
        Person(int _id, const char *_name) {
                std::cout << "Simple Constructor" << std::endl;

                id = _id;
                if (_name == NULL)
                        return;
                name = strdup(_name);
        }

        // Copy Constructor
        Person(const Person &p) {
                std::cout << "Copy Constructor" << std::endl;

                id = p.id;
                if (p.name == NULL)
                        return;
                name = strdup(p.name);
        }

        // Copy-Assignment Operator
        Person &operator=(const Person &p) {
                std::cout << "Copy-Assignment Operator" << std::endl;
                if (this == &p) /* self */
                        return *this;

                id = p.id;
                if (name != NULL)
                        free(name);
                name = strdup(p.name);
                return *this;
        }

        // Move Constructor
        Person(Person &&p) {
                std::cout << "Move Constructor" << std::endl;

                id = p.id;
                name = p.name;
                p.name = NULL;
        }

        // Move-Assignment Operator
        Person &operator=(Person &&p) {
                std::cout << "Move-Assignment Operator" << std::endl;
                if (this == &p) /* self */
                        return *this;

                id = p.id;
                if (name != NULL)
                        free(name);
                name = p.name;
                p.name = NULL;
                return *this;
        }

        void String(void) {
                if (name != NULL)
                        std::cout << name << std::endl;
                else
                        std::cout << "empty" << std::endl;
        }
};

Person CreatePerson(const char *name)
{
        return Person(1, name);
}

int main(void)
{
        Person x(1, "Hello World"); // <Simple Constructor>

        Person a(x);                       // line #1
        Person b(CreatePerson("Foobar"));  // line #2

        Person c;     // <Default Constructor>
        c = x;        // <Copy-Assignment Operator>
        x.String();   // Hello World

        std::cout << "--------------------------" << std::endl;

        Person y(1, "Hi World");   // <Simple Constructor>
        Person z(1, "Hello Hell"); // <Simple Constructor>

        Person d;          // <Default Constructor>
        d = std::move(y);  // <Move-Assignment Operator>
        y.String();        // empty

        Person e(std::move(z));   // <Move Constructor>
        z.String();               // empty

        return 0;
}
```




```
#include <iostream>

class Copyable {
public:
        Copyable(){}
        Copyable(const Copyable &c) {
                std::cout << "Copyied" << std::endl;
        }
};

Copyable CreateRValue()
{
        return Copyable(); //
}
void AcceptValue(Copyable c) { }
void AcceptReference(const Copyable &c) { }

int main(void)
{
        std::cout << "Passed by value:" << std::endl;
        AcceptValue(CreateRValue());

        std::cout << "Passed by reference:" << std::endl;
        AcceptReference(CreateRValue());

        return 0;
}
```





模板、智能指针

swap使用
http://dengzuoheng.github.io/cpp-swap


类型推导
https://zhuanlan.zhihu.com/p/54191444
C++异常机制分析
https://www.cnblogs.com/QG-whz/p/5136883.html

为什么建议你用nullptr而不是NULL
https://zhuanlan.zhihu.com/p/79883965
https://www.cnblogs.com/porter/p/3611718.html

彻底理清重载函数匹配
https://www.yanbinghu.com/2019/01/01/6209.html
赋值运算符重载
https://www.cnblogs.com/zpcdbky/p/5027481.html

new和malloc的区别
https://www.cnblogs.com/qg-whz/p/5140930.html

C/C++编译区别
https://www.cnblogs.com/diegodu/p/4580474.html

在 C++11 中引入了 `nullptr` 关键字，用来标识空指针，其类型为 `std::nullptr_t` ，用来替换 C 语言中的 NULL 。


在如上的编译时，使用如下参数。

g++ -std=c++11 -o shape shape.cpp -Wall -fno-elide-constructors

#### 编译器优化

现在大部分的编译器是支持 Return Value Optimization, RVO 或者 Named Return Value Optimization, NRVO 也就是返回值的优化，可以减少对复制构造函数的调用次数。

需要注意的几个点。

#### 赋值构造函数

也就是 `Rectangle(const Rectangle &r)` 函数，其中入参为 `const` 

#### 移动构造函数

参数中建议不要使用 `const` ，因为，在将原参数赋值给新对象之后，会将原对象重置，例如指针设置为 `nullptr`、文件句柄设置为 `-1` 等等。

注意 `Rectangle r4 = r2;` 以及 `r5 = r2;` 两个语句的区别，其中前者是在声明的时候进行赋值，调用的是赋值构造函数；而后者，是赋值语句，调用的是赋值操作符。

int&  b = 1; // ERROR 常量是右值，不能使用左值引用
int&& b = 1; // OK

int a = 1;
int&& b = a; // ERROR 不能将一个左值复制给一个右值引用

在 C++ 中，如果不存在，会默认创建四类特殊的函数：默认构造函数 (Default Constructor)、复制构造函数 (Copy Constructor)、复制赋值运算符 (Copy-Assignment Operator)、析构函数 (Destructor) ，通过这些默认的函数，可以保证 C++ 与 C 的语义相同。

另外，在 C++11 中，又引入了 move 的语义，同时默认会添加了 Move Constructor 以及 Move-Assignment Operator 。

https://docs.microsoft.com/en-us/cpp/cpp/explicitly-defaulted-and-deleted-functions?view=vs-2019

### 原因

right hand side, rhs

C++11引入的新特性
https://coolshell.cn/articles/5265.html

## 移动语义

假设，在 `Person` 类中管理了一个 C 语言的 `char *name` 字符串，默认的是浅拷贝，如果使用这种方式，必须要保证只有当所有对象都销毁之后，才可以释放该字符串指向的内存，否则可能会导致指针访问异常。

在管理类内部动态申请的对象时，通常采用的是深拷贝，这样根据三法则，就需要实现对应的析构函数、复制构造函数、复制赋值运算符。

------

其中 `line #1` 进行深度复制是有必要的，因为在 `a` 中会使用到 `x` 中的值，如果 `a` 因为 `x` 改变，那么显然不符合预期，这里的 `x` 也被称为左值。

而 `line #2` 中，函数 `CreatePerson()` 返回的是一个右值，如果编译器不执行返回值优化，那么会调用两次复制构造函数，因为中间会涉及到字符串的复制，而且临时变量还会销毁，导致不必要的资源开销。

所以，对于复制，可以通过移动构造函数减少字符串的复制。

注意，其中移动构造函数是可选的，如果没有，那么会使用复制构造函数替换；如果想优化性能，那么可以通过移动构造函数降低对不必要的资源开销。

### 左值转换为右值

对于左值会调用拷贝构造函数，如果某个左值是局部变量，其生命周期很短，那么可以通过 `std::move()` 将左值转换为右值，也就是明确告诉编译器，虽然我是一个左值，但不要使用拷贝构造函数，而是采用移动构造函数。

例如，如上的代码可以增加如下内容，显示使用右值，也就是移动构造函数、移动赋值操作符。

需要注意如下几点：

1. 虽然 `d = std::move(y)` 将 `y` 中的 `name` 资源赋值给了 `d` ，但是 `y` 并没有立即析构，只有当 `y` 离开作用域之后才会析构。
2. 如果不存在移动构造函数，那么 `std::move()` 会失效但不会发生错误，这是因为找不到移动构造函数后，会找到复制构造函数，这也就是为什么复制构造函数入参为 `const T&` 的原因。

## 通用引用 (Universal Reference)


<!--
https://www.cnblogs.com/qicosmos/p/4283455.html
https://liam.page/2016/12/11/rvalue-reference-in-Cpp/
https://www.cnblogs.com/Braveliu/p/6220906.html
https://juejin.im/post/59c3932d6fb9a00a4b0c4f5b
https://www.ibm.com/developerworks/cn/aix/library/1307_lisl_c11/index.html
https://blog.csdn.net/xiaolewennofollow/article/details/52559306


https://fzheng.me/2018/05/28/termdebug/
https://fzheng.me/2016/08/11/cpp11-multi-thread/

https://blog.csdn.net/xiaolewennofollow/article/details/52559364
https://blog.csdn.net/wangshubo1989/article/details/50357549
https://www.cnblogs.com/moodlxs/p/10111628.html

http://www.pandademo.com/2017/08/thread-safety-of-shared_ptr-and-weak_ptr/
http://blog.leanote.com/post/shijiaxin.cn@gmail.com/C-shared_ptr
https://www.jianshu.com/p/b6ac02d406a0
https://blog.csdn.net/peng864534630/article/details/77932574
-->

{% highlight text %}
{% endhighlight %}
