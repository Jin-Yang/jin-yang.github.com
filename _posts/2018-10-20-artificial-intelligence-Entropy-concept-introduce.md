---
title: 熵
layout: post
comments: true
language: chinese
usemath: true
category: [misc]
keywords:
description:
---


<!-- more -->

## 简介

### 信息量

信息量是对信息的度量，就跟时间的度量是秒一样，而接受到的信息量跟具体发生的事件有关。

信息量的大小跟随机事件的概率有关，概率越小的事情发生了产生的信息量越大；越大概率的事情发生了产生的信息量越小，如太阳从东边升起来了 (肯定发生嘛，没什么信息量)。

因此需要查找一个函数满足，事件的信息量应该随着其发生概率而递减的，且不能为负。当然，同时还需要满足，如下条件。

对于两个不相关的事件 x 和 y，那么观察到的两个事件同时发生时获得的信息应该等于观察到的事件各自发生时获得的信息之和，也就是 $h(x, y)=h(x) + h(y)$ 。

因为事件 $x$ $y$ 不相关，那么满足 $p(x,y) = p(x)*p(y)$ 条件，显然，需要引入一个对数函数。

所以，在引入了对数函数后，对应的信息量计算公式如下：

$$h(x) = - log_2 {p(x)}$$

其中负号是为了保证取值为正，底数只是信息论中的使用传统。

### 信息熵

信息量度量的是一个具体事件发生了所带来的信息，而信息熵 (Entropy) 则是在结果出来之前对可能产生信息量的期望，也就是所有可能发生事件所带来的信息量的期望。

信息熵的公式如下。

$$H(x) = - \sum_{i=1}^{n} p(x_i)log_2\ p(x_i)$$

其中 $P(x_i)$ 代表当随机事件 $X$ 为 $x_i$ 时的概率。

也就是说，信息熵可以作为一个系统复杂程度的度量，如果系统越复杂，出现不同情况的种类越多，那么它的信息熵也就越大。

如果将一维的随机变量分布推广到多维随机变量分布，则称其为联合熵 (Joint Entropy) ，定义为。


### 条件熵

条件熵的定义为，对于联合概率分布 $p(x, y)$ ，在已知随机变量 X 的条件下随机变量 Y 的不确定性。定义为 X 给定条件下 Y 的条件概率分布的熵对 X 的数学期望。

另外，可以证明条件熵 `H(X|Y)` 相当于联合熵 $H(X,Y)$ 减去单独的熵 $H(X)$ ，也就是。

$$H(Y|X)=H(X,Y)-H(X)$$

### 信息增益

如上，信息熵是代表随机变量的复杂度 (不确定性)，条件熵代表在某一个条件下，随机变量的复杂度 (不确定度) 。

而信息增益 (Information Gain) 的计算方式为：信息熵 - 条件熵 。

$$G(Y,X) = H(Y) - H(Y|X)$$

换句话说，信息增益代表了在一个条件下，信息复杂度（不确定性）减少的程度。

在决策树算法中，关键的操作就是每次选择一个特征，如果有多个特征，那么就需要按照一个标准来选择具体是哪一个特征。

此时就可以使用信息增益，当选择某个特征之后，如果对应的信息增益最大 (信息不确定性减少的程度最大)，那么就选取这个特征。

## 示例

假设有如下的数据。

|  Outlook |  Temperature  | Humidity |  Windy | PlayGolf |
|:--------:|:-------------:|:--------:|:------:|:--------:|
|   Rainy  |      Hot      |   High   |  False |    No    |
|   Rainy  |      Hot      |   High   |  True  |    No    |
| Overcast |      Hot      |   High   |  False |   Yes    |
|   Sunny  |     Mild      |   High   |  False |   Yes    |
|   Sunny  |     Cool      |  Normal  |  False |   Yes    |
|   Sunny  |     Cool      |  Normal  |  True  |    No    |
| Overcast |     Cool      |  Normal  |  True  |   Yes    |
|   Rainy  |     Mild      |   High   |  False |    No    |
|   Rainy  |     Cool      |  Normal  |  False |   Yes    |
|   Sunny  |     Mild      |  Normal  |  False |   Yes    |

对于随机变量 `Y = {Yes, No}` ，可以统计出，打球的频率为 `6/10=3/5` ，不打球的频率为 `4/10=2/5` ，那么对于 $Y$ 的熵，可以根据熵公式计算得到。

$$H(Y)=-\frac{3}{5} log_2 \frac{3}{5} - \frac{2}{5} log_2 \frac{2}{5} $$

可以通过如下的 Python 代码计算。

{% highlight text %}
>>> import math
>>> - 0.6 * math.log(0.6, 2) - 0.4 * math.log(0.4, 2)
0.9709505944546686
{% endhighlight %}

#### 条件熵

接下来计算条件熵，选择是否有风作为随机变量 `X = {Yes, No}` 。

当无风时，满足条件的有 7 个，在这 7 个数据中，打球的有 5 个，不打球的有 2 个，那么可以得到，当无风时的熵为。

$$H(Y|X=No)=-\frac{5}{7} log_2 \frac{5}{7} - \frac{2}{7} log_2 \frac{2}{7}$$

同样可以得到有风时的熵为。

$$H(Y|X=Yes)=-\frac{2}{3} log_2 \frac{2}{3} - \frac{1}{3} log_2 \frac{1}{3}$$

通过上述的结果，可以接着计算条件熵。

https://zhuanlan.zhihu.com/p/26596036
含有公式的推导
https://zhuanlan.zhihu.com/p/35379531
https://zhuanlan.zhihu.com/p/26551798




{% highlight text %}
{% endhighlight %}
