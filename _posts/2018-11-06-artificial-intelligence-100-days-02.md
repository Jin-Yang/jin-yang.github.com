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

## 线性回归

简单来说，现在有一批的数据，示例如下。

![linear regression example data]({{ site.url }}/images/ai/linear-regression-example-data.png "linear regression example data"){: .pull-center }

可以通过如下代码生成如上的测试数据。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as pylab

x = np.arange(0.0, 10.1, 0.1)
y = (2 * x - 5) + np.random.normal(scale=2.0**0.5, size=x.size)

pylab.scatter(x, y)
pylab.xlabel('x', fontsize=15)
pylab.ylabel('y', fontsize=15)
pylab.title('What our data looks like', fontsize=12)
pylab.show()
{% endhighlight %}

我们希望通过这些数据来拟合出一条直线。

$$y=\beta_1x+\beta_0$$

也就是要确定 $\beta_1$ 和 $\beta_0$ 的值，一般来说，最常用的计算方法是获取这些数据最小的 Root Mean Squared Error, RMSE ，也即。

$$\beta_1,\beta_0=argmin_{\beta_1,\beta_0}\sum_{i=1}^{N}(y_i - (\beta_1 x_i + \beta_0))^2$$

### 普通最小二乘法

如上所述，也就是找到一条参数对应的曲线，使得所有数据具有最小均方根误差 (RMSE)，可以直接使用 `Scikit-Learn` 中的 `LinearRegression` 模块，完整的代码如下。

{% highlight python %}
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

在如上的处理时，sklearn 要求输入的特征必须是二维数组的类型，但目前只有 1 个特征，需要用 `reshape()` 转行成二维数组的类型。

最终得到的直线如下所示。

![linear regression example]({{ site.url }}/images/ai/linear-regression-scikit-learn-example.png "linear regression example"){: .pull-center }

实际上这里缺少一个度量，也就是 置信界限 (confidence bounds)，当某个点的数据比较多时，认为这个范围内可信度大，而当数据较少时则不确定性增加。

有点类似于下图。


接下来介绍，如何通过贝叶斯进行线性估计。

## 贝叶斯线性回归

可以将贝叶斯公式修改为如下所示。

$$P(θ|D) = \frac{P(D|θ)P(θ)}{P(D)}$$

简单来说，该等式意味这，当给定数据 D 之后，参数 $\beta_0=0.5$ 且 $\beta_1=2.0$ 的概率是 0.5 ，而 $\beta_0=1.5$ 且 $\beta_1=3.0$ 的概率是 0.1 。

也就是说，后者只是在当前数据集出现的概率要低于前者，而非不可能出现。

其中的几个参数含义如下。

* $P(D\|θ)$ 似然函数，用来描述，在给定参数 $\theta$ 后，数据 $D$ 出现的概率是多少。
* $P(θ)$ 开始对 $\theta$ 的估计，越接近于后验估计，越能更好更快的找到后验估计。
* $P(D)$ 数据 $D$ 出现的概率，这是一个归一化的常数，用来确保右侧的后验概率分布是一个合理的概率密度。

假设有如下的数据模型。

$y={\beta_1}x+{\beta_0}+{\epsilon}\ \ \ where\  {\epsilon} {\sim} N(\mu, \sigma^2)$

其中 $\epsilon$ 表示噪声，为正态分布。

假设 $\beta_0$ 和 $\beta_1$ 满足正态分布，但是对应的参数有所区别。

$$\beta_0 \sim N(\mu=0, \sigma^2=9) \\ \beta_1 \sim N(\mu=0, \sigma^2=5)$$

### 先验 P(θ)

先验估计 `P(θ)` 是我们主观认为参数应该符合的概率分布，一般会使用高斯分布，相比来说高斯分布更容易简化计算。

注意，先验 `P(θ)` 非常重要，必须依赖好的先验，当先验与后验越接近，便会更快得到真正的后验。如果取的先验分布和后验分布一致，那么当从先验分布中抽样时，实际上是从后验中取样，这也正是我们需要的。

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
ax1.hist(b0, bins=x, density=True)
ax1.plot(x, get_normal_curve(x, mu0, sigma0), linewidth=3.0)

ax2.hist(b1, bins=x, density=True)
ax2.plot(x, get_normal_curve(x, mu1, sigma1), linewidth=3.0)
plt.show()
{% endhighlight %}

![linear regression parameters]({{ site.url }}/images/ai/linear-regression-parameters.png "linear regression parameters"){: .pull-center width="100%" }

如何选择 `P(θ)` 是非常重要的，后验估计会依赖 `P(θ)` ，也就是说，`P(θ)` 越接近于后验估计，越能快速的得到期望的结果。

### 似然函数 P(D|θ)

所谓的似然函数 $P(D\|\theta)$，实际表示了在使用参数 $\theta$ 时，数据 $D$ 出现的概率是多少。

接下来就是在已知参数的条件下，如何进行计算，也就是说，如何计算 $P(Y\|X,\theta)$ 。

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

如上，我们还假设噪音满足高斯分布，其对应的概率密度函数也就是。

$$f(x)={1 \over {\sigma {\sqrt {2\pi }}}} {e^{-{(x-\mu )^{2} \over 2\sigma ^{2}}}}$$

实际上，对于高斯分布的参数，还有两个假设：A) 均值为 0；B) 方差为固定常数，那么等式再次简化为：

$$P(Y|X,\theta)={\prod_{i=1}^N}{1 \over {\sigma_\epsilon {\sqrt {2\pi }}}} {e^{-{(\epsilon_i - 0)^2 \over 2\sigma_\epsilon^{2}}}}$$

另外，按照线性表达式，可以得到如下等式。

$${\epsilon_i}={y_i} - ({\beta_1} {x_i}+{\beta_0})$$

那么最终的等式为。

$$P(Y|X,\theta)={\prod_{i=1}^N}{1 \over {\sigma_\epsilon {\sqrt {2\pi }}}} {e^{-{({y_i} - ({\beta_1} {x_i}+{\beta_0}))^2 \over 2\sigma_\epsilon^{2}}}}$$

这也就是最终的等式，然后对表达式进行简化。

$$f_n(x_i | \mu = \beta_1 x_i + \beta_0, \sigma^2=\sigma_{\epsilon}^2)={1 \over {\sigma_\epsilon {\sqrt {2\pi }}}} {e^{-{({y_i} - ({\beta_1} {x_i}+{\beta_0}))^2 \over 2\sigma_\epsilon^{2}}}} $$

也就是。

$$P(Y|X, \theta)=\prod_{i=1}^N f_n(x_i | \beta_1 x_i + \beta_0, \sigma_{\epsilon}^{2})$$


### 后验估计 P(θ|D)

也就是根据所观察到的数据，估计参数 (这里是 $\beta_0$ 和 $\beta_1$ ) 的概率密度函数。

$$P(\beta_1,\beta_0|X,Y)=\frac{P(Y|X,\beta_0,\beta_1) P(\beta_1,\beta_0)}{P(X,Y)}$$

其中 $P(X,Y)$ 是一个常量，这里通过 $Z$ 表示。

$$P(\beta_1,\beta_0|X,Y)=\frac{P(Y|X,\beta_0,\beta_1) P(\beta_1,\beta_0)}{Z}$$

有两种方式可以获取到对应的后验估计：A) 通过公式推导；B) 通过对参数 $\beta_0$ 和 $\beta_1$ 的采样获取。

这里采用第二种方法，也就有如下等式。




## 其它

### 似然函数

这里通过一个简单的示例介绍似然函数的作用。

暂时将上面的 $\theta_0$ 参数取消，同时假设 $\theta_1=4$ ，也就是对应的等式如下所示。

$$y_i=\beta_1 x_i + \epsilon_i \ where \ \beta_1=4 \ and \ \epsilon_i \sim N(\mu=0, \theta^2=1)$$

如果将 $\beta_1$ 设置一个取值范围，例如 $[0, 8]$，那么同样可以得到对应 $y$ 的一个范围，也就是。

$$\hat{y_i}=\hat{\beta_1} x_i + \epsilon_i \ where \ \hat{\beta_1}\in [0, 8] \ and \ \epsilon_i \sim N(\mu=0, \theta^2=1)$$

可以通过如下的代码得到对应的结果。



<!--
https://towardsdatascience.com/unraveling-bayesian-dark-magic-non-bayesianist-implementing-bayesian-regression-e4336ef32e61

https://blog.csdn.net/lanchunhui/article/details/50172659
https://blog.csdn.net/u010329855/article/details/75281346

https://cloud.tencent.com/developer/news/114376


这里的公式有如下的假设：数据点 (x,y) 是 Independent and Identically Distributed 独立同分布的。

最大似然估计(Maximum likelihood estimation)

https://www.cnblogs.com/liliu/archive/2010/11/22/1883702.html
-->



{% highlight text %}
{% endhighlight %}
