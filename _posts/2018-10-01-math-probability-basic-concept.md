---
title: 概率论基本概念
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

<!-- more -->

## 基本概念

随机试验满足三个特征：A) 可以在相同条件下重复进行；B) 实验结果不止一个，而且事先可以明确所有结果；C) 实验前无法确定出现哪一个结果。



在统计学中，存在样本以及总体的概念区别，两者的区别在于是否可以获取到全部事件集合。

比如，我们要统计某个班学习成绩的均值和方差，因为整个班的成绩是都知道的，那么也就无需采样，直接计算即可。但是，如果我们要统计国内男性身高的均值和方差，获取全部结果太难，那么就只能进行采样。

一般通过 $N$ 表示全部事件数，通过 $n$ 表示样本数。

### 数据特征

简单来说，就是描述随机事件的整体分布情况，例如一维中的均值、方差、标准差等，多维的还会涉及协方差。

#### 均值 Mean

均值表示了数据集平均大小，总体和样本的计算方式相同，只是表示略有区别。假设整体 (或样本) 集合中的某个元素通过 $x_i$ 表示，那么整体均值 $\mu$ 和样本均值 $\bar{x}$ 表示如下。

$$\mu = \frac{1}{N} \sum_{i=1}^{N} x_i$$

$$\bar{x} = \frac{1}{n} \sum_{i=1}^{n} x_i$$

在某些条件下，均值很难体现数据的特征，也可以通过中位值 (Median 排序后中间位置的值)、众数 (Mode 出现频次最高的数值)、百分位数 (Percentile 排序后在某个百分比处的值) 等标识。

#### 方差 Variance

方差主要是为了衡量随机变量或者一组数据的离散程度，是变量离期望值的距离，整体方差 $\sigma^2$ 与样本方差 $S^2$ 略有区别。

$$\sigma^2=\frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^2$$

$$S^2=\frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})^2$$

标准差 (Standard Deviation) 也被称为均方差，就是方差的平方根。

### 多维

#### 协方差

方差是用来度量单个随机变量的离散程度，而协方差则一般用来刻画两个随机变量的相似程度，模仿方差的定义，协方差一般定义如下。

$$\sigma(x,y)=\frac{1}{n-1}\sum_{i=1}^{n}(x_i-\bar{x})(y_i-\bar{y})$$

其中 $\bar{x}$ 和 $\bar{y}$ 分别表示两个随机变量的样本均值，而方差 $\sigma^2_x$ 可以看做随机变量 $x$ 关于自身的协方差 $\sigma(x,x)$ 。

#### 协方差矩阵

假设有 $d$ 个随机变量 $x_k,k=1,2,\cdots,d$ ，那么这些随机变量的方差为。

$$\sigma^2_k=\sigma(x_k,x_k)=\frac{1}{n-1}\sum_{i=1}^{n}(x_{ki}-\bar{x}_k)^2, k=1,2,\cdots,d$$

其中，通过 $x_{ki}$ 表示随机变量 $x_k$ 中的第 $i$ 个观测样本，$n$ 表示样本数量，每个随机变量的样本数均为 $n$ 。

对于这些随机变量，还可以根据协方差的定义，分别求出两两之间的协方差。

$$\sigma(x_m,x_k)=\frac{1}{n-1}\sum_{i=1}^{n}(x_{mi}-\bar{x}_m)(x_{ki}-\bar{x}_k)$$

然后，协方差矩阵就可以定义为。

$$
\Sigma=
\begin{bmatrix}
\sigma(x_1,x_1) & \cdots & \sigma(x_1,x_d)  \\
\vdots & \ddots & \vdots \\
\sigma(x_d,x_1) & \cdots & \sigma(x_d,x_d) \\
\end{bmatrix}
\in \Bbb{R}^{d \times d}
$$

对角线上的元素为各个随机变量的方差，非对角线上的元素为两两随机变量之间的协方差，为对称矩阵 (Symmetric Matrix)。


### 其它

##### 均方误差 (Mean Squared Error)

它是"误差" (每个估计值与真实值的差) 的平方的期望值，也就是多个样本的时候，均方误差等于每个样本的误差平方再乘以该样本出现的概率的和。

##### 协方差 (Covariance)

用来描述同向程度，协方差的数值越大，两个变量同向程度也就越大。

##### 概率分布函数 (Probability Distribution Function)

用来描述一个取值对应出现的概率，包括了离散和随机。

##### 概率密度函数 (Probability Density Function)

概率密度函数是概率分布函数的导数。

##### 互斥 (Mutually Exclusive)

几个变量或事件之中的任一个不可能与其它一个或多个同时为真，或同时发生的情况。

#### 概率质量 VS. 概率密度

在概率论中，概率质量函数 (Probability Mass Function, PMF) 是离散随机变量在各特定取值上的概率；概率密度函数 (Probability Density Function, PDF) 用来描述该随机变量在某个确定的取值点附近可能性的函数。

两者的不同点在于: A) 概率质量函数是对离散随机变量定义的，本身代表该值的概率；B) 概率密度函数是对连续随机变量定义的，本身不是概率，只有对连续随机变量的概率密度函数在某区间内进行积分后才是概率。

对概率密度函数的积分又称为累积分布函数或者分布函数 (Cumulative Distribution Function, CDF)，用来描述一个实随机变量 `x` 的概率分布。


## 中心极限定理

中心极限定理 (Central Limit Theorem) 是指给定一个任意分布的总体，每次从这些总体中随机抽取 `n` 个样本，一共抽 `m` 次，那么把这 `m` 组抽样分别求出平均值，这些平均值的分布接近正态分布。

需要注意两点：A) 总体本身不要求是正态分布；B) 每组样本要足够大，但也不需要太大，一般大于 30 个即可。

### 示例

采用类似掷骰子的例子，随机取值范围为 `1~6` ，然后每组取 `50` 个，然后取 `50000` 组作为样本。

{% highlight python %}
#!/bin/python
# -*- coding: UTF-8 -*-
import numpy as np
import matplotlib.pyplot as plt

data = np.random.randint(1, 7, 10000)
#print(data.mean())  # 3.5
#print(data.var())

mean = []

def sample(data, nums):
	tmp = []
	for j in range(0, nums):
		tmp.append(data[int(np.random.random() * len(data))])
	return np.array(tmp)

for i in range(1, 50000):
	s = sample(data, 50)
	mean.append(s.mean())
plt.hist(mean, bins=100, color='blue', alpha=0.75)
plt.show()
{% endhighlight %}

<!--
https://blog.csdn.net/anshuai_aw1/article/details/82769742
https://zhuanlan.zhihu.com/p/25241653
-->

## 其它

### 样本方差

所谓方差包含了样本方差和总体方差，对应的分母略有区别，前者分母为 $n - 1$ 后者 为 $n$ 。

在概率统计中，大部分求解的都是样本方差，此时需要构造一个统计量的样本方差 (注意，这实际上是一个随机变量)，那么就需要保证这里所构造统计量的期望与总体方差相等，这样才能使统计量具有无偏性。

<!--
#### 贝赛尔修正

也就是上述的样本方差计算方式。在计算样本方差时，实际上是将样本方差视为了一个随机事件，根据中心极限定理，可以将样本方差的结果作为整体方差。

用来解释为什么样本方差的分母是 N-1
https://www.zhihu.com/question/20099757

https://www.matongxue.com/madocs/607/
https://blog.csdn.net/Hearthougan/article/details/77859173
-->


{% highlight text %}
{% endhighlight %}
