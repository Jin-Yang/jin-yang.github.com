---
title: 二分查找
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

二分查找的思路很简单，但是却有很多的细节问题，例如整型溢出、边界的递进等等。

<!-- more -->

## 简介

在进行二分查找时，包括了几个比较关键的点：A) 确定搜索的范围；B) 停止搜索条件；C) 边界递进的条件。

边界确定也就是开始设置的 `left` 值的大小，可以是 `length - 1` 或者 `length` ，那么其对应的初始搜索空间分别为 `[0, length - 1]` 以及 `[0, length)` ，为了方便处理，在进行迭代时也会按照相同的属性进行，也就是 `[left, right]` 或者 `[left, right)` 。

边界的选择决定后续的其它处理的细节。

## 基本搜索

在获取中间值得时候，采用的是 `left + (right - left) / 2` 而非 `(left + right) / 2` ，虽然两者等价，但是前者可以防止由于整形溢出导致的计算结果错误。当时偶数个元素时，会选择偏左的值，例如 `6` 第一元素为 `2` 而非 `3` 。

当通过中值查找到了对应的目标值后，就可以直接返回结果了；但是如果目标不存在，最终实际会通过 `while()` 循环中的判断条件退出，这样退出条件的判断也就很关键了。

示例代码如下。

{% highlight c %}
int binary_search(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -EINVAL;

        left = 0;
        right = length - 1;
        while (left <= right) { // [left, right]
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        return mid;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid - 1;
        }

        return -1;
}
{% endhighlight %}

### 退出条件

这里采用的是 `[0, length - 1]` 的边界，另外一种情况后面讨论。

使用 `while (left <= right)` 作为终止条件，那么当 `left = right + 1` 时会因为不满足条件而退出，其中比较关键的是会比较最后一次值，也就是 `right` (此时 `left` 和 `right` 相等)。

而 `while(left < right)` 的终止条件是 `left = right`，会忽略掉比较 `right` 对应的值，即使值存在，那么也可能会返回不存在。可以在最后添加 `return nums[left] == target ? left : -1;` 进行修改，消除异常。

### 边界更新

上述实际上已经判断过了 `mid` 值是否满足条件，显然接下来应该从 `[left, mid - 1]` 或者 `[mid + 1, right]` 继续搜索，这也就对应的每次搜索边界的更新方式。

<!--
个人感觉使用 `while(left < right)` 会更好一些，会减少一次不必要的 `mid` 值计算。
-->

### 其它

这里简单介绍另外的一种更新策略，对应的搜索空间为 `[left, right)` ，同时需要修改两个点。

* `while (left < right)` 对应终止条件是 `left = right`，而搜索空间为 `[left, right)` 空，可以正常终止。
* 更新时采用 `right = mid` ，因为在确认 `mid` 不满足条件后，对应的搜索空间应该是 `[left, mid)` 或 `[mid + 1, right)` 。

综上，可以将代码修改为。

{% highlight c %}
int binary_search(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -EINVAL;

        left = 0;
        right = length;
        while (left < right) { // [left, right)
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        return mid;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid;
        }

        return -1;
}
{% endhighlight %}

## 左边界搜索

假设存在有序数组 `[1, 2, 2, 2, 3]`，需要搜索 `2` ，默认的二分查找返回的是 `2` ，如果要得到左侧边界 (索引 `1`) 或者右侧边界 (索引 `3`) ，上述的方法是无法进行处理的。

当然可以线性查找，但是这样就无法保证二分查找对数级的复杂度了。

### 实现

这里的关键是，在判断 `array[mid] == target` 满足条件后，并没有直接返回，而是执行 `right = mid` ，也就是更新的搜索范围。

如果直接返回 `left` 或者 `right` 实际的含义是，数组中小于目标元素的个数，例如 `array = [2, 3, 5, 7], target = 1` 返回 `0` ，`array = [2, 3, 5, 7], target = 8` 返回 `4` 。

{% highlight c %}
int left_bound(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -1;

        left = 0;
        right = length - 1;
        while (left <= right) { // [left, right]
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        right = mid - 1;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid - 1;
        }
        //return left; // how many items less than target.
        if (left >= length || array[left] != target)
                return -1;
        return left;
}
{% endhighlight %}

同样更新下第二种方式的实现。

{% highlight c %}
int left_bound(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -1;

        left = 0;
        right = length;
        while (left < right) { // [left, right)
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        right = mid;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid;
        }
        //return left; // how many items less than target.
        if (left >= length || array[left] != target)
                return -1;
        return left;
}
{% endhighlight %}

## 右边界搜索

唯一特殊的是返回值为 `left - 1` ，这主要与 `left` 的更新策略有关，使用的是 `left = mid + 1` ，那么返回时就需要减去 `1` 。

{% highlight c %}
int right_bound(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -1;

        left = 0;
        right = length - 1;
        while (left <= right) { // [left, right]
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        left = mid + 1;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid - 1;
        }

        //return left; // how many items less or equal to target.
        if (right < 0 || array[right] != target) // left = right + 1
                return -1;
        return right;
}
{% endhighlight %}

同样，第二种实现方案为。

{% highlight c %}
int right_bound(int *array, int length, int target)
{
        int left, right, mid;

        if (array == NULL || length <= 0)
                return -1;

        left = 0;
        right = length;
        while (left < right) { // [left, right)
                mid = left + (right - left) / 2;
                if (array[mid] == target)
                        left = mid + 1;
                else if (array[mid] < target)
                        left = mid + 1;
                else if (array[mid] > target)
                        right = mid;
        }
        //return left; // how many items less or equal to target.
        if (left <= 0 || array[left - 1] != target)
                return -1;
        return left - 1;
}
{% endhighlight %}

## 总结

边界条件或者检查范围决定了搜索范围的更新策略，而 `while` 循环则决定了终止的条件。

{% highlight text %}
{% endhighlight %}
