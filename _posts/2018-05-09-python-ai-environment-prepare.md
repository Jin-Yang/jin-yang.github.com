---
title: Python AI 环境准备
layout: post
comments: true
language: chinese
category: [linux,python]
keywords: linux,numpy
description: 目前大部分的 AI 相关工具包、框架等都是通过 Python 实现的，包括了常用的 Numpy、Scikit-Learn、TensorFlow、PyTorch 等等。所以，这里搭建的环境主要也就是 Python 相关的。
---

目前大部分的 AI 相关工具包、框架等都是通过 Python 实现的，包括了常用的 Numpy、Scikit-Learn、TensorFlow、PyTorch 等等。

所以，这里搭建的环境主要也就是 Python 相关的，另外还有基本的 MNIST 数据集。

<!-- more -->

## Python 环境

这里主要是最基本的依赖包。

### 常用包

在 Python 的数据挖掘或者 ML 中，经常遇到如下的几种包。

#### Numpy

Python 没有提供数组，一般使用列表 (List) 代替使用，不过当数据量增加时会明显的变慢。Numpy 提供了数组支持，可以有效的提高处理速度，核心部分通过 C/C++ 实现，同时很多高级扩展包依赖它。

#### Scipy

提供矩阵支持，以及矩阵相关的数值计算模块。

#### Pandas

这个是 Panel Data 的简写，提供了强大的数据分析和探索工具，因金融数据分析工具而开发，支持类似 SQL 的数据增删改查，支持时间序列分析，灵活处理缺失数据。

#### Scikit-Learn

用于数据挖掘和数据分析的简单且有效的工具，它的基本功能主要被分为六个部分：分类(Classification)、回归(Regression)、聚类(Clustering)、数据降维(Dimensionality Reduction)、模型选择(Model Selection)、数据预处理(Preprocessing)。

#### Matplotlib

主要用于绘图和绘表，强大的数据可视化工具，另外，Seaborn 也是数据可视化的工具包。


### 安装

为了防止一些包冲突，可以通过 virtualenv 创建一个临时的环境。在安装时，建议使用 `pip` 命令而非 `yum` ，一般来说后者的包会比较老。

{% highlight text %}
----- 生成临时环境
$ mkdir -p ~/Workspace/tensorflow && cd ~/Workspace
$ virtualenv --no-site-packages tensorflow
$ source tensorflow/bin/activate

----- 会同时安装像Tkinter这类的库
# yum install python-tools

----- 安装TensorFlow
$ pip install --upgrade tensorflow

----- 安装Numpy
$ pip install --upgrade numpy

----- 安装Matplotlib
$ pip install --upgrade matplotlib
# yum install python-matplotlib

----- 安装Scikit-Learn
$ pip install -U scikit-learn

----- 安装StatsModels
$ pip install statsmodels
$ conda install statsmodels

{% endhighlight %}

然后通过如下代码进行测试。

{% highlight text %}
$ python
>>> import tensorflow as tf
>>> hello = tf.constant("Hello TensorFlow")
>>> sess = tf.Session()
>>> print sess.run(hello)
Hello TensorFlow
>>> a = tf.constant(10)
>>> b = tf.constant(32)
>>> print sess.run(a + b)
42
{% endhighlight %}

对于 Numpy 源码可以直接从 [www.numpy.org](http://www.numpy.org/) 上下载，如果是离线则下载完包之后直接通过如下命令安装。

{% highlight text %}
$ python setup.py build
$ python setup.py install
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
