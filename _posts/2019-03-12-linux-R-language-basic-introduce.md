---
title: R 语言简介
layout: post
comments: true
language: chinese
category: [program]
keywords: r,language
description: R 语言作为统计学一门语言，一直属于一个小众的工具集，直到大数据爆发后，越来越多的人开始使用 R 语言进行数据分析。是一套开源的数据分析解决方案，为统计计算和绘图而生的语言和环境，几行简答的代码就可以实现很复杂的功能。
---

R 语言作为统计学一门语言，一直属于一个小众的工具集，直到大数据爆发后，越来越多的人开始使用 R 语言进行数据分析。

是一套开源的数据分析解决方案，为统计计算和绘图而生的语言和环境，几行简答的代码就可以实现很复杂的功能。

<!-- more -->

## 简介

R 语言最初由 Ross Ihaka 和 Robert Gentleman 在新西兰奥克兰大学统计系设计开发，于 1993 年首次对外公开发布。

### 安装

最简单的方式是直接安装已经编译好的二进制包。

#### Windows

关于 Windows R 的安装，关键有三个对象：

* 基本运行 R 环境，可以从 [CRAN tsinghua mirrors](https://mirrors.tuna.tsinghua.edu.cn/CRAN/bin/windows/base) 中选择版本下载。
* 可选 RStudio 一个编辑环境，可以从 [www.rstudio.com](https://www.rstudio.com/products/rstudio/download/) 下载免费版本。

安装完 R 环境之后可以直接启动一个操作终端，可以不安装 RStudio 。

<!--
（可选）安装Rtools，这个是用来编译源码包的，一般是含有其他语言写的包需要该步骤，下载
-->

#### Linux

R 是通过 C 语言编写的，可以通过源码安装，也可以直接从 [CRAN Mirrors](https://cran.r-project.org/mirrors.html) 安装已经编译好的二进制包。

{% highlight text %}
# yum install readline-devel
# yum install libXt-devel
# mkdir -p /opt/R
$ ./configure --prefix=/opt/R --enable-R-shlib
$ make
# make install
{% endhighlight %}

对于 CentOS 来说，需要安装 [EPEL Repositories](https://fedoraproject.org/wiki/EPEL) 仓库源，然后通过 `yum install R` 进行安装。

最后直接通过 `R` 命令启动即可。

### 基本概念

R 是一种基于对象 (Object) 的语言，所有的东西都作为一个对象，包括了向量、函数、图形等等，那么在 R 中面向对象的编程也基于此。

对象中包含了很多属性，最重要或者最基本的有 `Mode` 和 `Class` ，前者表示对象在内存中是如何存储的 (numeric character list function) ，后者表示对象的抽象类型。

其中最基本的类有：数值 (numeric)、逻辑 (logical)、字符 (character)、列表 (list)，以及在此基础上构成了一些复杂的类，例如矩阵 (matrix)、数组 (array)、因子 (factor)、数据框 (dataframe)。

用户还可以自定义新类，但所有的类都是建立在这些基本的类之上，可以通过 `class()` `mode()` `typeof()` 查看相关的属性。

{% highlight text %}
> f <- data.frame(v = c(1, 2))
> class(f)                        # 对象的类
[1] "data.frame"
> mode(f)                         # 实际的存储类型
[1] "list"
> typeof(f)
[1] "list"
{% endhighlight %}

因为 R 是从 S 发展而来的，其中 `mode()` 是为了兼容 S ，相比来说 `typeof()` 是更新一种形式；而 `class()` 和前两者不是一套系统，它涉及到 R 语言的面向对象，返回的是该对象的类属性。

### 示例

{% highlight text %}
----- 帮助相关
> help(solve)      # 查看帮助信息
> ?solve           # 同上
> help.start()     # 启动一个WEB服务器，感觉终端下不太好用
> example(solve)   # 查看示例

----- 变量赋值，通过函数c()可以将参数组成一个新的向量
> x <- c(14.5, 9.8, 20.8, 4.5)
> c(14.5, 9.8, 20.8, 4.5) -> x
> assign("x", c(14.5, 9.8, 20.8, 4.5))
> 1/x              # 将所有的值取倒数
{% endhighlight %}

除了其它语言中都有的一些数据类型、操作符之外，还有其特有的，包括：向量、列表、矩阵、数组、因子、数据帧

## 软件包

也就是函数、编码、样本数据的集合，可以通过如下方式查看使用。

{% highlight text %}
> sessionInfo() # 当前R解析器的版本以及运行平台信息
> .libPaths()   # 当前所有软件包路径
> library()     # 所有安装的软件包名称，在上述的路径下
> search()      # 当前运行环境已经加载的包
{% endhighlight %}

可以通过如下方式安装相关的软件包，默认是从 CRAN 仓库中安装，允许在安装时指定路径。

{% highlight text %}
> install.packages("XML", lib="your/path")   # 安装时指定路径
> install.packages("XML", repos=c(CRAN="http://mirrors.ustc.edu.cn/CRAN/"))
> installed.packages()                    # 当前已经安装的软件包
> update.packages("XML")                  # 升级
{% endhighlight %}

如果没有通过 `lib` 参数指路径，而且对标准目录没有访问权限，会提示创建一个默认的目录，也可以通过 `R_LIBS_USER="~/R"` 环境变量指定，对于 CentOS 来说会默认加载 `/usr/lib64/R/etc/Renviron` 指定的路径。

> 启动时会加载一系列的配置文件，包括执行一些系统脚本，可以通过 `help(Startup)` 命令查看详细信息。

另外，包路径也可以通过 `R_LIBS` 环境变量进行设置，或者在解析器中通过 `.libPaths("path/to/your/lib")` 命令指定。

或者通过 `library("package_name", lib.loc = "path/to/your/lib")` 在加载时指定包的路径。


### 列表

列表是一种比较特殊的对象集合，其中的元素可以是任意类型，而且类型不必相同，它的元素可以由序号访问。

{% highlight text %}
student <- list(name="foobar", age=30, scores=c(85, 76, 90))
student            # 查看所有
student[1]         # 访问第一个元素
student$name       # 访问name元素
student$age <- 40  # 修改年龄
{% endhighlight %}

### 函数

使用关键字 `function` 来创建一个 R 函数，其定义的基本语法如下：

{% highlight text %}
function_name <- function(arg1, arg2, ...) {
	# function body, some statements
	return(object)
}
{% endhighlight %}

如下是自定义的函数，可以有参数，也可以为空，可以设置参数的默认值。

{% highlight text %}
foobar <- function(a, b = 10) {
	print(a + b)
}
foobar(1)
foobar(1, 10)
foobar(a = 1)
foobar(a = 1, b = 10)
{% endhighlight %}

上述的几个函数的调用结果相同。

另外，需要注意，函数的参数如果有错误，只有在使用时才会报错，例如：

{% highlight text %}
foobar <- function(a, b) {
	print(a + b)
}
foobar(1)
{% endhighlight %}

会报 `Error in foobar(1) : argument "b" is missing, with no default` 的错误。

### sample

这也是最常用的随机抽样功能，默认使用的是不放回抽样，也可以通过参数设置为放回抽样。

{% highlight text %}
> sample(x=1:10)            # 非放回抽样，抽样10次
> sample(x=1:100, size=10)  # 非放回抽样，100个样本，抽样10次
> sample(x=1:100, size=10, replace=T) # 放回抽样
{% endhighlight %}

注意，默认的入参顺序是 `(x, size, replace)` ，如果实际的入参顺序相同，那么实际上可以直接使用，例如 `sample(1:100, 10)` 。

对于 `x` 参数来说，可以是数值 (含浮点数)，也可以是字符串，例如掷骰子、抛硬币，实际上就属于放回抽样。抛硬币可以使用如下示例：

{% highlight text %}
> sample(x=c("H", "T"), size=10, replace=T)
{% endhighlight %}

另外，对于某些二项分布来说，实际抽样的概率未必相等，那么这时就可以使用 `prob` 参数，这时 probability 的缩写。示例如下：

{% highlight text %}
> sample(x=c("H", "T"), size=10, replace=T, prob=c(0.8,0.2))
{% endhighlight %}

在 prob 参数中，所有元素的概率加起来不需要等于 1 ，它只代表某元素被抽取的概率而已。

## 统计示例

详细可以通过 `help(Distributions)` 命令查看。

### 基本统计值

简单来说就是如何求平均值、中位数等。

#### 平均值

取数值的总和并除以数据序列中的值的数量，可以通过函数 `mean()` 来计算平均值。

{% highlight text %}
mean(x, trim = 0, na.rm = FALSE, ...)
{% endhighlight %}

使用参数如下：

* `x` 输入向量。
* `trim` 范围是 `0~0.5` 排序之后从两端删除 `N%` 的数据，类似于去除部分最大最小值。
* `na.rm` Bool 值，是否删除缺少的值。

{% highlight text %}
> print(mean(c(8, 1, 3, 2, 5, 6, 7, 9, 4, 17)))
[1] 6.2
{% endhighlight %}

如下示例，会先将向量中的值排序 `1 3 4 5 8 9 10`，当 `trim = 0.1` 时从每一端删除 `10%` 的值，也就是各删除 1 个值，然后再计算均值。

{% highlight text %}
> print(mean(c(8, 1, 3, 2, 5, 6, 7, 9, 4, 17), trim = 0.1))
[1] 5.5
{% endhighlight %}

如果缺少值，也就是向量中含有 `NA` 默认会返回 `NA` ，此时可以通过 `na.rm = TRUE` 将缺少的值删除。

{% highlight text %}
> print(mean(c(8, 1, 3, 2, 5, 6, 7, 9, 4, 17, NA)))
[1] NA
> print(mean(c(8, 1, 3, 2, 5, 6, 7, 9, 4, 17, NA), na.rm = TRUE))
[1] 6.2
{% endhighlight %}

#### 中位数

数据系列中的中间值被称为中位数，如果是偶数个，则会将中间的两个值相加除 2 ，可以通过 `median()` 函数来计算中位数。

其语法为 `median(x, na.rm = FALSE)` ，其中 `na.rm` 参数与上类似。

{% highlight text %}
> print(median(c(1, 3, 2, 4)))
{% endhighlight %}

### 线性回归

最简单的回归分析，在 R 中可以直接使用 `lm()` 函数进行模拟，第一个参数 `formula` 定义了模型参数，实际上就是 `X` `Y` 对应的向量值。

如下是简单的使用方法。

{% highlight text %}
> x <- c(0.10, 0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.20, 0.21, 0.23)
> y <- c(42.0, 43.5, 45.0, 45.5, 45.0, 47.5, 49, 53, 50, 55, 55, 60)
> plot(y~x)
> model <- lm(y~x)
> summary(model)  # 下面是输出的分析
Call:
lm(formula = y ~ x)

Residuals:
    Min      1Q  Median      3Q     Max 
-2.0431 -0.7056  0.1694  0.6633  2.2653 

Coefficients:
            Estimate Std. Error t value Pr(>|t|)    
(Intercept)   28.493      1.580   18.04 5.88e-09 ***
x            130.835      9.683   13.51 9.50e-08 ***
---
Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1

Residual standard error: 1.319 on 10 degrees of freedom
Multiple R-squared:  0.9481,    Adjusted R-squared:  0.9429 
F-statistic: 182.6 on 1 and 10 DF,  p-value: 9.505e-08
{% endhighlight %}

然后可以通过如下方式进行预测。

{% highlight text %}
> new <- data.frame(x = 0.16)     # 这里的输入数据必须是frame类型
> predict(model, newdata = new, interval = "prediction", level = 0.95) # level 指定预测的置信区间
{% endhighlight %}

<!--
http://lemondy.github.io/2015/07/01/R-%E7%BA%BF%E6%80%A7%E5%9B%9E%E5%BD%92/
-->

<!--
### 正态分布

如下链接的图很好
https://zh.wikipedia.org/wiki/%E6%A8%99%E6%BA%96%E5%88%86%E6%95%B8
https://upload.wikimedia.org/wikipedia/commons/b/bb/Normal_distribution_and_scales.gif

在 R 中提供了四个内置函数来生成正态分布。

dnorm(x, mean, sd)
pnorm(x, mean, sd)
qnorm(p, mean, sd)
rnorm(n, mean, sd)

其中：

* `x` 数字向量。
* `mean` 样本数据的平均值，默认为 0 。
* `sd` 标准差，默认值为 1 。

各个函数的使用示例如下。

#### dnorm()

计算出给定均值、标准差之后在每个点的概率分布的高度。

x <- seq(0, 20, by = .1)
y <- dnorm(x, mean = 10, sd = 3.5)
plot(x, y)

#### pnorm()

该函数给出正态分布随机数小于给定数值的概率，也就是 "累积分布函数" 。

x <- seq(0, 20, by = .1)
y <- pnorm(x, mean = 10, sd = 3.5)
plot(x, y)

#### qnorm()

x <- seq(0, 1, by = 0.02) # 对应的范围是0~1
y <- qnorm(x, mean = 10, sd = 3.5)

#### rnorm()

用于生成正态分布的随机数，其输入为样本的大小，然后可以通过绘制直方图显示生成数值的分布。

y <- rnorm(500, mean=10, sd=3.5)
hist(y)
-->

## 参考

详细的内容可以直接从 [The R Project for Statistical Computing](https://www.r-project.org/) 官网查看。


<!--
[R语言简介和基本操作](https://www.jianshu.com/p/782f9b2bc245)
[R语言简介](https://www.zhiwutong.com/file/2018/20180617213926831.pdf)

https://www.yiibai.com/r
https://www.jianshu.com/p/7130cf7ca453

https://www.yiibai.com/r

R语言-启动项相关文件配置
http://www.cnblogs.com/cloudtj/articles/7119077.html
-->


{% highlight text %}
{% endhighlight %}
