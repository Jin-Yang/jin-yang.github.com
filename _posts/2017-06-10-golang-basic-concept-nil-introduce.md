---
title: Golang nil 解析
layout: post
comments: true
language: chinese
category: [program]
keywords:
description:
---

<!-- more -->

## 简介

一般来说，无论是直接声明一个变量，或者通过 `new()` `make()` 创建，默认都会将对应的值设置为 0 ，可以参考 [Zero Value](https://golang.org/ref/spec#The_zero_value) ，也就是 `false` `0` `""` `nil` 等，每个类型对应的零值如下。

{% highlight text %}
bool       -> false
numbers    -> 0
string     -> ""

pointers   -> nil
slices     -> nil
maps       -> nil
channels   -> nil
functions  -> nil
interfaces -> nil
{% endhighlight %}

在 go 语言中，nil 是一个比较特殊的变量，其定义在 `builtin/builtin.go` 文件中，定义如下。

{% highlight go %}
// nil is a predeclared identifier representing the zero value for a
// pointer, channel, func, interface, map, or slice type.
var nil Type // Type must be a pointer, channel, func, interface, map, or slice type
{% endhighlight %}

也即 `nil` 并不是一个关键字，可将 `nil` 定义成其它，例如 `var nil = errors.New("hi")` ，只是不建议这么做。

另外，需要注意，结构体 `struct` 的零值不是 nil ，因为 struct 的零值跟其属性相关。

## 类型

`nil` 没有默认的类型，尽管它可以是多个类型的零值，此时必须显式或隐式指定每个 `nil` 用法的明确类型。

{% highlight go %}
package main

func main() {
	_ = (*struct{})(nil)
	_ = []int(nil)
	_ = map[int]bool(nil)
	_ = chan string(nil)
	_ = (func())(nil)
	_ = interface{}(nil)
	
	var _ *struct{} = nil
	var _ []int = nil
	var _ map[int]bool = nil
	var _ chan string = nil
	var _ func() = nil
	var _ interface{} = nil
}
{% endhighlight %}

其实 nil 是一个预定义的变量，而不是关键字，这样就意味着，你可以直接自己定义一个 nil 变量将预定义的变量覆盖。

{% highlight go %}
package main

import "fmt"

func main() {
    nil := 123
    fmt.Println(nil) // 123
    var _ map[string]int = nil // error
}
{% endhighlight %}

虽然是预定义的变量，但是它没有具体的类型，不像 `iota` 定义为 `int` 类型。

## 地址

因为 `nil` 实际上是预定义的变量，那么其对应的地址是相同的。

{% highlight go %}
package main

import (
    "fmt"
)

func main() {
	var m map[int]string
	var ptr *int
	var sl []int
	fmt.Printf("%p\n", m)       //0x0
	fmt.Printf("%p\n", ptr )    //0x0
	fmt.Printf("%p\n", sl )     //0x0
}
{% endhighlight %}

## 大小

不同的 nil 值，其大小是不同的。

{% highlight go %}
package main

import (
	"fmt"
	"unsafe"
)

func main() {
	var p *struct{} = nil
	fmt.Println( unsafe.Sizeof( p ) ) // 8

	var s []int = nil
	fmt.Println( unsafe.Sizeof( s ) ) // 24

	var m map[int]bool = nil
	fmt.Println( unsafe.Sizeof( m ) ) // 8

	var c chan string = nil
	fmt.Println( unsafe.Sizeof( c ) ) // 8

	var f func() = nil
	fmt.Println( unsafe.Sizeof( f ) ) // 8

	var i interface{} = nil
	fmt.Println( unsafe.Sizeof( i ) ) // 16
}
{% endhighlight %}

<!--
https://go101.org/article/nil.html
https://studygolang.com/articles/11535
-->

{% highlight go %}
{% endhighlight %}
