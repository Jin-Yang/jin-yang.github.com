---
title: 【专题】常用算法
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---


<!-- more -->

## 排序算法

最基础的算法。

### 稳定性

假设有以下的数，使用第一个数字进行排序。

{% highlight text %}
(4, 1)  (3, 1)  (3, 7)  (5, 6)
{% endhighlight %}

在这个状况下，有可能产生两种不同的结果，一个是依照相等的键值维持相对的次序，而另外一个则没有，也就是结果为。

{% highlight text %}
(3, 1)  (3, 7)  (4, 1)  (5, 6)   (稳定)
(3, 7)  (3, 1)  (4, 1)  (5, 6)   (不稳定)
{% endhighlight %}

不稳定排序算法可能会在相等的键值中改变纪录的相对次序，但是稳定排序算法从来不会如此。不稳定排序算法可以被特别地实现为稳定。作这件事情的一个方式是人工扩充键值的比较，如此在其他方面相同键值的两个对象间之比较，（比如上面的比较中加入第二个标准：第二个键值的大小）就会被决定使用在原先数据次序中的条目，当作一个同分决赛。然而，要记住这种次序通常牵涉到额外的空间负担。

<!--
原地排序(in-place)<br><br>
原地排序就是指不申请多余的空间来进行的排序，就是在原来的排序数据中比较和交换的排序。</li><br><li>

比较排序测试<br><br>
如果想要进行测试只需要输入如下命令
-->

## 树

* [基本概念](/post/algorithm-structure-trees-introduce.html)

## 图论

![graph theroy]({{ site.url }}/images/theme-graph-theory.png "graph theory"){: .pull-center width="25%"}

* [基本概念](/post/algorithm-graph-theroy-basic-introduce.html)

## 其它

* [背包问题](/post/algorithm-knapsack-problem-introduce.html)

{% highlight text %}
{% endhighlight %}
