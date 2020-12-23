---
title: 洗牌算法
layout: post
comments: true
usemath: true
language: chinese
category: [linux,misc]
keywords:
description:
---

需要保证是一个公平的随机算法，首先介绍如何打乱，然后是如何判断已经打乱，也就是符合均匀分布，满足公平。

<!-- more -->

## 洗牌算法

在介绍时会包含了两个基本的算法 Fisher-Yates Shuffle 以及 Knuth-Durstenfeld Shuffle ，一般会说后者是通过前者优化得来，但怎么感觉后者只是前者的计算机实现而已，所以还是只介绍后者的实现吧。

<!--
## Fisher-Yates Shuffle

最早由 Ronald A. Fisher 和 Frank Yates 提出，基本思想就是从原始数组中随机取一个之前没取过的数字到新数组中，具体如下：

1. 初始化原始数组和新数组，原始数组长度为n(已知)；
2. 从还没处理的数组（假如还剩k个）中，随机产生一个[0, k)之间的数字p（假设数组从0开始）；
3. 从剩下的k个数中把第p个数取出；
4. 重复步骤2和3直到数字全部取完；
5. 从步骤3取出的数字序列便是一个打乱了的数列。

下面证明其随机性，假设有 $n$ 个元素的数组，也就是要证明每个元素被放置在新数组中第 $i$ 个位置的概率是 $\frac{1}{n}$ 。

一个元素 $m$ 被放入第 $i$ 个位置的概率 $p$ 为，前 $i - 1$ 个位置没有选中 $m$ ，而第 $i$ 个位置选中 $m$ 的概率。

$$P = \frac{n-1}{n} \times \frac{n-2}{n-1} \times \cdots \times \frac{n-i+1}{n-i+2} \times \frac{1}{n-i+1}=\frac{1}{n}$$

据说时间复杂度是 O(n*n) 空间复杂度为 O(n) ，暂时不确定为啥是 n*n
-->

### Knuth-Durstenfeld Shuffle

每次从未处理的数据中随机取出一个数字，然后把该数字与尾部的数值进行交换，这样数组尾部存放的是已经处理过的数字。

出现在各个位置的概率计算如下。

* 出现在第 $n-1$ 位置处的概率为 $\frac{1}{n}$ 。
* 出现在第 $n-2$ 位置处的概率为 $\frac{n-1}{n} \times \frac{1}{n-1}$ 。
* ... ...
* 出现在第 $n-k$ 位置处的概率为 $\frac{n-1}{n} \times \frac{1}{n-1} \times \cdots \times \frac{n-k+1}{n-k+2} \times \frac{1}{n-k+1}=\frac{1}{n}$ 。

如下是一个简单的示例。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/* random value in [min, max] */
int do_random(int min, int max)
{
        return rand() % (max - min) + min;
}

void swap(int *arr, int i, int j)
{
        int t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
}

void shuffle(int *arr, int n)
{
        int i, rnd;

        for (i = 0; i < n - 1; i++) {
                rnd = do_random(i, n - 1);
                swap(arr, i, rnd);
        }
}

#define S 6
#define N 40000

int main(void)
{
        int i, j;
        int count[S] = {0};
        int array[S] = {0, 1, 2, 3, 4, 5};

        srand(time(NULL));
        for (i = 0; i < N; i++) {
                shuffle(array, S);
                for (j = 0; j < S; j++) {
                        if (array[j] == 0) {
                                count[j]++;
                                break;
                        }
                }
        }

        for (i = 0; i < S; i++)
                printf("%2d  %d\n", i, count[i]);
        return 0;
}
{% endhighlight %}

其中，对于判断条件 `i < n - 1` 来说，在只剩下了最后一个元素时，不再调用随机函数，这样一来随机函数的处理也会比较简单，无需判断边界是否相等。

还有一种，判断条件为 `i < n` ，也就是在剩下最后一个元素时，仍然会再调用一次 `do_random()` 函数，此时 `max` 与 `min` 相等，所以在函数中需要坐下处理，函数修改为。

{% highlight c %}
int do_random(int min, int max)
{
        if (min == max)
				return min;
        return rand() % (max - min) + min;
}
{% endhighlight %}

另外，还可以反向进行计算，对应的判断条件为。

{% highlight c %}
for (i = n - 1; i > 0; i--) {
        rnd = do_random(0, i);

for (i = n - 1; i >= 0; i--) {
        rnd = do_random(0, i);
{% endhighlight %}

对于边界的处理与上面的方式相同。

在验证的时候，可以统计某个位出现在各个位置的概率，如上是统计 `0` 出现在各个位置的次数。

## 蓄水池采样

其场景为，对一个很大的总体抽取 $K$ 个样本，总体可能会很大，无法完全放到内存中，也有可能不确定其大小。

这也就是要讲到的蓄水池采样算法 (Reservoir Sampling)，假设总体为 $N$ ，然后采样 $K$ 个，那么其执行过程为：

1. 构建一个可容纳 $K$ 个元素的数组，将序列的前 $K$ 个元素放入数组中。
2. 从第 $K+1$ 个元素开始，以 $\frac{k}{k+1}$ 的概率来决定该元素是否被替换到数组中。
3. 重复上一步骤，直到遍历完所有元素，剩下的元素就是所需采样的样本。

接下来，证明每个元素可能被采样的概率均为 $\frac{k}{n}$ 。

### 概率证明

对于第 $i \ (i \leqslant k)$ 个元素来说，在第 $k$ 步前被选中的概率为 $1$ ，当走到第 $k+1$ 步时，被 $k+1$ 个元素替换的概率为 $k+1$ 个元素被选中，且第 $i$ 被选中替换，也就是 $\frac{k}{k+1}\times \frac{1}{k}=\frac{1}{k+1}$ ，那么被保留的概率为 $1-\frac{1}{k+1}=\frac{k}{k+1}$ 。

依次类推，可以得到不被 $k+2$ 个元素替换的概率为 $\frac{k+1}{k+2}$ ，那么当运行到第 $n$ 步时，被保留的概率为，被选中且不被替换的概率，也就是。

$$1 \times \frac{k}{k+1} \times \frac{k+1}{k+2} \times \cdots \times \frac{n-1}{n}=\frac{k}{n}$$

同样，对于第 $j \ (j > k) $ 个元素来说，其在第 $j$ 步中被选中的概率为 $\frac{k}{j}$ ，而不被 $j+1$ 个元素替换掉的概率为 $1-\frac{1}{j+1} \times \frac{1}{k}=\frac{1}{j+1}$ 。那么，运行到第 $n$ 步时，被保留的概率同样为，被选中且不被替换的概率，也就是：

$$\frac{k}{j} \times \frac{j}{j+1} \times \frac{j+1}{j+2} \times \cdots \times \frac{n-1}{n}=\frac{k}{n}$$

也就是说，每个元素被保留的概率为 $\frac{k}{n}$ 。

### 代码实现

实现的采样代码以及测试如下。

{% highlight c %}
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void sampling(int *data, int k, int *pool, int n)
{
        int rnd, i;

        for (i = 0; i < k; i++)
                data[i] = pool[i];

        for (i = k; i < n; i++) {
                rnd = rand() % (i + 1);
                if (rnd < k)
                        data[rnd] = pool[i];
        }
}

#define N   10
#define K   5
#define CNT 60000

int main(void)
{
        int i, j, data[K];
        int array[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int count[N] = {0};

        srand(time(NULL));
        for (i = 0; i < CNT; i++) {
                sampling(data, K, array, N);
                for (j = 0; j < K; j++)
                        count[data[j]]++;
        }

        for (i = 0; i < N; i++)
                printf("%2d  %d\n", i, count[i]);

        return 0;
}
{% endhighlight %}

## 参考

* 与之相关的几个 LeetCode 试题 [382. Linked List Random Node](https://leetcode.com/problems/linked-list-random-node/)、[398. Random Pick Index](https://leetcode.com/problems/random-pick-index/)、[384. Shuffle an Array](https://leetcode.com/problems/shuffle-an-array/) 。

<!--
还有啥蓄水池算法
https://blog.csdn.net/qq_26399665/article/details/79831490

有不错的动画
https://bost.ocks.org/mike/shuffle/
-->

{% highlight text %}
{% endhighlight %}
