---
title: 【主题】人工智能
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![artificial intelligence logo]({{ site.url }}/images/artificial-intelligence-logo-01.jpg "artificial intelligence logo"){: .pull-center width="70%" }

科普作家 Isaac Asimov 阿西莫夫的机器人三定律。

> A robot may not injure a human being or, through inaction, allow a human being to come to harm.
>
> A robot must obey orders given it by human beings except where such orders would conflict with the First Law.
>
> A robot must protect its own existence as long as such protection does not conflict with the First or Second Law.

## Python 包

* [环境准备](/post/python-ai-environment-prepare.html) 包括了常用的 Python 工具、MNIST 数据集、TensorFlow 等。
* [Numpy 简介](/post/python-numpy-package-introduce.html) 一个扩充程序库，支持高级大量的维度数组与矩阵运算。
* [Scipy 简介](/post/python-scipy-package-intorduce.html) 可以处理插值、积分、优化、图像处理、常微分方程数值解的求解。
* [Pandas 简介](/post/python-pandas-package-introduce.html) 基于 NumPy 的工具库，主要是为了解决数据分析任务。
* [Matplotlib 简介](/post/python-matplotlib-package-intorduce.html) 绘图库包大量工具，几乎可以完成所有的图形绘制。

## 概率论与数理统计

### 概率论

* [基本概念](/post/math-probability-basic-concept.html)
* [离散概率分布](/post/math-probability-basic-concept-discrete-distribution-introduce.html)
* [连续概率分布](/post/math-probability-basic-concept-continuous-distribution-introduce.html)
* [连续概率 \-\- 高斯分布](/post/math-probability-continuous-normal-distribution-introduce.html) 包括了基本的高斯分布以及多元高斯分布。

统计推断的三个重要内容：抽样分布、参数估计、假设检验。

* [数理统计基本概念](/post/math-statistics-basic-concept.html)
* [似然估计](/post/math-statistics-likelihood-function-introduce.html)
* [核密度函数](/post/math-statistics-kernel-density-estimates-introduce.html)
* [熵概念简介](/post/artificial-intelligence-entropy-concept-introduce.html)

### 优化算法

* [基本优化算法](/post/math-basic-concept-optimize-method-introduce.html)


### 采样算法

* [贝叶斯简介](/post/math-statistics-basic-concept-bayes-theorem-introduce.html)
* [马尔科夫简介](/post/math-statistics-markov-process-introduce.html)
* [基本采样算法](/post/math-monte-carlo-sample-introduce.html) 一些常见的基本概念，例如蒙特卡罗、简单采样等。
* [MCMC采样 - Metropolis 算法](/post/math-monte-carlo-mcmc-metropolis-introduce.html) 介绍 MCMC 的基本概念，以及很简单的算法。

<!--
概率统计公式大全
https://www.zybuluo.com/catscarf/note/986628

关于矩阵介绍的不错资料
神奇的矩阵——第二季

iacs-courses.seas.harvard.edu/courses/am207/tag/lectures2.html
-->

### R 语言

R 语言作为统计学一门语言，一直属于一个小众的工具集，直到大数据爆发后，越来越多的人开始使用 R 语言进行数据分析。

* [R 语言简介](/post/linux-R-language-basic-introduce.html) 介绍 R 语言的基本概念。
* [R 绘图简介](/post/linux-R-language-graph-function-introduce.html) R 语言提供了非常强大的绘图功能，这里简单介绍。
* [R 概率函数](/post/linux-R-language-some-statistic-function-introduce.html) 一些常用概率函数的使用方法。
* [Stan 简介](/post/statistic-tools-stan-introduce.html) 包括了 Stan 的基本概念、使用方法，以及 RStan 的安装使用。

#### 参考

* 官方文档 [www.rdocumentation.org](https://www.rdocumentation.org/) ，可以直接搜索相关的主题。


## 机器学习 100 天

也就是 [Github 100 Days Of ML Code](https://github.com/Avik-Jain/100-Days-Of-ML-Code) 中的学习，可以参考 [Github 机器学习 100 天](https://github.com/MLEveryday/100-Days-Of-ML-Code) 。

* [AI 100 天 \-\- 01](/post/artificial-intelligence-100-days-01.html) 如何准备所需的数据。

<!--
https://blog.csdn.net/ybdesire/article/details/67701289
https://tracholar.github.io/wiki/machine-learning/sklearn-source.html
-->

## 深度学习

![artificial intelligence logo]({{ site.url }}/images/machine-learning-logo.jpg "artificial intelligence logo"){: .pull-center width="40%" }


## 其它

* [Rules of Machine Learning](https://developers.google.com/machine-learning/guides/rules-of-ml/) 介绍关于 ML 的一些原则，可以参考 [本地文档](/reference/machine/Rules of Machine Learning.pdf) 。
* 数学公式的编写可以参考 [MathJax Demo](https://www.mathjax.org/#demo) 。

<!--
https://blog.csdn.net/Mage_EE/article/details/75309174
https://www.zybuluo.com/knight/note/96093
https://www.zybuluo.com/codeep/note/163962  比较全


https://github.com/neverusedname/MyBooks/blob/master/%E6%B5%99%E6%B1%9F%E5%A4%A7%E5%AD%A6%E6%A6%82%E7%8E%87%E8%AE%BA%E4%B8%8E%E6%95%B0%E7%90%86%E7%BB%9F%E8%AE%A1(%E7%AC%AC%E5%9B%9B%E7%89%88).pdf
https://github.com/KeKe-Li/book/blob/master/AI/%E5%90%8C%E6%B5%8E%E7%BA%BF%E6%80%A7%E4%BB%A3%E6%95%B0%E6%95%99%E6%9D%90.pdf
https://github.com/KeKe-Li/book/blob/master/AI/%E5%90%8C%E6%B5%8E%E9%AB%98%E7%AD%89%E6%95%B0%E5%AD%A6%E7%AC%AC%E5%85%AD%E7%89%88%E4%B8%8A%E4%B8%8B%E5%86%8C.pdf


网络RST报文出现的场景
https://zhuanlan.zhihu.com/p/30791159


关于贝叶斯的介绍
https://github.com/markdregan/Bayesian-Modelling-in-Python
https://github.com/Tongzhenguo/ebooks 李航的机器学习
http://www.dgt-factory.com/uploads/2018/07/0725/%E7%BB%9F%E8%AE%A1%E5%AD%A6%E4%B9%A0%E6%96%B9%E6%B3%95.pdf
https://github.com/lovingers/ML_Books/blob/master/634901%2B%E8%B4%9D%E5%8F%B6%E6%96%AF%E6%96%B9%E6%B3%95%2B%2B%E6%A6%82%E7%8E%87%E7%BC%96%E7%A8%8B%E4%B8%8E%E8%B4%9D%E5%8F%B6%E6%96%AF%E6%8E%A8%E6%96%AD%2B%E4%B8%AD%E6%96%87%E7%89%88.pdf
https://zhuanlan.zhihu.com/p/27306970 很多不错的书籍介绍


https://www.zhihu.com/question/21277368
https://blog.csdn.net/jteng/article/details/54344766
https://applenob.github.io/machine_learning/MCMC/
https://applenob.github.io/archives/page/9/
https://www.infoq.cn/article/facebook-open-source-mass-forecasting-system-prophet
https://zhuanlan.zhihu.com/p/34071776


http://dreamrunner.org/blog/2014/06/28/qian-tan-memory-reordering/
https://www.digitalocean.com/community/tutorials/a-guide-to-time-series-forecasting-with-prophet-in-python-3
https://www.analyticsvidhya.com/blog/2017/09/common-machine-learning-algorithms/
https://facebook.github.io/prophet/docs/quick_start.html#python-api
https://mc-stan.org/docs/2_19/stan-users-guide/linear-regression.html

量化投资，不错的介绍
https://www.quantstart.com/articles/Bayesian-Statistics-A-Beginners-Guide
-->

{% highlight text %}
{% endhighlight %}
