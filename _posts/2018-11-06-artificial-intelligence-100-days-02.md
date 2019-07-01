---
title: AI 100 天 -- 02
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->

可以通过如下代码生成测试数据。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as pylab
from sklearn.linear_model import LinearRegression

x = np.arange(0.0, 10.1, 0.1)
y = (2 * x - 5) + np.random.normal(scale=2.0**0.5, size=x.size)

pylab.scatter(x, y)
pylab.xlabel('x', fontsize=18)
pylab.ylabel('y', fontsize=18)
pylab.title('What our data looks like', fontsize=20)
pylab.show()
{% endhighlight %}

### 普通最小二乘法

也就是找到一条参数对应的曲线，使得所有数据具有最小均方根误差 (RMSE) 。

<!--
（OLS）线性回归
-->

可以直接使用 `Scikit-Learn` 中的 `LinearRegression` 模块，完整的代码如下。

{% highlight text %}
import numpy as np
import matplotlib.pyplot as pylab
from sklearn.linear_model import LinearRegression

x = np.arange(0.0, 10.1, 0.1)
y = (2 * x - 5) + np.random.normal(scale=2.0**0.5, size=x.size)

lr = LinearRegression()
lr.fit(x.reshape(-1, 1), y)

pylab.scatter(x,y)
pylab.plot(x, lr.coef_[0]*x + lr.intercept_, color='r')
pylab.xlabel('x',fontsize=18)
pylab.ylabel('y',fontsize=18)
pylab.show()
{% endhighlight %}

实际上这里缺少一个度量，也就是 置信界限 (confidence bounds)，当某个点的数据比较多时，认为这个范围内可信度大，而当数据较少时则不确定性增加。

#### 先验重要性

注意，先验 `P(θ)` 非常重要，必须依赖好的先验，当先验与后验越接近，便会更快得到真正的后验。如果取的先验分布和后验分布一致，那么当从先验分布中抽样时，实际上是从后验中取样，这也正是我们需要的。


这里的公式有如下的假设：数据点 (x,y) 是 Independent and Identically Distributed 独立同分布的。







最大似然估计(Maximum likelihood estimation)

https://www.cnblogs.com/liliu/archive/2010/11/22/1883702.html



## 贝叶斯线性回归

假设有如下的公式。

$y={\beta_1}x+{\beta_0}+{\epsilon}\ \ \ where\  {\epsilon} {\sim} N(\mu, \sigma^2)$

其中 $\epsilon$ 表示噪声，为正态分布。


P(θ|D) = P(D|θ)P(θ)/P(D)

`P(D|θ)` 也就是似然 (Likelihood) 函数，用来描述，在给定参数 `θ` 后，数据 `D` 出现的概率是多少。
`P(θ)` 开始对 θ 的估计，越接近于后验估计，越能更好更快的找到后验估计。
`P(D)` 数据 D 出现的概率，这是一个归一化的常数，用来确保右侧的后验概率分布是一个合理的概率密度。

### 先验估计 P(θ)

先验估计 `P(θ)` 是我们主观认为参数应该符合的概率分布，一般会使用高斯分布，相比来说高斯分布更容易简化计算。

假设随机变量 X 服从期望为 $\mu$、标准差为 $\sigma$ 的正态分布，则可以记为：  

$X \sim N(\mu ,\sigma ^{2})$

对应的概率密度函数为：

$$f(x)={1 \over {\sigma {\sqrt {2\pi }}}} {e^{-{(x-\mu )^{2} \over 2\sigma ^{2}}}}$$

在 python 中，可以通过 `numpy.random.normal()` 生成对应的数据。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

def get_normal_curve(x, mu, sigma):
    return (1.0/(sigma * np.sqrt(2.0 * np.pi))) * np.exp(-(x - mu)**2/(2.0 * sigma ** 2))

mu0, sigma0 = 0.0, 3.0
mu1, sigma1 = 0.0, 2.0

b0 = np.random.normal(loc=mu0, scale=sigma0, size=10000)
b1 = np.random.normal(loc=mu1, scale=sigma1, size=10000)

x = np.arange(-10.0, 10.0, 0.05)
f, (ax1, ax2) = plt.subplots(1, 2, sharey=True, figsize=(18, 8))
ax1.hist(b0, bins=x, normed=True)
ax1.plot(x, get_normal_curve(x, mu0, sigma0), linewidth=3.0)

ax2.hist(b1, bins=x, normed=True)
ax2.plot(x, get_normal_curve(x, mu1, sigma1), linewidth=3.0)
plt.show()
{% endhighlight %}

如何选择 `P(θ)` 是非常重要的，后验估计会依赖 `P(θ)` ，也就是说，`P(θ)` 越接近于后验估计，越能快速的得到期望的结果。

### 似然函数 P(D|θ)

所谓的似然函数 `P(D|θ)`，实际表示了在使用参数 `θ` 时，数据 `D` 出现的概率是多少。

接下来就是在已知参数的条件下，如何进行计算，也就是说，如何计算 `P(Y|X,θ)` 。

首先，假设各个数据点 $(x_i,y_i)$ 是独立同分布 (Independent and Identically Distributed) 的，那么有如下的等式。

$$P(Y|X,\theta)=P(Y|X,{\beta_1},{\beta_0})={\prod_{i=1}^N}{P(y_i|x_i,\beta_1,\beta_0)}$$

根据如上的线性函数，也就是：

$$y={\beta_1}x+{\beta_0}+{\epsilon}\ \ \ where\  {\epsilon} {\sim} N(\mu, \sigma^2)$$

带入之后，可以得到：

$$P(Y|X,\theta)={\prod_{i=1}^N}{P({\beta_1}{x_i}+{\beta_0}+{\epsilon_i}|x_i,\beta_1,\beta_0)}$$

因为 $x$ $\beta_0$ $\beta_1$ 现在已知，对应的条件概率实际上是 1 ，所以，上式可以简化为。

$$P(Y|X,\theta)={\prod_{i=1}^N}{P({\epsilon_i}|x_i,\beta_1,\beta_0)}$$

另外，假设噪音 $\epsilon$ 与数据是相互独立的，那么上述的等式可以继续简化为：

$$P(Y|X,\theta)={\prod_{i=1}^N}{P({\epsilon_i})}$$

如上，我们还假设噪音满足高斯分布，其对应的概率密度函数 (Probability Density Function, PDF) 如上所示，也就是。

$$f(x)={1 \over {\sigma {\sqrt {2\pi }}}} {e^{-{(x-\mu )^{2} \over 2\sigma ^{2}}}}$$

实际上，对于高斯分布的参数，还有两个假设：A) 均值为 0；B) 方差为固定常数，那么等式再次简化为：

$$P(Y|X,\theta)={\prod_{i=1}^N}{1 \over {\sigma_\epsilon {\sqrt {2\pi }}}} {e^{-{(\epsilon_i - 0)^2 \over 2\sigma_\epsilon^{2}}}}$$

另外，按照线性表达式，可以得到如下等式。

$${\epsilon_i}={y_i} - ({\beta_1} {x_i}+{\beta_0})$$

那么最终的等式为。

$$P(Y|X,\theta)={\prod_{i=1}^N}{1 \over {\sigma_\epsilon {\sqrt {2\pi }}}} {e^{-{({y_i} - ({\beta_1} {x_i}+{\beta_0}))^2 \over 2\sigma_\epsilon^{2}}}}$$

https://towardsdatascience.com/unraveling-bayesian-dark-magic-non-bayesianist-implementing-bayesian-regression-e4336ef32e61

https://blog.csdn.net/lanchunhui/article/details/50172659
https://blog.csdn.net/u010329855/article/details/75281346







<!--
https://cloud.tencent.com/developer/news/114376
-->



{% highlight text %}
{% endhighlight %}
