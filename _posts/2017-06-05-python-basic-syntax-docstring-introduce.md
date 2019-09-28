---
title: Python DocString 简介
layout: post
comments: true
language: chinese
category: [program, python]
keywords: python,docstring
description: 也就是 Python 的文档字符串，提供了一种很方便的方式将文档与 modules、functions、classes 以及 methods ；该字符串是在对象的第一个语句中定义，然后通过 ```__doc__``` 引用。
---

也就是 Python 的文档字符串，提供了一种很方便的方式将文档与 modules、functions、classes 以及 methods ；该字符串是在对象的第一个语句中定义，然后通过 ```__doc__``` 引用。

<!-- more -->

{% highlight python %}
#!/usr/bin/env python
#-*- coding:utf-8 -*-
"""
This is an example for python docstring, including modules,
    functions, classes, methods, etc.
"""

import math
print math.__doc__            # 标准库的文档字符串
print str.__doc__             # 标准函数的文档字符串

def foo():
    """It's just a foo() function."""
    pass
class foobar(object):
    """It's just a foobar class."""
    def bar():
        """It's just a foobar.bar() function."""
        pass
    def get_doc(self):
        return self.__doc__
f = foobar()
print f.__doc__               # 通过实例返回类的文档字符串
print f.get_doc()             # 或者调用函数返回文档字符串
print __doc__                 # 本模块的文档字符串
print foo.__doc__             # 函数的文档字符串
print foobar.__doc__          # 自定义类的文档字符串
print foobar.bar.__doc__      # 类中方法的文档字符串
{% endhighlight %}

当然，也可以使用 pydoc 模块，这个模块是 Python 2.1 引入的，提供了一个交互界面显示对象的信息，当然也包括了文档字符串。

{% highlight python %}
from pydoc import help
def foobar():
    """
    It's just a foobar function.
    """
    return
help(foobar)
{% endhighlight %}

{% highlight python %}
{% endhighlight %}
