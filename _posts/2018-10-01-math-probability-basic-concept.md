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

简单整理介绍一些常见的概念。

##### 方差 (Variance)

方差(样本方差)是描述随机变量的离散程度，是变量离期望值的距离。

##### 标准差 (Standard Deviation)

也被称为均方差。

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

### 其它

#### 概率质量 VS. 概率密度

在概率论中，概率质量函数 (Probability Mass Function, PMF) 是离散随机变量在各特定取值上的概率；概率密度函数 (Probability Density Function, PDF) 用来描述该随机变量在某个确定的取值点附近可能性的函数。

两者的不同点在于: A) 概率质量函数是对离散随机变量定义的，本身代表该值的概率；B) 概率密度函数是对连续随机变量定义的，本身不是概率，只有对连续随机变量的概率密度函数在某区间内进行积分后才是概率。

对概率密度函数的积分又称为累积分布函数或者分布函数 (Cumulative Distribution Function, CDF)，用来描述一个实随机变量 `x` 的概率分布。


<!--
## 箱形图 Box and Whisker

是一种用作显示一组数据分散情况资料的统计图。

## 中心极限定理 Central Limit Theorem

给定一个任意分布的总体，每次从这些总体中随机抽取 n 个抽样，一共抽 m 次， 然后把这 m 组抽样分别求出平均值，这些平均值的分布接近正态分布。

其中需要注意：

1. 总体本身不要求是正态分布，可以是任意分布类型。
2. 样本每组要足够大，但也不需要太大。一般大于等于 30 即可。

相关的介绍
https://zhuanlan.zhihu.com/p/25241653

## 样本方差

用来解释为什么样本方差的分母是 N-1
https://www.zhihu.com/question/20099757
-->


{% highlight text %}
{% endhighlight %}
