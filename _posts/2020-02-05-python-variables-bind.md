---
title: Python 变量基本概念详解
layout: post
comments: true
language: chinese
tag: [Python, Program]
keywords: Python, 变量, 绑定, bind
description: 在 Python 中，所有的都是对象，变量与大部分常用语言有所区别，赋值的时候，严格来说不是赋值，而是变量的绑定，变量名称中并没有保存对象，而更像是在对象上添加了一个标签而已。这里简单介绍下 Python 中与变量相关的基本概念。
---

在 Python 中，所有的都是对象，变量与大部分常用语言有所区别，赋值的时候，严格来说不是赋值，而是变量的绑定，变量名称中并没有保存对象，而更像是在对象上添加了一个标签而已。

这里简单介绍下 Python 中与变量相关的基本概念。

<!-- more -->

## 简介

在 Python 中，有一个原则：**一切皆对象** 。例如最简单的整型、浮点数、字典、列表、元组等类型都是对象，而实际上，类、函数、模块等等，在 Python 中也都是对象。

对象是 Python 中对数据的抽象，每个对象都包含了：

* 身份标识，在对象创建后不再改变，可以通过 `id()` 查看，其中 `is` 对比的就是这里的标识，实际上，在 CPython 中的实现使用的是对象在内存中的地址。
* 类型，同样不可改变，一般决定了对象所支持的操作以及值的类型，可以通过 `type()` 函数返回对象的类型。
* 值，包含了：A) 不可变对象，例如数字、字符串、元组等；B) 可变对象，例如字典、列表等。

可以通过如下方式查看。

```
>>> val = 2
>>> id(val)
140404552979712
>>> type(val)
<class 'int'>
```

在使用对象时，无需显示销毁，可以直接通过垃圾回收清理资源，在 CPython 中使用计数模式进行探测，但是不能保证循环引用的垃圾回收。

### 名字 VS. 变量

几乎所有语言都有 "变量" 这一概念，Python 也同样存在，不过，严格来说，称之为名字更为贴切。

#### 其它语言变量

大部分的语言使用 `int a = 1` 代表着将值 `1` 放到了 `a` 这个盒子里。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-01.png "python variables bind method")

现在盒子里有了一个 `1` 整数；如果重新赋值 `a = 2` 这个整数，那么变量就被替换掉了。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-02.png "python variables bind method")

如果使用 `int b = a` 进行赋值，那么整型变量 `2` 就放到了一个新盒子中。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-02.png "python variables bind method")

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-03.png "python variables bind method")

也就是说，现在存在 `a` `b` 两个盒子，都装有整数 `2` 的拷贝，注意，是每个盒子都有一个单独的拷贝。

#### Python 的名字

而在 Python 中，是变量的名字或者标识符将标签一样捆绑到对应的对象上，同样是上述的 `a = 1` 初始化操作。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-04.png "python variables bind method")

也就是整数 `1` 对象有个叫做 `a` 的标签，如果重新给 `a` 赋值，只是将标签移动到另一对象上，而原先的 `1` 将不再有标签。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-05.png "python variables bind method")

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-06.png "python variables bind method")

此时 `1` 可能仍然存在 (没有被垃圾回收) ，只是名字 `a` 不再贴到对象 `1` 上而以，也就是无法通过 `a` 来访问 `1` 了。

如果，将一个名字赋值给另外一个名字，例如 `b = a` ，也只是再次将另一个名字标签捆绑到存在的对象上。

![python variables bind method](/{{ site.imgdir }}/python/python-variables-bind-method-07.png "python variables bind method")

也就是说，现在整数对象 `2` 被绑定到了两个不同的标签上而已。

### 总结

在官方文档 [Simple statements - Assignment statements](https://docs.python.org/3/reference/simple_stmts.html#assignment-statements) 中，有对变量赋值的基本定义，摘抄如下。

> Assignment statements are used to (re)bind names to values and to modify attributes or items of mutable objects.

也就是说，赋值语句用来将名字绑定或者重绑定给值，也用来修改可变对象的属性。

<!--
## 可变 VS. 不可变变量
-->

## 其它

在当前代码块的任意位置可以声明变量，而且该变量会在整个代码块中可见，但是，需要在使用变量之前完成绑定，否则会抛出 `UnboundLocalError` 的异常。

``` python
def foobar():
	print(val)
	val = 5   # <1>
foobar()
```

直接执行会报如下的错误。

```
$ python3 foobar.py
Traceback (most recent call last):
  File "foobar.py", line 4, in <module>
    foobar()
  File "foobar.py", line 2, in foobar
    print(val)
UnboundLocalError: local variable 'val' referenced before assignment
```

注意，如果没有 `<1>` 中的赋值语句，那么会报下面的错误。

```
$ python3 foobar.py
Traceback (most recent call last):
  File "foobar.py", line 3, in <module>
    foobar()
  File "foobar.py", line 2, in foobar
    print(val)
NameError: name 'val' is not defined
```

也就是上述所述，代码块任意位置的声明，在整个代码块的命名空间中都有效，但需要保证在使用前先绑定。

当然，在上述的示例中，如果全局命名空间中有变量的赋值也是可以的，例如如下示例。

``` python
val = 10
def foobar():
	global val
	print(val)
	val = 5
foobar()
```
