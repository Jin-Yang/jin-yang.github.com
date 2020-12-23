---
title: Python Scipy 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,scipy
description: Scipy 是一个用于数学、科学、工程领域的常用软件包，可以处理插值、积分、优化、图像处理、常微分方程数值解的求解、信号处理等问题。可以有效计算 Numpy 中的矩阵，从而使 Numpy 和 Scipy 可以协同工作，高效解决问题。
---

Scipy 是一个用于数学、科学、工程领域的常用软件包，可以处理插值、积分、优化、图像处理、常微分方程数值解的求解、信号处理等问题。

可以有效计算 Numpy 中的矩阵，从而使 Numpy 和 Scipy 可以协同工作，高效解决问题。

<!-- more -->

![scipy logo]({{ site.url }}/images/python/scipy-logo.png "scipy logo"){: .pull-center width="50%" }

## stats

在做 ML 的时候经常会遇到一些与概率相关的东西，其实在 `scipy.stats` 模块中已经提供了相关的一些特性，该模块封装了一些标准的随机变量，包括了连续型 (80多个) 和离散型 (10多个)。

### 常用方法

连续随机变量的主要公共方法有：

* rvs, Random VariateS 随机变量，也就是从这个分布中抽一些样本；
* pdf, Probability Density Function 概率密度函数；
* cdf, Cumulative Distribution Function 累计分布函数；
* sf, Survival Function 残存函数(1 - CDF)；
* ppf, Percent Point Function 分位点函数 (CDF 的逆)；
* isf, Inverse Survival Function 逆残存函数 (sf的逆)；

<!--
stats: Return mean, variance, (Fisher’s) skew, or (Fisher’s) kurtosis
moment: non-central moments of the distribution

stats:返回均值，方差，（费舍尔）偏态，（费舍尔）峰度。
moment:分布的非中心矩。
-->

可以通过如下方法查看支持的属性。

{% highlight text %}
from scipy import stats

dir(stats.norm)   # 查看正态分布的所有方法和属性
{% endhighlight %}

### 示例

如下使用该模块模拟正态分布的使用，默认使用的是标准正态分布，也就是平均值为 0 ，标准差为 1 的正态分布。

{% highlight text %}
>>> import numpy as np
>>> from scipy import stats

    #----- 使用默认的标准正态分布函数计算累积分布
>>> stats.norm.cdf(0)
0.5
>>> stats.norm.cdf([-1, 0, 1])
array([0.15865525, 0.5, 0.84134475])
>>> norm.cdf(np.array([-1, 0, 1]))
array([0.15865525, 0.5, 0.84134475])

    #----- 设置正态分布参数，loc(location)期望值，scale标准差
>>> x = stats.norm(loc=1.0, scale=2.0)

    #----- 计算随机变量的期望值和方差
>>> print(x.stats())
(array(1.), array(4.))

    #----- 对于正态分布随机取10000个值，并计算其均值和方差
>>> sample = x.rvs(size=10000)
>>> print(np.mean(sample), np.var(sample))
(0.9874473210585618, 4.086386197301687)

    #----- 对随机序列进行拟合，返回与随机取样值最吻合的随机变量参数
>>> print(stats.norm.fit(sample))
(0.9874473210585618, 2.0214811889556845)
{% endhighlight %}


<!--

NumPy库已经提供了一些基本的统计函数，如求期望、方差、中位数、最大值和最小值等。示例代码：

https://www.cnblogs.com/ttrrpp/p/6822214.html
SciPy Reference Guide
https://docs.scipy.org/doc/scipy/reference/
https://www.jianshu.com/p/abefd2d01684
https://zhuanlan.zhihu.com/p/36444829
https://stackoverflow.com/questions/29208440/fit-a-distribution-to-a-histogram
https://blog.csdn.net/baimafujinji/article/details/51374202
-->

## 参考

[Scipy Reference - Statistics scipy.stats](https://docs.scipy.org/doc/scipy/reference/tutorial/stats.html)

{% highlight text %}
{% endhighlight %}
