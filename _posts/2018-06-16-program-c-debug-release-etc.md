---
title: C 语言发布流程
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,cpp,c++,program
description: 整理下 C 语言中调试、发布的流程。
---

整理下 C 语言中调试、发布的流程。

<!-- more -->



## 编码规范

这里直接使用的是 Linux 的编码规范，中文可以查看 [Linux 内核代码风格](https://www.kernel.org/doc/html/v4.15/translations/zh_CN/coding-style.html) 。

{% highlight text %}
$ scripts/checkpatch.pl --no-tree -f log.c
{% endhighlight %}

如果是通过命令行执行，则可以执行如下命令检查提交代码是否符和规范。

{% highlight text %}
$ git diff agent/*.[ch] | contrib/checkpatch.pl --no-tree --no-signoff -
{% endhighlight %}

另外，也可以添加到 git 的 `pre-commit` 脚本中。

{% highlight text %}
$ cat >.git/hooks/pre-commit
#!/bin/bash
exec git diff --cached agent/*.[ch] | contrib/checkpatch.pl --no-tree --no-signoff -
^D
$ chmod 755 .git/hooks/pre-commit
{% endhighlight %}

注意，`pre-commit` 不能添加到版本仓库中。

<!---
谈谈Linux内核驱动的coding style
http://www.cnblogs.com/wwang/archive/2011/02/24/1960283.html
-->

### FAQ

{% highlight text %}
ERROR: need consistent spacing around '*' (ctx:WxV)
#88: FILE: include/linux/pmem.h:167:
+static inline void flush_cache_pmem(void __pmem *addr, size_t size)
{% endhighlight %}

实际上是无法识别 `__pmem` 引起的，此时需要在脚本的 `our $Sparse` 中添加 `__pmem` 。

#### 不检查

如果不需要检查一行记录，可以通过添加宏覆盖掉，例如使用 `__SUPP(x)` 宏，并在 `foreach my ...` 中增加如下语句。

{% highlight text %}
if ($line=~/__SUPER/) {
	next;
}
{% endhighlight %}

<!--
## Linux 代码正式发布以及问题排查

## 0. 开发测试

## 1. RPM包安装

## 2. DebugInfo

http://www.unknownroad.com/rtfm/gdbtut/gdbstack.html
https://sourceware.org/gdb/onlinedocs/gdb/Backtrace.html
https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf
http://www.brendangregg.com/blog/2016-08-09/gdb-example-ncurses.html
-->

## 正式发布



Backtrace 会有如下的问题：

* 没有行信息，无法确认是那个函数的那一行出了问题；
* 没有入参以及本地变量；
* 打印参数名仅供参考，因为static和inline的会被删除，除非添加了 `-rdynamic` 参数；

前两者实际上没有太好的办法处理，而第 3 点实际上可以通过一些手段进行定位。首先，需要了解栈是如何生成的。

CPU 中的有指针保存栈顶的寄存器，而每个栈帧同时包含了其调用的栈帧以及函数返回的地址，然后再通过当前的指针地址，就开确定当前执行的是那个函数。

同样，这里面会有几个问题：

1. 如果调用顺序为 `foo()->bar()` ，当 `bar()` 返回类型为 `void` ，那么可能直接忽略 `bar()` 跳转到 `foo()` 。
2. 在打印栈时，会在内存中查找最近的符号表，对于 `static` 或者 `inline` 类型的函数会被忽略。

通过上述内容的介绍基本上可以确认函数的执行顺序。

### 打印栈

最简答的是使用 GCC 的宏定义在日志中打印。

{% highlight c %}
#define TRACE_MSG fprintf(stderr, __FUNCTION__ "() [%s:%d] here I am\n", __FILE__, __LINE__)
{% endhighlight %}

也可以通过 `backtrace()` 获取调用栈，然后使用 `backtrace_symbols()` 解析其地址，或者会返回相对最近函数的偏移以及返回地址。

另外，也可以通过 `backtrace_symbols_fd()` 将栈的信息直接打印到文件中。

#### 触发位置

一般来说，需要在一些常见的异常信号中打印问题栈，例如 `SIGSEGV` `SIGBUS` `SIGILL` `SIGFPE` 等信号处理中。那么在使用之前需要先了解在内核中，是如何处理信号的。

1. 当内核需要向进程发送信号时，先分配一些所需要的信息添加到 `struct task` 结构体中，然后将 `signal-pending` 置位。
2. 在进程被调度到之前，内核会修改栈的结构，用来调用信号处理函数。
3. 在用户态中，一般先进入的是 libc 的函数，然后才是用户定义的函数。

所以，在上述的调用栈中，实际上大致内容如下，真正的回调函数在 `sigaction()` 之前。

{% highlight text %}
your_sig_handler()
sigaction() in libc.so
your_foobar()
{% endhighlight %}

<!--
https://www.linuxjournal.com/article/6391
sigcontext
-->

假设有如下的报错：

{% highlight text %}
 [0]: ./a.out() [0x400772]
 [1]: /lib64/libc.so.6(+0x35250) [0x7fc8014f7250]
 [2]: ./a.out() [0x400824]
 [3]: ./a.out() [0x40083a]
 [4]: ./a.out() [0x400859]
 [5]: /lib64/libc.so.6(__libc_start_main+0xf5) [0x7fc8014e3b35]
 [6]: ./a.out() [0x400689]
{% endhighlight %}

可以通过 `x` 命令查看其调用地址的反汇编代码。

{% highlight text %}
(gdb) x/20i 0x6f42a-30
{% endhighlight %}

### BackTrace

在代码中，如果发生异常，直接 coredump 会导致文件过大，此时可以通过 `backtrace` 在代码中打印日志信息。

{% highlight c %}
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>

void log_backtrace(int signal)
{
        (void) signal;
        int i, frames;
        void *callstack[128];
        char **strs;

        frames = backtrace(callstack, 128);
        strs = backtrace_symbols(callstack, frames);
        if (strs == NULL) {
                printf("ERROR: backtrace_symbols failed, %s", strerror(errno));
                exit(1);
        }

        for (i = 0; i < frames; ++i)
                printf(" [%d]: %s\n", i, strs[i]);
        free(strs);
        exit(1);
}

int bar(void)
{
        int *addr = NULL;

        *addr = 3;
        return 0;
}

int foo(void)
{
        bar();
        return 0;
}


int main(void)
{
        signal(SIGSEGV, log_backtrace);
        foo();
        return 0;
}
{% endhighlight %}

<!--
https://www.linuxjournal.com/article/6391
https://www.mawenbao.com/research/glibc-backtrace-parsing.html
http://silencewt.github.io/2015/05/11/Segmentation-Fault%E9%94%99%E8%AF%AF%E5%8E%9F%E5%9B%A0%E6%80%BB%E7%BB%93/
-->

### 符号表

直接通过 `gcc -o test main.c` 进行编译时，打印的信息不含具体是那个函数，都是 16 进制的地址，此时可以通过反汇编获取，不过比较麻烦。

可以通过 `objdump -d test > test.s` 命令直接反汇编。

此时可以在编译时添加 `-rdynamic` 参数，其作用是：

{% highlight text %}
-rdynamic
    Pass the flag -export-dynamic to the ELF linker, on targets that support it. This
instructs the linker to add all symbols, not only used ones, to the dynamic symbol
table. This option is needed for some uses of "dlopen" or to allow obtaining backtraces
from within a program.
{% endhighlight %}

另外，对于 C++ 其符号名是通过 mangle 之后的，为了获取具体的函数信息，需要 demangle 处理，例如：

{% highlight text %}
c++filt  "_Z16print_stacktracev"
{% endhighlight %}

当然，此时是无法查看函数是具体的哪一行的，可以利用 `addr2line` 查看，此时在编译时需要添加 `-g` 选项，例如：

{% highlight text %}
addr2line -Cifp -a 0x400a29 -e test
{% endhighlight %}

`-f` 用来打印函数名，`-C` 同时 demangle 处理，`-i` 同时处理 inline 函数。

<!--
1. 不包含任何的行信息，无法确定具体那个函数出问题。
2. 无法确定函数的入参以及本地的变量值；
3. 无法确定打印的栈函数是否为静态。

They contain no line number information, so you don’t know where in
a function something happened
You cannot see the values of arguments and local variables
You cannot trust the function names given in the backtrace, since
the debugger doesn’t know about static functions.


disas 0x41dde0,0x41ef00
disas 0x41e500,0x41efff


对程序进行汇编级调试。


----- (run)重新开始运行
(gdb) r
----- (break)针对地址设置断点
(gdb) b *0x0804ce2b

单步步过
(gdb) ni    (next instruction)
单步步入
(gdb) si    ( step instruction )
继续执行
( gdb )c

执行到返回
(gdb) finish










## Linux 代码正式发布以及问题排查

## 0. 开发测试
## 1. RPM包安装

## 2. DebugInfo

https://www.mawenbao.com/research/glibc-backtrace-parsing.html

使用库函数backtrace和backtrace_symbols定位段错误
http://blog.sina.com.cn/s/blog_590be5290102w5yw.html
https://blog.csdn.net/ieearth/article/details/49763481
https://blog.csdn.net/astrotycoon/article/details/8142588
http://silencewt.github.io/2015/05/11/Segmentation-Fault%E9%94%99%E8%AF%AF%E5%8E%9F%E5%9B%A0%E6%80%BB%E7%BB%93/

while true ; do  echo -e "HTTP/1.1 200 OK\n\n $(date)" | nc -l -p 1500  ; done




关于AK/SK介绍
https://bbs.huaweicloud.com/blogs/079b918999c111e7b8317ca23e93a891

https://github.com/jobbole/awesome-c-cn
-->

## 其它

### LIB 库版本问题

通常是因为当前系统的依赖库版本太低，而软件编译时使用了较高版本库引起的，常见的有 `glibc` `pthread` 等，一般报错如下。

{% highlight text %}
/lib64/libc.so.6: version `GLIBC_2.14' not found
{% endhighlight %}

可以通过如下步骤进行排查。

{% highlight text %}
----- 1. 当前系统支持版本
$ strings /lib64/libc.so.6 | grep GLIBC_
----- 2. 使用二进制文件依赖的版本
$ strings /your/binary/exec | grep GLIBC_
{% endhighlight %}

注意，如果在 `strip` 之前保存了符号表，实际上可以通过  `nm BINARY` 查看依赖库的版本号，不过 `strip` 之后则会丢失。



## 参考

符号表的详细信息可以查看 [ELF 符号表]({{ site.production_url }}/post/program-c-elf-symbol-section.html) 中的介绍。

关于 DebugInfo 的详细介绍可以查看 [MySQL Core 文件]({{ site.production_url }}/post/mysql-core-file.html) 中的介绍。



{% highlight text %}
{% endhighlight %}
