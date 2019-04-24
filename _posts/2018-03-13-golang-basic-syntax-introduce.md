---
title: Golang 语法简介
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,syntax,import,exception,reflection
description: 简单介绍常见的语法，例如 import、异常处理、反射等。
---

简单介绍常见的语法，例如 import、异常处理、反射等。

<!-- more -->

## 常用结构

### 赋值

有两种相关的符号，`=` 表示赋值，`:=` 声明变量并赋值。

{% highlight go %}
//----- 使用 = 必须先用 var 声明
var a
a = 100
var b = 100
var c int = 100

//----- 使用 := 系统会自动推断类型，不需要var关键字
d := 100
{% endhighlight %}

### 类型

<!--
http://colobu.com/2017/06/26/learn-go-type-aliases/
-->

可以通过 Type 把一个类型转换成另外一个类型而保持数据结构不变，如下所示：

{% highlight go %}
type Age int
type Height int
type Grade int
{% endhighlight %}

这里的 type 绝不只是对应于 C/C++ 中的 typedef，它不是用于定义一系列的别名，更关键的是定义了一系列互不相干的行为特征：通过这些互不相干的行为特征，本质上同一的事物表现出不同事物的特征，整数还是整数，但年龄却不是高度也不是分数。

可以分别为 Age、Height、Grade 定义出下列不同的行为(表示为方法或者函数)：

{% highlight go %}
// 超过50岁算老年
func (a Age) Old() bool {
	return a > 50
}
// 高于120cm需要买票
func (l Height) NeedTicket() bool {
	return l > 120
}
// 60分及格
func (g Grade) Pass() bool {
	return g >= 60
}
{% endhighlight %}

### 数组

数组是内置类型，相同数据类型的集合，下标从 0 开始，初始化后长度固定，且无法修改其长度。当作为方法的入参传入时将复制一份数组而不是引用同一指针，而且长度也是其类型的一部分，可以通过内置函数 `len(array)` 获取其长度。

可以通过如下方式初始化。

{% highlight go %}
// 长度为5的数组，其元素分别为1, 2, 3, 4, 5
[5] int {1,2,3,4,5}

// 长度为5的数组，未赋值的默认是 0 ，也就是其元素值依次为1, 2, 0, 0, 0
[5] int {1,2}

// 长度为5的数组，其长度是根据初始化时指定的元素个数决定的
[...] int {1,2,3,4,5}

// 长度为5的数组，key:value,其元素值依次为：0，0，1，2，3。在初始化时指定了2，3，4索引中对应的值：1，2，3
[5] int { 2:1,3:2,4:3}

// 长度为5的数组，起元素值依次为：0，0，1，0，3。由于指定了最大索引4对应的值3，根据初始化的元素个数确定其长度为5
[...] int {2:1,4:3}
{% endhighlight %}

#### 数组遍历

通常有如下的两种遍历方式。

{% highlight go %}
package main

import "fmt"

func main() {
        var arr = [...]int{1, 2, 3, 4, 5}

        for idx, val := range arr {
                fmt.Printf("array[%d] = %d\n", idx, val)
        }
        fmt.Println(">>>>>>>>>>>>>")

        for idx := 0; idx < len(arr); idx++ {
                fmt.Printf("array[%d] = %d\n", idx, arr[idx])
        }
}
{% endhighlight %}

### 结构体

通过结构体新建对象时的语法比较多，而且相比而言有些特殊。

{% highlight go %}
type Poem struct {
    Title  string
    Author string
    intro  string
}
{% endhighlight %}

需要注意下访问权限，如果属性的开头字母是大写的则在其它包中可以被访问，否则只能在本包中访问；类的声明和方法亦是如此。

然后可以通过如下的方法进行赋值。

{% highlight go %}
poem1 := &Poem{}
poem1.Author = "Heine"
poem2 := &Poem{Author: "Heine"}
poem3 := new(Poem)
poem3.Author = "Heine"
poem4 := Poem{}
poem4.Author = "Heine"
poem5 := Poem{Author: "Heine"}
{% endhighlight %}

### 嵌入类型

结构体类型可以包含匿名或者嵌入字段，该类型的名字会充当嵌入字段的字段名。

{% highlight go %}
package main

import (
    "log"
)

type User struct {
        Name  string
        Email string
}

type Admin struct {
        User
        Level string
}

func (u *User) Notify() error {
        log.Printf("User: Sending User Email To %s<%s>\n", u.Name, u.Email)
        return nil
}

func (a *Admin) Notify() error {
        log.Printf("Admin: Sending Admin Email To %s<%s>\n", a.Name, a.Email)
        return nil
}

type Notifier interface {
        Notify() error
}

func SendNotification(notify Notifier) error {
        return notify.Notify()
}

func main() {
        admin := &Admin{
                User: User{
                        Name:  "AriesDevil",
                        Email: "ariesdevil@xxoo.com",
                },
                Level: "super",
        }

        SendNotification(admin)
        admin.Notify()
        admin.User.Notify()
}
{% endhighlight %}

如果对于子类重新定义了接口，那么默认调用的时候是子类的，也可以显式调用父类。

### 函数

<!--
http://jordanorelli.com/post/42369331748/function-types-in-go-golang
-->

在 golang 中，支持匿名函数和闭包，其中定义函数的方式如下：

{% highlight go %}
func (p myType) funcName ( a, b int, c string ) ( r, s int ) {
	return
}
{% endhighlight %}

包括了定义函数的关键字 `func`，函数名称 `funcName`，入参 `a, b int, c string`，返回值 `r,s int` 以及函数体 `{}`。

而且，golang 可以为某个类型定义函数，也即为类型对象定义方法，也就是 `p myType` 参数，当然这不是必须的，如果为空则纯粹是一个函数，可以通过包名称访问。


## MAP

<!--
https://88250.b3log.org/optimizing-concurrent-map-access-in-go-chinese
https://colobu.com/2017/07/11/dive-into-sync-Map/
-->

在 1.9 版本的 `sync` 库中引入了并发 map 数据结构。

{% highlight text %}
The new Map type in the sync package is a concurrent map with
amortized-constant-time loads, stores, and deletes. It is safe
for multiple goroutines to call a Map's methods concurrently.
{% endhighlight %}

GoLang 内置类型 MAP 是用哈希表实现的，可以通过 `map[KeyType]ValueType` 定义，`ValueType` 可以是任意类型，包括 `map`，而 `KeyType` 是可以执行比较的类型，包括 `==` `!=` `<` `<=` `>` `>=` 。

如果请求的 Key 不存在，则返回 Value 类型的零值，其中 `int` 为 0、`string` 为 `""` 。

### 示例

{% highlight go %}
package main

import "fmt"

func main() {
        /*
                var m1 map[string]string = map[string]string{}
                var m2 map[string]string
                m2 = make(map[string]string) // OR
        */

        cities := map[string]string{
                "ZheJiang": "HangZou",
        }

        newCities := cities
        newCities["ZheJiang"] = "HangZhou" // Also change cities

        cities["LiaoNing"] = "ShenYang" // len(cities) == 1

        for c := range cities {
                fmt.Println("A: Capital of", c, "is", cities[c])
        }

        for c, v := range cities {
                fmt.Println("B: Capital of", c, "is", v)
        }

        if c, ok := cities["ShanDong"]; ok { // OR JUST CHECK _, ok := cities["ShanDong"]
                fmt.Println("Capital of LiaoNing is", c)
        } else {
                fmt.Println("Get capital failed")
        }

        fmt.Println(cities["NotExists"] == "")
        delete(cities, "LiaoNing")
        delete(cities, "NotExists") // No side effect

        fmt.Println("Current length", len(cities))
}
{% endhighlight %}

### 稳定性

当使用 `range` 循环遍历 MAP 时，遍历的顺序是不确定的，并且不能保证遍历顺序和下次遍历的顺序相同。

在运行时对 MAP 遍历的顺序做随机化处理，如果依赖于遍历顺序稳定性，必须自己去维护一个遍历顺序的独立数据结构，例如切片。

### 其它

与切片一样，MAP 是引用类型，当一个 MAP 赋值给一个新的变量时，它们都指向同一个内部数据结构，改变其中一个也会反映到另一个。

MAP 不能通过 `==` 操作符比较是否相等，能用来检测 MAP 是否为 `nil`。

<!--
https://studygolang.com/articles/11979
https://guidao.github.io/go_map.html
https://blog.csdn.net/Soooooooo8/article/details/70163475

[Go小技巧] 实现常用的KV缓存（有序且并发安全）
https://my.oschina.net/henrylee2cn/blog/741315

https://github.com/gostor/awesome-go-storage
内存数据库可以参考
https://github.com/tidwall/buntdb
https://github.com/patrickmn/go-cache
-->

## 字符串操作

这应该是最常见的，在 Golang 中有多种方式可以完成拼接，详见如下的测试程序。

{% highlight go %}
package main

import (
        "bytes"
        "fmt"
        "strings"
        "time"
)

var ways = []string{
        "fmt.Sprintf ",
        "+           ",
        "strings.Join",
        "bytes.Buffer",
}

func benchmarkStringFunction(n int, idx int) {
        var s string
        var buf bytes.Buffer

        v := "hello world, just for test"
        begin := time.Now()
        for i := 0; i < n; i++ {
                switch idx {
                case 0: // fmt.Sprintf
                        s = fmt.Sprintf("%s[%s]", s, v)
                case 1: // string +
                        s = s + "[" + v + "]"
                case 2: // strings.Join
                        s = strings.Join([]string{s, "[", v, "]"}, "")
                case 3: // stable bytes.Buffer
                        buf.WriteString("[")
                        buf.WriteString(v)
                        buf.WriteString("]")
                }

        }
        if idx == 3 {
                s = buf.String()
        }
        fmt.Printf("string len: %d\t", len(s))
        fmt.Printf("time of [%s]=\t %v\n", ways[idx], time.Since(begin))
}

func main() {
        for idx, _ := range ways {
                benchmarkStringFunction(10000, idx)
        }
}
{% endhighlight %}

执行结果如下。

{% highlight text %}
string len: 280000      time of [fmt.Sprintf ]=  366.809538ms
string len: 280000      time of [+           ]=  231.356836ms
string len: 280000      time of [strings.Join]=  497.997435ms
string len: 280000      time of [bytes.Buffer]=  867.259µs
{% endhighlight %}

结论: A) `strings.Join` 最慢；B) 其次为 `fmt.Sprintf` 和 `string +`；C) 最快为 `bytes.Buffer` 。

<!--
https://sheepbao.github.io/post/golang_byte_slice_and_string/
-->

## 协程

另外，一种方式是一直阻塞在管道中，利用 for 循环遍历。

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func readCommits(commitC <-chan *string) {
        for data := range commitC {
                if data == nil {
                        fmt.Println("Got nil data")
                        continue
                }
                fmt.Println(*data)
        }
}

func main() {
        commitC := make(chan *string)

        go func() {
                for {
                        s := "hi"
                        time.Sleep(1 * time.Second)

                        commitC <- &s
                }
        }()

        readCommits(commitC)
}
{% endhighlight %}

## 反射

反射允许你可以在运行时检查类型，包括检查、修改、创建变量、函数、结构体等。

{% highlight go %}
package main

import (
        "fmt"
        "reflect"
)

type Foobar struct {
        name string
}

func (f *Foobar) GetName() string {
        return f.name
}

func main() {
        s := "this is string"
        fmt.Println(reflect.TypeOf(s))
        fmt.Println(reflect.ValueOf(s))

        var x float64 = 3.4
        fmt.Println(reflect.ValueOf(x))

        a := new(Foobar)
        a.name = "foo bar"
        t := reflect.TypeOf(a)
        fmt.Println(t.NumMethod())
        b := reflect.ValueOf(a).MethodByName("GetName").Call([]reflect.Value{})
        fmt.Println(b[0])

}
{% endhighlight %}


## 管道 (channel)

<!--
http://colobu.com/2016/04/14/Golang-Channels/
-->

核心类型，可以看做是一个 FIFO 的阻塞消息队列，用于发核心单元之间发送和接收数据，默认是双向的，也可以指定方向，使用前必须要创建并初始化，

{% highlight go %}
package main

import "fmt"

func sum(s []int, c chan int) {
	sum := 0
	for _, v := range s {
		sum += v
	}
	c <- sum // send sum to c
}

func main() {
	s := []int{7, 2, 8, -9, 4, 0}

	c := make(chan int)

	go sum(s[:len(s)/2], c)
	go sum(s[len(s)/2:], c)
	x, y := <-c, <-c // receive from c

	fmt.Println(x, y, x+y)
}
{% endhighlight %}

如上中的 `make(chan int, 100)` ，上述的第二个参数可选，表示容量，也就是管道可以容纳的最多元素数量，代表管道的缓存大小。

大小默认是 0 ，也就是如果接收、发送没有准备好，另外一端就会阻塞，如果设置了缓存，只有 buffer 满了后 send 才会阻塞，当缓存空了后 receive 才会阻塞。

<!--
if-else语句，比较奇葩的是
https://gobyexample.com/if-else

Select语句
https://segmentfault.com/a/1190000006815341
-->

简单示例如下。

{% highlight go %}
package main

import "time"
import "fmt"

func main() {
	// For our example we'll select across two channels.
	c1 := make(chan string)
	c2 := make(chan string)

	// Each channel will receive a value after some amount
	// of time, to simulate e.g. blocking RPC operations
	// executing in concurrent goroutines.
	go func() {
		time.Sleep(1 * time.Second)
		c1 <- "one"
	}()
	go func() {
		time.Sleep(2 * time.Second)
		c2 <- "two"
	}()

	// We'll use `select` to await both of these values
	// simultaneously, printing each one as it arrives.
	for i := 0; i < 2; i++ {
		select {
		case msg1 := <-c1:
			fmt.Println("received", msg1)
		case msg2 := <-c2:
			fmt.Println("received", msg2)
		}
	}
}
{% endhighlight %}

### Select 行为

从一个 Topic 中提取的代码。

{% highlight go %}
// https://talks.golang.org/2012/concurrency.slide#32
select {
case v1 := <-c1:
	fmt.Printf("received %v from c1\n", v1)
case v2 := <-c2:
	fmt.Printf("received %v from c2\n", v1)
case c3 <- 23:
	fmt.Printf("sent %v to c3\n", 23)
default:
	fmt.Printf("no one was ready to communicate\n")
}
{% endhighlight %}

上述代码中包含了三个 case 子句以及一个 default 子句，前两个是 receive 操作，第三个是 send 操作，最后一个是默认操作。

当代码执行到 select 时，case 语句会按照源代码的顺序被评估，且只评估一次，评估的结果会出现下面这几种情况：

* 除 default 外，如果只有一个 case 语句评估通过，那么就执行这个 case 里的语句；
* 除 default 外，如果有多个 case 语句评估通过，那么通过伪随机的方式随机选一个；
* 如果 default 外的 case 语句都没有通过评估，那么执行 default 里的语句；
* 如果没有 default，那么 代码块会被阻塞，直到有一个 case 通过评估，否则会一直阻塞；

### 其它

另外，需要注意，如果向管道中发送的数据，而有其它的协程阻塞等待，那么条件满足后会先调度执行被阻塞的协程。

{% highlight go %}
package main

import (
        "fmt"
)

func main() {
        readyc := make(chan string)

        go func() {
                select {
                case str := <-readyc:
                        fmt.Printf("%s, Go !!!\n", str)
                }

        }()

		readyc <- "Reaaaaady"
		fmt.Println("Done")
}
{% endhighlight %}

也就是上述的输出为：

{% highlight text %}
Reaaaaady, Go !!!
Done
{% endhighlight %}

### 示例

#### Timeout

{% highlight go %}
package main

import "time"
import "fmt"

func main() {
        timeout := make (chan bool, 1)
        ch := make (chan int)
        go func() {
                time.Sleep(2 * time.Second)
                timeout <- true
        }()
        select {
        case <- ch:
        case <- timeout:
            fmt.Println("timeout!")
        }
}
{% endhighlight %}

因为没有向管道 ch 发送数据，默认应该是一直等待。也可以使用如下示例：

{% highlight go %}
package main

import (
        "fmt"
        "time"
)

func main() {
        donec := make(chan string)

        go func() {
                time.Sleep(1 * time.Second)
                donec <- "done"
        }()

        select {
        case s := <-donec:
                fmt.Printf("Got string '%s'.\n", s)
        case <-time.After(2 * time.Second):
                fmt.Println("Timeout")
        }
}
{% endhighlight %}

### 检查队列是否满

这里使用的是 default 这个特性。

{% highlight go %}
package main

import "fmt"

func main() {
        ch := make (chan int, 1)
        ch <- 1
        select {
                case ch <- 2:
                default:
                        fmt.Println("channel is full !")
        }
}
{% endhighlight %}

因为 ch 插入 1 的时候已经满了， 当 ch 要插入 2 的时候，发现 ch 已经满了此时默认应该是阻塞，不过因为有 default 子句，实际会执行 default 子语， 这样就可以实现对 channel 是否已满的检测， 而不是一直等待。

例如在并发处理多个 job 时，如果队列满了，则返回错误让用户重试。

#### Quit Channel

{% highlight go %}
package main

import (
        "fmt"
)

func boring(msg string, quit chan string) chan string {
        c := make(chan string)
        i := 0

        go func() {
                for {
                        i++
                        select {
                        case c <- fmt.Sprintf("%s: %d", msg, i):
                                fmt.Println("Send data")
                        case q := <-quit:
                                // cleanup
                                fmt.Printf("Got %q\n", q)
                                quit <- "See you!"
                                return
                        }
                }
        }()

        return c
}

func main() {
        quit := make(chan string)

        c := boring("Foobar", quit)

        for i := 5; i >= 0; i-- {
                fmt.Println(<-c)
        }

        quit <- "Bye!"

        fmt.Printf("Foobar says: %q\n", <-quit)
}
{% endhighlight %}

<!--
Quit Channel/Done Channel 还没有搞明白
https://segmentfault.com/a/1190000006815341
-->

## 竞态条件

如下是一个简单的示例，简单来说，是对一个内存中的值进行累加，

{% highlight go %}
package main

import (
        "fmt"
        "sync"
)

var (
        N         = 0
        waitgroup sync.WaitGroup
)

func counter(number *int) {
        *number++
        waitgroup.Done()
}

func main() {

        for i := 0; i < 1000; i++ {
                waitgroup.Add(1)
                go counter(&N)
        }
        waitgroup.Wait()
        fmt.Println(N)
}
{% endhighlight %}

如果运行多次，可以发现绝大多数情况下其累加值不是 1000 。

解决方法是在执行累加时，对操作加锁。

{% highlight go %}
package main

import (
        "fmt"
        "sync"
)

var (
        N         = 0
        mutex     sync.Mutex
        waitgroup sync.WaitGroup
)

func counter(number *int) {
        mutex.Lock()
        *number++
        mutex.Unlock()
        waitgroup.Done()
}

func main() {

        for i := 0; i < 1000; i++ {
                waitgroup.Add(1)
                go counter(&N)
        }
        waitgroup.Wait()
        fmt.Println(N)
}
{% endhighlight %}

### 竞态条件检测

GoLang 工具内置了竞态检测工具，只需要加上 `-race` 即可。

{% highlight text %}
$ go test -race mypkg    // test the package
$ go run -race mysrc.go  // compile and run the program
$ go build -race mycmd   // build the command
$ go install -race mypkg // install the package
{% endhighlight %}

使用该参数，GO 会记录不同线程对共享变量的访问情况，如果发现非同步的访问则会退出并打印告警信息。


## 枚举类型

GoLang 并没有提供 `enum` 的定义，不过可以使用 `const` 来模拟枚举类型。

{% highlight go %}
const (
	STATUS_NORMAL int = 0
	STATUS_FAILED int = 1
)
{% endhighlight %}

另外，也可以使用 `iota` 。

### iota

这是 GoLang 语言中的常量计数器，只能在常量的表达式中使用；该关键字出现时将被重置为 0，然后每新增一行常量声明将使 iota 计数一次。

例如，`time` 包中定义时间类型的方式如下，其中的 `_` 符号表示跳过。

{% highlight go %}
type Weekday int

const (
    Sunday Weekday = iota
    Monday
    Tuesday
    Wednesday
    _
    Friday
    Saturday
)
{% endhighlight %}

注意，`iota` 的增长是按照行顺序，如果在同一行，那么值是不会增长的。

其它常见场景。

#### 位掩码

{% highlight go %}
package main

import (
        "fmt"
)

type Flags uint

const (
        FlagUp           Flags = 1 << iota // is up
        FlagBroadcast                      // supports broadcast access capability
        FlagLoopback                       // is a loopback interface
        FlagPointToPoint                   // belongs to a point-to-point link
        FlagMulticast                      // supports multicast access capability
)

func IsUp(v Flags) bool     { return v&FlagUp == FlagUp }
func TurnDown(v *Flags)     { *v &^= FlagUp }
func SetBroadcast(v *Flags) { *v |= FlagBroadcast }
func IsCast(v Flags) bool   { return v&(FlagBroadcast|FlagMulticast) != 0 }

func main() {
        var v Flags = FlagMulticast | FlagUp
        fmt.Printf("%b %t\n", v, IsUp(v)) // "10001 true"
        TurnDown(&v)
        fmt.Printf("%b %t\n", v, IsUp(v)) // "10000 false"
        SetBroadcast(&v)
        fmt.Printf("%b %t\n", v, IsUp(v))   // "10010 false"
        fmt.Printf("%b %t\n", v, IsCast(v)) // "10010 true"
}
{% endhighlight %}

#### 定义数量级

{% highlight go %}
type ByteSize float64

const (
    _           = iota                   // ignore first value by assigning to blank identifier
    KB ByteSize = 1 << (10 * iota)       // 1 << (10*1)
    MB                                   // 1 << (10*2)
    GB                                   // 1 << (10*3)
    TB                                   // 1 << (10*4)
    PB                                   // 1 << (10*5)
    EB                                   // 1 << (10*6)
    ZB                                   // 1 << (10*7)
    YB                                   // 1 << (10*8)
)
{% endhighlight %}

它并不能用于产生 1000 的幂，因为 GoLang 并没有计算幂的运算符。

#### 中间间隔

{% highlight go %}
const (
	i = iota     // 0
	j = 3.14     // 3.14
	k = iota     // 2
	l            // 3
)
{% endhighlight %}

## 语法糖

### ...

主要有两种用法：A) 用于函数有多个不定参数场景；B) slice 可以被打散进行传递。

{% highlight go %}
package main

import "fmt"

func foobar(args ...string) {
        for _, v := range args {
                fmt.Println(v)
        }
}

func main() {
        var foostr = []string{"hello", "world"}
        var barstr = []string{"hi", "fooo", "baar,"}

        foobar("foo", "bar", "foobar")

        fmt.Println(append(barstr, foostr...))
}
{% endhighlight %}

### Byte VS. Rune

两种实际上是 `uint8` 和 `uint32` 类型，byte 用来强调数据是 RawData，而不是数字；而 rune 用来表示 Unicode 编码的 CodePoint。

中文字符使用 3 个字节保存(为什么使用的是3个字节保存，还没有搞清楚)。

{% highlight go %}
s := "hello你好"
fmt.Println(len(s))         // 11
fmt.Println(len([]rune(s))) // 7，需要先转换为rune的切片在使用内置len函数
s = "你好"
fmt.Println(len(s))         // 6
fmt.Println(len([]rune(s))) // 2
s = "你"
fmt.Println([]byte(s)) // 三个字节，也就是中文的表示方法
fmt.Println(rune('你')) // 输出20320(0x4F60)，'你' 的编码
{% endhighlight %}

<!--
Strings, bytes, runes and characters in Go
https://blog.golang.org/strings
-->


<!--

## ServeMux VS. Handler

Handler 负责输出 HTTP 响应的头和正文，任何满足了 `http.Handler` 接口的对象都可作为一个处理器。

type Handler interface {
	ServeHTTP(ResponseWriter, *Request)
}

Go 语言的 HTTP 包自带了几个函数用作常用处理器，比如FileServer，NotFoundHandler 和 RedirectHandler。我们从一个简单具体的例子开始：

https://segmentfault.com/a/1190000006812688

rafthttp/transport.go

func (t *Transport) Handler() http.Handler {
	pipelineHandler := newPipelineHandler(t, t.Raft, t.ClusterID)
	streamHandler := newStreamHandler(t, t, t.Raft, t.ID, t.ClusterID)
	snapHandler := newSnapshotHandler(t, t.Raft, t.Snapshotter, t.ClusterID)
	mux := http.NewServeMux() //http 请求路由
	mux.Handle(RaftPrefix, pipelineHandler) /* /raft */
	mux.Handle(RaftStreamPrefix+"/", streamHandler)  /* /raft/stream/ */
	mux.Handle(RaftSnapshotPrefix, snapHandler)      /* /raft/snapshot */
	mux.Handle(ProbingPrefix, probing.NewHandler())  /* /raft/probing */
	return mux
}

func newPeerHandler(cluster api.Cluster, raftHandler http.Handler, leaseHandler http.Handler) http.Handler {
        mh := &peerMembersHandler{
                cluster: cluster,
        }

        mux := http.NewServeMux()
        mux.HandleFunc("/", http.NotFound)
        mux.Handle(rafthttp.RaftPrefix, raftHandler)
        mux.Handle(rafthttp.RaftPrefix+"/", raftHandler)
        mux.Handle(peerMembersPrefix, mh)
        if leaseHandler != nil {
                mux.Handle(leasehttp.LeasePrefix, leaseHandler)
                mux.Handle(leasehttp.LeaseInternalPrefix, leaseHandler)
        }
        mux.HandleFunc(versionPath, versionHandler(cluster, serveVersion))
        return mux
}

### Byte VS. Rune

两种实际上是 `uint8` 和 `uint32` 类型，byte 用来强调数据是 RawData，而不是数字；而 rune 用来表示 Unicode 编码的 CodePoint。

中文字符使用 3 个字节保存(为什么使用的是3个字节保存)。

s := "hello你好"
fmt.Println(len(s))         // 11
fmt.Println(len([]rune(s))) // 7，需要先转换为rune的切片在使用内置len函数
s = "你好"
fmt.Println(len(s))         // 6
fmt.Println(len([]rune(s))) // 2
s = "你"
fmt.Println([]byte(s)) // 三个字节，也就是中文的表示方法
fmt.Println(rune('你')) // 输出20320(0x4F60)，'你' 的编码

Strings, bytes, runes and characters in Go
https://blog.golang.org/strings



package main

import (
        "bytes"
        "encoding/binary"
        "fmt"
)

func IntToBytes(n int) []byte {
        bytesBuffer := bytes.NewBuffer([]byte{})
        binary.Write(bytesBuffer, binary.BigEndian, int32(n))
        return bytesBuffer.Bytes()
}

func main() {
        var b0 bytes.Buffer
        b0.Write([]byte("Hello "))
        fmt.Println(b0.String())

        b1 := new(bytes.Buffer)
        b1.WriteString("Hi World")
        b1.WriteByte('!')
        fmt.Println(b1.String())

        /*
                // OR
                b2 := bytes.NewBufferString("swift")
                b3 := bytes.NewBuffer([]byte("swift"))
                b4 := bytes.NewBuffer([]byte{'s', 'w', 'i', 'f', 't'})

                // Empty Buffer
                b5 = bytes.NewBufferString("")
                b6 = bytes.NewBuffer([]byte{})
        */
}


The Go Programming Language Specification
https://golang.org/ref/spec

## New() VS. Make()

两者都是内建函数，都是用来申请内存的，其声明为。

func new(Type) *Type
func make(Type, size IntegerType) Type

其中前者返回的是指向这个新分配已初始化对象的指针；后者只能分配 slice、map 或者 chan 对象，返回的是初始化后对象的引用而非指针。 

对于 slice、map、chan 对象，声明对象之后实际上变量的值是 nil ，此时没有分配内存。

如果不做特殊声明，函数是按值传参，也就是参数的副本，在函数内部对值修改不影响值的本身。不过通过 `make(T, args)` 返回的值通过函数传递参数之后可以直接修改，也就是 map，slice，channel 。

另外，对于 struct 对象来说，完全可以不使用 new 完成初始化操作。
-->

{% highlight go %}
{% endhighlight %}
