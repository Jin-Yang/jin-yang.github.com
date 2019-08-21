---
title: Python Matplotlib 简介
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: python,matplotlib
description: 简单来说，Matplotlib 是 Python 中的一个绘图库，包含了大量的工具，几乎可以通过该工具完成你所需要的任何图形，包括散点图、正弦曲线，甚至是三维图形。这一工具经常用在数据可视化中，这里简单介绍其使用方法。
---

简单来说，Matplotlib 是 Python 中的一个绘图库，包含了大量的工具，几乎可以通过该工具完成你所需要的任何图形，包括散点图、正弦曲线，甚至是三维图形。

这一工具经常用在数据可视化中，这里简单介绍其使用方法。

<!-- more -->

## 简介

在正式使用之前，强烈建议先查看下官方的 [示例库](https://matplotlib.org/gallery.html) ，包含了样例以及对应的代码，几乎包含了所能想到的图形。

## 常用函数

{% highlight text %}
plt.figure()       定义一个图像
    figsize=(3, 1) 图片的宽、高设置，单位为英寸(1英寸=2.54厘米)
plt.plot()         绘制图形
plt.axhline()      绘制水平线
plt.imshow()       根据像素绘制图片

plt.legend()       显示图例
plt.title()        标题
plt.xlabel()       X轴标记
plt.ylabel()       Y轴标记

{% endhighlight %}

### subplot

{% highlight text %}
subplot(nrows, ncols, index, **kwargs)
{% endhighlight %}

用来将一个图分割成多个，前三个参数分别用来指定行、列、当前使用序号，

{% highlight text %}
subplot(2, 2, 1)  # 创建两行两列，并使用第一个(第一行+第一列)
subplot(221)      # 等价于上面的函数，是简写
{% endhighlight %}

如果在一个图中绘制出的图片比较混乱，可以将图片分割成多个，通过 `subplot()` 进行分割，并标示使用序号 (从1开始向右) 。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0.0, 5.0, num=100)
y1 = np.sin(np.pi * x)
y2 = np.cos(np.pi * x)
y3 = np.cos(np.pi * x * 2)

#plt.plot(x, y1, 'b--', label='$sin(\pi * x)$')
#plt.plot(x, y2, 'r--', label='$cos(\pi * x)$')
#plt.plot(x, y3, 'g--', label='$cos(\pi * x * 2)$')

plt.subplot(211)
plt.plot(x, y3, 'g--', label='$cos(\pi * x * 2)$')
plt.legend(loc='lower right')

plt.subplot(223)
plt.plot(x, y1, 'b--', label='$sin(\pi * x)$')
plt.legend(loc='lower right')

plt.subplot(224)
plt.plot(x, y2, 'r--', label='$cos(\pi * x)$')
plt.legend(loc='lower right')
plt.show()
{% endhighlight %}

上述的绘图实际上是在默认的窗口中，也可以通过 `fig = plt.figure()` 新建一个窗口，然后通过 `ax = fig.add_subplot(221)` 进行添加。

### imshow

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

p = np.linspace(-5, 5, num=1000)
xs, ys = np.meshgrid(p, p)
z = np.sqrt(xs ** 2 + ys ** 2)

# 可以选择cool hot
plt.imshow(z, cmap=plt.cm.gray)
plt.show()
{% endhighlight %}

可以坐标为 `0~1000`，也就是传入的 z 矩阵对应的是 `1000x1000` ，z 的索引是图像的坐标，而其值是通过图的颜色显示出来的。

另外，`imshow()` 函数还提供了图形插值方式，对于没有的数据可以通过插值的方式进行填充。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

data = np.add.outer(range(8), range(8)) % 2
plt.imshow(data, cmap=plt.cm.gray, interpolation='nearest', origin="lower")
plt.show()
{% endhighlight %}

其它的插值算法还有 `bilinear` `bicubic` 等。

## 动画

最简单的，可以通过如下方式生成动画，但是暂时不太确定如何进行保存。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0, 20, 500)

plt.figure(figsize=(8, 4))
for i in range(100):
    plt.clf()
    y = np.sin(x + np.pi * 0.1 * i)
    plt.plot(x, y, color='blue', linewidth=1)
    plt.pause(0.05)
{% endhighlight %}

另外，也可以使用 Matplotlib 库中的 animation 模块，如下是一个简单的示例。

{% highlight python %}
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig, ax = plt.subplots()
x = np.arange(0, 2*np.pi, 0.01)
line, = ax.plot(x, np.sin(x))

def init():
    line.set_ydata([np.nan] * len(x))
    return line,

def animate(i):
    line.set_ydata(np.sin(x + i / 100))  # update the data.
    return line,

ani = animation.FuncAnimation(fig, animate, init_func=init,
                              interval=2, blit=True, save_count=50)

# To save the animation, use e.g.
# ani.save("movie.mp4")
# ani.save('animation.gif', writer='pillow')
# ani.save('animation.html', writer='html')
# or
# from matplotlib.animation import FFMpegWriter
# writer = FFMpegWriter(fps=15, metadata=dict(artist='Me'), bitrate=1800)
# ani.save("movie.mp4", writer=writer)
plt.show()
{% endhighlight %}

<!--
animation.FuncAnimation()

fig        用来绘制图像的对象；
func       每个帧会调用的函数；
init_func  可选初始化函数；
interval   帧间隔时间，单位是毫秒；
frames     指定更新序列，可以是整数、函数、循环对象，如果是整数，则指定的是总的帧数；
save_count 保存动画的帧数；
repeat     是否循环动画；
blit       是否优化绘图。

https://matplotlib.org/3.1.1/api/_as_gen/matplotlib.animation.FuncAnimation.html
-->

## 参数

### rcParams

通过该参数可以设置图片大小、颜色、像素、分辨率等参数信息。

详细可以参考 [The top level matplotlib module](https://matplotlib.org/api/matplotlib_configuration_api.html#matplotlib.RcParams) 中的介绍。

## FAQ

### 常见报错

#### ImportError: No module named _tkinter

默认 Python 安装包没有提供这个库，需要安装 `tkinter` 包，直接 `yum install tkinter` 即可，据说源码编译时，需要确保添加了 `--enable-unicode=ucs2` 参数。

另外，还需要通过安装 `python-tools` 或者 `python3X-tools` 。

#### Found existing installation: pyparsing 1.5.6

安装 `matplotlib` 包时，会依赖上述的包，主要是版本问题，可以从 [pypi.org/pyparsing](https://pypi.org/project/pyparsing/) 上查看最新的包，例如是 `2.2.0` ，那么可以通过如下方式安装。

{% highlight text %}
# pip install -I pyparsing==2.2.0
{% endhighlight %}

#### UserWarning: Matplotlib is currently using agg...

完整的报错为 `UserWarning: Matplotlib is currently using agg, which is a non-GUI backend, so cannot show the figure.` 。

主要是此时默认的后端是 `agg` 导致，可以通过如下命令确认。

{% highlight text %}
>>> import matplotlib
>>> matplotlib.get_backend()
'agg'
{% endhighlight %}

最简单的，将 `plg.show()` 替换为 `fig.savefig()` ，也就是直接保存为图片，而非通过窗口显示。

另外，也可以通过 `matplotlib.use('TkAgg')` 设置后端。



<!---
http://codingpy.com/article/a-quick-intro-to-matplotlib/
http://python.jobbole.com/87831/
-->



{% highlight text %}
{% endhighlight %}
