---
title: eBPF 简介
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

BPF 的目的是尽量早的过滤掉不需要的报文，所以，会通过用户空间的一些工具 (例如 tcpdump)，通过 `bpf()` 系统 API 将指令发送到内核。而内核为了防止恶意代码，会先进行静态扫描，然后再加载运行。

最开始的 BPF 只用于网络包的过滤，而过滤器的执行是基于寄存器的虚拟机。

### BPF

实际上 tcpdump 命令所使用的 libpcap 库是基于 BPF 的，在使用相关参数过滤报文时 (例如 `host 192.168.1.1` `tcp and port 80` 等)，这些表达式会被编译成 BPF 指令。

可以通过 `-d` 参数，将对应的指令显示出来。

{% highlight text %}
# tcpdump -d -i eth0 tcp and port 80
{% endhighlight %}

## eBPF

原有的 BPF(Classic BPF, cBPF) 依然支持，而 eBPF 设计了更丰富的指令集、增加了寄存器，更接近于真实硬件，性能大幅提高。

> The original patch that added support for eBPF in the 3.15 kernel showed that eBPF was up to four times faster on x86-64 than the old classic BPF (cBPF) implementation for some network filter microbenchmarks, and most were 1.5 times faster.

RHEL 8 采用的是 4.18 内核版本，支持 eBPF、cgroup V2 等比较新的特性，不过对于 CentOS 7 来说，需要先升级内核版本以及开发库。

### 调试工具

使用 `bpftool` 工具可以查看当前已经加载的 eBPF 程序，通过 `yum install bpftool` 命令安装，不过要注意内核版本。

在最新的内核源码里应该包含了该工具的源码，可以手动编译。

<!--
https://www.redhat.com/en/blog/introduction-ebpf-red-hat-enterprise-linux-7

bpftool prog list
bpftool prog dump xlated id 3
-->

也可以使用 `bcc-tools` 中的 `/usr/share/bcc/tools/bpflist` 命令查看。

{% highlight text %}
# /usr/share/bcc/tools/bpflist
PID	COMM         	TYPE 	COUNT
13159  killsnoop    	prog 	2   
13159  killsnoop    	map  	2   
{% endhighlight %}


<!--
另外，增加了 `bpf()` 系统调用，用来与内核中的 eBPF 进行交互。

{% highlight c %}
int bpf(int cmd, union bpf_attr *attr, unsigned int size);
{% endhighlight %}

### 指令集

一条 eBPF 指令 8 字节长，对应的数据结构如下。

{% highlight c %}
struct bpf_insn {
	__u8  code;       /* opcode 操作码 */
	__u8  dst_reg:4;  /* dest register 目标寄存器 */
	__u8  src_reg:4;  /* source register 源寄存器 */
	__s16 off;        /* signed offset 偏移 */
	__s32 imm;        /* signed immediate constant 立即数 */
};
{% endhighlight %}

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


https://linux.cn/article-9507-1.html?pr
https://blog.csdn.net/pwl999/article/details/82884882
https://linux.cn/article-9630-1.html
-->


## 示例

在内核代码的 `samples/bpf` 目录下，包含了一个 libbpf 的库，可以不用直接调用 bpf() 接口。

## 参考

* 性能分析的大牛 Brendan Gregg 提供了很多参考资料，包括了 [Linux Extended BPF Tracing Tools](http://www.brendangregg.com/ebpf.html) 以及 [Golang bcc/BPF Function Tracing](http://www.brendangregg.com/blog/2017-01-31/golang-bcc-bpf-function-tracing.html) 。
* [Awesome eBPF](https://github.com/zoidbergwill/awesome-ebpf) 在 GitHub 中整理的一些与 eBPF 相关的资料。


<!--
关于BPF详细的介绍，详细清单
https://linux.cn/article-9507-1.html


https://opensource.com/article/17/9/intro-ebpf

关于cBPF的相关介绍
https://www.tcpdump.org/papers/
https://www.kernel.org/doc/Documentation/networking/filter.txt

## eBPF

eBPF 有点类似于 V8 引擎，

https://github.com/iovisor/bcc/blob/master/docs/tutorial.md
https://github.com/iovisor/bcc/blob/master/docs/tutorial_bcc_python_developer.md




eBPF的相关资料
https://github.com/zoidbergwill/awesome-ebpf
https://qmonnet.github.io/whirl-offload/2016/09/01/dive-into-bpf/


介绍如何使用eBPF的最简单内容
https://github.com/pratyushanand/learn-bpf/
https://opensource.com/article/17/9/intro-ebpf

关于eBPF的概览
http://vger.kernel.org/netconf2015Starovoitov-bpf_collabsummit_2015feb20.pdf


## Startify
https://blog.csdn.net/mdl13412/article/details/44081489

eBPF 从头开始，一篇很详细的文章
https://bolinfest.github.io/opensnoop-native/
https://github.com/bolinfest/rust-ebpf-demo

ulimits的设置
https://feichashao.com/ulimit_demo/
https://blogs.oracle.com/linux/notes-on-bpf-1


https://sematext.com/blog/linux-kernel-observability-ebpf/
-->



{% highlight text %}
{% endhighlight %}
