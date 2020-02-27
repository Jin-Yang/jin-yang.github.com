---
title: GoLang 陷阱
layout: post
comments: true
language: chinese
usemath: true
category: [linux,program,misc]
keywords: stan
description:
---

简单列举一些 GoLang 中容易犯的错误。

<!-- more -->

## 数组

### 数组是值传递

如果需要在函数内修改，建议使用切片而非数组。

{% highlight go %}
package main

import "fmt"

func main() {
        x := [3]int{1, 2, 3}
        y := []int{1, 2, 3}

        func(a [3]int, b []int) {
                a[0] = 7
                fmt.Printf("inner %v\n", a)

                b[0] = 7
                fmt.Printf("inner %v\n", b)
        }(x, y)
        fmt.Printf("outer %v\n", x)
        fmt.Printf("outer %v\n", y)
}
{% endhighlight %}

上述的变量中，`y` 作为切片，在函数内的修改会影响到函数外的变量，因为它修改的是指针。

## 异常处理

### 恢复的使用方式

`recover()` 函数必须在 `defer` 的函数中调用，不允许通过函数再次嵌套，不能直接调用，否则会失效。

{% highlight go %}
package main

import "fmt"

func main() {
        x := [3]int{1, 2, 3}
        y := []int{1, 2, 3}

        func(a [3]int, b []int) {
                a[0] = 7
                fmt.Printf("inner %v\n", a)

                b[0] = 7
                fmt.Printf("inner %v\n", b)
        }(x, y)
        fmt.Printf("outer %v\n", x)
        fmt.Printf("outer %v\n", y)
}
{% endhighlight %}

## 调度

### 协程泄露

与很多高级语言类似，GoLang 自带了内存回收的特性，一般内存不会泄露，但可能会出现在 GC 的时候出现卡顿。

但是，对于协程来说就可能会出现泄露，同时该协程引用的对象也无法释放。

{% highlight go %}
package main

import (
        "context"
        "fmt"
)

func main() {
        ctx, cancel := context.WithCancel(context.Background())

        ch := make(chan int)
        go func() {
                for i := 0; ; i++ {
                        // ch <- i
                        select {
                        case <-ctx.Done():
                                return
                        case ch <- i:
                        }
                }
        }()

        for v := range ch {
                fmt.Println(v)
                if v == 5 {
                        cancel()
                        break
                }
        }
}
{% endhighlight %}

上述会起一个后台的协程向管道中添加数据，如果工作协程退出，那么后台协程就可能会泄露。其中 context 包是最常用的，可以有效的处理协程之间的嵌套调用。

### 独占CPU

协程采用的是协作式强占调度，本身不会主动放弃 CPU ，如果在某个协程中死循环，那么就可能会导致其它的协程无法调度。

{% highlight go %}
package main

import (
        "fmt"
        "runtime"
)

func main() {
        runtime.GOMAXPROCS(1)

        go func() {
                for i := 0; i < 10; i++ {
                        fmt.Println(i)
                }
        }()

        //for {} // this will hang forever.
        for {
                runtime.Gosched()
        }
        // OR 'select' for quit signal.
}
{% endhighlight %}

有两种规避的方式，在死循环中允许协程重新调度，或者使用阻塞的方式等待退出。

<!--
常见的并发模式
https://chai2010.cn/advanced-go-programming-book/ch1-basic/ch1-06-goroutine.html




GoLang 是一门静态语言，所有变量需要标示变量类型，同时在编译阶段会进行检查。

package main

import (
        "fmt"
)

func main() {
        var num int32 = 5
        var ptr *int32 = &num

        fmt.Println((*int32)(ptr))
        fmt.Println((*float32)(ptr))
}

当尝试编译时会有如下的报错。

# command-line-arguments
...: cannot convert ptr (type *int32) to type *float32

## unsafe

在标准库中提供了一个 unsafe 库，可以打破 GoLang 的类型和内存安全检查机制，也就是 `unsafe.Pointer` ，可以表示任意类型的指针，并进行转换，类似 C 中的 `void *` 指针。

上述的装换可以修改为，也可以引用变量，但是打印的值可能无法辨识。

fmt.Println((*float32)(unsafe.Pointer(ptr)))

其中，`unsafe.Pointer` 可以和任意类型的指针以及 `uintptr` 相互进行转换。

> uintptr 是一个内置的类型，可以用来存储指针的整形，会根据不同的平台适配。注意，在 GC 看来，uintptr 是一个整形，不能感知其保存指针所指向的对象，可能会被回收。

### Offset

GoLang 中结构体的存储有点类似于 C 中的结构体：A) 结构体的成员变量在内存中是一块连续的内存；B) 结构体的初始地址就是第一个成员变量地址；C) 通过成员的偏移量加上结构体基地址指针，可以得到成员的地址。

package main

import (
        "fmt"
        "unsafe"
)

type Person struct {
        Name string
        Age  int
}

func main() {
        p := Person{Name: "foobar", Age: 18}
        ptr := unsafe.Pointer(&p)

        sptr := (*string)(unsafe.Pointer(ptr))
        *sptr = "FooBar"

        iptr := (*int64)(unsafe.Pointer(uintptr(ptr) + unsafe.Offsetof(p.Age)))
        *iptr = 20

        fmt.Println(p)
}

将 `ptr` 转换为 `uintptr` 可以用来进行地址的计算，再加上结构体中成员的偏移地址，就可以获取到需要的地址指针。

注意，不能在计算的时候使用如下的方式。

base := uintptr(ptr)
iptr := (*int64)(unsafe.Pointer(base + unsafe.Offsetof(p.Age)))

其中的 `base` 变量可能会被回收掉，那么接下来的计算就不再确定，也就是说，不能定义 `uintptr` 类型的变量。

### Alignof

结构体中的成员会做对齐，不能单纯通过字节数进行计算。

package main

import (
        "fmt"
        "unsafe"
)

type W struct {
        b byte  // 1
        i int32 // 4
        j int64 // 8
}

func main() {
        var w *W = new(W)

        fmt.Printf(" Size = %d\n", unsafe.Sizeof(*w))   // 16
        fmt.Printf("Align = %d\n", unsafe.Alignof(w.i)) // 4
}
-->

{% highlight text %}
{% endhighlight %}
