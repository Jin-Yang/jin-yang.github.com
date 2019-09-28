---
title: GoLang 闭包简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: grpc,golang
description: 在很多语言里实际上都支持闭包，例如 Python、Lua 等，GoLang 实际上也支持，功能基本类似，如果对闭包的概念比较熟悉，实际上也很好理解。这里简单介绍其概念以及常见的错误使用场景。
---

在很多语言里实际上都支持闭包，例如 Python、Lua 等，GoLang 实际上也支持，功能基本类似，如果对闭包的概念比较熟悉，实际上也很好理解。

这里简单介绍其概念以及常见的错误使用场景。

<!-- more -->

## 简介

如下是文档中的示例。

{% highlight go %}
package main

import "fmt"

func main() {
	result := getNum()
	fmt.Println(result())
}

func getNum() func() int {
	i := 0

	return func() int {
		i += 1
		return i
	}
}
{% endhighlight %}

实际上，上述是一个简化的版本，其完整定义为。

{% highlight go %}
func getNum() (func() int) {
	i := 0

	return func() int {
		i += 1
		return i
	}
}
{% endhighlight %}

也就是说，其返回的是一个函数指针，而这个函数指针内部同时含有一个变量，其中的变量的生命周期与闭包相同。

同样可以参考如下示例。

{% highlight go %}
package main

import "fmt"

func main() {
	result := getNum()
	for i := 0; i < 10; i++ {
		fmt.Println(result())
	}
}

func getNum() func() int {
	i := 0

	return func() int {
		i += 1
		return i
	}
}
{% endhighlight %}

此时会返回 `1` 到 `10` 的值，也就是说函数中的值会一直存在。

### 基本概念

简单来说，闭包是可以包含自由变量的代码块，这些变量不在这个代码块内或者任何全局上下文中定义，而是在定义代码块的环境中定义。

闭包通常用于：

* 避免添加太多的全局变量和全局函数，减少因此产生的命名冲突等，避免污染全局环境。
* 闭包给访问外部函数定义的内部变量创造了条件。

## 常见错误

GoLang 在使用全局变量要小心闭包，如下是一个简单的复现。

{% highlight go %}
package main

import (
	"fmt"
	"net/http"
)

type Logger struct{}

func (this *Logger) Debug() {
	if this == nil {
		panic("fuck")
	}
	fmt.Println("hello world")
}

var __logger *Logger

func AppLog() *Logger {
	return __logger
}

func InitConf() {
	__logger = &Logger{}
}

var logger = AppLog()

func HelloWorld(w http.ResponseWriter, r *http.Request) {
	AppLog().Debug() // ok
	logger.Debug()   // panic
}

func main() {
	InitConf()
	http.HandleFunc("/", HelloWorld)
	http.ListenAndServe(":8088", nil)
}
{% endhighlight %}

简单来说，分明已经通过 `InitConf()` 初始化过 `__logger` 了，实际上这都是由于闭包引起的，也就是 `http.HandlerFunc()` 函数，其实是闭包。

最终 `HelloWorld` 中使用的 `logger` 不是全局变量的 `logger` 而是闭包里的自由变量，在闭包生成的时候 `InitConf()` 还没被调用，所以 `__logger` 是 `nil`。

{% highlight text %}
{% endhighlight %}
