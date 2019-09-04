---
title: PyMC 使用简介
layout: post
comments: true
language: chinese
usemath: true
category: [program,misc]
keywords: pymc
description: PyMC3 是一个用 Python 编写的开源的概率编程框架，完全通过 Python 代码来定义模型，并使用 Theano 通过变分推理进行梯度计算，并使用 C 实现加速运算。目前的 Theano 库已经不再维护，而 PyMC3 团队会单独维护与 PyMC3 相关的一些特性。
---

PyMC3 是一个用 Python 编写的开源的概率编程框架，完全通过 Python 代码来定义模型，并使用 Theano 通过变分推理进行梯度计算，并使用 C 实现加速运算。

目前的 Theano 库已经不再维护，而 PyMC3 团队会单独维护与 PyMC3 相关的一些特性。

<!-- more -->

## 简介

### 安装

可以直接通过 `pip install pymc3` 进行安装，不过 PyMC3 依赖 Python3 。

## 示例

### 广义线性模型

Generalized Linear Model, GLM 一个简单的贝叶斯线性回归模型，其参数具有正态分布先验，其中观测值 $Y$ 具有正态分布，其期望 $\mu$ 是两个预测变量的线性组合，也就是 $X_1$ 和 $X_2$ 。

PyMC3 中的模型定义语言与统计中的公式很接近，一般是一条语句对应一个公式。

{% highlight text %}
import numpy as np
import matplotlib.pyplot as plt

np.random.seed(123)

alpha=1
sigma=1
beta =[1, 2.5]

N=100

X1=np.random.randn(N)
X2=np.random.randn(N)

Y = alpha + beta[0]*X1 + beta[1]*X2 + np.random.randn(N)*sigma

import pymc3 as pm

basic_model = pm.Model()
with basic_model:
    alpha=pm.Normal('alpha',mu=0,sd=10)
    beta=pm.Normal('beta',mu=0,sd=10,shape=2)
    #sigma=pm.Normal('sigma',sd=1)
    sigma=pm.HalfNormal('sigma',sd=1)

    mu=alpha+beta[0]*X1+beta[1]*X2

    Y_obs=pm.Normal('Y_obs', mu=mu,sd=sigma,observed=Y)
    trace = pm.sample(2000)
    pm.summary(trace)
    pm.traceplot(trace)

plt.show()

{% endhighlight %}


{% highlight text %}
WARNING (theano.tensor.blas): Using NumPy C-API based implementation for BLAS functions.
Auto-assigning NUTS sampler...
Initializing NUTS using jitter+adapt_diag...
Multiprocess sampling (2 chains in 2 jobs)
NUTS: [sigma, beta, alpha]
Sampling 2 chains: 100%|████████████| 5000/5000 [00:10<00:00, 495.47draws/s]
{% endhighlight %}

<!--
https://blog.csdn.net/jackxu8/article/details/71080865

PyMC3通用线性模型
https://blog.csdn.net/jackxu8/article/details/71154502

https://www.bilibili.com/read/cv1324586/

http://yangli.name/2018/11/10/20181110pymc3/
-->


## 参考

* [GitHub PyMC3](https://github.com/pymc-devs/pymc3) 官方网站以及帮助文档 [PyMC3 Getting Started](https://docs.pymc.io/getting_started) 。

<!--



只写了一部分的书籍，可以配合上面的<2>示例书籍
https://github.com/markdregan/Bayesian-Modelling-in-Python

2. Bayesian Methods for Hackers 基于PyMC3
https://github.com/CamDavidsonPilon/Probabilistic-Programming-and-Bayesian-Methods-for-Hackers

[pandas.pydata.org docs visualization](https://pandas.pydata.org/pandas-docs/version/0.19.0/visualization.html) 详细介绍 Pandas 中支持的数据展示方法。

如何选择图像的数据展示方式
https://cloud.tencent.com/developer/article/1041505
https://www.datadoghq.com/blog/timeseries-metric-graphs-101/
https://homes.cs.washington.edu/~jheer//files/zoo/

https://blog.csdn.net/jackxu8/article/details/71080865
-->



{% highlight text %}
{% endhighlight %}
