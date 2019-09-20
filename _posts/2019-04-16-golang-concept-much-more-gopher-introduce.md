---
title: GoLang Gopher
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: stan
description:
---

在人类自然语言学界有一个很著名的 "萨丕尔-沃夫" 假说，语言影响或决定人类的思维方式。

Language inuences/determines thought. - Sapir-Whorf hypothesis

那么 Gopher 的价值观是什么。

<!-- more -->

## Overall Simplicity

GoLang 的内部实现很复杂，但是其对外暴露的接口却很简单。

* 仅有的 25 个关键字，主流编程语言中最少；
* go 最简单的启动协程方式；
* gc 减少内存管理的成本；
* constants 简化常量的定义和使用；
* interface 纯方法集合，纯行为定义，隐式实现；
* package 代码组织的唯一形式。

包括代码风格，代码的写法都没有太多的选择，也就是说炫技很难，但是可读性很好。

### 变量名

通过 gofmt 工具，GoLang 提供了统一的代码风格，而变量的命名在不影响可读的情况下尽量短，例如。

{% highlight text %}
for i, v := range s
t := time.Now()
b := new(bytes.Buffer)
{% endhighlight %}

### 最小化思维

所有的代码、工具都是尽量简单。

#### 循环

只提供了一个 `for` 循环，其它语言里的 `while` `do while` 都可以通过该语句实现。

{% highlight text %}
for i := 0; i < cnt; i++ {}   // normal loop
for i, v := range f.Value {}  // iterator loop
for {}                        // dead loop
for COND {}                   // while
for {                         // do while
	if COND {
		break
	}
}
{% endhighlight %}

#### 常量

常量只是数字，可以是整型或者浮点型，无需显示指定类型 (例如U UL)，会自动判断。

{% highlight text %}
const Pi = 3.1415
const (
	MaxFrameSize = 1 << 14
	MinFrameSize = 0
)
seconds := time.Nanoseconds()/1e9  // int64
{% endhighlight %}

#### 错误处理

## Preference in Concurrency

通过 interface 决定了程序的静态结构，而并发则影响着程序的运行。Go 提供了三种并发原语：

* 通过协程并发，内部实现了调度单元，对外暴露的却是同步接口；
* 使用管道进行通讯，利用管道将多个协程组合到一起；
* select使得协程可以处理多个管道操作。

在设计程序时，需要识别分解出独立的计算单元，并添加到协程中执行，各个协程之间通过管道和 select 建立联系。

下面从各个协程之间的联系，介绍一些常见的模式。

### Detached

此时与父进程没有关系，一般其生命周期与进程相同，会执行一些特定的任务，例如监控。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        go func() {
                ticker := time.NewTicker(time.Second * 1)
                for range ticker.C {
                        fmt.Println(time.Now().Format("2006-01-02 15:04:05"))
                }
        }()

        time.Sleep(time.Duration(3) * time.Second)
}
{% endhighlight %}

其实现通常采用 `for select` 方式，然后通过 timer 或者 event 驱动。

### Parent Child

父进程需要等待子进程的退出，并做一些资源的清理。

如果只需要等待一个 child ，可以使用如下方式。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func HandleMessage(msgCh <-chan string, quit <-chan bool) {
        for {
                select {
                case m := <-msgCh:
                        fmt.Println(m)
                case <-quit:
                        fmt.Println("Quit now")
                }
        }
}

func main() {
        quit := make(chan bool)
        msgCh := make(chan string)

        go HandleMessage(msgCh, quit)
        for i := 0; i < 3; i++ {
                msgCh <- "Hello World"
        }
        quit <- true // close(quit)
        time.Sleep(time.Duration(100) * time.Microsecond)
}
{% endhighlight %}

如果是多个 Children ，那么可以通过关闭管道的方式退出，此时所有的 `case <-quit` 都会收到 false (实际是零值) 。

当需要获取协程的退出状态时，就需要另外的管道进行通讯。

<!--
https://tonybai.com/2017/04/20/go-coding-in-go-way/
-->

{% highlight text %}
{% endhighlight %}
