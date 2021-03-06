---
title: 详细介绍 GoLang 的包管理机制
layout: post
comments: true
language: chinese
tag: [Program, GoLang]
keywords: golang,module
description: GoLang 的包管理方式经过了几次的修改，从最开始的通过环境变量管理，导致衍生了很多三方的包管理工具，再到最新官方提供的包管理工具。这里会简单介绍包管理的历史，然后详细介绍最新的包管理方案。
---

当拿到代码后，用户首先看到和接触的就是源码文件的布局、命名还有包的结构，漂亮的代码，布局清晰、易读易懂，就像是设计严谨的 API 一样。

相比其它语言，GoLang 对包、变量、代码格式，甚至代码组织结构等，都有详细的约束，这里详细介绍其包管理的规则。

<!-- more -->

## 简介

GoLang 包的命名遵循简洁、小写、单数和与目录同名的原则，这样便于引用和快速定位查找。一个包中可以根据功能拆分为多个文件，不同的文件实现不同功能点；相同包下的函数可以直接使用。

对于自带的标准包，例如 `net/http` 采用的是全路径，net 是最顶级的包，然后是 http 包，包的路径跟其在源码中的目录路径相同，这样就便于查找。

自己或者公司开发的程序而言，一般采用域名作为顶级包名的方式，这样就不用担心和其他开发者包名重复了，例如 `github.com/coreos`。

### main 包

当把一个 go 文件的包名声明为 main 时，就等于告诉编译器这是一个可执行程序，会尝试把它编译为一个二进制的可执行文件。

main 包可以被拆分成多个文件，但是只能有一个 `main()` 函数入口，假设被拆成了 `main.go` 和 `foobar.go` 那么直接运行时需要包含两个文件，如下。

{% highlight text %}
----- 直接运行
$ go run main.go foobar.go

----- 编译，生成的程序名称以第一个文件为准
$ go build main.go foobar.go
{% endhighlight %}

### import

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

## 包管理 (历史)

最初的包是通过环境变量进行查找的，其中 `GOROOT` 保存 Go 的源码目录，而通过 `GOPATH` 保存项目工程目录，当然，如果由多个目录，可以通过 `:` 分割来设置多个。

在实际使用时，通过会将一些常用的三方库保存在 `GOPATH` 的第一个目录下，所以，此时的目录一般为。

{% highlight text %}
WORKSPACE
 |-src/github.com/hello/world   引用的三方库
 |    /foobar                   项目实现的代码
 |    /foobar/mymath            项目子模块
 |-bin
 |-pkg
{% endhighlight %}

其中，在 `foobar` 目录下保存了项目代码，如果要引用 `mymath` 模块，需要调用 `import foobar/mymath` 才可以；然后，通过 `go install foobar` 安装，此时会在 `bin/` 目录下生成对应的二进制文件。

注意，如果通过 `go build foobar` 会在当前目录下生成二进制文件。

### 示例

新建一个临时的项目工程 `mkdir /tmp/foobar && GOPATH=/tmp/foobar` 。

简单来说，上述的三方库引入一个最简单打印输出。

{% highlight go %}
// src/github.com/hello/world/hello.go
package world

import "fmt"

func Hi() {
        fmt.Println("Hello World!")
}
{% endhighlight %}

如下是一个项目内的代码。

{% highlight go %}
// src/foobar/main.go
package main

import (
        "fmt"
        "foobar/mymath"
        "github.com/hello/world"
)

func main() {
        world.Hi()
        fmt.Printf("sqrt(4) %.2f\n", mymath.Sqrt(4))
}
{% endhighlight %}

以及项目中的子模块。

{% highlight go %}
// src/foobar/mymath/sqrt.go
package mymath

import "math"

func Sqrt(x float64) float64 {
        return math.Sqrt(x)
}
{% endhighlight %}

### Vendor

从 v1.5 开始开始引入 vendor 包模式，如果项目目录下有 vendor 目录，那么 go 工具链会优先使用 vendor 内的包进行编译、测试等，而不是之前的 `GOPATH` `GOROOT` 等环境变量。

实际上，这之后第三方的包管理思路都是通过这种方式来实现，比如说由社区维护准官方包管理工具 dep ，不过官方不认可。

如果只维护了一个项目，而且该目录下包含的都是与当前项目相关的内容，那么实际上维护起来还好，但是如果有多个项目，而且想共用一些三方仓库，那么维护起来就比较麻烦。

{% highlight text %}
WORKSPACE
 |-src/github.com/some/third                  引用的一些通用三方库
 |    /foobar                                 项目实现的代码
 |    /foobar/mymath                          项目子模块
 |    /foobar/vendor/github.com/hello/world   单个项目引用的三方库
 |-bin
 |-pkg
{% endhighlight %}

在 v1.11 中加入了 Go Module 作为官方包管理形式，在 v1.11 和 v1.12 版本中 gomod 不能直接使用，可以执行 `go env` 命令查看是否有 GOMOD 判断是否已开启。

如果没有开启，可以通过设置环境变量 `export GO111MODULE=on` 开启，当使用 modules 时，会完全忽略原有的 vendor 机制。

接着详细介绍如何使用 Go 标准的包管理方案 Modules 。

## Module (最新)

到目前为止，仍然要强依赖于 GOPATH 变量的设置，所以要么已经完全设置好了，要么就每个项目维护一个打包脚本，在该脚本中设置相应的环境变量。

之所以会引入依赖，无非是为了复用自己 (或别人) 的工作成果，但这样会存在很多的不确定因素：包的 API 会变化，内部行为会变化，改包的依赖会变化，包与包之间的不同依赖相互冲突等等。

不仅如此，随着软件开发规模的逐步增大，涉及到的外部依赖越来越多，手动管理的所有依赖愈发不可能。所以需要依赖管理，需要有个工具或者规范来描述和定义包与包之间的依赖关系，并自动化的去处理、解析和满足这些依赖。

GoLang 的 Module 是官方提供的一个依赖管理方案，各个发布包通过版本进行兼容性约束，不同的依赖包同时引入了版本管理，以保证兼容性。

这里实际是强制要求所有的模块遵循语义化版本规则 [Semantic Versioning](https://semver.org/) ，规定，当主版本号大于等于 `v2` 时，这个模块在 `import` 路径的结尾上必须指定主版本号；也就是说，当主版本号为 `v0` 或者 `v1` 的时候可以省略。

根据语义化版本的要求，v0 是不需要保证兼容性的，可以随意的引入破坏性变更，所以不需要显式的写出来；而省略 v1 更大程度上是为了兼容现有的代码库，很少有版本会超过 v2 。

<!--
Creating a new module.
Adding a dependency.
Upgrading dependencies.
Adding a dependency on a new major version.
Upgrading a dependency to a new major version.
Removing unused dependencies.
-->

### 0. 准备环境

在使用之前，首先设置如下的环境变量。

{% highlight text %}
export GO111MODULE=on
export GOPROXY=https://goproxy.cn
export GONOSUMDB=*

----- 版本大于1.13(推荐)
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct

----- 查看当前设置的环境变量
go env
{% endhighlight %}

其中 `GO111MODULE` 打开模块支持，忽略 `GOPATH` 以及 `vendor` 目录；`GONOSUMDB` 默认下载完依赖模块后，会检查其校验值，默认是 `https://sum.golang.org` ，这里过滤掉所有的包。

校验地址可以通过 `GOSUMDB` 指定地址及其公钥值，与此相关的还有 `GOPRIVATE` `GONOPROXY` 几个环境变量。

**注意** 如果之前已经设置了环境变量，例如 `GOPROXY` ，那么通过 `go env -w` 修改会报 `does not override conflicting OS environment variable` 类似的错误，主要是因为通过 `go env` 修改时不支持覆盖，此时可以直接命令行修改。

### 1. 创建新模块

这里尝试在 `/tmp/HelloWorld` 目录下创建一个项目，先创建目录 `/tmp/HelloWorld` ，然后添加如下文件。

{% highlight go %}
// hello.go
package hello

func Hello() string {
        return "Hello World."
}
{% endhighlight %}

{% highlight go %}
// hello_test.go
package hello

import "testing"

func TestHello(t *testing.T) {
        want := "Hello, world."
        if got := Hello(); got != want {
                t.Errorf("Hello() = %q, want %q", got, want)
        }
}
{% endhighlight %}

当执行测试时，会有如下的输出信息。

{% highlight text %}
$ go test
PASS
ok      _/tmp/HelloWorld    0.020s
{% endhighlight %}

因为当前目录不在 `$GOPATH` 且不是一个 Module 工程，所以上述的测试结果会根据当前路径生成一个虚拟的包名称。

当初始化包之后再执行，会直接输出包的名称。

{% highlight text %}
$ go mod init example.com/hello
go: creating new go.mod: module example.com/hello
$ go test
PASS
ok      example.com/hello    0.020s
{% endhighlight %}

注意，在 `go mod init` 过程中，会在 `$GOPATH` 目录下创建一个子目录 `pkg/mod/cache` 来缓存版本信息，所以使用时需要保证当前用户对该目录有写权限。

完成初始化之后会自动创建一个 `go.mod` 文件。

{% highlight text %}
$ cat go.mod
module example.com/hello

go 1.13
{% endhighlight %}

改文件只会出现在模块的顶层。

### 2. 添加依赖

将上述的 `hello.go` 文件引入一个三方模块。

{% highlight go %}
package hello

import "rsc.io/quote"

func Hello() string {
        return quote.Hello()
}
{% endhighlight %}

然后再次执行 `go test` 。

{% highlight text %}
$ go test
go: downloading rsc.io/quote v3.1.0+incompatible
go: extracting rsc.io/quote v3.1.0+incompatible
go: downloading rsc.io/sampler v1.3.0
go: extracting rsc.io/sampler v1.3.0
go: downloading golang.org/x/text v0.0.0-20170915032832-14c0d48ead0c
go: extracting golang.org/x/text v0.0.0-20170915032832-14c0d48ead0c
go: finding rsc.io/sampler v1.3.0
go: finding golang.org/x/text v0.0.0-20170915032832-14c0d48ead0c
PASS
ok      example.com/hello       0.003s
{% endhighlight %}

相关的工具链会解析 `go.mod` 中的包并下载，如果没有指定，那么会尝试下载最新的包及其相关的依赖包。然后会更新 `go.mod` 以及 `go.sum` 文件，包会缓存在 `$GOPATH/pkg/mod` 目录下。

{% highlight text %}
module example.com/hello

go 1.13

require rsc.io/quote v3.1.0+incompatible
{% endhighlight %}

整个模块的依赖可以通过 `go list -m all` 查看。

#### go.mod

这里简单介绍一些相关的版本命名方式，详细可以参考 [Pseudo Versions](https://golang.org/cmd/go/#hdr-Pseudo_versions) 中的介绍。

建议使用标准的 `vX.Y.Z` 的 tag 格式，如果没有会使用 `v0.0.0-yyyymmddhhmmss-abcdefabcdef` 的格式，其中 `v0.0.0` 表示最新的 tag ，接着是 UTC 提交时间，以及最近一次提交的 hash 值。这样，Go 就可以通过时间比较那个的版本最新。

{% highlight text %}
golang.org/x/text v0.0.0-20170915032832-14c0d48ead0c
{% endhighlight %}

还有一种是打了 tag ，但是没有使用模块，会有类似 `v3.2.1+incompatible` 的版本号。

### 3. 依赖升级

上述的 `golang.org/x/text` 因为默认的最小依赖原则，实际上下载的是一个老的版本，这里直接尝试更新成最新的版本。

{% highlight text %}
$ go get golang.org/x/text
go: finding golang.org/x/text v0.3.2
go: downloading golang.org/x/text v0.3.2
go: extracting golang.org/x/text v0.3.2
$ go test
PASS
ok      example.com/hello       0.003s
{% endhighlight %}

也就是升级成了 `v0.3.2` 版本，而且测试通过。此时的 `go.mod` 文件会同步更新成如下内容。

{% highlight text %}
module example.com/hello

go 1.13

require (
        golang.org/x/text v0.3.2 // indirect
        rsc.io/quote v3.1.0+incompatible
)
{% endhighlight %}

其中 `indirect` 表示非本模块直接引入的包，其它的标识可以查看 `go help modules` 命令。

同样的方式尝试更新 `rsc.io/sampler` 包。

{% highlight text %}
$ go get rsc.io/sampler
go: finding rsc.io/sampler v1.99.99
go: downloading rsc.io/sampler v1.99.99
go: extracting rsc.io/sampler v1.99.99
$ go test
--- FAIL: TestHello (0.00s)
    hello_test.go:8: Hello() = "99 bottles of beer on the wall, 99 bottles of beer, ...", want "Hello, world."
FAIL
exit status 1
FAIL    example.com/hello       0.002s
{% endhighlight %}

不过这次更新后的测试没有通过，也就是说最新的版本是不兼容的，通过如下命令查看该包当前的版本。

{% highlight text %}
$ go list -m -versions rsc.io/sampler
rsc.io/sampler v1.0.0 v1.2.0 v1.2.1 v1.3.0 v1.3.1 v1.99.99
{% endhighlight %}

然后尝试使用 `v.1.3.1` 版本，也就是命令 `go get rsc.io/sampler@v1.3.1` ，其中 `@XXXX` 用来指定具体的版本号，默认是 `@latest` 。

### 4. 指定大版本号

引入一个函数 `Proverb` 返回那句 Go 里面经典的 `Concurrency is not parallelism.`，相关源码文件更新如下。

{% highlight go %}
package hello

import (
        "rsc.io/quote"
        quoteV3 "rsc.io/quote/v3"
)

func Hello() string {
        return quote.Hello()
}

func Proverb() string {
        return quoteV3.Concurrency()
}
{% endhighlight %}

{% highlight go %}
package hello

import "testing"

func TestHello(t *testing.T) {
        want := "Hello, world."
        if got := Hello(); got != want {
                t.Errorf("Hello() = %q, want %q", got, want)
        }
}

func TestProverb(t *testing.T) {
        want := "Concurrency is not parallelism."
        if got := Proverb(); got != want {
                t.Errorf("Proverb() = %q, want %q", got, want)
        }
}
{% endhighlight %}

然后同样调用 `go test` 测试。

{% highlight text %}
$ go test
go: finding rsc.io/quote/v3 v3.1.0
go: downloading rsc.io/quote/v3 v3.1.0
go: extracting rsc.io/quote/v3 v3.1.0
PASS
ok      example.com/hello       0.003s
{% endhighlight %}

然后这个 hello 模块会依赖 `rsc.io/quote` 的两个版本。

{% highlight text %}
$ go list -m rsc.io/q...
rsc.io/quote v3.1.0+incompatible
rsc.io/quote/v3 v3.1.0
{% endhighlight %}

每个大版本的路径都会添加版本号信息，例如上述的 `v3` 版本路径为 `rsc.io/quote/v3`，也就是 [Semantic Import Versioning](https://research.swtch.com/vgo-import) ，对于不兼容的版本使用不同的路径。

一般来说，同一个大版本中，应该是向前兼容的，当然也有例外，例如上述的 `rsc.io/sampler v1.99.99` 。

通过版本号的控制，可以对代码中的不同部分逐渐升级。

### 5. 升级到同一版本

上述的 `rsc.io/quote` 引入了两个版本，这里将其统一到同一个版本，也就是最新版本。首先查看文档，确认其对应的接口变化。

{% highlight text %}
$ go doc rsc.io/quote/v3
package quote // import "rsc.io/quote/v3"

Package quote collects pithy sayings.

func Concurrency() string
func GlassV3() string
func GoV3() string
func HelloV3() string
func OptV3() string
{% endhighlight %}

然后直接替换为如下。

{% highlight go %}
package hello

import "rsc.io/quote/v3"

func Hello() string {
    return quote.HelloV3()
}

func Proverb() string {
    return quote.Concurrency()
}
{% endhighlight %}

### 6. 清理不需要的包

如上，已经不再依赖 `rsc.io/quote` 这个包了，但是通过 `go list -m all` 查看时仍然存在。

{% highlight text %}
$ cat go.mod
module example.com/hello

go 1.13

require (
        golang.org/x/text v0.3.2 // indirect
        rsc.io/quote v3.1.0+incompatible
        rsc.io/quote/v3 v3.1.0
        rsc.io/sampler v1.3.1 // indirect
)
{% endhighlight %}

这主要是因为，像 `go test` `go build` 这类的工具，很容易发现那些包需要，但如果要确认那些不再依赖，需要加载所有的包依赖。

可以通过 `go mod tidy` 手动清理。

{% highlight text %}
$ cat go.mod
module example.com/hello

go 1.13

require (
        golang.org/x/text v0.3.2 // indirect
        rsc.io/quote/v3 v3.1.0
        rsc.io/sampler v1.3.1 // indirect
)
{% endhighlight %}

相关包的依赖关系可以通过 `go mod graph` 查看。

### 7. 其它

#### 常用命令

```
----- 查看所有依赖
go list -u -m all
```

<!--
## Minimal Version Selection
https://xuanwo.io/2019/05/27/go-modules/
## 最小版本选择
go list -m -versions github.com/sirupsen/logrus
-->

#### Semantic Import Versioning

详细可以参考 [Semantic Import Versioning](https://research.swtch.com/vgo-import) 中的相关介绍，这里只是对其关键信息的摘录。

主要是为了解决，当前项目引入了一个非兼容的包之后如何进行处理，而 go 的原则是，只要是相同的路径，对应的包就是向前兼容的，对于不兼容的包，则通过 `vN` 版本号解决。

这样带来的好处是，对于不兼容的接口，允许代码完成灰度的升级替换。

## 三方包

在 1.5 版本之前，包的管理方式简单的粗暴，仅通过环境变量进行设置。

{% highlight text %}
GOROOT=/usr/local/golang
GOPATH=/home/USER/golang
GOBIN=/usr/local/golang/bin
{% endhighlight %}

其中 `GOROOT` 会保存编译器、工具链、基础源码库等基础代码，而 `GOPATH` 是用户自定义的代码所在位置。

如果执行 `go install` 安装包，那么对应的二进制会保存在 `${GOBIN}/bin` 目录下，如果 `GOBIN` 环境变量不存在，那么就会保存在 `${GOPATH}/bin` 目录下。

当通过 `go get -v` 下载包时，会将下载依赖包源码保存到 `${GOPATH}/src` 目录下，然后在 `${GOPATH}/pkg` 目录下生成该包的静态库，那么下次使用就不用再从源码编译。

在包搜索时，会依次查找 `${GOROOT}` 以及 `${GOPATH}` ，所以尽量不要重名。

## 参考

* [Organizing Go code](https://talks.golang.org/2014/organizeio.slide#1) 。
* [Using Go Modules](https://blog.golang.org/using-go-modules) 总共有四篇文章介绍如何使用、迁移、发布基于 Modules 的包。
* [Go & Versioning](https://research.swtch.com/vgo) Russ Cox 关于 Go 版本管理的一些讨论文章汇总，以及相关原则 [The Principles of Versioning in Go](https://research.swtch.com/vgo-principles) 。

{% highlight text %}
{% endhighlight %}
