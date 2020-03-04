---
title: GoLang 逃逸分析
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## 引子

有一个非常诡异的现象，通过修改一行的输出，会影响到切片值的保存。

{% highlight go %}
package main

import (
        "fmt"
)

func main() {
        s := []byte("")

        s1 := append(s, 'a')
        s2 := append(s, 'b')

        //fmt.Println(s1, "===", s2)          // <1>
        fmt.Println(string(s1), string(s2)) // <2>
}
{% endhighlight %}

如果有 `<1>` 中的代码，那么在 `<2>` 中打印的结果是 `a b` ，否则打印的结果是 `b b` 。

### 分析

Slice 是建立在 Array 之上的，Array 主要包含了类型及其大小两个属性，对应的变量是一个值，而非指针，也就意味着在函数传参的时候会复制一份数组。

其中变量 `s` 是一个 slice 结构，其定义在 `runtime/slice.go` 中，如下。

{% highlight text %}
type slice struct {
        array unsafe.Pointer
        len   int
        cap   int
}
{% endhighlight %}

有如下几个场景。

{% highlight text %}
var slice []int            // 申请的空变量
slice.array = nil          // 没有指向
slice.len = 0
slice.cap = 0

slice := []int{}           // 申请了大小为0的空间
slice := make([]int, 0)
slice.array = 0xc000044698 // 指向了申请的内存空间
slice.len = 0
slice.cap = 0
{% endhighlight %}


{% highlight text %}
package main

import (
        "fmt"
)

func main() {
        s := []byte{'W', 'O', 'R', 'L', 'D'}
        s1 := s[2:4]
        fmt.Println(string(s))
        s1[0] = 'r'
        fmt.Println(string(s))
}
{% endhighlight %}

新建一个 `Slice` 的时候不会复制数组，而是指向数组，这也就意味着，同时会修改数据中的另外一部分。

## 增长

当访问超过了 `cap` 大小的元素时，会引发 `panic` ，那么如果想扩展其大小时应该如何处理。

如果要增加 Slice 的大小，需要新增一个，并将数据复制过去，也可以使用 append() 函数，在 [Effective Go](https://golang.org/doc/effective_go.html#slices) 中有该函数相关的处理逻辑，简单来说，当超过了容量后，会自动进行扩容。

{% highlight text %}
func Append(slice, data []byte) []byte {
    l := len(slice)
    if l + len(data) > cap(slice) {  // reallocate
        // Allocate double what's needed, for future growth.
        newSlice := make([]byte, (l+len(data))*2)
        // The copy function is predeclared and works for any slice type.
        copy(newSlice, slice)
        slice = newSlice
    }
    slice = slice[0:l+len(data)]
    copy(slice[l:], data)
    return slice
}
{% endhighlight %}

<!--
Go Slices: usage and internals
https://blog.golang.org/go-slices-usage-and-internals
-->



{% highlight text %}
{% endhighlight %}
