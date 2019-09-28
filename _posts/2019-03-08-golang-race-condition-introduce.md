---
title: Golang 竞态检查
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: golang,data race
description: Race Conditions 是最隐晦而且难以捉摸的编程错误之一，一般会在代码部署到生产之后很长时间才会发作，但是如果要通过 Golang 进行并发编程，那么就会很容易遇到。Go 的并发机制使得编写干净并发代码变得容易，但它们并不能防止竞态条件。
---

Race Conditions 是最隐晦而且难以捉摸的编程错误之一，一般会在代码部署到生产之后很长时间才会发作，但是如果要通过 Golang 进行并发编程，那么就会很容易遇到。

Go 的并发机制使得编写干净并发代码变得容易，但它们并不能防止竞态条件。

<!-- more -->

## 示例

当两个协程同时访问相同的变量并且访问中的至少一个是写入时，就会发生数据竞争。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        a := 1
        go func() {
                a = 2
        }()
        fmt.Println("a is ", a)

        time.Sleep(1 * time.Second)
}
{% endhighlight %}

这里 main 和另一个匿名协程，都会访问 a 变量，并且有对 a 的写操作，所以会触发竞态条件。

## 使用方式

在 Golang 中自带的 race 的检测，在命令行中，可以使用 `-race` 参数。

{% highlight text %}
$ go test -race mypkg    
$ go run -race mysrc.go 
$ go build -race mycmd   
$ go install -race mypkg 
{% endhighlight %}

### 环境变量

另外，还可以使用环境变量 `GORACE` 来设置一些参数，其格式为：

{% highlight text %}
GORACE="option1=val1 option2=val2"
{% endhighlight %}

可选的参数有：

* `log_path` 输出结果的写入路径，默认 stderr ；
* `exitcode` 在检测到race后退出时的状态码，默认为 66 ；
* `strip_path_prefix` 输出结果中去掉文件夹前缀，这样更简洁，默认空字符串；
* `history_size` 每个协程的访问记录保存大小，可设置为 `32K * 2 ** history_size`，默认是 1 ；
* `halt_on_error` 是否程序在触发了第一次 data race 就退出，默认是 0 ；

增加 `history_size` 大小可以避免出现 `failed to restore the stack` 的错误，但是代价是增加内存使用量。

{% highlight text %}
$ GORACE="strip_path_prefix=/tmp/test/" go run -race
a is  1
==================
WARNING: DATA RACE
Write at 0x00c4200a4010 by goroutine 6:
  main.main.func1()
      main.go:11 +0x38

Previous read at 0x00c4200a4010 by main goroutine:
  main.main()
      main.go:13 +0x88

Goroutine 6 (running) created at:
  main.main()
      main.go:10 +0x7a
==================
Found 1 data race(s)
exit status 66
{% endhighlight %}

从报告可以看到一处 data race，在 `main.go:11` 处有协程 #6 对 `0x00c4200a4010` 的写入，在 `main.go:13` 处有 `main goroutine` 对同一变量 `0x00c4200a4010` 的读取操作。

## 规避手段

遇到 race 后的解决办法一般来说有三种：channels、锁和原子操作。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        a := 1
        done := make(chan int, 1)
        go func() {
                a = 2
                done <- 1
        }()
        <-done
        fmt.Println("a is ", a)

        time.Sleep(1 * time.Second)
}
{% endhighlight %}

上述是通过管道来保证数据同步。

## 参考

* [Introducing the Go Race Detector](https://blog.golang.org/race-detector)
* [Thread Sanitizer Algorithm Introduce](https://github.com/google/sanitizers/wiki/ThreadSanitizerAlgorithm)

<!--
https://www.slideshare.net/InfoQ/looking-inside-a-race-detector
https://benjaminogles.github.io/blog/how-thread-sanitizer-works/
https://www.infoq.com/presentations/go-race-detector/?utm_source=infoq&utm_medium=slideshare&utm_campaign=slidesharesf
-->

{% highlight text %}
{% endhighlight %}
