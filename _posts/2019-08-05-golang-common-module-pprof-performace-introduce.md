---
title: GoLang pprof 模块
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: stan
description:
---

GoLang 提供了非常强大的代码性能分析工具 pprof，可以非常方便的分析代码运行性能，同时非常直观。

<!-- more -->

## 简介

监控代码性能的有两个包 `net/http/pprof` 以及 `runtime/pprof` ，其中前者是以 HTTP 的方式将数据暴露出来，实际上其内部封装的仍然是 `runtime/pprof` 。

另外，在 `go test` 工具中，同时也封装了 `runtime/pprof` 这个包，可以通过类似 `-cpuprofile=cpu.prof` `-memprofile=mem.prof` 的参数指定。

使用过程分成了两步，首先需要运行代码收集相关的性能数据，然后通过相关的工具对数据进行分析，查找程序运行的瓶颈点。

### runtime/pprof

对于 `runtime/pprof` 这个库，在 [runtime/pprof](https://golang.org/pkg/runtime/pprof/) 中展示了相关的使用方式，这里简单借助斐波纳契数列介绍其使用方法。

package main

import (
        "flag"
        "fmt"
        "log"
        "os"
        "runtime/pprof"
)

var (
        cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file.")
)

func fibonacci(num int) int {
        if num < 2 {
                return 1
        }
        return fibonacci(num-1) + fibonacci(num-2)
}

func main() {
        flag.Parse()
        if *cpuprofile != "" {
                f, err := os.Create(*cpuprofile)
                if err != nil {
                        log.Fatal("could not create CPU profile: ", err)
                }
                defer f.Close()

                if err := pprof.StartCPUProfile(f); err != nil {
                        log.Fatal("could not start CPU profile: ", err)
                }
                defer pprof.StopCPUProfile()
        }

        for i := 0; i < 30; i++ {
                nums := fibonacci(i)
                fmt.Println(nums)
        }
}

在编译完上述的代码之后，可以通过 `--cpuprofile` 参数指定输出的 CPU 性能采集文件。

$ go build fibonacci.go
$ ./fibonacci --cpuprofile=fibonacci.prof
$ go tool pprof fibonacci fibonacci.prof
File: fibonacci
Type: cpu
Time: Apr 1, 2020 at 11:32am (CST)
Duration: 200.49ms, Total samples = 10ms ( 4.99%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top  # 也可以使用topN命令，N表示显示多少个top信息
Showing nodes accounting for 10ms, 100% of 10ms total
      flat  flat%   sum%        cum   cum%
      10ms   100%   100%       10ms   100%  main.fibonacci
         0     0%   100%       10ms   100%  main.main
         0     0%   100%       10ms   100%  runtime.main
(pprof) list fibonacci # 查看函数采样

### net/http/pprof

https://pdf.us/2019/02/18/2772.html

## 内存泄漏排查

在 GoLang 引入了协程之后，在有效提高性能的同时，因为各个协程之间是软限制的协作关系，导致了协程泄漏的风险，而且大部分 Go 中的内存泄漏都是因为协程的泄漏引起的，因为对于常规的对象都可以通过 gc 进行回收。

## 内存使用检查

通过 `go tool pprof http://localhost:6060/debug/pprof/heap` 直接访问栈内存信息。

### top

默认按指标大小列出前 10 个函数，比如内存是按内存占用多少，CPU 是按执行时间多少。

(pprof) top
Showing nodes accounting for 520.61MB, 99.90% of 521.11MB total
Dropped 9 nodes (cum <= 2.61MB)
      flat  flat%   sum%        cum   cum%
  520.61MB 99.90% 99.90%   520.61MB 99.90%  main.main
         0     0% 99.90%   520.61MB 99.90%  runtime.main

默认 `top` 命令会列出前 5 个统计数据，各个列的含义如下：

flat   本函数占用的内存量。
flat%  本函数内存占使用中内存总量的百分比。
sum%   前面每一行flat百分比的和，比如第2行的99.90%其实是99.90%+0%。
cum    累计量，假设main函数调用了foobar函数，那么函数foobar的内存占用量也会被统计进来。
cum%   累计量占总量的百分比。

### list

确认某个函数中资源的开销，可以具体到某一行的数据，如果函数名不明确，会进行模糊匹配，比如 `list main` 会列出 `main.main` 和 `runtime.main` 。

(pprof) list main.main
Total: 521.11MB
ROUTINE ======================== main.main in /tmp/foobar/main.go
  520.61MB   520.61MB (flat, cum) 99.90% of Total
         .          .     18:   }()
         .          .     19:
         .          .     20:   tick := time.Tick(time.Second / 100)
         .          .     21:   var buf []byte
         .          .     22:   for range tick {
  520.61MB   520.61MB     23:           buf = append(buf, make([]byte, 1024*1024)...)
         .          .     24:   }
         .          .     25:}

## 参考

[Profiling Go Programs](https://blog.golang.org/pprof)

[High Performance Go Workshop](https://dave.cheney.net/high-performance-go-workshop/dotgo-paris.html) 绝对是经典。

关于单核 CPU 的性能
https://preshing.com/20120208/a-look-back-at-single-threaded-cpu-performance/
https://juejin.im/post/5dca56ff518825575a358e9e

实战Go内存泄露
https://segmentfault.com/a/1190000019222661
https://xargin.com/pprof-and-flamegraph/
深入golang runtime的调度
https://zboya.github.io/post/go_scheduler/

可以在单元测试中添加goleak检测
https://github.com/uber-go/goleak
https://github.com/google/gops

## 防止内存泄漏的原则
https://golangnote.com/topic/222.html
https://blog.haohtml.com/archives/19308

{% highlight text %}
{% endhighlight %}
