---
title: Python AI 环境准备
layout: post
comments: true
language: chinese
category: [linux,python]
keywords: linux,numpy
description: 目前大部分的 AI 相关工具包、框架等都是通过 Python 实现的，包括了常用的 Numpy、Scikit-Learn、TensorFlow、PyTorch 等等。所以，这里搭建的环境主要也就是 Python 相关的。
---

目前大部分的 AI 相关工具包、框架等都是通过 Python 实现的，包括了常用的 Numpy、Scikit-Learn、TensorFlow、PyTorch 等等。

所以，这里搭建的环境主要也就是 Python 相关的，另外还有基本的 MNIST 数据集。

<!-- more -->

## Python 环境

这里主要是最基本的依赖包。

### 常用包

在 Python 的数据挖掘或者 ML 中，经常遇到如下的几种包。

#### Numpy

Python 没有提供数组，一般使用列表 (List) 代替使用，不过当数据量增加时会明显的变慢。Numpy 提供了数组支持，可以有效的提高处理速度，核心部分通过 C/C++ 实现，同时很多高级扩展包依赖它。

#### Scipy

提供矩阵支持，以及矩阵相关的数值计算模块。

#### Pandas

这个是 Panel Data 的简写，提供了强大的数据分析和探索工具，因金融数据分析工具而开发，支持类似 SQL 的数据增删改查，支持时间序列分析，灵活处理缺失数据。

#### Scikit-Learn

用于数据挖掘和数据分析的简单且有效的工具，它的基本功能主要被分为六个部分：分类(Classification)、回归(Regression)、聚类(Clustering)、数据降维(Dimensionality Reduction)、模型选择(Model Selection)、数据预处理(Preprocessing)。

#### Matplotlib

主要用于绘图和绘表，强大的数据可视化工具，另外，Seaborn 也是数据可视化的工具包。


### 安装

为了防止一些包冲突，可以通过 virtualenv 创建一个临时的环境。

{% highlight text %}
----- 生成临时环境
$ mkdir -p ~/Workspace/tensorflow && cd ~/Workspace
$ virtualenv --no-site-packages tensorflow
$ source tensorflow/bin/activate

----- 安装TensorFlow
$ pip install --upgrade tensorflow

----- 安装Numpy
$ pip install --upgrade numpy
{% endhighlight %}

然后通过如下代码进行测试。

{% highlight text %}
$ python
>>> import tensorflow as tf
>>> hello = tf.constant("Hello TensorFlow")
>>> sess = tf.Session()
>>> print sess.run(hello)
Hello TensorFlow
>>> a = tf.constant(10)
>>> b = tf.constant(32)
>>> print sess.run(a + b)
42
{% endhighlight %}

对于 Numpy 源码可以直接从 [www.numpy.org](http://www.numpy.org/) 上下载，如果是离线则下载完包之后直接通过如下命令安装。

{% highlight text %}
$ python setup.py build
$ python setup.py install
{% endhighlight %}

## MNIST

MNIST 数据集来自 National Institute of Standards and Technology, NIST 美国国家标准与技术研究所，其中的训练集由来自 250 个不同人手写的数字构成，其中 50% 是高中学生， 50% 来自人口普查局的工作人员。

很多教程都是通过该数据集开始。

### 下载

MNIST 数据集可在 [yann.lecun.com](http://yann.lecun.com/exdb/mnist/) 获取, 它包含了四个部分:

{% highlight text %}
Training set images: train-images-idx3-ubyte.gz (9.9 MB, 解压后 47 MB, 包含 60,000 个样本)
Training set labels: train-labels-idx1-ubyte.gz (29 KB, 解压后 60 KB, 包含 60,000 个标签)
Test set images:     t10k-images-idx3-ubyte.gz  (1.6 MB, 解压后 7.8 MB, 包含 10,000 个样本)
Test set labels:     t10k-labels-idx1-ubyte.gz  (5KB, 解压后 10 KB, 包含 10,000 个标签)
{% endhighlight %}

然后通过 `gunzip FILE.gz` 解压文件即可。

### 格式转换

图片是以字节的形式进行存储, 可以将其读取到 Numpy 的 array 中, 以便训练和测试算法。

{% highlight python %}
import os
import struct
import numpy as np

def load_mnist(path, kind='train'):
    """Load MNIST data from `path`"""
    labels_path = os.path.join(path, '%s-labels-idx1-ubyte' % kind)
    with open(labels_path, 'rb') as lbpath:
        magic, n = struct.unpack('>II', lbpath.read(8))
        labels = np.fromfile(lbpath, dtype=np.uint8)

    images_path = os.path.join(path, '%s-images-idx3-ubyte' % kind)
    with open(images_path, 'rb') as imgpath:
        magic, num, rows, cols = struct.unpack('>IIII', imgpath.read(16))
        images = np.fromfile(imgpath, dtype=np.uint8).reshape(len(labels), 784)

    return images, labels
{% endhighlight %}

在 MNIST 数据集中的每张图片由 `28x28` 个像素点构成, 每个像素点用一个灰度值表示，在这里, 我们将 28 x 28 的像素展开为一个一维的行向量, 这些行向量就是图片数组里的行(每行 784 个值, 或者说每行就是代表了一张图片).

`load_mnist()` 返回两个数组, 第一个是一个 n x m 维的 NumPy array(images), 这里的 n 是样本数(行数), m 是特征数(列数). 训练数据集包含 60,000 个样本, 测试数据集包含 10,000 样本.


第二个数组 `labels` 包含了相应的目标变量, 也就是手写数字的类标签 (0-9)。

<!--
https://github.com/GoogleCloudPlatform/tensorflow-without-a-phd
https://github.com/yhlleo/mnist
https://github.com/zalandoresearch/fashion-mnist


https://blog.csdn.net/simple_the_best/article/details/75267863
-->

在 MNIST 网站中，有对数据集的描述，如下：

{% highlight text %}
TRAINING SET LABEL FILE (train-labels-idx1-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000801(2049) magic number (MSB first)
0004     32 bit integer  60000            number of items
0008     unsigned byte   ??               label
0009     unsigned byte   ??               label
........
xxxx     unsigned byte   ??               label
The labels values are 0 to 9.12345678910
{% endhighlight %}

然后通过如下的代码，首先读入 magic number 以及 label 的数目；然后通过调用 `fromfile()` 将字节读入 Numpy array 。其中 `struct.unpack` 的 `>II` 有两个部分：`>` 表示大端存储，`II` 表示两个无符号整数。

{% highlight text %}
magic, n = struct.unpack('>II', lbpath.read(8))
labels = np.fromfile(lbpath, dtype=np.uint8)12
{% endhighlight %}


<!--
通过执行下面的代码, 我们将会从刚刚解压 MNIST 数据集后的 mnist 目录下加载 60,000 个训练样本和 10,000 个测试样本.

为了了解 MNIST 中的图片看起来到底是个啥, 让我们来对它们进行可视化处理. 从 feature  matrix 中将 784-像素值 的向量 reshape 为之前的 28*28 的形状, 然后通过 matplotlib 的 imshow 函数进行绘制:

import matplotlib.pyplot as plt

fig, ax = plt.subplots(
    nrows=2,
    ncols=5,
    sharex=True,
    sharey=True, )

ax = ax.flatten()
for i in range(10):
    img = X_train[y_train == i][0].reshape(28, 28)
    ax[i].imshow(img, cmap='Greys', interpolation='nearest')

ax[0].set_xticks([])
ax[0].set_yticks([])
plt.tight_layout()
plt.show()1234567891011121314151617

我们现在应该可以看到一个 2*5 的图片, 里面分别是 0-9 单个数字的图片.



此外, 我们还可以绘制某一数字的多个样本图片, 来看一下这些手写样本到底有多不同:



fig, ax = plt.subplots(
    nrows=5,
    ncols=5,
    sharex=True,
    sharey=True, )

ax = ax.flatten()
for i in range(25):
    img = X_train[y_train == 7][i].reshape(28, 28)
    ax[i].imshow(img, cmap='Greys', interpolation='nearest')

ax[0].set_xticks([])
ax[0].set_yticks([])
plt.tight_layout()
plt.show()123456789101112131415

执行上面的代码后, 我们应该看到数字 7 的 25 个不同形态:

另外, 我们也可以选择将 MNIST 图片数据和标签保存为 CSV 文件, 这样就可以在不支持特殊的字节格式的程序中打开数据集. 但是, 有一点要说明, CSV 的文件格式将会占用更多的磁盘空间, 如下所示:

train_img.csv: 109.5 MB
train_labels.csv: 120 KB
test_img.csv: 18.3 MB
test_labels: 20 KB


如果我们打算保存这些 CSV 文件, 在将 MNIST 数据集加载入 NumPy array 以后, 我们应该执行下列代码:

np.savetxt('train_img.csv', X_train,
           fmt='%i', delimiter=',')
np.savetxt('train_labels.csv', y_train,
           fmt='%i', delimiter=',')
np.savetxt('test_img.csv', X_test,
           fmt='%i', delimiter=',')
np.savetxt('test_labels.csv', y_test,
           fmt='%i', delimiter=',')12345678

一旦将数据集保存为 CSV 文件, 我们也可以用 NumPy 的 genfromtxt 函数重新将它们加载入程序中:

X_train = np.genfromtxt('train_img.csv',
                        dtype=int, delimiter=',')
y_train = np.genfromtxt('train_labels.csv',
                        dtype=int, delimiter=',')
X_test = np.genfromtxt('test_img.csv',
                       dtype=int, delimiter=',')
y_test = np.genfromtxt('test_labels.csv',
                       dtype=int, delimiter=',')12345678

不过, 从 CSV 文件中加载 MNIST 数据将会显著发给更长的时间, 因此如果可能的话, 还是建议你维持数据集原有的字节格式.
-->










## 测试数据集

测试用的数据集，通常用来学习或者模拟算法的数据库，有很多不错的模型，详细可以参考 [Dataset loading utilities](https://scikit-learn.org/stable/datasets/index.html) 。

### make_blobs

<!--
https://scikit-learn.org/stable/modules/generated/sklearn.datasets.make_blobs.html
-->

用于生成具有高斯分布的 blobs 点，可以控制生成 blobs 的数量，生成样本的数量以及一系列其它属性。考虑到 blobs 的线性可分性质，适用于线性分类问题。

{% highlight text %}
sklearn.datasets.make_blobs(n_samples=100, n_features=2, centers=None, cluster_std=1.0,
		center_box=(-10.0, 10.0), shuffle=True, random_state=None)

其中参数如下：
   n_samples 待生成的样本的总数。
   n_features 每个样本的特征数，会保存在返回的对象中。
   centers 表示类别数。
   cluster_std 表示每个类别的方差。
{% endhighlight %}

例如生成 2 类数据，其中一类比另一类具有更大的方差，可以将 `cluster_std` 设置为 `[1.0, 3.0]` ，如下是生成三类数据用于聚类 (100 个样本，每个样本有 2 个特征)：

{% highlight python %}
import matplotlib.pyplot as plt
from sklearn.datasets import make_blobs

data, target = make_blobs(
	n_samples=100,             # 生成的总样本数，注意是总数
	n_features=2,              # 样本的特征数，也就是生成数据的维度，默认是2
	centers=3,                 # 总的类别，需要与如下的方差保持一致，默认是5
	cluster_std=[1.0,2.0,3.0]) # 每个类别的方差，默认是1

# 在2D图中绘制样本的散点图，每个样本颜色不同
plt.scatter(data[:,0], data[:,1], c=target);
plt.show()
{% endhighlight %}

其中 `scatter()` 函数中的 c 参数标示了分类。

#### centers

也可以根据 `n_features` 设置不同聚类的中心坐标位置，例如，当 `n_features=2` 时，可以设置 `centers=[[0, 0], [2, 2]]` ，此时会出现两个分类，其中心坐标分别是 `(0, 0)` 和 `(2, 2)` 。

### make_moons

用于二进制分类并且将生成一个漩涡模式，可以控制 moon 形状中的噪声量，以及要生产的样本数量，适用于能够学习非线性类边界的算法。

{% highlight python %}
import matplotlib.pyplot as plt
from sklearn.datasets import make_moons

data, target = make_moons(
	n_samples=500,     # 生成的样本总数
	noise=0.05,        # 噪声的大小
	random_state=1)    # 随机状态

plt.scatter(data[:,0], data[:,1], c=target);
plt.show()
{% endhighlight %}

其中 `random_state` 的作用可以参考后面的介绍，简单来说，默认每次都会生成一个随机值，如果设置为固定值，那么每次生成的结果集都是固定的。

### make_circles

用于生成一个数据集落入同心圆的二进制分类问题，适用于可以学习复杂的非线性流行的算法。

示例可以直接将上述 `make_moons` 替换为 `make_circles` 即可。

## 其它

### random_state

实际上，这个是一个生成随机数的种子，在生成的测试数据集中可以用来生成不同的测试数据。

因为 sklearn 中很多算法都含有随机的因素，为了进行可重复的训练，那么可以固定一个 random_state ，然后对模型进行调参。


{% highlight text %}
{% endhighlight %}
