---
title: 【专题】Python 编程语言
layout: post
comments: true
language: chinese
tag: [Python, Topic]
---

详解介绍 Python 的使用。

<!-- more -->

![Python Logo]({{ site.url }}/images/python/python-logo.png "Python Logo"){: .pull-center width="420"}

通常当我们讨论 Python 时，指的是 Python 语言以及 CPython 实现。而实际上 Python 只是一种语言的规范，可以根据该规范使用不同的语言去实现相应的解析器，除了 CPython 之外，常见的还有 PyPy、Jython、IronPython、MicroPython 等。

对于传统语言，如 C/C++ 等，会直接将代码编译为机器语言后运行，而对于不同的平台或者 CPU 需要重新编译才可以，而 Python 可以直接跨平台运行。

CPython 通过 C 语言实现，也是目前使用最为广泛的版本，虽然 PyPy 现在的发展势头不错，不过估计短时间内还是不会替代 CPython。CPython 也需要编译 (编译成字节码)，然后运行，其核心实际上是一个字节码解析器 (Bytecode Interpreter)，用于模拟堆栈操作，或者称之为 Virtual Stack Machines。

如果没有特殊说明的话，在此特指 CPython；另外，比较想提一下的是 MicroPython，这是一个用于微控制器的 Python 实现 ^_^

Just More Pythonic ~~~

## CPython

也就是通过 C 语言实现的 Python 规范，也是目前使用最为广泛的版本，此时，包含了 Python2 以及 Python3 两个版本，而且两个版本的特性差别很大，并不兼容。

这里主要介绍 Python3 的使用，其中可能会有部分介绍 Python2 。

### 环境准备

* [基本环境准备](/post/python-environment-prepare.html) 在 Linux、Windows 中如何搭建 Python 的开发环境，包括了如何创建虚拟环境。

### 基本概念

一些常见的语法、规范等。

* [变量绑定](/post/python-variables-bind.html) 一切皆对象导致了变量与其它语言概念上有所差异。
* [命名空间和作用域](/post/python-namespace-scope.html) 在命名空间中保存了变量名到值的映射，而作用域决定了那些变量是有效的。
* [模块机制](/post/python-modules.html) 关于模块、包的介绍，包括了基本概念、包的引入、查找顺序、创建模块等。
* [入参格式](/post/python-parameter-methods.html) Python 提供了很灵活的入参方式，包括了常见的位置参数还有关键词参数。
* [上下文管理](/post/python-basic-syntax-with.html) 实际上也就是常用的 with 语句，可以简化资源管理以及异常处理。

### 常用模块

* [Logging](/post/python-modules-logging.html) 用来打印日志信息。

### 杂七杂八

介绍一些乱七八糟的东西。

* [虚拟环境工作原理](/post/python-virtual-environment-details.html) 通过 virtualenv 可以创建独立 Python 工作环境，这里介绍其详细的原理。
* [内置模块和内置命名空间介绍](/post/python-builtins-module-and-builtin-namespace.html) 在 CPython 的实现中，包含了 builtins 模块，与内置命名空间的查找容易混淆。

### 常用连接

* [docs.python.org](https://docs.python.org/3/reference/index.html) 官方的 CPython 介绍文档，包含了详细概念。
* [PEP-8 Style Guide for Python Code](https://www.python.org/dev/peps/pep-0008/) Python 官方的编程规范。


<!--
### 常用模块

* [Python 基本概念](/post/python-basic-introduce.html)，常见问题，例如搜索路径。
* [Python2 VS. Python3](/post/python-version2-vs-version3-introduce.html) 两个版本之间的区别。
* [Python 基本语法](/post/python-basic-syntax-introduce.html)，一些常见容易混淆的概念，例如异常、Class等。
* [Python 的垃圾回收](/post/python-garbage-collection.html)，详细介绍 Python 特有的垃圾回收机制。
* [Python 动态执行](/post/python-eval.html)，允许通过 exec 和 eval 执行以字符串形式表示的代码片段，这里简单介绍。
* [Python ORM 简介](/post/python-orm-introduce.html)，简单介绍一些常用 ORM 工具，及其使用方法。

这里简单介绍一些常见的语法使用方式。

* [Python With 语句介绍](/post/python-basic-syntax-with-introduce.html)，方便的异常处理，可以使代码更加简洁。
* [Python DocString 介绍](/post/python-basic-syntax-docstring-introduce.html)，也就是一些注释信息，包括了模块、函数等。
* [Python 杂项](/post/python-tips.html)，记录了 Python 中常见技巧，一些乱七八糟的东西。
* [包安装工具详细介绍](/post/python-install-package-tools.html)
-->

## Flask

一个使用 Python 编写的轻量级 Web 应用框架，采用 BSD 授权。

* [Flask 简介](/post/flask-introduce.html)，简单介绍 flask 的安装、配置、使用，常用的三方模块等。
* [Flask 常见示例](/post/flask-tips.html)，包括了 Flask 中的一些常见示例，可以作为参考使用。
* [Nginx uWSGI Flask](/post/nginx-uwsgi-flask.html)，这里简单介绍如何通过 Nginx + uWSGI 搭建 Flask 运行环境。
* [Flask 请求处理流程](/post/flask-request-process.html)，介绍一次请求所经过的处理过程。
* [Flask 上下文理解](/post/flask-context.html)，主要介绍上下文、session 的使用以及源码的实现。
* [Flask 路由控制](/post/flask-route.html)，介绍 flask 中 URL 是如何进行路由的。
* [Flask 单元测试](/post/flask-unittest.html)，简单介绍对 flask 进行单元测试。
* [Flask Restful API](/post/flask-basic-restfull-api-and-test.html)，介绍如何使用 Restful-API 以及相关的测试工具。


<!--
* [Flask 完整例子](/post/flask-examples.html)，实际上就是 Flask 中的完整示例，包括了单元测试等相关的内容。
-->

## Others

记录乱七八糟的东西。

* [SaltStack 简介](/post/saltstack-introduce.html)，一个轻量级的运维工具，具备配置管理、远程执行、监控等功能。
* [Ansible 简介](/post/python-ansible.html)，一个配置管理工具，无需安装服务端和客户端，只要有ssh即可，而且使用简单。
* [Python 异步任务队列](/post/python-async-queue.html)，介绍一些常用的调度系统，如APScheduler、Redis Queue、Celery等。
* [ZeroMQ 简介](/post/zeromq-introduce.html)，一个 C++ 编写的高性能分布式消息队列，非常简单好用的传输层。


{% highlight text %}
{% endhighlight %}
