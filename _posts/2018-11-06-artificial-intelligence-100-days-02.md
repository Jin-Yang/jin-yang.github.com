---
title: AI 100 天 -- 02
layout: post
comments: true
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


<!--
https://cloud.tencent.com/developer/news/114376
-->


{% highlight text %}
{% endhighlight %}
