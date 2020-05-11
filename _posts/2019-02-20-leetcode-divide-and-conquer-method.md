---
title:
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: flink
description:
---


<!-- more -->


https://leetcode.com/tag/divide-and-conquer/


这里简单介绍一下递归、分治算法、动态规划、贪心算法、回溯算法的区别和联系。

递归是一种编程技巧，后面的几种算法基本都可以通过递归来实现；分治算法是将原问题不断拆解为更小的子问题处理，一般是二分，例如归并排序；贪心算法是动态规划的一个子集，动态规划可以通过记录结果，可降低对重复子问题的处理，从而提高效率；回溯算法在穷举的基础上增加了剪枝。

## 递归 Recursion

在描述某个问题的当前状态时，需要用到上个状态 (例如马尔科夫链) 甚至是之前的状态，这种方式成为递归，是一种编程技巧。

## 回溯 Backtrack

从一个问题的某种可能出发，遍历改场景下的所有结果，在遍历完之后回到原点，然后再从另外的一种可能出发继续搜索，这种不断回溯然后再继续寻找的方法就是回溯法。

解决一个回溯问题，实际上就是一个决策树的遍历过程。

1. Result 已经根据具体的条件做出的选择；
2. Items 选择的列表，本次可以选择那些元素；
3. 结束条件，到达了决策树的最后一层，一般使用的是递归方法，作为结束。

所以，大致的流程就是。

result = []
backtrack(result, items):
    if isOver():
		dumpInfo()  # 在某个决策树的叶子节点结束
        return
    for item in items[]:
		items.remove(item)  # 选择列表中删除
		result.add(item)    # 添加到结果中
        backtrack(result, items)
		result.remove(item)
		items.add(item)


## 全排列

给定一组数字或者字符串，其长度为 `n`，打印出其全排列，例如，对于字符串 `ABC` 的全排列有六种：`ABC` `ACB` `BAC` `BCA` `CAB` `CBA` 六种。

假设给定 `n` 个元素的数组，而且已经按照编号从小到大排列。

### 递归

可以使用递归方法求解，也就是先固定第一个元素，然后求出后 `n-1` 个元素的全排列；然后依次循环，直到遍历到最后一个元素。

以上述的 `ABCD` 字符串为例，其全排列为：

1. `A` 后面跟 `BCD` 的全排列；
2. `B` 后面跟 `ACD` 的全排列；
3. `C` 后面跟 `ABD` 的全排列；
4. `D` 后面跟 `ABC` 的全排列。

以第一行为例，在递归求解 `BCD` 的全排列即可，代码如下。

#include <stdio.h>

void swap(char *a, char *b)
{
        char t;
        t = *a;
        *a = *b;
        *b = t;
}

void permute(char *array, int begin, int end)
{
        int i;

        if (begin >= end) {
                for (i = 0; i <= end; i++)
                        printf("%c", array[i]);
                puts("");
                return;
        }

        for(i = begin; i <= end; i++) {
                swap(&array[begin], &array[i]);
                permute(array, begin + 1, end);
                swap(&array[begin], &array[i]);
        }
}

int main(void)
{
        char array[] = "ABC";

        permute(array, 0, 2);

        return 0;
}

对于上述的解法，如果出现了重复字符，例如 `ABB` ，那么同样会出现六种情况，而且会有重复，所以在交换前先检查之前是否有重复。

#include <stdio.h>

void swap(char *a, char *b)
{
        char t;
        t = *a;
        *a = *b;
        *b = t;
}

/* check [from, to) is same with to */
int is_repeat(char *array, int from, int to)
{
        for(int i = from; i < to; i++) {
                if (array[to] == array[i])
                        return 1;
        }
        return 0;
}

void permute(char *array, int begin, int end)
{
        int i;

        if (begin >= end) {
                for (i = 0; i <= end; i++)
                        printf("%c", array[i]);
                puts("");
                return;
        }

        for(i = begin; i <= end; i++) {
                if (is_repeat(array, begin, i))
                        continue;
                swap(&array[begin], &array[i]);
                permute(array, begin + 1, end);
                swap(&array[begin], &array[i]);
        }
}

int main(void)
{
        char array[] = "ABB";

        permute(array, 0, 2);

        return 0;
}



因为全排列有 `n!` 种类型，所以，无论采用什么方法，其对应的时间复杂度都是 `O(n!)` 。

非递归的实现
https://www.cnblogs.com/bakari/archive/2012/08/02/2620826.html
https://blog.csdn.net/so_geili/article/details/71078945
https://blog.csdn.net/maoyuanming0806/article/details/78930324
https://blog.csdn.net/e3399/article/details/7543861

## N 皇后

这是经典的问题，也就是在一个 `N*N` 的棋盘上放置 `N` 个皇后，保证其不能相互攻击。注意，皇后会攻击同一行、同一列、同一斜线上的皇后。

### 回溯算法 BackTrack

采用的是递归方式，当不满足的时候会返回，然后继续选择下一个节点判断是否满足，依次类推。

因为是个二维的棋盘，最直接的想法就是使用一个二维数组，在第 `i` 行 `j` 列上放置皇后，依次判断行、列、对角线、斜对角线上是否冲突即可。

在做递归时，行数依次递进，所以行是不会出现冲突的，那么直接查看列、右上方、左上方是否冲突即可。

#include <math.h>
#include <stdio.h>

#define QUEEN_MAX  8

int count;
char board[QUEEN_MAX][QUEEN_MAX];

void show(void)
{
        int i, j;

        for (i = 0; i < QUEEN_MAX; i++) {
                for (j = 0; j < QUEEN_MAX; j++)
                        printf(" %c", board[i][j]);
                printf("\n");
        }
        printf("---------------- %02d\n", ++count);
}

/* can we put Queen on the (row, col). */
int is_valid(int row, int col)
{
        int i, j;

        /* check the column */
        for (i = 0; i < QUEEN_MAX; i++)
                if (board[i][col] == 'Q')
                        return 0;

        /* check right-up */
        for (i = row - 1, j = col + 1; i >= 0 && j < QUEEN_MAX; i--, j++)
                if (board[i][j] == 'Q')
                        return 0;

        /* check left-up */
        for (i = row - 1, j = col - 1; i >= 0 && j >= 0; i--, j--)
                if (board[i][j] == 'Q')
                        return 0;

        return 1;
}

void EightQueen(int row)
{
        int col;

        if (row >= QUEEN_MAX) {
                show();
                return;
        }

        for (col = 0; col < QUEEN_MAX; col++) {
                if (is_valid(row, col) == 0)
                        continue;
                board[row][col] = 'Q';
                EightQueen(row + 1);
                board[row][col] = '.';
        }
}

int main(void)
{
        int i, j;

        for (i = 0; i < QUEEN_MAX; i++)
                for (j = 0; j < QUEEN_MAX; j++)
                        board[i][j] = '.';

        EightQueen(0);
        return 0;
}

不过这样实现有些复杂，更多的是采用一个一维数组实现。

把棋盘存储为一个 `N` 维数组，其中的第 `i` 个元素值代表第 `i` 行的皇后的列位置。在判断冲突时，因为每行只有一个皇后，显然行不会冲突，列冲突直接判断之前是否存在元素值与当前的列相等；斜线则是通过比较 `|row - i| == |col - a[i]|` 是否相同。

#include <math.h>
#include <stdio.h>

#define QUEEN_MAX  8

int queen[QUEEN_MAX];

void show(void)
{
        int i;
        for(i = 0; i < QUEEN_MAX; i++)
                printf("(%d, %d)  ", i, queen[i]);
        printf("\n");
}

int is_valid(int row)
{
        int col;
        for (col = 0; col < row; col++)
                if (queen[col] == queen[row] || abs(queen[row] - queen[col]) == (row - col))
                        return 0;
        return 1;
}

void EightQueen(int row)
{
        int col;

        if (row >= QUEEN_MAX) {
                show();
                return;
        }

        for (col = 0; col < QUEEN_MAX; col++) {
                queen[row] = col;
                if (is_valid(row))
                        EightQueen(row + 1);
        }
}

int main(void)
{
        EightQueen(0);
        return 0;
}

回溯算法没有动态规划的重叠子问题，无法通过记录历史值减少计算量，只能在当判断了非法时进行剪枝，如果无法剪枝，那么就成了 `O(n!)` 级的复杂度了，而且任何方法无法优化。



{% highlight text %}
{% endhighlight %}
