---
title: 微积分基本概念
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

<!-- more -->

## 微积分

正常该学科的核心应该是微分和积分，只是很多《高等数学》讲解的是微分的一个性质导数。

对于一元函数 $y=f(x)$ 来说，其微分定义为 $dy=f'(x)dx$ ，那么如果将 $dx$ 看做自变量，那么函数 $dy=f'(x)dx$ 就代表了切线。

在一个很小的范围内，就可以通过直线代替曲线，也就是所谓的以直代曲，而洛必达法则、泰勒公式、积分基本定理、牛顿迭代法等，通过这一概念解释就会简单很多。

## 导数 Derivative

导数是微积分的形态之一，一般通过极限进行定义。

$$
f^{'}(x_0) = \lim_{\Delta x \to 0} \frac{\Delta y}{\Delta x} = \lim_{\Delta x \to 0} \frac{f(x_0 + \Delta x) - f(x_0)}{\Delta x}
$$

导数 $f^{'}(x_0)$ 反映的是函数 $f(x)$ 在点 $x_0$ 处沿 $x$ 轴正方向的变化率。

当 $x$ 的变化量 $\Delta x$ 趋于 0 时，记作微元 $dx$ 。

<!--
最初导数是通过极大极小值进行推导的，在后续的发展中，逐渐转换为通过极限进行定义。
https://www.zhihu.com/question/22199657/answer/115178055
https://zh.wikipedia.org/wiki/%E5%AF%BC%E6%95%B0#/media/File:Derivative_-_geometric_meaning.svg
-->

## 偏导数 Partial Derivative

偏导数是导数在多元函数上的扩展，两者的本质一样，都是当自变量的变化量趋于 $0$ 时，函数值的变化量与自变量变化量比值的极限。

对于曲线来说，其切线只有一条，但对于曲面来说，某个点的切线有无数条。直观地说，偏导数就是函数在某一点上沿坐标轴正方向的变化率。

* $f_x(x,y)$ 指的是函数在 $y$ 方向不变，函数值沿着 $x$ 轴方向的变化率；
* $f_y(x,y)$ 指的是函数在 $x$ 方向不变，函数值沿着 $y$ 轴方向的变化率。

以二元函数为例，相当于一个平面，那么平面上任意一点的切线会有很多条；偏导数就是选择其中一条，并求出它的斜率，最常见的是垂直于 $y$ 轴或者 $x$ 轴的切线。

## 全导数 total derivative

方向导数、偏导数是特殊的全导数

<!--
## 梯度下降 Gradient Descent
https://www.zybuluo.com/codeep/note/163962
-->





{% highlight text %}
{% endhighlight %}
