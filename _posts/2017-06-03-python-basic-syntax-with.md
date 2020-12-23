---
title: Python With 语句详细介绍
layout: post
comments: true
language: chinese
tag: [Program, Python]
keywords: python,with
description: 通常会有一些任务，开始需要进行设置，处理任务，事后做清理工作。对于这种场景，Python 的 with 语句提供了一种非常方便的处理方式。这里详细介绍其实现的原理。
---

通常会有一些任务，开始需要进行设置，处理任务，事后做清理工作，对于这种场景，Python 的 with 语句提供了一种非常方便的处理方式。一个很好的例子是文件处理，你需要获取一个文件句柄，从文件中读取数据，然后关闭文件句柄。

这里详细介绍其实现的内部原理。

<!-- more -->

## 简介

正常的处理过程以及使用 `with` 时的处理过程如下。

``` python
file = open("/tmp/foo.txt")
try:
	data = file.read()
finally:
	file.close()

with open("/tmp/foo.txt") as file:
	data = file.read()
```

使用 `with` 语句要简单很多，这也就是 [PEP-343 The "with" Statement](https://www.python.org/dev/peps/pep-0343/) 规范定义的内容，实际上就是使用如下语句。

```
with VAR = EXPR:
    BLOCK
```

最终替换为。

```
VAR = EXPR
VAR.__enter__()
try:
    BLOCK
finally:
    VAR.__exit__()
```

也就是说，其基本思想就是调用 `with` 语句返回对象的 `__enter__()` 和 `__exit__()` 方法。

### 上下文管理

实际上，`with` 语句使用的就是上下文管理器，基于所谓的上下文管理协议 (Context Management Protocol)，该协议要求对象必须要实现 `__enter__()` 和 `__exit__()` 两个方法。

之所以采用上下文管理器，实际上就是为了优雅解决如下两个问题：

* 用来操作 (包括了创建、获取、释放) 资源，例如文件操作、数据库连接等。
* 以一种更优雅的方式处理异常。

当发生异常时，会调用 `__exit__` 方法，异常就以参数传入该函数，总共包含了三个参数：A) `type` 异常类型；B) `value` 异常值；C) `traceback` 异常的错误栈信息。如果正常，那么这三个参数都是 `None` ，该函数默认返回 `False` ，异常会再次抛出；当返回 `True` 时，意味着异常已经捕获。

## 使用详解

紧跟 `with` 后面的语句被求值后，返回对象的 `__enter__()` 被调用，这个方法的返回值将被赋值给 `as` 后面的变量；当 `with` 后面的代码块全部被执行完之后，将调用前面返回对象的 `__exit__()` 。

``` python
class Sample:
	def __enter__(self):
		print("In __enter__()")
		return "Foo"

	def __exit__(self, type, value, trace):
		print("In __exit__()")

def get_sample():
	print("In get_sample()")
	return Sample()

with get_sample() as sample:
	print("sample:", sample)
```

执行的输出如下。

```
In get_sample()
In __enter__()
sample: Foo
In __exit__()
```

也就是说，执行的顺序为 A) `get_sample();` 返回 `Sample` 对象； B) 执行 `Sample` 对象中的 `__enter__()` 函数，将返回的值赋值给 `sample` 变量； C) 执行代码块； D) 最后执行 `Sample` 对象中的 `__exit__()` 。

注意，上述的 `get_sample()` 也可以使用 `Sample()` 构造函数，效果一样。

### 异常处理

而 `with` 语句的真正强大之处是它可以处理异常，在 `__exit__()` 函数中，有三个参数 `val` `type` 和 `trace`，这些参数在异常处理中相当有用。

``` python
class Sample:
	def __enter__(self):
		return self

	def __exit__(self, type, value, trace):
		print("type:", type)
		print("value:", value)
		print("trace:", trace)
		return True

	def do_something(self):
		bar = 1/0
		return bar + 10

with Sample() as sample:
	sample.do_something()
```

在 `with` 后面的代码块抛出任何异常时，`__exit__()` 方法被执行，正如上述的例子所示。在异常抛出时，与之关联的 `type` `value` 和 `stack trace` 传给 `__exit__()` 方法，因此抛出的 `ZeroDivisionError` 异常被打印出来了。

注意，如果 `__exit__()` 返回非 `True` 会再次抛出异常。

## contextlib

在如上的示例中，为了使用上下文管理器而实现一个类，显然有些复杂，实际上 Python 已经提供了一个 `contextlib` 库，通过修饰符就可以实现。

``` python
import contextlib

@contextlib.contextmanager
def open_file(filename):
	# __enter__
	print("open file %s." % filename)
	handler = open(filename, "r")

	yield handler

	# __exit__
	print("close file %s." % filename)
	handler.close()

with open_file("/tmp/foobar.txt") as file:
	for line in file:
		print(line)
```

在被装饰的函数中，最为关键的是生成器，也就是 `yield` 关键字，在此之前就相当于 `__enter__` 函数中的内容，而在此之后就相当于 `__exit__` 中的内容。

不过上述的实现没有处理异常，如果需要，应该使用如下方式。

``` python
import contextlib

@contextlib.contextmanager
def open_file(filename):
	# __enter__
	print("open file %s." % filename)
	handler = open(filename, "r")

	try:
		yield handler
	except Exception as e:
		print("got exception")
	finally:
		print("close file %s." % filename)
		handler.close()

with open_file("/tmp/foobar.txt") as file:
	for line in file:
		print(line)
```
