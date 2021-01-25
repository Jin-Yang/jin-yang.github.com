---
title: GoLang INI 配置文件解析
layout: post
comments: true
language: chinese
tag: [Program, GoLang]
keywords:
description:
---

INI 是 Initialization File 的缩写，也就是初始化文件，原来是 Windows 系统配置文件所采用的存储格式，不过因为其使用简单，所以后来被广泛应用。

这里介绍如何通过 GoLang 解析。

<!-- more -->

## 简介

INI 配置文件由节 (Section)、健 (Key)、值 (Value) 组成，其中注释一般是以 `;` `#` 开头，尽量放到每行的开始，有些解析器可能会报错，示例如下。

``` ini
; Here is comments.
[Section]
Key=Value
```

这里介绍 [github.com/go-ini/ini](https://github.com/go-ini/ini) 的使用方式，可以参考官网 [ini.unknwon.io](https://ini.unknwon.io/) 中的介绍。

通常是使用 `gopkg.in/ini.v1` ，也就是指定一个版本，如果要使用最新的版本，可以引用 `github.com/go-ini/ini` 。

### 示例

如下是一个配置文件。

``` ini
# possible values : production, development
app_mode = development

[paths]
# Path to where grafana can store temp files, sessions, and the sqlite3 db (if that is used)
data = /home/git/grafana

[server]
# Protocol (http or https)
protocol = http

# The http port  to use
http_port = 9999

# Redirect to correct domain if host header does not match domain
# Prevents DNS rebinding attacks
enforce_domain = true
```

然后可以通过如下代码解析。

``` go
package main

import (
    "fmt"
    "os"

    "gopkg.in/ini.v1"
)

func main() {
    cfg, err := ini.Load("my.ini")
    if err != nil {
        fmt.Printf("Fail to read file: %v", err)
        os.Exit(1)
    }

    // Classic read of values, default section can be represented as empty string
    fmt.Println("App Mode:", cfg.Section("").Key("app_mode").String())
    fmt.Println("Data Path:", cfg.Section("paths").Key("data").String())

    // Let's do some candidate value limitation
    fmt.Println("Server Protocol:",
        cfg.Section("server").Key("protocol").In("http", []string{"http", "https"}))
    // Value read that is not in candidates will be discarded and fall back to given default value
    fmt.Println("Email Protocol:",
        cfg.Section("server").Key("protocol").In("smtp", []string{"imap", "smtp"}))

    // Try out auto-type conversion
    fmt.Printf("Port Number: (%[1]T) %[1]d\n", cfg.Section("server").Key("http_port").MustInt(9999))
    fmt.Printf("Enforce Domain: (%[1]T) %[1]v\n", cfg.Section("server").Key("enforce_domain").MustBool(false))

    // Now, make some changes and save it
    cfg.Section("").Key("app_mode").SetValue("production")
    cfg.SaveTo("my.ini.local")
}
```

<!--
开始使用
从数据源加载

一个 数据源 可以是 []byte 类型的原始数据，或 string 类型的文件路径。您可以加载 任意多个 数据源。如果您传递其它类型的数据源，则会直接返回错误。

cfg, err := ini.Load([]byte("raw data"), "filename")

或者从一个空白的文件开始：

cfg := ini.Empty()

当您在一开始无法决定需要加载哪些数据源时，仍可以使用 Append() 在需要的时候加载它们。

err := cfg.Append("other file", []byte("other raw data"))
-->

## 基础操作


### 操作分区 Section

``` go
//----- 创建一个分区
err := cfg.NewSection("SectionName")

//----- 获取分区，默认分区使用空字符串 "" 代替
section, err := cfg.GetSection("SectionName")

//----- 如果能确定分区必然存在，可使用如下方法
section := cfg.Section("SectionName")

//----- 获取所有分区对象或名称
sections := cfg.Sections()
names := cfg.SectionStrings()
```

如果通过 `cfg.Section("SectionName")` 获取的分区是不存在的，那么会自动创建并返回一个对应的分区对象。

### 操作键 Key

``` go
//----- 获取某个分区下的键
key, err := cfg.Section("SectionName").GetKey("KeyName")

//----- 也可以直接获取键而忽略错误
key := cfg.Section("SectionName").Key("KeyName")

//----- 判断某个键是否存在
yes := cfg.Section("SectionName").HasKey("KeyName")

//----- 创建一个新的键
err := cfg.Section("SectionName").NewKey("Name", "Value")

//----- 获取分区下的所有键或键名
keys := cfg.Section("SectionName").Keys()
names := cfg.Section("SectionName").KeyStrings()
```

<!--
获取分区下的所有键值对的克隆：
hash := cfg.GetSection("").KeysHash()
-->

### 操作键值 Value

``` go
//----- 获取一个类型为字符串的值
val := cfg.Section("SectionName").Key("KeyName").String()

//----- 获取值的同时通过自定义函数进行处理验证
val := cfg.Section("SectionName").Key("KeyName").Validate(func(in string) string {
	if len(in) == 0 {
		return "default"
	}
	return in
})

//----- 也可以直接获取原值
val := cfg.Section("SectionName").Key("KeyName").Value()

//----- 判断某个原值是否存在
yes := cfg.Section("SectionName").HasValue("Value")

//----- 获取值，会进行转换
val, err = cfg.Section("SectionName").Key("KeyName").Float64()
val = cfg.Section("SectionName").Key("KeyName").MustFloat64()
val = cfg.Section("SectionName").Key("KeyName").MustFloat64(3.14)
```

对于获取值的方法 `Must` 开头允许接收一个相同类型的参数作为默认值，还可以使用 `Bool` `Int` `Int64` `Uint` `Uint64` `TimeFormat` `Time` ，时间可用参数指定时间格式，例如 `TimeFormat(time.RFC3339)`，以及使用当前时间作为默认值 `MustTimeFormat(time.RFC3339, time.Now())` 。

另外，对于布尔值来说，其中 `true` 可以是 `1` `t` `T` `TRUE` `true` `True` `YES` `yes` `Yes` `y` `ON` `on` `On`，而 `false` 可以是 `0` `f` `F` `FALSE` `false` `False` `NO` `no` `No` `n` `OFF` `off` `Off` 。

<!--
如果我的值有好多行怎么办？

[advance]
ADDRESS = """404 road,
NotFound, State, 5000
Earth"""

嗯哼？小 case！

cfg.Section("advance").Key("ADDRESS").String()/* --- start ---
404 road,
NotFound, State, 5000
Earth
------  end  --- */

赞爆了！那要是我属于一行的内容写不下想要写到第二行怎么办？

[advance]
two_lines = how about \
    continuation lines?
lots_of_lines = 1 \
    2 \
    3 \
    4

简直是小菜一碟！

cfg.Section("advance").Key("two_lines").String() // how about continuation lines?cfg.Section("advance").Key("lots_of_lines").String() // 1 2 3 4

需要注意的是，值两侧的单引号会被自动剔除：

foo = "some value" // foo: some valuebar = 'some value' // bar: some value

这就是全部了？哈哈，当然不是。
操作键值的辅助方法

获取键值时设定候选值：

v = cfg.Section("").Key("STRING").In("default", []string{"str", "arr", "types"})v = cfg.Section("").Key("FLOAT64").InFloat64(1.1, []float64{1.25, 2.5, 3.75})v = cfg.Section("").Key("INT").InInt(5, []int{10, 20, 30})v = cfg.Section("").Key("INT64").InInt64(10, []int64{10, 20, 30})v = cfg.Section("").Key("UINT").InUint(4, []int{3, 6, 9})v = cfg.Section("").Key("UINT64").InUint64(8, []int64{3, 6, 9})v = cfg.Section("").Key("TIME").InTimeFormat(time.RFC3339, time.Now(), []time.Time{time1, time2, time3})v = cfg.Section("").Key("TIME").InTime(time.Now(), []time.Time{time1, time2, time3}) // RFC3339

如果获取到的值不是候选值的任意一个，则会返回默认值，而默认值不需要是候选值中的一员。

验证获取的值是否在指定范围内：

vals = cfg.Section("").Key("FLOAT64").RangeFloat64(0.0, 1.1, 2.2)vals = cfg.Section("").Key("INT").RangeInt(0, 10, 20)vals = cfg.Section("").Key("INT64").RangeInt64(0, 10, 20)vals = cfg.Section("").Key("UINT").RangeUint(0, 3, 9)vals = cfg.Section("").Key("UINT64").RangeUint64(0, 3, 9)vals = cfg.Section("").Key("TIME").RangeTimeFormat(time.RFC3339, time.Now(), minTime, maxTime)vals = cfg.Section("").Key("TIME").RangeTime(time.Now(), minTime, maxTime) // RFC3339

自动分割键值到切片（slice）

当存在无效输入时，使用零值代替：

// Input: 1.1, 2.2, 3.3, 4.4 -> [1.1 2.2 3.3 4.4]// Input: how, 2.2, are, you -> [0.0 2.2 0.0 0.0]vals = cfg.Section("").Key("STRINGS").Strings(",")
vals = cfg.Section("").Key("FLOAT64S").Float64s(",")
vals = cfg.Section("").Key("INTS").Ints(",")
vals = cfg.Section("").Key("INT64S").Int64s(",")
vals = cfg.Section("").Key("UINTS").Uints(",")
vals = cfg.Section("").Key("UINT64S").Uint64s(",")
vals = cfg.Section("").Key("TIMES").Times(",")

从结果切片中剔除无效输入：

// Input: 1.1, 2.2, 3.3, 4.4 -> [1.1 2.2 3.3 4.4]// Input: how, 2.2, are, you -> [2.2]vals = cfg.Section("").Key("FLOAT64S").ValidFloat64s(",")
vals = cfg.Section("").Key("INTS").ValidInts(",")
vals = cfg.Section("").Key("INT64S").ValidInt64s(",")
vals = cfg.Section("").Key("UINTS").ValidUints(",")
vals = cfg.Section("").Key("UINT64S").ValidUint64s(",")
vals = cfg.Section("").Key("TIMES").ValidTimes(",")

当存在无效输入时，直接返回错误：

// Input: 1.1, 2.2, 3.3, 4.4 -> [1.1 2.2 3.3 4.4]// Input: how, 2.2, are, you -> errorvals = cfg.Section("").Key("FLOAT64S").StrictFloat64s(",")
vals = cfg.Section("").Key("INTS").StrictInts(",")
vals = cfg.Section("").Key("INT64S").StrictInt64s(",")
vals = cfg.Section("").Key("UINTS").StrictUints(",")
vals = cfg.Section("").Key("UINT64S").StrictUint64s(",")
vals = cfg.Section("").Key("TIMES").StrictTimes(",")

保存配置

终于到了这个时刻，是时候保存一下配置了。

比较原始的做法是输出配置到某个文件：

// ...err = cfg.SaveTo("my.ini")
err = cfg.SaveToIndent("my.ini", "\t")

另一个比较高级的做法是写入到任何实现 io.Writer 接口的对象中：

// ...cfg.WriteTo(writer)
cfg.WriteToIndent(writer, "\t")

高级用法
递归读取键值

在获取所有键值的过程中，特殊语法 %(<name>)s 会被应用，其中 <name> 可以是相同分区或者默认分区下的键名。字符串 %(<name>)s 会被相应的键值所替代，如果指定的键不存在，则会用空字符串替代。您可以最多使用 99 层的递归嵌套。

NAME = ini[author]NAME = UnknwonGITHUB = https://github.com/%(NAME)s[package]FULL_NAME = github.com/go-ini/%(NAME)s

cfg.Section("author").Key("GITHUB").String()        // https://github.com/Unknwoncfg.Section("package").Key("FULL_NAME").String()    // github.com/go-ini/ini

读取父子分区

您可以在分区名称中使用 . 来表示两个或多个分区之间的父子关系。如果某个键在子分区中不存在，则会去它的父分区中再次寻找，直到没有父分区为止。

NAME = iniVERSION = v1IMPORT_PATH = gopkg.in/%(NAME)s.%(VERSION)s[package]CLONE_URL = https://%(IMPORT_PATH)s[package.sub]

cfg.Section("package.sub").Key("CLONE_URL").String()    // https://gopkg.in/ini.v1

读取自增键名

如果数据源中的键名为 - ，则认为该键使用了自增键名的特殊语法。计数器从 1 开始，并且分区之间是相互独立的。

[features]
-: Support read/write comments of keys and sections
-: Support auto-increment of key names
-: Support load multiple files to overwrite key values

cfg.Section("features").KeyStrings()    // []{"#1", "#2", "#3"}

映射到结构

想要使用更加面向对象的方式玩转 INI 吗？好主意。

Name = Unknwonage = 21Male = trueBorn = 1993-01-01T20:17:05Z[Note]Content = Hi is a good man!Cities = HangZhou, Boston

type Note struct {
    Content string
    Cities  []string}

type Person struct {
    Name string
    Age  int `ini:"age"`
    Male bool
    Born time.Time
    Note
    Created time.Time `ini:"-"`
}

func main() {
    cfg, err := ini.Load("path/to/ini")    // ...
    p := new(Person)
    err = cfg.MapTo(p)    // ...

    // 一切竟可以如此的简单。
    err = ini.MapTo(p, "path/to/ini")    // ...

    // 嗯哼？只需要映射一个分区吗？
    n := new(Note)
    err = cfg.Section("Note").MapTo(n)    // ...}

结构的字段怎么设置默认值呢？很简单，只要在映射之前对指定字段进行赋值就可以了。如果键未找到或者类型错误，该值不会发生改变。

// ...p := &Person{
    Name: "Joe",
}// ...

这样玩 INI 真的好酷啊！然而，如果不能还给我原来的配置文件，有什么卵用？
从结构反射

可是，我有说不能吗？

type Embeded struct {
    Dates  []time.Time `delim:"|"`
    Places []string
    None   []int}

type Author struct {
    Name      string `ini:"NAME"`
    Male      bool
    Age       int
    GPA       float64
    NeverMind string `ini:"-"`
    *Embeded
}

func main() {
    a := &Author{"Unknwon", true, 21, 2.8, "",
        &Embeded{
            []time.Time{time.Now(), time.Now()},
            []string{"HangZhou", "Boston"},
            []int{},
        }}
    cfg := ini.Empty()
    err = ini.ReflectFrom(cfg, a)    // ...}

瞧瞧，奇迹发生了。

NAME = UnknwonMale = trueAge = 21GPA = 2.8[Embeded]Dates = 2015-08-07T22:14:22+08:00|2015-08-07T22:14:22+08:00Places = HangZhou,BostonNone =

名称映射器（Name Mapper）

为了节省您的时间并简化代码，本库支持类型为 NameMapper 的名称映射器，该映射器负责结构字段名与分区名和键名之间的映射。

目前有 2 款内置的映射器：

    AllCapsUnderscore ：该映射器将字段名转换至格式 ALL_CAPS_UNDERSCORE后再去匹配分区名和键名。
    TitleUnderscore ：该映射器将字段名转换至格式 title_underscore 后再去匹配分区名和键名。

使用方法：

type Info struct{
    PackageName string}

func main() {
    err = ini.MapToWithMapper(&Info{}, ini.TitleUnderscore, []byte("package_name=ini"))    // ...

    cfg, err := ini.Load([]byte("PACKAGE_NAME=ini"))    // ...
    info := new(Info)
    cfg.NameMapper = ini.AllCapsUnderscore
    err = cfg.MapTo(info)    // ...}

使用函数 ini.ReflectFromWithMapper 时也可应用相同的规则。
映射/反射的其它说明

任何嵌入的结构都会被默认认作一个不同的分区，并且不会自动产生所谓的父子分区关联：

type Child struct {
    Age string}

type Parent struct {
    Name string
    Child
}

type Config struct {
    City string
    Parent
}

示例配置文件：

City = Boston[Parent]Name = Unknwon[Child]Age = 21

很好，但是，我就是要嵌入结构也在同一个分区。好吧，你爹是李刚！

type Child struct {
    Age string}

type Parent struct {
    Name string
    Child `ini:"Parent"`
}

type Config struct {
    City string
    Parent
}

示例配置文件：

City = Boston[Parent]Name = UnknwonAge = 21

获取帮助

    API 文档
    创建工单

常见问题
字段 BlockMode 是什么？

默认情况下，本库会在您进行读写操作时采用锁机制来确保数据时间。但在某些情况下，您非常确定只进行读操作。此时，您可以通过设置 cfg.BlockMode = false来将读操作提升大约 50-70% 的性能。
为什么要写另一个 INI 解析库？

许多人都在使用我的 goconfig 来完成对 INI 文件的操作，但我希望使用更加 Go 风格的代码。并且当您设置 cfg.BlockMode = false 时，会有大约 10-30% 的性能提升。

为了做出这些改变，我必须对 API 进行破坏，所以新开一个仓库是最安全的做法。除此之外，本库直接使用 gopkg.in 来进行版本化发布。（其实真相是导入路径更短了）

https://cloud.tencent.com/developer/article/1066126
-->
