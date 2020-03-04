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

## 包管理

原则是，尽量保持目录的清晰，支持一些常见工具 (如 vim-go 等) 的使用。

最初是通过 `GOROOT` 保存 Go 的源码目录，而通过 `GOPATH` 保存项目工程目录，当然可以通过 `:` 分割来设置多个。

在实际使用时，通过会将一些常用的三方库保存在 `GOPATH` 的第一个目录下。

所以，此时的目录一般为。

{% highlight text %}
WORKSPACE
 |-src/github.com/hello/world   引用的三方库
 |    /foobar                   项目实现的代码
 |    /foobar/mymath            项目子模块
 |-bin
 |-pkg
{% endhighlight %}

其中，在 `foobar` 目录下保存的是项目目录，而如果要引用 `mymath` 模块的代码，需要调用 `import foobar/mymath` 才可以。

然后通过 `go install foobar` 安装，此时会在 `bin/` 目录下生成对应的二进制文件。

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


## Module

到目前为止，仍然要强依赖于 GOPATH 变量的设置，所以要么已经完全设置好了，要么就每个项目维护一个打包脚本，在该脚本中设置相应的环境变量。

<!--
Creating a new module.
Adding a dependency.
Upgrading dependencies.
Adding a dependency on a new major version.
Upgrading a dependency to a new major version.
Removing unused dependencies.
-->

这里实际是强制要求所有的模块遵循语义化版本规则 [Semantic Versioning](https://semver.org/) 。

当主版本号大于等于 `v2` 时，这个模块在 `import` 路径的结尾上必须指定主版本号；也就是说，当主版本号为 `v0` 或者 `v1` 的时候可以省略。

根据语义化版本的要求，v0 是不需要保证兼容性的，可以随意的引入破坏性变更，所以不需要显式的写出来；而省略 v1 更大程度上是为了兼容现有的代码库，很少有版本会超过 v2 。


### 0. 准备环境

在使用之前，首先设置如下的环境变量。

{% highlight text %}
export GO111MODULE=on
export GOPROXY=http://cmc-cd-mirror.rnd.huawei.com/goproxy/
export GONOSUMDB=*
{% endhighlight %}

其中 `GO111MODULE` 打开模块支持，忽略 `GOPATH` 以及 `vendor` 目录；`GONOSUMDB` 默认下载完依赖模块后，会检查其校验值，默认是 `https://sum.golang.org` ，这里过滤掉所有的包。

校验地址可以通过 `GOSUMDB` 指定地址及其公钥值，与此相关的还有 `GOPRIVATE` `GONOPROXY` 几个环境变量。

### 1. 创建新模块

在目录 `/tmp/HelloWorld` 目录下创建如下文件。

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

注意，在 `go mod init` 过程中，会在 `$GOPATH` 目录下创建一个子目录 `pkg/mod/cache` ，来缓存版本信息，需要保证该目录有权限。

完成初始化之后会新建一个 `go.mod` 文件。

{% highlight text %}
$ cat go.mod
module example.com/hello

go 1.13
{% endhighlight %}

改文件只会出现在模块的顶层，如果在子目录下引入包，那么即使子目录+包名称。

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

#### Semantic Import Versioning

详细可以参考 [Semantic Import Versioning](https://research.swtch.com/vgo-import) 中的相关介绍，这里只是对其关键信息的摘录。

主要是为了解决，当前项目引入了一个非兼容的包之后如何进行处理，而 go 的原则是，只要是相同的路径，对应的包就是向前兼容的，对于不兼容的包，则通过 `vN` 版本号解决。

这样带来的好处是，对于不兼容的接口，允许代码完成灰度的升级替换。

<!--
## Minimal Version Selection

https://xuanwo.io/2019/05/27/go-modules/

## 最小版本选择

go list -m -versions github.com/sirupsen/logrus
-->

<!--
go.mod 提供了module, require、replace和exclude 四个命令

module  语句指定包的名字（路径）
require 语句指定的依赖项模块
replace 语句可以替换依赖项模块
exclude 语句可以忽略依赖项模块


添加依赖

新建一个 server.go 文件，写入以下代码：


package main

import (
	"net/http"

	"github.com/labstack/echo"
)

func main() {
	e := echo.New()
	e.GET("/", func(c echo.Context) error {
		return c.String(http.StatusOK, "Hello, World!")
	})
	e.Logger.Fatal(e.Start(":1323"))
}
复制代码执行 go run server.go 运行代码会发现 go mod 会自动查找依赖自动下载：
$ go run server.go
go: finding github.com/labstack/echo v3.3.10+incompatible
go: downloading github.com/labstack/echo v3.3.10+incompatible
go: extracting github.com/labstack/echo v3.3.10+incompatible
go: finding github.com/labstack/gommon/color latest
go: finding github.com/labstack/gommon/log latest
go: finding github.com/labstack/gommon v0.2.8
# 此处省略很多行
...

   ____    __
  / __/___/ /  ___
 / _// __/ _ \/ _ \
/___/\__/_//_/\___/ v3.3.10-dev
High performance, minimalist Go web framework
https://echo.labstack.com
____________________________________O/_______
                                    O\
⇨ http server started on [::]:1323
复制代码现在查看go.mod 内容：
$ cat go.mod

module hello

go 1.12

require (
	github.com/labstack/echo v3.3.10+incompatible // indirect
	github.com/labstack/gommon v0.2.8 // indirect
	github.com/mattn/go-colorable v0.1.1 // indirect
	github.com/mattn/go-isatty v0.0.7 // indirect
	github.com/valyala/fasttemplate v1.0.0 // indirect
	golang.org/x/crypto v0.0.0-20190313024323-a1f597ede03a // indirect
)
复制代码go module 安装 package 的原則是先拉最新的 release tag，若无tag则拉最新的commit，详见 Modules官方介绍。 go 会自动生成一个 go.sum 文件来记录 dependency tree：
$ cat go.sum
github.com/labstack/echo v3.3.10+incompatible h1:pGRcYk231ExFAyoAjAfD85kQzRJCRI8bbnE7CX5OEgg=
github.com/labstack/echo v3.3.10+incompatible/go.mod h1:0INS7j/VjnFxD4E2wkz67b8cVwCLbBmJyDaka6Cmk1s=
github.com/labstack/gommon v0.2.8 h1:JvRqmeZcfrHC5u6uVleB4NxxNbzx6gpbJiQknDbKQu0=
github.com/labstack/gommon v0.2.8/go.mod h1:/tj9csK2iPSBvn+3NLM9e52usepMtrd5ilFYA+wQNJ4=
github.com/mattn/go-colorable v0.1.1 h1:G1f5SKeVxmagw/IyvzvtZE4Gybcc4Tr1tf7I8z0XgOg=
github.com/mattn/go-colorable v0.1.1/go.mod h1:FuOcm+DKB9mbwrcAfNl7/TZVBZ6rcnceauSikq3lYCQ=
... 省略很多行
复制代码
再次执行脚本 go run server.go 发现跳过了检查并安装依赖的步骤。
可以使用命令 go list -m -u all 来检查可以升级的package，使用go get -u need-upgrade-package 升级后会将新的依赖版本更新到go.mod
* 也可以使用 go get -u 升级所有依赖

go get 升级

运行 go get -u 将会升级到最新的次要版本或者修订版本(x.y.z, z是修订版本号， y是次要版本号)
运行 go get -u=patch 将会升级到最新的修订版本
运行 go get package@version 将会升级到指定的版本号version
运行go get如果有版本的更改，那么go.mod文件也会更改

示例二：改造现有项目(helloword)
项目目录为：
$ tree
.
├── api
│   └── apis.go
└── server.go

1 directory, 2 files
复制代码server.go 源码为：
package main

import (
    api "./api"  // 这里使用的是相对路径
    "github.com/labstack/echo"
)

func main() {
    e := echo.New()
    e.GET("/", api.HelloWorld)
    e.Logger.Fatal(e.Start(":1323"))
}
复制代码api/apis.go 源码为：
package api

import (
    "net/http"

    "github.com/labstack/echo"
)

func HelloWorld(c echo.Context) error {
    return c.JSON(http.StatusOK, "hello world")
}
复制代码
使用 go mod init *** 初始化go.mod

$ go mod init helloworld
go: creating new go.mod: module helloworld
复制代码
运行 go run server.go

go: finding github.com/labstack/gommon/color latest
go: finding github.com/labstack/gommon/log latest
go: finding golang.org/x/crypto/acme/autocert latest
go: finding golang.org/x/crypto/acme latest
go: finding golang.org/x/crypto latest
build command-line-arguments: cannot find module for path _/home/gs/helloworld/api
复制代码首先还是会查找并下载安装依赖，然后运行脚本 server.go，这里会抛出一个错误：
build command-line-arguments: cannot find module for path _/home/gs/helloworld/api
复制代码但是go.mod 已经更新：
$ cat go.mod
module helloworld

go 1.12

require (
        github.com/labstack/echo v3.3.10+incompatible // indirect
        github.com/labstack/gommon v0.2.8 // indirect
        github.com/mattn/go-colorable v0.1.1 // indirect
        github.com/mattn/go-isatty v0.0.7 // indirect
        github.com/valyala/fasttemplate v1.0.0 // indirect
        golang.org/x/crypto v0.0.0-20190313024323-a1f597ede03a // indirect
)
复制代码那为什么会抛出这个错误呢？
这是因为 server.go 中使用 internal package 的方法跟以前已经不同了，由于 go.mod会扫描同工作目录下所有 package 并且变更引入方法，必须将 helloworld当成路径的前缀，也就是需要写成 import helloworld/api，以往 GOPATH/dep 模式允许的 import ./api 已经失效，详情可以查看这个 issue。

更新旧的package import 方式

所以server.go 需要改写成：
package main

import (
    api "helloworld/api"  // 这是更新后的引入方法
    "github.com/labstack/echo"
)

func main() {
    e := echo.New()
    e.GET("/", api.HelloWorld)
    e.Logger.Fatal(e.Start(":1323"))
}
复制代码
一个小坑：开始在golang1.11 下使用go mod 遇到过 go build github.com/valyala/fasttemplate: module requires go 1.12 这种错误，遇到类似这种需要升级到1.12 的问题，直接升级golang1.12 就好了。幸亏是在1.12 发布后才尝试的go mod 🤷‍♂️


到这里就和新创建一个项目没什么区别了

使用replace替换无法直接获取的package
由于某些已知的原因，并不是所有的package都能成功下载，比如：golang.org下的包。
modules 可以通过在 go.mod 文件中使用 replace 指令替换成github上对应的库，比如：
replace (
	golang.org/x/crypto v0.0.0-20190313024323-a1f597ede03a => github.com/golang/crypto v0.0.0-20190313024323-a1f597ede03a
)
复制代码或者
replace golang.org/x/crypto v0.0.0-20190313024323-a1f597ede03a => github.com/golang/crypto v0.0.0-20190313024323-a1f597ede03a

当执行 `go build` 或者 `go test` 时，会自动下载相关的包，并更新 `go.mod` 文件，如果需要特定的版本，那么可以通过如下方式指定。

go get foobar@v1.2.3
go get foobar@master
go get foobar@e3702bed2

也可以直接编辑 `go.mod` 文件。

开启了 Module 之后，不在原来原有的 `GOPATH` 机制，下载的包会保存在 `

在开启了 Module 机制后，其中 `go get` 的机制重新实现，分别对应了如下的代码。

----- 不开启
${GOROOT}/src/cmd/go/internal/get/get.go
----- 开启
${GOROOT}/src/cmd/go/internal/modget/get.go

希望一个项目就在一个目录下，而不是类似 `src/github.com/foobar` 这样的目录。

在 GoLang 中分为了三个层级：Module、Package、Files ，在如上的讨论中大部分都是基于 Package 的，在下面的讨论中是基于 Module 的。

一般一个 Module 对应了一个仓库，当然，一个仓库中也允许存在多个 Module ，只是略复杂。

https://github.com/golang/go/wiki/Modules

### Replace

因为网络等原因，有些包无法通过 `go get` 获取，此时就需要用到 `replace` 指令。

src/github.com/foobar/saying  引用
src/github.com/foobar/demo    主项目

$ export GOPATH=/tmp/foobar
$ mkdir -p "${GOPATH}/src/github.com/foobar/saying"
$ cd "${GOPATH}/src/github.com/foobar/saying"
$ go mod init
$ cat <<EOF > saying.go
package saying

import "fmt"

func Greet(name string) string {
	return fmt.Sprintf("Hi, %s!", name)
}
EOF

$ mkdir -p "${GOPATH}/src/github.com/foobar/demo"
$ cd "${GOPATH}/src/github.com/foobar/demo"
$ go mod init
$ cat <<EOF > main.go
package  main

import (
    "fmt"

    "github.com/foobar/saying"
)

func main(){
	fmt.Println(saying.Greet("GoModule"))
}
EOF

然后在 `demo` 的目录下通过 `go run main.go` 直接运行，此时会尝试下载 `saying` 模块，但是因为没有实际上传，最终会下载失败。

手动修改 `go.mod` 文件为如下，其中 `require` 为关键。

module github.com/foobar/demo

go 1.13

require github.com/foobar/saying v0.0.0
replace github.com/foobar/saying => ../saying

注意，如果有多个版本，目前不会从本地获取相关的版本。

### 多版本

http://ljchen.net/2018/11/24/Go-Modules%E4%BE%9D%E8%B5%96%E7%AE%A1%E7%90%86/


https://juejin.im/post/5c8e503a6fb9a070d878184a
-->


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

<!--
/post/golang-basic-package-introduce.html
从 v1.5 开始开始引入 vendor 包模式，如果项目目录下有 vendor 目录，那么 go 工具链会优先使用 vendor 内的包进行编译、测试等。

实际上，这之后第三方的包管理思路都是通过这种方式来实现，比如说由社区维护准官方包管理工具 dep ，不过官方不认可。

在 v1.11 中加入了 Go Module 作为官方包管理形式，在 v1.11 和 v1.12 版本中 gomod 不能直接使用，可以执行 `go env` 命令查看是否有 GOMOD 判断是否已开启。

如果没有开启，可以通过设置环境变量 `export GO111MODULE=on` 开启。

----- 查看所有依赖
go list -u -m all

当使用 modules 时，会完全忽略原有的 vendor 机制。

## sync 扩展

官方的 sync 包，提供了基础的 Map、Mutex、WaitGroup、Pool 等功能的支持。

在基础的 sync 包的基础上，官方还提供了一个高效的扩展包 golang.org/x/sync，包括了 errgroup、semaphore、singleflight、syncmap 等工具。

这里简单介绍其使用方法，以及部分实现原理。

Shell的变量替换
https://www.cnblogs.com/fhefh/archive/2011/04/22/2024750.html

这里使用的是 Go 1.13 版本。

假设将官方的库安装到 `/opt/golang` 目录下，常用的三方库保存在 `/opt/golang/vendor` 目录下，在 `/etc/profile` 文件中添加如下内容。

export GOPATH=/opt/golang/vendor
export GOROOT=/opt/golang
pathmunge "${GOROOT}/bin"
pathmunge "${GOPATH}/bin"

这样，可以确保所有的 Go 版本保存在 `$GOROOT` 中，通用三方包保存在 `$GOPATH/src` 目录下。

go install github.com/jstemmer/gotags
https://github.com/jstemmer/gotags/releases

#!/bin/bash

#REPO_PATH="foobar.com/foobar"
REPO_PATH="foobar"

project_build() {
        out="bin"
        go build foobar
}

pathmunge() {
        if [[ -z "${GOPATH}" ]]; then
                GOPATH=$1
                return
        fi

        case ":${GOPATH}:" in
        *:"$1":*)
                ;;
        *)
                if [[ "$2" = "after" ]] ; then
                        GOPATH=${GOPATH}:$1
                else
                        GOPATH=$1:${GOPATH}
                fi
        esac
}

project_setup_gopath() {
        DIR=$(dirname "$0")
        CDIR=$(cd "${DIR}" && pwd)
        cd "${CDIR}"

        PRG_GOPATH="${CDIR}/gopath"
        if [[ -d "${PRG_GOPATH}" ]]; then
                rm -rf "${PRG_GOPATH:?}/"
        fi
        mkdir -p "${PRG_GOPATH}"

        pathmunge "${PRG_GOPATH}"
        echo "Current GOPATH=${GOPATH}"
        ln -s "${CDIR}/vendor" "${PRG_GOPATH}/src"
        if [[ ! -L "${CDIR}/vendor/${REPO_PATH}" ]]; then
                ln -s "${CDIR}" "${CDIR}/vendor/${REPO_PATH}"
        fi
}

ETCD_SETUP_GOPATH=1

if [[ "${ETCD_SETUP_GOPATH}" == "1" ]]; then
        project_setup_gopath
fi

# only build when called directly, not sourced
if echo "$0" | grep "build$" >/dev/null; then
        project_build
fi
https://n3xtchen.github.io/n3xtchen/go/2018/10/30/go-mod-local-pacakge
http://www.r9it.com/20190611/go-mod-use-dev-package.html
https://www.cnblogs.com/apocelipes/p/10295096.html
https://allenwind.github.io/2017/09/16/Golang%E5%AE%9E%E7%8E%B0%E4%BF%A1%E5%8F%B7%E9%87%8F/
https://yangxikun.com/golang/2017/03/07/golang-singleflight.html
https://segmentfault.com/a/1190000018464029
https://zhuanlan.zhihu.com/p/44585993
https://studygolang.com/articles/22525
https://github.com/golang/sync/tree/master/syncmap
https://blog.csdn.net/mrbuffoon/article/details/85263480
https://gocn.vip/question/161
https://zhuanlan.zhihu.com/p/64983626
https://blog.csdn.net/jiankunking/article/details/78818953
https://medium.com/@deckarep/gos-extended-concurrency-semaphores-part-1-5eeabfa351ce
-->

{% highlight text %}
{% endhighlight %}
