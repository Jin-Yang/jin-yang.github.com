---
title: 图论简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

## DAG

有向无环图 Directed Acyclic Graph, DAG 是有向图的一种，常被用来表示事件之间的驱动依赖关系，管理任务之间的调度。

<!--
## 基本概念

在图中关键的是两个元素：A) Vertex/Node 顶点/节点；B) Edge 变，可以是有向或者无向。

在分类的时候，大部分是根据边的特性进行定义，例如是否有权重，是否有方向等。
-->

## 实现方式

图的存储结构分为两种：A) 邻接矩阵；B) 邻接表。

### Adjacency Matrix，邻接矩阵

其存储方式是采用两个数组来表示图，包括了：A) 一维数组存储图中顶点信息；B) 二维数组 (邻接矩阵) 存储图中的边或弧的信息。

最终这个矩阵会是一个对称矩阵。

### Adjacency List，邻接表

先通过一维数组列出所有顶点信息，然后再以链表表示所有与之相连的顶点。


<!--
图不错
http://alrightchiu.github.io/SecondRound/graph-introjian-jie.html

https://www.cnblogs.com/XMU-hcq/p/6065057.html
-->

<!--
#### 广度优先搜索 Breadth-First Search, BFS
http://alrightchiu.github.io/SecondRound/graph-breadth-first-searchbfsguang-du-you-xian-sou-xun.html

在执行 BFS 时，通过

* queue 用来辅助完成 BFS 功能，用于确定新查询的起点；
* color 标示节点 (Vertex) 是否被找到，包括了：A) 白色，未被发现；B) 灰色，已经被找到；C) 黑色，查找已经结束；
* distance 每个节点与起点的距离，那么距离 0 就是起点；
* predecessor 记录某个节点是被那个节点找到的，以便回溯路径；



#### 深度优先搜索 Depth First Search, DFS

还可以采用深度优先搜索，当遇到一个节点后则继续查找，如果节点 A 与 B 存在一条路径，那么通过 DFS 必然可以找到该路径，虽然未必是最短路径。

每个节点会保存两个时间，分别为：A) discover 发现时间，例如 A 找到 B 之后，B 的发现时间会在 A 的基础上加 1 ；B) finish 结束时间，也就是如下染为黑色时的时间。

通过颜色标示各个节点状态：A) 白色，未被发现；B) 灰色，已经发现但未结束；C) 黑色，以改节点为起点的搜索已经结束。

http://alrightchiu.github.io/SecondRound/graph-depth-first-searchdfsshen-du-you-xian-sou-xun.html

对于 DFS 来说，如果存在一条从 X 到 Y 的路径，那么 X 的结束时间一定比 Y 的结束时间大。




## 拓扑排序

在图论中，所谓的拓扑排序 (Topological Sorting) 是一个有向无环图 (Directed Acyclic Graph, DAG) 的所有顶点的线性序列，该序列需要满足下面的几个条件：

* 每个顶点出现且只出现一次。
* 如果存在一条从顶点 A 到顶点 B 的路径，那么在序列中顶点 A 出现在顶点 B 的前面。

如上就是 Topological Order 的定义，这也就意味着，拓扑排序可能会有多种，而且只要有向无环图才有拓扑排序。

### Method #1

一般被称为 Kahn's Algorithm 。

https://blog.csdn.net/lisonglisonglisong/article/details/45543451

### Method #2

使用 DFS 进行一次排序，然后

如果在 DAG 上执行一次 DFS ，当存在一条从 V(x) 到 V(y) 的路径时，那么对应的结束时间 finish(x) > finish(y) ，证明可以参考
http://www.personal.kent.edu/~rmuhamma/Algorithms/MyAlgorithms/GraphAlgor/topoSort.htm

https://saco-evaluator.org.za/presentations/camp2_2018/topsort-tian.pdf




当图各个节点相互连通的时候会有生成树，如果不连通，则会有多个生成树，组成生成森林。

## 最小生成树 Minimum Spanning Tree, MST

考虑如下的场景，某个地方有 N 个村庄，现在需要在 N 个村庄之间修路，每个村庄之前的距离不同，问如何选择最短的路径，将各个村庄连接起来。

这一场景可以简单归纳为最小生成树问题，正式的表述为：给定一个无方向的带权图，以最小代价连接所有顶点所用边的最小集合。

所谓的生成树，实际上是在图中定义一个树，该树具有如下的特性：

* 连接图中所有的节点；
* 因为最终是树，不会出现环，对于 V 个节点，只有 V - 1 个边。

最小生成树是连通无向带权图的一个子图，能够连接图中的所有顶点而且无环，生成树边的所有权重总和作为该生成树的权重，其中最小的权重就是最小生成树，同时需要注意以下几个点：

* 并未对图中的权限做唯一约束，那么就可能出现多个最小生成树；
* MST 只要求最小的权重和，并未对 Root 节点、树是否平衡、树高等做限制。

解决最小生成树最常用的是 Prime 和 Kruskal 算法，分别从点和边下手解决了该问题。
https://www.jianshu.com/p/efcd21494dff

### 基本概念

在通过算法生成最小生成树的时候，会使用一个临时的集合保存边，每次添加一个满足 MST 条件的边，最终该集合会包含所有的 MST 边。

### Kruskal's Algorithm

按照权重由小到大的顺序选择边，同时避免产生环，这样就可以找到 MST 。

### Prim's Algorithm


## 最短路径 Shortest Path

需要考虑的是一个 Weighted Directed Graph ，其中 Weight 表示的是成本，注意，权值并未限制为正数。

###################################
## 网络流 Network Flow
###################################

假设有个污水处理厂要进行排污，包括了一个进水孔以及排水孔，其排污能力与水管中的排水能力有关，而这里的需求就是找到最大流量。

上述的问题，实际上是一个比较常见的问题，解决的方案就是如下讨论的网络流。

## 网络流 Network Flow

这是一个 Weighted Directed Graph ，每个边具有非负流量，而且网络中有两个特殊的顶点：A) 源点 Source；B) 终点 Sink 。对于流网络，需要满足如下的几个特性：

1. 容量限制 Capacity Constraint，从 A 流向 B 的流量不能超过 Edge(A, B) 的容量；
2. 反对称性 Skew Symmetry，如果定义从 A 到 B 的流量为 5 个单位，那么从 B 到 A 则为 -5 个单位：
3. 流守恒性 Flow Conservation，除了源点以及终点之外，所有的流入总和应该等于流出总和。

那么，最大流问题 Maximum-Flow Problem 就可以表述为，给出流网络，希望找到从源点到汇点的最大流。

### 基本概念

首先介绍两个比较重要的概念：A) 残余网络 Residual Networks；B) 增广路径 Augmenting Paths；

#### Residual Networks, 残余网络

用来记录当前网络的边还剩余多少容量 (Residual Capacity) 可以让流流过。

c(x, y) 原来水管孔径大小；
f(x, y) 当前水管已经有多少流量；
c_f(x, y) 水管还能容纳多少流量。

残余网络记录了反向流向，可以简单理解为对已经分配流量的重新分配。

#### Augmenting Paths, 增广路径

在残余网络中，所有能够从源点到终点的路径 (也就是能够增加流量的路径) 被称为增广路径。

### Ford-Fulkerson Algorithm

是一种解决最大流的方法，如果使用 BFS 搜索路径，则又被称为 Edmonds-Karp Algorithm ，可以简述为：

https://www.cnblogs.com/biyeymyhjob/archive/2012/07/31/2615833.html
https://github.com/guruprasadnk7/DAGSimilarityKatz
https://github.com/pimpale/cDAG/blob/master/main.c
https://github.com/MauricioRoman/graph2olap
http://alrightchiu.github.io/SecondRound/shortest-pathintrojian-jie.html
https://durant35.github.io/2017/05/19/Algorithms_MaximizingFlow/
http://alrightchiu.github.io/SecondRound/flow-networksmaximum-flow-ford-fulkerson-algorithm.html
http://alrightchiu.github.io/SecondRound/mu-lu-yan-suan-fa-yu-zi-liao-jie-gou.html

https://github.com/jtorvald1/GraphCycleDetection

https://juejin.im/post/5c1bb40a6fb9a049f36211b0
https://juejin.im/post/5b8fc5536fb9a05d2d01fb11

Floyd-Warshall 全源最短路径算法
https://www.cnblogs.com/gaochundong/p/floyd_warshall_algorithm.html

一个C编写的图数据库
https://github.com/stevedekorte/vertexdb

通过Redis存储的图数据库
https://github.com/RedisGraph/RedisGraph


#### Augmenting Paths, 增广路径

在残余网络中，所有能够从源点到终点的路径 (也就是能够增加流量的路径) 被称为增广路径。

https://www.cnblogs.com/biyeymyhjob/archive/2012/07/31/2615833.html
https://github.com/guruprasadnk7/DAGSimilarityKatz
https://github.com/pimpale/cDAG/blob/master/main.c
https://github.com/MauricioRoman/graph2olap
http://alrightchiu.github.io/SecondRound/shortest-pathintrojian-jie.html
https://durant35.github.io/2017/05/19/Algorithms_MaximizingFlow/
http://alrightchiu.github.io/SecondRound/flow-networksmaximum-flow-ford-fulkerson-algorithm.html
http://alrightchiu.github.io/SecondRound/mu-lu-yan-suan-fa-yu-zi-liao-jie-gou.html

https://github.com/jtorvald1/GraphCycleDetection

https://juejin.im/post/5c1bb40a6fb9a049f36211b0
https://juejin.im/post/5b8fc5536fb9a05d2d01fb11
-->


{% highlight text %}
{% endhighlight %}
