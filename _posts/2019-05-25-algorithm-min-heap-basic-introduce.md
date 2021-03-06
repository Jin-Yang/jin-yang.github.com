---
title: 最小堆简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

## 简介

堆 (heap) 又被为优先队列 (priority queue)，通过堆可以按照元素的优先级取出元素，而不是按照元素进入队列的先后顺序取出元素。

堆的一个经典的实现是完全二叉树 (Complete Binary Tree)，这样实现的堆成为二叉堆 (Binary Heap)，二叉树要求前 `n-1` 层必须填满，第 `n` 层也必须按照从左到右的顺序被填满，比下图:

![complete binary tree]({{ site.url }}/images/structure/complete_binary_tree.jpg "complete binary tree"){: .pull-center }

为了实现优先级，还需要要求任意节点的优先级不小于 (或不大于) 它的子节点。

### 实现

在实现时通过数组完成，对于 n 叉堆来说，下标为 x 的元素，其孩子节点的下标范围是 `[nx+1, nx+n]`，比如 2 叉堆，下标为 x 的元素，其孩子节点的下标为 `2x+1` 和 `2x+2` 。

如下简单以二叉堆作为示例，有如下特点 (中括号假设开始为 1 ，小括号的开始为 0 )：

* 堆的根节点存放在数组位置 `array[1] (0)` 的地方；
* 父节点 `i` 的左子节点在位置 `array[2*i] (2*i+1)`；
* 父节点 `i` 的右子节点在位置 `array[2*i+1] (2*i+2)`；
* 子节点 `i` 的父节点在位置 `array[floor(i/2)] (floor((i-1)/2))`。
* 最多非子节点数为 `floor(i/2) floor((i - 1)/2)` 。

上述的 `floor()` 表示取整，例如 `5/2=2` 。在实现时，可以将 `0` 作为临时存储，那么第一个元素就可以 `1` 开始。

{% highlight text %}
                  0                                       1
          /               \                        /              \
         1                 2                      2                3
      /     \           /     \               /      \          /      \
     3       4        5        6             4        5        6         7
   /   \   /   \    /   \    /   \         /   \    /   \    /    \    /    \
  7    8   9   10  11   12  13   14       8    9   10   11  12    13  14    15
 
             C 语言格式                                正常格式
{% endhighlight %}

在实现时，包括了两个主要操作：A) UpHeap 在添加的时候；B) DownHeap 删除节点的时候。

当删除节点的数值时，原来的位置就会出现一个孔，填充这个孔的方法是把最后的叶子的值赋给该孔，并下调到合适位置，最后把该叶子删除。


<!--
所以，对于Libev中的4叉堆实现而言，下标为k的元素（对应在正常实现中的下标是k-3），其孩子节点的下标范围是[4(k-3)+1+3, 4(k-3)+4+3]；其父节点的下标是((k-3-1)/4)+3。

对于Libev中的2叉堆实现而言，下标为k的元素（对应在正常实现中，其下标是k-1），其孩子节点的下标范围是[2(k-1)+1+1,  2(k-1)+2+1]，也就是[2k, 2k+1]；其父节点的下标是((k-1-1)/2)+1，也就是k/2。
-->




{% highlight text %}
{% endhighlight %}
