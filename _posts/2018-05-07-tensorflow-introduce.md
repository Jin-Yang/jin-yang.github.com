---
title: TensorFlow 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,tensorflow
description:
---


<!-- more -->

## 其它

### 常见告警处理

#### CPU instructions was not compiled

在第一次调用 `Session()` 时会报错，完整的报错是 `Your CPU supports instructions that this TensorFlow binary was not compiled to use: AVX2 FMA` ，

<!--
除了通常的算术和逻辑，现代CPU提供了许多低级指令，称为扩展，例如， SSE2，SSE4，AVX等来自维基百科：

    高级矢量扩展（AVX）是英特尔在2008年3月提出的英特尔和AMD微处理器的x86指令集体系结构的扩展，英特尔首先通过Sandy Bridge处理器在2011年第一季度推出，随后由AMD推出Bulldozer处理器在2011年第三季度.AVX提供了新功能，新指令和新编码方案。
    特别是，AVX引入了融合乘法累加（FMA）操作，加速了线性代数计算，即点积，矩阵乘法，卷积等。几乎所有机器学习训练都涉及大量这些操作，因此将会支持AVX和FMA的CPU（最高达300％）更快。该警告指出您的CPU确实支持AVX（hooray！）。
-->

由于 tensorflow 默认分布是在没有 CPU 扩展的情况下构建的，例如 SSE4.1、SSE4.2、AVX、AVX2、FMA 等，默认版本 (通过 `pip install tensorflow` 安装的版本) 旨在与尽可能多的 CPU 兼容。

为次，如果不需要关心 AVX 的支持，可以简单地忽略此警告：

{% highlight text %}
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
{% endhighlight %}

也可以重新编译 TensorFlow 源码，也就是用 Tensorflow 称为 bazel 的 ad-hoc 构建系统，构建会比较复杂，但是可以针对具体的 CPU 进行优化，编译之后，不仅警告消失，tensorflow 性能也应该有所改善。

## 参考

官方文档 [www.tensorflow.org/tutorials](https://www.tensorflow.org/tutorials/) 。

[Tensorflow Without A PHD](https://github.com/GoogleCloudPlatform/tensorflow-without-a-phd) 总共有六篇课程，Google 提供的官方课程，包含了视频(youtube)、代码、教程等。

[MNIST classification by TensorFlow](https://github.com/sugyan/tensorflow-mnist) 一个提供了前端展示的示例程序。


{% highlight text %}
{% endhighlight %}
