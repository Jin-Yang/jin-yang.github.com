---
title: Stan 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: stan
description: 概率建模分析使用较多的三个软件有 Stan PyMc3 Edward ，其中后者是 Google 出品的，据说在某些场景下要比 Stan 快很多。Stan 是一种语言，可以在生物、物理、工程等领域进行统计建模、数据分析、预测等。
---

概率建模分析使用较多的三个软件有 Stan PyMc3 Edward ，其中后者是 Google 出品的，据说在某些场景下要比 Stan 快很多。

Stan 是一种语言，可以在生物、物理、工程等领域进行统计建模、数据分析、预测等。

<!-- more -->

![stan logo]({{ site.url }}/images/stan_logo.png "stan logo"){: .pull-center width="30%" }

## 简介

使用 stan 这种概率编程语言时，需要先理解下贝叶斯方法。

### 安装 RStan

RStan 为 R 提供了一个接口，安装方式可以查看 [RStan Getting Started](https://github.com/stan-dev/rstan/wiki/RStan-Getting-Started) 中的介绍。

Ubuntu 可以直接安装二进制文件，不过在 CentOS 中没有找到对应的二进制 RPM 包，所以还是从源码开始安装。

{% highlight text %}
----- 安装依赖包，需要使用curl-config命令
# yum install libcurl-devel
{% endhighlight %}

安装源码需要依赖 `CXX14` 实际上，只需要支持 `C++11` 特性即可。

{% highlight text %}
dotR <- file.path(Sys.getenv("HOME"), ".R")
if (!file.exists(dotR)) dir.create(dotR)
M <- file.path(dotR, "Makevars")
if (!file.exists(M)) file.create(M)
cat("\nCXX14FLAGS=-O3 -march=native -mtune=native -fPIC",
    "CXX14=g++ -std=c++11",
    file = M, sep = "\n", append = TRUE)
{% endhighlight %}

<!--
This is what worked for me:
CXX_STD = CXX14
CXX14 = g++ -std=c++11
CXX14FLAGS = -O3 -fPIC -Wno-unused-variable -Wno-unused-function -DBOOST_PHOENIX_NO_VARIADIC_EXPRESSION
-->

然后在 R 语言中执行 `library("rstan")` 加载该库即可。

## 参考

详细可以参考官方网站 [mc-stan.org](https://mc-stan.org/users/interfaces/) 中的相关页面。

<!--
https://github.com/stan-dev/rstan/wiki/RStan-Getting-Started-(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)

一些比较经典的参考资料
https://www.leiphone.com/news/201701/13iyBwtzlcaCx4QX.html
-->

{% highlight text %}
{% endhighlight %}
