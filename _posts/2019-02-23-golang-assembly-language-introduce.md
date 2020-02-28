---
title: GoLang 汇编语言
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

现在的汇编通常作为从高阶语言到机器码的中间产品，表示方式常见的有 Intel 以及 AT&T 两种，不知道为啥 GoLang 选择的是参考 Plan 9 这种汇编语言，其使用的方式与前两者有很大的区别。

这里简单介绍。

<!-- more -->

## 示例

{% highlight text %}
$ cat simple.go
package main

func main() {
        println(3)
}

$ GOOS=linux GOARCH=amd64 go tool compile -S simple.go
$ GOOS=linux GOARCH=amd64 go build -gcflags -S simple.go
{% endhighlight %}

其中 `-N -l` 参数用来禁止代码优化，或者使用 `-gcflags "-N -l"` 参数，也可以通过如下方式生成汇编代码。

{% highlight text %}
----- 编译成中间对象，然后反汇编，也可以指定函数
$ go tool compile -N -l simple.go
$ go tool objdump simple.o
$ go tool objdump -s main simple.o

----- 编译成二进制文件后反汇编
$ go build -o simple simple.go
$ go tool objdump -s main.main simple
{% endhighlight %}

## 指令集

### 符号

预定义的符号，用户定义的符号通过 FP SB 加上偏移进行计算。

{% highlight text %}
FP: Frame pointer: arguments and locals.
PC: Program counter: jumps and branches.
SB: Static base pointer: global symbols.
SP: Stack pointer: top of stack.
{% endhighlight %}

其中 `SB` 可以认为是内存的开始地址，那么 `foo(SB)` 就表示 `foo` 符号在内存中地址；然后可以通过 `foo<>(SB)` 表示仅对当前文件可见，类似于 C 中的 `static`；同时也可以在当前地址的基础上加个偏移 `foo+4(SB)` 。

`FP` 定义了虚拟的栈指针，用于函数的传参，在 64 位的机器上，`0(FP)` 表示第一个参数，`8(FP)` 表示第二个参数，以此类推。

### 函数定义

一般每个函数汇编的第一行代码类似如下。

{% highlight text %}
TEXT    "".Format(SB), $88-24
{% endhighlight %}

`"".` 表示了函数的命名空间，`Format(SB)` 表示函数在内存中的地址，

### 数据移动

通过 `$NUM` 表示常数，可以是负数，默认为 10 进制，也可以通过 `$0x123` 表示十六进制，通过后缀标识操作的字节数。

{% highlight text %}
MOVB $1,    DI   // 1B 赋值给DI
MOVW $0x10, BX   // 2B 赋值给BX
MOVD $-10,  DX   // 4B 赋值给DX
MOVQ $1,    AX   // 8B 赋值给AX
{% endhighlight %}

操作的顺序与 AT&T 语法相同，跟 Intel 语法相反。

### 地址计算

也就是 LEA 操作，全称为 Load Effective Address ，在 64 位平台上就是 LEAQ 指令。

{% highlight text %}
LEAQ type.int(SB), AX   // 将全局变量byte.int的地址赋值给AX
{% endhighlight %}

### 其它

#### FUNCDATA/PCDATA

这是 GoLang 自带的指令，用来给垃圾回收器标识，例如 `PCDATA $0, gclocals` 表示 `$0` 变量是局部变量，需要回收。

## 示例

循环累加一个数组中的成员，并打印结果。

{% highlight text %}
package main

func main() {
        t := 0
        l := []int{9, 45, 23, 67, 78}

        for _, v := range l {
                t += v
        }

        println(t)
}
{% endhighlight %}

关键操作部分摘抄如下。

{% highlight text %}
0x0041 00065 (main.go:4)   XORL   AX, AX   // 将两个寄存器清零，保存在循环中的偏移信息
0x0043 00067 (main.go:4)   XORL   CX, CX   // 保存了累加值

0x0045 00069 (main.go:7)   JMP    82       // 跳转到82指定的指令处
0x0047 00071 (main.go:7)   MOVQ   ""..autotmp_5+16(SP)(AX*8), DX  // 将数组AX个元素添加到DX
0x004c 00076 (main.go:7)   INCQ   AX       // 累加AX序号
0x004f 00079 (main.go:8)   ADDQ   DX, CX   // 做加法
0x0052 00082 (main.go:7)   CMPQ   AX, $5   // 将AX与5比较
0x0056 00086 (main.go:7)   JLT    71       // 如果上面结果小于0则跳转到71处

0x0058 00088 (main.go:11)  MOVQ   CX, "".t+8(SP)  // 结果保存到栈帧，准备调用println函数
{% endhighlight %}

上述的 `MOVQ   ""..autotmp_5+16(SP)(AX*8), DX` 部分相比比较复杂，单独拆出来介绍。

* `""..autotmp_5+16(SP)` 其中 `autocmp_*` 为自动生成的变量名，`SP` 为栈帧；
* `(AX*8)` 表示偏移，因为是64bits，所以乘以 8 ；

整体就是将 `l` 数组中的第 `AX` 个元素添加到 DX 寄存器中。

## 参考

* [A Quick Guide to Go's Assembler](https://golang.org/doc/asm) 官方关于汇编的介绍。

{% highlight text %}
{% endhighlight %}
