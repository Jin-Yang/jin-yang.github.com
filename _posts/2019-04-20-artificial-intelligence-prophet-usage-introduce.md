---
title: Prophet 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

Facebook 提供的 Prophet 算法主要是用来处理单变量的时间序列，对于一些异常数据以及确实的情况可以很好的处理，对于周期性的数据几乎可以全自动地预测未来的走势。

这里简单介绍其使用方法。

<!-- more -->

## 简介

该算法是基于时间序列分解以及机器学习的拟合完成，其中的拟合采用 Stan 实现，同时提供了 R 以及 Python 的接口。

### 安装

#### Linux

依赖一些基本的包，例如 `pandas` `matplotlib` `numpy`，其它的一些包可以通过 virtualenv 进行独立的安装。

{% highlight text %}
----- 准备虚拟环境
$ virtualenv prophet --python=python3.6
$ source prophet/bin/activate
{% endhighlight %}

其中部分库，例如 `ephem` 需要 Python 头文件，因此需要安装 `python36-devel` 。

{% highlight text %}
----- 安装依赖库
$ pip3 install pystan fbprophet
{% endhighlight %}

#### Windows

在 Windows 上建议使用 [Anaconda](https://www.anaconda.com) 安装，可以从 [Anaconda Distribution](https://www.anaconda.com/distribution/) 下载相关的安装包。

> Anaconda 是一个 Python 的发行版本，包括了一个跨平台的包管理工具 Conda，常用于一些数据分析场景中的环境准备，可以很好的解决依赖，尤其是像 numpy scipy 这种对底层 C 库有依赖的包管理。

可以通过如下命令使用。

{% highlight text %}
----- 创建一个Prophet的基本环境，可以激活、退出、查看包、删除
conda create --name prophet python=3.7
conda activate prophet
conda deactivate
conda list --name prophet
conda remove --name prophet --all
{% endhighlight %}

创建的环境会保存在安装目录的 `envs` 目录下，每个环境作为一个单独的目录保存。

{% highlight text %}
----- 安装相关的依赖
conda install pystan
conda install -c conda-forge fbprophet
{% endhighlight %}

### 示例

在 [GitHub FaceBook](https://github.com/facebook/prophet/blob/master/examples) 中提供了相关的示例，这里采用 `example_wp_log_peyton_manning.csv` 进行简单的测试。




{% highlight text %}
Prophet()              fbprophet/forecaster.py
predict_trend()
 |-piecewise_linear()
 |-piecewise_logistic()
{% endhighlight %}





## 其它

### 异常

在安装相关的版本库时，可能会遇到如下的报错。

#### PyStan

在安装 `pystan` 时会出现 `error: command 'gcc' failed with exit status 1` 的报错，此时需要确保 `libffi-devel` `python-devel` `openssl-devel` 开发库已经安装。

安装完成之后，可以通过如下的代码进行测试。

{% highlight python %}
import pystan

model_code = 'parameters {real y;} model {y ~ normal(0,1);}'
model = pystan.StanModel(model_code=model_code)  # this will take a minute
y = model.sampling(n_jobs=1).extract()['y']
y.mean()                                         # should be close to 0
{% endhighlight %}

#### FBProphet

该库对 pystan 的版本也有依赖，目前看支持的版本有 `2.17.1.0` ，可以通过如下方式安装。

{% highlight text %}
$ pip3 uninstall pystan
$ pip3 install pystan==2.17.1.0
{% endhighlight %}

在导入时可能会报错 `Importing plotly failed. Interactive plots will not work.`，那么此时就需要安装 `plotly` 包。

{% highlight text %}
$ pip3 install plotly
{% endhighlight %}

在 Windows 中，可以直接通过 `conda install plotly` 安装包。

## 参考

<!--
curl -O https://assets.digitalocean.com/articles/eng_python/prophet/AirPassengers.csv





均方根误差
https://blog.csdn.net/FrankieHello/article/details/82024526



微积分的历史
https://zhuanlan.zhihu.com/p/31963102
https://www.zhihu.com/question/22199657?rf=35793939
https://www.zhihu.com/question/26201436
https://zhuanlan.zhihu.com/p/26855264
https://math.fudan.edu.cn/gdsx/LUNWEN/%E5%BE%AE%E7%A7%AF%E5%88%86%E6%80%9D%E6%83%B3%E7%9A%84%E4%BA%A7%E7%94%9F%E4%B8%8E%E5%8F%91%E5%B1%95%E5%8E%86%E5%8F%B2.pdf

泰勒公式
https://www.zhihu.com/question/25627482
https://zh.wikipedia.org/wiki/%E6%B3%B0%E5%8B%92%E5%85%AC%E5%BC%8F
https://www.cnblogs.com/alexanderkun/articles/3866053.html
https://www.matongxue.com/madocs/7/
https://charlesliuyx.github.io/2018/02/16/%E3%80%90%E7%9B%B4%E8%A7%82%E8%AF%A6%E8%A7%A3%E3%80%91%E6%B3%B0%E5%8B%92%E7%BA%A7%E6%95%B0/

编译器的设计
https://www.tutorialspoint.com/compiler_design/index.htm


梯度下降验算示例，以及分类示例
https://towardsdatascience.com/an-introduction-to-gradient-descent-c9cca5739307

Loss Function介绍
https://zhuanlan.zhihu.com/p/37663120

Rosenbrock 函数是一个经典的非凸优化函数，由 Howard Harry Rosenbrock 在 1960 年提出，通常用于测试优化算法的性能，其最小值位于一个狭小的形似山谷的位置，找到山谷很容易，但是很难找到最小值。

其函数定义为。

$$f(x, y)=(a - x)^2 + b(y - x^2)^2$$

在 $(a, a^2)$ 处有全局的最小值 $0$ ，使用通常会选取 $a=1, b=100$ 。

各种梯度下降算法的总结，那是相当全
https://zhuanlan.zhihu.com/p/77380412
https://ruder.io/optimizing-gradient-descent/

https://blog.csdn.net/pengjian444/article/details/71075544
Rosenbrock 的介绍
https://github.com/syahrulhamdani/Gradient-Descent-for-Rosenbrock-Function
https://upload.wikimedia.org/wikipedia/commons/3/32/Rosenbrock_function.svg
https://xavierbourretsicotte.github.io/Intro_optimization.html

导数、偏导数、梯度等的含义
https://www.jianshu.com/p/4d3210c67c42
https://blog.csdn.net/qq_36330643/article/details/78603550
针对Rosenbrock的梯度下降算法实现
https://blog.csdn.net/pengjian444/article/details/71075544

不只是梯度下降算法
http://people.duke.edu/~ccc14/sta-663-2018/notebooks/S09G_Gradient_Descent_Optimization.html

Mean Square Error, MSE 均方误差

Exponentially Weighted Average 指数移动平均

梯度是一个向量，标示了函数增速最快的方向 (???)，对其取反向之后就是降速最快的方向，梯度下降就是通过梯度的反向进行迭代，其步长决定了趋近最小值的速度。

梯度下降算法有几个变形，无非是根据数据量在参数精度以及执行时间的平衡。

Batch Gradient Descent
Stochastic Gradient Descent
Mini-batch Gradient Descent

PyStan官方文档
https://pystan.readthedocs.io/en/latest/
https://pystan.readthedocs.io/en/latest/_modules/pystan/model.html

LBFGS BFGS Newton

牛顿法最优化
http://www.go60.top/view/81.html 包含了机器学习的大部分算法
https://blog.csdn.net/u012705410/article/details/47209007

https://zhuanlan.zhihu.com/p/29672873
http://www.chokkan.org/software/liblbfgs/


概率编程
https://mc-stan.org/
基于PyMC的概率编程与贝叶斯推断介绍，最新可以参考如下内容
https://github.com/CamDavidsonPilon/Probabilistic-Programming-and-Bayesian-Methods-for-Hackers




#### make_future_dataframe()

一个辅助函数，用来扩展 `ds` 列，需要根据时间的间隔以及期望扩展时间设置参数。

* `periods=356` 增加的长度。
* `freq='D'` 指定粒度，详细参考 [Pandas DataOffset](https://pandas.pydata.org/pandas-docs/stable/user_guide/timeseries.html#dateoffset-objects)。

## 简介

https://github.com/facebook/prophet
https://github.com/facebook/prophet/tree/master/examples

FaceBook 提供了与 sklearn 库一致的接口，当创建了一个 Prophet 实例后，可以通过 `fit()` 做拟合，并通过 `predic()` 进行预测。

使用的数据包含了两列数据：A) `ds` 表示时间戳；B) `y` 预测的变量；所以，当使用 `pd.dataframe` 时，假设原来的列名为 `timestamp` 和 `value` ，那么就可以通过如下方式进行转换。

df = df.rename(columns={'timestamp':'ds', 'value':'y'})

如果时间使用的时间戳，那么可以通过如下方式转换为 `YYYY-MM-DD hh:mm:ss` 的格式。

df['ds'] = pd.to_datetime(df['ds'], unit='s')

预测的结果中，比较关键的有 `yhat` `yhat_lower` `yhat_upper`，分别表示时间序列的预测值及其上下界。

在分析时间序列时，一种常见的分析方法是时间序列分解 (Decomposition of Time Series)，一般会将时间序列分成几个部分，包括了季节项、趋势项、剩余项，通常还有节假日的效应。所以，在 Prophet 算法里面，同时考虑了以上四项，也就是：

## 预测

常见的算法有 Holt-Winters 指数平滑、ARIMA、DeepLearning 的 LSTM

Prophet 大规模时序预测
https://zhuanlan.zhihu.com/p/52330017 包括各个参数的使用
https://blog.csdn.net/chivalrousli/article/details/60956587
https://vectorf.github.io/2017/03/14/20170314-Prophet%E4%B9%8B%E4%BD%BF%E7%94%A8%E7%AF%87%EF%BC%88%E4%B8%89%EF%BC%89/

详细代码可以参考 [Prophet Quick Start](https://facebook.github.io/prophet/docs/quick_start.html) 中的介绍。

Prophet 是通过 R 语言实现的代码，另外，通过 Python 语言对其进行封装。所以，如果要研究其源码，最好还是查看 R 语言的实现。

与 Python 相关的代码保存在 `python` 目录下。

m = Prophet() fbprophet/forecaster.py 主要是创建一个对象

"LBFGS", "BFGS", "Newton"

针对Spark中的ML模块的分析
https://github.com/endymecy/spark-ml-source-analysis

predictive_samples() API 访问原始的后验预测值，会调用 sample_posterior_predictive() 完成。

fit()
 |-set_auto_seasonalities()
 |-make_all_seasonality_features()
 |-set_changepoints()
 |-get_prophet_stan_model() 会在fbprophet目录中持久化一个stan模型，也就是stan_model/prophet_model.pkl，会在初始化的时候安装
 |-stan_init()

predict()
 |-predict_uncertainty()
   |-sample_posterior_predictive()
     |-sample_model()
       |-sample_predictive_trend()
         |-piecewise_linear()
         |-piecewise_logistic()

在安装 `fbprophet` 时，会在 `setup.py` 中的 `build_stan_model()` 函数中生成并持久化一个 Stan 模型，对应的模型在 `prophet.stan` 文件中定义。

$ find -name 'prophet.stan'
./R/inst/stan/prophet.stan
./python/stan/win/prophet.stan
./python/stan/unix/prophet.stan

在使用 Python 时，因为不同的版本，甚至采用类似 `virtualenv` 的方式安装，会导致各个安装包的路径不同。

实际上，在 settools 中提供了一个 `pkg_resources` 包，可以用来定位包的路径，例如已经安装了 `numpy` ，那么可以通过如下方式确定 `doc` 目录在当前系统中的绝对路径。

>>> import pkg_resources
>>> pkg_resources.resource_filename("numpy", "doc/")

https://github.com/cjlin1/liblinear
https://github.com/client9/libinjection

机器学习——几种距离度量方法比较
https://my.oschina.net/hunglish/blog/787596
-->

{% highlight text %}
{% endhighlight %}
