---
title: Python 闭包以及装饰器介绍
layout: post
comments: true
language: chinese
tag: [Python, Program]
keywords:
description:
---

<!-- more -->



## 修饰符

所谓修饰工作就是想给现有的模块加上一些小装饰（一些小功能，这些小功能可能好多模块都会用到），但又不让这个小装饰侵入到原有的模块中的代码里去。

如下是最简单的方式。

{% highlight python %}
#!/usr/bin/env python
def foobar(cb):
    print "I am foobar", id(cb)
    return cb
@foobar
def test(id):
    print "I am test"
print "Get test() ID:", id(test)

# OUTPUT:
# I am foobar 139900142408360
# Get test() ID: 139900142408360
{% endhighlight %}

该函数实际调用了 test = foobar(test)，也就是会调用 foobar() 函数，而且 test() 函数没有改变。

上面的示例只是简单介绍下修饰符的调用方法，而实际使用较多的是如下的方式。

{% highlight python %}
#!/usr/bin/env python
def foo(fn):
    print "I am foo()", id(fn)
    def wrapper():
        print "hello, %s" % fn.__name__
        fn()
        print "goodby, %s" % fn.__name__
    return wrapper
@foo
def foobar():
    print "I am foobar()", id(foobar)
print "Get foobar() ID:", id(foobar)
foobar()

# OUTPUT:
# I am foo() 140187446047400
# Get foobar() ID: 140187446047520
# hello, foobar
# I am foobar() 140187446047520
# goodby, foobar
{% endhighlight %}

这里实际上是采用了闭包的概念，上述的操作实际上等价于 foobar = foo(foobar) + foobar()。foo(foobar) 返回了 wrapper() 函数，所以， foobar 变量其实变成了 wrapper 的函数对象，而后面调用 foobar() 的执行，其实就变成了 wrapper()。

除了上述的之外还可以通过 class 的修饰符来实现。

{% highlight python %}
#!/usr/bin/env python
class foo(object):
    def __init__(self, fn):
        print "hello foo.__init__()", id(fn)
        self.fn = fn
    def __call__(self):
        self.fn()
        print "hello foo.__call__()"
@foo
def foobar():
    print "I am foobar()", id(foobar)
print "Get foobar() ID:", id(foobar)
foobar()

# OUTPUT:
# hello foo.__init__() 139943209067056
# Get foobar() ID: 139943209032976
# I am foobar() 139943209032976
# hello foo.__call__()
{% endhighlight %}

在创建修饰符时，首先调用 \_\_init\_\_()，此时需要一个 fn 参数；而执行时会调用 \_\_call\_\_() 。


对于多个修饰符/带参数的为：

{% highlight python %}
#!/usr/bin/env python
def foo_1(fn):
    print "I am foo_1()", id(fn)
    def wrapper_1():
        print "hello 111, %s" % fn.__name__
        fn()
        print "goodby 111, %s" % fn.__name__
    return wrapper_1
def foo_2(fn):
    print "I am foo_2()", id(fn)
    def wrapper_2():
        print "hello 222, %s" % fn.__name__
        fn()
        print "goodby 222, %s" % fn.__name__
    return wrapper_2
@foo_2
@foo_1
def foobar():
    print "I am foobar", id(foobar)
foobar()        # equal to: foobar = foo_2(foo_1(foobar))

# OUTPUT:
# I am foo_1() 140528766391072
# I am foo_2() 140528766391192
# hello 222, wrapper_1
# hello 111, foobar
# I am foobar 140528766391312
# goodby 111, foobar
# goodby 222, wrapper_1
{% endhighlight %}

对于多个修饰符，实际上等价于 foobar = foo_2(foo_1(foobar)) ，也就是 foo_1(foobar) 返回了 wrapper_1() 接着会调用 foo_2()，因此最后 foobar = foo_2(wrapper_1) 。

另外一种，就是带有参数的修饰符。

{% highlight python %}
#!/usr/bin/env python
def foo(arg):
    def real_decorator(fn):
        def wrapped():
            print "hello %s, %s" % (arg, fn.__name__)
            fn()
            print "goodby %s, %s" % (arg, fn.__name__)
        return wrapped
    return real_decorator

@foo("hi")
def foobar():
    print "hello world"
foobar()                # foobar = foo("hi")(foobar)

@foo("22")
@foo("11")
def foobar1():
    print "hello world"
foobar1()
{% endhighlight %}
带有参数的实际等价于 foobar = foo("hi")(foobar) ，首先执行的是 foo("hi")，返回一个真正的修饰符；然后剩下的就进入了之前的修饰符逻辑。<br><br><br>



是新类的特性


描述符实际上就是可以重用的属性，

通常Python默认对属性的操作是从对象的字典(\_\_dict\_\_)中获取get，设置set或者删除delete。如，对于实例a，a.x的查找顺序为a.\_\_dict\_\_['x']，接着是type(a).\_\_dict\_\_['x']，然后是父类中查找。而如果属性x是一个描述符，那么访问a.x时不再从字典\_\_dict\_\_中读取，而是调用描述符的\_\_get\_\_()方法，对于设置和删除也是同样的原理。因此个人猜测没有验证，实际上是先查找属性是否有\_\_get\_\_()等方法，如果没有则在\_\_dict\_\_中查找。

例如，我们要求薪水的值应该大于0，可以在\_\_init\_\_中进行检测，但是可以在外部调用修改为负值，详见property.py中的PayBug类。为了对该值进行检测，可以通过@property修饰符进行修改，内容详见property.py中的Pay类。

但是这样修改仍然存在这麻烦，如果一个类里含有多个相似的属性（要求不能为负值），那么需要重复添加多个类似的函数。

我们可以通过类的__init__函数对变量进行初始化（初始化的变量名与类的变量名相同），当通过instance.__dict__查看时，不存在对应的变量；只能通过Class.__dict__来查看。而对于正常的类，这样操作实际是保存了一组类变量和一组实例变量。也就是说我们可以在__init__中对属性进行赋值，但是操作还是在描述符中进行。

在参考文献1中，在描述符类中采用了WeakKeyDictionary来解决不同实例之间的冲突，但是测试发现，现在的Python中不存在类似的问题。




## 参考

* [Compound Statements](https://docs.python.org/reference/compound_stmts.html#function) 在 Python 的 function 语法中，包含了对修饰符语法的详细定义。
* [Python Decorators Library](https://wiki.python.org/moin/PythonDecoratorLibrary) 包含了很多修饰符的实现，可用来参考借鉴。
