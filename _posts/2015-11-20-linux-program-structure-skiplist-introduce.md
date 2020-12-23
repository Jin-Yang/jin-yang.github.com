---
title: 数据结构 -- SkipList
layout: post
comments: true
language: chinese
category: [program]
keywords: 数据结构,skiplist
description: 是由 William Pugh 于 1990 年发表了 Skip lists a probabilistic alternative to balanced trees ，也就是设计初衷是作为替换平衡树的一种选择。这是一种随机化数据结构，基于并联的链表，其效率可比拟二叉查找树。
---

William Pugh 于 1990 年发表了 `Skip lists: a probabilistic alternative to balanced trees` 论文，也就是设计初衷是作为替换平衡树的一种选择，这是一种随机化数据结构，基于并联的链表，其效率可比拟二叉查找树。

同时，可以支持排序。

<!-- more -->

## 简介

对于有序链表，查找的时间复杂度为 `O(n)`，尽管真正的插入与删除操作节点复杂度只有 `O(1)`，但都需要先查找到节点的位置，从而降低了有序链表的性能。

而 SkipList 采用 "空间换时间" 的策略，除了原始链表外还保存一些 "跳跃" 的链表，从而可以达到加速查找的效果。

### 链表优化

对于链表来说，如果通过指针顺序查找，就需要忍受 `O(n)` 的效率；如果采用数组实现，可以通过二分查找优化到 `O(lgn)` ，主要是因为二分查找需要用到中间位置的节点，而链表不能随机访问。

如果链表在保存原始数据的同时，保存中间节点的位置，那么就可以获取到高性能的查找方式。简单来说，其处理思想类似如下方式：

* 结合了链表和二分查找的思想；
* 将原始链表和一些通过 "跳跃" 生成的链表组成层；
* 第 0 层是原始链表，越上层 "跳跃" 的步距越大，对应链表元素也越少；
* 上层链表是下层链表的子序列，在查找时从顶层向下，不断缩小搜索范围。

其实现如下。

![skiplist]({{ site.url }}/images/structure/skiplist.png "skiplist"){: .pull-center width="80%" }

主要由如下的几部分组成：

* 表头，负责维护跳表的节点指针；
* 节点，包括了元素值以及多个层的指针；
* 层，保存了指向其它元素的指针，用来从上层依次查找；
* 表尾，通过 `NULL` 组成，标识跳跃表结束。

SkipList 每层的数量不会严格按照 `2:1` 的比例，而是对每个要插入的元素随机一个层数。
随机层数的计算过程如下：

每个节点都有第一层
那么它有第二层的概率是p，有第三层的概率是p*p
不能超过最大层数


在 Redis 中的实现如下。

{% highlight c %}
/* Returns a random level for the new skiplist node we are going to create.
 * The return value of this function is between 1 and ZSKIPLIST_MAXLEVEL
 * (both inclusive), with a powerlaw-alike distribution where higher
 * levels are less likely to be returned. */
int zslRandomLevel(void) {
    int level = 1;
    while ((random()&0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
        level += 1;
    return (level<ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}
{% endhighlight %}

其中 `ZSKIPLIST_P` 的值是 `0.25`，也就是说存在上一层的概率是 `1/4` 。


## 其它

### SkipList VS. Tree

对于 AVL 树来说，有着严格的 `O(logN)` 的查询效率，但是由于插入过程中可能需要多次旋转，导致插入效率较低，因而才有了在工程界更加实用的红黑树。

但红黑树有个问题就是在并发环境下使用不方便，在更新数据时，红黑树有个平衡的过程，在这个过程中会涉及到较多的节点，需要锁住更多的节点，从而降低了并发性能。

另外，SkipList 的实现要简单很多，目前在 Redis、BigTable 中有使用。

## Rank

一般在游戏中会有一个排名，通过跳表可以很容易实现相关的功能，在计算排名次序时，与在跳表中查找的时间复杂度是一样的，仍然是 `O(lgN)`，而对于二叉树，貌似查找排名只能按照顺序遍历的方式来统计。

<!--
游戏排行榜 – 基于skiplist计算rank排名
https://yuerblog.cc/2019/02/13/skiplist-rank/
-->

<!--
### 插入

标准的链表在插入时需要先找到前驱点，然后再将节点插入到链表中，而 SkipList 则需要确认每个层的链表。

首先，需要生成一个随机数 (小于 MAX_LEVEL)，作为新节点的层数，并将新节点插入到 0~k-1 层的链表中。

https://github.com/begeekmyfriend/skiplist

## 随机数

SkipList 是一种概率算法，非常依赖于生成的随机数，不能使用平均分布，需要满足 `p=0.5` 的几何分布，基本可以满足从顶向下达到二分查找的效果。

另外，SkipList 还支持 Rank 的功能，也就是查找到某个元素之后，确认该元素排在第几位。

https://www.jianshu.com/p/fcd18946994e
https://juejin.im/post/5cb885a8f265da03973aa8a1



https://redisbook.readthedocs.io/en/latest/internal-datastruct/skiplist.html

SkipList
https://yuerblog.cc/2019/02/13/skiplist-rank/
http://zhangtielei.com/posts/blog-redis-skiplist.html

一个基于SkipList的KV存储
https://github.com/Softmotions/iowow

走近源码：Redis跳跃列表究竟怎么跳
https://juejin.im/post/5cb885a8f265da03973aa8a1
-->


{% highlight text %}
{% endhighlight %}
