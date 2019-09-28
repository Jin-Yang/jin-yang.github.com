---
title: AI 100 天 -- 01
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

简单来说，就是准备所需的数据。

<!-- more -->

## DAY 01

### 导入 Python 库

目前 AI 使用较多的是 Python，主要是因为这是一个简单(包括语法、环境)的胶水语言(Glue Language)，往往同一个场景包含了多种模块，而且如果性能不满足，那么完全可以通过 C 重新实现，例如 NumPy 。

最常用的是 NumPy 和 Pandas，前者提供了基本的数组操作，后者则提供了强大的数据分析工具，实际上除此之外还有 Scipy、Matplotlib 分别用于支持矩阵运算、绘图。

关于两个库的详细介绍可以参考。

### 导入数据集

有很多政府公开的数据是以 Comma Separated Values, CSV 格式保存，例如 [data.gov.uk](https://data.gov.uk/)、[catalog.data.gov](https://catalog.data.gov/dataset)、[www.kaggle.com](https://www.kaggle.com/) 等，这也是最常使用的数据格式。

如果是通过 Pandas 模块处理数据，那么可以直接使用 Pandas 进行处理，示例如下。

{% highlight python %}
import pandas as pd

df1 = pd.DataFrame({'c1':[1,2,3], 'c2':[4,5,6]})
df1.to_csv("test.csv", index=False, sep=',')
df2 = pd.read_csv('test.csv')

df2.shape    # 显示行数以及属性数(列数)
df2.head()   # 展示前5行的数据
{% endhighlight %}

默认会增加一列的索引 (行名称为空)，可以通过 `index=False` 将其关闭。

除此之外，Python 中同时也提供了一个 [CSV](https://docs.python.org/2/library/csv.html) 的标准库。

{% highlight python %}
import csv

with open("test.csv", "w") as csvfile:
	writer = csv.writer(csvfile)
	writer.writerow(["index", "c1", "c2"])
	writer.writerows([[0,1,3], [1,2,3], [2,3,4]])
with open("test.csv", "r") as csvfile:
	reader = csv.reader(csvfile)
	for line in reader:
		print line
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
