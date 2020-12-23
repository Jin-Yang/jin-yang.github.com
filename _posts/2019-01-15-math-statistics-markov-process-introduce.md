---
title: 马尔科夫简介
layout: post
comments: true
usemath: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->

## 转移概率

转移概率是指随机变量从一个时刻到下一个时刻，从状态 $s_i$ 转移到另外一个状态 $s_j$ 的概率。

$$P \left(i \rightarrow j \right) := P_{i,j} = P\left( X_{t+1}=s_j\mid X_t=s_i \right)$$

其中 $:=$ 表示定义为。

通过 $\pi_k^{(t)}$ 表示随机变量 $X$ 在时刻 $t$ 取值为 $s_k$ 的概率，则随机变量 $X$ 在时刻 $t+1$ 取值为 $s_i$ 的概率为。

$$
\begin{align}
\pi _i^{\left ( t+1 \right )} &=P\left ( X_{t+1}=s_i \right ) \\ 
&= \sum_{k}P\left ( X_{t+1}=s_i\mid X_{t}=s_k \right )\cdot P\left ( X_{t}=s_k \right )\\
&= \sum_{k}P_{k,i}\cdot \pi _k^{\left ( t \right )}
\end{align}
$$

假设状态的数目为 $j$ ，则有：

$$
\left ( \pi _1^{\left ( t+1 \right )},\cdots ,\pi _j^{\left ( t+1 \right )} \right )=\left ( \pi _1^{\left ( t \right )},\cdots ,\pi _j^{\left ( t \right )} \right )\begin{bmatrix}
P_{1,1} & P_{1,2} & \cdots  & P_{1,j}\\ 
P_{2,1} & P_{2,2} & \cdots  & P_{2,j}\\ 
\vdots  & \vdots  &  & \vdots \\ 
P_{j,1} & P_{j,2} & \cdots  & P_{j,j}
\end{bmatrix}
$$

### 转移概率矩阵

也就是上面的两个状态转移使用的矩阵。

$$
P_{ij} =
        \begin{pmatrix}
        p_{11} & p_{12} & \cdots & p_{1j} \\
        p_{21} & p_{22} & \cdots & p_{2j} \\
        \vdots & \vdots & \ddots & \vdots \\
        p_{i1} & p_{i2} & \cdots & p_{ij} \\
        \end{pmatrix}
$$

其中状态转移矩阵的每一行的和为 1 。

$$\sum_{j=1}^\infty P_{ij}=1$$

## 马尔可夫链

假设 $X_t$ 是随机变量 $X$ 在离散时刻 $t$ 的取值，如果该变量随时间变化的转移概率


## 马尔科夫

对于连续时间，称之为马尔科夫随机过程；而离散时间称为马尔科夫链。

对马尔科夫链来说，当前的状态仅与上一个状态有关。

### 细致平衡条件

细致平衡条件 (Detailed Balance Condition) 的定义为，在给定一个马尔科夫链后，如果分布 $\pi$ 和概率转移矩阵 $P$ 满足下面的等式。

$${\pi_i}P_{ij}={\pi_j}P_{ji}$$

则此马尔科夫链具有一个平稳分布 (Stationary Distribution) $\pi$ 。

注意，这是一个充分条件而非必要条件，也就是说存在平稳分布的马尔科夫链，不满足此细致平衡条件。

<!--
https://blog.csdn.net/bitcarmanlee/article/details/82819860
https://blog.csdn.net/guolindonggld/article/details/79597491
http://www.twistedwg.com/2018/06/07/MCMC-method.html
https://blog.csdn.net/bitcarmanlee/article/details/82819860
-->


{% highlight text %}
{% endhighlight %}
