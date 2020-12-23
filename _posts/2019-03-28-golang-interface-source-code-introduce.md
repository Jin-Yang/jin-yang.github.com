---
title: GoLang 接口源码解析
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

## 空接口

空接口类型实现没有要求，可以将任意一个值赋给空接口类型的变量，然后通过类型断言判断实际的接口类型。

{% highlight go %}
var any interface{}
any = true
any = 3.1415926
any = "Hello World"
any = map[string]int{"one":1}
any = new(bytes.Buffer)
{% endhighlight %}

空接口实际是在非空接口的实现上进行了优化，对应了 `eface` (也就是空接口) 的实现。

{% highlight go %}
type eface struct {
	_type *_type          // 类型信息
	data  unsafe.Pointer
}
{% endhighlight %}

其中的 `_type` 类型定义如下。

{% highlight go %}
type _type struct {
    size       uintptr
    ptrdata    uintptr // size of memory prefix holding all pointers
    hash       uint32
    tflag      tflag
    align      uint8
    fieldalign uint8
    kind       uint8
    alg        *typeAlg
    // gcdata stores the GC type data for the garbage collector.
    // If the KindGCProg bit is set in kind, gcdata is a GC program.
    // Otherwise it is a ptrmask bitmap. See mbitmap.go for details.
    gcdata    *byte
    str       nameOff
    ptrToThis typeOff
}
{% endhighlight %}

标识类型信息，包括了什么类型、类型大小、对齐信息等。

## 非空接口实现

{% highlight go %}
// runtime/runtime2.go
type iface struct {
	tab  *itab
	data unsafe.Pointer
}
{% endhighlight %}

其中 `tab` 成员指向的结构体，包括了类型的元数据以及一些函数的列表。

{% highlight go %}
// runtime/runtime2.go
type itab struct {
	inter *interfacetype // 接口信息，包含了接口信息
	_type *_type         // 类型信息
	hash  uint32         // copy of _type.hash. Used for type switches.
	_     [4]byte
	fun   [1]uintptr     // 该类型的方法(函数)列表，含接口
}
{% endhighlight %}

其中 `interfacetype` 包含了接口的定义信息，例如接口类型、包名称、包含方法名。

{% highlight go %}
// runtime/type.go
type interfacetype struct {
   typ     _type       // 接口类型
   pkgpath name        // 包路径
   mhdr    []imethod   // 接口方法(函数)列表，按字典序排序
}

type imethod struct {
   name nameOff        // 方法名
   ityp typeOff        // 描述方法参数返回值等细节  ???? 函数的签名如何实现
}
{% endhighlight %}


也就是说，非接口除了需要保存对象之外，还会保存接口类型 (通过 `interfacetype` 定义) 。

当调用接口方法时，实际上调用的就是 `s.itab->fun[0]` 函数，这里只保存了接口中定义的方法 (即使结构体可能定义了多个方法)，如果接口中没有定义，那么会直接报错。

{% highlight go %}
package main

import "fmt"

type Shape interface {
        Area() float64
}

type Rectangle struct {
        Width, Height float64
}

func (r *Rectangle) Area() float64 {
        return r.Height * r.Width
}

func (r *Rectangle) String() string {
        return fmt.Sprintf("width %g height %g", r.Width, r.Height)
}

func main() {
        var s Shape

        s = &Rectangle{Width: 10, Height: 20}
        fmt.Println(s.Area())
        //fmt.Println(s.String())
}
{% endhighlight %}

在上述注释掉的代码中，因为 `Shape` 接口没有定义 `String()` 方法，所以在编译阶段就会报错 `s.String undefined (type Shape has no field or method String)` 。


## 源码研究

赋值划分为两种：A) struct 类型，可能含有函数实现；B) 常规类型，例如 int string 等。

### eface

也就是会调用 `convT2E64()` 函数创建一个 eface 对象。

{% highlight go %}
package main

import "fmt"

func main() {
        var x int
        var y interface{}

        x = 1
        y = x
        fmt.Println(y)
}
{% endhighlight %}

其中关键的汇编代码如下，也就是通过 `convT2E64()` 函数

{% highlight text %}
0x001d 00029 (simple.go:10)     LEAQ    type.int(SB), AX   # 取int类型的地址
0x0024 00036 (simple.go:10)     PCDATA  $2, $0
0x0024 00036 (simple.go:10)     MOVQ    AX, (SP)           # 准备调用函数前的栈帧
0x0028 00040 (simple.go:10)     MOVQ    $1, 8(SP)
0x0031 00049 (simple.go:10)     CALL    runtime.convT2E64(SB) # 创建空接口
{% endhighlight %}

其中 `convT2E64()` 函数对应的代码如下。

{% highlight go %}
func convT2E64(t *_type, val uint64) (e eface) {
        var x unsafe.Pointer
        if val == 0 {
                x = unsafe.Pointer(&zeroVal[0])
        } else {
                x = mallocgc(8, t, false)
                *(*uint64)(x) = val
        }
        e._type = t
        e.data = x
        return
}
{% endhighlight %}

该函数中，对 0 值做了优化，可以缓存 `maxZero` 大小的零值，其中 `mallocgc()` 用来分配内存。

有些文章指出，如果值可以被一个指针的地址所保存，那么会直接使用，无需指针，不过测试使用的还是指针。

{% highlight go %}
package main

import (
        "fmt"
        "unsafe"
)

type eface struct {
        _type unsafe.Pointer
        data  unsafe.Pointer
}

func main() {
        v := 10
        var a interface{} = v

        var f eface = *(*eface)(unsafe.Pointer(&a))
        fmt.Println(f.data, ":", *(*int)(f.data))
}
{% endhighlight %}

输出的是 `0xc000014098 : 10` 。

### iface

也就是会调用 `convT2Inoptr()` 函数创建一个 eface 对象。

{% highlight go %}
package main

import "fmt"

type Shape interface {
        Area() float64
}

type Rectangle struct {
        Width, Height float64
}

func (r Rectangle) Area() float64 {
        return r.Height * r.Width
}

func main() {
        var s Shape

        r := Rectangle{Width: 10, Height: 20}
        s = r
        fmt.Println(s)
}
{% endhighlight %}

同样汇编后的关键代码如下。

{% highlight text %}
0x003d 00061 (simple.go:21)     LEAQ    go.itab."".Rectangle,"".Shape(SB), AX
0x0044 00068 (simple.go:21)     PCDATA  $2, $0
0x0044 00068 (simple.go:21)     MOVQ    AX, (SP)
0x0048 00072 (simple.go:21)     PCDATA  $2, $1
0x0048 00072 (simple.go:21)     LEAQ    ""..autotmp_2+48(SP), AX  // 将r变量的地址保存到AX
0x004d 00077 (simple.go:21)     PCDATA  $2, $0
0x004d 00077 (simple.go:21)     MOVQ    AX, 8(SP)
0x0052 00082 (simple.go:21)     CALL    runtime.convT2Inoptr(SB)
{% endhighlight %}

其中 `convT2Inoptr()` 函数的实现如下。

{% highlight go %}
func convT2Inoptr(tab *itab, elem unsafe.Pointer) (i iface) {
        t := tab._type
        if raceenabled {
                raceReadObjectPC(t, elem, getcallerpc(), funcPC(convT2Inoptr))
        }
        if msanenabled {
                msanread(elem, t.size)
        }
        x := mallocgc(t.size, t, false)
        memmove(x, elem, t.size)
        i.tab = tab
        i.data = x
        return
}
{% endhighlight %}

也就是说，返回变量的接口类型是没有变，始终为 `Shape` 接口类型，只是其对应的值修改了。

## 接口转换原理

在上述的 `iface` 源码解析中，实际封装了接口的类型 `interfacetype` 和实体类型的类型 `_type`，当判断某个类型是否满足接口时，会将类型与接口的方法集进行匹配，如果包含则认为实现了该接口。

假设类型有 `m` 个方法，接口有 `n` 个方法，那么如果遍历则其时间复杂度为 `O(mn)`，在实现时，实际会对方法集的函数按照函数名的字典序排序，所以优化后的实际时间复杂度为 `O(m+n)` 。

### 源码解析

{% highlight go %}
package main

import (
        "fmt"
)

type Stringer interface {
        String() string
}

type Shape interface {
        Area() float64
        String() string
}

type Rectangle struct {
        Width, Height float64
}

func (r *Rectangle) Area() float64 {
        return r.Height * r.Width
}

func (r *Rectangle) String() string {
        return fmt.Sprintf("width %g height %g", r.Width, r.Height)
}

func main() {
        var r Shape
        var s Stringer

        r = &Rectangle{Width: 10, Height: 20}
        fmt.Println(r.Area())

        s = r
        fmt.Println(s.String())
}
{% endhighlight %}

首先，将 `Rectangle` 结构体隐式转换为 `type Shape interface` 接口变量，然后又通过 `s = r` 进行了一次接口之间的变量转换，这里调用的是 `runtime.convI2I()` 函数。

{% highlight go %}
func convI2I(inter *interfacetype, i iface) (r iface) {
        tab := i.tab
        if tab == nil {
                return
        }
        if tab.inter == inter { // 接口相同，直接赋值给新的返回变量即可
                r.tab = tab
                r.data = i.data
                return
        }
        r.tab = getitab(inter, tab._type, false)
        r.data = i.data
        return
}
{% endhighlight %}

其中 `iface` 是由 `tab` 和 `data` 两个字段组成，而 `data` 在入参 `i` 中，那么只需要找到新的 `tab` 就可以了。当接口类型相同时，直接简单的做个赋值即可，下面介绍不同的时候。

为了减少上述 `O(m+n)` 的匹配过程，会缓存到一个 Hash 表中，其中的 Key 就是通过接口类型 `interfacetype` 和实体类型 `_type` 的值组成，可以通过 `getitab()` 函数查找。

当没有查找到时，会通过 `additab()` 函数添加到 Hash 表中，这里比较关键的是如何拼装出来 `itab` 变量，会检查合法性，并将结构体中定义的函数指针添加到变量中，注意，是只包含了接口中定义的函数。

## 断言

接口的断言就是通过 `type-switch` 或者 `v.(T)` 来判断一个接口变量是否满足某个接口的要求。

{% highlight go %}
package main

import "fmt"

type Shape interface {
        Area() float64
}

type Rectangle struct {
        Width, Height float64
}

func (r Rectangle) Area() float64 {
        return r.Height * r.Width
}

func main() {
        var r interface{}

        r = Rectangle{Width: 10, Height: 20}
        if v, ok := r.(Shape); ok {
                fmt.Println(v.Area())
        }

        switch r.(type) {
        case Shape:
                fmt.Println("it's a shape interface.")
        }
}
{% endhighlight %}

上述代码其实隐含了一个默认的类型转换，从 `struct` 转换到接口变量。

{% highlight text %}
0x0066 00102 (simple.go:20)     MOVQ    24(SP), AX
0x006b 00107 (simple.go:20)     MOVQ    16(SP), CX
0x0070 00112 (simple.go:21)     PCDATA  $2, $2
0x0070 00112 (simple.go:21)     LEAQ    type."".Shape(SB), DX
0x0077 00119 (simple.go:21)     PCDATA  $2, $1
0x0077 00119 (simple.go:21)     MOVQ    DX, (SP)
0x007b 00123 (simple.go:21)     MOVQ    CX, 8(SP)
0x0080 00128 (simple.go:21)     PCDATA  $2, $0
0x0080 00128 (simple.go:21)     MOVQ    AX, 16(SP)
0x0085 00133 (simple.go:21)     CALL    runtime.assertE2I2(SB)
{% endhighlight %}


{% highlight go %}
{% endhighlight %}
