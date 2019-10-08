---
title: 基本优化算法
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords:
description:
---


<!-- more -->

## 最优化

### 函数求根

或者称为方程的根，或函数的零点。

最优化一般就是求函数的最大/小值，一般也是极值，那么在这个函数的最大/小值时，它所在的位置的导数为 0 。

因此，求最优化的时候，可以先求导，解得函数的零点，从而得到函数的最大/小值。

可以用 `scipy.optimize` 模块中的 `bisect()` 或 `brentq()` 函数来求根，后者会更快一些。

{% highlight python %}
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

func = lambda x: np.cos(x) - x
x = np.linspace(-5, 5, 1000)
y = func(x)
r = opt.brentq(func, -5, 5) # OR opt.bisect(f, -5, 5)

plt.plot(x, y)
plt.axhline(0, color='k')
plt.scatter([r], [0], c='r', s=100)
plt.show()
{% endhighlight %}

### 函数最小化

求最小值就是一个最优化问题，如果需要求最大值，实际只需对函数做一个转换，比如加一个负号、取倒数，都可以转成求最小值问题。

在 `scipy` 中提供了一个 `minimize()` 通用函数，取代了以前的很多最优化函数，是个通用的接口，背后有很多方法支撑。

{% highlight python %}
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

func = lambda x: 1 - np.sin(x) / x
x = np.linspace(-20, 20, 1000)
y = func(x)

x0 = 3
xmin = opt.minimize(func, x0).x
#x0 = 10
#xmin = opt.basinhopping(func, x0, stepsize=5).x

plt.plot(x, y)
plt.scatter(x0, func(x0), marker='o', s=100)
plt.scatter(xmin, func(xmin), marker='v', 1=300)
plt.show()
{% endhighlight %}

当初始值为 3 时，可以成功找到最小值；但是当初始值为 10 时，却只能找到局部的最小值点。

另外，可以使用 `basinhopping()` 找到全局的最优点，其实现的算法可以查看官方文档。

### 曲线拟合

当给定一组数据后，它可能是沿着一条线散布，曲线拟合就是找到一条最优曲线来拟合这些数据，一般是指这些点和线之间的距离最小。

在模拟的时候呈现一个线性关系，这里找到一条斜线来拟合这些点，这也就是经典的一元线性回归。

其目标是找到直线，使点和线的距离最短，也就是最优的斜率和截距。

{% highlight python %}
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

def deviations(theta, x, y):
    return (y - theta[0] - theta[1] * x)

# squared error
def sefunc(theta, x, y):
    return np.sum(((y - theta[0] - theta[1] * x)) ** 2)

size = 50        # 点个数
(m, b) = (2, -1) # 斜率和截距
dy = 2.0         # 标准差

np.random.seed(10)
x = 10 * np.random.random(size) # 服从均匀分布
y = np.random.normal(b + m * x, dy)

theta = opt.minimize(sefunc, [0, 1], args=(x, y)).x
#theta, _ = opt.leastsq(deviations, [0, 1], args=(x, y))

xhat = np.linspace(0, 10, num=size)
yhat = theta[0] + theta[1] * xhat

plt.errorbar(x, y, dy, fmt='.k', ecolor='lightgray')
plt.plot(xhat, yhat, '-k')
plt.show()
{% endhighlight %}

上述求解采用的是误差平方和，实际上有现成的最小二乘法可以使用，也就是上面的 `leastsq()` 方法。

<!--
#### 非线性最小二乘
在帮助文档中，提供了很多相关算法的介绍
https://docs.scipy.org/doc/scipy/reference/optimize.html
https://www.cnblogs.com/NaughtyBaby/p/5590081.html
-->

{% highlight text %}
{% endhighlight %}
