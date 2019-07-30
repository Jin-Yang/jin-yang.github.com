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

{% highlight text %}
 addr       contents        running     access

       +-----------------+
       |   ~ StackTop ~  |
 High  +-----------------+
  |    |    rbp(start)   |     |
  |    +-----------------+     |(main)
  V    |  ARGS9 ~ ARGS7  |     |       16(%rbp) ~ 32(%rbp)
  |    +-----------------+     |
  V    |  ReturnAddress  |     v            8(%rbp)
  |    +-----------------+   -----
  |    |    rbp(main)    |     |             (%rbp)
  V    +-----------------+     |(foobar)
  |    |  LocalVariable  |     v           -4(%rbp)
 Low   +-----------------+   -----
{% endhighlight %}

栈帧的格式基本如下所示，`$rsp` 寄存器保存了当前栈的地址，可以通过 `pushq` `popq` `call` 等指令进行隐式操作，通过 `$rbp` 保存栈帧的地址，并进行相对寻址。

## 寄存器

在测试阶段，通常不会开启优化，所以，直接查看变量即可。对于线上的代码，通常需要开启代码优化，因此需要在调试时注意寄存器的使用情况。

{% highlight text %}
$rip                                  指令寄存器，指向当前执行的代码位置
$rsp                                  栈指针寄存器，指向当前栈顶，可以通过pushq popq进行自动操作
$rax $rbx $rcx $rdx $rsi $rdi $rbp    通用寄存器
$r8 $r9 $r10 $r11 $r12 $r13 $r14 $r15
{% endhighlight %}

## 函数传参

在具体的 CPU 硬件中，函数的运行需要借助硬件的栈 (Stack) 能力，为了保证各个模块的函数直接可以相互调用，那么就需要遵守 Calling Convention，这也是 ABI (Application Binary Interface) 的一部分。

如下的示例中，都是以如下函数作为测试。

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
	movl    %edi, -4(%rbp)
	movl    %esi, -8(%rbp)
	movl    %edx, -12(%rbp)
	movl    %ecx, -16(%rbp)
	movl    %r8d, -20(%rbp)
	movl    %r9d, -24(%rbp)
	movl    -8(%rbp), %eax
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
	movl    16(%rbp), %eax
	addl    %eax, %edx
	movl    24(%rbp), %eax
	addl    %eax, %edx
	movl    32(%rbp), %eax
	addl    %edx, %eax
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

<!--
函数调用过程
https://blog.csdn.net/Jogger_Ling/article/details/64443470
-->


{% highlight python %}
{% endhighlight %}
