---
title: 连续概率分布
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

介绍一些常见的概率分布。

<!-- more -->

## 柯西分布

其概率密度对应了如下的函数。

$$
\begin{align}
f(x; x_0,\gamma) &= \frac{1}{\pi \gamma \left[{1+(\frac{x-x_0}{\gamma})^2} \right]} \\
&=\frac{1}{\pi} \left[\frac{\gamma}{(x-x_0)^2+\gamma^2} \right]
\end{align}
$$

其中 $x_0$ 和 $\gamma$ 分别定义了位置参数以及尺度参数，对应的累积分布函数为。

$$F(x; x_0,\gamma) = \frac{1}{\pi} arctan \left( \frac{x-x_0}{\gamma} \right) + \frac{1}{2}$$

当 $x_0=0, \gamma=1$ 时，被称为标准柯西分布，其概率密度函数为：

$$f(x; 0,1) = \frac{1}{\pi (1+x^2)}$$

![cauchy distribution]({{ site.url }}/images/ai/cauchy_distribution_example.png "cauchy distribution"){: .pull-center width="90%" }

可以通过如下代码生成上述的图像。

{% highlight python %}
#!/bin/python
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

x = np.linspace(-10, 10, num=200)
fig = plt.figure(figsize=(12, 6))
for x0, gamma, c in zip([0, 0, 0, -2, 2], [0.5, 1, 2, 1, 1], colors):
    y = stats.cauchy.pdf(x, x0, gamma)
    plt.plot(x, y, lw=2, c=c, label=r"$x_0={0:.1f}, \gamma={1:.1f}$".format(x0, gamma))
    plt.fill_between(x, y, color=c, alpha=.1)

plt.legend(loc=0)
plt.ylabel("PDF($x$)")
plt.xlabel("$x$")
plt.show()
{% endhighlight %}

另外，正态分布的采样还可以使用 Ziggurat 算法，改算法基于拒绝采样。

## Beta 分布

Beta Distribution 是指定义在 $(0, 1)$ 区间的连续概率分布，有两个参数 $\alpha > 0, \beta > 0$ 。

对应的概率密度函数为。

$$
\begin{align}
f(x; \alpha,\beta) &= \frac{x^{\alpha - 1} (1 - x) ^ {\beta - 1}} {\int_0^1 u^{\alpha - 1} (1 - u)^{\beta - 1} du} \\
&= \frac{\Gamma(\alpha + \beta)}{\Gamma(\alpha) \Gamma(\beta)} x^{\alpha - 1} (1 - x)^{\beta - 1} \\
&= \frac{1}{B(\alpha, \beta)} x^{\alpha - 1} (1 - x)^{\beta - 1}
\end{align}
$$

如果随机变量 $X$ 服从参数为 $\alpha,\beta$ 的贝塔分布，那么通常使用 $X \sim Beta(\alpha, \beta)$ 表示。

其中 $\Gamma(z)$ 是伽马函数，改函数是阶乘在实数和复数域上的扩展，如果 $n$ 为正整数，那么：

$$\Gamma(n)=(n-1)!$$

对于实数部分为正的复数 $z$ ，那么伽马函数的定义为：

$$\Gamma(z)=\int_0^{\infty} \frac{t^{z-1}}{e^t}dt$$

Beta 分布对应的期望和方差分别为。

$$
\begin{align}
E(X) &= \frac{\alpha}{\alpha + \beta} \\
Var(X) &= \frac{\alpha \beta}{(\alpha + \beta)^2 (\alpha + \beta + 1)}
\end{align}
$$

另外，需要注意，在贝叶斯推断中，Beta 分布是 Bernoulli、二项分布、负二项分布和几何分布的共轭先验分布。

对应累积分布函数为。

$$F(x; \alpha, \beta)=\frac{B_x(\alpha, \beta)}{B(\alpha, \beta)}=I_x(\alpha, \beta)$$

其中 $B_x(\alpha, \beta)$ 是不完全 $B$ 函数，而 $I_x(\alpha, \beta)$ 是不完全贝塔函数。

![beta distribution]({{ site.url }}/images/ai/beta_distribution_example.png "beta distribution"){: .pull-center width="80%" }

可以通过如下代码生成上述的图像。

{% highlight python %}
#!/bin/python
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

x = np.linspace(0, 1, num=200)
fig = plt.figure(figsize=(12, 6))
for alpha, beta, c in zip([0.5, 5, 1, 2, 2], [0.5, 1, 3, 2, 5], colors):
    y = stats.beta.pdf(x, alpha, beta)
    plt.plot(x, y, lw=2, c=c, label=r"$\alpha={0:.1f}, \beta={1:.1f}$".format(alpha, beta))
    plt.fill_between(x, y, color=c, alpha=.1)

plt.legend(loc=0)
plt.ylabel("PDF($x$)")
plt.xlabel("$x$")
plt.show()
{% endhighlight %}



<!--
Beta分布
https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.beta.html
https://www.zhihu.com/question/30269898
https://www.jianshu.com/p/6ee90ba47b4a
Gamma函数的来源
https://cosx.org/2013/01/lda-math-gamma-function/

https://blog.csdn.net/watkinsong/article/details/46348853

Exponential 指数分布 -> Gamma 分布 -> Beta 分布
-->

{% highlight text %}
{% endhighlight %}
