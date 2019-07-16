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

<!-- more -->

## 高斯分布

<!--
均值方差的推导
https://blog.csdn.net/su_jz/article/details/52579723
-->

假设随机变量 $x$ 服从一个均值为 $\mu$ ，标准差为 $\sigma$ 的正态分布，则可以记为：

$$X\sim N(\mu ,\sigma ^{2})$$

对应的概率密度函数为。

$$ f(x)={1 \over \sigma {\sqrt {2\pi }}}\,e^{-{(x-\mu )^{2} \over 2\sigma ^{2}}}$$

![normal distribution example]({{ site.url }}/images/ai/normal_distribution_example0.png "normal distribution example"){: .pull-center width="100%" }

该图是通过如下的代码生成。

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

x = np.linspace(-10, 20, num=200)
fig = plt.figure(figsize=(12, 6))
for mu, sigma, c in zip([0.5, 1, 2, 3, 4], [3, 1, 3, 2, 5], colors):
	y = stats.norm.pdf(x, mu, sigma)
	plt.plot(x, y, lw=2, c=c, label=r"$\mu={0:.1f}, \sigma={1:.1f}$".format(mu, sigma))
	plt.fill_between(x, y, color=c, alpha=.1)

plt.legend(loc=0)
plt.ylabel("PDF($x$)")
plt.xlabel("$x$")
plt.show()
{% endhighlight %}

注意，`matplotlib.mlab` 中的 `normpdf()` 已经不再支持了，可以使用 `scipy.stats.norm.pdf()` 函数替换。另外，也可以通过如下方式计算。

{% highlight text %}
def norm_pdf(x, mu, sigma):
    return np.exp(-((x - mu)**2)/(2*(sigma**2)))/(sigma*np.sqrt(2*np.pi))
{% endhighlight %}

### 随机采样

通过 numpy 中的 `random.normal()` 函数可以对正态分布进行采样。

![normal distribution sample]({{ site.url }}/images/ai/normal_distribution_sample.png "normal distribution sample"){: .pull-center width="70%" }

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

mu, sigma = 0, 1
size = 10000
np.random.seed(0)

s = np.random.normal(mu, sigma, size)
n, bins, patches = plt.hist(s, bins=100, density=True)
y = stats.norm.pdf(bins, mu, sigma)

plt.plot(bins, y, 'r--')
plt.xlabel('Expectation')
plt.ylabel('Probability')
plt.title('histogram of normal distribution: $\mu = 0$, $\sigma=1$')
plt.show()
{% endhighlight %}

### 多元正态分布

多元正态分布是一维正态分布向更高维度的推广，这种分布由其均值和协方差矩阵来确定，类似于一维正态分布中的均值和方差。

<!--
https://zhuanlan.zhihu.com/p/36982945
https://zhuanlan.zhihu.com/p/36522776
-->

## 参考


{% highlight text %}
{% endhighlight %}
