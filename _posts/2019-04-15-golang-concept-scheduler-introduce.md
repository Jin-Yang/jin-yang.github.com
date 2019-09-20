---
title: GoLang 调度机制
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: stan
description:
---

在 Go 语言中的核心是协程，其实现在用户态，那么就需要在用户态实现其调度器，在 1.1 版本对调度器进行了重构，也就是现在的 GMP 模型。

另外，为了防止协程不让出 CPU 导致其它协程饿死，在 1.2 版本加入了强占式调度器。

<!-- more -->

## 简介

相比线程来说，协程具有更低的内存栈占用 (2KB/8KB) ，上下文切换更快 (10ns/Nus) 。

其核心是 GMP 的调度关系。

* G Goroutine 协程，也就是代码中通过 go 关键词创建的对象；
* P Processor 逻辑处理器，会保存执行的上下文；
* M Work Thread/Machine 工作线程，实际 OS 调度执行的单元，无上下文。

内核通过线程 (或进程) 实现并发和并行，而对于 Go 来说，通过线程做并行，而使用协程做并发，协程可能会被调度到其它线程上执行。

代码的实现在 `$GOROOT/src/runtime/proc.go` 中实现。

<!--
findrunnable() 选择需要运行的协程
-->


### 调度流程

在 M 和 P 绑定了之后，M 会不断从 P 中取出 G 并运行，如果 P 的本地没有则从全局获取，当全局也没有的时候就从其它的 P 中 Steal 部分 G 来执行。

<!--
包含了源码的部分解析
https://wudaijun.com/2018/01/go-scheduler/
很多的调度示例
https://segmentfault.com/a/1190000018775901
https://www.jianshu.com/p/3071738503ee
https://colobu.com/2017/05/04/golang-runtime-scheduler/
https://tonybai.com/2017/06/23/an-intro-about-goroutine-scheduler/
https://www.jianshu.com/p/1288b8fec84d
-->


## 调试

### trace

提供了一个 trace 工具可以查看协程的具体调度方式。

{% highlight go %}
package main

import (
        "fmt"
        "os"
        "runtime/trace"
)

func main() {
        f, err := os.Create("trace.out")
        if err != nil {
                panic(err)
        }
        defer f.Close()

        if err = trace.Start(f); err != nil {
                panic(err)
        }
        defer trace.Stop()

        fmt.Println("Hello trace")
}
{% endhighlight %}

然后运行，解析输出的结果。

{% highlight text %}
$ go run trace.go
$ go tool trace trace.out
{% endhighlight %}

通过浏览器可以查看输出的结果。

<!--
https://making.pusher.com/go-tool-trace/
https://mp.weixin.qq.com/s/nf_-AH_LeBN3913Pt6CzQQ
-->

### Debug Trace

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        for i := 0; i < 5; i++ {
                time.Sleep(time.Second)
                fmt.Println("Hello scheduler")
        }
}
{% endhighlight %}

然后通过如下命令运行。

{% highlight text %}
$ GODEBUG=schedtrace=1000 go run main.go
{% endhighlight %}

其中的 `schedtrace=1000` 多久打印一次调度器的信息，单位是 ms ，其输出类似如下。

{% highlight text %}
SCHED 0ms: gomaxprocs=8 idleprocs=5 threads=5 spinningthreads=1 idlethreads=0 runqueue=0 [0 0 0 0 0 0 0 0]
SCHED 1001ms: gomaxprocs=8 idleprocs=8 threads=5 spinningthreads=0 idlethreads=3 runqueue=0 [0 0 0 0 0 0 0 0]
Hello scheduler
SCHED 2002ms: gomaxprocs=8 idleprocs=8 threads=5 spinningthreads=0 idlethreads=3 runqueue=0 [0 0 0 0 0 0 0 0]
Hello scheduler
SCHED 3004ms: gomaxprocs=8 idleprocs=8 threads=5 spinningthreads=0 idlethreads=3 runqueue=0 [0 0 0 0 0 0 0 0]
Hello scheduler
SCHED 4005ms: gomaxprocs=8 idleprocs=8 threads=5 spinningthreads=0 idlethreads=3 runqueue=0 [0 0 0 0 0 0 0 0]
Hello scheduler
SCHED 5013ms: gomaxprocs=8 idleprocs=8 threads=5 spinningthreads=0 idlethreads=3 runqueue=0 [0 0 0 0 0 0 0 0]
Hello scheduler
{% endhighlight %}

输出的信息如下。

* SCHED 标示是调度器的输出信息。
* 0ms 从程序启动到输出这行日志的时间；
* gomaxprocs P的数量，本例有8个P，默认是CPU逻辑核数；
* idleprocs 处于idle状态的P的数量，与上述值的差就是正在执行的P数量；
* threads M的数量，包含了调度器使用M的数量以及sysmon的使用线程；
* spinningthreads 处于自旋状态的M数量；
* idlethread 处于idle状态的M数量；
* runqueue=0 全局调度队列中G的数量；
* 接下来对应了 8 个 local queue 中G的数量。

另外，在加上 `scheddetail=1` 可以看到更详细的调度信息。

{% highlight text %}
$ GODEBUG=schedtrace=1000,scheddetail=1 go run main.go
{% endhighlight %}

<!--
https://tonybai.com/2017/06/23/an-intro-about-goroutine-scheduler/
-->

## 参考

* [The Go Scheduler](https://morsmachine.dk/go-scheduler) 关于 Go 调度算法的基本介绍。


{% highlight text %}
{% endhighlight %}
