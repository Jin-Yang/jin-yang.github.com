---
title: 基本初等函数
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

基本初等函数包括了：幂函数、指数函数、对数函数、三角函数、反三角函数五类。

<!-- more -->

## 三角函数

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(-np.pi, np.pi, 300, endpoint=True)
plt.plot(x, np.cos(x))
plt.plot(x, np.sin(x))
plt.show()
{% endhighlight %}


## 指数函数

其定义如下。

$$y=a^x \ (a>0, a \neq 1)$$

其中 $a$ 为常数，具有如下特性：A) $y$ 永远为正数；B) 当 `x=0` 时，对应 `y=1` 。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

x = np.arange(-10, 10, 0.1)

y1 = 2**x
y2 = 2**(-x)
#y3 = np.exp(x)

plt.plot(x, y1, color='green', label='$2^x$')
plt.plot(x, y2, color='red', label='$2^{-x}$')
plt.legend()
plt.show()
{% endhighlight %}

如果要计算底为自然数的指数，可以使用 `numpy.exp()` 函数，其入参可以是数组，而对于 `math.exp()` 的入参只能是单个的数值。



{% highlight text %}
{% endhighlight %}
