---
title: 离散概率分布
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

<!-- more -->

## 伯努利分布

也就是 Bernoulli Distribution，又名两点分布或者 `0-1` 分布，是一个离散型概率分布，主要是为纪念瑞士科学家 雅各布·伯努利 而命名。

最简单的离散概率，在一次试验中，事件 A 出现的概率为 $$p(0\le p \le 1)$$，不出现的概率为 $q=1-p$，那么相应的概率分布为：

$$f_X(x)=p^x(1-p)^{1-x}=\begin{cases}p&\text{if x=1}\\q&\text{if x=0}\end{cases}$$

对应的期望为：

$$E[X]=\sum_{i=0}^1 x_i f_X(x)=0 + p = p$$

方差为：

$$var[X]=\sum_{i=0}^1(x_i -E[X])^2 f_X(x)=(0-p)^2(1-p)+(1-p)^2p=p(1-p)=pq$$

也就是最简单的离散概率分布，只有一个独立的事件，只可能出现两种结果。

## 二项分布

这个是在伯努利分布基础上的概率分布，在 N 次独立的伯努利试验中，期望某个结果 (事件发生了多少次) 出现的概率。

也就是说，在单次试验中，结果 A、B 出现的概率分别为 $p$ 、$q$，且 $p+q=1$ ，那么如果 `n=10` 即在 10 次实验中，结果出现 0 次、1 次、...、10 次的概率分别是多少？这样的概率分布呈现出什么特征呢？这就是二项分布所研究的内容。

例如，对于一枚硬币，其出现正面和反面的概率各为 0.5 ，那么如果掷 3 次，一般会出现 8 种结果，分别是 正正正、正正反、正反正、反正正、正反反、反正反、反反正、反反反。每个结果出现的概率是 0.125 ，那正面出现 3 次、2 次、1 次、0 次的概率分别是 0.125、0.375、0.375、0.125。

实际上，二项分布是统计学家在分析实验结果的基础上的一种总结，其计算概率的一般公式为：

$$b(x,n,p)=C_n^xp^xq^{n-x}$$

其中，$n$ 表示实验总的次数，$x$ 表示出现某个结果的次数，$p$ 表示时间出现的概率。$C_n^x$ 为组合，一般也表示为 $$\begin{pmatrix} n \\ x \\ \end{pmatrix}$$，其计算公式如下：

$$C_n^x=\frac{n\times(n-1)\times\cdots\times(n-x+1)}{x\times(x-1)\times\cdots\times1}=\frac{n!}{(n-x)!x!}$$

注意，二项分布是建立在有放回抽样的基础上的，也就是抽出一个样品测量或处理完后再放回去，然后抽下一个。不过现实中一般都是非放回抽样，这时就需要用超几何分布来计算概率。

## 泊松分布

也就是 Poisson Distribution ，其对应的概率质量函数如下。

$$P(X=k)=\frac{\lambda^k e^{-\lambda}}{k!},\ k=0,1,2,..., \ \lambda \in \mathbb{R}_{>0}$$

若 $X$ 服从参数为 $\lambda$ 的泊松分布，则记为 $X \sim \pi(\lambda)$ 或者 $X \sim P(\lambda)$ 。

![poisson distribution pmf]({{ site.url }}/images/math/poisson_distribution_pmf.png "poisson distribution pmf"){: .pull-center width="80%" }

上图对应的代码如下。

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

k = np.arange(20)
fig = plt.figure(figsize=(12, 6))
colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

for lambda_, c in zip([1.0, 4.0, 6.5, 8.0, 10], colors):
    y = stats.poisson.pmf(k, lambda_)
    #plt.bar(k, y, lw=2, edgecolor=c, color=c, alpha=0.2, label=r"$\lambda=%.1f$" % lambda_)
    plt.plot(k, y, lw=2, marker='o', color=c, alpha=0.8, label=r"$\lambda=%.1f$" % lambda_)

plt.legend(loc=0)
plt.ylabel("PMF($k$)")
plt.xlabel("$k$")
plt.title("Probability mass function of a Poisson random variable; differing $\lambda$ values")
plt.show()
{% endhighlight %}

其数学期望和方差相等，均等于其参数 $\lambda$ ，也就是 $E(X)=Var(X)=\lambda$ 。

<!--
有关于GB/T2828中的介绍
https://zhuanlan.zhihu.com/p/24692791

PS. 高尔顿钉板实际上是符合二项分布的。

## 超几何分布

与二项分布不同的是，这里采用的是非放回抽样。

## 泊松分布 VS. 指数分布

在日常生活中，可以将大量事件解释成有固定频率，例如：

某医院平均每小时出生3个婴儿
某公司平均每10分钟接到1个电话
某超市平均每天销售4包xx牌奶粉
某网站平均每分钟有2次访问

它们的特点就是，可以通过统计预估这些事件的总数，但是没法知道具体的发生时间。已知平均每小时出生 3 个婴儿，请问下一个小时，会出生几个？有可能一下子出生 6 个，也有可能一个都不出生。


泊松分布的概率质量函数为：

$$P(X=k)=\frac{e^{-\lambda}\lambda^k}{k!}$$


实际上，泊松分布可以通过二项分布进行模拟。

### 参考

泊松分布的现实意义是什么，为什么现实生活多数服从于泊松分布？
https://www.zhihu.com/question/26441147

## 指数分布 Exponential Distribution

指数分布是事件的时间间隔的概率。下面这些都属于指数分布。

婴儿出生的时间间隔
来电的时间间隔
奶粉销售的时间间隔
网站访问的时间间隔

指数分布的公式可以从泊松分布推断出来。如果下一个婴儿要间隔时间 t ，就等同于 t 之内没有任何婴儿出生。


## Beta Distribution

贝塔分布是一个连续的概率分布，它只有两个参数，一般用于某项实验的成功概率建模。

https://blog.csdn.net/watkinsong/article/details/46348853



排列 Arrangement 、组合 Combination
https://zhuanlan.zhihu.com/p/41855459
https://www.zhihu.com/question/26094736
-->




## 参考


{% highlight text %}
{% endhighlight %}
