---
title: GoLang Array VS. Slice
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: 
description:
---

两者十分类似，很容易造成混淆。

<!-- more -->

## 简介

### 数组

数组会根据元素个数以及其中元素的类型分配内存空间，这也就意味着，在编译阶段必须包含这两个元素，要么显示指定，要么可以在编译阶段间接推断出来。

{% highlight go %}
package main

import (
        "fmt"
)

func main() {
        array := [5]int{1, 2, 3, 4, 5}
        //array := [5]int{1, 2}
        //array := [...]int{1, 2, 3, 4, 5}
        //array := [5]int{4: 5}
        //array := [...]int{2: 3, 3: 4, 4: 5}

        fmt.Printf("array %#v\n", array)
}
{% endhighlight %}

数组的序号与 C 语言类似，都是从 0 开始。

当数组创建完成之后，其大小是不能进行修改的，如果需要调整其大小，只能新建一个数组。

当作为参数传入时，会直接赋值一个数组，导致在某些场景下会比较浪费内存。

### 切片

相比数组来说，切片可以动态扩容，包括了 len 长度和 cap 容量两个属性，分别表示当前大小，以及最大容量。

{% highlight go %}
package main

import (
        "fmt"
)

func main() {
        slice := []int{1, 2, 3, 4, 5}
        //array := [5]int{1, 2, 3, 4, 5}
        //slice := array[:]
        //slice := array[1:5]
        //slice := make([]int, 5, 20)

        fmt.Printf("slice %#v\n", slice)
}
{% endhighlight %}

切片是引用类型，当作为参数传入时类似于引用同一指针，修改值将会影响其它的对象。

#### 源码

Golang 中关于 slice 的实现在 `src/runtime/slice.go` 文件中。

{% highlight go %}
type slice struct {
        array unsafe.Pointer
        len   int
        cap   int
}
{% endhighlight %}

其中，比较关键的是 `growslice()` 函数的实现，也就是在切片的容量不够的时候如何进行扩容。

如果通过 `append()` 添加元素超过了 cap 的值后，会进行扩容，而扩容之后同时需要将原有的数据复制到新的内存空间中，类似于 C 中的 `realloc()` 操作。

所以，尽量可以提前分配内存空间，避免不必要的内存申请。

## Array VS. Slice

如上所述，当通过 Array 作为入参时会复制一份数据，而对于 Slice 来说，实际上也是复制一份，只是，两者指向的实际内存空间是相同的。

{% highlight go %}
package main

import (
        "fmt"
)

func foobar(a [4]int, s []int) {
        fmt.Printf("array in func %p, %v\n", &a, a)
        fmt.Printf("slice in func %p, %v\n", &s, s)
        s[0] = 100
}

func main() {
        array := [4]int{10, 20, 30, 40}
        slice := array[:]

        fmt.Printf("array in main %p, %v\n", &array, array)
        fmt.Printf("slice in main %p, %v\n", &slice, slice)

        foobar(array, slice)
        fmt.Printf("slice after foobar %p, %v\n", &slice, slice)
}
{% endhighlight %}

最终会输出如下的内容，注意最后针对 Slice 元素的修改。

{% highlight text %}
array in main 0xc4200121a0, [10 20 30 40]
slice in main 0xc42000a060, [10 20 30 40]
array in func 0xc420012200, [10 20 30 40]
slice in func 0xc42000a0a0, [10 20 30 40]
slice after foobar 0xc42000a060, [100 20 30 40]
{% endhighlight %}

{% highlight go %}
{% endhighlight %}
