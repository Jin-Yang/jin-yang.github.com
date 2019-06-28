---
title: 高斯分布简介
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description: Normal distribution 正态分布 也称为 Gaussian Distribution 高斯分布，这是一个在数学、物理、工程等领域都非常重要的概率分布，在统计学的很多方面都有着重大影响力。
---

`Normal distribution 正态分布` 也称为 `Gaussian Distribution 高斯分布`，这是一个在数学、物理、工程等领域都非常重要的概率分布，在统计学的很多方面都有着重大影响力。

<!-- more -->

## 简介

![normal distribution pdf]({{ site.url }}/images/math/normal_distribution_pdf.png "normal distribution pdf"){: .pull-center width="70%" }

## Python

如下示例，生成均值为 0 方差为 1 的正态分布，可以进行测试，可以查看其均值和方差。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

mean = 0
sigma = 1
arr = np.random.normal(mean, sigma, size=1000)
abs(mean - np.mean(arr)) < 0.01
abs(sigma - np.std(arr, ddof = 1)) < 0.01

plt.plot(arr)
plt.show()
{% endhighlight %}

在如上程序计算样本方差时，因为 `ddof=1` 也就意味者计算的是样本方差的无偏估计。

另外，也可以通过如下的方式绘制一个正弦 `sin` 曲线，同时增加一些高斯噪声信息。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

mean = 0
sigma = 0.5
xarr = np.arange(0.0, 10.0, 0.01)
garr = np.random.normal(mean, sigma, size=1000)

plt.plot(xarr, 10 * np.sin(2 * np.pi * xarr) + garr)
plt.show()
{% endhighlight %}

<!--
## 误差(Error) VS. 残差(Residual)

* 误差是指样本对母本(无法观察到的)均值及真实值的均值的偏离。
* 残差则是指样本和观察值(样本总体)或回归值(拟合)的差额。 

## 白噪声

要求时序序列是 独立同分布(Independent and Identically Distributed) 且其均值为 0，如果符合高斯分布，那么就是高斯白噪声。
-->

<!-- 白噪声 VS. 高斯白噪声 -->



{% highlight text %}
{% endhighlight %}
