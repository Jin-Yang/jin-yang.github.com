---
title: GoLang 并发模型
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: stan
description:
---


<!-- more -->


## 其它

### Concurrency VS. Parallelism

很多的一些基本概念源自于 2012.1.11 年 Rob Pike 的相关演讲，也就是 [Concurrency is not Parallelism](https://talks.golang.org/2012/waza.slide#1) 。

一般中文会将上述翻译为 并发 (Concurrency) 和 并行 (Parallelism)，如下是对其的总结。

#### Tips #1

* Concurrency is about dealing with lots of things at once.
* Parallelism is about doing lots of things at once.

也即是说，并发是在同时处理多个任务，但是有可能真正在执行的只有一个；并行则是同时在执行多个任务。

有点类似于，单核上我们可以同时跑多个任务，但是各个任务之间是循环调度的，某个时间点实际在运行的只有一个；而在多核上，却可以明确的同时运行多个任务。

#### Tips #2

* Concurrency is about structure.
* Parallelism is about execution.

并发实际很早就有，对于 Golang 来说，最终是通过协程完成的，而类似 Linux 则是通过线程完成。

而对于并行而言，实际上需要硬件 (一般也就是 CPU) 的支持。

#### 总结

They are Not the same, but related.

简单来说，并发实际上是简化了编程的难度，包括操作系统以及 Golang 实际上都是对基本的运行单元进行了封装，对于用户来说是并发执行的。


{% highlight text %}
{% endhighlight %}
