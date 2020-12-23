---
title: 线性回归 基本介绍
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---

回归分析 (Regression Analysis) 是确定两种或两种以上变量间相互依赖的定量关系的一种统计分析方法，线性回归 (Linear Regression) 也是最基本的。

这里介绍其概念、公式推导以及基本的实现。

<!-- more -->

## 简介

线性回归是机器学习中一个最基本的类型，试图从数据集中找到一个线性模型，这个模型可以准确反映出输入 $X_i$ 和输出 $Y_i$ 的对应关系。

假设每个数据样本有 $d$ 维，对应的线性函数如下。

$$f(x) = w_1 x_1 + w_2 x_2 + \cdots + w_d x_d + b$$

首先从一个简单的示例介绍。

### 线性回归

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

$$y=w_1 x + w_0$$

也就是要确定 $w_1$ 和 $w_0$ 的值，一般来说，最常用的计算方法是获取这些数据最小的 Root Mean Squared Error, RMSE ，也即。

$$w_1, w_0=arg \ min_{w_1, w_0} \sum_{i=1}^{N}(y_i - (w_1 x_i + w_0))^2$$

直接使用现成的库，后面在介绍直接使用 numpy 的方式。

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

这一问题涉及到了后面介绍的贝叶斯线性回归。

## 公式推导

### 一元推导

假设在一个样本集合 $D$ 中，有 $n$ 个样本 ${(x_1, y_1), (x_2, y_2), \cdots , (x_n, y_n)}$ 。

#### 代价函数

一般通过均方误差 (欧式距离) 对结果进行评估，也就是代价函数 (Cost Function) ，表示为如下。

$$(w^*, b^*)=arg \ min_{(w,b)} \sum_{i=1}^{m} (f(x_i) - y_i)^2$$

$arg \ min$ 是指后面表达式值最小时，对应 $(w, b)$ 的取值，实际上就是最小二乘法。接着就是要看如何求解参数，使得代价函数最小。

$$E(w, b) = \sum_{i=1}^m(y_i - wx_i - b)^2$$

其中，函数 $E$ 是关于参数 $(w, b)$ 的凸函数，那么这也就变成了一个凸优化问题，其中 $w$ 和 $b$ 是未知参数，使其对应的偏导为 $0$ 后，就可以得到最优解。

#### 公式推导

首先分别对 $w$ 和 $b$ 求偏导，注意，这里使用的是一元。

$$
\begin{align}
\frac{\partial{E(w,b)}}{\partial{w}}&=\frac{\partial{\sum_{i=1}^m(y_i-wx_i-b)^2}}{\partial{w}} \\
&=\frac{\partial{\sum_{i=1}^m((y_i-b)^2+w^2x_i^2-2w(y_i-b)x_i)}}{\partial{w}} \\
&= \sum_{i=1}^m(2wx_i^2-2(y_i-b)x_i) \\
&= 2(w\sum_{i=1}^m{x_i^2}-\sum_{i=1}^m(y_i-b)x_i)\\

\frac{\partial{E(w,b)}}{\partial{b}}&=\frac{\partial{\sum_{i=1}^m(y_i-wx_i-b)^2}}{\partial{b}} \\
&=\frac{\partial{\sum_{i=1}^m((y_i-wx_i)^2+b^2-2b(y_i-wx_i))}}{\partial{b}} \\
&= \sum_{i=1}^m(2b-2(y_i-wx_i)) \\
&= 2(mb-\sum_{i=1}^m{(y_i-wx_i)})
\end{align}
$$

然后令偏导 (也就是上述的结果) 为 $0$ 最终求解出结果。

$$
\begin{align}
w&=\frac{m\sum {x_i y_i}-\sum {x_i}\sum {y_i}}{m\sum {x_i^2}-(\sum x_i)^2} \\
b&=\frac{1}{m}\sum_{i=1}^{m}(y_i-wx_i)=\frac{\sum {x_i^2} \sum{ y_i}-\sum {x_i}\sum {x_i y_i}}{m\sum {x_i^2}-(\sum x_i)^2}
\end{align}
$$

### 高维推导

假设在一个样本集合 $D$ 中，有 $n$ 个样本 ${(X_1, Y_1), (X_2, Y_2), \cdots , (X_n, Y_n)}$ ，其中每个样本由 $d$ 个属性，可以表示为输入特征向量为 $X = (x_1, x_2, \cdots , x_d)^T$ ，对应模型的预测输出 $f(x)$ 可以表示为输入 $X$ 的线性函数，也就是。

$$f(x) = w_1 x_1 + w_2 x_2 + \cdots + w_d x_d + b$$

对应的矩阵形式为。

$$Y=f(x)=w^T x + b$$

其中 $x$ 和 $w$ 均为 $d \times 1$ 的列向量，其中 $w$ 代表权重 (Weight) 的意思。接着将 $w$ 和 $b$ 合并为 $\theta$ 。

https://blog.csdn.net/caicaiatnbu/article/details/93770442
https://zhuanlan.zhihu.com/p/48255611
http://bourneli.github.io/linear-algebra/calculus/2016/04/30/linear-algebra-12-linear-regression-matrix-calulation.html
https://zhuanlan.zhihu.com/p/33899560
https://zr9558.com/2016/10/28/linearregression/


## 梯度下降

上面讨论的都是解析解，也就是直接通过公式进行计算，接着讨论的是通过梯度下降进行求解。


https://zhuanlan.zhihu.com/p/25434586








<!--
https://towardsdatascience.com/unraveling-bayesian-dark-magic-non-bayesianist-implementing-bayesian-regression-e4336ef32e61

https://blog.csdn.net/lanchunhui/article/details/50172659
https://blog.csdn.net/u010329855/article/details/75281346

https://cloud.tencent.com/developer/news/114376


这里的公式有如下的假设：数据点 (x,y) 是 Independent and Identically Distributed 独立同分布的。

最大似然估计(Maximum likelihood estimation)

https://www.cnblogs.com/liliu/archive/2010/11/22/1883702.html



## 线性回归

对于线性回归问题，通常有两种解决方法，最小二乘法和梯度下降法；而最小二乘法又有两种求解思路，代数求解和矩阵求解。

https://cloud.tencent.com/developer/article/1594856
https://huhuhang.com/post/machine-learning/linear-regression-ols-gradient-descent

https://zhuanlan.zhihu.com/p/79852058
https://www.jianshu.com/p/fb9712b6e5ea
https://zhuanlan.zhihu.com/p/47476638
https://blog.csdn.net/cuihuijun1hao/article/details/78211225
-->



{% highlight text %}
{% endhighlight %}
