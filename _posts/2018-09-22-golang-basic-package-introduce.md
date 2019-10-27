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

### Module

到目前为止，仍然要强依赖于 GOPATH 变量的设置，所以要么已经完全设置好了，要么就每个项目维护一个打包脚本，在该脚本中设置相应的环境变量。

<!--
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
