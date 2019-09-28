---
title: Huffman 编码简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: huffman
description: 哈夫曼编码 Huffman Coding 是一种变长的前缀码，使用的算法是 David A. Huffman 还在 MIT 的学生时提出的，并且在 1952 年发表了名为 `A Method for the Construction of Minimum-Redundancy Codes` 的文章。 编码过程叫做哈夫曼编码，是一种普遍的熵编码技术，包括用于无损数据压缩领域。
---

哈夫曼编码 Huffman Coding 是一种变长的前缀码，使用的算法是 David A. Huffman 还在 MIT 的学生时提出的，并且在 1952 年发表了名为 `A Method for the Construction of Minimum-Redundancy Codes` 的文章。

编码过程叫做哈夫曼编码，是一种普遍的熵编码技术，包括用于无损数据压缩领域。

<!-- more -->

## 简介

哈夫曼编码使用一种特别的方法为信号源中的每个符号设定二进制码，出现频率更大的符号将获得更短的比特，出现频率更小的符号将被分配更长的比特，以此来提高数据压缩率，提高传输效率。

开始编码时，首先需要扫描数据源获取每个符号出现的频率，然后根据字符频度构建哈夫曼树，并以此生成哈夫曼编码。压缩时，再次扫描数据源，根据编码表将编码存入压缩文件，同时存入编码表。解压时，读取编码表，然后读取编码匹配编码表找到对应字符，存入文件，完成解压。

### 哈夫曼树

将 N 个权值，作为叶结点，并构造一棵二叉树，而这棵树的特点是，对于 N 个叶节点，叶节点的值为给定的权值，而内部节点的值为子树的权值和。这样的二叉树有很多，但树的带权路径和达到最小，则这棵树被称为哈夫曼树。

![huffman tree]({{ site.url }}/images/programs/huffman-tree-0.jpg "huffman tree"){: .pull-center }

### 最短编码

下面是对给定一棵树的编码规则：

1. 将单词的出现次数作为叶子结点，每个字符出现次数作为该叶子结点的权值。
2. 树中所有左分支表示字符 0，右分支表示字符 1，将依次从根结点到每个叶子结点所经过的分支的二进制位的序列作为该结点对应的字符编码。
3. 由于从根结点到任何一个叶子结点都不可能经过其它叶子，这种编码一定是前缀编码，树的带权路径长度正好是文件编码的总长度。

也就时说，如下图所示的哈夫曼树，可以编码为：

![huffman tree]({{ site.url }}/images/programs/huffman-tree-1.jpg "huffman tree"){: .pull-center }

因为树的带权路径和正好是文件编码的总长度，而哈夫曼树的带权路径和是最小的，故对应的编码也是最优的。

## 参考

各种语言常见的示例可以参考 [Huffman coding](http://rosettacode.org/wiki/Huffman_coding) 。

{% highlight text %}
{% endhighlight %}
