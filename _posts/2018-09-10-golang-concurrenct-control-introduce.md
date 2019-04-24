---
title: GoLang 并发控制
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: grpc,golang
description:
---


<!-- more -->

## 并发控制

控制并发有两种经典的方式：`WaitGroup` 和 `Context` 。

### WaitGroup

一种控制并发的方式，它的这种方式是控制多个 goroutine 同时完成，有点类似于 `waitpid()` 函数，用于多个协程同步，通过 `Add()` 增加，`Done()` 减小，`wait()` 会等待到 0 。

{% highlight go %}
package main

import (
        "fmt"
        "sync"
        "time"
)

func main() {
        var wg sync.WaitGroup
        wg.Add(2)
        go func() {
                time.Sleep(1 * time.Second)
                fmt.Println("No. #1 Done")
                wg.Done()
        }()
        go func() {
                time.Sleep(2 * time.Second)
                fmt.Println("No. #2 Done")
                wg.Done()
        }()
        wg.Wait()
        fmt.Println("OK, All Done")
}
{% endhighlight %}

### Channel

简单来说，就是通过 `Channel` 和 `Select` 这种方式进行同步。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        stop := make(chan bool)
        go func() {
                for {
                        select {
                        case <-stop:
                                fmt.Println("Quit now")
                                return
                        default:
                                fmt.Println("Running")
                                time.Sleep(1 * time.Second)
                        }
                }
        }()
        time.Sleep(5 * time.Second)
        fmt.Println("Time up...")
        stop <- true
        fmt.Println("Bye Bye")
}
{% endhighlight %}

如上的例子中，通过预先定义的一个 chan 来通知后台的协程。

这种方式比较适合与一些可预期的简单场景，例如有多个协程需要控制、协程又派生了子协程。

### Context

对于上述的场景，常见的是 HTTP 请求，每个 Request 都需要开启一个协程，同时这些协程又有可能派生其它的协程，例如处理身份认证、Token校验等；Context 就是提供了一种协程的跟踪方案。

对于 go1.6 及之前版本使用 `golang.org/x/net/context` 而在 1.7 版本之后已移到标准库 context 。

#### 简介

Context 的调用应该是链式的，通过 WithCancel、WithDeadline、WithTimeout 或 WithValue 派生出新的 Context，当父 Context 被取消时，其派生的所有 Context 都将取消。

上述 WithXXX 返回新的 Context 和 CancelFunc，调用 CancelFunc 将取消子代，移除父代对子代的引用，并且停止所有定时器；未调用 CancelFunc 将泄漏子代，直到父代被取消或定时器触发。

<!--
注意，一般来说，在使用时需要遵循以下规则：

1. 不要将 Contexts 放入结构体，相反应该作为第一个参数传入，命名为 ctx；
2. 即使函数允许，也不要传入 nil 的 Context；如果不知道用哪种 Context，可以使用 context.TODO()；
使用context的Value相关方法只应该用于在程序和接口中传递的和请求相关的元数据，不要用它来传递一些可选的参数
相同的 Context 可以传递给在不同的goroutine；Context 是并发安全的。
-->

`context.Background()/TODO()` 会返回一个空 Context，一般将该 Context 作为整个 Context 树的根节点，然后调用 withXXX 函数创建可取消的子 Context，并作为参数传递给其它的协程，从而实现跟踪该协程的目的。

如下是一个示例。

{% highlight go %}
package main

import (
        "context"
        "fmt"
        "time"
)

func main() {
        ctx, cancel := context.WithCancel(context.Background())
        go watch(ctx, "CTX1")
        go watch(ctx, "CTX2")
        go watch(ctx, "CTX3")
        time.Sleep(5 * time.Second)
        fmt.Println("Time up...")
        cancel()
        time.Sleep(1 * time.Second)
}

func watch(ctx context.Context, name string) {
        for {
                select {
                case <-ctx.Done():
                        fmt.Println(name, "Quit now")
                        return
                default:
                        fmt.Println(name, "Running")
                        time.Sleep(1 * time.Second)
                }
        }
}
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
