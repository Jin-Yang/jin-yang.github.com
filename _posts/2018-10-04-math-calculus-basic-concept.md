---
title: 微积分基本概念
layout: post
comments: true
language: chinese
usemath: true
category: [linux,misc]
keywords: linux,auto,completion
description:
---

<!-- more -->

## 微积分

正常该学科的核心应该是微分和积分，只是很多《高等数学》讲解的是微分的一个性质导数。

对于一元函数 $y=f(x)$ 来说，其微分定义为 $dy=f'(x)dx$ ，那么如果将 $dx$ 看做自变量，那么函数 $dy=f'(x)dx$ 就代表了切线。

在一个很小的范围内，就可以通过直线代替曲线，也就是所谓的以直代曲，而洛必达法则、泰勒公式、积分基本定理、牛顿迭代法等，通过这一概念解释就会简单很多。

## 导数 Derivative

导数是微积分的形态之一，一般通过极限进行定义。

$$
f'(x_0) = \lim_{\Delta x \to 0} \frac{\Delta y}{\Delta x} = \lim_{\Delta x \to 0} \frac{f(x_0 + \Delta x) - f(x_0)}{\Delta x} = \lim_{x \to x_0} \frac{f(x) - f(x_0)}{x - x_0}
$$

导数 $f' (x_0)$ 反映的是函数 $f(x)$ 在点 $x_0$ 处沿 $x$ 轴正方向的变化率。

当 $x$ 的变化量 $\Delta x$ 趋于 0 时，记作微元 $dx$ 。

### 常用规则

根据上述的定义，在实际使用过程中可以整理常用函数的微分，以及一些常见的计算规则。

#### 基本初等函数求导

##### 幂函数

$$\frac{d}{dx}{x^n}=nx^{n-1}$$

##### 指数函数

$$\frac{d}{dx}{a^x}=ln(a)a^x$$

$$\frac{d}{dx}{e^x}=e^x$$

##### 对数函数

$$\frac{d}{dx}{log_a(x)}=\frac{1}{xln(a)}$$

$$\frac{d}{dx}{ln(x)}=\frac{1}{x}$$

##### 三角函数

$$\frac{d}{dx}{sin(x)}=cos(x)$$

$$\frac{d}{dx}{cos(x)}=-sin(x)$$

$$\frac{d}{dx}{tan(x)}=sin^2(x)=\frac{1}{cos^2(x)}=1+tan^2(x)$$

##### 反三角函数

$$\frac{d}{dx}{arcsin(x)}=\frac{1}{\sqrt{1-x^2}} \quad -1 \lt x \lt 1$$

$$\frac{d}{dx}{arccos(x)}=-\frac{1}{\sqrt{1-x^2}} \quad -1 \lt x \lt 1$$

$$\frac{d}{dx}{arctan(x)}=\frac{1}{1+x^2}$$

#### 组合运算规则

##### 加法规则

$$(\alpha f + \beta g)' = \alpha f' + \beta g'$$

##### 乘法规则

$$(fg)' = f'g + fg'$$

##### 除法规则

$$\left( \frac{f}{g} \right)' = \frac{f'g - fg'}{g^2}$$

##### 链接规则

假设有 $f(x)=h(g(x))$ ，那么对 $x$ 求导数可得。

$$f'(x) = h'(g(x)) g'(x)$$



<!--
最初导数是通过极大极小值进行推导的，在后续的发展中，逐渐转换为通过极限进行定义。
https://www.zhihu.com/question/22199657/answer/115178055
https://zh.wikipedia.org/wiki/%E5%AF%BC%E6%95%B0#/media/File:Derivative_-_geometric_meaning.svg




## 微分定义

对于单变量微分有多中表示方法，假设有函数 $y=f(x)$ ，其对应的导数可以通过 $\frac{dy}{dx}$ $\frac{df(x)}{dx}$ $\frac{d}{dx}f(x)$ 表示，也可以简写为 $f'$ 或者 $f'(x)$ 。

### 求偏导

也就是说，对于多元函数而言，会针对每个变量求导数，而且是通过 $\frac{\partial}{\partial x}$ 而非 $\frac{d}{dx}$ 表示，当对某个变量求导时，其它的变量将被视为常量。

以函数 $f(x,y)=3x^2y$ 为例，其偏导为 $\frac{\partial}{\partial x}{xy}=3x^2y=$ 以及 $\frac{\partial}{\partial y}{xy}=x$ ，也就是在对某个变量求偏导时，其它变量视为常量。

对变量 $x$ 的偏导为。

$$\frac{\partial}{\partial x}{f(x,y)}=\frac{\partial}{\partial x}{3x^2y}=3y\frac{\partial}{\partial x}{x^2}=3y2x=6yx$$

对变量 $y$ 的偏导为。

$$\frac{\partial}{\partial y}{f(x,y)}=\frac{\partial}{\partial y}{3x^2y}=3x^2\frac{\partial}{\partial y}{y}=3x^2 \times 1=3x^2$$

通常会以向量的方式表示，这样也被称为梯度 (Gradient) ，表示为：

$$\nabla f(x,y)=\left[\frac{\partial}{\partial x}{f(x,y)} , \ \frac{\partial}{\partial y}{f(x,y)} \right]=\left[6yx,\ 3x^2 \right]$$

也就是说，梯度实际上是偏导的向量表示方法。

https://www.researchgate.net/publication/322949882_The_Matrix_Calculus_You_Need_For_Deep_Learning




线性回归求导
https://www.jianshu.com/p/e25b0f413c56
https://blog.csdn.net/u012421852/article/details/79562125
https://blog.51cto.com/12133258/2051527

### Sigmoid Function

也被称为 Logistic Function ，其公式为 $g(x)=\frac{1}{1+e^{-x}}$ ，在逻辑回归中比较常用，满足如下的特性。

$$\frac{d}{dx} g(x) = g(x) (1 - g(x))$$

可以通过如下几种方式进行证明。

#### Method #1
https://blog.csdn.net/kamidox/article/details/48791725



## 导数发展史

一般将导数定义为如下。

$$f' (x)=\frac{dy}{dx}$$

其中 $dx$ 和 $dy$ 都被称为 $x$ $y$ 的微分，都是无穷小量，所以导数也被莱布尼茨称为微商 (微分之商) 。




在不同的阶段，各个大牛实际上对导数都有不同的定义以及表示方式。
https://zh.wikipedia.org/wiki/%E5%AF%BC%E6%95%B0
-->

## 偏导数 Partial Derivative

偏导数是导数在多元函数上的扩展，两者的本质一样，都是当自变量的变化量趋于 $0$ 时，函数值的变化量与自变量变化量比值的极限。

对于曲线来说，其切线只有一条，但对于曲面来说，某个点的切线有无数条。直观地说，偏导数就是函数在某一点上沿坐标轴正方向的变化率。

* $f_x(x,y)$ 指的是函数在 $y$ 方向不变，函数值沿着 $x$ 轴方向的变化率；
* $f_y(x,y)$ 指的是函数在 $x$ 方向不变，函数值沿着 $y$ 轴方向的变化率。

以二元函数为例，相当于一个平面，那么平面上任意一点的切线会有很多条；偏导数就是选择其中一条，并求出它的斜率，最常见的是垂直于 $y$ 轴或者 $x$ 轴的切线。

## 全导数 total derivative

方向导数、偏导数是特殊的全导数

<!--
## 梯度下降 Gradient Descent
https://www.zybuluo.com/codeep/note/163962
-->


## 矩阵求导 Matrix Derivative

矩阵的微积分本质上是多元变量的微积分问题，只是应用在矩阵空间上而已。

梯度计算的是单个多元函数的导数，而矩阵求导实际上是针对的多个多元函数求导的表示方法，一行表示一个多元函数的梯度。

### 求导类型

因为存在标量 (Scalar)、向量 (Vector)、矩阵 (Matix) ，那么在求导时，根据分子和分母的区别，分成了如下几类。

|     类型     |             标量 $y$                |              向量 $\bf y$               |            矩阵 $\bf Y$             |
| :----------: | :---------------------------------: | :-------------------------------------: | :---------------------------------: |
| 标量 $x$     | $\frac{\partial y}{\partial x}$     | $\frac{\partial \bf y}{\partial x}$     | $\frac{\partial \bf Y}{\partial x}$ |
| 向量 $\bf x$ | $\frac{\partial y}{\partial \bf x}$ | $\frac{\partial \bf y}{\partial \bf x}$ |                                     |
| 矩阵 $\bf X$ | $\frac{\partial y}{\partial \bf X}$ |                                         |                                     |

<!--
https://blog.csdn.net/u010976453/article/details/54381248

矩阵求导的时候有三个重要的公式 (按照分子规则) 。

$$
\frac{dX^TAX}{dX}=2AX \qquad
\frac{dX^TA}{dX}=A \qquad
\frac{AX}{dX} =A^T
$$

详细可以参考 [Matrix Calculus](https://en.wikipedia.org/wiki/Matrix_calculus) 
https://www.jianshu.com/p/e818917ffd9d

假设有函数 $g(x,y)=2x+y^8$ ，其对应的偏导为 $\nabla g(x,y)=\left[2, 8y^7 \right]$ ，再算上上述的 $f(x,y)=3x^2y$ 函数，可以得到雅克比矩阵 (Jacobian Matrix) ，也就是：

$$
J=\left[ \begin{matrix} \nabla f(x,y) \\ \nabla g(x,y) \end{matrix} \right]
 =\left[ \begin{matrix} \frac{\partial f(x,y)}{\partial x} & \frac{\partial f(x,y)}{\partial y} \\   \frac{\partial g(x,y)}{\partial x} & \frac{\partial g(x,y)}{\partial y} \end{matrix} \right]
 =\left[ \begin{matrix} 6yx & 3x^2\\ 2 & 8y^7 \end{matrix} \right]
$$

注意，上述只是其中的一种表示方法，称为 _分子布局_ (Numerator Layout) ，另外一种是 _分母布局_ (Denominator Layout) ，其实就是分子布局的转置 (可以根据行向量那一部分没有变化判断)，其结果为。

$$
\left[ \begin{matrix} 6yx & 2 \\ 3x^2 & 8y^7 \end{matrix} \right]
$$

https://blog.csdn.net/nomadlx53/article/details/50849941
-->


{% highlight text %}
{% endhighlight %}
