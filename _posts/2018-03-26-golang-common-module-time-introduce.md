---
title: GoLang time 模块
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,time,module
description: GoLang 中的 time 模块包含了一些与时间相关的函数，例如格式转换、定时器等。
---

GoLang 中的 time 模块包含了一些与时间相关的函数，例如格式转换、定时器等。

<!-- more -->

## 简介

golang 通过 time 包的函数初始化定时器，可以通过 `reset()`、`stop()` 重置和停止定时器。另外，定时器存在一个 `chan time.Time` 类型的缓冲 channel 字段 C，时间到了之后，定时器就会向自己的 C 字段发送一个 `time.Time` 类型的元素值。

### 时间间隔

在 time 模块中，定义了一个 `time.Duration` 类型，实际上就是一个 `int64` 类型，同时定义了一些常用的宏定义。

{% highlight go %}
type Duration int64
const (
	Nanosecond  Duration = 1
	Microsecond          = 1000 * Nanosecond
	Millisecond          = 1000 * Microsecond
	Second               = 1000 * Millisecond
	Minute               = 60 * Second
	Hour                 = 60 * Minute
)
{% endhighlight %}

## 定时器

最简单的定时器。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        t := time.NewTimer(2 * time.Second)

        now := time.Now()
        fmt.Printf("Now time : %v.\n", now)

        expire := <-t.C
        fmt.Printf("Expiration time: %v.\n", expire)
}
{% endhighlight %}

## Ticker

周期性的触发时间事件，会以指定的时间间隔重复的向通道 C 发送时间值。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        var ticker *time.Ticker = time.NewTicker(1 * time.Second)

        go func() {
                for t := range ticker.C {
                        fmt.Println("Tick at", t)
                }
        }()

        time.Sleep(time.Second * 5)
        ticker.Stop()
        fmt.Println("Ticker stopped")
}
{% endhighlight %}

如果使用周期性的定时器，可以使用 `time.Ticker` 类型。

<!--
package main

import (
        "fmt"
        "time"
)

func main() {
        t := time.NewTicker(1 * time.Second)
        defer t.Stop()

        for {
                select {
                case <-t.C:
                        fmt.Println(time.Now())
                }
        }
}
-->

## 其它

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        time.AfterFunc(
                1*time.Second,
                func() {
                        fmt.Printf("Expire: %v.\n", time.Now())
                })
        time.Sleep(2 * time.Second)
}
{% endhighlight %}

可以将定时器字段 C 的大小输出，可以发现并没有缓冲任何值。也就是说，在给定了自定义函数后，默认的处理方法 (向C发送代表绝对到期时间的元素值) 就不会被执行了。


{% highlight go %}
{% endhighlight %}
