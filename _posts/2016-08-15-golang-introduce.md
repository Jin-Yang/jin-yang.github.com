---
title: Golang 简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,简介
description: golang 目前越来越火，包括了 Docker、InfluxDB、etcd 等等，越来越多的工程都在使用 golang 。虽然它的语法都比较 "奇葩"，一些高级语言支持的特性却都不支持，但其高并发、高效率等等，也使其热度不减。这篇文章主要简单介绍下 golang 环境的搭建。
---

golang 目前越来越火，包括了 Docker、InfluxDB、etcd 等等，越来越多的工程都在使用 golang 。虽然它的语法都比较 "奇葩"，一些高级语言支持的特性却都不支持，但其高并发、高效率等等，也使其热度不减。

这篇文章主要简单介绍下 golang 环境的搭建。

<!-- more -->

![golang logo]({{ site.url }}/images/go/golang-logo.png "golang logo"){: .pull-center width="60%" }

## 环境安装

golang 有多种安装方式，主要包括了三种最常见的安装方式：

* 第三方工具。也就是一些常见操作系统的软件管理工具，如 Ubuntu 的 apt-get、CentOS 的 yum、Mac 的 homebrew 等，还有就是类似 GVM 的工具。

* Go 标准包安装。官网也提供了已经编译好的二进制安装包，支持 Windows、Linux、Mac等系统，包括 X86、ARM 等平台。

* Go 源码安装。对于经常使用 `*nix` 类系统的用户，尤其对于开发者来说，这种方式比较熟悉，而且从源码安装可以自己进行定制。

其中，上面提到了一个 GVM 工具，如果想在同一个系统中安装多个版本，建议可以使用该工具。

### YUM 安装

在 CentOS 7 中可以通过 `yum list all | grep golang` 查看、安装，该二进制包在 base 中。

{% highlight text %}
# yum install golang
{% endhighlight %}

### 标准包安装

可以从 [golang.org/dl](https://golang.org/dl) 上下载源码包，当然也可以从 [golangtc.com](http://golangtc.com/download)、[studygolang.com](https://studygolang.com/) 上下载。其中包的名称类似于 go1.6.linux-amd64.tar.gz ，从中选择相应的平台，压缩包中是已经编译好的二进制包，可以直接下载解压即可。

### 源码安装

直接下载源码安装，之前的时候需要 gcc 编译，现在基本可以通过 go 搞定了。

#### I. 下载源码

go 的 [官方页面](http://www.golang.org/) 之前打不开，可以从 [Golang 中国](http://golangtc.com/) 上下载，不过最近发现好像是放开了。

#### II. 设置环境变量

通常可以在 `~/.bashrc` 中设置如下的环境变量，然后通过 `source ~/.bashrc` 使其立即生效。

{% highlight text %}
export GOPATH=$HOME/Workspace/go             ← 可以指定多个通过:分隔
export GOROOT=${GOPATH}/goroot               ← 默认在/usr/local/go，否则需要指定
#export GOBIN=$GOROOT/bin                    ← 指定默认bin目录
#export GOARCH=386
#export GOOS=linux
export PATH=${PATH}:${GOBIN}
{% endhighlight %}

#### III. 编译源码

如果下载的是只是源码，可以通过 `./all.bash` 进行编译，也可以下载包含可执行文件的 tarball 包。

{% highlight text %}
$ go env                                     ← 查看环境变量
$ go version                                 ← 查看版本
{% endhighlight %}

#### IV. 其它

<!--
-x 列出go build调用到的所有命令
-->

例如交叉编译可以使用：

{% highlight text %}
$ CGO_ENABLED=0 GOOS=windows GOARCH=386 go build test.go
参数：
	CGO_ENABLED 指明cgo工具是否可用
	GOOS        程序构建的目标操作系统，例如 linux windows
	GOARCH      程序构建环境的目标计算架构，例如 32bits-386 64bits-amd64
{% endhighlight %}

通过 `-gcflags` 指定编译的选项，例如 `go build -gcflags '-N -l'`，常见参数有：

{% highlight text %}
-N 禁止编译优化
-l 禁止内联，一定程度上减小可执行程序大小，不过可能增加执行时间
{% endhighlight %}

可以使用 `go tool compile --help` 查看 `gcflags` 各参数含义。

可以通过 `-ldflags` 指定的链接选项，例如 `go build -ldflags '-w -s'`，常见的有：

{% highlight text %}
-w 禁止生成debug信息，此时不方便gdb调试
-s 禁用符号表
{% endhighlight %}

更多可以通过 `go tool link --help` 查看 `ldflags` 各参数含义。

### 最佳实践

一般来说，可以将已经编译好的二进制保存到 `/usr/local/golang` ，然后设置 `GOROOT` 为上述值。

对于三方包，可以保存到个人目录下，例如 `~/Workspace/golang` 目录下，此时就需要将 `GOPATH` 设置为该值，此时将三方包存放到 `${GOPATH}/src` 目录下即可。

注意，此时通过 `go get` 等命令安装的包都会保存在 `GOPATH` 指定的目录下，也就是说，`GOROOT` 中保存的是原始的包，`GOPATH` 中保存新安装的三方包、二进制等。

## 示例代码

### Hello world

保存如下的内容为 `hello.go` 文件，然后通过 `go run hello.go` 执行，此时会输出 `Hello World!` 。

{% highlight go %}
package main
import "fmt"
func main() {
    fmt.Printf("Hello World!\n")
}
{% endhighlight %}

当然可以用更简单的。

{% highlight go %}
package main
func main() {
    println("Hello World!")
}
{% endhighlight %}

另外，在源码目录下，可以直接通过 ```go build``` 进行编译，此时将编译该目录下所有的源码，并生成与 **目录名称** 相同的二进制文件。

当然，也可以在源码目录下执行 ```go install``` 进行安装；或者将上述代码放到 ```$GOPATH/src/examples/hello``` 目录下，然后执行 ```go install examples/hello``` 即可，会编译安装到 `$GOPATH/bin` 目录下。

### 单元测试

<!--
go test -v

package testing101
func Sum(numbers []int) int {
	sum :=0
	for _,n :=range numbers {
		sum +=n
	}
	return sum
}

3 测试程序

package testing101

import (
	"testing"
)

func TestSum(t *testing.T) {
	numbers := []int{1, 2, 3, 4, 5}
	expected := 15
	actual := Sum(numbers)

	if actual != expected {
		t.Errorf("Expected the sum of %v to be %d but instead got %d!", numbers, expected, actual)

	}
}
-->


## 配置vim

通过 Vundle 安装 `Plugin 'fatih/vim-go'` 也就是 [fatih/vim-go](https://github.com/fatih/vim-go)，此时会安装高亮显示代码，不过还需要通过 `:GoInstallBinaries` 命令安装相关的二进制文件，此时会安装在 `$GOPATH/bin` 目录下。

最早之前代码都是维护在 code.google 上的，现在大多在 github 上，可以直接查看 github 上的相关文档，对于安装失败的可以直接 google 之。

当直接通过 `:GoInstallBinaries` 命令安装时，由于 `golang.org` 被墙，会导致很多命令安装失败，此时可以下载 `golang.org/x/tools` 然后放置到源码目录下，通过 `go install` 安装。

可以通过 [gopm.io](https://gopm.io/) 或者 [github.com/golang](https://github.com/golang) 下载。

<!--
http://studygolang.com/articles/1785   vim配置
http://golanghome.com/post/550         Go编码规范指南
http://www.yankay.com/go-clear-concurreny/
-->


<!--
瘦身<br>
go 采用静态编译，编译后的文件会很大，可以通过 go build -ldflags "-s -w" 这种方式编译。

首先我们看一下为什么会比其他语言大些：

Go 编译的可执行文件都包含了一个运行时(runtime)，和我们习惯的Java/.NET VM有些类似。

运行时负责内存分配（Stack Handing、GC Heap）、垃圾回收（Garbage Collection）、Goroutine调度（Schedule）、引用类型（slice、map、channel）管理，以及反射（Reflection）等工作。Go程序进程启动后会自动创建两个goroutine，分别用于执行main入口函数和GC Heap管理。

也正是因为编译文件中嵌入了运行时，使得其可执行文件相较其他语言更大一些。但Go的二进制可执行文件都是静态编译的，无需其他任何链接库等文件，更利于分发。

我们可以通过下面的方法将其变小点：

采用：go build -ldflags "-s -w" 这种方式编译。

解释一下参数的意思：

-ldflags： 表示将后面的参数传给连接器（5/6/8l）
-s：去掉符号信息
-w：去掉DWARF调试信息

注意：
-s 去掉符号表（这样panic时，stack trace就没有任何文件名/行号信息了，这等价于普通C/C+=程序被strip的效果）
-w 去掉DWARF调试信息，得到的程序就不能用gdb调试了
两个可以分开使用
实际项目中不建议做这些处理，没啥必要。

-s去掉符号表（然后panic时候的stack trace就没有任何文件名/行号信息了，这个等价于普通C/C++程序被strip的效果），
-w去掉DWARF调试信息，得到的程序就不能用gdb调试了。
-s和-w也可以分开使用，一般来说如果不打算用gdb调试，
-w基本没啥损失。-s的损失就有点大了。

</li></ol>
相关的文档，可以参考安装包的 doc 目录下的内容。
</p>

-->



## 环境变量

`go build` 和 `go install` 命令会尝试匹配目录而非源码文件，会查找 `$GOROOT/src` 和 `$GOPATH/src` 目录的同名目录。例如，代码保存在 `/tmp/test/src/foobar` 目录下，可以通过如下方式编译。

{% highlight text %}
$ GOPATH=$GOROOT:/tmp/test go run main.go
{% endhighlight %}

介绍下一些常见的环境变量使用方法，可以通过 go env 查看当前的环境变量。

### GOROOT

指定 golang 的安装路径，如果通过 yum 安装，可以使用系统默认的值，此时直接通过 unset GOROOT 取消该环境变量即可。

### GOBIN

install 编译存放路径，不允许设置多个路径。可以为空，此时可执行文件会保存到 $GOPATH/bin 目录下。

### GOPATH

唯一一个 **必须设置** 的环境变量，GOPATH 的作用是告诉 Go 命令和其他相关工具，在那里去找到安装在你系统上的 Go 包。这是一个路径的列表，一个典型的 GOPATH 设置如下，类似 PATH 的设置，Windows 下用分号分割。

{% highlight text %}
export GOPATH=$HOME/go:$HOME/go/gopath
export PATH=$PATH:${GOPATH//://bin:}/bin
{% endhighlight %}

最后一条，在 Linux/Mac 中把每个 GOPATH下 的 bin 都加入到 PATH 中。

每一个列表中的路径是一个工作区的位置，每个工作区都有源文件、相关包的对象、执行文件。GOPATH 必须设置编译和安装包，即使用标准的 Go 目录树，类似如下：

{% highlight text %}
|-- bin/
|   `-- foobar               (installed command)
|-- pkg/
|   `-- linux_amd64/
|       `-- foo/
|           `-- bar.a        (installed package object)
`-- src/
    `-- foo/
        |-- bar/             (go code in package bar)
        |   `-- bar.go
        `-- foobar/          (go code in package main)
            `-- foobar.go
{% endhighlight %}


### GOROOT VS. GOPATH

GOROOT 和 GOPATH 是需要进行设置的变量，如果不设置，会尝试自动获取，其规则为：

* GOROOT 自动从 go 执行文件目录去除 `bin` 即可，也就是说，只要保证 go 命令在 `PATH` 环境变量中即可；
* GOPATH 用来保存三方包目录，默认会设置成 `$HOME/go`。

容易出错的是，将 `GOPATH` 设置为了 `$GOROOT`，此时如果使用 `go get` 会有如下报错：

{% highlight text %}
cannot download, $GOPATH must not be set to $GOROOT. For more details see: 'go help gopath'
{% endhighlight %}

实际上，GoLang 定义了很多内置的标准，如果发现 `$GOPATH/src` 包含了标准包，则会报上述的错误，也就是 `$GOROOT/src` 中包含的是标准包，而 `$GOPATH/src` 包含的是三方包。

也即是说，约束为 A) `$GOROOT` 不能等于 `$GOPATH` ；B) `$GOPATH` 不能是标准包的路径。

<!--
需要注意的是，GROOT 和 GOPATH **可以设置为不同的路径** 。
-->

### Go Vendor

在 1.6 版本之后新增了一个 vendor 目录，此时依赖包的查找顺序为：

* 当前目录是否存在 vendor 目录。
* 向上级目录查找，直到找到 src下的 vendor目录。
* 在 GOPATH 下面查找依赖包。
* 在 GOROOT 目录下查找。

建议在项目中，只在代码库的顶级目录下保存一个 vendor 目录，如果出现如下目录，则会报错：

{% highlight text %}
- main.go
- vendor/
  - foo/
  - bar/
    - vendor/foo/
{% endhighlight %}

这里会将两个 `foo` 包认为是一样的，那么在执行 `go build` 时就会报错。

## 常用命令

简单列举一下一些常用的工具。

{% highlight text %}
$ go version            # 当前的版本
$ go env                # 环境变量
$ go list               # 列出全部已安装的package
$ go run                # 编译并运行Go程序
$ go help               # 查看帮助
$ go get                # 下载源码包+安装
$ go install            # 安装，需要保证源码已经下载
{% endhighlight %}

<!--
go fix 用来修复以前老版本的代码到新版本，例如go1之前老版本的代码转化到go1
-->

### go build

用于编译，在包的编译过程中，若有必要，会同时编译与之相关联的包。

<!--
    如果是普通包，当你执行go build命令后，不会产生任何文件。

    如果是main包，当只执行go build命令后，会在当前目录下生成一个可执行文件。如果需要在$GOPATH/bin木下生成相应的exe文件，需要执行go install 或者使用 go build -o 路径/a.exe。

    如果某个文件夹下有多个文件，而你只想编译其中某一个文件，可以在 go build 之后加上文件名，例如 go build a.go；go build 命令默认会编译当前目录下的所有go文件。

    你也可以指定编译输出的文件名。比如，我们可以指定go build -o myapp.exe，默认情况是你的package名(非main包)，或者是第一个源文件的文件名(main包)。

    go build 会忽略目录下以”_”或者”.”开头的go文件。

    如果你的源代码针对不同的操作系统需要不同的处理，那么你可以根据不同的操作系统后缀来命名文件。例如有一个读取数组的程序，它对于不同的操作系统可能有如下几个源文件：

array_linux.go
array_darwin.go
array_windows.go
array_freebsd.go

go build的时候会选择性地编译以系统名结尾的文件（Linux、Darwin、Windows、Freebsd）。例如Linux系统下面编译只会选择array_linux.go文件，其它系统命名后缀文件全部忽略。
-->

### go clean

用来移除当前源码包里面编译生成的文件。

{% highlight text %}
{% endhighlight %}

<!--
，这些文件包括：
    _obj/ 旧的object目录，由Makefiles遗留
    _test/ 旧的test目录，由Makefiles遗留
    _testmain.go 旧的gotest文件，由Makefiles遗留
    test.out 旧的test记录，由Makefiles遗留
    build.out 旧的test记录，由Makefiles遗留
    *.[568ao] object文件，由Makefiles遗留
    DIR(.exe) 由 go build 产生
    DIR.test(.exe) 由 go test -c 产生
    MAINFILE(.exe) 由 go build MAINFILE.go产生
-->

### go fmt

主要是用来帮你格式化所写好的代码文件，只需要简单执行 fmt go test.go 命令，就可以让 go 帮我们格式化我们的代码文件。

其中通过 -w 可以将格式化结果保存到文件中，当然可以使用目录作为参数，格式化整个工程。

### go get

用来动态获取远程代码包的，目前支持的有 BitBucket、GitHub、Google Code 和 Launchpad。

这个命令在内部实际上分成了两步操作：1) 下载源码包；2) 执行 go install。下载源码包的 go 工具会自动根据不同的域名调用不同的源码工具，当然必须确保安装了合适的源码管理工具。

另外，其实 go get 支持自定义域名的功能，具体参见 go help remote 。

### go install

该命令在内部实际上分成了两步操作：1) 生成目标文件，可执行文件或者 .a 包；2) 把编译好的结果移到 $GOPATH/pkg 或者 $GOPATH/bin 目录下。

### go test

该命令会自动读取源码目录下面名为 *_test.go 的文件，生成并运行测试用的可执行文件，输出的信息类似：

{% highlight text %}
ok   archive/tar   0.011s
FAIL archive/zip   0.022s
ok   compress/gzip 0.033s
{% endhighlight %}

默认的情况下，不需要任何的参数，它会自动把你源码包下面所有 test 文件测试完毕，当然你也可以带上参数，详情请参考 go help testflag 。

### go doc

go doc 命令其实就是一个很强大的文档工具。

{% highlight text %}
$ go doc builtin                  # 查看builtin文档
$ go doc net/http                 # http包的报文帮助
$ go doc fmt Printf               # 查看包中的某个函数
$ go doc -http=:8080              # 浏览器打开127.1:8080
{% endhighlight %}

通过 -http 参数，将会看到一个 golang.org 的本地 copy 版本，通过它你可以查看 pkg 文档等其它内容。设置了GOPATH 后，不但会列出标准包的文档，还会列出你本地 GOPATH 中所有项目的相关文档，这对于经常被限制访问的用户来说是一个不错的选择。

<!--
$ go doc -src fmt Printf           # 查看相应的代码
-->


## 三方包

简单以安装 [GoLang Colorized Output](https://github.com/bclicn/color) 包为例。

{% highlight text %}
$ go get github.com/bclicn/color
{% endhighlight %}

也可以手动下载源码，放到 `$GOPATH/src/github.com/bclicn/color` 目录下，然后直接执行 `go install` 命令。

如果没有下载源码，可能会导致如下的报错。

{% highlight text %}
$ go install color
can't load package: package color: cannot find package "color" in any of:
        $GOROOT/goroot/src/color (from $GOROOT)
        $GOPATH/src/color (from $GOPATH)
{% endhighlight %}

不过由于万能的 XXX 导致很多依赖包下载失败，常见的是 `golang.org/x` 包，此时可以从 [GitHub GoLang](https://github.com/golang) 上下载，例如 [Github golang/tools](https://github.com/golang/tools) 就是工具包，可以直接将上述的代码仓库下载下来，然后放到 `$GOPATH/golang.org/x/tools` 目录下即可。


## 参考

官方网站 golang.org 被墙，常见的工具可以从 [Gopm Registry](https://gopm.io/) 上下载，而文档资料等可以从 [Golang 中国](http://golangtc.com/) 上查看。

另外，[Github - Golang](https://github.com/golang) 提供了很多 golang.org/x/ 的镜像包，只需要下载并保存到 $GOPATH/src 目录下。

{% highlight text %}
{% endhighlight %}
