---
title: 梯度下降算法
layout: post
comments: true
usemath: true
language: chinese
category: [program,linux,misc]
keywords:
description:
---


<!-- more -->




## 梯度下降法 Gradient Descent

所谓的梯度实际上就是多元函数的 "导数"，对于 $n$ 元函数 $f(x_1,x_2, \cdots, x_n)$ 的梯度是一个长度为 $n$ 的向量，其中向量中第 $k$ 个元素为函数 $f$ 对 $x_k$ 的偏导数。

$$
\nabla f(x_1, x_2, \cdots, x_n) = \left( \frac{\partial f}{\partial x_1}, \frac{\partial f}{\partial x_2}, \cdots, \frac{\partial f}{\partial x_n} \right)
$$


{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def Rosenbrock(x, y):
    return (1 - x) ** 2 + 100 * (y - x ** 2) ** 2

step = 0.05
X, Y = np.meshgrid(np.arange(-2, 2 + step, step), np.arange(-2, 3 + step, step))
Z = Rosenbrock(X, Y)

fig = plt.figure()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap=plt.get_cmap('hsv'))
fig.colorbar(surf, shrink=0.3, aspect=10)

ax.contour(X, Y, Z, zdir='z', offset=-2, cmap=plt.get_cmap('hsv'))

plt.show()
{% endhighlight %}


![rosenbrock]({{ site.url }}/images/ai/optimize-gradient-descent-rosenbrock.png "rosenbrock"){: .pull-center }


而n
元函数f(x1,x2,⋯,xn)的梯度是一个长度为n的向量，向量中第k个元素为函数f对变量xk的偏导数。
∇f(x1,x2,⋯,xn)=(∂f∂x1,∂f∂x2,⋯,∂f∂xn)

假设，我们有一个二元函数
f(x1,x2)=x21+x1x2−3x2
那么f的梯度为
∇f=(∂f∂x1,∂f∂x2)=(2x1+x2,x1−3)

例如在点(1,2)
，梯度∇f的取值为
∇f(1,2)=(2+2,1−3)=(4,−2)

2. 什么是梯度下降法（Gradient Descent）

对于凸优化问题来说（不了解凸优化的同学可先移步这里），导数为0（梯度为0向量）的点，就是优化问题的解。

为了找到这个解，我们沿着梯度的反方向进行线性搜索，每次搜索的步长为某个特定的数值α

，直到梯度与0向量非常接近为止。上面描述的这个方法就是梯度下降法。迭代算法的步骤如下：

    （1）当i=0

，自己设置初始点x0=(x01,x02,⋯,x0n)，设置步长（也叫做学习率）α，设置迭代终止的误差忍耐度tol
。
（2）计算目标函数f
在点xi上的梯度∇fxi
。
（3）计算xi+1
，公式如下
xi+1=xi−α∇fxi
（4）计算梯度∇fxi+1
。如果∥∇fxi+1∥2≤tol则迭代停止，最优解的取值为xi+1；如果梯度的二范数大于tol，那么i=i+1

    ，并返回第（3）步。

3.梯度下降法在机器学习中的应用

在机器学习中，我们通常会有一个损失函数L(β)
，其中向量β=(β0,β1,⋯,βn)是模型中的参数，我们需要找到最优的β来最小化损失函数L(β)。所以说，模型的训练过程也就是寻找最优解的过程。

<!---
其中的非线性函数示例可能会导致无法找到最小值
https://zh.wikipedia.org/wiki/%E6%A2%AF%E5%BA%A6%E4%B8%8B%E9%99%8D%E6%B3%95
-->


简单的Python示例
https://ctmakro.github.io/site/on_learning/gd.html
https://www.jiqizhixin.com/articles/2016-11-21-4
http://kissg.me/2017/07/23/gradient-descent/
https://www.jianshu.com/p/aa40508e3cc9
http://sofasofa.io/tutorials/python_gradient_descent/index.php

https://blog.csdn.net/bu2_int/article/details/81333737

https://www.cnblogs.com/xsxsz/p/10355247.html
https://blog.csdn.net/bu2_int/article/details/81333737


{% highlight text %}
{% endhighlight %}
