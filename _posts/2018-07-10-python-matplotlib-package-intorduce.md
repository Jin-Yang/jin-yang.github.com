---
title: Python Matplotlib 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,matplotlib
description: 简单来说，Matplotlib 是 Python 中的一个绘图库，包含了大量的工具，几乎可以通过该工具完成你所需要的任何图形，包括散点图、正弦曲线，甚至是三维图形。这一工具经常用在数据可视化中，这里简单介绍其使用方法。
---

简单来说，Matplotlib 是 Python 中的一个绘图库，包含了大量的工具，几乎可以通过该工具完成你所需要的任何图形，包括散点图、正弦曲线，甚至是三维图形。

这一工具经常用在数据可视化中，这里简单介绍其使用方法。

<!-- more -->

## 简介

在正式使用之前，强烈建议先查看下官方的 [示例库](https://matplotlib.org/gallery.html) ，包含了样例以及对应的代码，几乎包含了所能想到的图形。

<!--
plt.figure() 定义一个图像
plt.plot() 绘制图形
plt.axhline() 绘制水平线
plt.legend() 显示图例
plt.title() 标题
plt.xlabel() X轴标记
plt.ylabel() Y轴标记
-->

## 参数

### rcParams

通过该参数可以设置图片大小、颜色、像素、分辨率等参数信息。

详细可以参考 [The top level matplotlib module](https://matplotlib.org/api/matplotlib_configuration_api.html#matplotlib.RcParams) 中的介绍。

## FAQ

### 常见报错

#### ImportError: No module named _tkinter

默认 Python 安装包没有提供这个库，需要安装 `tkinter` 包，直接 `yum install tkinter` 即可。

据说源码编译时，需要确保添加了 `--enable-unicode=ucs2` 参数。

#### Found existing installation: pyparsing 1.5.6

安装 `matplotlib` 包时，会依赖上述的包，主要是版本问题，可以从 [pypi.org/pyparsing](https://pypi.org/project/pyparsing/) 上查看最新的包，例如是 `2.2.0` ，那么可以通过如下方式安装。

{% highlight text %}
# pip install -I pyparsing==2.2.0
{% endhighlight %}


<!---
http://codingpy.com/article/a-quick-intro-to-matplotlib/
http://python.jobbole.com/87831/
-->



{% highlight text %}
{% endhighlight %}
