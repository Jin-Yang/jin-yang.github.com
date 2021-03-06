---
title: 连续概率 -- 正态分布
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

`Normal distribution 正态分布` 也称为 `Gaussian Distribution 高斯分布`，这是一个在数学、物理、工程等领域都非常重要的概率分布，在统计学的很多方面都有着重大影响力。

<!-- more -->

## 简介

<!--
均值方差的推导
https://blog.csdn.net/su_jz/article/details/52579723

高斯分布概率密度函数（PDF）和累积分布函数(CDF)
https://blog.csdn.net/renwudao24/article/details/44465407
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

通过 numpy 中的 `random.normal()` 函数可以对正态分布进行采样。如下示例，生成均值为 0 方差为 1 的正态分布，可以进行测试，可以查看其均值和方差。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

mean = 0
sigma = 1
arr = np.random.normal(mean, sigma, size=1000)
abs(mean - np.mean(arr)) < 0.01
abs(sigma - np.std(arr, ddof = 1)) < 0.01

plt.plot(arr)
plt.show()
{% endhighlight %}

在如上程序计算样本方差时，因为 `ddof=1` 也就意味者计算的是样本方差的无偏估计。

另外，也可以通过如下的方式绘制一个正弦 `sin` 曲线，同时增加一些高斯噪声信息。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

mean = 0
sigma = 0.5
xarr = np.arange(0.0, 10.0, 0.01)
garr = np.random.normal(mean, sigma, size=1000)

plt.plot(xarr, 10 * np.sin(2 * np.pi * xarr) + garr)
plt.show()
{% endhighlight %}

<!--
## 误差(Error) VS. 残差(Residual)

* 误差是指样本对母本(无法观察到的)均值及真实值的均值的偏离。
* 残差则是指样本和观察值(样本总体)或回归值(拟合)的差额。 

## 白噪声

要求时序序列是 独立同分布(Independent and Identically Distributed) 且其均值为 0，如果符合高斯分布，那么就是高斯白噪声。
-->

### 采样示例

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

## 多元高斯分布

多元高斯分布 (Multivariate Gaussian Distribution) 的形式很简单，就是一元高斯分布的在向量形式中的推广，把向量 $X=[x_1, x_2,\cdots, x_n]^T$ 称作是均值为 $\mu \in {\bf R}^n$ ，协方差矩阵为 $\Sigma \in S^n$ 的多元高斯分布，其概率密度函数的形式为。

$$f_x(x_1, \cdots ,x_n)=\frac{1}{\sqrt{(2 \pi)^n |\Sigma|}} e^{-\frac{(x-\mu)^T (x-\mu)}{2\Sigma}}$$

这种分布由均值和协方差矩阵确定，类似于一维正态分布中的均值和方差，可以看到，与一维的高斯分布有两点不同。

* 多维高斯公式把一维中的 $(x - \mu)^2$ 替换成了 $(x - \mu)^T (x - \mu)$ 矩阵操作，而该公式也可以写成 $(x_1 - \mu_1)^2 + (x_1 - \mu_1 )^2 + \cdots + (x_n - \mu_n )^2$，那么一维就只对应了 $(x_1 - \mu_1 )^2$ 。
* 多维高斯分布把方差 $\sigma^2$ 换为了协方差矩阵 $\Sigma$，也就是说到了高维之后，不仅仅有各维自己的方差，还有两两之间的协方差。

协方差反应两个变量的相关程度，将方差和协方差放到一个矩阵中就是协方差矩阵 $\Sigma$ 。


<!--
https://drivingc.com/p/5b793eac2392ec689a39c56b
-->

### 其它

在 Python 中，可以通过 `numpy.random.multivariate_normal()` 方法用于根据实际情况生成一个多元正态分布矩阵，该函数定义如下：

{% highlight text %}
multivariate_normal(mean, cov, size=None, check_valid=None, tol=None)
    mean        1*N 均值
    cov         N*N 协方差矩阵，对称且为半正定的矩阵。
    size        生成数据的维度，例如size=(10, 100则输出的矩阵为10*100*N
    check_valid 检查协方差矩阵是否合法。
{% endhighlight %}

如下是个一个参考的示例。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

## Gaussian Distribution
#cov = np.eye(1) * 3
#mean = np.array([3])
#Y = np.random.multivariate_normal(mean, cov, 10, 'raise')

## Multivariate Gaussian Distribution
cov = np.eye(2) * 3
mean = np.array([3, 100])
Y = np.random.multivariate_normal(mean, cov, (10, 3), 'raise')
{% endhighlight %}

如上生成了一个 `10 * 3 * 2` 的数据，因为协方差矩阵使用的是对角矩阵，也就意味着各个变量之前是独立的，其中均值对应 `mean` 变量，方差就都是 `1` ，可以通过如下方式计算均值、方差等信息。

{% highlight text %}
>>> np.mean(Y, axis=0)  # 计算均值，同样可以计算方差等信息
array([[  2.96771613,  99.65476306],
       [  2.8397508 ,  99.88631836],
       [  3.09584658, 100.50960993]])
>>> np.std(Y, axis=0)
array([[0.98685019, 0.99880091],
       [0.98338639, 1.02355389],
       [1.00234377, 0.98393021]])
{% endhighlight %}

如下是一个示例。

#### 二元绘图

![normal distribution 2d example]({{ site.url }}/images/ai/normal_distribution_2d_display01.png "normal distribution 2d example"){: .pull-center width="60%" }

实现代码如下。

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable

size = 10000
colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

male_mu = np.array([172, 60])
male_sigma = np.array([[36, 15], [15, 9]])
male = np.random.multivariate_normal(male_mu, male_sigma, size)

female_mu = np.array([165, 56])
female_sigma = np.array([[25, 12], [12, 8]])
female = np.random.multivariate_normal(female_mu, female_sigma, size)

fig = plt.figure(figsize=(8, 8))
axes = fig.gca()  # get current axes
axes.scatter(*female.T, c='red', label='female', s=10, alpha=0.1);
axes.scatter(*male.T, c='blue', label='male', s=10, alpha=0.1);

def figure_rug(axes, x, y, color='blue'):
	xlims = axes.get_xlim()
	ylims = axes.get_ylim()

	zx1 = np.zeros(len(x))
	zx2 = np.zeros(len(x))
	zx1.fill(ylims[1])
	zx2.fill(xlims[1])

	axes.plot(x, zx1, marker='|', color=color, markersize=8, alpha=0.2)
	axes.plot(zx2, y, marker='_', color=color, markersize=8, alpha=0.2)

	axes.set_xlim(xlims)
	axes.set_ylim(ylims)

figure_rug(axes, female.T[0], female.T[1], 'red')
figure_rug(axes, male.T[0], male.T[1], 'blue')

axes.grid(False)
axes.legend(loc='upper left');
plt.show()
{% endhighlight %}

数据与上面相同，只是绘制的方式有所区别。

![normal distribution 2d example]({{ site.url }}/images/ai/normal_distribution_2d_display02.png "normal distribution 2d example"){: .pull-center width="60%" }

{% highlight python %}
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable

size = 10000
colors = ['deepskyblue', 'red', 'green', 'blue', 'purple']

male_mu = np.array([172, 60])
male_sigma = np.array([[36, 15], [15, 9]])
male = np.random.multivariate_normal(male_mu, male_sigma, size)

female_mu = np.array([165, 56])
female_sigma = np.array([[25, 12], [12, 8]])
female = np.random.multivariate_normal(female_mu, female_sigma, size)

fig = plt.figure(figsize=(8, 8))
axes = fig.gca()  # get current axes
axes.scatter(*female.T, c='red', label='female', s=10, alpha=0.1);
axes.scatter(*male.T, c='blue', label='male', s=10, alpha=0.1);

def remove_border(axes=None, top=False, right=False, left=True, bottom=True):
	ax = axes or plt.gca()
	ax.spines['top'].set_visible(top)
	ax.spines['right'].set_visible(right)
	ax.spines['left'].set_visible(left)
	ax.spines['bottom'].set_visible(bottom)
	
	#turn off all ticks
	ax.yaxis.set_ticks_position('none')
	ax.xaxis.set_ticks_position('none')
	
	#now re-enable visibles
	if top:
		ax.xaxis.tick_top()
	if bottom:
		ax.xaxis.tick_bottom()
	if left:
		ax.yaxis.tick_left()
	if right:
		ax.yaxis.tick_right()
def make_mhist(axes, x, y, color='blue'):
	ax2 = axes[1]
	ax3 = axes[2]
	for tl in (ax2.get_xticklabels() + ax2.get_yticklabels() +
			ax3.get_xticklabels() + ax3.get_yticklabels()):
		tl.set_visible(False)
	style = {'histtype':'step', 'color':color, 'alpha':0.4}
	ax2.hist(x, np.linspace(np.min(x), np.max(x), 200), **style)
	ax3.hist(y, np.linspace(np.min(y), np.max(y), 200), orientation='horizontal', **style)

divider = make_axes_locatable(axes)
axesx = divider.append_axes("top", 1.5, pad=0.0, sharex=axes)
axesy = divider.append_axes("right", 1.5, pad=0.0, sharey=axes)
axesx.grid(False)
axesy.grid(False)

remove_border(axesx, right=True, left=False)
remove_border(axesy, right=False, left=True, bottom=False, top=True)

make_mhist([axes, axesx, axesy], male.T[0], male.T[1], color='blue')
make_mhist([axes, axesx, axesy], female.T[0], female.T[1], color='red')

fig.subplots_adjust(left=0.15, right=0.95)
axes.grid(False)
axes.legend(loc='upper left');
plt.show()
{% endhighlight %}

<!--
https://zhuanlan.zhihu.com/p/36982945
https://zhuanlan.zhihu.com/p/36522776

https://zhuanlan.zhihu.com/p/44860862
https://zhuanlan.zhihu.com/p/32315762
https://zhuanlan.zhihu.com/p/58987388

https://borgwang.github.io/ml/2019/07/28/gaussian-processes.html
https://drivingc.com/p/5b793eac2392ec689a39c56b
https://www.zybuluo.com/fsfzp888/note/1462244
https://www.visiondummy.com/2014/04/geometric-interpretation-covariance-matrix/
-->

{% highlight text %}
{% endhighlight %}
