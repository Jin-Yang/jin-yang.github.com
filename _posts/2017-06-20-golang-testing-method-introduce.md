---
title: Golang 单元测试
layout: post
comments: true
language: chinese
category: [program]
keywords:
description:
---

程序开发中比较重要的一点是如何可以高效的进行单元测试，可以保证快速发现定位问题。

在 GoLang 中自带了一个轻量级的测试框架 `testing` 以及 `go test` 命令来实现单元测试和性能测试。

<!-- more -->

## 简介

假设有一个简单的除法实现，对应文件 `math.go` 。

{% highlight go %}
package math

import (
        "errors"
)

func Division(a, b float64) (float64, error) {
        if b == 0 {
                return 0, errors.New("divisor should not be 0")
        }

        return a / b, nil
}
{% endhighlight %}

与之对应的单元测试文件 `math_test.go` 为。

{% highlight go %}
package math

import "testing"

func TestDivision(t *testing.T) {
        if rc, err := Division(6, 2); rc != 3 || err != nil {
                t.Fatal("invalid division result.")
        } else {
                t.Log("division test pass.")
        }
}

func TestDivisionFail(t *testing.T) {
        if _, err := Division(6, 0); err == nil {
                t.Error("division did not work as expected.")
        } else {
                t.Log("divisor 0 pass.", err)
        }
}
{% endhighlight %}

那么对应的单元测试的示例如下，需要确保遵循如下的规则：

* 文件名需要以 `_test.go` 结尾，在执行 `go test` 时会自动匹配对应代码；
* 测试文件引入 `testing` 包，且所有的测试用例必须以 `Test` 开头；
* 可以通过测试函数的入参 `t *testing.T` 来标记错误状态信息；

在测试函数中，可以通过调用 `testing.T` 中定义的相关方法来标记测试信息。

* `Fail()` 标记失败，但继续执行当前测试函数；
* `FailNow()` 失败，立即终止当前测试函数执行；
* `Log()` 输出错误或者调试信息；
* `Error()` 等价于 `Fail + Log` ；
* `Fatal()` 等价于 `FailNow + Log` ；
* `Skip()` 跳过当前函数，通常用于未完成的测试用例。

然后可以通过 `go test` 执行测试，如果通过，默认只会打印通过的信息，可以通过 `go test -v` 查看详情。

{% highlight text %}
$ go test -v
=== RUN   TestDivision
--- PASS: TestDivision (0.00s)
        math_test.go:9: division test pass.
=== RUN   TestDivisionFail
--- PASS: TestDivisionFail (0.00s)
        math_test.go:17: divisor 0 pass. divisor should not be 0
PASS
ok      _/workspace/golang/math      0.002s
{% endhighlight %}

在执行时，也可以通过 `-run="TestTwo"` 运行具体的测试用例。

## 性能测试

除了上述的单元测试，还可以通过该模块执行一些简单的压力测试，默认不会执行性能测试，可以通过 `-test.bench=".*"` 执行对应的压力测试用例。

例如，在 `math_test.go` 文件中，同时添加性能测试代码。

{% highlight go %}
package math

import "testing"

func BenchmarkDivision(b *testing.B) {
        for i := 0; i < b.N; i++ {
                Division(4, 5)
        }
}

func BenchmarkDivisionConsuming(b *testing.B) {
        b.StopTimer()
        // some init stuff.
        b.StartTimer()
        for i := 0; i < b.N; i++ {
                Division(4, 5)
        }
}
{% endhighlight %}

在如上的性能测试中，通过 `b.StopTimer()` `b.StartTimer()` 可暂定对时间的计时，在这中间可以做些初始化的操作。

{% highlight text %}
$ go test -test.bench=".*"
goos: linux
goarch: amd64
BenchmarkDivision-8             2000000000               0.39 ns/op
BenchmarkDivisionConsuming-8    2000000000               0.39 ns/op
PASS
ok      _/workspace/golang/math      1.655s
{% endhighlight %}

也可以通过 `go test -bench=.` 进行性能测试。

<!--
## pprof

可以用来做些性能测试，并找出瓶颈。

----- 生成测试数据
$ go test -bench=. -cpuprofile cpu.out
----- 通过命令行显示具体的结果
$ go tool pprof -text cpu.out
----- 保存为图片或者PDF
$ go tool pprof -svg cpu.out > cpu.svg
$ go tool pprof -pdf cpu.out > cpu.pdf

另外，对于网络、内存都有相关的工具。

## mock

## 覆盖率
-->


{% highlight go %}
{% endhighlight %}
