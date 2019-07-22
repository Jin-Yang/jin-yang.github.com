---
title: 马尔科夫简介
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->


## 马尔科夫

对于连续时间，称之为马尔科夫随机过程；而离散时间称为马尔科夫链。

对马尔科夫链来说，当前的状态仅与上一个状态有关。

### 转移概率矩阵

$$
P_{ij} =
        \begin{pmatrix}
        p_{11} & p_{12} & \cdots & p_{1j} \\
        p_{21} & p_{22} & \cdots & p_{2j} \\
        \vdots & \vdots & \ddots & \vdots \\
        p_{i1} & p_{i2} & \cdots & p_{ij} \\
        \end{pmatrix}
$$

其中状态转移矩阵的每一行的和为 1 。

$$\sum_{j=1}^\infty P_{ij}=1$$

### 细致平衡条件

细致平衡条件 (Detailed Balance Condition) 的定义为，在给定一个马尔科夫链后，如果分布 $\pi$ 和概率转移矩阵 $P$ 满足下面的等式。

$${\pi_i}P_{ij}={\pi_j}P_{ji}$$

则此马尔科夫链具有一个平稳分布 (Stationary Distribution) $\pi$ 。

注意，这是一个充分条件而非必要条件，也就是说存在平稳分布的马尔科夫链，不满足此细致平衡条件。

<!--
https://blog.csdn.net/bitcarmanlee/article/details/82819860
https://blog.csdn.net/guolindonggld/article/details/79597491
http://www.twistedwg.com/2018/06/07/MCMC-method.html
https://blog.csdn.net/bitcarmanlee/article/details/82819860
-->


{% highlight text %}
{% endhighlight %}
