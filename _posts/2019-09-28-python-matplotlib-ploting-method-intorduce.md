---
title: Python 绘图方法
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,matplotlib
description:
---

介绍一些 Python 中常见的图片绘制方法。

<!-- more -->


## 箱形图

用作显示一组数据分散情况资料的统计图，会显示中位数、四分位数、异常值等信息，可以直观查看样本的统计特征值。

各个特征点对应的信息为：

* 中位数 `Median(Q2/50th Percentile)` 所有数据排序后的中间值。
* 第一四分位数 `First Quartile(Q1/25th Percentile)` 样本数值小到大排列第 25% 的数字。
* 第三四分位数 `Third Quartile(Q3/75th Percentile)` 样本数值小到大排列第 75% 的数字。
* 四分位距 `Interquartile Range, IQR` 第三四分位数与第一四分位数的差距。

另外，最大最小值通过 `Q3 + 1.5 * IQR` 以及 `Q1 - 1.5 * IQR` 进行计算，超过这一范围的就被称为异常值 (Outliers) 。

中间的细线也被称为 Whisker ，所以，箱型图也被称为箱须图 Box-Whisker Plot 。

![boxplot VS pdf]({{ site.url }}/images/ai/ploting-boxplot-vs-pdf.png "boxplot VS. pdf"){: .pull-center }

如下是一个简单的 Python 代码。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

np.random.seed(100)
data = np.random.normal(size=(1000, 3), loc=[0, 1, 3.5], scale=[1, 1.5, 3])
plt.boxplot(data, labels=["A", "B", "C"], sym="o")
plt.show()
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
