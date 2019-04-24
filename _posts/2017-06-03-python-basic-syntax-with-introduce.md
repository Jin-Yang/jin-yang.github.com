---
title: Python With 语句简介
layout: post
comments: true
language: chinese
category: [program, python]
keywords: python,with
description: 通常会有一些任务，开始需要进行设置，处理任务，事后做清理工作。对于这种场景，Python 的 with 语句提供了一种非常方便的处理方式。一个很好的例子是文件处理，你需要获取一个文件句柄，从文件中读取数据，然后关闭文件句柄。
---

通常会有一些任务，开始需要进行设置，处理任务，事后做清理工作。对于这种场景，Python 的 with 语句提供了一种非常方便的处理方式。一个很好的例子是文件处理，你需要获取一个文件句柄，从文件中读取数据，然后关闭文件句柄。

<!-- more -->

正常的处理过程以及使用 with 时的处理过程如下：

{% highlight python %}
file = open("/tmp/foo.txt")
try:
    data = file.read()
finally:
    file.close()

with open("/tmp/foo.txt") as file:
    data = file.read()
{% endhighlight %}

基本思想是 with 之后的函数返回的对象必须有  `__enter__()` 和 `__exit__()` 方法。

紧跟 with 后面的语句被求值后，返回对象的 `__enter__()` 被调用，这个方法的返回值将被赋值给 as 后面的变量；当 with 后面的代码块全部被执行完之后，将调用前面返回对象的 `__exit__()` 。

{% highlight python %}
#!/usr/bin/env python
class Sample:
    def __enter__(self):
        print "In __enter__()"
        return "Foo"

    def __exit__(self, type, value, trace):
        print "In __exit__()"

def get_sample():
    print "In get_sample()"
    return Sample()

with get_sample() as sample:
    print "sample:", sample
{% endhighlight %}

执行的顺序为 A) `get_sample();` 返回 Sample 对象； B) Sample: `__enter__()`; 将返回的值赋值给 sample； C) 执行代码块； D) 执行 Sample: `__exit__()` 。

with 真正强大之处是它可以处理异常。`__exit__()` 有三个参数，val, type 和 trace，这些参数在异常处理中相当有用。

{% highlight python %}
#!/usr/bin/env python
class Sample:
    def __enter__(self):
        return self

    def __exit__(self, type, value, trace):
        print "type:", type
        print "value:", value
        print "trace:", trace

    def do_something(self):
        bar = 1/0
        return bar + 10

with Sample() as sample:
    sample.do_something()
{% endhighlight %}

在 with 后面的代码块抛出任何异常时，`__exit__()` 方法被执行。正如例子所示，异常抛出时，与之关联的 type，value 和 stack trace 传给 `__exit__()` 方法，因此抛出的 `ZeroDivisionError` 异常被打印出来了。

{% highlight python %}
{% endhighlight %}
