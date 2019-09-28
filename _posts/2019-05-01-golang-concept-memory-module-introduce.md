---
title: GoLang 内存模型
layout: post
comments: true
language: chinese
usemath: true
category: [linux,program,misc]
keywords: stan
description:
---

在 Golang 中所谓的内存模型，定义的是，对多个协程中共享的变量，一个协程中怎样可以看到其它协程的写入。

当多个协程同时操作一个数据时，可以通过管道、同步原语 (sync 包中的 Mutex 以及 RWMutex)、原子操作 (sync/atomic 包中)。

除此之外，为了保证语义的正确性，Golang 还对一些常见的场景做了语义上的约束。

<!-- more -->

## 内存模型

那么什么是内存模型？它定义了什么？为什么要讨论内存模型？

原文的定义为。

> The Go memory model specifies the conditions under which reads of a variable in one goroutine can be guaranteed to observe values produced by writes to the same variable in a different goroutine.

## Happens Before

该概念比较难理解，很容易理解为时间顺序，而且不止 Golang 语言才有，在并发编程里很常见。

通常定义如下：

假设 A 和 B 表示一个多线程程序执行的两个操作，如果 A Happens-Before B，那么就意味着 A 操作对内存的影响，将对执行 B 的线程 (且在执行 B 之前) 可见。

这里有个约束，如果 A 和 B 在相同线程中执行，且 A 操作声明在 B 之前，那么 A Happens-Before B 。

注意，Happens-Before 并不意味着时间上的顺序。

1. A happens-before B 并不意味着 A 在 B 之前发生。
2. A 在 B 之前发生并不意味着 A happens-before B。

另外，需要注意，这里所谓的 A B 操作，对于代码或者 CPU 来说，可能不止一条命令。

### Tip #1

这里的是示例采用 C 语言，如下代码。

{% highlight c %}
int A, B;

void foobar(void)
{
	A = B + 1;  // <1>
	B = 0;      // <2>
}
{% endhighlight %}

可以通过 `gcc -O2 -S foobar.c` 得到汇编程序，如下：

{% highlight text %}
movl	B(%rip), %eax
movl	$0, B(%rip)       // store to B
addl	$1, %eax
movl	%eax, A(%rip)     // store to A
{% endhighlight %}

也就是说，最终在执行的时候，实际上是先执行的 `<2>` 然后再执行 `<1>` ，也即是说即使 `<1>` Happens-Before `<2>` ，那么实际执行的时候顺序相反。

### Tip #2

如下的示例中，`<1>` + `<2>` 以及 `<3>` + `<4> 都满足 Happens-Before 条件，`<3>` 只有在满足 `<2>` 时才会执行，也就是说，`<2>` 发生在 `<3>` 之前，但是并不意味着 `<2>` Happens-Before `<3>` 。

{% highlight c %}
int ready = 0;
int answer = 0;
void publish()
{
	answer = 42;  // (1)
	ready = 1;    // (2)  A
}
void consume()
{
	if (ready)                  // (3)  B
		printf("%d\n", answer); // (4)
}
{% endhighlight %}

这也就意味着，对于上述的代码，有可能 `publish()` 和 `consume()` 分别在不同的线程中执行，而打印的值有可能仍然是 `0` 。

### 总结

简单来说，个人理解为，所谓的 `Happens-Before` 以及 `Happens-After` 实际上对应了变量何时可见。

## 同步机制

针对不同的场景，有不同的语义定义。

### 初始化

程序的初始化操作通过单个协程实现，但是这个初始化协程可能会再创建一个协程，这就可能会导致两者并发运行。

> If a package p imports package q, the completion of q's init functions happens before the start of any of p's.

这也就意味着， `main.main()` 函数会 `Happens-After` 所有的 init 函数。

### 协程

包括了协程的创建和销毁。

#### 创建

> The go statement that starts a new goroutine happens before the goroutine's execution begins.

{% highlight go %}
package main

import (
        "fmt"
        "sync"
)

var a string
var wg sync.WaitGroup

func foobar() {
        fmt.Print(a)
        wg.Done()
}

func hello() {
        a = "Hello World\n"
        go foobar()
}

func main() {
        wg.Add(1)
        hello()
        wg.Wait()
}
{% endhighlight %}

上述代码，会保证 `a` 的赋值发生在 `foobar()` 之前，所以最终会输出 `Hello World` 字符串。

注意，因为协程调度的原因，字符串的输出可能会发生在 `foobar()` 执行完成之后。

#### 销毁

对于协程的退出，是没有任何的关于 `Happens-Before` 的保证。

{% highlight go %}
package main

import (
        "sync"
)

var a string
var wg sync.WaitGroup

func hello() {
        a = "Hello World\n"
        wg.Done()
}

func main() {
        wg.Add(1)
        go hello()
        print(a)
        wg.Wait()
}
{% endhighlight %}

在赋值给 `a` 变量之后，没有带任何的同步机制，所以该变量的赋值并不能保证对其它协程可见。

如果需要保证逻辑顺序，那么就应该通过锁或者管道机制建立相对的执行顺序。

### 管道

其实 Golang 一直提倡 `Do not communicate by sharing memory; instead, share memory by communicating.` ，而管道又是承载通讯的实现方式。

所以，在 Golang 的代码中有大量的关于管道的通讯，对于管道需要遵循如下规则。

1. A send on a channel happens before the corresponding receive from that channel completes.
2. The closing of a channel happens before a receive that returns a zero value because the channel is closed.
3. A receive from an unbuffered channel happens before the send on that channel completes.
4. The kth receive on a channel with capacity C happens before the k+Cth send from that channel completes.

接下来，逐条讲解其具体的含义。

#### Tip #1

保证发送 happens-before 接收。

{% highlight go %}
package main

var (
        c = make(chan int, 10)
        a string
)

func f() {
        a = "Hello World\n"
        c <- 0
}

func main() {
        go f()

        <-c
        print(a)
}
{% endhighlight %}

上述的代码可以确保输出 `Hello World\n` 字符串，因为 更新 a 变量 Happens-Before 通过管道 c 发送，而发送 Happens-Before 接收，接收 Happens-Before 打印数据。

所以，通过传递性，可以确保最终输出的字符串。

#### Tip #2

实际上确保了关闭管道时的行为，也就意味着在上述的代码中，如果将 `c <- 0` 替换为 `close(c)` 最终的效果也是一样的。

#### Tip #3

{% highlight go %}
package main

var (
        c = make(chan int)
        a string
)

func f() {
        a = "Hello World\n"
        <-c
}

func main() {
        go f()
        c <- 0
        print(a)
}
{% endhighlight %}

上述的代码同样可以保证输出字符串，依赖的关系为，更新 a 变量 Happens-Before 从 c 管道接收，而接收 Happens-Before 发送，发送 Happens-Before 打印数据。

但是，如果将管道设置为带缓冲区的，那么就不能保证输出字符串了，可能输出字符串，可能是空 (多数情况)，也可能会引起程序的崩溃。

#### Tip #4

例如，从容量为 3 的通道接受第 3 个元素 Happens-Before 向通道第 3+3 次写入完成。

这条规则实际上是对第三条的扩充，主要应用在带有缓冲区的管道。

最常应用的场景通过缓冲管道实现一个信号量：当前管道大小对应了已经消耗信号，容量代表了最大信号量个数，向管道写入表示获取一个信号量，读取则表示释放信号量。

因此，也可以通过带有缓冲区的管道作为并发的限制。

{% highlight go %}
package main

import (
        "fmt"
        "math/rand"
        "sync"
        "time"
)

var limit = make(chan int, 3)
var wg sync.WaitGroup

func Hello(index int) {
        r := rand.Intn(5)
        fmt.Printf("#%d sleep %d seconds.\n", index, r)
        time.Sleep(time.Duration(r) * time.Second)
        wg.Done()
}

var work []func(int)

func main() {
        work := append(work, Hello, Hello, Hello, Hello, Hello, Hello)
        wg.Add(len(work))

        for i, w := range work {
                go func(w func(int), index int) {
                        limit <- 1
                        w(index)
                        <-limit
                }(w, i)
        }
        wg.Wait()
}
{% endhighlight %}

上述的程序会立即启动 `work` 数组中的函数，但是实际运行的时候只有三个同时运行。

#### 总结

其实比较好理解发送数据 Happens-Before 接收数据，但是第三条是 WTF !!! 感觉这个是特意用来进行同步的，而非作为数据传输的通道。

### Locks

在 sync 包中，包含了两类锁类型，分别是 `sync.Mutex` 以及 `sync.RWMutex` 。

> For any sync.Mutex or sync.RWMutex variable l and n < m, call n of l.Unlock() happens before call m of l.Lock() returns.

### Once

在 sync 包中，提供了 `Once()` 机制，可以在多个协程中调用，不过最终只会在某个协程中执行，而其它协程会阻塞并直到该函数执行完成。

> A single call of f() from once.Do(f) happens (returns) before any call of once.Do(f) returns.

测试程序如下，不过暂时没有想清楚如何构造异常的场景。

{% highlight go %}
package main

import (
        "sync"
)

var a string
var once sync.Once
var wg sync.WaitGroup

func setup() {
        a = "Hello World\n"
}

func doprint() {
        once.Do(setup)
        print(a)
        wg.Done()
}

func main() {
        wg.Add(2)

        go doprint()
        go doprint()
        wg.Wait()
}
{% endhighlight %}

即使通过多个协程调用 `doprint()` 函数，实际最终只调用一次 `setup()` 函数，然后会再调用 `print()` 函数，也就是说 `setup()` Happens-Before `print()` ，所以最终会打印两次 `Hello World` 。

## 坑

其实是什么时候会发生一些不正确 (或者不符合预期) 的案例。

一般来说，如果读写发生了并发，并不意味着满足时间上的顺序关系，详见上面关于 Happens-Before 的讨论。

### Tip #1

对于如下的代码，实际上是没有任何的同步机制的，可能会出现各种异常的场景。

{% highlight go %}
package main

var a, b int

func f() {
        a = 1
        b = 2
}

func g() {
        print(b)
        print(a)
}

func main() {
        go f()
        g()
}
{% endhighlight %}

可能会打印 `00` `20` `01` `21` 几种，如果在运行时添加 `-race` 检测可以看到报错。

### Tip #2

采用 Double-Checked 的方式，该实现在 Java 中比较常见，暂时不太确定其使用场景。

简单来说，就是尝试通过 Double-Checked 的方式，来避免对同步机制的开销。

## 参考

* [The Go Memory Model](https://golang.org/ref/mem) 关于 Golang 的内存模型的官方文档介绍。

<!--
http://ifeve.com/golang-mem/
https://tiancaiamao.gitbooks.io/go-internals/content/zh/10.1.html
-->

{% highlight text %}
{% endhighlight %}
