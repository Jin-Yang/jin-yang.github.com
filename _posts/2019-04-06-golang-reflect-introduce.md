---
title: GoLang 反射简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

与 C/C++ 不同，GoLang 的指针是不支持指针运算和转换，GoLang 是静态语言，所有变量必须标明其类型，不同类型变量不能执行赋值、比较、计算等操作；指针也有其对应的类型，在编译阶段同样会进行检查。

同时，GoLang 在运行阶段提供了一定的灵活性。

<!-- more -->

## 简介

在 GoLang 中 Reflect 是一个很关键的包，可以在代码运行的阶段操作任意对象，是元编程 (MetaPrograming) 的基础，但是也很容易混淆。

> 元编程通常操作其它 (含自身) 程序的数据，在运行阶段完成本来是在编译阶段完成的工作，从而使一些修改无需重新编译，一般需要通过反射完成。

### 基本概念

GoLang 是静态类型，每个变量在编译阶段已经确定了其类型，例如如下示例。

{% highlight text %}
type MyInt int

var i int
var j MyInt
{% endhighlight %}

其中变量 `i` `j` 是不同的静态类型，虽然两者的底层数据类型是相同的，但是如果不经过转换是无法直接赋值的。

比较特殊的是接口，它定义了一系列的函数，一个接口变量可以保存任意类型的变量，只要改变量实现了接口定义的方法即可。

### 基本示例

变量包括了 `(type, value)` 两部分，分别代表了变量的类型及其值。

在 GoLang 的标准库中定义了 `type Value struct` 以及 `type Type interface` 两个类型，对应了源码中的 `src/reflect/{value.go,type.go}` 文件，所有的反射操作都是基于这两个类型进行的，分别可以通过 `reflect.ValueOf()` 以及 `reflect.TypeOf()` 两个函数获取，函数声明如下。

{% highlight text %}
func ValueOf(i interface{}) Value
func TypeOf(i interface{}) Type
{% endhighlight %}

对于基本类型的操作示例如下。

{% highlight go %}
package main

import (
        "fmt"
        "reflect"
)

func main() {
        var pi float64 = 3.1415926

        fmt.Println(reflect.TypeOf(pi), reflect.ValueOf(pi))
}
{% endhighlight %}

## 法则

反射作为元编程的一种方式，可以减少重复代码提高灵活度，但同时是一把双刃剑，过量使用会使程序逻辑变得复杂难以维护。同时在使用时，需要遵循三大规则。

* 从 `interface{}` 变量可以反射出反射对象 `reflect.Value` ；
* 从反射对象 `reflect.Value` 可以获取 `interface{}` 变量；
* 要修改反射对象，其值必须可设置；

### 法则 <1>

也就是可以从 **空接口** 变量获取到反射对象，虽然有些基本变量可以直接调用，例如 `int` `string` `float64` 等，实际上在调用 `ValueOf()` 或者 `TypeOf()` 时，因为入参类型为 `interface{}` ，从而隐含了类型转换。

{% highlight text %}
func TypeOf(i interface{}) Type
func ValueOf(i interface{}) Value
{% endhighlight %}

实际上，这一规则在源码中有很好的体现，也解释了为什么不能是非空的接口变量。

其中有 `Kind()` 函数需要关注，返回的是底层的类型，而 `Type()` 则是指定的类型，例如。

{% highlight go %}
type MyInt int
var x MyInt = 7

v := reflect.ValueOf(x)
fmt.Println(v.Type(), v.Kind())
{% endhighlight %}

会输出 `main.MyInt int` 。

### 法则 <2>

实际上就是第一条的逆向操作，可以通过 `Interface()` 接口可以获取到 `interface{}` ，实际上就是将值以及类型重新封装，然后获取结果后可以根据具体的类型进行转换。

{% highlight go %}
package main

import (
        "fmt"
        "reflect"
)

func main() {
        var pi float64 = 3.1415926

        v := reflect.ValueOf(pi)
        npi := v.Interface().(float64)

        fmt.Printf("got value %f %f\n", npi, v.Interface())
}
{% endhighlight %}

在转换之后，因为是空接口类型，需要手动转换到具体的类型。

另外，一些公共函数的入参是空类型，那么实际上可以直接使用 `v.Interface()` 返回的值即可，无需再手动转换。

### 法则 <3>

是否可以设置是 `reflect.Value` 的一个特性 (可以通过 `v.CanSet()` 检查)，例如如下的示例就会报错。

{% highlight text %}
var pi float64 = 3.1415926
v := reflect.ValueOf(pi)
v.SetFloat(3.14)
{% endhighlight %}

报错的内容为 `panic: reflect: reflect.Value.SetFloat using unaddressable value` ，报错信息有些歧义，实际是因为 `v` 不能设置。	

`Settability` 有点类似于 `Addressability` ，通过判断映射的对象是否包含 (或者可以操作) 原始值来判定的，例如上述的 `float64` ，因为 Go 的函数调用是传值，所以在转换为 `Value` 之后实际上是原始值的副本。

如果直接修改副本，那么实际对原始值没有任何的影响，而且没有任何的意义，所以 Go 直接禁用了这一场景。

所以，这里在通过 `ValueOf()` 获取 `Value` 对象时，应该使用指针。

{% highlight text %}
var pi float64 = 3.1415926

v := reflect.ValueOf(&pi)
v.Elem().SetFloat(3.14)
fmt.Println(pi)
{% endhighlight %}

注意，如果不调用 `Elem()` ，指针对象仍然不可以设置，上述操作相当于如下。

{% highlight text %}
pi := 3.1415926
p := &pi
*p = 3.14
{% endhighlight %}

<!--
当对指针获取反射对象时，可以通过 `reflect.Elem()` 获取这个指针指向的元素类型，相当于对指针变量做了 `*` 操作。
-->

## 示例

### 调用函数

在 GoLang 中，函数可以像 int float 类型的变量那样赋值给某个变量，然后再调用的。

{% highlight go %}
package main

import "fmt"

func hello() {
        fmt.Println("Hello world!")
}

func main() {
        f := hello
        f()
}
{% endhighlight %}

函数和其它的变量一样，不过其反射的类型是 `reflect.Func` ，如果要调用该函数，可以通过 `Call()` 方法实现。

不过需要注意的是，`Call()` 方法的入参是 `reflect.Value` 类型的 slice ，返回值与之相同。

{% highlight go %}
package main

import (
        "fmt"
        "reflect"
        "strconv"
)

func Format(v int) string {
        fmt.Println("Value is", v)
        return strconv.Itoa(v)
}

func main() {
        fv := reflect.ValueOf(Format)
        r := fv.Call([]reflect.Value{reflect.ValueOf(20)}) // fv.Call(nil)
        fmt.Println("Result is", r[0].Interface().(string))
		//fmt.Println("Result is", r[0].String())
}
{% endhighlight %}

接着看下结构体中如何调用函数，与上面的区别是，需要通过 `Method()` 或者 `MethodByName()` 获取对应的函数。

{% highlight go %}
package main

import (
        "fmt"
        "reflect"
)

type Rectangle struct {
        Width, Height float64
}

func (r *Rectangle) Area() float64 {
        return r.Height * r.Width
}

func (r *Rectangle) Update(w, h float64) {
        r.Width = w
        r.Height = h
}

func (r *Rectangle) String() string {
        return fmt.Sprintf("width %g height %g", r.Width, r.Height)
}

func main() {
        rect := &Rectangle{Width: 10, Height: 20}
        e := reflect.ValueOf(&rect).Elem()
        fmt.Println(e.MethodByName("String").Call(nil)[0])
        //fmt.Println(e.Method(1).Call(nil)[0])

        e.MethodByName("Update").Call([]reflect.Value{reflect.ValueOf(20.0), reflect.ValueOf(40.0)})
        //e.Method(2).Call([]reflect.Value{reflect.ValueOf(20.0), reflect.ValueOf(40.0)})
        fmt.Println(e.MethodByName("String").Call(nil)[0])
}
{% endhighlight %}

注意 `Method()` 函数中的顺序是函数名排序后的结果。另外，如果 `rect` 变量不是指针，那么就不用再调用 `Elem()` 方法。

{% highlight go %}
func main() {
        rect := Rectangle{Width: 10, Height: 20}
        e := reflect.ValueOf(&rect)
        fmt.Println(e.MethodByName("String").Call(nil)[0])
}
{% endhighlight %}

这里需要注意的是，上面的 `reflect.ValueOf(&rect)` 必须要取地址，否则返回的是一个结构体，那么不会支持类似 `MethodByName()` 这种操作。

实际上还隐藏了一个操作，也就是将 `&rect` 赋值给了一个接口变量，并返回给了变量 `e` 。

### 其它

如果结构体中是私有成员，那么即使通过反射也无法获取，会报 `reflect.Value.Interface: cannot return value obtained from unexported field or method` 错误。

<!--
## 面向对象编程

首先，所谓的面向对象编程 Object Oriented Programming, OOP，是一种编程的方法论，并不与具体的语言绑定，只是有些语言提供了特有的关键词支持这一方法 (例如 C++、Java 等)，而有些则需要编程规范约定 (例如 C) 。

GoLang 的面向对象编程有如下的特性：

1. 支持面向对象编程，与传统的 C++、Java 略有区别，并非纯粹的面向对象语言。
2. 没有指定 Class ，而是通过 Struct 以及 Interface 实现。
3. 去掉了传统 OOP 语言的继承、方法重载、构造函数和析构函数、This 指针等等，相比更加简洁。
4. 支持继承、封装、多态等特性，只是实现的方式不同，例如继承通过匿名字段实现、多态通过接口来实现。

实际上，GoLang 主要是通过 interface 实现面向对象的编程，或者严格来说是 Duck Type ，提供了静态语言的一些特性，又有运行时的灵活性。

https://juejin.im/post/5db2a88c5188256499774327
https://blog.csdn.net/tdcqzd/article/details/81255363


其中传入的 `any` 参数可以是任意值，然后通过 `comma ok` 操作判断该类型是否具有 `type Stringer interface`，也就是是否含有 `String()` 函数。

如果含有的话，那么会直接调用该函数；如果没有，则会尝试几个基本类型的转换。这也是 `fmt` 包的执行方式。

在编译阶段会检查隐式的类型转换，通过上述的接口方式，也可以在运行阶段进行类型检查。


通过 Duck Type 实现了一种松耦合的特性，优先使用组合而非继承，相比类似 C++ Java 来说会更加灵活。


有两种调用函数的方式 (关键是确定函数地址)：A) 编译阶段通过方法表格静态确定，例如 C++、Java 等；B) 运行阶段查找，例如 JavaScript、Python 等，这一类为了提升性能会做各种的缓存。

GoLang 实际介于两者之间，含有方法表，但在运行阶段动态查找。

### 示例

在标准 io 库中定义了读写接口，这里仅以读取接口为例。

type Reader interface {
    Read(p []byte) (n int, err error)
}

在使用时，只要实现了上述的 `Read()` 函数，都可以赋值给 `io.Reader` 接口的变量，例如。

var r io.Reader
r = os.Stdin
r = bufio.NewReader(r)
r = new(bytes.Buffer)

无论 `r` 变量的值是什么，其对应的类型在编译阶段已经确定，也就是 `io.Reader` 类型。

比较特殊的是 `interface {}` 也就是空接口，因为没有定义任何函数，所以可以将任意类型赋值给该变量。


类型不需要显式声明它实现了某个接口，同时一个类型可以实现多个接口。



Working with Errors in Go 1.13
https://blog.golang.org/go1.13-errors

http://www.cnhalo.net/2017/03/15/dive-into-tls-with-golang/
-->

## 参考

* [The Laws of Reflection](https://blog.golang.org/laws-of-reflection)


{% highlight text %}
{% endhighlight %}
