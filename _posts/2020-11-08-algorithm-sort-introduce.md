---
title: 排序算法简介
layout: post
comments: true
language: chinese
tag: [SoftWare]
keywords:
description:
---


<!-- more -->

## 简介

简单介绍下常见的概念。

* 原地排序(in-place)，不申请多余空间进行排序，通过在原数据中比较和交换完成排序。

### 稳定性

不稳定排序算法可能会在相等的键值中改变纪录的相对次序，但是稳定排序算法从来不会如此。假设有以下的数，使用第一个数字进行排序。

{% highlight text %}
(4, 1)  (3, 1)  (3, 7)  (5, 6)
{% endhighlight %}

在这个状况下，有可能产生两种不同的结果，一个是依照相等的键值维持相对的次序，而另外一个则没有，也就是结果为。

{% highlight text %}
(3, 1)  (3, 7)  (4, 1)  (5, 6)   (稳定)
(3, 7)  (3, 1)  (4, 1)  (5, 6)   (不稳定)
{% endhighlight %}

不稳定排序算法可以被特别地实现为稳定，通常需要人工扩充键值的比较，比如上面增加对第二个键值大小的比较。

## 冒泡排序 Bubble Sort

优点是编程简单，对于几乎已经排序好的速度较快，而逆序和常规性能较差，通常用来与其它算法进行比较。

![bubble algorithm](/{{ site.imgdir }}/algorithm/sort/bubble1.gif "bubble algorithm")

简单示例如下。

![bubble algorithm](/{{ site.imgdir }}/algorithm/sort/bubble2.gif "bubble algorithm")

对于 n 个数组排序的伪代码 (排序后为从大到小) 如下所示。

```
for  limit = n – 1 to 1 begin
   for i = 0 to limit - 1 begin
     if array[i] > array[i+1] then   // 记一次比较
        swap array[i] and array[i+1] // 记 3 次赋值
   end
end
```

基本步骤为：

* 比较相邻两个数据，如果前者大于后者则交换。
* 将第 `1` 个数据进行 `n-1` 次比较和交换后，则最大的数据就会 "沉" 到 `n` 的位置。
* 不断重复上步，直到排序完成。

该算法对记录执行 `n-1` 次遍历 (使用标志的改进版本遍历次数将会减少)，每次遍历将执行 `n-1` 次比较，对逆序会执行 `n-1` 次同样次数的比较和交换。因此，该算法的 **最坏运行时间** 为逆序时，此时执行 `(n-1)+(n-2)+...+1=n(n-1)/2` 次比较和交换，即与 `n(n-1)` 成正比，或者只是仅仅与 `n^2` 成正比。当顺序时运行时间最优，此时只有 `n-1` 比较，而没有赋值操作。

常见的一个改进版本为设置一个标志，如果没有交换则说明排序已经完成，此时可以退出不必进行不必要的交换。

另一个改进版本为，如果对于 `i` 以下的数组已经排序完成且都大于 `i` 之前的数据，因此可以在下次排序中只对 `i` 之前的数据进行排序。相见代码 `Bubble.c` 中的 `BubbleSort_Pro()` 实现。

算法性能，最好为顺序时 `O(n)`，最坏为逆序时 `O(n^2)`，平均为 `O(n^2)`，不需要额外的存储空间，为原地排序，且排序是稳定的。如果使用通常用于较少数据的排序，对于大量数据的排序应该选择其他排序方法。

<center>
<table style="font-size: small;" align="center" bgcolor="lightyellow" border="1" cellpadding="3" cellspacing="0" width="90%">
<tbody>
<tr align="center" bgcolor="lightblue">
	<td>方法</td><td colspan="6">排序过程</td><td>i取值</td><td>交换</td><td>比较</td><td width=180>其他</td></tr>
<tr align="center"><td rowspan="6">基本</td><td>2</td><td>3</td><td>0</td><td>4</td><td>5</td><td>6</td><td>NaN</td><td>NaN</td><td>NaN</td><td align="left" rowspan="6">基本冒泡排序，第二步已经完成了排序，但是仍会进行比较。</td></tr>
<tr align="center"><td>2</td><td>0</td><td>3</td><td>4</td><td>5</td><td bgcolor="lightgreen">6</td><td>5</td><td>1</td><td>5</td></tr>
<tr align="center"><td>0</td><td>2</td><td>3</td><td>4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>4</td><td>1</td><td>4</td></tr>
<tr align="center"><td>0</td><td>2</td><td>3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>3</td><td>0</td><td>3</td></tr>
<tr align="center"><td>0</td><td>2</td><td bgcolor="lightgreen">3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>2</td><td>0</td><td>2</td></tr>
<tr align="center"><td>0</td><td bgcolor="lightgreen">2</td><td bgcolor="lightgreen">3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>1</td><td>0</td><td>1</td></tr>
<tr align="center" bgcolor="silver"><td>总计</td><td colspan="7"></td><td>2</td><td>15</td><td></td></tr>

<tr align="center"><td rowspan="4">改进1</td><td>2</td><td>3</td><td>0</td><td>4</td><td>5</td><td>6</td><td>NaN</td><td>NaN</td><td>NaN</td><td align="left" rowspan="4">冒泡排序改进1，第三步时已没有数据交换，因此判断排序完成，退出。</td></tr>
<tr align="center"><td>2</td><td>0</td><td>3</td><td>4</td><td>5</td><td bgcolor="lightgreen">6</td><td>5</td><td>1</td><td>5</td></tr>
<tr align="center"><td>0</td><td>2</td><td>3</td><td>4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>4</td><td>1</td><td>4</td></tr>
<tr align="center"><td>0</td><td bgcolor="lightgreen">2</td><td bgcolor="lightgreen">3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>3</td><td>0</td><td>3</td></tr>
<tr align="center" bgcolor="silver"><td>总计</td><td colspan="7"></td><td>2</td><td>12</td><td></td></tr>

<tr align="center"><td rowspan="3">改进2</td><td>2</td><td>3</td><td>0</td><td>4</td><td>5</td><td>6</td><td>NaN</td><td>NaN</td><td>NaN</td><td align="left" rowspan="3">冒泡排序改进2，对于已完成的排序，将不会再进行排序。</td></tr>
<tr align="center"><td>2</td><td>0</td><td>3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>5</td><td>1</td><td>5</td></tr>
<tr align="center"><td>0</td><td bgcolor="lightgreen">2</td><td bgcolor="lightgreen">3</td><td bgcolor="lightgreen">4</td><td bgcolor="lightgreen">5</td><td bgcolor="lightgreen">6</td><td>1</td><td>1</td><td>1</td></tr>
<tr align="center" bgcolor="silver"><td>总计</td><td colspan="7"></td><td>2</td><td>6</td><td></td></tr>
</tbody></table></center>

## 插入排序 Insertion Sort

![insertion algorithm](/{{ site.imgdir }}/algorithm/sort/insertion.gif "insertion algorithm")

通过构建有序序列，对于未排序数据，在已排序序列中从后向前扫描，找到相应位置并插入。插入排序在实现上，通常采用 in-place 排序 (即只需用到 `O(1)` 的额外空间的排序)，因而在从后向前扫描过程中，需要反复把已排序元素逐步向后挪位，为最新元素提供插入空间。

与打牌时的排序相似，每选取一张牌时，就相对于以前的牌插入的正确的位置。实现插入排序和实现冒泡排序一样简单，不过，要考虑一个重要的要点：在选取每个新记录并准备把它插入到有序记录当中时，应该从头查找还是从末尾查找？通常这与需要排序数据的顺序有关，如果是顺序则应该在末尾查找，如果是逆序应该从头查找；此处为 **从末尾查找** 。

```
for step = 1 to N-1 begin
   temp = array[step]     // 记 1 次赋值
   for  i = step  to 1 begin
      if array[i - 1] > temp then // 记 1 次比较
         array[i ] = array[i - 1] // 记 1 次赋值
      else
         break
   end
   array[i] = temp      // 记 1 次赋值
end
```

在每次插入一个数据时都会进行遍历以找到合适的位置，最坏的情况为 step 次，与冒泡排序不同的是在每一步中不会执行完全交换，而是“半交换”。另外插入排序将会避开一半的比较次数，因为它插入新记录时通常只会查看一半的有序记录。

如果是正序则需要在开始处的一次赋值，总计 `(n-1)` 次赋值，等价为 `(n-1)/2` 次交换；`(n-1)` 次比较。

如果是逆序则需要在开始处的一次赋值，并在结束处有一次赋值，每个次循环中有 step 次赋值，因此总计 `(n-1)+(n-1)+[1+2+...+n-1] = (n-1)(n+4)/2` 次赋值，等价为 `(n-1)(n+4)/4` 次交换；每个子循环中有 step 次比较，总计 `1+2+...+n-1=n(n-1)/2` 次比较。

算法性能，最好为顺序时 `O(n)`，最坏为逆序时 `O(n^2)`，平均为 `O(n^2)`，不需要额外的存储空间，为原地排序，且排序是稳定的。插入排序不适合对于数据量比较大的排序应用，如果需要排序的数据量很小，例如，量级小于千，那么插入排序还是一个不错的选择。

示例代码如下。

``` c
void InsertionSort(int array[], unsigned int first, unsigned int last)
{
	int i, j;
	int temp;

	for(i = first+1; i <= last; i++){
		// store the original sorted array in temp
		temp = array[i];
		// compare the new array with temp
#ifdef INCREASE__SORT
		for(j = i; temp<array[j-1] && j>first; j--){
#else
		for(j = i; temp>array[j-1] && j>first; j--){
#endif
			// all larger elements are moved one pot to the right.
			array[j] = array[j-1];
		}
		array[j] = temp;
	}
}
```


<!--
file:///run/media/andy/Folder/Html/Html/Program/linux/algo_struct.html
-->
