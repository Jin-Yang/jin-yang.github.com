---
title: Linux eBPF 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: bpf,ebpf
description: Berkeley Packet Filter, BPF 提供了强大的网络包过滤规则，可以确定应该检查哪些流量、忽略哪些流量等，而内核近几年发展的 Extended BPF, eBPF 实际上将应用范围，处理效率进行了更新。eBPF 是 Linux 内核近几年最为引人注目的特性之一，通过一个内核内置的字节码虚拟机，完成数据包过滤、调用栈跟踪、耗时统计、热点分析等等高级功能。
---

Berkeley Packet Filter, BPF 提供了强大的网络包过滤规则，可以确定应该检查哪些流量、忽略哪些流量等，而内核近几年发展的 Extended BPF, eBPF 实际上将应用范围，处理效率进行了更新。

eBPF 是 Linux 内核近几年最为引人注目的特性之一，通过一个内核内置的字节码虚拟机，完成数据包过滤、调用栈跟踪、耗时统计、热点分析等等高级功能。

<!-- more -->

## 简介

最开始的 BPF 只用于网络包的过滤，过滤器是基于寄存器的虚拟机。

### BPF

实际上 tcpdump 命令所使用的 libpcap 库是基于 BPF 的，在使用相关参数过滤报文时 (例如 `host 192.168.1.1` `tcp and port 80` 等)，这些表达式会被编译成 BPF 指令。

可以通过 `-d` 参数，将对应的指令显示出来。

{% highlight text %}
# tcpdump -d -i eth0 tcp and port 80
{% endhighlight %}

## eBPF

原有的 BPF(Classic BPF, cBPF) 依然支持，而 eBPF 设计了更丰富的指令集、增加了寄存器，更接近于真实硬件，性能大幅提高。

> The original patch that added support for eBPF in the 3.15 kernel showed that eBPF was up to four times faster on x86-64 than the old classic BPF (cBPF) implementation for some network filter microbenchmarks, and most were 1.5 times faster.

另外，增加了 `bpf()` 系统调用，用来与内核中的 eBPF 进行交互。

{% highlight c %}
int bpf(int cmd, union bpf_attr *attr, unsigned int size);
{% endhighlight %}

### 指令集

一条 eBPF 指令 8 字节长，对应的数据结构如下。

{% highlight c %}
struct bpf_insn {
	__u8	code;		/* opcode */
	__u8	dst_reg:4;	/* dest register */
	__u8	src_reg:4;	/* source register */
	__s16	off;		/* signed offset */
	__s32	imm;		/* signed immediate constant */
};
{% endhighlight %}


<!--
共8位，0,1,2这三位表示的是该操作的大类别：0X07

BPF_LD(0x00) /   BPF_LDX(0x01) /   BPF_ST(0x02) /  BPF_STX(0x03) /   BPF_ALU(0x04) /   BPF_JMP(0x05) /   BPF_RET(0x06) /    BPT_MISC(0x07)

对于 LD大类 来说：

3,4位表示的是位宽0x00 代表4字节，0x08：2字节，0x10：1个字节 0X18

5.6.7三位表示的是：0XE0

BPF_IMM / BPF_ABS / BPF_IND / BPF_MEM / BPF_LEN / BPF_MSH

对于ALU和JMP大类来说：

4,5,6,7 高四位表示的是具体的操作：0xf0

BPF_ADD / BPF_SUB / BPF_MUL / BPF_DIV / BPF_OR / BPF_AND / BPF_LSH / BPF_RSH / BPF_NEG / BPF_MOD / BPF_XOR / BPF_JA / BPF_JEQ / BPF_JGT / BPF_JGE / BPF_JSET /

第3位表示的是：0x08

BPF_K/BPF_X

BPF使用的寄存器包括：

* R0    - return value from in-kernel function, and exit value for eBPF program
* R1 - R5   - arguments from eBPF program to in-kernel function
* R6 - R9   - callee saved registers that in-kernel function will preserve
* R10   - read-only frame pointer to access stack

code 操作码(8 bit)

目标寄存器(4 bit)，源寄存(4 bit)，偏移(16bit)，立即数(32bit)


https://linux.cn/article-9507-1.html?pr
https://blog.csdn.net/pwl999/article/details/82884882
https://linux.cn/article-9630-1.html
-->



## BCC

bcc 是通过 Python 编写的一个 eBPF 工具集，使得 "编写BPF代码-编译成字节码-注入内核-获取结果-展示" 整个过程更加便捷。

详细可以参考 [Github IOVisor/BCC](https://github.com/iovisor/bcc) 中的相关介绍。

## 参考

* 性能分析的大牛 Brendan Gregg 提供了很多参考资料，包括了 [Linux Extended BPF Tracing Tools](http://www.brendangregg.com/ebpf.html) 以及 [Golang bcc/BPF Function Tracing](http://www.brendangregg.com/blog/2017-01-31/golang-bcc-bpf-function-tracing.html) 。


<!--
RHEL 8 采用的是 4.18 内核版本，支持 eBPF、cgroup V2 等比较新的特性。

关于BPF详细的介绍，详细清单
https://linux.cn/article-9507-1.html
-->



{% highlight text %}
{% endhighlight %}
