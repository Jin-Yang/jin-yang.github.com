---
title: R 概率函数
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->



## 均匀分布

也就是 Random Uniform ，对应了 `runif()` 函数。

{% highlight text %}
runif(n, min=0, max=1)
   n 生成随机数的数量；
   min 均匀分布的下限，默认是 0；
   max 均匀分布的上限，默认是 1。
{% endhighlight %}

可以不指定 `min` 和 `max` 值，此时会直接使用默认的值。

如下生成 100 个均匀分布的随机数，并绘制概率直方图，以及均匀分布的密度函数线。

{% highlight text %}
> x=runif(100)
> hist(x, prob=T, col=gray(.9), main="uniform on [0,1]")
> curve(dunif(x,0,1), add=T)
{% endhighlight %}

## 正态分布随机数

在R语言中，生成正态分布随机数的函数是：rnorm()

{% highlight text %}
rnorm(n, mean=0, sd=1)
   n 随机数的数量；
   mean 正太分布的均值，默认为 0；
   sd 正太分布的标准差，默认为 1。
{% endhighlight %}

同样可以忽略 `mean` 和 `sd` 的值。

随机产生 100 个标准正态分布随机数，并作其概率直方图，再添加正态分布的密度函数线。

{% highlight text %}
> x=rnorm(1000)
> hist(x, breaks=100, prob=T, main="normal mu=0 sigma=1")
> curve(dnorm(x), add=T)
{% endhighlight %}

## 二项分布随机数

二项分布 Binomial 是指 n 次独立重复伯努利试验成功的次数的分布，每次伯努利试验的结果只有两个，成功和失败，记成功的概率为 p 。

{% highlight text %}
rbinom(n, size, prob)
  n 生成的随机数数量；
  size 进行伯努利试验的次数；
  prob 一次贝努力试验成功的概率。
{% endhighlight %}


{% highlight text %}
> par(mfrow=c(1, 3))
> p=0.25
> for(n in c(10, 20, 50))
{
	x=rbinom(100, n, p)
	hist(x, prob=T, main=paste("n =", n))
	xvals=0:n
	points(xvals, dbinom(xvals,n,p), lwd=3)
}
> par(mfrow=c(1, 1))
{% endhighlight %}

除了上述获取随机数的 `rbinom()` 函数之外，与二项分布相关的还有：

{% highlight text %}
pbinom(q, size, prob) 计算累积概率
dbinom(x, size, prob) 计算取某个值的特定概率
qbinom(p, size, prob) quantile function 分位数函数。
{% endhighlight %}

例如，在一次实验中，事件 A 发生的概率为 0.1，那么进行 11 次这样的实验，观察到 4 次的概率是多少？

{% highlight text %}
> dbinom(4, 11, 0.1)
[1] 0.0157838
{% endhighlight %}

观察到小于等于 4 次的概率是多少？

{% highlight text %}
> pbinom(4, 11, 0.1)
[1] 0.997249
{% endhighlight %}

## 指数分布随机数

R生成指数分布随机数的函数是:rexp（）  

其句法是：rexp（n,lamda=1） n表示生成的随机数个数，lamda=1/mean

{% highlight text %}
> x=rexp(100,1/10)           # 生成100个均值为10的指数分布随机数
> hist(x,prob=T,col=gray(0.9),main=“均值为10的指数分布随机数”) 
> curve(dexp(x,1/10),add=T) ＃添加指数分布密度线
{% endhighlight %}



{% highlight text %}
{% endhighlight %}
