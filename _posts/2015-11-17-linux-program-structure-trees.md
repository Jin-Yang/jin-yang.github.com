---
title: 数据结构 -- Tree
layout: post
comments: true
language: chinese
category: [program]
keywords: 数据结构,树,tree
description: 抽象地说，基本上有序列的地方就可以应用树，因为树结构即是一种序列索引结构，这里简单介绍常见的树结构。
---

抽象地说，基本上有序列的地方就可以应用树，因为树结构即是一种序列索引结构，这里简单介绍常见的树结构。

<!-- more -->

## 简介

二叉树 (Binary tree) 每个节点最多只有两个分支的树结构，其分支通常被称为 "左子树" 和 "右子树" 。二叉搜索树 (Binary Search Tree) 是二叉树的一种，对于所有节点，其左子树的值小于根结点的值，右子树的值大于根结点的值。

查找、插入和删除的效率与数的高度 ```log n``` 相关，最坏可能达到 ```O(n)``` ，如下是其最坏情况，几乎达到了线性查找。

![tree]({{ site.url }}/images/structure/tree_unbalanced_bst.jpg "tree"){: .pull-center }

为了保证树的高度，也就出现了如下的平衡二叉树，例如 AVL Tree、Red-Black Tree。

## AVL Tree

自平衡二叉查找树 (AVL Tree) 中任意节点的两个子树的高度差最大为 1，查找、插入和删除在平均和最坏情况下都是 ```O(log n)```，插入和删除可能需要通过一次或多次树旋转来重新平衡这个树。这种二叉树得名于其发明者 G.M. Adelson-Velsky 和 E.M. Landis 在 1962 年的论文《An algorithm for the organization of information》。

![avl tree example]({{ site.url }}/images/structure/tree_avl_example.png "avl tree example"){: .pull-center width="90%" }

一般每个节点会保存一个平衡因子 (Balance Factor) 或者通过子树高度计算，一般是左子树的高度减去它的右子树的高度，平衡因子为 1、0 或 -1 的节点被认为是平衡的，而 -2、2 的节点被认为是不平衡的，并需要重新平衡这个树。

为了保持平衡，AVL Tree 可能需要执行四种的旋转方式：

![avl tree example]({{ site.url }}/images/structure/tree_avl_rotations.png "avl tree example"){: .pull-center width="90%" }

<!--
http://btechsmartclass.com/DS/U5_T2.html
https://www.tutorialspoint.com/data_structures_algorithms/avl_tree_algorithm.htm
-->

## Red-Black Tree

Red-Black Tree 和 AVL Tree 是常用的平衡二叉搜索树，可以保证插入、查找、删除操作的最坏情况都是 ```O(log n)``` 。

<!--
Both red-black trees and AVL trees are the most commonly used balanced binary search trees and they support insertion, deletion and look-up in guaranteed O(logN) time. However, there are following points of comparison between the two:

    AVL trees are more rigidly balanced and hence provide faster look-ups. Thus for a look-up intensive task use an AVL tree.
    For an insert intensive tasks, use a Red-Black tree.
    AVL trees store the balance factor at each node. This takes O(N) extra space. However, if we know that the keys that will be inserted in the tree will always be greater than zero, we can use the sign bit of the keys to store the colour information of a red-black tree. Thus, in such cases red-black tree takes O(1) extra space.
In general, the rotations for an AVL tree are harder to implement and debug than that for a Red-Black tree.

http://web.mit.edu/~emin/Desktop/ref_to_emin/www.old/source_code/red_black_tree/index.html

https://github.com/mirek/rb_tree


Rust语言
https://github.com/rustcc/rust-zh
Rust安装
https://blog.csdn.net/tjzzy/article/details/88635514

Binary Tree (二叉树 只有两个节点)
 |-> Full Binary Tree(完满二叉树 叶子在同一层)
 |-> Complete Binary Tree(完整二叉树 按照从上到下从左到右满足FBT)
 |-> Splay Tree(伸展树)
 |-> B+ Tree
 |-> Radix Tree
 |-> Binary Search Tree(二叉搜索树 按照Key排序)
    |-> Threaded Binary Tree(线索二叉树)
    |-> AVL Tree(自平衡二叉树)
    |-> Red Black Tree(红黑树)


接著來看RBT的五項特徵：

* 每个节点只能是红色或者黑色；
* 根节点一定是黑色；
* 每个叶子节点一定是黑色；
* 当某个节点是红色时，其两个子节点必定是黑色，不能出现两个红色节点相连，黑色节点没有限制；
* 从任何一个节点到叶子节点的黑色节点数量相同。

通过这些约束保证了红黑树的关键特性：从根到叶子的最长路径不多于最短路径的两倍。保证树是大致平衡的，当红黑树的节点数为 `n` 时，树的高度不会超过 `2log(n+1)` 。

上述的性质 4 保证了根到叶子的路径不能有两个相邻的红色节点，最短路径可能都是黑色节点，最长路径可能有交替的红黑节点。而性质 5 保证了没有路径能多于任何其它路径的两倍长。

现实中 C++ 中的 `std::set` 和 `std:map` 使用的就是红黑树，包括了内核中的很多数据维护。

相比于 AVL 树来说，牺牲了部分平衡性以换取插入和删除时的旋转操作。

伸展数实现介绍
https://www.cnblogs.com/vamei/archive/2013/03/24/2976545.html

https://github.com/fbuihuu/libtree
https://github.com/alrightchiu/SecondRound

~/workspace/kernel/linux-4.19.59/lib/rbtree.c

如果标明结构体中的成员是 2 字节对齐的，那么可以将红黑的标识添加到 `parent` 的指针中，这样可以减少一个成员的使用。

https://www.geeksforgeeks.org/red-black-tree-vs-avl-tree/
http://alrightchiu.github.io/SecondRound/treeshu-introjian-jie.html

搜索顺序

* Pre-Order 先当前节点，然后左节点，最后右节点。
* In-Order 首先左节点，然后当前节点，最后右节点。
* Post-Order 首先左节点，然后右节点，最后当前节点。
* Level-Order 每层按照从左到右的顺序遍历。

在 C99 中定义了 `intptr_t` `uintptr_t` 两种类型，在头文件 `stdint.h` 中，都是为了适配不同机器 (32bits 64bits) 上的 `void *` 指针，同时也可保存 `char` `short` `int` `long` 证书类型的值。

https://github.com/ShiqiYu/libfacedetection




##############################################
## 树
##############################################

## LSM-Tree

Log Structured Merge Tree 的数据组织方式被应用于多种数据库，如 LevelDB、HBase、Cassandra 等，相比 B+ Tree 以及其它的索引存储实现，提供了更好的写入性能。

在存储数据时，会将随机写转换为顺序写，其处理逻辑类似如下。

1. 写入或者更新时，只更新内存中的数据结构，一般通过某种数据结构 (例如 SkipList) 保持有序；
2. 为了防止内存中的数据掉电丢失，一般也会将数据追加写到磁盘 Log 文件中，做容灾恢复；
3. 内存中的数据定时或按固定大小地刷新到磁盘，注意，只会新建文件，不会更新磁盘上的已有文件；
4. 随着数据写入，磁盘上会有越来越多的文件，会定期合并文件。

LSM-Tree 的树节点核心分成两种，保存在内存中的称为 MemTable ，在磁盘上的为 SSTable 。


## 基本概念

* Degree 度，一个节点拥有子树的个数，例如 A 为 3，F 为 2 ，N 为 0；
* Root 根，也即是最上层的节点，其父节点是 NULL；
* Leaf 叶子节点，没有子节点的节点被称为叶子；
* Siblings 同胞，具有相同父节点的节点，例如 BCD；
* Descendant 子嗣，从某个节点出发，能通过 child 指针找到的节点；
* Ancestor 祖先，同上，不过是通过 parent 指针指向的节点；
* Path 路径，能够通过 child 或者 parent 指针将两个节点连接起来的路径，例如 ABEK；
* Level 层级，将 root 定义为 1 ，其它节点为其 parent 的层级加一；
* Height of Node 节点高度，某个节点到叶子节点的最长路径数，例如 F 为 1，D 为 2，叶子节点为 0；
* Height of Tree 数高度，也就是根节点的高度。
* Depth 节点深度，一个节点与根节点之间的边数，例如 F 为 2，L 为 3。


一般来说，会按照 Graph、Tree、Binary Tree、Binary Search Tree、Red Black Tree 的层级依次向下。

在 Graph 的基础上增加 "不能存在环" 的限制则对应了 Tree；在 Tree 的基础上添加 "最多两个子节点" 就成了二元树；再增加 "按照 Key 大小排序" 就得到了 Binary Search Tree；然后再在每个节点上增加红黑标示，以平衡树的高度，减少搜索时间，那么就得到了 Red Black Tree 。

除了红黑树之外，还有 AVL Tree、2-3-4 Tree、Splay Tree 等。

## 二叉树

如果树的节点只有指向左子树和右子树时，则称为二叉树 Binary Tree ，其中两类二叉树比较常见：Full Binary Tree 以及 Complete Binary Tree 。

### Full Binary Tree

也被称为 Perfect Binary Tree ，具有如下特性：

* 所有的内部节点有且只有两个子树；
* 所有的叶子节点都有相同的高度。

如果整棵树有 `n` 层，那么有 `2^n - 1` 个节点，其第 `i` 个节点的左子树序号为 `2i` ，右子树的序号为 `2i + 1` 。

### Complete Binary Tree

当一棵树按照 Full Binary Tree 的顺序，也就是从上到下，由左到右的顺序。

一般来说，二叉树具有如下的特性：
1. 任意结点的左子树不空，则左子树上所有结点的值均小于它的根结点的值；
2. 若任意结点的右子树不空，则右子树上所有结点的值均大于它的根结点的值；
3. 任意结点的左、右子树也分别为二叉查找树。
4. 没有键值相等的结点。

红黑树同样也是对二叉树的改进版本，通过一些性质使得树相对平衡，使得最终查找、插入、删除的时间复杂度最坏情况下依然为 O(lgn)。

但它在二叉查找树的基础上增加了着色和相关的性质使得红黑树相对平衡，这主要是通过红黑树的 5 条性质保证：

1. 每个结点要么是红的，要么是黑的。
2. 根结点是黑的。
3. 每个叶结点 (叶结点即指树尾端NIL指针或NULL结点) 是黑的。
4. 如果一个结点是红的，那么它的两个子节点都是黑的。
5. 对于任一结点而言，其到叶结点树尾端NIL指针的每一条路径都包含相同数目的黑结点。

通过这 5 条性质使得一棵 N 个结点是红黑树始终保持了 logN 的高度；其中上述的 "叶结点" 或 "NULL结点" 不包含数据而只充当树的结束标志，这些结点通常会在绘图中被省略。

### 树的旋转

在执行了一些插入和删除之后，新的结构可能会违背红黑树的性质，为此需要对数重新着色，并执行相关的旋转操作，从而继续保持其性质和平衡。

https://github.com/julycoding/The-Art-Of-Programming-By-July/blob/master/ebook/zh/03.01.md

1. 可以设置多个节点 (Node)，每个节点里有多个虚拟节点；
2. 红黑树维护的是虚拟节点的信息。




#define SIZE 50000

#define HASH_T_KEY  int
#define HASH_T_VAL  int

#define KEY_INVALID 0

struct hash_entry {
        HASH_T_KEY key;
        HASH_T_VAL value;
};

struct hash_table {
        int size;
        struct hash_entry *items;
};

static int hash_func(int key, int size)
{
        int rc;

        assert(size > 0);
        rc = key % size;
        return rc < 0 ? rc + size : rc;
}

struct hash_table *hash_create(int size)
{
        struct hash_table *t;

        if (size <= 0)
                return NULL;

        t = calloc(1, sizeof(*t));
        if (t == NULL)
                return NULL;
        t->items = calloc(size, sizeof(struct hash_entry));
        if (t->items == NULL) {
                free(t);
                return NULL;
        }

        return t;
}

void hash_destroy(struct hash_table *t)
{
        if (t == NULL)
                return;
        if (t->items != NULL)
                free(t->items);
        free(t);
}

void hash_put(struct hash_table *t, HASH_T_KEY key, HASH_T_VAL value)
{
        int idx;

        idx = hash(key);
        while (t->items[idx].key != key)
                idx = (idx + 1) % t->size;
        t->items[idx].key = key;
        t->items[idx].value = value;
}

HASH_T_VAL hash_get(struct hash_table *t, HASH_T_KEY key)
{
        int idx;

        idx = hash(key);
        while (t->items[idx].key != KEY_INVALID) {
                if (t->items[idx].key == key)
                        return t->items[idx].value;
                idx = (idx + 1) % t->size;
        }

        return KEY_INVALID;
}

-->








{% highlight text %}
{% endhighlight %}
