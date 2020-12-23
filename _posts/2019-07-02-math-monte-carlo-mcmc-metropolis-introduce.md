---
title: MCMC 采样 - Metropolis
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: stan
description:
---

在介绍贝叶斯算法时，有讨论如何计算后验概率，适用于一些简单的分布，例如可以通过共轭先验进行简化。

但是对于一些复杂的场景，很难进行简化，那么就需要借助 MCMC 工具了。

<!-- more -->

## 简介

在介绍贝叶斯的时候，其证明过程实际上忽略掉了 $P(D)$ 的计算，两者采用的是概率等价，但是，如果要是实际计算，那么就需要统计 $P(D)$ 的值，而当在很高的维度时 (几千甚至上万)，其计算量就会变的非常大。

在通过 MCMC 计算的过程中，会忽略概率很小的点，主要计算概率大的点，这样对最终结果的影响会很小。

### 理论基础

MCMC 的理论基础是马尔可夫过程，为了在一个指定的分布上采样，从任一状态出发，模拟马尔可夫过程，不断进行状态转移，最终收敛到平稳分布。

### 处理流程

实际上，目前有很多关于 MCMC 算法的变种，不过，其处理的一般流程为。

1. 在参数空间中选择一个初始值；
2. 在参数空间中通过某种算法提议一个新值；
3. 根据先验信息和观测数据决定接收或者拒绝新的值；
4. 如果接受，则跳转到新的位置，并且返回到 Step1；
5. 如果拒绝，则保持当前位置并返回到 Step1；
6. 连续采用一系列点，最后返回接受的点集合。

不同的 MCMC 算法的区别在于：A) 如何进行跳转；B) 决定是否要进行跳转。

## Metropolis 算法

Metropolis 采用正态分布来进行跳跃，正态分布的 $\mu$ 为当前位置 $\theta_{current}$，而 $\sigma$ 可以自主选择，也就是该算法的参数。

在 $\sigma$ 采用不同的参数值时，决定了算法的收敛速度。

* 当 $\sigma$ 大则意味着能够跳的更远，并且搜索更多的后验参数空间，但是容易跳过高概率的地方；
* 当 $\sigma$ 过小，会导致收敛过慢。

### 实验

假设需要从标准柯西分布中进行采样，如下通过 Metropolis 算法来生成样本。

1. 设置初始的 $u$ 值，而且初始状态 $\theta^0=u$；
2. 根据正态分布选择下一个状态 $\theta^{(*)}=q(\theta\|\theta^{(t)})$；
3. 计算接收概率 $\alpha=min\left(1, \frac{p\left(\theta^{(*)}\right)}{p\left(\theta^{(t)}\right)} \right)$。
4. 从均匀分布 $(0,1)$ 生成一个随机值 $a$；
5. 如果 $a \leqslant \alpha$ 那么接受新生成的值 $\theta^{(t+1)}=\theta^{(*)}$，否则 $\theta^{(t+1)}=\theta^{(t)}$；
6. 重复第2步。

如下是对应的测试代码。

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt

def simple_cauchy(theta):
    return 1.0 / (1.0 + theta ** 2)

size = 5000
sigma = 10
(theta_min, theta_max) = (-30, 30)
u = stats.uniform.rvs(size=size)
theta = np.zeros(size)
theta[0] = stats.uniform.rvs(loc=theta_min, scale=theta_max - theta_min)

for i in range(size - 1): # [0, size)
    theta[i+1] = stats.norm.rvs(loc=theta[i], scale=sigma)
    alpha = min(1, simple_cauchy(theta[i+1])/simple_cauchy(theta[i]))
    if u[i] > alpha:
        theta[i+1] = theta[i]

plt.subplot(211)
plt.ylim(theta_min, theta_max)
plt.plot(range(size), theta, 'g-')

(x_min, x_max) = (int(min(theta)), int(max(theta)))
x = np.arange(x_min, x_max, (x_max - x_min) / size)
plt.subplot(212)
plt.hist(theta, 100, density=1, facecolor='red', alpha=0.5)
plt.plot(x, stats.cauchy.pdf(x, 0, 1), lw=1, c='blue')
plt.show()
{% endhighlight %}

![sampling]({{ site.url }}/images/ai/monte-carlo-method-metropolis-example.png  "sampling"){: .pull-center width="90%"}

<!--
介绍Metropolis采样算法
https://www.zybuluo.com/zhuanxu/note/1025594
https://blog.csdn.net/google19890102/article/details/51755242

Metropolis-Hastings
https://www.zybuluo.com/zhuanxu/note/1026672
https://blog.csdn.net/google19890102/article/details/51785156
https://twiecki.io/blog/2015/11/10/mcmc-sampling/
https://blog.csdn.net/google19890102/article/details/51755245
https://www.quantstart.com/articles/Markov-Chain-Monte-Carlo-for-Bayesian-Inference-The-Metropolis-Algorithm
-->

{% highlight text %}
{% endhighlight %}
