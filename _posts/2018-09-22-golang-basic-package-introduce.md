---
title: GoLang 包管理
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: grpc,golang
description: 当拿到代码后，用户首先看到和接触的就是源码文件的布局、命名还有包的结构。漂亮的代码，布局清晰、易读易懂，就像是设计严谨的 API 一样。Go 语言有自己的命名与代码组织规则。
---

当拿到代码后，用户首先看到和接触的就是源码文件的布局、命名还有包的结构。漂亮的代码，布局清晰、易读易懂，就像是设计严谨的 API 一样。

Go 语言有自己的命名与代码组织规则。

<!-- more -->

## 包命名布局

GoLang 包的命名遵循简洁、小写、单数和与目录同名的原则，这样便于引用和快速定位查找。一个包中可以根据功能拆分为多个文件，不同的文件实现不同功能点；相同包下的函数可以直接使用。

对于自带的标准包，例如 `net/http` 采用的是全路径，net 是最顶级的包，然后是 http 包，包的路径跟其在源码中的目录路径相同，这样就便于查找。

自己或者公司开发的程序而言，一般采用域名作为顶级包名的方式，这样就不用担心和其他开发者包名重复了，例如 `github.com/coreos`。

## main 包

当把一个 go 文件的包名声明为 main 时，就等于告诉编译器这是一个可执行程序，会尝试把它编译为一个二进制的可执行文件。

main 包可以被拆分成多个文件，但是只能有一个 `main()` 函数入口，假设被拆成了 `main.go` 和 `foobar.go` 那么直接运行时需要包含两个文件，如下。

{% highlight text %}
----- 直接运行
$ go run main.go foobar.go

----- 编译，生成的程序名称以第一个文件为准
$ go build main.go foobar.go
{% endhighlight %}

## import

在 golang 中可以通过如下的方式导入。

{% highlight go %}
import(
	"fmt"       // 标准库
	"./model"   // 本地库
	"model/png" // 加载$GOPATH/src/model/png中的库
	. "png"     // 直接使用相关的函数即可，无需包前缀
	p "png"     // 重命名包名
)
{% endhighlight %}

另外，常见的一种操作符 `_` ，例如：

{% highlight go %}
import ("database/sql" _ "github.com/ziutek/mymysql/godrv")
{% endhighlight %}

这里其实只是引入该包，当导入一个包时，它所有的 `init()` 函数就会被执行，如果仅仅是希望它的 `init()` 函数被执行，此时就可以使用 `_` 。

引入的初始化顺序为：

![golang logo]({{ site.url }}/images/go/init-sequence.png "golang logo"){: .pull-center width="70%" }

1. import pkg 的初始化过程；
2. pkg 中定义的 const 变量初始化；
3. pkg 中定义的 var 全局变量；
4. pkg 中定义的 init 函数，可能有多个。


{% highlight text %}
{% endhighlight %}
