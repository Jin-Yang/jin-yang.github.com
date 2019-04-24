---
title: 数理统计基本概念
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

<!-- more -->


### 经验分布

经验分布 Empirical Distribution Functions, EDF 。

如上所述，统计学大部分研究的就是如何通过样本来评估总体分布函数。

假设有样本 $X_1, X_2, \cdots, X_n$ ，其对应的总体分布函数为 $F$ ，那么最简答的方法就是将样本看成一个随机变量的所有取值，且取每个值的概率为 $\frac{1}{n}$ ，该随机变量的分布就是经验分布，所对应的分布函数 $F_n$ 称为经验分布函数。

根据 Glivenko–Cantelli Theorem 格利文科定理，随着样本数 $n$ 的增加，$F_n$ 会一直逼近 $F$，也就是说可以将 $F_n$ 看做是 $F$ 的近似。

<!--
https://zhuanlan.zhihu.com/p/27920193
https://blog.csdn.net/baimafujinji/article/details/51720090
-->



{% highlight text %}
{% endhighlight %}
