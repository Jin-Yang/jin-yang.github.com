---
title: C++ Promise Future
layout: post
comments: true
language: chinese
category: [linux]
keywords:
description:
---

异步编程通常用在一些对性能要求较高的场景中，当下发一个请求之后，在任务完成前一般不会阻塞等待，而是切换到其它任务继续执行。

<!-- more -->

## 简介

异步编程的关键是如何进行通讯，一般有两种方式：A) 能够知道任务有没有完成，例如信号量、条件变量等；B) 在任务完成后执行一段特定的逻辑，也就是回调函数。

对于回调函数来说，如果一个任务会分成多个异步阶段完成，那么需要在每个阶段的回调函数中加入下阶段的代码，这就会导致多次循环。

Future 和 Promise 源于函数式语言，其目的是分离一个值和产生值的方法，从而简化异步代码的处理。两者相互协作完成，其中 Promise 用来承诺某段时间后可以获取对象，然后再通过 Future 来获取这个结果。

### Future

C++ 中提供的 `std::future` 简单来说是提供了一种访问异步操作结果的机制。

对于异步操作一般很难马上获取操作结果，可以同步等待或者通过 `future` 异步获取结果，通常有三种状态：A) `deferred` 未开始；B) `ready` 已完成；C) `timeout` 超时。

获取 `future` 结果有三种方式：A) `get()` 等待异步操作结束并返回结果；B) `wait()` 只等待异步操作完成，没有返回值；C) `wait_for()` 超时等待返回结果。

### 示例

通过 `promise` 设置对象，然后调用 `future` 获取上述设置的值，因为是异步，那么 `future` 可能会阻塞等待。

<!--
https://www.cnblogs.com/haippy/p/3280643.html
https://blog.csdn.net/jiange_zh/article/details/51602938
https://www.jianshu.com/p/3a00e5c2d71d
https://blog.csdn.net/AC_hell/article/details/72718363
-->


{% highlight text %}
{% endhighlight %}
