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

目前 AI 使用较多的是 Python，主要是因为这是一个简单 (包括语法、环境)的胶水语言，如果性能不满足，完全可以通过 C 重新实现，例如 NumPy 。

如下是常用库的使用简介。

* [环境准备](/post/python-ai-environment-prepare.html) 包括了常用的 Python 工具、TensorFlow 等。
* [Numpy 简介](/post/python-numpy-package-introduce.html) 一个扩充程序库，支持高级大量的维度数组与矩阵运算。
* [Scipy 简介](/post/python-scipy-package-intorduce.html) 可以处理插值、积分、优化、图像处理、常微分方程数值解的求解。
* [Pandas 简介](/post/python-pandas-package-introduce.html) 基于 NumPy 的工具库，主要是为了解决数据分析任务。
* [Matplotlib 简介](/post/python-matplotlib-package-intorduce.html) 绘图库包大量工具，几乎可以完成所有的图形绘制。

<!--
* [StatsModels 简介](/post/python-matplotlib-package-intorduce.html)

https://www.statsmodels.org/stable/index.html

skleran 常用子模块 ：

sklearn 是一个机器学习包。
 分类 ：SVM ， K近邻 ，随机森林 ， 逻辑回归等。
 回归 ：Lasso ,岭回归 等。
 聚类 ：K-means ,谱聚类等。
 降维 ：PCA ,特征选择 ，矩阵分解等。
 模型选择 ：网格搜索， 交叉验证 ，指标矩阵。
 预处理： 特征提取，正态化。

statsmodels常用子模块

回归模型：线性回归 ，通用线性回归，鲁邦线性模型  ，线性混合效应模型等。
方差分析（ANOVA）。
时间序列分析：AR , ARMA , ARIMA , VAR等。
非参数方法： 核密度估计 ， 核回归。
统计模型结果可视化。

statsmodels更专注于统计推理，提供不确定性评价和P值参数，
sklearn更专注于预测。
-->

### 其它

* [数据集简介](/post/math-machine-learning-some-datasets-introduce.html) 包括 CSV、示例函数、SciKit-Learn 测试集等。

## 数学基础

### 微积分

* [基本概念](/post/math-calculus-basic-concept.html)

### 线性代数

<!--
http://www.math.uwaterloo.ca/~hwolkowi//matrixcookbook.pdf
-->

### 概率论与数理统计

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
* [梯度下降算法](/post/math-gradient-descent-optimize-method-introduce.html)

### 其它

* [基本初等函数](/post/math-basic-elementary-function.html)

## 机器学习

机器学习所解决的问题，主要包括了数值的预测和分类，前者一般是回归模型，例如线性回归；后者的方法则是五花八门，例如决策树、KNN、支持向量机、朴素贝叶斯等等。

其实，两类问题本质上讲是一样的，都是通过学习已有数据，构建模型，然后对未知数据进行预测，如果是连续的数值预测就是回归问题，若是离散的类标号预测，那么就变成了分类问题。

* [线性回归 \-\- 基本介绍](/post/machine-learning-algorithms-linear-regression-basic-introduce.html)
* [线性回归 \-\- 贝叶斯方法](/post/machine-learning-algorithms-linear-regression-bayes-introduce.html)
* [逻辑回归](/post/machine-learning-algorithms-logistic-regression-introduce.html)
* [决策树](/post/artificial-intelligence-decision-tree-introduce.html)

### 其它

* [高斯过程](/post/artificial-intelligence-gaussian-process-introduce.html)
* [损失函数](/post/machine-learning-algorithms-loss-functions-introduce.html)


<!--
SVM
Naive Bayes
kNN
K-Means
Random Forest
Dimensionality Reduction Algorithms
Gradient Boosting algorithms
	GBM
	XGBoost
	LightGBM
	CatBoost
-->

## 概率编程

* [贝叶斯简介](/post/math-statistics-basic-concept-bayes-theorem-introduce.html)
* [马尔科夫简介](/post/math-statistics-markov-process-introduce.html)
* [基本采样算法](/post/math-monte-carlo-sample-introduce.html) 一些常见的基本概念，例如蒙特卡罗、简单采样等。
* [MCMC采样 - Metropolis 算法](/post/math-monte-carlo-mcmc-metropolis-introduce.html) 介绍 MCMC 的基本概念，以及很简单的算法。
* [Stan 简介](/post/statistic-tools-stan-introduce.html) 包括了 Stan 的基本概念、使用方法，以及 RStan 的安装使用。

<!--
概率统计公式大全
https://www.zybuluo.com/catscarf/note/986628


机器学习的基本算法
https://www.analyticsvidhya.com/blog/2017/09/common-machine-learning-algorithms/
-->

## R 语言

R 语言作为统计学一门语言，一直属于一个小众的工具集，直到大数据爆发后，越来越多的人开始使用 R 语言进行数据分析。

* [R 语言简介](/post/linux-R-language-basic-introduce.html) 介绍 R 语言的基本概念。
* [R 绘图简介](/post/linux-R-language-graph-function-introduce.html) R 语言提供了非常强大的绘图功能，这里简单介绍。
* [R 概率函数](/post/linux-R-language-some-statistic-function-introduce.html) 一些常用概率函数的使用方法。

#### 参考

* 官方文档 [www.rdocumentation.org](https://www.rdocumentation.org/) ，可以直接搜索相关的主题。

<!--
### 其它

* [数据拟合] 通过numpy实现
https://drivingc.com/p/5af5ab892392ec35c23048e2

谈谈 Bias-Variance Tradeoff
https://liam.page/2017/03/25/bias-variance-tradeoff/

## 机器学习 100 天

也就是 [Github 100 Days Of ML Code](https://github.com/Avik-Jain/100-Days-Of-ML-Code) 中的学习，可以参考 [Github 机器学习 100 天](https://github.com/MLEveryday/100-Days-Of-ML-Code) 。

https://blog.csdn.net/ybdesire/article/details/67701289
https://tracholar.github.io/wiki/machine-learning/sklearn-source.html
-->

## 深度学习

![artificial intelligence logo]({{ site.url }}/images/machine-learning-logo.jpg "artificial intelligence logo"){: .pull-center width="40%" }


## 参考资料

### 机器学习

#### Pattern Recognition and Machine Learning

常用资料 [Book English](https://www.microsoft.com/en-us/research/uploads/prod/2006/01/Bishop-Pattern-Recognition-and-Machine-Learning-2006.pdf)、[NoteBooks](https://github.com/ctgk/PRML/tree/master/notebooks)、[Python Implemented](https://github.com/ctgk/PRML) ，也可以参考本地保存 [En](/reference/machine/Bishop-Pattern-Recognition-and-Machine-Learning-2006.pdf)、[CN](/reference/machine/Bishop-Pattern-Recognition-and-Machine-Learning-CN.pdf) 。

<!--
#### Machine Learning, A Probabilistic Perspective

/reference/machine/Machine-Learning_A-Probabilistic-Perspective.pdf

https://doc.lagout.org/science/Artificial%20Intelligence/Machine%20learning/Machine%20Learning_%20A%20Probabilistic%20Perspective%20%5BMurphy%202012-08-24%5D.pdf


#### The Elements of Statistical Learning

频率学派

包括了 [英文版](https://esl.hohoweiya.xyz/book/The%20Elements%20of%20Statistical%20Learning.pdf) 以及 [中文版](https://esl.hohoweiya.xyz/index.html) 的地址。

/reference/machine/The-Elements-of-Statistical-Learning.pdf


Deap Learning 圣经

《统计学习方法》李航
《西瓜书》
《Pattern Recognition and Machine Learning》PRML

## 台大-林轩田  机器学习基石 技法
## 张志华 机器学习导论 统计机器学习
## 吴恩达 CS229
## 徐亦达 概率模型
## 台大-李弘毅

白板推导
-->

### 其它

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




列举了数学公式中一些常见的表示符号
https://zh.wikipedia.org/wiki/%E6%95%B0%E5%AD%A6%E7%AC%A6%E5%8F%B7%E8%A1%A8

数理统计学简史
https://github.com/yuanliangding/books/blob/master/%E6%95%B0%E5%AD%A6-%E5%85%B6%E5%AE%83-%E6%95%B0%E5%AD%A6%E5%8F%B2/%E6%95%B0%E7%90%86%E7%BB%9F%E8%AE%A1%E5%AD%A6%E7%AE%80%E5%8F%B2.pdf
-->

{% highlight text %}
{% endhighlight %}
