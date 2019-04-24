---
title: GCC 常用技巧
layout: post
comments: true
language: chinese
usemath: true
category: [linux,program]
keywords: linux,program,gcc
description: Clang 是一个 C++ 编写，基于 LLVM 的 C/C++、Objective-C 语言的轻量级编译器，在 2013.04 开始，已经全面支持 C++11 标准。
---

Clang 是一个 C++ 编写，基于 LLVM 的 C/C++、Objective-C 语言的轻量级编译器，在 2013.04 开始，已经全面支持 C++11 标准。

<!-- more -->

![clang logo]({{ site.url }}/images/programs/clang_logo.png "clang logo"){: .pull-center }

## pragma

```#pragma``` 宏定义在本质上是声明，常用的功能就是注释，尤其是给 Code 分段注释；另外，还支持处理编译器警告。

{% highlight c %}
#pragma clang diagnostic push

//----- 方法弃用告警
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//----- 不兼容指针类型
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
//----- 未使用变量
#pragma clang diagnostic ignored "-Wunused-variable"
//----- 无返回值
#pragma clang diagnostic ignored "-Wmissing-noreturn"
//... ...

#pragma clang diagnostic pop
{% endhighlight %}

## \_\_attribute\_\_

### \_\_attribute\_\_((format))

该属性用于自实现的字符串格式化参数添加类似 printf() 的格式化参数的校验，判断需要格式化的参数与入参是否相同。

{% highlight text %}
format (archetype, string-index, first-to-check)

__attribute__((format(printf,m,n)))
__attribute__((format(scanf,m,n)))
  m : 第m个参数为格式化字符串(从1开始)；
  n : 变长参数(也即"...")的第一个参数排在总参数的第几个；
{% endhighlight %}

如下是使用示例。

{% highlight text %}
void myprint(const char *format,...) __attribute__((format(printf,1,2)));
void myprint(int l，const char *format,...) __attribute__((format(printf,2,3)));
{% endhighlight %}

如下是一个简单的使用示例。

{% highlight c %}
#include <stdio.h>

extern void myprint(const char *format,...) __attribute__((format(printf,1,2)));

int myprint(char *fmt, ...)
{
    int result;
    va_list args;
    va_start(args, fmt);
    fputs("foobar: ", stderr);
    result = vfprintf(stderr, fmt, args);
    va_end(args);
    return result;
}
int main(int argc, char **argv)
{
    myprint("i=%d\n",6);
    myprint("i=%s\n",6);
    myprint("i=%s\n","abc");
    myprint("%s,%d,%d\n",1,2);
 return 0;
}
{% endhighlight %}

编译时添加 `-Wall` 就会打印 Warning 信息，如果去除，实际上不会显示任何信息，通常可以提前发现常见的问题。

### \_\_attribute\_\_((constructor))

这是 GCC 的扩展机制，通过上述的属性，可以使程序在开始执行或停止时调用指定的函数。

```__attribute__((constructor))``` 在 main() 之前执行，```__attribute__((destructor))``` 在 main() 执行结束之后执行。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

static  __attribute__((constructor)) void before()
{
    printf("Hello World\n");
}

static  __attribute__((destructor)) void after()
{
    printf("Bye World!\n");
}

int main(int args,char ** argv)
{
    printf("Live...\n");
    return EXIT_SUCCESS;
}
{% endhighlight %}

如果有多个函数，可以指定优先级，其中 0~100 (含100)系统保留。在 main 之前顺序为有小到大，退出时顺序为由大到小。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

static  __attribute__((constructor(102))) void before102()
{
    printf("Hello World 102\n");
}

static  __attribute__((destructor(102))) void after102()
{
    printf("Bye World! 102\n");
}

static  __attribute__((constructor(101))) void before101()
{
    printf("Hello World 101\n");
}

static  __attribute__((destructor(101))) void after101()
{
    printf("Bye World! 101\n");
}

int main(int args,char ** argv)
{
    printf("Live...\n");
    return EXIT_SUCCESS;
}
{% endhighlight %}

在使用时也可以先声明然再定义


{% highlight c %}
#include <stdio.h>
#include <stdlib.h>

void before() __attribute__((constructor));
void after() __attribute__((destructor));

void before()
{
    printf("Hello World\n");
}

void after()
{
    printf("Bye World!\n");
}

int main(int args,char ** argv)
{
    printf("Live...\n");
    return EXIT_SUCCESS;
}
{% endhighlight %}

### \_\_attribute\_\_((visibility))

程序调用某个函数 A，而 A 函数存在于两个动态链接库 liba.so 和 libb.so 中，并且程序执行需要链接这两个库，此时程序调用的 A 函数到底是来自于 a 还是 b 呢？

这取决于链接时的顺序，首先链接的库会更新符号表，比如先链接 liba.so，这时候通过 liba.so 的导出符号表就可以找到函数 A 的定义，并加入到符号表中，而不会再查找 libb.so 。

也就是说，这里的调用严重的依赖于链接库加载的顺序，可能会导致混乱。

gcc 的扩展中有如下属性 `__attribute__ ((visibility("hidden")))` 可以用于抑制将一个函数的名称被导出，对连接该库的程序文件来说，该函数是不可见的，使用的方法如下：

<!--
-fvisibility=default|internal|hidden|protected
gcc的visibility是说，如果编译的时候用了这个属性，那么动态库的符号都是hidden的，除非强制声明。
-->

#### 1. 创建一个c源文件

{% highlight c %}
#include<stdio.h>
#include<stdlib.h>

__attribute ((visibility("default"))) void not_hidden()
{
    printf("exported symbol\n");
}

void is_hidden()
{
    printf("hidden one\n");
}
{% endhighlight %}

想要做的是，第一个函数符号可以被导出，第二个被隐藏。

#### 2. 生成动态库

先编译成一个动态库，使用到属性 `-fvisibility` 。

{% highlight text %}
----- 编译
$ gcc -shared -o libvis.so -fvisibility=hidden foobar.c

----- 查看符号链接
# readelf -s libvis.so |grep hidden
 7: 0000040c 20 FUNC GLOBAL DEFAULT 11 not_hidden
48: 00000420 20 FUNC LOCAL  HIDDEN  11 is_hidden
51: 0000040c 20 FUNC GLOBAL DEFAULT 11 not_hidden
{% endhighlight %}

可以看到，属性确实有作用了。

#### 3. 编译链接

现在试图链接程序。

{% highlight c %}
int main()
{
    not_hidden();
    is_hidden();
    return 0;
}
{% endhighlight %}

试图编译成一个可执行文件，链接到刚才生成的动态库。

{% highlight text %}
$ gcc -o exe main.c -L ./ -lvis
/tmp/cckYTHcl.o: In function `main':
main.c:(.text+0x17): undefined reference to `is_hidden'
{% endhighlight %}

说明了 hidden 确实起到作用了。

### \_\_attribute\_\_((sentinel))

该属性表示，此可变参数函数需要一个 `NULL` 作为最后一个参数，这个 `NULL` 参数一般被叫做 "哨兵参数"。例如，有如下程序：

{% highlight c %}
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

void foo(char *first, ...)
{
    char *p = (char *)malloc(100), *q = first;

    va_list args;
    va_start(args, first);
    while (q) {
        strcat(p, q);
        q = va_arg(args, char *);
    }
    va_end(args);

    printf("%s\n", p);

    free(p);
}

int main(void)
{
    foo("Hello", "World");

    return 0;
}
{% endhighlight %}

当通过 `gcc main.c -Wall` 进行编译时，会发现没有任何警告，不过很显然，调用 `foo()` 时最后一个参数应该是个 `NULL` 以表明 "可变参数就这么多"。

编译完成后，如果尝试运行则会打印一些乱码，显然是有问题的。

正常来说，应该通过如下方式调用 `foo("Hello", "World", NULL);`，为此，就需要用到了上述的属性，用于表示最后一个参数需要为 `NULL` 。

{% highlight c %}
void foo(char *first, ...) __attribute__((sentinel));
{% endhighlight %}

这样再不写哨兵参数，在编译时编译器就会发出警告了。


但是，对于同样使用可变参数的 `printf()` 来说，为什么就不需要哨兵属性，实际上，通过第一个参数就可以确定需要多少个参数，如下。

{% highlight c %}
/*
 * 第一个参数中规定了有两个待打印项，所以打印时会取 "string" 和 1，多写的 "another_string" 会被忽略。
 * printf()在被调用时明确知道此次调用需要多少个参数，所以也就无需哨兵参数的帮忙。
 */
printf("%s %d\n", "string", 1, "another_string");
{% endhighlight %}




{% highlight text %}
{% endhighlight %}
