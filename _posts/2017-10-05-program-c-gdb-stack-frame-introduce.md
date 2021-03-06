---
title: GDB 栈帧简介
layout: post
comments: true
language: chinese
category: [program]
keywords:  program,c,stack frame
description: 栈是一块内存空间，会从高地址向低地址增长，同时在函数调用过程中，会通过栈寄存器来维护栈帧相关的内容。函数运行时，栈帧 (Stack Frame) 非常重要，包含了函数的局部变量以及函数调用之间的传参。
---

栈是一块内存空间，会从高地址向低地址增长，同时在函数调用过程中，会通过栈寄存器来维护栈帧相关的内容。

函数运行时，栈帧 (Stack Frame) 非常重要，包含了函数的局部变量以及函数调用之间的传参。

<!-- more -->

## 简介

这里的介绍都是以 x86_64 为基础，而栈帧的操作大部分是与寄存器相关，不同的架构使用寄存器的方式略有区别。

{% highlight text %}
addr       contents       running          access           comments

High  +-----------------+
 |    |   ~ StackTop ~  |
 |    +-----------------+ -----
 |    |    rbp(start)   |   |
 |    +-----------------+   |(main)
 |    |  ARGS9 ~ ARGS7  |   V     16(%rbp) ~ 32(%rbp) <- caller将参数压栈
 |    +-----------------+ -----
 |    |  ReturnAddress  |   |          8(%rbp)        <- call指令默认压栈操作
 |    +-----------------+   |
 |    |    rbp(main)    |   |           (%rbp)        <- callee负责保存上个函数栈基址方便恢复
 |    +-----------------+ -----
 |    +  ARGS6 ~ ARGS0  +   |
 |    +-----------------+   | (foobar)
 V    |  LocalVariable  |   V         -4(%rbp)
Low   +-----------------+ -----
{% endhighlight %}

栈帧的格式基本如下所示，`$rsp` 寄存器保存了当前栈的地址，可以通过 `pushq` `popq` `call` 等指令进行隐式操作，通过 `$rbp` 保存栈帧的地址，并进行相对寻址。

## 寄存器

在测试阶段，通常不会开启优化，所以，直接查看变量即可。对于线上的代码，通常需要开启代码优化，因此需要在调试时注意寄存器的使用情况。

{% highlight text %}
$rip                                  指令寄存器，指向当前执行的代码位置
$rsp                                  栈指针寄存器，指向当前栈顶，可以通过pushq popq进行自动操作
$rbp                                  栈帧指针，用来标示当前栈帧的起始位置；

$rax $rbx $rcx $rdx $rsi $rdi $rbp    通用寄存器
$r8 $r9 $r10 $r11 $r12 $r13 $r14 $r15
{% endhighlight %}

另外，`%rdi` `%rsi` `%rdx` `%rcx` `%r8` `%r9` 六个寄存器用于存储函数调用的前六个参数，超过则通过栈传递；`%rax` 用来返回结果。

另外，需要区分 "Caller Save" 以及 "Callee Save" 寄存器，在某个函数中，会使用到通用寄存器，那么在子函数中这些寄存器的值可能被覆盖，所以需要确定寄存器的保存方式。

## 函数传参

在具体的 CPU 硬件中，函数的运行需要借助硬件的栈 (Stack) 能力，为了保证各个模块的函数直接可以相互调用，那么就需要遵守 Calling Convention，这也是 ABI (Application Binary Interface) 的一部分。

详细的可以通过 `man syscall` 查看，如下的示例中，都是以如下函数作为测试。

{% highlight c %}
int foobar(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
        return a + b + c + d + e + f + g + h + i;
}

int main(void)
{
        return foobar(1, 2, 3, 4, 5, 6, 7, 8, 9);
}
{% endhighlight %}

### 汇编

可以通过 `gcc -S main.c` 查看对应的汇编代码。

{% highlight text %}
foobar:
	pushq   %rbp              # 保存上次的栈
	movq    %rsp, %rbp        # 同时使用rbp进行栈的快速操作
	movl    %edi, -4(%rbp)    # 将通过寄存器传递的参数保存在栈中
	movl    %esi, -8(%rbp)
	movl    %edx, -12(%rbp)
	movl    %ecx, -16(%rbp)
	movl    %r8d, -20(%rbp)
	movl    %r9d, -24(%rbp)   # 到此为止
	movl    -8(%rbp), %eax    # 开始加法计算，edx保存了计算结果
	movl    -4(%rbp), %edx
	addl    %eax, %edx
	movl    -12(%rbp), %eax
	addl    %eax, %edx
	movl    -16(%rbp), %eax
	addl    %eax, %edx
	movl    -20(%rbp), %eax
	addl    %eax, %edx
	movl    -24(%rbp), %eax
	addl    %eax, %edx
	movl    16(%rbp), %eax    # 这里是通过栈传递的参数
	addl    %eax, %edx
	movl    24(%rbp), %eax
	addl    %eax, %edx
	movl    32(%rbp), %eax
	addl    %edx, %eax        # 计算最后一次加法同时将结果保存在eax中
	popq    %rbp              # 恢复main的栈
	ret

main:
	pushq %rbp                # 将$rsp减一个指针长度(8Bytes)，并将$rbp的值写入到rsp指向的地址处
	movq　%rsp, %rbp          # 将$rsp赋值给rbp寄存器，完成main栈帧的保存
	subq  $24, %rsp           # 需要通过栈传递三个参数，每个参数占用8Bytes(实际有效的是高4Bytes)

	movl  $9, 16(%rsp)
	movl  $8, 8(%rsp)
	movl  $7, (%rsp)

	movl  $6, %r9d            # 剩余的6个参数通过寄存器进行传递
	movl  $5, %r8d
	movl  $4, %ecx
	movl  $3, %edx
	movl  $2, %esi
	movl  $1, %edi
	call  foobar              # 将地址添加到栈顶
	leave
{% endhighlight %}

一般 Linux 下会优先将参数压到寄存器中，只有当寄存器不够所有的参数时，才会将入参压到栈上，一般入参的压栈顺序为 `$rdi` `$rsi` `$rdx` `$rcx` `$r8` `$r9` ，返回值通过 `$rax` 传递。

如上的示例中，第 7 8 9 个参数会通过栈进行传递，也就是从 `$rsp` 向下的地址

另外，可以看到函数的入参方式是从右到左。

从汇编看，完成一个函数调用关键执行就是 `call` `pushq` `popq` `ret` `leave` 等指令。

### 寄存器

可以通过如下方式验证上面的内容。

{% highlight text %}
(gdb) info registers      # 可以简写为i r
rax            0xe      14
rbx            0x0      0
rcx            0x4      4
rdx            0x3      3
rsi            0x2      2
rdi            0x1      1
rbp            0x7fffffffe0c0   0x7fffffffe0c0
rsp            0x7fffffffe0c0   0x7fffffffe0c0
r8             0x5      5
r9             0x6      6
... ...

(gdb) x/10xw 0x7fffffffe0c0
0x7fffffffe0c0: 0xffffe0f0      0x00007fff      0x004005cd      0x00000000
0x7fffffffe0d0: 0x00000007      0x00000000      0x00000008      0x00000000
0x7fffffffe0e0: 0x00000009      0x00007fff
{% endhighlight %}

如上为小端地址，所以包括了 `$rbp` 地址 `0x7fffffffe0f0` ，返回地址为 `0x4005cd` ，以及参数 7 8 9 。因为，入参为 4 字节，所以高位实际上是无效的，可能含有脏数据。

另外，可以直接通过 `info frame` 查看当前栈的信息，包括了参数信息。

## 获取中断号

有些时候会在中断处发生死锁，但是很难确认中断号是多少，例如如下的示例。

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void foobar(int sig)
{
        fprintf(stdout, "got signal %d.\n", sig);
}

void handler(int sig)
{
        foobar(sig);
}

int main()
{
        if (signal(SIGIO, handler) == SIG_ERR) {
                fprintf(stderr, "register handler for SIGIO failed, %d:%s.",
                                errno, strerror(errno));
                return -1;
        }

        while(1);

        return 0;
}
{% endhighlight %}

通过 `gcc -o foobar foobar.c -O0 -g` 进行编译，如果使用 `-O2` 或者默认，有可能设置的断点地址不匹配。

有的时候，如果存在信号不安全的函数，那么就可能会发生死锁，而此时通过 gdb 获取栈时，会发现在某个栈帧处，有如下的信息。

{% highlight text %}
(gdb) bt
#0  0x00000000004004dc in foo ()
#1  0x00000000004004f8 in signal_handler ()
#2  <signal handler called>
#3  0x000000000040050d in main ()
{% endhighlight %}

但是没有提供具体的参数信息，主要是如果要 gdb 打印信息，那么必须要知道参数个数、各个参数大小等信息。

不过还好，我们知道信号处理函数的入参及其大小，也就是只有一个用来标示那个信号的参数，可以直接切换到 `frame 1` ，然后通过上述方式查看。

<!--
(gdb) info frame 1
Stack frame at 0x7fff84277c30:
rip = 0x4004f8 in handler; saved rip 0x301e2301b0
called by frame at 0x7fff84277c38, caller of frame at 0x7fff84277c10
Arglist at 0x7fff84277c20, args:
Locals at 0x7fff84277c20, Previous frame's sp is 0x7fff84277c30
Saved registers:
  rbp at 0x7fff84277c20, rip at 0x7fff84277c28

也就是参数在 `0x7fff84277c20` 地址处，因为入参是 32bit 的，然后可以通过如下打印具体的值。

(gdb) x/1 0x7fff84277c1c
0x7fff84277c1c:    0x0000001d

x/1 0x7ffcfc8e4194

注意，详细的传参可以通过上述的栈帧查看。
-->

如果是多线程，同时又使用了信号的同步机制，那么就可能会在不同的线程中出现多个信号处理。

<!--
函数调用过程
https://blog.csdn.net/Jogger_Ling/article/details/64443470


多线程环境下的内存调试
http://www.0xffffff.org/2017/01/22/39-multi-thread-memory-bug/
https://zhuanlan.zhihu.com/p/27339191
-->


{% highlight python %}
{% endhighlight %}
