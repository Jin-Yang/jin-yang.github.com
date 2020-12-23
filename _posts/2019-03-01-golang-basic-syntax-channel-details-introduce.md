---
title: GoLang 管道详解
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: 
description:
---

在 Go 语言中，通过协程和管道实现了 Communicating Sequential Processes, CSP 模型，两者承担了通信和同步中的重要角色。

<!-- more -->

## 简介

通过 Channel 提供了如下的语义：

* 协程安全，需要遵从 Go 的内存模型；
* 在不同的协程之间发送消息，提供 FIFO 语义；
* 可以让协程阻塞、非阻塞。

### 基本示例

根据管道的数据流方向，管道的类型大致可以分成三种：A) 从管道接收 `<-chan int` ；B) 发送到管道 `chan<- int` ；C) 双向，也就是不指定任意方向。

一个简单的示例，由三个协程完成，依次完成类似流处理的步骤，包括了：A) 顺序生成数据；B) 计算其平方值；C) 输出到终端打印。

{% highlight go %}
package main

import (
        "fmt"
)

func counter(out chan<- int) {
        for x := 0; x < 10; x++ {
                out <- x
        }
        close(out)
}

func squarer(out chan<- int, in <-chan int) {
        for v := range in {
                out <- v * v
        }
        close(out)
}

func printer(in <-chan int) {
        for v := range in {
                fmt.Println(v)
        }
}

func main() {
        cntChan := make(chan int)
        sqrChan := make(chan int)

        go counter(cntChan)
        go squarer(sqrChan, cntChan)

        printer(sqrChan)
}
{% endhighlight %}

上述管道，在声明时是一个双向管道，也就是为了可以通过生产者写入，然后再由消费者进行消费。为了防止乱用，可以在向函数传参的时候将管道修改为 **单向**，这样对于接收管道来说是不允许关闭，可以防止误操作。

### Tips #1

判断管道是否关闭。

如果将 `squarer()` 函数替换为如下，实际上是有问题的。

{% highlight go %}
func squarer(out chan<- int, in <-chan int) {
        for {
                v := <-in
                out <- v * v
        }
        close(out)
}
{% endhighlight %}

当 `in` 管道关闭之后，通过 `v := <-in` 读取到的数据会始终为 `0` ，那么就会一直在计算输出 `0` ，而实际上已经关闭。可以通过 `v, ok := <-in` 在获取数据的同时判断管道是否已经关闭。

{% highlight go %}
func squarer(out chan<- int, in <-chan int) {
        for {
                v, ok := <-in
                if !ok {
                        break
                }
                out <- v * v
        }
        close(out)
}
{% endhighlight %}

而 `range` 可以自动判断管道是否关闭。

### Tips #2

如果一个管道已经关闭，继续发送数据会导致系统 `panic` ，例如，假设 `squarer()` 只会消费三个数据，并关闭写入端，也就是函数修改如下。

{% highlight go %}
func squarer(out chan<- int, in chan int) {
        cnt := 0
        for v := range in {
                out <- v * v
                cnt++
                if cnt == 3 {
                        close(in)
                        break
                }
        }
}
{% endhighlight %}

此时当 `counter()` 写入时就说 `panic` 异常。

到目前为止，还有找到在写入时如何判断管道是否关闭，估计除了通过 `defer` 捕获异常之外没有太好的办法，最好还是从设计模式上就直接规避掉。

## 使用

<!--
http://colobu.com/2016/04/14/Golang-Channels/
-->

核心类型，可以看做是一个 FIFO 的阻塞消息队列，用于发核心单元之间发送和接收数据，默认是双向的，也可以指定方向，使用前必须要创建并初始化，

{% highlight go %}
package main

import "fmt"

func sum(s []int, c chan int) {
	sum := 0
	for _, v := range s {
		sum += v
	}
	c <- sum // send sum to c
}

func main() {
	s := []int{7, 2, 8, -9, 4, 0}

	c := make(chan int)

	go sum(s[:len(s)/2], c)
	go sum(s[len(s)/2:], c)
	x, y := <-c, <-c // receive from c

	fmt.Println(x, y, x+y)
}
{% endhighlight %}

如上中的 `make(chan int, 100)` ，上述的第二个参数可选，表示容量，也就是管道可以容纳的最多元素数量，代表管道的缓存大小。

大小默认是 0 ，也就是如果接收、发送没有准备好，另外一端就会阻塞，如果设置了缓存，只有 buffer 满了后 send 才会阻塞，当缓存空了后 receive 才会阻塞。

<!--
if-else语句，比较奇葩的是
https://gobyexample.com/if-else

Select语句
https://segmentfault.com/a/1190000006815341
-->

简单示例如下。

{% highlight go %}
package main

import "time"
import "fmt"

func main() {
	// For our example we'll select across two channels.
	c1 := make(chan string)
	c2 := make(chan string)

	// Each channel will receive a value after some amount
	// of time, to simulate e.g. blocking RPC operations
	// executing in concurrent goroutines.
	go func() {
		time.Sleep(1 * time.Second)
		c1 <- "one"
	}()
	go func() {
		time.Sleep(2 * time.Second)
		c2 <- "two"
	}()

	// We'll use `select` to await both of these values
	// simultaneously, printing each one as it arrives.
	for i := 0; i < 2; i++ {
		select {
		case msg1 := <-c1:
			fmt.Println("received", msg1)
		case msg2 := <-c2:
			fmt.Println("received", msg2)
		}
	}
}
{% endhighlight %}

### Select 行为

可以将每个 `select` 语句理解为一个事件，后面的语句表示对事件的处理。

{% highlight go %}
// https://talks.golang.org/2012/concurrency.slide#32
select {
case v1 := <-c1:
	fmt.Printf("received %v from c1\n", v1)
case v2 := <-c2:
	fmt.Printf("received %v from c2\n", v1)
case c3 <- 23:
	fmt.Printf("sent %v to c3\n", 23)
default:
	fmt.Printf("no one was ready to communicate\n")
}
{% endhighlight %}

上述代码中包含了三个 case 子句以及一个 default 子句，前两个是 receive 操作，第三个是 send 操作，最后一个是默认操作。

当代码执行到 select 时，case 语句会按照源代码的顺序被评估，且只评估一次，评估的结果会出现下面这几种情况：

* 除 default 外，如果只有一个 case 语句评估通过，那么就执行这个 case 里的语句；
* 除 default 外，如果有多个 case 语句评估通过，那么通过伪随机的方式随机选一个；
* 如果 default 外的 case 语句都没有通过评估，那么执行 default 里的语句；
* 如果没有 default，那么 代码块会被阻塞，直到有一个 case 通过评估，否则会一直阻塞；

注意，如果是 `v1 := <- nil` 语句，那么会直接因为从 `nil` 中读取而阻塞，而非 `panic` 崩溃。

### 其它

另外，需要注意，如果向管道中发送的数据，而有其它的协程阻塞等待，那么条件满足后会先调度执行被阻塞的协程。

{% highlight go %}
package main

import (
        "fmt"
)

func main() {
        readyc := make(chan string)

        go func() {
                select {
                case str := <-readyc:
                        fmt.Printf("%s, Go !!!\n", str)
                }

        }()

		readyc <- "Reaaaaady"
		fmt.Println("Done")
}
{% endhighlight %}

也就是上述的输出为：

{% highlight text %}
Reaaaaady, Go !!!
Done
{% endhighlight %}

## 示例

### Timeout

{% highlight go %}
package main

import "time"
import "fmt"

func main() {
        timeout := make (chan bool, 1)
        ch := make (chan int)
        go func() {
                time.Sleep(2 * time.Second)
                timeout <- true
        }()
        select {
        case <- ch:
        case <- timeout:
            fmt.Println("timeout!")
        }
}
{% endhighlight %}

因为没有向管道 ch 发送数据，默认应该是一直等待。也可以使用如下示例：

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        donec := make(chan string)

        go func() {
                time.Sleep(1 * time.Second)
                donec <- "done"
        }()

        select {
        case s := <-donec:
                fmt.Printf("Got string '%s'.\n", s)
        case <-time.After(2 * time.Second):
                fmt.Println("Timeout")
        }
}
{% endhighlight %}

### 检查队列是否满

这里使用的是 default 这个特性。

{% highlight go %}
package main

import "fmt"

func main() {
        ch := make (chan int, 1)
        ch <- 1
        select {
                case ch <- 2:
                default:
                        fmt.Println("channel is full !")
        }
}
{% endhighlight %}

因为 ch 插入 1 的时候已经满了， 当 ch 要插入 2 的时候，发现 ch 已经满了此时默认应该是阻塞，不过因为有 default 子句，实际会执行 default 子语， 这样就可以实现对 channel 是否已满的检测， 而不是一直等待。

例如在并发处理多个 job 时，如果队列满了，则返回错误让用户重试。

### Quit Channel

{% highlight go %}
package main

import (
        "fmt"
)

func boring(msg string, quit chan string) chan string {
        c := make(chan string)
        i := 0

        go func() {
                for {
                        i++
                        select {
                        case c <- fmt.Sprintf("%s: %d", msg, i):
                                fmt.Println("Send data")
                        case q := <-quit:
                                // cleanup
                                fmt.Printf("Got %q\n", q)
                                quit <- "See you!"
                                return
                        }
                }
        }()

        return c
}

func main() {
        quit := make(chan string)

        c := boring("Foobar", quit)

        for i := 5; i >= 0; i-- {
                fmt.Println(<-c)
        }

        quit <- "Bye!"

        fmt.Printf("Foobar says: %q\n", <-quit)
}
{% endhighlight %}

<!--
Quit Channel/Done Channel 还没有搞明白
https://segmentfault.com/a/1190000006815341
-->



<!--
运行时调度器和内存管理系统是如何支持Channel的，

### 实现

内存在 `$GOROOT/src/runtime/chan.go` 中实现，其实就是一个带锁的环形队列。
type hchan struct {
  ...
  buf      unsafe.Pointer // 指向一个环形队列
  ...
  sendx    uint   // 发送 index
  recvx    uint   // 接收 index
  ...
  lock     mutex  //  互斥量
}

当通过 `ch := make(chan int, 3)` 创建管道时，会在堆中分配空间、初始化，并返回一个指针，在使用时直接传递指针即可，无需在取地址。

#### 接收和发送

在发送和接收的过程中，会将对象复制一份，这样，所有协程只会共享 `hchan` 这个结构体。

#### 阻塞和恢复

Go 的调度器是 `M:N` 调度模型，既 `N` 个协程会运行于 `M` 个 OS 线程中。换句话说，一个 OS 线程中，可能会运行多个协程。

如果要运行一个协程 `G` ，在一个线程 `M` 中，必须持有一个改协程的上下文 `P` 。

https://blog.lab99.org/post/golang-2017-10-04-video-understanding-channels.html
https://github.com/gophercon/2017-talks/blob/master/KavyaJoshi-UnderstandingChannels/Kavya%20Joshi%20-%20Understanding%20Channels.pdf
https://zhuanlan.zhihu.com/p/27917262
https://i6448038.github.io/2019/04/11/go-channel/


## Resiliency

### Breaker

断路器，主要是为了防止由于某个节点故障导致整个调用链路上的整体调用异常，最终导致服务不可用。

也就是说，断路器是保证即使生产者服务宕机，可以确保整个服务优雅地处理问题，并将应用的其余部分从级联故障中保存下来。

### Retrier

重试机制，通常来说某个服务会部署多个实例 (主机) ，当一个实例宕机之后应该要重试其它机器。

这也就意味着，Retrier 一般会包含在 Breaker 中。

https://github.com/eapache/go-resiliency

通过Turbine进行Stream Aggregator
https://medium.com/netflix-techblog/hystrix-dashboard-turbine-stream-aggregator-60985a2e51df
https://blog.csdn.net/weixin_38748858/article/details/100781369
https://about.sourcegraph.com/go/grpc-in-production-alan-shreve
Netflix 最新的限流神器 Concurrency Limits
https://fredal.xin/netflix-concuurency-limits
https://medium.com/@NetflixTechBlog/performance-under-load-3e6fa9a60581
https://github.com/Netflix/concurrency-limits
高效的流式处理
https://github.com/bmizerany/perks
-->

## 参考

* GoLang 的并发控制、编程参考模型 [Pipelines](https://blog.golang.org/pipelines) 。

{% highlight text %}
{% endhighlight %}
