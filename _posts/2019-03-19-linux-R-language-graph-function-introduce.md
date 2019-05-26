---
title: R 语言绘图
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: r,language
description: 原生的 R 语言提供了非常强大的绘图功能，基本不需要其它辅助，就可以绘制非常炫目的图片，再加上各种各样的神级 R 包 (例如 `ggplot2`、`plotly` 等)，更是如虎添翼。 另外，Python 语言中的 Matplotlib 库，同样提供了非常强大的绘图功能。
---

原生的 R 语言提供了非常强大的绘图功能，基本不需要其它辅助，就可以绘制非常炫目的图片，再加上各种各样的神级 R 包 (例如 `ggplot2`、`plotly` 等)，更是如虎添翼。

另外，Python 语言中的 Matplotlib 库，同样提供了非常强大的绘图功能。

<!-- more -->

## 基本绘图

包括了散点、折线、曲线。

{% highlight text %}
x<-c(1, 2, 3, 4, 5, 6)
y1<-c(3, 8, 19, 24, 6, 1)
y2<-c(1, 25, 21, 3, 2, 1)
{% endhighlight %}

#### 散点图 && 折线图

两者都可以通过 `plot()` 函数进行绘制，区别是前者使用默认的 type 值，也就是 `type="p"`；后者需要设置为 `type="b"`。

{% highlight text %}
plot(x, y1, col="red", pch=1, xlim=c(0,6), ylim=c(0,30), xlab="DAY", ylab="DATA")
参数：
  col="red" 点的颜色
  pch=1 符号属性 0~25 常见的如，0 正方形、1 原形、2 三角形等
  bg="green" 设置点的填充色，注意，只有形状为21~25的点是有填充色的
  xlim=c(0,6) ylim=c(0,30) 坐标刻度范围
  xlab="DAY" ylab="DATA" 坐标标题
  type="p" 绘图类型，p 点、l 线、b 点线
{% endhighlight %}
  
其中散点图也可以通过 `plot() + points()` 函数进行绘制。

{% highlight r %}
plot(1:5, 1:5, xlim=c(0,6), ylim=c(0,30), type = "n")
points(x, y1, type="p")
参数：
  cel=1/c(2,4) 用来表示大小
  lwd=1/c(2,4) 设置点的边框的宽度
{% endhighlight %}

对于折线图，也可以通过 `lines()` 函数绘制。

{% highlight r %}
lines(x, y2, col="green", pch=16, xlim=c(0,6), ylim=c(0,30), xlab="WEEK", ylab="STUDENT", type="b")
参数：
  lty=N  1 实线连接 2 虚线 3 点线
{% endhighlight %}

#### 光滑曲线

{% highlight r %}
sp1 = spline(x, y1, n=1000)
sp2 = spline(x, y2, n=1000)
plot(sp1, col="red", type="l" ,xlim=c(0,6), ylim=c(0,30))
{% endhighlight %}

首先需要通过 `spline()` 函数生成相关的数据集，其中 `n` 表示插值的数量。

### 保存图片

{% highlight text %}
> savePlot(filename="foobar.png", type="png", device=dev.cur())
{% endhighlight %}

## 其它

#### 设置栅格

{% highlight text %}
grid(nx=6, ny=6, lwd=2)
  nx ny 设置 X Y 轴方向的栅格的宽度；
  lwd 栅格线的宽度
{% endhighlight %}

#### 增加图例

{% highlight text %}
legend("topright", legend=c("y1","y2"), pch=c(1,16), col=c("red","green"), lwd=2, lty=c(1,2))
{% endhighlight %}

#### 绘制参数

通过 `par()` 函数可以设置或者查询图像的参数。

`mfcol`, `mfrow` 设置小图数量与位置，设置为 `c(nr, nc)` 表示把图分为 nr 行 nc 列个小图，图形顺序按列排 (mfcol) 或按行排 (mfrow)。

<!--
https://www.r-project.org/nosvn/pandoc/mmand.html
x <- seq(0, 4*pi, pi/64)
y <- cos(x) + runif(length(x), -0.2, 0.2)
y_smoothed <- gaussianSmooth(y, 6)

plot(x, y, pch=19, col="grey50", xaxt="n")
axis(1, (0:4)*pi, expression(0,pi,2*pi,3*pi,4*pi))
lines(x, y_smoothed, lwd=2)
-->

## 参考

* [R graph gallery](https://www.r-graph-gallery.com/) 一些绘图的基本示例，在使用时可以参考。
* [R 语言绘图速查手册](https://yulijia.net/projects/SimpleGraphswithR.pdf) 或者保存的 [本地文档](/reference/programs/SimpleGraphswithR.pdf) 。

<!--
https://www.jianshu.com/p/9a164ba9e673
https://zhuanlan.zhihu.com/p/25074456
-->

{% highlight text %}
{% endhighlight %}
