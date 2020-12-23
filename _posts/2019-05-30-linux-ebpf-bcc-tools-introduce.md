---
title: BCC 工具使用
layout: post
comments: true
language: chinese
category: [misc]
keywords: bpf,ebpf,bcc
description: 正常来说，如果要使用 eBPF 提供的能力，需要完成 "编写BPF代码-编译成字节码-注入内核-获取结果-展示" 这一整套流程，而且会非常复杂。而 bcc 是通过 Python 编写的一个 eBPF 工具集，使得上述的 整个过程更加便捷。
---

正常来说，如果要使用 eBPF 提供的能力，需要完成 "编写BPF代码-编译成字节码-注入内核-获取结果-展示" 这一整套流程，而且会非常复杂。

而 bcc 是通过 Python 编写的一个 eBPF 工具集，使得上述的 整个过程更加便捷。

<!-- more -->

## 简介

可以通过 `yum install bcc-tools` 安装。

注意，该工具依赖 Python2 ，所以，如果安装了 Python3，那么需要重新指定。

{% highlight text %}
# /usr/share/bcc/tools/killsnoop
TIME      PID    COMM             SIG  TPID   RESULT
15:50:42  23361  bash             15   23359  0
{% endhighlight %}

如上的测试用例，会捕获进程发送的信号。

## 开发

如下是一个最简单的示例，会监控 `clone()` 系统调用，并将相关的信息打印出来。

{% highlight python %}
#!/usr/bin/python

from bcc import BPF

BPF(text="""
int kprobe__sys_clone(void *ctx)
{
	bpf_trace_printk("Hello, World!\\n");
	return 0;
}
""").trace_print()
{% endhighlight %}

输出的内容如下。

{% highlight text %}
# python test.py
	bash-31608 [003] d...  4499.926795: : Hello, World!
	bash-31608 [003] d...  4499.926795: : Hello, World!
	bash-31608 [003] d...  4499.926795: : Hello, World!
	bash-31608 [000] d...  4511.861866: : Hello, World!
	... ...
{% endhighlight %}

其中上面的程序内容包括了如下几部分：

* `text='...'` 包含的是 C 语言编写的 BPF 程序；
* `kprobe__sys_clone()` 对应内核 kprobes 的动态跟踪，也就是 `sys_clone()` 接口；
* `void *ctx` 这里没有用到该参数，直接设置为 `void *` 类型；
* `bpf_trace_printk()` 一个通用的打印函数，会输出到 `trace_pipe` 中，如下详解；
* `.trace_print()` 用来读取上面的输出。

`bpf_trace_printk()` 会将信息输出到 `/sys/kernel/debug/tracing/trace_pipe` 文件中，不过有几个限制：A) 最大只有三个参数；B) 只能有一个 `%s` 输出；C) 全局公用，可能会写乱。

后面，可以使用 `BPF_PERF_OUTPUT()` 替换。

### 格式输出

如下的功能与上面功能相同，只是会将输出进行格式化。

{% highlight python %}
from bcc import BPF

# define BPF program
prog = """
int hello(void *ctx)
{
	bpf_trace_printk("Hello, World!\\n");
	return 0;
}
"""

# load BPF program
b = BPF(text=prog)
b.attach_kprobe(event=b.get_syscall_fnname("clone"), fn_name="hello")

# header and format output
print("%-18s %-16s %-6s %s" % ("TIME(s)", "COMM", "PID", "MESSAGE"))
while 1:
	try:
		(task, pid, cpu, flags, ts, msg) = b.trace_fields()
	except ValueError:
		continue
	print("%-18.9f %-16s %-6d %s" % (ts, task, pid, msg))
{% endhighlight %}

其输出内容如下。

{% highlight text %}
# python test.py
TIME(s)            COMM             PID    MESSAGE
4499.926795000     bash             13679  Hello, World!
4499.926795000     bash             13679  Hello, World!
4511.861866000     bash             13679  Hello, World!
4511.861866000     bash             13679  Hello, World!
... ...
{% endhighlight %}

如上使用 `hello()` 作为通用函数，而非使用 `kprobe__` 的前缀。接着，调用 `attach_kprobe()` 将 `hello()` 函数添加到系统调用 `clone()` 处，该函数可以调用多次。

### MAP 使用

一般在关机前，为了确保所有的数据都已经保存，通常会执行多次 `sync` 命令，这里会通过 MAP 保存上次的统计时间，如果两个 `sync` 的命令执行间隔小于 1 秒，那么就打印相关的信息。

{% highlight python %}
from __future__ import print_function
from bcc import BPF

# load BPF program
b = BPF(text="""
#include <uapi/linux/ptrace.h>

BPF_HASH(last);

int do_trace(struct pt_regs *ctx)
{
	u64 ts, *tsp, delta, key = 0;

	// attempt to read stored timestamp
	tsp = last.lookup(&key);
	if (tsp != 0) {
		delta = bpf_ktime_get_ns() - *tsp;
		// output if time is less than 1 second
		if (delta < 1e9)
			bpf_trace_printk("%d\\n", delta / 1000000);
	}

	// update stored timestamp
	ts = bpf_ktime_get_ns();
	last.update(&key, &ts);
	return 0;
}
""")

b.attach_kprobe(event=b.get_syscall_fnname("sync"), fn_name="do_trace")
print("Tracing for quick sync's... Ctrl-C to end")

# format output
start = 0
while 1:
	(task, pid, cpu, flags, ts, ms) = b.trace_fields()
	if start == 0:
		start = ts
	ts = ts - start
	print("At time %.2f s: multiple syncs detected, last %s ms ago" % (ts, ms))
{% endhighlight %}

这里的输出内容如下，可以通过 `sync; usleep 100000; sync` 命令测试。

{% highlight text %}
# python test.py
Tracing for quick sync's... Ctrl-C to end
At time 122.39 s: multiple syncs detected, last 35 ms ago
At time 126.98 s: multiple syncs detected, last 43 ms ago
At time 129.17 s: multiple syncs detected, last 50 ms ago
At time 133.50 s: multiple syncs detected, last 134 ms ago
{% endhighlight %}

其中 `bpf_ktime_get_ns()` 函数用来获取纳秒时间，而与 MAP 相关的操作有：A) `BPF_HASH(last)` 定义一个名称为 `last` 的 KV MAP ，其类型为 `uint64`；B) 其中相关的 `lookup()` `delete()` `update()` 等用来操作对象。

## 参考

* 详细可以参考 [Github IOVisor/BCC](https://github.com/iovisor/bcc) 中的相关介绍。
* 两个不错的文档 [bcc Python Developer Tutorial](https://github.com/iovisor/bcc/blob/master/docs/tutorial_bcc_python_developer.md) 以及 [bcc Tutorial](https://github.com/iovisor/bcc/blob/master/docs/tutorial.md) 。

{% highlight text %}
{% endhighlight %}
