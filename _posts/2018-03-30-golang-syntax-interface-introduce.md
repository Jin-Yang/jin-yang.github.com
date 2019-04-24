---
title: Golang 语法之接口
layout: post
comments: true
language: chinese
category: [program,golang]
keywords: golang,interface
description: Interface 定义了方法集，只要某个类型实现了该接口的超集(实现了接口定义的所有方法，而且可能还有其它方法)，那么就可以说这个类型实现了该接口。
---

在 Go 语言的实际编程中，几乎所有的数据结构都围绕 Interface 展开，这是 GoLang 中所有数据结构的核心。

首先，Go 不是一种典型的 OO 语言，它在语法上不支持类和继承的概念，而通过 Interface 可以看到多态的影子。

<!-- more -->

## 简介

Interface 定义了方法集，只要某个类型实现了该接口的超集(实现了接口定义的所有方法，而且可能还有其它方法)，那么就可以说这个类型实现了该接口。

这意味着，一个类型可能实现了多个接口，例如，所有的类型都实现了 `interface {}` 这个接口。

Interface 会在编译时检查，在运行时动态生成，如果有类似类型的报错，在编译阶段就可以发现，而不像 Python 会在运行时报错。

上面说的 `interface {}` 比较容易混淆，它定义的是空接口，所有的类型都实现了该接口，在函数传参定义类型时，可以认为是 C 语言中的 `void *`，可接收任意类型的参数。

<!--
每个 interface 变量，都包含了两个指针，分别指向方法和该 interface 中定义的变量，关于其实现的详细解释可以参考 https://research.swtch.com/interfaces 以及 https://golang.org/doc/effective_go.html 。

I don’t want to bleat on about this endlessly.
-->

### 简单示例

一般通过如下的规则判断一个类型或者指针是否实现了该接口：

* 类型 `*T` 的对象可调用方法集含类型为 `*T` 或 `T` 的所有方法集。
* 类型 `T` 的对象可调用方法集含类型为 `T` 的所有方法集。

同时也可以得出一条推论：

* 类型 `T` 的对象不能调用方法集类型为 `*T` 的方法集。

{% highlight go %}
package main

import (
	"log"
)

type Notifier interface {
        Notify() error
}

type User struct {
        Name  string
        Email string
}

func (u *User) Notify() error {
        log.Printf("User: Sending User Email To %s<%s>\n", u.Name, u.Email)
        return nil
}

func SendNotification(notify Notifier) error {
        return notify.Notify()
}

func main() {
        user := User{
                Name:  "FooBar",
                Email: "foobar@example.com",
        }

        SendNotification(&user)
}
{% endhighlight %}

如果将 `SendNotification(&user)` 替换为 `SendNotification(user)` 将会报错。

## 理解接口

首先 Interface 是一种类型，可以参考上述示例的定义方法。

{% highlight go %}
type Notifier interface {
        Notify() error
}
{% endhighlight %}

它的定义可以看出用了 type 关键字，更准确的说 interface 是一种具有一组方法的类型，这些方法定义了 interface 的行为。

允许不带任何方法的 interface ，称为 `Empty Interface`，也就是上述的 `interface {}`。

在 go 中没有显式的关键字用来实现 interface，只需要实现 interface 包含的方法即可。

### 变量存储

interface 变量存储的是实现者的值

{% highlight go %}
package main

import (
	"log"
)

// #1
type Notifier interface {
        Notify() error
}

type User struct {
        Name  string
        Email string
}

// #2
func (u *User) Notify() error {
        log.Printf("User: Sending User Email To %s<%s>\n", u.Name, u.Email)
        return nil
}

func SendNotification(notify Notifier) error {
        return notify.Notify()
}

func main() {
        user := User{
                Name:  "FooBar",
                Email: "foobar@example.com",
        }

        SendNotification(&user)
}
{% endhighlight %}

同样复制上述的示例，其中 `#1` 定义了接口，`#2` 实现了接口中定义的方法，也就是说 `User` 是接口 `Notifier` 的实现，接着通过 `SendNotification(&user)` 完成了一次对接口类型的使用。

其比较重要的用途就在 `SendNotification()` 函数中，如果有多种类型实现了这个接口，这些类型的值都可以直接使用接口的变量存储。

{% highlight go %}
user := User{}
var n Notifier      // 声明一个接口对象
n = &user           // 将对象赋值到接口变量
SendNotification(n) // 调用该类型变量实现的接口
{% endhighlight %}

也就是说，接口变量中存储的是实现了该接口类型的对象值，这种能力称为 `duck typing`。

在使用接口时不需要显式声明要实现哪个接口，只需要实现对应接口中的方法即可，go 会自动进行检查，并在运行时完成从其他类型到接口类型的自动转换。

即使实现了多个接口，go 也会在使用对应接口时实现自动转换，这就是接口的魔力所在。

### 类型判断

如上，如果有多种类型实现了 `Notifier` 这个接口，那么在调用接口时，如何判断接口变量保存的究竟时那种类型的实现。

此时可以使用 `comma, ok` 的形式做区分，也就是 `value, ok := em.(T)`，其中 `em` 是接口类型的变量，`T` 代表要判断的类型，`value` 是接口变量存储的值，`ok` 返回是否类型 `T` 的实现。

例如，上述的示例可以修改为。

{% highlight go %}
func SendNotification(notify Notifier) error {
	if n, ok := notify.(*User); ok {
		log.Printf("User implements Notifier %+v\n", n)
	}
        return notify.Notify()
}
{% endhighlight %}

当 `ok` 是 `true` 表明 `notify` 存储的是 `*User` 类型的值，`false` 则不是，这种区分能力叫 `Type assertions` (类型断言)。

如果需要区分多种类型，可以使用 `switch` 语句，如下，其中 `Foo` 未定义会报错。

{% highlight go %}
func SendNotification(notify Notifier) error {
	switch n := notify.(type) {
		case *User:
			log.Printf("notify store *User, %+v", n)
		case *Foo:
			log.Printf("notify store *Foo, %+v", n)
	}
        return notify.Notify()
}
{% endhighlight %}

### 空接口

`interface{}` 是一个空的接口类型，如前所述，可以认为所有的类型都实现了 `interface{}`，那么如果定义一个函数参数是 `interface{}` 类型，这个函数可以接受任何类型作为它的参数。

<!--
func doSomething(v interface{}){
}

如果函数的参数 v 可以接受任何类型，那么函数被调用时在函数内部 v 是不是表示的是任何类型？并不是，虽然函数的参数可以接受任何类型，并不表示 v 就是任何类型，在函数 doSomething 内部 v 仅仅是一个 interface 类型，之所以函数可以接受任何类型是在 go 执行时传递到函数的任何类型都被自动转换成 interface{}。go 是如何进行转换的，以及 v 存储的值究竟是怎么做到可以接受任何类型的，感兴趣的可以看看 [Russ Cox 关于 interface 的实现](https://research.swtch.com/interfaces) 。



既然空的 interface 可以接受任何类型的参数，那么一个 interface{}类型的 slice 是不是就可以接受任何类型的 slice ?

func printAll(vals []interface{}) { //1
	for _, val := range vals {
		fmt.Println(val)
	}
}
func main(){
	names := []string{"stanley", "david", "oscar"}
	printAll(names)
}

上面的代码是按照我们的假设修改的，执行之后竟然会报 cannot use names (type []string) as type []interface {} in argument to printAll 错误，why？

这个错误说明 go 没有帮助我们自动把 slice 转换成 interface{} 类型的 slice，所以出错了。go 不会对 类型是interface{} 的 slice 进行转换 。为什么 go 不帮我们自动转换，一开始我也很好奇，最后终于在 go 的 wiki 中找到了答案 https://github.com/golang/go/wiki/InterfaceSlice 大意是 interface{} 会占用两个字长的存储空间，一个是自身的 methods 数据，一个是指向其存储值的指针，也就是 interface 变量存储的值，因而 slice []interface{} 其长度是固定的N*2，但是 []T 的长度是N*sizeof(T)，两种 slice 实际存储值的大小是有区别的(文中只介绍两种 slice 的不同，至于为什么不能转换猜测可能是 runtime 转换代价比较大)。

但是我们可以手动进行转换来达到我们的目的。

var dataSlice []int = foo()
var interfaceSlice []interface{} = make([]interface{}, len(dataSlice))
for i, d := range dataSlice {
	interfaceSlice[i] = d
}
-->

### Receiver 类型

如果将上述 `SendNotification(&user)` 改为 `SendNotification(user)`，执行时会报如下的错。

{% highlight go %}
cannot use user (type User) as type Notifier in argument to SendNotification:
        User does not implement Notifier (Notify method has pointer receiver)
{% endhighlight %}

上述报错的大致意思是说，`User` 没有实现 `Notifier` ，这里的关键是 `User` 的 `Notify()` 方法的 Receiver 是个指针 `*User` 。

接口的定义并没有严格规定实现者的方法 Receiver 是个 `Value Receiver` 还是 `Pointer Receiver`，不过如果定义为 `Pointer` 而使用 `Value` ，那么会导致报错。


那么，反过来会怎样，如果 Receiver 是 Value，函数用 Pointer 的形式调用？

{% highlight go %}
package main

import (
	"log"
)

type User struct {
        Name  string
        Email string
}

func (u User) Notify() error {
        log.Printf("User: Sending User Email To %s<%s>\n", u.Name, u.Email)
        return nil
}

type Notifier interface {
        Notify() error
}

func SendNotification(notify Notifier) error {
        return notify.Notify()
}

func main() {
        user := User{
                Name:  "AriesDevil",
                Email: "ariesdevil@xxoo.com",
        }

        SendNotification(user)
        SendNotification(&user)
}
{% endhighlight %}

从执行代码可以看到无论是 Pointer 还是 Value 都可以正确执行。

导致这一现象的原因是什么？

如果是按 Pointer 调用，会自动进行转换，因为有了指针总是能得到指针指向的值是什么；如果是 Value 调用，GoLang 将无从得知 Value 的原始值是什么，因为 Value 是份拷贝。

**GoLang 会把指针进行隐式转换得到 Value，但反过来则不行。**

对于 Receiver 是 Value 的方法来说，任何在方法内部对 Value 做出的改变都不影响调用者所看到的 Value，这就是按值传递。

### 其它

如果实现的类型不一致，那么在如下的调用时同样会报错。

{% highlight go %}
package main

import (
	"fmt"
)

type Animal interface {
	Speak() string
}

type Dog struct {}
// #2 func (d Dog) Speak() string {
func (d *Dog) Speak() string {
	return "Woof!"
}

type Cat struct {}
// #2 func (c Cat) Speak() string {
func (c *Cat) Speak() string {
	return "Meow!"
}

func main() {
	// #2 animals := []Animal{Dog{}, Cat{}}
	animals := []Animal{&Dog{}, &Cat{}}
	for _, animal := range animals {
		fmt.Println(animal.Speak())
	}
}
{% endhighlight %}

可以保持现状，或者都修改为 `#2` 的方式，但是如果有不一致的，将会报错。

## 参考

[理解 Go interface 的 5 个关键点](https://sanyuesha.com/2017/07/22/how-to-understand-go-interface/) 很好的一篇介绍 GoLang 中接口的文章。


{% highlight go %}
{% endhighlight %}
