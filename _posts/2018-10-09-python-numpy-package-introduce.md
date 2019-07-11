---
title: Python Numpy 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,numpy
description: NumPy 是 Python 的一个扩充程序库，支持高级大量的维度数组与矩阵运算，此外也针对数组运算提供大量的数学函数库。内部解除了 Python 的 PIL (全局解释器锁)，同时使用 C/C++ 做扩展，运算效率极好，是大量机器学习框架的基础库。
---

NumPy 是 Python 的一个扩充程序库，支持高级大量的维度数组与矩阵运算，此外也针对数组运算提供大量的数学函数库。内部解除了 Python 的 PIL (全局解释器锁)，同时使用 C/C++ 做扩展，运算效率极好，是大量机器学习框架的基础库。

<!-- more -->

## 常用操作

#### 数组操作

{% highlight text %}
#----- 初始化，分别初始化为1、0、顺序、随机数
np.array([0, 1, 2, 3, 4])
np.ones(3)
np.zeros(3)
np.arange(5)
np.arange(0, 5)
np.random.random(3)

#----- 计算，可以是等维的数组，也可以是标量
np.ones(3) + np.zeros(3)
np.ones(3) * 10.

#----- 切片操作，与Python的切片基本类似，序号从0开始
data = np.arange(5)
data[0:2]
data[1:]

#----- 聚合函数，包括了min max sum mean std(标准差)
data.max()
{% endhighlight %}

#### 矩阵操作

{% highlight text %}
#----- 初始化，可以用数组或者函数初始化，大小为3行*2列
np.array([[1, 2], [3, 4], [5, 6]])
np.ones((3, 2))
np.zeros((3, 2))
np.random.random((3, 2))

#----- 计算，同维的可以直接执行四则运算，可能会执行广播规则
np.ones((3, 2)) + 2 * np.ones((3, 1))
------ 也可以是矩阵乘法，需要严格确保纬度满足要求，如下两种方式相同
a = np.array([1, 2, 3])
b = np.array([[1, 2], [3, 4], [5, 6]])
np.dot(a, b)
a.dot(b)

#----- 切片操作，与数组相似，只是多了几个纬度而已
data = np.ones((3, 2))
data[0, 1]
data[1:3]      # 两个纬度都是1:3
data[0:2, 0]

#----- 聚合函数，可以指定具体的纬度
data.max(axis=0)
{% endhighlight %}


## NDArray

这是一个多维数组对象，该对象由 `实际数据` 和 `元数据` 组成，其中大部分操作仅仅修改元数据部分，而不改变底层的实际数据。

注意，实际的数据必须要保证是同质的。

{% highlight python %}
import numpy as np

#----- 多维可以通过shape属性查看维度
arr.shape

#----- 修改维度
arr.shape = 3, 2
arr = arr.reshape(3, 2)

#----- 数组展开，前者只是展示格式不同，而后者会申请内存
arr.ravel()
arr.flatten()

#----- 转置矩阵
arr = np.arange(6).reshape(2, 3)
arr.transpose()

#----- 其它
arr = np.arange(6).reshape(2, 3)
arr.dtype     # 类型
arr.shape     # 维度(行, 列)
arr.ndim      # 行
arr.size      # 总大小，也就是 行*列
arr.itemsize  # 单个元素的大小
arr.data      # 真正的数据信息，包括地址、元素个数等
{% endhighlight %}

<!--
#### 其它
http://www.sohu.com/a/325758681_505915
转置可以直接使用 T 或者用 reshape() 函数
-->


## 广播规则

注意，执行 Broadcast 的前提在于，两个矩阵执行的位运算，而非矩阵乘法运算 `np.dot(A, B)` ，后者相对来说会更加严格。

最简单的示例如下，两者等价，其中后者用到了广播规则。

{% highlight text %}
>>> a = np.array([1.0, 2.0, 3.0])
>>> b = np.array([2.0, 2.0, 2.0])
>>> a * b
array([ 2.,  4.,  6.])

>>> a = np.array([1.0, 2.0, 3.0])
>>> b = 2.0
>>> a * b
array([ 2.,  4.,  6.])
{% endhighlight %}

简单来说，在执行运算时，需要满足：

* 当前维度的值相等 (行、列都相同)；
* 当前维度的值有一个是 1 (另外一个相同)。

测试示例如下。

{% highlight text %}
a = np.array([
	[ 0,  0,  0],
	[10, 10, 10],
	[20, 20, 20],
	[30, 30, 30]])
b = np.array([1, 2, 3])
#b = np.array([1, 2, 3, 4])  会报错
#ValueError: operands could not be broadcast together with shapes (4,3) (4,)
print(a + b)
{% endhighlight %}

![numpy broadcast example]({{ site.url }}/images/ai/numpy-broadcast-example.gif "numpy broadcast example"){: .pull-center }

另外的形式与上类似。

{% highlight text %}
a = np.array([
	[ 0,  0,  0],
	[10, 10, 10],
	[20, 20, 20],
	[30, 30, 30]])
b = np.array([1, 2, 3, 4]).reshape(4, 1)
#b = np.array([1, 2, 3]).reshape(3, 1)  会报错
#ValueError: operands could not be broadcast together with shapes (4,3) (3,1)
print(a + b)
{% endhighlight %}

## 随机数

这一模块用于生成随机数，包含了一系列的函数。

#### rand()

{% highlight python %}
numpy.random.rand(d0, d1, ..., dn)
{% endhighlight %}

生成范围是 `[0, 1)` 的数据，数据是均匀分布，其中 `dN` 代表了维度，例如 `rand(4, 2)` 生成 4 行 2 列的数据。

#### randn()

{% highlight text %}
numpy.random.randn(d0, d1, ..., dn)
{% endhighlight %}

与 `rand()` 函数类似，只是生成的为正态分布。

#### uniform()

{% highlight text %}
numpy.random.uniform(low=0.0, high=1.0, size=None)
{% endhighlight %}

用于生成均匀分布 (uniform distribution) 的随机数，其中 `size` 可以是数值或者元组。

### RandomState

其入参可以是 `int` `array` `None` ，与 `seed()` 函数类似，其中 `RandomState()` 是线程安全的，相比来说更为复杂，可以替换 `rand()` 函数。

使用 `RandomState()` 获得随机数生成器时，只要种子相同那么最终生成的序列也就是相同的，一般的使用方法如下：

{% highlight text %}
import numpy as np

for i in xrange(4):
    rnd = np.random.RandomState(1)
    print rnd.uniform(0, 1, (2, 3))
{% endhighlight %}

### 正态随机值

{% highlight python %}
numpy.random.normal(loc=0.0, scale=1.0, size=None)
{% endhighlight %}

生成一个符合正态分布的随机序列，详细可以参考 [docs.scipy.org](https://docs.scipy.org/doc/numpy/reference/generated/numpy.random.normal.html) 中的介绍。

{% highlight python %}
import numpy
import matplotlib.pyplot as plt

mean = 0
sigma = 1
arr = numpy.random.normal(mean, sigma, size=1000)
abs(mean - np.mean(s)) < 0.01
abs(sigma - np.std(s, ddof = 1)) < 0.01

plt.plot(arr)
plt.show()
{% endhighlight %}

<!--
此时生成的是区间为 `[0, 1]` 的均匀分布随机数。
https://www.jianshu.com/p/214798dd8f93

-->

## 常用技巧

### linspace()

{% highlight python %}
numpy.linspace(start, stop, num=50, endpoint=True, retstep=False, dtype=None)
{% endhighlight %}

在指定的范围内生成间隔均匀的数字，开始值、终值和元素个数创建表示等差数列的一维数组，可以通过 endpoint 参数指定是否包含终值，默认值为 True，即包含终值。

与之类似的是 `arange()` 函数，不过该函数不含终止值。

### meshgrid()

简单来说，就是生成网格，最常用的是将在两个坐标轴上的点在平面上画网格，如下是一个简单的示例：

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

X, Y = np.meshgrid(np.linspace(0, 1, 5), np.linspace(0, 1, 3))
plt.plot(X, Y, marker='.', color='blue', linestyle='none')
plt.show()
{% endhighlight %}

这在很多的示例中会使用，后面再详细介绍。

### histogram()

用来统计一个数组的直方库，也可以使用 `plt.hit()` 函数直接绘制一个图。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

arr = np.array([22, 87, 5, 43, 56, 73, 55, 54, 11, 20, 51, 5, 79, 31, 27])
hist, bins = np.histogram(arr, bins = [0, 20, 40, 60, 80, 100])

plt.hist(arr, bins = [0, 20, 40, 60, 80, 100])
plt.title("histogram")
plt.show()
{% endhighlight %}

### cumsum()

`cumsum()` 函数用于计算累加，可以有如下的几种方式。

{% highlight python %}
import numpy as np

# 1 2
# 2 3
a = np.cumsum([[1, 2], [2, 3]])
print(a)   #[1 3 5 8] 由前面的值依次累加

b = np.cumsum([[1, 2], [2, 3]], axis=0)
print(b)   # [[1 2] [3 5]] 每列累加

c = np.cumsum([[1,2],[2,3]],axis=1)
print(c)   # [[1 3] [2 5]] 每行累加
{% endhighlight %}

## 参考

* [Numpy Broadcasting](https://docs.scipy.org/doc/numpy-1.15.0/user/basics.broadcasting.html) 关于 Numpy 中的 Broadcasting 的相关介绍。

{% highlight text %}
{% endhighlight %}
