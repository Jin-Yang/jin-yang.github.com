---
title: 采样算法
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: stan
description: 在使用 Monte-Carlo 时，除了所谓的采样之外，在解决贝叶斯问题的时候，很多地方还使用了积分，这时就是通过 Monte-Carlo 的样本和去近似积分。而所谓的采样，实际上就是根据某种分布去生成一些数据点，最简单的例如抛硬币、掷骰子等，服从均匀分布。
---

在使用 Monte-Carlo 时，除了所谓的采样之外，在解决贝叶斯问题的时候，很多地方还使用了积分，这时就是通过 Monte-Carlo 的样本和去近似积分。

而所谓的采样，实际上就是根据某种分布去生成一些数据点，最简单的例如抛硬币、掷骰子等，服从均匀分布。

<!-- more -->

## 蒙特卡罗方法

Monte Carlo Method 也就是通过重复随机采样模拟对象的概率与统计的问题，在物理、化学、经济学和信息技术领域均具有广泛应用。

简单来说，就是通过随机采样的方式，以频率估计概率。

### PI 计算

假设要估计圆周率 `Pi` 的值，选取一个边长为 `1` 的正方形，在正方形内作一个内切圆，那么可以计算出圆的面积与正方形面积之比为 `Pi/4` 。

![pi example]({{ site.url }}/images/ai/monte-carlo-method-pi-example.png "pi example"){: .pull-center width="30%" }

在正方形内随机生成大量的点，那么圆形区域内点的个数与所有点的个数之比，就可以认为近似等于 `Pi/4` 。

其中采样值越大，越接近真实值 `3.141592653589793` 。

{% highlight python %}
import math
import random

inside = 0
total = 1000

for i in range(0, total):
        # generate random x, y in [0, 1].
        x = random.random()
        y = random.random()

        # increment if inside unit circle.
        if math.sqrt(x**2 + y**2) <= 1.0:
                inside += 1
# inside / total = pi / 4
pi = (float(inside) / float(total)) * 4
print(pi)
{% endhighlight %}


<!--
https://blog.csdn.net/jteng/article/details/54344766
-->

## 简单采样

对于均匀分布来说，就可以通过随机数发生器来实现，包括了伪随机数发生器，然后再以此为基础实现一些比较复杂的采样。

### 离散分布

可以把概率分布看做一个区间段，然后判断随机数 `u` 落在哪个区间段内，而区间段的长度和概率成正比，这样采样完全符合原来的分布。

例如有样本空间 `{A, B, C, D}` ，对应的概率为 `p(x)=[0.1, 0.5, 0.3, 0.1]` 。

{% highlight python %}
import numpy as np

def sample_discrete(vec):
	u = np.random.rand()
	start = 0
	for i, num in enumerate(vec):
		if u > start:
			start += num
		else:
			return i-1
	return i

sample_space = ["A", "B", "C", "D"]
sample = dict([(w, 0) for w in sample_space])
for i in range(10000):
    s = sample_discrete([0.1, 0.5, 0.3, 0.1])
    sample[sample_space[s]] += 1
for k in sample:
    print(k, " : ", sample[k])
{% endhighlight %}

### 正态分布

一般来说会使用 `Box-Muller` 变换，也就是通过服从均匀分布的随机变量，来构建服从高斯分布的随机变量的一种方法。

简单来说，选取两个服从 `[0, 1]` 上均匀分布的随机变量 `U1`、`U2` ，满足如下公示，那么 `X`、`Y` 满足均值为 0 ，方差为 1 的高斯分布。

$$X=cos(2{\pi}U_1)\sqrt{-2ln{U_2}} \\ Y=sin(2{\pi}U_1)\sqrt{-2ln{U_2}}$$

#### 公示推导

对于正态分布函数 (PDF) 有如下公式，其中 $\mu$ 是平均值，$\sigma$ 是标准差。

$$f(x)=\frac{1}{\sqrt{2\pi}\sigma}e^\frac{(x-\mu)^2}{2\sigma^2}$$

假设有两个随机变量

<!--
https://zhuanlan.zhihu.com/p/38638710
-->

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

def norm_sample(size, mu, sigma):
    u1 = np.random.rand(size)
    u2 = np.random.rand(size)

    x = np.sqrt(-2*np.log(u1))*np.cos(2*np.pi*u2)
    y = np.sqrt(-2*np.log(u1))*np.sin(2*np.pi*u2)

    x1 = mu + x * sigma
    y1 = mu + y * sigma

    return x1, y1

def norm_pdf(x, mu, sigma):
    return np.exp(-((x - mu)**2)/(2*(sigma**2)))/(sigma*np.sqrt(2*np.pi))

size = int(1e+05)
mu, sigma = 0, 1
x = np.arange(-4., 4., 0.1)

(y1, y2) = norm_sample(size, mu, sigma)

f, (ax1, ax2) = plt.subplots(1, 2, sharey=True)
ax1.plot(x, norm_pdf(x, mu, sigma), 'r', lw=2)
ax1.hist(y1, bins=200, density=True)

ax2.plot(x, norm_pdf(x, mu, sigma), 'r', lw=2)
ax2.hist(y2, bins=200, density=True)

plt.show()
{% endhighlight %}

## 参考

<!--
机器学习中的Monte-Carlo，写的还是不错的
https://applenob.github.io/1_MCMC.html
-->

{% highlight text %}
{% endhighlight %}
