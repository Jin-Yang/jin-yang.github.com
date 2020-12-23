---
title: Python 模块简介
layout: post
comments: true
tag: [Python, Program]
language: chinese
keywords: python,模块
description: 在 Python 中，通过模块 (Module) 可以定义函数、类、变量等，从而更有逻辑地组织 Python 代码。这里介绍模块的引入、搜索、制作等一些常见的问题。
---

在 Python 解析器中，每次重新进入，之前定义的函数、变量等都会丢失，所以，通常是将代码保存在文件中运行，而随着功能增加，文件会变得越来越大，为了方便维护，会将其拆分成多个文件。

其它语言也有类似的问题，Python 将其称为模块 (Module) ，通过模块可以定义函数、类、变量等，从而更有逻辑地组织 Python 代码。

这里介绍模块的引入、搜索、制作等一些常见的问题。

<!-- more -->

![python modules]({{ site.url }}/images/python/python-modules-logo.jpg "python modules"){: .pull-center}

## 简介

在模块内运行时，可以通过全局变量 `__name__` 获取，最常见的就是 `__main__` 的判断，也就是是否为主 Pyhton 脚本。

``` python
if __name__ == "__main__":
	print("Hello World")
```

也就是该文件可以作为模块引入其它代码中，也可以作为一个独立的执行文件运行，而判断的逻辑就是这里的 `if __name__ == "__main__"` 语句，直接调用脚本时，对应 `__name__` 变量保存的是 `__main__` ，如果是引入的模块，则是模块名称。

### 模块导入

可以通过 `import` 关键字导入模块，也就是添加到当前的命名空间中，根据不同的使用场景，有如下的几种方式。

``` python
# 直接导入模块，可以通过math.cos()调用
import math

# 导入模块中的函数，这样可以直接调用cos sin，不过很容易被覆盖
from math import cos, sin

# 导入模块的所有符号
from math import *

# 导入模块的符号，为了防止名字覆盖，将其重命名
from math import pi as PI
```

导入的模块会保存在 `sys.modules` 中，这是一个字典，如果多次导入第二次实际不会有加载的动作。

## 模块分类

Python 中包括了三类的模块：A) 内置 (builtin) 模块，编译在解析器的内部，可以直接使用；B) 标准模块，安装 python 时已经安装，包括了一些 C 的实现，也有纯 Python 实现，例如 `os` `gc` 等；C) 第三方包，需要手动安装，如 `MySQLdb` 。

常用模块(库)可参考 [The Python Standard Library](https://docs.python.org/library/) 中的详细介绍，其中部分模块可以在源码的 Modules 目录下查看，如 `ctypes`、`gcmodule` 等；有些包含在 Python 目录下，如 `sysmodule.c`、`bltinmodule.c` 等。

其中，后者的模块会直接编译到 python 可执行文件中，可以通过 `nm python` 查看，不过对于发行版本，通常已经将调试信息清理过，可以添加 `-D` 参数查看。

### 内置模块

对于 builtin 模块，可以参考源码中的 `builtin_methods[]` 数组，包括了 `abs`、`compile`、`format`、`filter` 等一些常见的函数实现。

``` c
static PyMethodDef builtin_methods[] = {
    {"__import__",      (PyCFunction)builtin___import__, METH_VARARGS | METH_KEYWORDS, import_doc},
    {"abs",             builtin_abs,        METH_O, abs_doc},
    ... ...
    {"compile",         (PyCFunction)builtin_compile,    METH_VARARGS | METH_KEYWORDS, compile_doc},
    {"delattr",         builtin_delattr,    METH_VARARGS, delattr_doc},
    {"dir",             builtin_dir,        METH_VARARGS, dir_doc},
    {"divmod",          builtin_divmod,     METH_VARARGS, divmod_doc},
    {"eval",            builtin_eval,       METH_VARARGS, eval_doc},
    {"execfile",        builtin_execfile,   METH_VARARGS, execfile_doc},
    {"filter",          builtin_filter,     METH_VARARGS, filter_doc},
    {"format",          builtin_format,     METH_VARARGS, format_doc},
    {"getattr",         builtin_getattr,    METH_VARARGS, getattr_doc},
    ... ...
    {NULL,              NULL},
};
```

如上，对于 buildin 模块，在源码中，其有效的函数名通常是 `buildin_xxx()`；另外，对于内置模块，不会存在 `__file__` 属性，而三方的模块可以通过该属性查看模块的安装路径；当然，也可以通过类似 `help(abs)` 的命令查看帮助信息。

### 标准模块

也就是安装 Python 的时候默认就安装好的模块，以 CentOS 为例，保存在 `/usr/lib64/python${VERSION}` 目录下，其中的 `${VERSION}` 标识 Python 的版本号，而且，如果是 32 位系统，对应的是 `/usr/lib` 目录。

例如 `logging`、`unittest`、`multiprocessing` 等，在安装完之后就可以直接使用。

### 三方模块

对于一些三方模块，可以通过通用的工具进行安装。

{% highlight text %}
# python setup.py build                    ← 下载的源码直接安装
# easy_install PACKAGE                     ← 通过easy_install或pip安装
# pip install PACKAGE
{% endhighlight %}

默认会安装在 `/usr/lib64/python${VERSION}/site-packages/` 目录下。

## 搜索路径

模块在加载时，会按照如下的顺序搜索：A) 导入的模块是否为内置模块；B) 从 `sys.path` 变量中搜索模块名。

而 `sys.path` 变量在加载时就已经生成，可以通过 `python -c "import sys; print(sys.path)"` 查看搜索路径，默认是 A) 当前目录；B) `PYTHONPAHT` 环境变量；C) 安装默认路径，例如上述的 `/usr/lib64/python${VERSION}` 目录。

### 安装新模块

如果新安装的模块在其它路径下，那么就需要动态添加搜索路径，可以有如下的几种方式。

1. 修改 `PYTHONPATH` 环境变量，指定到新模块所在路径。
2. 增加 `*.pth` 文件，在遍历已知目录时，如果遇到 `.pth` 文件，会自动将其中的路径添加到 `sys.path` 变量中。
3. 通过 `sys` 模块的 `append()` 方法增加搜索路径。

操作详见如下内容。

{% highlight text %}
----- 修改~/.bashrc文件
export PYTHONPATH=$PYTHONPATH:/home/foobar/workspace

----- 在默认安装路径/usr/lib64/python2.7下增加extra.pth文件，内容如下
/home/foobar/workspace

----- 通过append()函数添加路径
import sys
sys.path.append('/home/foobar/workspace')
{% endhighlight %}

在加载完模块之后，可以通过如下命令查看模块的路径。

{% highlight python %}
import sys
sys.modules['foobar']

foobar.__file__
{% endhighlight %}

另外遇到一个比较奇葩的问题，安装完 Redis 之后，发现 root 用户可以 `import` 模块，而非 root 用户会报错。首先，十有八九是权限的问题导致的，在 root 导入模块后查看其路径，修改权限；对于 `easy_install` 安装，会修改 `easy-install.pth` 文件，也需要保证其它用户对该文件可读。

## 创建模块

只有当目录包含一个 `__init__.py` 文件时，才会被认作是一个包，最简单的是一个空文件；当然，也可以在这个文件中执行一些初始化代码，或者为 `__all__` 变量赋值。

其中 `__all__` 列表用于指定 `from package import *` 方式导入的名字，示例如下。

三方模块的目录结构为。

{% highlight text %}
$ tree
.
|-- bar
|   |-- __init__.py
|   |-- __init__.pyc
|   |-- test.py
|   `-- test.pyc
|-- foo
|   `-- __init__.py
`-- test.py
2 directories, 6 files
{% endhighlight %}

各个文件的内容如下。

{% highlight python %}
##### cat bar/__init__.py
barstr = "Hello World, IN BAR"
from test import bar

##### cat bar/test.py
def add(a, b):
    print "Bar", a+b
def bar(var):
    print id(var)

##### cat test.py
# -*- coding:utf-8 -*-
from bar import barstr, test, bar
print barstr                          # 打印__init__.py中的变量
test.add(1,2)                         # 执行bar/test.py中的函数
bar("function in bar/test.py")        # 执行bar/test.py中的函数
{% endhighlight %}

接下来列举的就是一些常用的三方模块了，可以在使用的时候以供参考。
