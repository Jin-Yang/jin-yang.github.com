---
title: 数据预处理
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: stan
description:
---


<!-- more -->


## 数据预处理
## 归一化

归一化一般是将数据映射到 `(0, 1)` 之间的小数，会把有量纲转换为无量纲的数据，也就是纯量。而标准化通常包含了三种方式：Z-Score、最大最小、小数定标法等等，

<!--
也就是通过不同的函数将数据集进行映射，常用的算法有：A) 线性函数转换 $y=(x-x_min)(x_max - x_min)$；B) 对数函数转换 $y=log_10(x)$；C) 反余切函数转换 $y=arccot(x)*2/\pi$ 。
-->

归一化和标准化的概念实在是太相似了，实在是分不清楚，索性还是直接借用 [WikiPedia Feature Scaling](https://en.wikipedia.org/wiki/Feature_scaling) 中的概念比较好，不再区分所谓的归一化以及标准化。

## Rescaling

https://en.wikipedia.org/wiki/Feature_scaling

感觉这篇文章讲的比较清晰
https://www.cnblogs.com/jasonfreak/p/5448385.html
https://zhuanlan.zhihu.com/p/29957294


## 标准化

### Z-Score

按照 Z-Score 计算的数据满足正态分布，而正态分布也被 Z 分布，所以该方法被称为 Z-Score 。

计算公式为 $z=\frac{x-\mu}{\delta}$ ，其中 $x$ 为观测值，$\mu$ 为总体均值，$\delta$ 为总体标准差，大于零表示大于均值，如果为 1 则表示数据比均值大一个标准差。

实际计算时，很难获取总体的均值和方差，一般会采用样本的特征值。

#### 示例

假设小明英语考了 90 分，语文考了 80 分，那么小红的英语和语文哪个考的好？

如果单纯比较分数，显然英语要好一些，但是因为难度不一样，所以单纯的看分数很难确定整体的排名，可以通过上述的 Z-Score 进行标准化。

假设，英语的均值和方差分别为 95 和 5 ，语文的均值和方差为 70 和 10 ，那么小明英语和语文的 Z-Score 成绩为 `-1` 和 `1` ，也就是说实际上语文的成绩要比英语成绩好。

https://upload.wikimedia.org/wikipedia/commons/2/25/The_Normal_Distribution.svg

#### Python

在 sklearn 中提供了两个方法。

import numpy as np
from sklearn import preprocessing

x = np.array([[1, -1, 2], [2, 0, 0], [0, 1, -1]])

# 直接将给定数据进行标准化
xs = preprocessing.scale(x)
print(xs.mean(axis=0))  # 均值，都为0
print(xs.std(axis=0))   # 方差，都为1

# 使用StandardScaler类进行解析
scaler = preprocessing.StandardScaler().fit(x)
print(scaler.mean_, scaler.var_)      # 查看样本的均值和方差
print(scaler.transform(x))            # 进行转换
print(scaler.transform([[-1, 1, 0]])) # 根据训练集对测试集进行转换

其中 `scale()` 会根据当前的数据进行计算，当使用 `StandardScaler` 类时，可以保存训练集中的特征值 (均值、方差)，然后直接转换测试集的数据，无需重复计算。

### 最小-最大规范化

Min-Max Normalization 也就是进行线性变换，将属性缩放到一个指定的最大和最小值 (通常是0~1) 之间。

#### Python

可以使用 `sklearn` 中的 `preprocessing.MinMaxScaler` 类实现。

scaler = preprocessing.MinMaxScaler()
xs = scaler.fit_transform(x)
print(scaler.scale_, scaler.min_)      # 缩放因子、最小值等属性
print(scaler.transform([[-3, -1, 4]])) # 将相同的缩放应用到测试集中

### 正则化 Normalization

正则化的过程是将每个样本缩放到单位范数（每个样本的范数为1），如果后面要使用如二次型（点积）或者其它核方法计算两个样本之间的相似性这个方法会很有用。

Normalization主要思想是对每个样本计算其p-范数，然后对该样本中每个元素除以该范数，这样处理的结果是使得每个处理后样本的p-范数（l1-norm,l2-norm）等于1。

#### Python

# 通过函数直接转换
print(preprocessing.normalize(x, norm='l2'))

# 使用类对训练集和测试集进行拟合和转换
norm = preprocessing.Normalizer().fit(x)
print(norm.transform(x))

https://www.cnblogs.com/hudongni1/articles/5499307.html
sklearn preprocessing
https://www.studyai.cn/modules/preprocessing.html

## 数据转换

在进行数据分析时，很多的模型要求数据需要呈现正态分布，当数据出现了倾斜后者分布不均衡的时候，就需要将数据变换为正太分布。

BOX-COX 变换
https://www.jianshu.com/p/a0bac114705b
https://www.mql5.com/zh/articles/363  关于BOX-COX很经典的介绍
https://www.leiphone.com/news/201801/T9JlyTOAMxFZvWly.html



特征工程
https://www.zhihu.com/question/29316149/answer/110159647



{% highlight text %}
{% endhighlight %}
