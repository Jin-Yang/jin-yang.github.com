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

对一些基本概念的简单介绍。

### 信息量

信息量是对信息的度量，就跟时间的度量是秒一样，而接受到的信息量跟具体发生的事件有关。

信息量的大小跟随机事件的概率有关，概率越小的事情发生了产生的信息量越大；越大概率的事情发生了产生的信息量越小，如太阳从东边升起来了 (肯定发生嘛，没什么信息量)。

因此需要查找一个函数满足，事件的信息量应该随着其发生概率而递减的，且不能为负。当然，同时还需要满足，如下条件。

对于两个不相关的事件 $x$ 和 $y$ ，那么观察到的两个事件同时发生时获得的信息应该等于观察到的事件各自发生时获得的信息之和，也就是 $h(x, y)=h(x) + h(y)$ 。

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

另外，可以证明条件熵 $H(X\|Y)$ 相当于联合熵 $H(X,Y)$ 减去单独的熵 $H(X)$ ，也就是。

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



/post/artificial-intelligence-entropy-concept-introduce.html

决策树 (Decision Tree) 是一个树结构，可以是二叉树或者非二叉树，期间会对每个非叶节点表示的特征进行测试，每个分支代表这个特征属性在某个值域上的输出，而每个叶节点存放一个类别。

使用决策树进行决策的过程就是从根节点开始，测试待分类项中相应的特征属性，并按照其值选择输出分支，直到到达叶子节点，将叶子节点存放的类别作为决策结果。

## 简介

常用的决策树算法有 `ID3`、`C4.5` 和 `CART` ，它们都是采用贪心 (非回溯) 方法，自顶向下递归的分治方法构造。

这几个算法区别在于选择属性划分方法：A) `ID3` 信息增益；B) `C4.5` 信息增益率；C) `CART` Gini基尼指数。

### 条件熵

$$
\begin{align}
H(Y|X) & = \sum_{x \in X}p(x)H(Y|X=x) \\
& = -\sum_{x \in X} p(x) \sum_{y \in Y}p(y|x)log\ p(y|x) \\
& = -\sum_{x \in X} \sum_{y \in Y}p(x,y)log\ p(y|x) \\
\end{align}
$$

#### 条件熵

H(Y|X) = p(X=No)H(Y|X=No) + p(X=Yes)H(Y|X=Yes)

>>> import math
>>> HYX_Y = -(5/7) * math.log(5/7, 2) - (2/7) * math.log(2/7, 2)
>>> HYX_N = -(2/3) * math.log(2/3, 2) - (1/3) * math.log(1/3, 2)
>>> 0.7*HYX_Y + 0.3*HYX_N
0.8796731482129885

Entropy(S) = ∑ – p(I) . log2p(I)
Gain(S, A) = Entropy(S) – ∑[p(S|A) . Entropy(S|A)]

???Gain的计算方式有点疑问???

/post/artificial-intelligence-decision-tree-introduce.html
## ID3

### 示例

上面的训练集中有 4 个属性，即属性集合 `{OUTLOOK, TEMPERATURE, HUMIDITY, WINDY}`，而类标签有 2 个，即 `{Yes, No}`，分别表示适合户外运动和不适合户外运动，其实是一个二分类问题。

总共有 14 个训练样本，其中 5 个为 No ，9 个为 Yes ，那么对应的信息熵计算为。

$$
\begin{align}
Entropy(Decision) & = -p(Y) log_2 {p(Y)} - p(N) log_2 {p(N)} \\
& = - \frac{9}{14} log_2( \frac{9}{14}) -  \frac{5}{14} log_2( \frac{5}{14}) = 0.940
\end{align}
$$

可以通过 Python 直接计算。

>>> import math
>>> HY = -(9/14) * math.log(9/14, 2) - (5/14) * math.log(5/14, 2)
0.9402859586706309

接着，对每个属性分别计算其信息熵增益。
有很多示例的介绍
https://sefiks.com/2017/11/20/a-step-by-step-id3-decision-tree-example/
http://shiyanjun.cn/archives/417.html
关于GradientBoosting以及决策树的介绍
https://github.com/serengil/chefboost



React示例程序
https://dev.to/drminnaar/11-react-examples-2e6d


## CART
https://www.csuldw.com/2015/05/08/2015-05-08-decision%20tree/
import numpy as np
import pandas as pd

data = pd.DataFrame({
	# Rain Overcast Sunny
    "Outlook":     ["S", "S", "O", "R", "R", "R", "O", "S", "S", "R", "S", "O", "O", "R"],
	# Hot Mild Cool
    "Temperature": ["H", "H", "H", "M", "C", "C", "C", "M", "C", "M", "M", "M", "H", "M"],
	# High Normal
    "Humidity":    ["H", "H", "H", "H", "N", "N", "N", "H", "N", "N", "N", "H", "N", "H"],
	# Weak Strong
    "Windy":       ["W", "S", "W", "W", "W", "S", "S", "W", "W", "W", "S", "S", "W", "S"],
	# Yes No
    "Decision":    ["N", "N", "Y", "Y", "Y", "N", "Y", "N", "N", "Y", "N", "N", "N", "N"]
})

# Mapping the letters in the dataframe to numbers
data["Outlook"]     = data["Outlook"].map({"R":0, "O": 1, "S": 2})
data["Temperature"] = data["Temperature"].map({"C":0, "M": 1, "H": 2})
data["Humidity"]    = data["Humidity"].map({"N":0, "H": 1})
data["Windy"]       = data["Windy"].map({"F":0, "T": 1})
data["Decision"]    = data["Decision"].map({"N":0, "Y": 1})

ID3 is acronym of Iterative Dichotomiser.
https://github.com/serengil/decision-trees-for-ml


<!--

Decision Tree
https://zhuanlan.zhihu.com/p/32164933
https://machinelearningmastery.com/implement-decision-tree-algorithm-scratch-python/
https://machinelearningmastery.com/classification-and-regression-trees-for-machine-learning/

############################################
## CART 决策树
############################################

Classfication And Regression Tree, CART 采用二分递归分割的技术将当前样本集分为两个子样本集，使得生成的每个非叶子节点都有两个分支。

非叶子节点的特征取值为 True 和 False，其中左分支取值为 True，右分支取值为 False，因此 CART 算法生成的决策树是结构简洁的二叉树。CART 可以处理连续型变量和离散型变量，利用训练数据递归的划分特征空间进行建树，用验证数据进行剪枝。

如果待预测分类是离散型数据，则CART生成分类决策树。
如果待预测分类是连续性数据，则CART生成回归决策树。


https://blog.csdn.net/weixin_40604987/article/details/79296427
https://zhuanlan.zhihu.com/p/53337716


决策树竟然还有剪枝
https://blog.csdn.net/u012328159/article/details/79285214
https://zhuanlan.zhihu.com/p/30296061


从头开始实现ID3算法
https://zhuanlan.zhihu.com/p/27905967


/post/program-c-gcc-security-options.html
-fPIE 是编译选项；而 -pie 是连接选项
https://andrewpqc.github.io/2019/08/04/go-pre-commit/


Drone CI/CD
	https://segmentfault.com/a/1190000018459195
https://www.jianshu.com/p/ede1f917c41a
-->


{% highlight text %}
{% endhighlight %}
