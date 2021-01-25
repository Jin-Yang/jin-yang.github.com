---
title: Golang 常用模块
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,log
description: 简单介绍常见的三方模块使用，例如 log、unsafe 等。
---

简单介绍常见的三方模块使用，例如 log、unsafe 等。

<!-- more -->

## fmt

其中最常用的是 `%v`，这是一个通用的格式化方式，用来打印 struct 的成员变量名称。

{% highlight text %}
%v   默认格式，只打印各个字段的值，没有字段名称；
%+v  当打印结构体时，同时会添加字段名称；
%#v  类似+v，同时会打印对象名称，并用语法标示数据类型，例如字符串会加""；
%T   变量对应的类型名称 pakage.struct；
{% endhighlight %}

示例如下。

{% highlight go %}
package main

import (
        "fmt"
)

type User struct {
        name string
        age  int
}

func main() {
        u := &User{name: "andy", age: 30}

        fmt.Printf("%v\n", *u)  // {andy 30}
        fmt.Printf("%+v\n", *u) // {name:andy age:30}
        fmt.Printf("%#v\n", *u) // main.User{name:"andy", age:30}
        fmt.Printf("%T\n", *u)  // main.User
}
{% endhighlight %}

## 日志 (log)

这也是官方标准的日志库，使用方式与 fmt 类似，只是默认输出时会添加时间信息。

{% highlight go %}
package main

import "log"

func main() {
        log.Println("Hello world")
        log.Printf("Hi...")
}
{% endhighlight %}

可以在启动时通过 `init()` 函数进行一些初始化操作，其中 log 库提供了一些常见的配置项，可以通过 `func SetFlags(flag int)` 设置。

{% highlight go %}
const (
	Ldate         = 1 << iota     // 日期示例: 2009/01/23
	Ltime                         // 时间示例: 01:23:23
	Lmicroseconds                 // 毫秒示例: 01:23:23.123123.
	Llongfile                     // 绝对路径和行号: /a/b/c/d.go:23
	Lshortfile                    // 文件和行号: d.go:23.
	LUTC                          // 日期时间转为UTC时区的
	LstdFlags     = Ldate | Ltime // 默认的格式输出
)
{% endhighlight %}

也可以通过 `func SetPrefix(prefix string)` 设置日志的输出前缀。

另外，提供了 `Fatalln()` 和 `Panicln()` 函数，前者会在打印日志后直接退出；后者如果没有使用 `recover()` 函数则会打印错误栈信息后退出。



### 日志格式定制

在 `init()` 函数中，对日志的格式进行定制化处理。

其它常见的三方库有 [GitHub ZAP](https://github.com/uber-go/zap) 中的介绍。

<!--
//定义logger, 传入参数 文件，前缀字符串，flag标记
func New(out io.Writer, prefix string, flag int) *Logger

log 的源码实现也很简单，可以参考
http://www.flysnow.org/2017/05/06/go-in-action-go-log.html
-->

## bytes

{% highlight go %}
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
{% endhighlight %}

## unsafe

这是一个很特殊的包，可以绕过 GoLang 本身的一些语法检查直接操作对象，从而可能会导致不可移植(可控)，而且使用比较危险。该包中包含了三个函数，以及一种类型：

{% highlight go %}
func Alignof(variable ArbitraryType) uintptr
func Offsetof(selector ArbitraryType) uintptr
func Sizeof(variable ArbitraryType) uintptr

Pointer *ArbitraryType
{% endhighlight %}

上述函数类似于 C 中的宏，在编译时求值，而非运行时，也就是说它的结果可以分配给常量。

另外，`uintptr` 实际上是整型，其大小根据不同的平台变化，可以用来保存指针，例如在 32Bits 下是 4Bytes，在 64Bits 下是 8Bytes 。

### 使用示例

在官方文档中有介绍其常见的使用场景。

#### unsafe.Pointer VS. uintptr

`uintptr` 是一个可以容纳指针地址的整数类型，及时该变量仍然有效，但是其所指向地址的数据可能已经被 GC 回收掉。

`unsafe.Pointer` 是一个通用的指针类型，如果该变量有效，那么其指向的地址出的数据就不会被 GC 回收掉。

因为 `uintptr` 是一个整型，可以进行算术运算；那么就可以使用上述两者绕过限制操作变量，计算结构体中变量的偏移。

如下是两个示例，分别用来操作数组和结构体成员。

{% highlight go %}
package main

import (
        "fmt"
        "unsafe"
)

func main() {
        a := [4]int{0, 1, 2, 3}
        p1 := unsafe.Pointer(&a[1])
        p3 := unsafe.Pointer(uintptr(p1) + 2*unsafe.Sizeof(a[0]))
        *(*int)(p3) = 6
        fmt.Println("a =", a) // a = [0 1 2 6]

        type Person struct {
                name   string
                age    int
                gender bool
        }

        who := Person{"John", 30, true}
        pp := unsafe.Pointer(&who)
        pname := (*string)(unsafe.Pointer(uintptr(pp) + unsafe.Offsetof(who.name)))
        page := (*int)(unsafe.Pointer(uintptr(pp) + unsafe.Offsetof(who.age)))
        pgender := (*bool)(unsafe.Pointer(uintptr(pp) + unsafe.Offsetof(who.gender)))
        *pname = "Alice"
        *page = 28
        *pgender = false
        fmt.Println(who) // {Alice 28 false}
}
{% endhighlight %}

## signal

关于信号处理主要在 `os/signal` 中实现，其中包含了两个主要的方法：

* Notify() 监听收到的信号
* Stop() 取消监听

{% highlight go %}
func Notify(c chan<- os.Signal, sig …os.Signal)
{% endhighlight %}

简单使用用例。

{% highlight go %}
package main

import (
        "fmt"
        "os"
        "os/signal"
        "syscall"
)

func main() {
        c := make(chan os.Signal)
        //signal.Notify(c) // default all signal
        signal.Notify(c, os.Interrupt, os.Kill, syscall.SIGUSR1)
        fmt.Println("Waiting signal")

        s := <-c
        fmt.Println("Quit with", s)
}
{% endhighlight %}

最经常使用的用例如下，可以通过捕获信号来做一些清理操作。

{% highlight go %}
package main

import (
        "fmt"
        "os"
        "os/signal"
        "syscall"
        "time"
)

func main() {
        c := make(chan os.Signal, 128)
        signal.Notify(c, syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM,
				syscall.SIGQUIT, syscall.SIGUSR1, syscall.SIGUSR2)

        go func() {
                for s := range c {
                        switch s {
                        case syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT:
                                fmt.Println("Quit with signal", s)
                                os.Exit(0)
                        case syscall.SIGUSR1:
                                fmt.Println("USR1", s)
                        case syscall.SIGUSR2:
                                fmt.Println("USR2", s)
                        default:
                                fmt.Println("Other", s)
                        }
                }
        }()

        fmt.Println("Starting ...")
        for {
                fmt.Println("Waiting ...")
                time.Sleep(time.Second)
        }
}
{% endhighlight %}


<!--
http://shanks.leanote.com/post/golang%E4%BF%A1%E5%8F%B7%E5%A4%84%E7%90%86
https://github.com/polaris1119/The-Golang-Standard-Library-by-Example/blob/master/chapter16/16.03.md
-->

## Container

在官方代码 `container` 目录下包含了三个容器类型的数据类型，也就是 `heap`、`list` 和 `ring`，这里简单介绍其使用方式。

### Heap

这里的堆使用的数据结构是最小二叉树，即根节点比左边子树和右边子树的所有值都小，其对应的接口定义为：

{% highlight go %}
type Interface interface {
	Len() int
	Less(i, j int) bool
	Swap(i, j int)
}

type Interface interface {
	sort.Interface
	Push(x interface{}) // add x as element Len()
	Pop() interface{}   // remove and return element Len() - 1.
}
{% endhighlight %}

也就是说，堆的接口继承自 `sort.Interface` 对应了 `sort/sort.go` 中的实现，那么对于堆来说，总共包含了上述的五个接口实现。

### List

这里实现的是一个双向链表，其对应类型的定义如下。

{% highlight go %}
type List struct {
	root Element
	len  int
}

type Element struct {
	next, prev *Element
	list *List
	Value interface{}
}
{% endhighlight %}

注意，这里定义的是类型而非接口。

其中的示例以及接口可以直接参考 [golang.org/pkg/container/list](https://golang.org/pkg/container/list/) 中的介绍。

### Ring

环其实也是链表的实现，只是其尾部就是头部，所以每个元素实际上就可以代表自身的这个环，而不需要像 list 那样维护两个数据结构。

{% highlight go %}
type Ring struct {
	next, prev *Ring
	Value      interface{}
}
{% endhighlight %}

初始化的时候，需要先定义好环的大小，然后可以对其每个元素进行操作。同时还提供一个 `Do()` 函数，能遍历一遍环，对每个元素执行次函数调用。

## exec

exec 用来执行命令，实际上是将 `os.StartProcess()` 进行包装使得它更容易映射到 `stdin` 和 `stdout` 。

其源码在 `os/exec/` 中实现，其中核心的结构体为 `type Cmd struct` ，另外很多的示例可以参考源码目录下的 `example_test.go` 文件。

{% highlight go %}
type Cmd struct {
	Path         string　　    　// 运行命令的路径，可以是绝对路径或者相对路径
	Args         []string　　    // 命令参数
	Env          []string        // 环境变量，为 nil 时使用当前进程的环境变量
	Dir          string　　    　// 指定命令运行时的工作目录
	Stdin        io.Reader　　   // 标准输入，nil 则从os.DevNull读取
	Stdout       io.Writer       // 标准输出
	Stderr       io.Writer　　   // 标准错误输出，nil 重定向到os.DevNull设备中

	ExtraFiles   []*os.File 　　
	SysProcAttr  *syscall.SysProcAttr
	Process      *os.Process         // 对应os.Process中的实现
	ProcessState *os.ProcessState　　// 一个进程退出时的信息，当调用Wait()或者Run()时会生成该对象
}
{% endhighlight %}

常用接口。

{% highlight go %}
// 在环境变量中查找可执行的二进制文件，可以在绝对路径、相对路径下查找
func LookPath(file string) (string, error)

// 仅设置Cmd结构中的Path和Args参数，如果不含路径分隔符则会尝试通过LookPath查找
func Command(name string, arg ...string) *Cmd
func CommandContext(ctx context.Context, name string, arg ...string) *Cmd
{% endhighlight %}


{% highlight go %}
package main

import (
        "bytes"
        "log"
        "os/exec"
        "strings"
)

func main() {
        cmd := exec.Command("tr", "a-z", "A-Z")
        cmd.Stdin = strings.NewReader("some input")

        var out bytes.Buffer
        cmd.Stdout = &out

        err := cmd.Run()
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Got ===> %q\n", out.String())
}
{% endhighlight %}

{% highlight go %}
// 运行命令并同时返回标准输出和标准错误
func (c *Cmd) Output() ([]byte, error)
func (c *Cmd) CombinedOutput() ([]byte, error)
{% endhighlight %}

如果执行命令出错会同时设置 `error` 错误。

{% highlight go %}
package main

import (
        "log"
        "os/exec"
)

func main() {
        cmd := exec.Command("ls", "-alh", "/")
        out, err := cmd.CombinedOutput()
        if err != nil {
                log.Fatal(err)
        }
        log.Printf("Got ===>%s", string(out))
}
{% endhighlight %}

{% highlight go %}
// 开始执行命令，如果需要，应该使用Wait()等待命令执行完毕
func (c *Cmd) Start() error
// 执行指定的命令并且等待执行结束，实际上是Start()+Wait()的组合
func (c *Cmd) Run() error
{% endhighlight %}

{% highlight go %}
package main

import (
        "fmt"
        "os"
        "os/exec"
)

func main() {
        cmd := exec.Command("ls")
        cmd.Stdout = os.Stdout
        cmd.Run()
        fmt.Println(cmd.Start()) //exec: already started
}
{% endhighlight %}

注意，一个 Command 只能使用 `Start()` 或者 `Run()` 中的一个启动命令，不能两个同时使用。

{% highlight go %}
// 等待命令退出，必须和Start一起使用
func (c *Cmd) Wait() error
{% endhighlight %}



{% highlight go %}
// 将标准输入、输出等重定向，返回一个Pipe，在命令退出时会关闭这些Pipe
func (c *Cmd) StderrPipe() (io.ReadCloser, error)
func (c *Cmd) StdoutPipe() (io.ReadCloser, error)
func (c *Cmd) StdinPipe() (io.WriteCloser, error)
{% endhighlight %}

{% highlight go %}
package main

import (
        "fmt"
        "os"
        "os/exec"
)

func main() {
        cmd := exec.Command("cat")

        stdin, err := cmd.StdinPipe()
        if err != nil {
                fmt.Println(err)
        }
        _, err = stdin.Write([]byte("Hi World!!!\n"))
        if err != nil {
                fmt.Println(err)
        }
        stdin.Close()

        cmd.Stdout = os.Stdout
        cmd.Start()
}


package main

import (
        "fmt"
        "io/ioutil"
        "os/exec"
)

func main() {
        cmd := exec.Command("ls", "-alh")
        stdout, err := cmd.StdoutPipe()
        cmd.Start()
        content, err := ioutil.ReadAll(stdout)
        if err != nil {
                fmt.Println(err)
        }
        fmt.Println(string(content))
}
{% endhighlight %}

<!---
https://colobu.com/2017/06/19/advanced-command-execution-in-Go-with-os-exec/
https://blog.kowalczyk.info/article/wOYk/advanced-command-execution-in-go-with-osexec.html
-->

os 包有一个 `StartProcess()` 可以用来调用或启动外部系统命令和二进制可执行文件，三个入参分别为：A) 运行的进程；B) 传递选项或参数；C) 系统环境基本信息的结构体。

<!--
StartProcess()  os/exec.go
 |-startProcess()

http://blog.51cto.com/allragedbody/1747146
-->

{% highlight go %}
package main

import (
        "fmt"
        "os"
)

func main() {

        proc, err := os.StartProcess(
                "/bin/ls",
                []string{"ls", "-l"},
                &os.ProcAttr{
                        Env: os.Environ(),
                        Files: []*os.File{
                                os.Stdin,
                                os.Stdout,
                                os.Stderr,
                        },
                })
        if err != nil {
                fmt.Printf("Error %v starting process!", err)
                os.Exit(1)
        }
        fmt.Printf("The process id is %v\n", proc)

        _, err = proc.Wait()
        if err != nil {
                fmt.Printf("Error %v wait process!", err)
                os.Exit(1)
        }

}
{% endhighlight %}

### 进程管理

{% highlight go %}
package main

import (
        "errors"
        "fmt"
        "os"
        "os/exec"
        "sync"
        "syscall"
        "time"
)

type procInfo struct {
        cmdset  []string
        cmd     *exec.Cmd
        mu      sync.Mutex
        waitErr error
        cond    *sync.Cond
}

var procs map[string]*procInfo
var wg sync.WaitGroup

func init() {
        procs = make(map[string]*procInfo)

        procs["FOOBAR"] = &procInfo{
                cmdset: []string{"/bin/sh", "-c", "sleep 1000"},
        }

        for k, v := range procs {
                procs[k].cond = sync.NewCond(&v.mu)
        }
}

// spawn command that specified as proc.
func spawnProc(proc string) error {
        p := procs[proc]

        cmd := exec.Command(p.cmdset[0], p.cmdset[1:]...)
        cmd.Stdin = nil
        cmd.Stdout = nil // TODO: some log
        cmd.Stderr = nil
        cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
        cmd.Env = os.Environ()
        //cmd.Env = append(os.Environ(), fmt.Sprintf("PORT=%d", 1234))

        fmt.Printf("Starting %s\n", proc)

        err := cmd.Start()
        if err != nil {
                fmt.Printf("Failed to start %s: %s\n", proc, err)
                return err
        }
        p.cmd = cmd
        p.mu.Unlock()

        err = cmd.Wait()

        p.mu.Lock()
        p.cond.Broadcast()
        p.waitErr = err
        p.cmd = nil
        fmt.Printf("Terminating %s\n", proc)
        return nil
}

func terminateProc(proc string, signal os.Signal) error {
        p := procs[proc].cmd.Process
        if p == nil {
                return nil
        }

        pgid, err := syscall.Getpgid(p.Pid)
        if err != nil {
                return err
        }

        // use pgid, ref: http://unix.stackexchange.com/questions/14815/process-descendants
        pid := p.Pid
        if pgid == p.Pid {
                pid = -1 * pid
        }

        target, err := os.FindProcess(pid)
        if err != nil {
                return err
        }
        return target.Signal(signal)
}

func startProc(proc string) error {
        p, ok := procs[proc]
        if !ok || p == nil {
                return errors.New("unknown proc: " + proc)
        }

        p.mu.Lock()
        if procs[proc].cmd != nil { /* already running */
                p.mu.Unlock()
                return nil
        }

        wg.Add(1)
        go func() {
                spawnProc(proc)
                wg.Done()
                p.mu.Unlock()
        }()
        return nil
}

func stopProc(proc string, signal os.Signal) error {
        if signal == nil {
                signal = syscall.SIGTERM
        }
        p, ok := procs[proc]
        if !ok || p == nil {
                return errors.New("unknown proc: " + proc)
        }

        p.mu.Lock()
        defer p.mu.Unlock()

        if p.cmd == nil { /* not start yet */
                return nil
        }

        err := terminateProc(proc, signal)
        if err != nil {
                return err
        }

        timeout := time.AfterFunc(10*time.Second, func() {
                p.mu.Lock()
                defer p.mu.Unlock()
                if p, ok := procs[proc]; ok && p.cmd != nil {
                        err = p.cmd.Process.Kill()
                }
        })
        p.cond.Wait()
        timeout.Stop()

        return err
}

func main() {
        startProc("FOOBAR")

        stopProc("FOOBAR", nil)

        wg.Wait()
        fmt.Println("OK, All Done")
}
{% endhighlight %}

如上实际上是通过单个协程处理一个进程，也可以通过信号量来处理。

{% highlight go %}
signalC := make(chan os.Signal, 128)
signal.Notify(signalC, syscall.SIGCHLD)
defer signal.Stop(signalC)

go func() {
	for s := range signalC {
		switch s {
		case syscall.SIGCHLD:
			for {
				var wstatus syscall.WaitStatus

				pid, err := syscall.Wait4(-1, &wstatus, syscall.WNOHANG, nil)
				if syscall.EINTR == err {
					continue
				} else if syscall.ECHILD == err {
					break
				}

				fmt.Printf("Reaper cleanup: pid=%d, wstatus=%+v\n",
					pid, wstatus)
			}

		default:
			fmt.Println("Other", s)
		}
	}
}()
{% endhighlight %}


<!--
虽然 Go 中的协程相比系统线程来说已经是轻量级了，但是在高并发时，协程的频繁创建和销毁同样对 GC 造成很大的压力。

而协程池 [GitHub grpool](https://github.com/ivpusic/grpool) 就是对协程的封装。

也可以参考
https://www.cnblogs.com/276815076/p/8416652.html

Golang 公共变量包——expvar
https://www.jianshu.com/p/330512971c86
-->


{% highlight go %}
{% endhighlight %}
