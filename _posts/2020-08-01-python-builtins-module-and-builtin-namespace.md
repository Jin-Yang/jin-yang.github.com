---
title: Python 内置模块和内置命名空间介绍
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: Python, 内置模块, 内置命名空间, builtins
description: 在 CPython 的实现中有一个内置的 builtins 模块，同时还包含了一个内置的命名空间，其 Key 被称为 __builtins__ ，两者在查找时会相互关联，不过在不同模块中实现又不相同，这里详细介绍。
---

在 CPython 的实现中有一个内置的 builtins 模块，同时还包含了一个内置的命名空间，其 Key 被称为 __builtins__ ，两者在查找时会相互关联，不过在不同模块中实现又不相同。

这里详细介绍其区别。

<!-- more -->

## 简介

在 Python 中有很多内置的实现，例如 `BaseException` 这类异常、`map` `eval` 等函数、`__name__` `__package__` 等内置的变量等等，而这些都保存在内置命名空间中，会在查找方法、变量时优先查找，不过其在不同的场景下略有区别。

### 交互命令

首先在交互命令中，可以通过如下方式查看。

```
----- 查看全局变量名称
>>> globals().keys()
dict_keys(['__name__', '__doc__', '__package__', '__loader__', '__spec__', '__annotations__', '__builtins__'])
>>> __builtins__
<module 'builtins' (built-in)>

----- 导入内置模块的实现
>>> import builtins
>>> builtins is __builtins__
True
```

也就是在全局命名空间中存在一个关键字为 `__builtins__` 的对象，而该对象实际上就是指向了内置模块 `builtins` 。

注意，Python3 将 Python2 中的 `__builtin__` 模块，重命名为了 `builtins` 模块，所以，如下的代码如果要在 Python2 中测试，需要进行简单的替换，或者，也可以直接参考下面的参考文件。

也就是说 `__builtins__` 和 `builtins` 是相同的，所以，为什么 Python 会这么实现呢？

## 关联

详细可以参考 [Execution Model](https://docs.python.org/reference/executionmodel.html#builtins-and-restricted-execution) 中的介绍，这里摘抄如下。

> CPython implementation detail: Users should not touch __builtins__; it is strictly an implementation detail. Users wanting to override values in the builtins namespace should import the builtins module and modify its attributes appropriately.
>
> The builtins namespace associated with the execution of a code block is actually found by looking up the name __builtins__ in its global namespace; this should be a dictionary or a module (in the latter case the module’s dictionary is used). By default, when in the __main__ module, __builtins__ is the built-in module builtins; when in any other module, __builtins__ is an alias for the dictionary of the builtins module itself.

转述下，用户应该忽略 `__builtins__` 的实现，这只是 CPython 的实现，并非 Python 标准规定的，如果使用可能会导致使用其它解析器时报错。如果要覆盖内置的一些函数，那么应该导入 `builtins` 模块，然后再通过该模块覆盖。

对于内置命名空间 CPython 会通过 `__builtins__` 变量查找，可以是一个字典，也可以是模块 (会使用模块的字典查找)。 默认，在 `__main__` 模块中 `__builtins__` 对应了使用内置 `builtins`，如上所示，而在其它模块使用的是内置模块 `builtins` 的字典。

### 验证

上述使用交互命令行和运行脚本的方式一样，创建一个 `foobar.py` 文件，示例如下。

``` python
import builtins

print("__name__ is:", __name__)
print("builtins is __builtins__:", builtins is __builtins__)
print("type(builtins):", type(builtins))
print("type(__builtins__):", type(__builtins__))
print("__builtins__ is builtins.__dict__:", __builtins__ is builtins.__dict__)
```

然后通过 `python3 foobar.py` 命令运行，输出如下。

```
$ python3 foobar.py
__name__ is: __main__
builtins is __builtins__: True
type(builtins): <class 'module'>
type(__builtins__): <class 'module'>
__builtins__ is builtins.__dict__: False
```

也可以执行解析器 `python3` ，然后，在解析器中通过 `import foobar` 引入模块，输出如下。

```
$ python3
>>> import foobar
__name__ is: foobar
builtins is __builtins__: False
type(builtins): <class 'module'>
type(__builtins__): <class 'dict'>
__builtins__ is builtins.__dict__: True
```

也就是说，在 `__main__` 模块中，其中 `__builtins__` 对应的值是 `builtins` 内置模块，而在非 `__main__` 模块中使用的是 `builtins.__dict__` 。

## 总结

`__builtins__` 是 CPython 自己内部的实现，与 Python 标准没有关系，在编程时，不建议直接使用，如果需要，应该直接使用 `builtins` 内置模块。

例如，在 `__main__` 模块中，可以使用 `__builtins__.map` ，但是在非 `__main__` 模块中，会直接报 `AttributeError: 'dict' object has no attribute 'map'` 的错误。如果使用 `__builtins__["map"]` 也会面临相同的问题，会直接报 `TypeError: 'module' object is not subscriptable` 的错误。

## 参考

* [What's the deal with \_\_builtins\_\_ vs \_\_builtin\_\_](http://mathamy.com/whats-the-deal-with-builtins-vs-builtin.html) 在 Python2 中，关于 `__builtins__` 以及 `__builtin__` 的详细介绍。

