---
title: Golang 调试方法
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: golang,data race
description:
---


<!-- more -->

## 调用栈

通常来说为了看一个偶现的问题可以多打印些日志，然后等待复现。不过实际上可以通过 GoLang 异常时打印的堆栈查看问题出现的部分现场。

在如下的代码中，会在 `Example()` 函数中打印具体的错误栈信息。

{% highlight go %}
package main

func main() {
        slice := make([]string, 2, 4)
        Example(slice, "hello", 10)
}

func Example(slice []string, str string, i int) {
        panic("Want stack trace")
}
{% endhighlight %}

其中打印的栈信息如下。

{% highlight text %}
panic: Want stack trace

goroutine 1 [running]:          协程1出的问题
main.Example(0xc420047f38, 0x2, 0x4, 0x46b0e2, 0x5, 0xa)  那个函数出的问题
        /tmp/foobar.go:9 +0x39  具体代码位置
main.main()
        /tmp/foobar.go:5 +0x72
exit status 2
{% endhighlight %}

接下来看下起打印的入参，对应的声明以及调用栈如下。

{% highlight text %}
// Declaration
main.Example(slice []string, str string, i int)

// Call to Example by main.
slice := make([]string, 2, 4)
Example(slice, "hello", 10)

// Stack trace
main.Example(0x2080c3f50, 0x2, 0x4, 0x425c0, 0x5, 0xa)
{% endhighlight %}

可以看到打印的入参是 6 个，而实际函数中是 3 个。这跟 GoLang 中对象的表示方法相关，其中 Slice 包括了地址、长度、容量；String 包括了地址、长度；Int 单个数字。

## 方法

也就是将方法进行封装。

{% highlight go %}
package main

import "fmt"

type trace struct{}

func main() {
        slice := make([]string, 2, 4)

        var t trace
        t.Example(slice, "hello", 10)
}

func (t *trace) Example(slice []string, str string, i int) {
        fmt.Printf("Receiver Address: %p\n", t)
        panic("Want stack trace")
}
{% endhighlight %}

因为方法声明的是一个指针，在调用方法时，会传入该方法的对象指针，那么最终打印的栈为。

{% highlight text %}
Receiver Address: 0x546fa8
panic: Want stack trace

goroutine 1 [running]:
main.(*trace).Example(0x546fa8, 0xc420047f38, 0x2, 0x4, 0x4b463a, 0x5, 0xa)
        /tmp/foobar.go:16 +0x8c
main.main()
        /tmp/foobar.go:11 +0x93
exit status 2
{% endhighlight %}

<!--
https://www.ardanlabs.com/blog/2015/01/stack-traces-in-go.html
-->

{% highlight text %}
{% endhighlight %}
