---
title: PID 控制介绍
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,automatic
description:
---

<!-- more -->

![PID]({{ site.url }}/images/automatic/pid-control-schema.png "PID"){: .pull-center width="80%" }


## PID

Proportional Integral Derivative, PID

其中有些相关的简称 Desired SetPoint, SP；measured process variable, PV

PID 部分分别被称为 Term ，例如 proportional integral and derivative terms 。

一个很不错的网站，很多与PID相关的介绍
http://brettbeauregard.com/blog/
Arduino提供的一个PID实现，以及详细的介绍
http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

对于 PID 参数，可以类似如下的方式简单理解：A) P 是对现在情况的处理；I 是包含了过去的历史信息；D 是对未来的预测。

* P 一般权重最大，也就是可以对当前的数据做出快速响应，一般可以是 PI 或者 PD，但是极少出现 ID 的情况。
* I 是对过去的记忆，如果过去一段时间的曲线太高，就会自动将曲线整体下移，反之亦然。
* D 是对未来趋势的一种预测，通过微分可以预测其变化的速度，然后适当调整。


简单来说，会通过调整输入使得输出等于期望的输出 (SetPoint) 。

详见 PIDV1.py 中的实现。

### Sample Time

第一版的方案中，实际上对应的采集的时间间隔可能会不同的，那么就会导致如下问题：

1. 由于采集间隔一直变化，那么每次PID的行为不同，不方便进行调参。
2. 每次都需要对积分和微分进行额外的计算。

这样带来的好处包括了：

1. 无论 Compute 接口被调用了多少次，其进行 PID 计算时都是以固定的周期执行；
2. 在真正执行的时候可以将原有的乘法和除法在设置采样时间时直接替换掉。

详见 PIDV2.py 中的实现。

### Derivative Kick

每次重新调整 SetPoint，那么就会导致 error 变大，对于微分部分而言，再除以采集间隔，那么在 PID 中的微分部分会变得非常大。

这样调整之后，PID 调节过程只对输入敏感，在修改 SetPoint 时不会再有剧烈的变动，当然，真实的系统在 Setpoint 变化时也会做响应的调整，只是调整的幅度不再像之前那么大了。

详见 PIDV3.py 中的实现。

### Tuning Changes

在程序运行时动态调整 PID 的参数信息，对于上述的实现，如果直接修改参数，由于积分的累加作用，那么会导致系统波动。

此时可以将积分中的累加拆开，这在数学关系上是等价，但是对于一个真正的系统作用却很大。

详见 PIDV4.py 中的实现。

## 参考

<!--
https://github.com/ivmech/ivPID
-->

基于 Python 的控制系统仿真，文档可以参考 [Python Control Systems Library](https://python-control.readthedocs.io) ，以及 [GitHub Repos](https://github.com/python-control/python-control) 。

<!--
https://blog.csdn.net/ouening/article/details/53074839
-->

{% highlight text %}
{% endhighlight %}
