---
title: GoLang 异常处理
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: grpc,golang
description: Golang 中的错误处理是一个被大家经常拿出来讨论的话题(另外一个是泛型)，这里简单介绍其使用方法。
---

Golang 中的错误处理是一个被大家经常拿出来讨论的话题(另外一个是泛型)，这里简单介绍其使用方法。

<!-- more -->

## 错误 VS. 异常

错误和异常是两个不同的概念，非常容易混淆，而通常是将其看做错误，即使程序中可能有异常抛出，也将异常及时捕获并转换成错误。

* 错误和异常如何区分？
* 错误处理的方式有哪几种？
* 什么时候需要使用异常终止程序？
* 什么时候需要捕获异常？

### 如何区分

错误通常是指业务正常处理流程中出现了问题，例如文件打开失败、如参为空指针等；而异常非意料中的问题出现，例如内存申请失败等。

GoLang 中引入 `error` 类型作为错误处理的标准模式，类似于 C 中的错误码，可以逐层返回，直到被处理。

<!--
Golang中引入两个内置函数panic和recover来触发和终止异常处理流程，同时引入关键字defer来延迟执行defer后面的函数。
一直等到包含defer语句的函数执行完毕时，延迟函数（defer后的函数）才会被执行，而不管包含defer语句的函数是通过return的正常结束，还是由于panic导致的异常结束。你

可以在一个函数中执行多条 defer 语句，它们的执行顺序与声明顺序相反。

当程序运行时，如果遇到引用空指针、下标越界或显式调用panic函数等情况，则先触发panic函数的执行，然后调用延迟函数。调用者继续传递panic，因此该过程一直在调用栈中重复发生：函数停止执行，调用延迟执行函数等。如果一路在延迟函数中没有recover函数的调用，则会到达该携程的起点，该携程结束，然后终止其他所有携程，包括主携程（类似于C语言中的主线程，该携程ID为1）。

错误和异常从Golang机制上讲，就是error和panic的区别。很多其他语言也一样，比如C++/Java，没有error但有errno，没有panic但有throw。

Golang错误和异常是可以互相转换的：

错误转异常，比如程序逻辑上尝试请求某个URL，最多尝试三次，尝试三次的过程中请求失败是错误，尝试完第三次还不成功的话，失败就被提升为异常了。
异常转错误，比如panic触发的异常被recover恢复后，将返回值中error类型的变量进行赋值，以便上层函数继续走错误处理流程。
-->

### 示例

例如，在 `regexp` 包中有两个函数 `Compile` `MustCompile`，它们的声明如下：

{% highlight go %}
func Compile(expr string) (*Regexp, error)
func MustCompile(str string) *Regexp
{% endhighlight %}

同样的功能，不同的设计：

* `Compile()` 基于错误处理设计，适用于用户输入场景，当用户输入的正则表达式不合法时，该函数会返回一个错误。
* `MustCompile()` 基于异常处理设计，适用于硬编码场景，调用者明确知道输入不会引起函数错误，当出现异常时则直接触发 Panic 异常。

也就是说，必须要明确什么是错误什么是异常，否则很容易出现一切皆错误或一切皆异常的情况。

### 常见异常场景

1. 空指针引用、内存访问越界(含下标越界)、除数为0；对于 C 来说会产生 `SEGV` 信号。
2. 不应该出现的分支，比如default；通常用在测试阶段及时发现错误。
3. 内存不足，对于这一场景其实很难恢复，要么保持原有功能，要么退出。

## 常见场景

简单介绍常见示例。

#### 失败的原因单一

此时直接通过 `bool` 标示，而非使用 `error` 。

{% highlight go %}
func CheckHostType(hostType string) error {
	switch hostType {
	case "virtual_machine":
		return nil
	case "bare_metal":
		return nil
	}
	return errors.New("CheckHostType ERROR:" + host_type)
}
{% endhighlight %}

可以看出，该函数失败的原因只有一个，重构一下代码。

{% highlight go %}
func IsValidHostType(hostType string) bool {
	return hostType == "virtual_machine" || hostType == "bare_metal"
}
{% endhighlight %}

当导致失败的原因不止一个时，例如常见的 IO 操作，此时用户需要了解更多的错误信息，可以通过 `error` 返回。

#### 无失败

常见的一些，例如对象已经申请，只需要设置成员等函数，就没有必要返回错误。

{% highlight go %}
func setTenantId(id int) error {
	self.TenantId = id
	return nil
}
{% endhighlight %}

对于上面的函数设计，就会有下面的调用代码。

{% highlight go %}
err := setTenantId(id)
if err != nil {
	// log && free resource
	return errors.New(...)
}
{% endhighlight %}

根据正确的姿势，重构一下代码。

{% highlight go %}
func setTenantId(id) {
	self.TenantId = id
}
{% endhighlight %}

于是调用代码变为。

{% highlight go %}
setTenantId(id)
{% endhighlight %}

#### 错误值统一定义

可以在 Golang 的每个包中增加一个错误对象定义文件，如下所示：

{% highlight go %}
var ERR_EOF = errors.New("EOF")
var ERR_CLOSED_PIPE = errors.New("io: read/write on closed pipe")
var ERR_NO_PROGRESS = errors.New("multiple Read calls return no data or error")
var ERR_SHORT_BUFFER = errors.New("short buffer")
var ERR_SHORT_WRITE = errors.New("short write")
var ERR_UNEXPECTED_EOF = errors.New("unexpected EOF")
{% endhighlight %}


#### 使用 defer

一般通过判断 `error` 的值来处理错误，如果当前操作失败，需要将本函数中已经创建的资源删除掉，示例代码如下：

{% highlight go %}
func deferDemo() error {
	err := createResource1()
	if err != nil {
		return ERR_CREATE_RESOURCE1_FAILED
	}

	err = createResource2()
	if err != nil {
		destroyResource1()
		return ERR_CREATE_RESOURCE2_FAILED
	}

	err = createResource3()
	if err != nil {
		destroyResource1()
		destroyResource2()
		return ERR_CREATE_RESOURCE3_FAILED
	}

	err = createResource4()
	if err != nil {
		destroyResource1()
		destroyResource2()
		destroyResource3()
		return ERR_CREATE_RESOURCE4_FAILED
	}
	return nil
}
{% endhighlight %}

当 GoLang 的代码执行时，如果遇到 defer 的闭包调用，则压入堆栈；当函数返回时，会按照后进先出的顺序调用闭包。

闭包的参数是值传递，而对于外部变量却是引用传递，所以闭包中的外部变量 `err` 的值就变成外部函数返回时最新的 `err` 值。

所以，根据这个结论，重构上面的示例代码。

{% highlight go %}
func deferDemo() error {
	err := createResource1()
	if err != nil {
		return ERR_CREATE_RESOURCE1_FAILED
	}
	defer func() {
		if err != nil {
			destroyResource1()
		}
	}()

	err = createResource2()
	if err != nil {
		return ERR_CREATE_RESOURCE2_FAILED
	}
	defer func() {
		if err != nil {
			destroyResource2()
		}
	}()

	err = createResource3()
	if err != nil {
		return ERR_CREATE_RESOURCE3_FAILED
	}
	defer func() {
		if err != nil {
			destroyResource3()
		}
	}()

	err = createResource4()
	if err != nil {
		return ERR_CREATE_RESOURCE4_FAILED
	}
	return nil
}
{% endhighlight %}

### 其它

比较简单的示例。

##### error 类型位置

应放在返回值类型列表的最后，对于返回值类型 `error`，用来传递错误信息。

{% highlight go %}
resp, err := http.Get(url)
if err != nil {
	return nill, err
}
{% endhighlight %}

`bool` 作为返回值类型时也一样。

{% highlight go %}
value, ok := cache.Lookup(key)
if !ok {
	// ...cache[key] does not exist…
}
{% endhighlight %}

<!--
姿势五：错误逐层传递时，层层都加日志
根据笔者经验，层层都加日志非常方便故障定位。

说明：至于通过测试来发现故障，而不是日志，目前很多团队还很难做到。如果你或你的团队能做到，那么请忽略这个姿势:)


姿势八：当上层函数不关心错误时，建议不返回error
对于一些资源清理相关的函数（destroy/delete/clear），如果子函数出错，打印日志即可，而无需将错误进一步反馈到上层函数，因为一般情况下，上层函数是不关心执行结果的，或者即使关心也无能为力，于是我们建议将相关函数设计为不返回error。

姿势九：当发生错误时，不忽略有用的返回值
通常，当函数返回non-nil的error时，其他的返回值是未定义的(undefined)，这些未定义的返回值应该被忽略。然而，有少部分函数在发生错误时，仍然会返回一些有用的返回值。比如，当读取文件发生错误时，Read函数会返回可以读取的字节数以及错误信息。对于这种情况，应该将读取到的字符串和错误信息一起打印出来。

说明：对函数的返回值要有清晰的说明，以便于其他人使用。


姿势七：当尝试几次可以避免失败时，不要立即返回错误
如果错误的发生是偶然性的，或由不可预知的问题导致。一个明智的选择是重新尝试失败的操作，有时第二次或第三次尝试时会成功。在重试时，我们需要限制重试的时间间隔或重试的次数，防止无限制的重试。

两个案例：

我们平时上网时，尝试请求某个URL，有时第一次没有响应，当我们再次刷新时，就有了惊喜。
团队的一个QA曾经建议当Neutron的attach操作失败时，最好尝试三次，这在当时的环境下验证果然是有效的。
-->





## 错误处理

GoLang 通过内置的错误接口提供了非常简单的错误处理机制，其中 error 类型是一个接口，其定义如下。

{% highlight go %}
type error interface {
	Error() string
}
{% endhighlight %}

可以通过实现 `Error()` 返回具体的报错信息，简单示例如下。

{% highlight go %}
package main

import (
        "errors"
        "fmt"
)

func Sqrt(f float64) (float64, error) {
        if f < 0 {
                return 0, errors.New("math: square root of negative number")
        }

        return f, nil
}

func main() {
        if res, err := Sqrt(-1); err != nil {
                fmt.Println(err)
        } else {
                fmt.Println(res)
        }
}
{% endhighlight %}

在函数中，通过 `errors.New()` 新建并返回一个错误信息；在调用方，如果返回的结果为 nil 则输出错误，在 `fmt` 中处理 error 时会调用 `Error()` 方法输出错误信息。

<!--
https://ethancai.github.io/2017/12/29/Error-Handling-in-Go/
-->

## 异常处理

Go 追求的是简洁优雅，没有提供传统的 `try ... catch ... finally` 这种异常处理方式，引入的是 `defer` `panic` `recover` 。也就是在 Go 中抛出一个 `panic` 异常，然后在 `defer` 中通过 `recover` 捕获这个异常，然后正常处理。

Go 对待异常 (准确说是panic) 态度是：没有全面否定异常的存在，但极不鼓励多用异常。

{% highlight go %}
package main

import "fmt"

func main() {
        defer func() {
                if err := recover(); err != nil {
                        fmt.Println(err)
                }
                fmt.Println("Process panic done")
        }()

        foobar()
}

func foobar() {
        fmt.Println("Before panic")
        panic("Panicing ...")
        fmt.Println("After panic")
}
{% endhighlight %}

通过 `go run main.go` 执行会输出如下内容。

{% highlight text %}
Before panic
Panicing ...
Process panic done
{% endhighlight %}

也就是说，在 `Panic` 之后的内容不会再执行。

<!--
参考：http://blog.dccmx.com/2012/01/exception-the-go-way/
http://kejibo.com/golang-exceptions-handle-defer-try/
http://bookjovi.iteye.com/blog/1335282
https://github.com/astaxie/build-web-application-with-golang/blob/master/02.3.md
-->



<!--
异常处理的正确姿势
姿势一：在程序开发阶段，坚持速错
去年学习Erlang的时候，建立了速错的理念，简单来讲就是“让它挂”，只有挂了你才会第一时间知道错误。在早期开发以及任何发布阶段之前，最简单的同时也可能是最好的方法是调用panic函数来中断程序的执行以强制发生错误，使得该错误不会被忽略，因而能够被尽快修复。

姿势二：在程序部署后，应恢复异常避免程序终止
在Golang中，虽然有类似Erlang进程的Goroutine，但需要强调的是Erlang的挂，只是Erlang进程的异常退出，不会导致整个Erlang节点退出，所以它挂的影响层面比较低，而Goroutine如果panic了，并且没有recover，那么整个Golang进程（类似Erlang节点）就会异常退出。所以，一旦Golang程序部署后，在任何情况下发生的异常都不应该导致程序异常退出，我们在上层函数中加一个延迟执行的recover调用来达到这个目的，并且是否进行recover需要根据环境变量或配置文件来定，默认需要recover。
这个姿势类似于C语言中的断言，但还是有区别：一般在Release版本中，断言被定义为空而失效，但需要有if校验存在进行异常保护，尽管契约式设计中不建议这样做。在Golang中，recover完全可以终止异常展开过程，省时省力。

我们在调用recover的延迟函数中以最合理的方式响应该异常：

打印堆栈的异常调用信息和关键的业务信息，以便这些问题保留可见；
将异常转换为错误，以便调用者让程序恢复到健康状态并继续安全运行。
我们看一个简单的例子：

func funcA() error {
    defer func() {
        if p := recover(); p != nil {
            fmt.Printf("panic recover! p: %v", p)
            debug.PrintStack()
        }
    }()
    return funcB()
}

func funcB() error {
    // simulation
    panic("foo")
    return errors.New("success")
}

func test() {
    err := funcA()
    if err == nil {
        fmt.Printf("err is nil\\n")
    } else {
        fmt.Printf("err is %v\\n", err)
    }
}

我们期望test函数的输出是：

err is foo
实际上test函数的输出是：

err is nil
原因是panic异常处理机制不会自动将错误信息传递给error，所以要在funcA函数中进行显式的传递，代码如下所示：

func funcA() (err error) {
    defer func() {
        if p := recover(); p != nil {
            fmt.Println("panic recover! p:", p)
            str, ok := p.(string)
            if ok {
                err = errors.New(str)
            } else {
                err = errors.New("panic")
            }
            debug.PrintStack()
        }
    }()
    return funcB()
}
姿势三：对于不应该出现的分支，使用异常处理
当某些不应该发生的场景发生时，我们就应该调用panic函数来触发异常。比如，当程序到达了某条逻辑上不可能到达的路径：

switch s := suit(drawCard()); s {
    case "Spades":
    // ...
    case "Hearts":
    // ...
    case "Diamonds":
    // ... 
    case "Clubs":
    // ...
    default:
        panic(fmt.Sprintf("invalid suit %v", s))
}
姿势四：针对入参不应该有问题的函数，使用panic设计
入参不应该有问题一般指的是硬编码，我们先看“一个启示”一节中提到的两个函数（Compile和MustCompile），其中MustCompile函数是对Compile函数的包装：

func MustCompile(str string) *Regexp {
    regexp, error := Compile(str)
    if error != nil {
        panic(`regexp: Compile(` + quote(str) + `): ` + error.Error())
    }
    return regexp
}
所以，对于同时支持用户输入场景和硬编码场景的情况，一般支持硬编码场景的函数是对支持用户输入场景函数的包装。
对于只支持硬编码单一场景的情况，函数设计时直接使用panic，即返回值类型列表中不会有error，这使得函数的调用处理非常方便（没有了乏味的"if err != nil {/ 打印 && 错误处理 /}"代码块）。
-->





## 参考

<!--
https://www.jianshu.com/p/f30da01eea97
https://medium.com/@sebdah/go-best-practices-error-handling-2d15e1f0c5ee
https://medium.com/@hussachai/error-handling-in-go-a-quick-opinionated-guide-9199dd7c7f76
https://blog.questionable.services/article/http-handler-error-handling-revisited/
https://mwholt.blogspot.com/2015/05/handling-errors-in-http-handlers-in-go.html


https://blog.csdn.net/m0_38132420/article/details/78020299
-->


{% highlight text %}
{% endhighlight %}
