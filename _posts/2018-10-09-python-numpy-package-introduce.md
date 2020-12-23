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

## linalg

在 Numpy 中的 `linalg` 模块包含了线性代数相关的函数，可以用来计算逆矩阵、特征值、线性方程组以及行列式等。

### 逆矩阵

需要保证矩阵是方阵且可逆，否则会抛出 `LinAlgError` 异常。

{% highlight text %}
>>> a = np.mat([[4, 7], [2, 6]])
>>> b = np.linalg.inv(a)
>>> b * a
{% endhighlight %}

最后可以通过 `b * a` 验证结果是否为单位矩阵。

### 线性方程组

可以求解如 $Ax = b$ 的线性方程组。

{% highlight text %}
>>> A = np.mat([[1, -2, 1], [0, 2, -8], [-4, 5, 9]])
>>> b = np.array([0, 8, -9])
>>> x = np.linalg.solve(A, b)
>>> np.dot(A, x)
{% endhighlight %}

<!--
https://www.cnblogs.com/xieshengsen/p/6836430.html
-->

### 其它

#### array VS. mat

在生成矩阵时，要求的格式不同，其中 `array()` 只能使用列表，而 `mat()` 可以使用字符串表示 (可以通过分号 `;` 或者逗号 `,` 分割)。

{% highlight text %}
>>> a = np.mat("1 2; 3 4")
>>> b = np.mat([[1, 2], [3, 4]])
>>> c = np.array([[1, 2], [3, 4]])
{% endhighlight %}

当然，生成的类型也不同，分别是 `matix` 以及 `array` 。

另外，比较关键的是计算矩阵的乘法，除了线性代数中严格的乘法定义，`numpy` 还支持对应位的乘积，也就是函数 `multiply()` ，两者的 **默认行为** 是不同的。

* `mat()` 矩阵乘积可以使用星号 `*` 或 `.dot()` 计算，对应位相乘则需要显示调用 `multiply()` 。
* `array()` 中的星号 `*` 与 `multiply()` 相同，计算矩阵乘积需要显示调用 `.dot()` 。

使用如上创建的矩阵，对应的结果如下。

{% highlight text %}
>>> np.multiply(a, b)
matrix([[ 1,  4],
        [ 9, 16]])
>>> a*b
matrix([[ 7, 10],
        [15, 22]])
>>> a.dot(b)
matrix([[ 7, 10],
        [15, 22]])
{% endhighlight %}

## NDArray

这是一个多维数组对象，该对象由 `实际数据` 和 `元数据` 组成，其中大部分操作仅仅修改元数据部分，而不改变底层的实际数据。注意，实际的数据必须要保证是同质的，也就是类型相同。

所谓的多维数组，常见的，如向量 (Vector) 是一维，矩阵 (Matrix) 是二维，张量 (Tensor) 是三维，每个维度都会对应一个坐标轴，如下是一个示例。

{% highlight text %}
>>> a = np.reshape(np.array(range(24)), [2, 3, 4])
>>> a
array([[[ 0,  1,  2,  3],
        [ 4,  5,  6,  7],
        [ 8,  9, 10, 11]],

       [[12, 13, 14, 15],
        [16, 17, 18, 19],
        [20, 21, 22, 23]]])
{% endhighlight %}

对于上述的数组，可以理解成从外向内的维度一次是 2、3、4 。

### axis

在 numpy 中的很多函数可以对多维数组进行统计，常见的有 `sum()` `mean()` `var()` `std()` ，下面以 `sum()` 函数为例，介绍其中的 `axis` 参数的含义。

对于 `sum()` 函数，官方的文档解释为 `Sum of array elements over a give dimension. It returns an array with the same shape as input, with the specified dimension removed.` ，也就是对指定维度求和，返回的结果只是少了 axis 那一个维度。

仍以上述示例为例。

{% highlight text %}
>>> np.sum(a, axis=0)   # (2, 3, 4) --> (3, 4)
array([[12, 14, 16, 18],
       [20, 22, 24, 26],
       [28, 30, 32, 34]])
>>> np.sum(a, axis=1)   # (2, 3, 4) --> (2, 4)
array([[12, 15, 18, 21],
       [48, 51, 54, 57]])
>>> np.sum(a, axis=2)   # (2, 3, 4) --> (2, 3)
array([[ 6, 22, 38],
       [54, 70, 86]])
{% endhighlight %}

注意，上述的 `np.sum(a, axis=2)` 与 `np.sum(a, axis=-1)` 是等价的。

### 常见操作

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

### RandomState VS. Seed

为了保证实验结果可以复现，尤其是一些示例代码，可以对一些随机数据加入可重复的特性，一般包括了两种方法，全局的还有独立的。

所谓全局，就是通过 `np.random.seed()` 函数设置一个种子，那么后续生成的随机值基本固定。

{% highlight python %}
import numpy as np

for i in range(4):
	np.random.seed(0)
	print(np.random.rand(10))
{% endhighlight %}

但这样会影响到所有全局随机函数，如果想只对部分函数生效，那么就可以使用如下方法。

{% highlight python %}
import numpy as np

for i in range(4):
	rnd = np.random.RandomState(1)
	print(rnd.rand(10))
{% endhighlight %}

对于 `RandomState()` 函数来说，其入参可以是 `int` `array` `None` ，与 `seed()` 函数类似，其中 `RandomState()` 是线程安全的，相比来说更为复杂，可以替换 `rand()` 函数。

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
