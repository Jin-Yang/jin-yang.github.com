---
title: GoLang 命令行参数解析
layout: post
comments: true
language: chinese
tag: [Program, GoLang]
keywords: golang,flag,模块,命令行,参数解析
description: 绝大部分语言在标准库中都已经提供了对命令行参数的解析，GoLang 其参数保存在 `os.Args` 变量中，而解析则通过标准库 `flag` 实现，这里简单介绍其使用方法。
---

绝大部分语言在标准库中都已经提供了对命令行参数的解析，GoLang 其参数保存在 `os.Args` 变量中，而解析则通过标准库 `flag` 实现，这里简单介绍其使用方法。

<!-- more -->

## 命令行参数

在 `os` 包中有一个 string 类型的切片变量 `os.Args` ，其中 `os.Args[0]` 放的是程序本身的名字，所以实际参数为 `os.Args[1:]` 。

```
fmt.Println("Parameters:", os.Args[1:])
```

## flag 库

可以通过标准库中 `flag` 库作为命令行入参的校验，其定义的变量类型是指针，获取对应值的时候需要添加引用 `*` 。

{% highlight go %}
package main

import (
        "flag"
        "fmt"
)

var (
        name   = flag.String("name", "nick", "Input Your Name")
        age    = flag.Int("age", 28, "Input Your Age")
        gender = flag.String("gender", "male", "Input Your Gender")
)
var address string

func main() {
        flag.StringVar(&address, "address", "China", "Your Address")
        flag.Parse()

        fmt.Printf("args=%s, num=%d\n", flag.Args(), flag.NArg())
        for i := 0; i != flag.NArg(); i++ {
                fmt.Printf("arg[%d]=%s\n", i, flag.Arg(i))
        }

        fmt.Println("name=", *name)
        fmt.Println("age=", *age)
        fmt.Println("gender=", *gender)
        fmt.Println("address=", address)
}
{% endhighlight %}

在命令行中，可以使用如下的参数之一 `-age=XXX` `-age XXX` `--age=XXX` `--age XXX` 。

