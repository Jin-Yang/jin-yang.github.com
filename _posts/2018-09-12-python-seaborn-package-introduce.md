---
title: Python Seaborn 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,seaborn
description: Seaborn 是一个基于 matplotlib 的可视化库，提供了一个更上层的 API 封装，从而可以更容易的绘制图形。
---

Seaborn 是一个基于 matplotlib 的可视化库，提供了一个更上层的 API 封装，从而可以更容易的绘制图形。

<!-- more -->



## 简介

### 数据集

提供了很多示例数据集，可以通过 `sns.load_dataset("iris")` 方式加载，实际上是从 [Seaborn Data](https://github.com/mwaskom/seaborn-data) 上下载的数据，也可以通过 `get_dataset_names()` 查看当前存在的数据集。

也可以将数据集离线下载保存。

## 绘图

### pairplot

主要是为了展示变量两两之间的关系，包括了线性、非线性、是否相关等，如下以

{% highlight python %}
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

iris = sns.load_dataset('iris')     # require internet
iris = pd.read_csv('data/iris.csv') # offline downloaded

sns.pairplot(iris)
plt.show()
{% endhighlight %}

默认对角线上是各个属性的直方图，非对角线上是两个不同属性之间的相关图，可以明显看到相关属性的相关关系，例如花瓣的长度和宽度。

有些常用参数这里简单介绍，包括了控制图的形状，例如：A) `kind` 非对角线 `scatter` `reg` ；B) `diag_kind` 对角线 `hist` `kde` 。当使用 `reg` 时，会同时拟合一条直线，更方便用于观测。

{% highlight python %}
sns.pairplot(iris, kind='reg', diag_kind='kde')
{% endhighlight %}

可以通过 `hue` 参数设置分类。

{% highlight python %}
sns.pairplot(iris, hue='species')
{% endhighlight %}

在通过 `hue` 参数分类之后，可以直接通过直方图、散点图区分不同种类的花，基本有肉眼可见的差异。分类可以通过 `palette` 控制色调，`markers` 设置散点格式。

{% highlight python %}
sns.pairplot(iris, hue='species', palette='husl')
sns.pairplot(iris, hue='species', markers=["+", "s", "D"])
{% endhighlight %}

另外，可以只对比部分类型，通过 `vars`、`x_vars`、`y_vars` 传入，注意，后两者需要同时指定。

{% highlight python %}
sns.pairplot(iris, vars=['sepal_length', 'petal_length'])
sns.pairplot(iris, x_vars=['sepal_length', 'petal_width'],
	y_vars=['sepal_width', 'petal_length'])
{% endhighlight %}

更多的样式可以通过 `plot_kws` 和 `diag_kws` 控制，分别表示非对角线以及对角线上的样式。

{% highlight python %}
sns.pairplot(iris, diag_kind="kde",
	plot_kws=dict(s=50, edgecolor="w", color="g", alpha=.5),
	diag_kws=dict(shade=True, color="r"))
{% endhighlight %}

<!--
## 参考

http://seaborn.pydata.org/

一个不错的数据可视化网站
https://www.gapminder.org/
-->


{% highlight text %}
{% endhighlight %}
