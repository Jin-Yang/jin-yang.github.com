---
title: 详细介绍 Python 闭包以及装饰器
layout: post
comments: true
language: chinese
tag: [Python, Program]
keywords: 闭包, closure, 装饰器, decorator
description: 装饰器可以为 Python 提供一些修饰工作，所谓修饰工作就是想给现有的模块加上一些小功能，但又不让这个小功能侵入到原有的模块中的代码里去，这里详细介绍其使用方式。
---

装饰器可以为 Python 提供一些修饰工作，所谓修饰工作就是想给现有的模块加上一些小功能，但又不让这个小功能侵入到原有的模块中的代码里去，最常见的三种函数修饰器 `@staticmethod`、`@classmethod` 和 `@property` ，都是 Python 的内置函数。

这里详细介绍其使用方式。

<!-- more -->

## @ 符号

在介绍修饰符之前先介绍下 `@` 符号的使用。

``` python
def foobar(func):
        print("before invoke...")
        func()
        print("after invoke...")
        return "FooBarFunc|"

@foobar
def hello(arg="World"):
        print("Hello", arg)
        return "HelloFunc|"

print("====> Start here")  # <1>
print(hello)
print(foobar)
#print(hello("hi")) # TypeError: 'str' object is not callable
```

然后执行的输出如下。

```
before invoke...
Hello World
after invoke...
====> Start here
FooBarFunc|
<function foobar at 0x7f6c7145be18>
```

正常来说，在 `<1>` 之前是不会有应用的代码执行的，但是上面实际上是有执行，关键在于上述 `@foobar` 的作用，实际等价于 `hello = foobar(hello())` ，也就是上面 `Start here` 之前的输出。

在执行到 `<1>` 之后，`hello` 符号绑定的已经不再是一个函数，而是字符串了，所以，在执行 `hello("hi")` 时会报错。

### 使用类

因为上面的 `hello` 符号已经确定，作为一个字符串也没有其它太好的用处。

``` python
class FooBar(object):
        def __init__(self, func):
                print("in __init__()")
                self.func = func
        def __call__(self):
                print("in __call__()")
                self.func()

@FooBar
def hello():
        print("in hello()")

print("====> Start here")
hello()
print(type(hello))
```

其输出为。

```
in __init__()
====> Start here
in __call__()
in hello()
<class '__main__.FooBar'>
```

其中 `hello()` 会被初始化为一个 `FooBar` 实例，并通过重载调用运算符 `__call__` ，这样只是开上去仍然是一个函数调用，实际上调用的是类的一个方法。

在实现的 `__call__` 方法中，就可以做一些定制化的操作了。

### 使用函数

上述定义类的方式难免有些重，可以使用等价的内部函数，使用效果等价。

``` python
def foobar(func):
        print("before enclosed function...")
        def wrapper():
                print("before invoke...")
                func()
                print("after invoke...")
        return wrapper

@foobar
def hello():
        print("Hello World")

print("====> Start here")
hello()
print(type(hello))
```

输出的内容如下。

```
before enclosed function...
====> Start here
before invoke...
Hello World
after invoke...
<class 'function'>
```

这也是最常用的方式，可以在 [PythonDecoratorLibrary](http://wiki.python.org/moin/PythonDecoratorLibrary) 中有些常见的使用方法，比如自动添加属性、重试逻辑等等。

## 闭包

最后一种方式，实际上使用的就是闭包，只有掌握闭包才可以更好理解并编写装饰器，所以，这里先介绍下闭包的基本概念。

在 Python 中定义了四种作用域：局部 (Local)、嵌套 (Enclosing)、全局 (Global)、内置 (Build-in)，详细可以查看 [Python 作用域](/post/python-namespace-scope.html) 中的相关介绍，其中的嵌套作用域就与这里的闭包相关，而且有点像为闭包量身定做。

``` python
def foobar():
    var = "free variable"
    def wrapper():
        print(var)
    return wrapper
foobar()()
```

上述的 `var` 就在所谓的嵌套作用域，而嵌套函数就是闭包，所谓闭包，是指延伸了作用域的函数，在函数中能够访问未在函数定义体中定义的非全局变量 (一般在嵌套函数中出现) 。

其中变量 `var` 会有一个专业名字，叫做自由变量。

### 实现原理

自由变量的名称以及对应的值，可以通过如下方式查看。

``` python
>>> def foobar():
...     var = "free variable"
...     def wrapper():
...         print(var)
...     return wrapper
...
>>> closure = foobar()
>>> closure.__code__.co_freevars   # 名称在字节码中保存
('var',)
>>> closure.__closure__            # 保存自由的cell序列
(<cell at 0x7f068636edc8: str object at 0x7f068463a4b0>,)
>>> closure.__closure__[0].cell_contents   # 对象的值
'free variable'
```

注意，自由变量必须是可变对象 (如列表、字典等)，否则只能读取无法更新。

## 高级技巧

### 参数装饰器

一个接受参数的装饰器其调用逻辑如下，假设有如下的装饰器。

```
@decorator(x, y, z)
def func(a, b):
	pass
```

装饰器处理过程跟下面的调用是等效的。

```
def func(a, b):
	pass
func = decorator(x, y, z)(func)
```

所以 `decorator(x, y, z)` 的返回结果必须是一个可调用对象，它接受一个函数作为参数并包装它。

### 添加参数

如果要使用参数，如下是最简单的方式。

``` python
def foobar(func):
        def wrapper(arg="World"):
                return func(arg)
        return wrapper

@foobar
def hello(arg):
        print("Hello", arg)

hello()
hello("Python")
```

注意，如果有默认参数，应该添加到 `def wrapper()` 函数中，而不是 `def hello()` 函数，否则在执行 `hello()` 函数时会报错。

### 多参数

上面只能使用一个参数，那么当被修饰的函数 (例如上面的 `hello()`) 有多个参数时，那么就必须要在修改 `wrapper()` 的定义，很麻烦。

可以使用 Python 提供的动态参数方式，示例如下。

``` python
def foobar(func):
        def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
        return wrapper

@foobar
def hey(name):
        print("Hey", name)

@foobar
def hello(name, punc):
        print("Hello", name, punc)

hey("Python")
hello("World", "!")
```

### 嵌套

也就是使用多个修饰器，示例如下。

```
@funA
@funB
@funC
def fun():
    pass
```

其执行的顺序是 `fun = funA(funB(funC(fun)))` 。

### functools.wraps

在 `functools` 模块中，提供了 `wraps` 修饰符，可以用来消除装饰器对原函数造成的影响，即对原函数的相关属性进行拷贝，已达到装饰器不修改原函数的目的。

在使用 Python 装饰器时，被装饰后的函数实际上已经是另外一个函数了，函数名绑定的是另外的一个嵌入式函数对象，其对应的函数名 `__name__`、文档 `__doc__` 等都会发生改变。

添加 `functools.wrap` 之后就可以保留上述的函数属性。

``` python
import functools

def log_it(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
                """A wrapper function."""
                print("Calling function {}".format(func.__name__))
                return func(*args, **kwargs)
        return wrapper

@log_it
def square(x):
        """Calculate the square of the given number."""
        return x * x

if __name__ == '__main__':
        print(square(3))

        print(square.__doc__)
        print(square.__name__)
```

如果查看 `functools.wrap` 的代码，就可以看到，实际上就是替换了一些指定的属性。

## 参考

* [PEP 0318](https://www.python.org/dev/peps/pep-0318/) 修饰符的官方 PEP 。
* [Compound Statements](https://docs.python.org/reference/compound_stmts.html#function) 在 Python 的 function 语法中，包含了对修饰符语法的详细定义。
* [Python Decorators Library](https://wiki.python.org/moin/PythonDecoratorLibrary) 包含了很多修饰符的实现，可用来参考借鉴。
