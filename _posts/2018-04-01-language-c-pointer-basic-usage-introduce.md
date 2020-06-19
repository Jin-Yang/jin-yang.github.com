---
title: C 指针简介
layout: post
language: chinese
category: [program]
keywords: align
description:
---

在 C 语言中，指针是一个特殊的变量，存储值指向内存中的一个地址，一个指针包含了四方面的内容：指针类型、指向类型，指针所指向的内存区以及指针本身所占据的内存区。

指针或许是 C 语言中最复杂的东西了。

<!-- more -->

## 简介

指针大小是指指针变量在内存中所占存储空间的大小，这与机器的位数相关，例如 32 位是 4 字节，64 位是 8 字节等等，这也就是为什么所有的指针都可以通过 `void *` 进行转换。

而指针所指对象的大小，是指指针指向对象的内存空间的大小，如 `int *` 所指对象为 `int` 类型，大小为 4 字节。

这里以 `int` 类型简单介绍，其它类型可以类推。

{% highlight text %}
int p;            // 整型变量
int *p;           // 整型指针
int p[3];         // 整型数组
int *p[3];        // 整型指针数组，结合顺序为[]+*，先是一个数组，存储了整型指针的数组
int (*p)[3];      // 整型数组指针，结合顺序为*+[]，先是一个指针，指向的是保存了3个整型元素数组
int **p;          // 二级指针
int p(int);       // 函数声明，含一个int参数
Int (*p)(int);    // 函数指针，结合顺序为*+()，先是一个指针，指针指向的是一个函数
{% endhighlight %}


<!--
`int (*p[10])(int*)` 指针数组，数组元素是 `int (*p)(int *)` 的函数指针。
`int *p[10]` 指针数组，`[]` 优先级高于 `*`，首先是一个数组，数组元素为 `int *` 。
`int (*p)[10]` 数组指针，`p` 先与 `*` 结合表示是一个指针，指向的是一个 `int` 类型，且大小为 10 列的数组。
`int*(*p)[10]` 与上类似，只是保存的是 `int *` 类型指针。
int *(*p(int))[3]; //可以先跳过,不看这个类型,过于复杂从P 开始,先与()结合,说明P 是一个函数,然后进入()里面,与int 结合,说明函数有一个整型变量参数,然后再与外面的*结合,说明函数返回的是一个指针,,然后到最外面一层,先与[]结合,说明返回的指针指向的是一个数组,然后再与*结合,说明数组里的元素是指针,然后再与int 结合,说明指针指向的内容是整型数据.所以P 是一个参数为一个整数据且返回一个指向由整型指针变量组成的数组的指针变量的函数
-->

### 指针操作

在指针进行运算的时候，与指针指向对象的大小密切相关，指针的加减操作是根据其指向类型的大小进行运算，需要注意的是对 `void *` 指针的操作，是按照与 `char *` 类似的规则进行的。

{% highlight c %}
#include <stdio.h>

int main(void)
{
	char chars[] = {1, 2, 3, 4, 5, 6, 7, 8};
	char *chptr = chars;

	int ints[] = {1, 2, 3, 4, 5, 6, 7, 8};
	int *intptr = ints;

	void *ptr = chars;

	printf("%p %p\n", chptr, chptr + 1);
	printf("%p %p\n", intptr, intptr + 1);
	printf("%p %p\n", ptr, ptr + 1);
	printf("%p %p\n", ptr, ((int *)ptr) + 1);

	return 0;
}
{% endhighlight %}

输出的结果为。

{% highlight text %}
0x7ffccadeea60 0x7ffccadeea61
0x7ffccadeea40 0x7ffccadeea44
0x7ffccadeea60 0x7ffccadeea61
0x7ffccadeea60 0x7ffccadeea64
{% endhighlight %}

另外，需要注意的是，像 `++` 这类单目运算的使用规则，只能操作变量，不能是常量或者表达式，例如 `((int *)ptr)++` 在编译阶段就会报错 `lvalue required as increment operand` 。

如下也是类似的场景。

{% highlight text %}
char arr[] = "Hi!";
arr++; // ERROR

char *ptr = arr;
ptr++; // OK
{% endhighlight %}

其中 `arr` 是 `char []` 类型，表示数组的首地址和第一个元素地址，无法直接通过 `++` 取移动。另外，`++a--` `(++a)--` `a++ + ++b` 也是一样的报错。

## 指针

### 指针常量 VS. 常量指针

前面是一个修饰词，后面的是中心词。

#### 常量指针

**常量指针** 首先是一个指针，指向的是常量，即指向常量的指针；可以通过如下的方式定义：

{% highlight c %}
const int a = 7;
const int *p = &a;
{% endhighlight %}

对于常量，我们不能对其内容进行修改；指针的内容本身是一个地址，通过常量指针指向一个常量，为的就是防止我们写程序过程中对指针误操作出现了修改常量这样的错误，如果我们修改常量指针的所指向的空间的时候，编译系统就会提示我们出错信息。

在 C 语言中，通常定义的字符串会返回一个常量指针，因此字符串不能赋值给字符数组，只能赋值到指针。

总结一下，<font color="red">常量指针就是指向常量的指针，指针所指向的地址的内容是不可修改的，指针本身的内容是可以修改的</font> 。


#### 指针常量

**指针常量**  首先是一个常量，再才是一个指针；可以通过如下的方式定义：

{% highlight c %}
int a = 7;
int * const p = &a; // OR int const *p = &a;
{% endhighlight %}

常量的性质是不能修改，指针的内容实际是一个地址，那么指针常量就是内容不能修改的常量，即内容不能修改的指针，指针的内容是什么呀？指针的内容是地址，所以，说到底，就是不能修改这个指针所指向的地址，一开始初始化，指向哪儿，它就只能指向哪儿了，不能指向其他的地方了，就像一个数组的数组名一样，是一个固定的指针，不能对它移动操作。

它只是不能修改它指向的地方，但这个指向的地方里的内容是可以替换的，这和上面说的常量指针是完全不同的概念。

作一下总结，<font color="red">指针常量就是是指针的常量，它是不可改变地址的指针，但是可以对它所指向的内容进行修改</font> 。

源码可以参考 [github const_pointer.c]({{ site.example_repository }}/c_cpp/c/const_pointer.c) 。

### 与一维数组

假设有如下数组，

{% highlight c %}
int Array[] = {1, 2, 3, 4};
int *ptr = Array;
{% endhighlight %}

其中 Array 为指针常量，而 ptr 为指针变量，且 ```ptr = &Array[0]```，那么如下的操作相同 ```ptr[i] <=> *(ptr+i)``` 以及 ```Array[i] <=> *(Array + i)``` 。

如下，简单介绍下常见操作。

#### *ptr++

由于 ```*``` 和 ```++``` 优先级相同，结合性为由右至左，即 ```*ptr++``` 等价于 ```*(ptr++)``` ，由于 ```++``` 为后继加，所以当得到 ```*ptr``` 后再处理 ```++```；所以 ```*ptr++``` 等于 1，进行此项操作后 ```*ptr``` 等于 2。

执行的步骤为 1) ```++``` 操作符产生 ptr 的一份拷贝；2) ```++``` 操作符增加 ptr 的值；3) 在 ptr 上执行间接访问操作。

#### ++*ptr

利用优先级和结合性可得，```++*ptr``` 等价于 ```++(*ptr)``` ，此时 ```Array[0]``` 为 2，返回 2 。

#### *ptr++

利用优先级和结合性可得，```*ptr++``` 等价于 ```*(ptr++)``` ，返回 1，ptr 值加 1 。


## 函数指针

通过函数指针，允许程序在不同的状态时，调用不同的函数，而又不需要改变当前的处理逻辑。函数的调用方式如下。

{% highlight c %}
#include <stdio.h>

void foobar(int);

int main(void)
{
        foobar(100);
        return 0;
}

void foobar(int v)
{
        printf("got %d\n", v);
}
{% endhighlight %}

如上，是一个简单的函数声明及其实现，其中 `foobar()` 是一个函数，代表了一段代码的实现。那么函数名的作用是什么？

一个数据变量的内存地址可以存储在相应的指针变量中，函数的首地址也以存储在某个函数指针变量中，然后就可以通过这个函数指针变量来调用所指向的函数了。

在 C 语言中，任何一个变量需要先声明，然后才能使用，函数指针变量同样也应该要先声明。

{% highlight c %}
//----- 函数指针变量声明，函数实现的参数、返回值需要相同
void (*foobar)(int);
{% endhighlight %}

与函数声明基本类似，只是将 `foobar` 替换成了 `(*foobar)` 而已，这样它就成为了一个指针。

### 使用方法

函数指针有如下的几种使用方法。

{% highlight c %}
#include <stdio.h>

void foobar(int);

int main(void)
{
        void (*foo)(int);

        foo = foobar;
        foo(1);
        (*foo)(2);

        foo = &foobar;
        foo(1);
        (*foo)(2);

        (*foobar)(3);

        return 0;
}

void foobar(int v)
{
        printf("got %d\n", v);
}
{% endhighlight %}

总结如下。

* `foobar` 函数名与 `foo` 函数指针一样，都是函数指针；只是前者为常量，后者为变量。
* 函数调用可以以 `(*foobar)(10)` 方式使用，但不方便，所以允许以 `foobar(1)` 的方式调用；这也就意味着函数指针的操作相同。
* 函数变量赋值时，可以写成 `foo=foobar` 的形式，也可以写成 `foo=&foobar` 的形式。
* 在声明时 `void foobar(int)` 和 `void (*foobar)(int)` 不能互换，因为这是两种不同的方式。
* 函数指针变量也可以存入一个数组内，数组的声明方法为 `int (*foobars[10])(int)` 。


<!--
函数指针
https://www.cnblogs.com/windlaughing/archive/2013/04/10/3012012.html
-->


{% highlight text %}
{% endhighlight %}
