---
title: GoLang 内存管理
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

在 C 语言中，通过 `malloc()` 函数可以动态分配内存，常用的有 ptmalloc2(glib)、tcmalloc(Google)、jemaloc(FaceBook)，后两者在避免内存碎片以及性能上都有较大的优势。

而 Go 中的内存分配器，其原理与 tcmalloc 类似，简单的说就是维护一块大的全局内存，每个线程 (P) 维护一块小的私有内存，私有内存不足再从全局申请。

<!-- more -->

## 简介

TCMalloc 算法的全称为 Thread-Caching Malloc ，其核心思想就是把内存分为多级管理，从而降低锁的粒度。

<!--
schedinit()
 |-mallocinit()
-->

### 初始化


在 Go 程序启动的时候会先申请一块虚拟内存 (未真正分配物理内存)，然后会切割成小块自己管理，在 64 位的机器上，内容如下。

{% highlight text %}
+-------------+---------------+----------------------------+
| spans(512M) |  bitmap(16G)  |        arena(512G)         |
+-------------+---------------+----------------------------+
{% endhighlight %}

<!--
https://juejin.im/post/5c888a79e51d456ed11955a8
-->

其中，arena 区域就是所谓的堆区，动态分配的内存都是在这个区域，会把内存分成 8KB 大小的页，mspan

`bitmap` 区域顾名思义，就是标示 `arena` 中指针大小，总体大小为 `512G/(4*8B)=16G` 。

`spans` 区域保存的是 `arena` 内存管理数据结构 `mspan` 的指针，其大小为 `512GB/8KB*8B=512MB` 。

<!--
## 参考
https://www.jianshu.com/p/47691d870756
http://www.opscoder.info/golang_mem_management.html
https://juejin.im/post/5c888a79e51d456ed11955a8
-->

## 逃逸分析

在 C/C++ 中，所有的内存都是统一管理，在调用 malloc 或者 new 函数时，会在堆上分配一块内存空间，而且程序员需要负责释放资源，否则就会发生内存泄漏。

而 Go 语言中，通过 new 分配的内存不一定在堆上，而且会通过 GC 自动回收内存。

### 简介

所谓的逃逸分析，就是编译器执行静态代码分析后，对内存管理进行的优化和简化。

在编译原理中，分析指针动态范围的方法称之为逃逸分析。通俗来讲，当一个对象的指针被多个方法或线程引用时，就称这个指针发生了逃逸。

通过逃逸分析的结果，决定一个变量到底是分配在堆上还是分配在栈上。

在堆上的内存需要通过 GC 进行回收，其使用效率要比栈上的内存慢很多。

### 测试

通过 `-gcflags` 相关的参数，可以判断变量是否发生了逃逸。

{% highlight go %}
package main

import "fmt"

func foo() *int {
        t := 3
        return &t
}

func main() {
        x := foo()
        fmt.Println(*x)
}
{% endhighlight %}

使用如下命令进行测试，其中 `-l` 是为了防止发生内联。

{% highlight text %}
$ go build -gcflags '-m -l' main.go
# command-line-arguments
./main.go:7:9: &t escapes to heap
./main.go:6:2: moved to heap: t
./main.go:12:14: *x escapes to heap
./main.go:12:13: main ... argument does not escape
{% endhighlight %}

可以看到 `foo()` 函数中 `t` 变量发生了逃逸，而且 `x` 也同样发生了逃逸。

后者是因为有些函数参数为 `interface` 类型，比如 `fmt.Println(a ...interface{})` ，那么编译期间很难确定其参数的具体类型，也会发生逃逸。

另外，也可以通过 `go tool compile -S main.go` 反汇编，查看是否存在 `runtime.newobject` 的调用。

## Garbage Collection

当前 Go 使用的垃圾回收机制是三色标记法配合写屏障和辅助 GC，而其中的三色标记法是标记-清除法的一种增强版本。

如下是 Go 中 GC 算法的里程碑：

{% highlight text %}
v1.1 Stop The World, STW
v1.3 Mark STW, Sweep 并行
v1.5 三色标记法
v1.8 hybrid write barrier
{% endhighlight %}

经典的 GC 算法有：A) 引用计数 (Reference Counting)；B) 标记-清除 (Mark and Sweep)；C) 复制收集 (Copy and Collection)。

如上，Go 中使用的是标记-清除的改进版本。

### 三色标记清除算法

原始的标记-清除算法包含了两步：A) 找出可达对象，并标记；B) 回收未标记对象。在做垃圾回收的过程中，需要停止程序的运行，也就会导致程序卡顿。

<!--
Tricolor Mark-and-Sweep Algorithm

这个 GC 过程还是有两次很短的 STW(stop the world) 过程：

* 进行 root 内存对象的栈扫描；
* 标记阶段的终止暂停。
https://making.pusher.com/golangs-real-time-gc-in-theory-and-practice/
https://www.cnblogs.com/zkweb/p/7880099.html

站点SPM
https://www.biaodianfu.com/spm.html






###############################
## GoLang 内存管理
###############################

## 引子

首先看个示例。

package main

func foo() *int {
        var m1 int = 11
        return &m1
}

func main() {
        m := foo()
        println(*m)
}

对于 C/C++ 程序员来说，这个是完全无法理解的，在函数 `foo()` 中将局部变量 `m1` 的地址返回了，当函数 `foo` 退出时，`m1` 所使用的栈地址会同样消亡。一般 `C/C++` 都会给出告警，在运行时可能会出现异常。

在 [GoLang FAQ](https://golang.org/doc/faq#stack_or_heap) 中有相关的介绍。

简单来说，会自动决定变量存放在栈还是堆上，编译器会做逃逸分析(escape analysis)，当发现变量的作用域没有跑出函数范围，就可以在栈上，反之则必须分配在堆。

一个函数内局部变量，不管是不是动态 `new` 出来的，它会被分配在堆还是栈，是由编译器做逃逸分析之后做出的决定。



关于内存管理很不错的一片文章
https://www.do1618.com/archives/1328/go-%E5%86%85%E5%AD%98%E9%80%83%E9%80%B8%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/


Golang源码探索(三) GC的实现原理
https://www.cnblogs.com/zkweb/p/7880099.html

## 编译器提示
https://segmentfault.com/a/1190000016743220
https://golang.org/cmd/compile/#hdr-Compiler_Directives


源码保存在 `src/cmd/compile` 路径下。
https://segmentfault.com/a/1190000016523685
https://juejin.im/post/5dccb05ee51d4510ba708ff1
https://zhuanlan.zhihu.com/p/43845771

https://www.do1618.com/archives/1328/go-%E5%86%85%E5%AD%98%E9%80%83%E9%80%B8%E8%AF%A6%E7%BB%86%E5%88%86%E6%9E%90/
https://github.com/mushroomsir/blog/blob/master/Go%E4%B8%ADstring%E8%BD%AC%5B%5Dbyte%E7%9A%84%E9%99%B7%E9%98%B1.md#4-%E9%80%83%E9%80%B8%E5%88%86%E6%9E%90
https://github.com/qcrao/Go-Questions/blob/master/%E7%BC%96%E8%AF%91%E5%92%8C%E9%93%BE%E6%8E%A5/%E9%80%83%E9%80%B8%E5%88%86%E6%9E%90%E6%98%AF%E6%80%8E%E4%B9%88%E8%BF%9B%E8%A1%8C%E7%9A%84.md
-->



{% highlight text %}
{% endhighlight %}
