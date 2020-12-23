---
title: 详细介绍 Python 函数声明以及调用方式
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: Python, 参数, args, kargs
description: Python 提供了非常灵活的参数声明以及调用方式，除了最常用的位置参数，还包括了默认值以及关键词匹配方式，而且对于列表、字典很方便展开。这里详细介绍 Python 的参数声明、调用方式。
---

Python 提供了非常灵活的参数声明以及调用方式，除了最常用的位置参数，还包括了默认值以及关键词匹配方式，而且对于列表、字典很方便展开。

这里详细介绍 Python 的参数声明、调用方式。

<!-- more -->

## 声明方式

Python 中函数声明的时候有四种方式：

* `foobar(a, b, c)` 根据位置匹配形参，需要严格保证数量、顺序一致，也是最常见的一种。
* `foobar(a=1, b=2, c=3)` 提供了默认值，可以根据关键字传参，无需再关注参数的顺序，例如 `foobar(b=5, c=8, a=10)`；也不再要求参数一致，例如 `foobar()` 使用默认值，`foobar(a=10)` 除了参数 `a` 其它使用默认值。
* `foobar(*args)` 可以传入任意参数，参数会添加到 `args` 元组中，好处是参数长度不再限制，但是本身仍然存在顺序。
* `foobar(**kargs)` 同样可传入任意参数，不过是以字典方式传递，不再有顺序的限制。

注意，上述元组和字典声明中的 `*` 和 `**` 符号只是一种标识方式，并没有实际的含义；而 `args` 和 `kargs` 也只是习惯性的写法，可以换成其它参数。

也可以将上述的四种方式混合使用，也就是 `foobar(a, b=2, *args, **kargs)` ，需要注意其顺序，允许有些参数不存在，但是必须要确保其顺序。

## 函数调用

在通过函数调用参数的时候，有两种方式：A) 关键词参数 (Keyword Argument)，也就是 `foobar(a=1, b=2)` 这种调用方式；B) 位置参数 (Positional Argument) 也被称为非关键词参数 (Non-Keyword Argument)，最常用的 `foobar(1, 2)` 方式，参数位置通过声明已经约束明确好。

注意，这里的调用方式并未与上述的声明 (定义) 方式绑定，例如，如下的示例。

``` python
def foo(a, b, c):
        print(a, b, c)
def bar(a=1, b=2, c=3):
        print(a, b, c)
foo(c=3, a=1, b=2)  # (1, 2, 3)
bar(1, 2, 3)        # (1, 2, 3)
```

不过，传参的时候有个优先级，位置参数需要放到最开始，然后才能使用关键词参数，如下的使用方式是错误的。

``` python
def foobar(a, b, c=1):
	print(a, b, c)
#foobar(c=1, 1, 2) # ERROR 需要先使用非关键词参数
#foobar(1, 2, b=3) # ERROR 参数b被赋值了两次
```

实际上，匹配过程也是按照上述的混合声明的顺序进行，也就是：

1. 按照顺序匹配位置参数。
2. 将关键词参数赋值给对应的形参。
3. 将剩余非关键词参数打包成元组，并传递给 `*args` 参数。
4. 将剩余关键词参数打包成字典，并传递给 `**kargs` 参数。

对于默认参数，如果已经通过位置参数覆盖，那么就不能再次通过关键词参数传值，例如如下示例。

``` python
def foobar(a, b=2, *args, **kargs):
        print(a, b, args, kargs)
#foobar(1, 3, 5, 7, b=9) # FAIL
foobar(1)               # (1, 2, (), {})
foobar(1, 3)            # (1, 3, (), {})
foobar(1, 3, 5)         # (1, 3, (5,), {})
foobar(1, 3, 5, 7)      # (1, 3, (5, 7), {})
foobar(1, 3, 5, 7, c=9) # (1, 3, (5, 7), {'c': 9})
```

## 其它

元组和字典可以展开，在传参的过程中非常好用，如下是一些常见的使用方法。

``` python
def foo(*args, **kargs):
        print(args, kargs)

data = ("hello", "world")
foo(data)   # ((('hello', 'world'),), {})
foo(*data)  # (('hello', 'world'), {})

data = {
        "hi": "hey",
        "bye": "byebye",
}
foo(data)   # (({'bye': 'byebye', 'hi': 'hey'},), {})
foo(*data)  # (('bye', 'hi'), {})
foo(**data) # ((), {'hi': 'hey', 'bye': 'byebye'})
```

对于元组，通过 `*` 符号可以展开 (也可以展开列表、字典 Key)，然后传递到函数，在函数中的 `args` 变量会重新组合起来，就好像直接将元组传入一样；对于字典原理也基本类似，只是需要使用 `**` 符号扩展。

还有如下一种场景。

``` python
def bar(a, b):
        print(a, b)
data = ("hello", "world")
bar(*data)  # ('hello', 'world')

data = {
        "a": "hello", 
        "b": "world"
}
bar(**data) # ('hello', 'world')
```

元组会按照顺序展开，而字典则需要确保 `key` 与参数的名称相同。

## 总结

声明和传参的方式类似，但是需要保证其顺序；元组和字典可以很方便直接传参。

