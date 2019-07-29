---
title: Prophet 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->

## 简介

### 环境准备

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

## 参考

<!--
curl -O https://assets.digitalocean.com/articles/eng_python/prophet/AirPassengers.csv
-->

{% highlight text %}
{% endhighlight %}
