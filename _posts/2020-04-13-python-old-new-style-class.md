---
title: Python 新类和旧类区别
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: Python, 新类, 旧类, new style class
description: 在 Python 2.2 版本中引入了新式类 (New Style Class)，所以，通常将在此之前的类称为经典类或者旧类，而在此之后的称为新式类，两者在使用时略有区别。这里会简单介绍两者的区别。

---

在 Python 2.2 版本中引入了新式类 (New Style Class)，所以，通常将在此之前的类称为经典类或者旧类，而在此之后的称为新式类，两者在使用时略有区别。

这里会简单介绍两者的区别。

<!-- more -->

## 简介

之所以这么做的原因，官方给出的解释是，统一类 Class 和类型 Type 。实际上，可以比对 Python2 和 Python3 的区别。

```
$ python2
>>> two = 2
>>> type(two)
<type 'int'>

$ python3
>>> two = 2
>>> type(two)
<class 'int'>
```

显然，在 Python2 中，通过 `type()` 函数返回的是 `<type 'int'>` ，而在 Python3 中返回的则是 `<class 'int'>` 类型，其时，严格来说，两者是等价的，只是在 Python2 中将内置类型和用户定义的类进行了区分，而 Python3 则将两者进行了统一。

```
$ python2
>>> class FooBar(object):
...     pass
...
>>> foobar = FooBar()
>>> type(foobar)
<class '__main__.FooBar'>

$ python3
>>> class FooBar(object):
...     pass
...
>>> foobar = FooBar()
>>> type(foobar)
<class '__main__.FooBar'>
```

如上，对于用户定义的类，在 Python2 和 Python3 中是一致的。在旧类中，通过 `foobar.__class__` 返回的与上述相同，而 `type(foobar)` 返回的一直是 `<type 'instance'>` 。

所以，对于类来说，从 Python2.2 到之后的 Python2.X 版本，以及到现在的 Python3.X 版本，其内部实现一直在变化，但趋势是更加统一了。

另外，在引入新类之后，同时引入了更多的内置属性，也包括了描述符的引入等。

### Python2 VS. Python3

首先，建议使用新类，不同的 CPython 实现版本会略有区别。

<!--
所以，为了确保自己使用的是新式类，有两种以下方法：
1. 元类，在类模块代码的最前面加入如下代码 __metaclass__ = classname(自定义的某个新式类)。
2. 类都从内建类object直接或者间接地继承。
-->

在 Python2 中默认都是经典类，只有显式继承了 `object` 才是新式类；而在 Python3 中默认都是新式类，不需要再显式继承 `object` 即可。

## 区别以及新增特性

如上所示，在新式类中，内置的 `__class__` 属性和 `type()` 函数获取信息保持一致。

### 搜索顺序

在类的多重继承时，对于属性的搜索顺序发生了改变：A) 经典类，先深入继承树左侧，返回，再开始找右侧，也就是深度优先搜索；B) 新式类，先水平搜索，然后再向上移动，也就是广度优先搜索。

假设有如下的 Python 代码。

``` python
class A():
    def __init__(self):
        pass
    def save(self):
        print("This is from A")

class B(A):
    def __init__(self):
        pass

class C(A):
    def __init__(self):
        pass
    def save(self):
        print("This is from C")

class D(B, C):
    def __init__(self):
        pass

foobar =  D()
foobar.save()
```

也就是说，其继承关系为。

```
  A
 B C
  D
```

对于经典类，其输出为 `This is from A` ，而新类的输出为 `This is from C` 。

```
$ python2 foobar.py
This is from A

$ python3 foobar.py
This is from C
```

### 新类增加 \_\_slots\_\_ 属性

新式类增加了 `__slots__` 内置属性，可以把实例属性限定到 `__slots__` 规定的范围内。

``` python
class ClassA():
	__slots__ = ('name', 'age')

class ClassB(object):
	__slots__ = ('name', 'age')

ca = ClassA()
cb = ClassB()

ca.foobar = "Andy"
cb.foobar = "Andy"
```

注意，需要通过 Python2 运行，因为在 Python3 中，所有的类都是新式类。

```
$ python2 /tmp/foobar.py
Traceback (most recent call last):
  File "/tmp/foobar.py", line 11, in <module>
    cb.foobar = "Andy"
AttributeError: 'ClassB' object has no attribute 'foobar'
```

都通过 `__slots__` 进行了设置，在旧类中，允许访问对象 `ca.foobar` ，不会报错；而在访问 `cb.foobar` 时，会直接抛出异常。

每个类的实例都会有一个 `__dict__` 内置属性，用来记录实例中所有的属性和方法，也是通过这个字典，可以让实例绑定任意的属性。当类存在 `__slots__` 属性时，旧不会存在 `__dict__` 属性，从而可以减少内存的开销，适合创建小实例。

### 新类增加 \_\_getattribute\_\_ 方法

每次访问对象属性时，都会调用该方法，可定制一些属性访问策略。注意，如果直接访问类的属性该方法是不会被调用的。

