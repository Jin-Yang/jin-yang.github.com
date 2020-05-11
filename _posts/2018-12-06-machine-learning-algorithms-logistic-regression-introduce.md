---
title: 逻辑回归
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---

在机器学习里，有一类比较特殊的算法，就是逻辑回归 (Logistic Regression)，虽然被称为 "回归" 算法，实际上却是标准的解决分类问题的模型，也就是采用与回归类似的思路解决了分类问题，严格来说是二分类。

简单来说，就是建立代价函数，然后通过优化方法迭代求解出最优的模型参数。

<!-- more -->

## 简介

二分类就是根据给定的输入判断它是 A 类还是 B 类，这也是最简单的分类问题，例如判断邮件是否为垃圾邮件，线性回归输出的是一个数值而非标签，如果使用一个固定的阈值判断，那么就是感知机 (Perceptron) 。

逻辑回归没有直接预测标签，而是预测标签为 A 的概率，也就是在 $[0, 1]$ 区间，例如当大于 0.5 时就是 A 类，否则就是 B 类。

### 逻辑回归

逻辑回归实际上是一个函数与线性模型的合并，为什么？简单来说，线性回归模型 $f(x) = w^T x$ 的值域是 $(-\infty, +\infty)$ ，而概率则是在 $[0, 1]$ 区间，所以，可以通过一个函数进行影射。

常用的是 `sigmoid` 函数。

$$\sigma(x)=\frac{1}{1 + e^{-x}}$$

图如下所示。

![sigmoid]({{ site.url }}/images/ai/sigmoid-function.png "sigmoid"){: .pull-center width="60%" }

对应的代码为。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

x= np.linspace(-8, 8, 2000)
y = [1/(1 + np.exp(-i)) for i in x]

plt.xlim((-8, 8))
plt.ylim((0.00, 1.00))
plt.yticks([0, 0.5, 1.0],[0, 0.5, 1.0])
plt.plot(x, y, color='darkblue')

ax=plt.gca()
ax.spines['right'].set_color('none')
#ax.spines['top'].set_color('none')

ax.xaxis.set_ticks_position('bottom')
ax.yaxis.set_ticks_position('left')

ax.spines['left'].set_position(('data', 0))
ax.spines['bottom'].set_position(('data', 0))

plt.xlabel("sigmoid")
plt.show()
{% endhighlight %}

这个函数求导时有个很好的特性。





将线性回归模型的输出作为 `sigmoid` 函数的输入，那么最后就得到了逻辑回归模型。

$$y=\sigma(f(x))=\sigma(w^T x)=\frac{1}{1 + e^{-w^T x}}$$

当训练好了 $w^T$ 之后，且已知变量 $x$ 后，代入求解 $P(y=1\|x;w)$ 以及 $P(y=0\|x;w)$ 的概率，并选择较大的值作为分类结果，所以，关键是如何通过训练样本求得参数 $w^T$ 。

其中 $P(y=1\|x;w)$ 的意思是，在给定 $x$ 和 $w$ 的条件下 $y=1$ 的概率，一般会选择 $0.5$ 作为边界，当然，如果对正例的判别准确性要求高，可以选择阈值大一些。

### 损失函数

事件的结果只有两个标签 $y_n \in (0, 1)$ ，事件发生的概率 (也就是标签为 1 ) 为 $p$ ，也就是：

$$P_{y=1}=p=\frac{1}{1+e^{-w^Tx}}$$

那么，标签 $0$ 的概率为 $P_{y=0}=1-P$ ，可以直接写成：

$$P(y|x)=\begin{cases}p, & y=1 \\ 1-p, &y=0 \end{cases}$$

为了方便进行计算，也可以写成等价：

$$P(y_i|x_i)=p^{y_i}(1-p)^{1-y_i}$$

该函数的含义为，对于一个样本 $(x_i,y_i)$ ，标签是 $y_i$ 对应的概率是 $P(y_i\|x_i)$ ，对于一组数据，其样本概率 (或者是似然函数) 为：

$$P=P(x_1|y_1)P(x_2|y_2) \cdots P(x_n|y_n)=\prod_{i=1}^{n}p^{y_i}(1-p)^{1-y_i}$$

最终这是一个变参为 $w$ 的函数，可以直接使用最大似然求解参数，也就是一个优化问题。为了方便计算，直接取对数。

<!--
$$
\begin{align}
F(w)=ln(P)&=log\prod_{i=1}^n{\mu^{x_i}(1-\mu)^{1-x_i}} \\
&=\sum_{i=1}^n{log\{ \mu^{x_i}(1-\mu)^{1-x_i} \}} \\
&=\sum_{i=1}^n{\{log{(\mu^{x_i})}+log{(1-\mu)^{1-x_i}} \}} \\
&=\sum_{i=1}^n{\{x_i log{\mu}+({1-x_i})log{(1-\mu)} \}}
\end{align}
$$
-->



<!--
损失函数用来衡量模型的准确程度，线性回归使用的是最小二乘法，而逻辑回归采用的是最大似然估计。
-->


## 总结

<!--
优点：
1）速度快，适合二分类问题
2）简单易于理解，直接看到各个特征的权重
3）能容易地更新模型吸收新的数据
缺点：
对数据和场景的适应能力有局限性，不如决策树算法适应性那么强


逻辑回归和多重线性回归的区别

Logistic回归与多重线性回归实际上有很多相同之处，最大的区别就在于它们的因变量不同，其他的基本都差不多。正是因为如此，这两种回归可以归于同一个家族，即广义线性模型（generalizedlinear model）。
这一家族中的模型形式基本上都差不多，不同的就是因变量不同。这一家族中的模型形式基本上都差不多，不同的就是因变量不同。

    如果是连续的，就是多重线性回归
    如果是二项分布，就是Logistic回归
    如果是Poisson分布，就是Poisson回归
    如果是负二项分布，就是负二项回归

4. 逻辑回归用途

    寻找危险因素：寻找某一疾病的危险因素等；
    预测：根据模型，预测在不同的自变量情况下，发生某病或某种情况的概率有多大；
    判别：实际上跟预测有些类似，也是根据模型，判断某人属于某病或属于某种情况的概率有多大，也就是看一下这个人有多大的可能性是属于某病。


https://www.jianshu.com/p/2fb6919d5047
https://zhuanlan.zhihu.com/p/44591359
https://zhuanlan.zhihu.com/p/74874291
https://www.jianshu.com/p/0cfabca442d9
https://www.cnblogs.com/lxs0731/p/8573044.html
https://www.cnblogs.com/VitoLin21/p/11395753.html
-->


<!--
君主论
血酬定律
潜规则
爱的博弈
洞穴奇案
-->

{% highlight text %}
{% endhighlight %}
