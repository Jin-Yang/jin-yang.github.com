---
title: Python 虚拟环境工作原理
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: Python, 虚拟环境, virtalenv
description: Python 的虚拟环境用来创建一个相对独立的执行环境，尤其是一些依赖的三方包，最常见的如不同项目依赖同一个但是不同版本的三方包，而且，在虚拟环境中的安装包不会影响到系统的安装包。不过，其具体的工作原理是怎样的，这里详细介绍。
---

Python 的虚拟环境用来创建一个相对独立的执行环境，尤其是一些依赖的三方包，最常见的如不同项目依赖同一个但是不同版本的三方包，而且，在虚拟环境中的安装包不会影响到系统的安装包。

不过，其具体的工作原理是怎样的，这里详细介绍。

<!-- more -->

## 简介

几乎每个语言都包含自己的包管理工具，这是一个非常复杂的话题，而不同语言选择的实现又略有区别，都会做一些选择和取舍。而 Python 的包管理解决方案很多，例如 pip、virtualenv、pyenv 等等。

不过 Python 语言本身的机制决定了其原理一样。

### 使用

比较常用的是 virtualenv 工具，可以参考 [Guide to Python](https://docs.python-guide.org/) 中的详细介绍，另外，Python3 也提供了自己的虚拟环境创建模块，在创建完成后基本都是通过一个脚本启用独立环境。

例如，如下是使用 virtualenv 以及 venv 的虚拟环境创建过程。

```
$ mkdir /tmp/project && cd /tmp/project

$ virtualenv --no-site-packages foobar
$ python3 -m venv foobar
```

然后，就可以通过 `source foobar/bin/activate` 命令激活新环境。

### 激活脚本

所谓的独立环境，无非就是解决两个问题：A) 执行 Python 解析器所使用的版本；B) 使用独立的包。其中前者，在 Linux 主要是通过 `PATH` 环境变量设置，在 `activate` 脚本中有如下的内容。

``` bash
VIRTUAL_ENV="/tmp/project/foobar"
export VIRTUAL_ENV

_OLD_VIRTUAL_PATH="$PATH"
PATH="$VIRTUAL_ENV/bin:$PATH"
export PATH
```

也就是将创建的目录添加到 `PATH` 环境变量最开始，那么就会优先查找该路径，这样就解决了 python 解析器独立的问题。

## 工作原理

如果要使用独立的包，那么关键就是如何在通过 `import` 导入时查找到所需的包。

包的查找顺序可以查看 [Python 模块简介](/post/python-modules.html) 中的介绍，简单来说，就是先查看是否是内置模块，然后再从 `sys.path` 列表指定的地址中搜索。所以，这里的关键就是 `sys.path` 列表的生成。

### 关于 sys.prefix

<!--
这里关键是 `sys.prefix` 生成，会以该路径
-->

在 Python 启动的时候，会先加载一个强依赖的 `os.py` 包，而查找这个包是根据解析器的当前路径，以及固定的查找规则来实现的。

简单来说，就是在当前路径加上 `lib/python${VERSION}/os.py` 逐层向上查找，注意，如果是 64 位的操作系统，那么会使用 `lib64` 替换掉之前的 `lib` 路径。

例如，默认的 Python3 的解析器路径为 `/usr/bin/python3.6` ，那么基础路径是 `/usr/bin/` ，所以，其查找顺序为。

```
/usr/bin/lib64/python3.6/os.py
/usr/lib64/python3.6/os.py
/lib64/python3.6/os.py
```

只要在任意路径上找到 `os.py` 包，那么就会退出查找，并设置好 `sys.prefix` 变量，详细可以通过 `strace python` 查看，会有如下的搜索路径。

```
stat("/usr/bin/Modules/Setup", 0x7fffb7146300) = -1 ENOENT (No such file or directory)
stat("/usr/bin/lib64/python2.7/os.py", 0x7fffb71462f0) = -1 ENOENT (No such file or directory)
stat("/usr/bin/lib64/python2.7/os.pyc", 0x7fffb71462f0) = -1 ENOENT (No such file or directory)
stat("/usr/lib64/python2.7/os.py", {st_mode=S_IFREG|0644, st_size=25910, ...}) = 0
```

在查找到 `os.py` 之后，会将该路径设置为 `sys.prefix` 变量，然后解析器就会到 `${sys.prefix}/lib/python${VERSION}` 目录下查找包。

### 总结

那么其工作原理就是，将 python 解析器保存在 `${VENV_PATH}/bin/python` ，然后创建 `${VENV_PATH}/lib/python${VERSION}` 目录，并将相关的文件复制到该目录下，可以复制文件，也可以使用软连接。

## 其它

如上，如果是 Python3 就可以直接使用内置的 `venv` 模块，其原理与上述的相同，同时通过 `pyvenv.cfg` 配置文件来标识原始的 home 位置，该文件的内容如下。

```
home = /usr/bin
include-system-site-packages = false
version = 3.6.8
```

如果 `include-system-site-packages` 为 `true`，解释器启动时就会将系统的库添加到 `sys.path` 里面，这样在虚拟环境就可以 `import` 系统中安装的包了。

注意，Python3 提供的 `venv` 模块只能根据当前版本创建，不能支持 Python2 。

## 参考

* [Virtualenv](https://virtualenv.pypa.io/en/latest/) 官方文档，细节可以参考该文档。
* [Creation of virtual environments](https://docs.python.org/3/library/venv.html) Python3 提供的 venv 介绍，包括常见参数以及配置文件。

<!--
https://docs.python-guide.org/dev/virtualenvs/
-->
