---
title: GDB 基本使用
layout: post
comments: true
language: chinese
category: [program]
keywords: gdb
description:
---

GNU Project Debugger, GDB 一个代码调试工具，通过系统提供的 ptrace 接口实现的控制进程，然后可以在进程内部查看信息，甚至调用函数。

<!-- more -->

## 简介

在使用 gdb 时，需要通过 `-g` 参数把调试信息加到可执行文件中，否则没有函数名、变量名、代码地址，所代替的全是运行时的内存地址。

{% highlight text %}
> cc -g hello.c -o hello
> g++ -g hello.cpp -o hello
{% endhighlight %}

gdb 详细使用方式可以通过 `gdb --help` 查看。

注意，gdb 在保证命令不冲突的前提下，提供了简写命令，例如 `list` 查看源码，实际上通过 `l` 也可以。

## 启动

常见的有如下几种启动方式。

{% highlight text %}
----- 直接通过GDB启动进程
$ gdb <program>
----- 调试Core文件
$ gdb <program> core
----- Attach到正在运行的进程上，也可以进入gdb后执行attach命令
$ gdb <program> <PID>
{% endhighlight %}

另外一些启动时的常用参数为。

{% highlight text %}
-symbols/-s <file>
   指定文件读取符号表。
-se file
   从指定文件中读取符号表信息，并把他用在可执行文件中。
-core/-c <file>
   调试Core文件。
-directory/-d <directory>
   加入一个源文件的搜索路径，默认是PATH指定的路径。
{% endhighlight %}

是否查找到源文件，可以通过 `list/l` 命令查看。

### 参数设置

在通过 `r/run` 正式运行前，可以通过如下方式设置运行参数、环境变量等信息。

{% highlight text %}
----- 程序的运行参数设置、查看
(gdb) set args <arguments>

----- 查看设置好的运行参数
(gdb) show args
{% endhighlight %}

<!--
### 运行环境

path <dir> 可设定程序的运行路径。
show paths 查看程序的运行路径。
set environment varname [=value] 设置环境变量。如：set env USER=hchen
show environment [varname] 查看环境变量。

3、工作目录。
cd <dir> 相当于shell的cd命令。
pwd 显示当前的所在目录。

-->


### 内存查看

也就是 `examine` 命令，通常简写为 `x` ，对应的命令格式为 `x/nfu <ADDR>` 。

{% highlight text %}
* n 显示内存个数；
* f 显示方式；
  - x 按十六进制格式显示变量；
  - d 按十进制格式显示变量；
  - u 按十进制格式显示无符号整型；
  - o 按八进制格式显示变量；
  - t 按二进制格式显示变量；
  - a 按十六进制格式显示变量；
  - i 指令地址格式；
  - c 按字符格式显示变量；
  - f 按浮点数格式显示变量；
* u 一个地址单元的长度；
  - b 单字节；
  - h 双字节；
  - w 四字节；
  - g 八字节；
{% endhighlight %}

### 信息查看

主要是通过 `info` 命令查看各种信息。


{% highlight text %}
----- 查看函数信息，可以使用正则表达式
(gdb) info functions
{% endhighlight %}


#### 变量查看

包括了全局、局部、静态变量，以及调用函数的参数。

{% highlight text %}
----- 查看所有的全局和静态变量
(gdb) info variables
----- 当前栈的局部变量，包括了本函数中的静态变量
(gdb) info locals
----- 查看参数
(gdb) info args
{% endhighlight %}

### 其它

{% highlight text %}
----- 查看版本信息，默认启动时会打印
(gdb) show version

----- 查看版权信息
(gdb) show copying
(gdb) show warranty

----- 退出时无需确认，直接退出
(gdb) set confirm off

----- 关闭分页，会将信息全部输出
(gdb) set pagination off
(gdb) set height 0
{% endhighlight %}

其中时通过 `-q` 或者 `--quiet` 可以禁止打印版本信息，

## 多线程

先介绍一下GDB多线程调试的基本命令。

{% highlight text %}
info threads                                 显示所有线程，通过星号标示当前线程
thread ID                                    切换当前调试的线程为指定ID的线程。
thread apply <all|ID1 ID2 ... IDn> <command> 指定多个线程执行命令
break <filename:lineno> thread all           在所有线程上设置断点
{% endhighlight %}


{% highlight text %}
set scheduler-locking off|on|step
{% endhighlight %}

调试多线程时，如果使用 step 或者 continue 命令调试当前被调试线程，其他线程也是同时执行的，可以通过如上的参数进行设置，参数的含义为：

* off 默认值，不锁定任何线程，也就是所有线程都执行；
* on 只有当前被调试程序会执行；
* step 在单步的时候，除了next过一个函数以外，只有当前线程会执行。

## 死锁查看

{% highlight text %}
(gdb) info threads                # 可以查看那些线程在等锁
(gdb) thread apply all bt
{% endhighlight %}












<!--
注意，大部分的函数，例如 `backtrace()` `printf()` `malloc()` 并不是信号安全的，会有概率导致死锁。

所以，一般在实践时，对一些异常场景打印栈，简单来说，就是 `So it is a calculated risk we are taking.` 。

https://software.intel.com/en-us/articles/how-memory-is-accessed


4、程序的输入输出。
info terminal 显示你程序用到的终端的模式。
使用重定向控制程序输出。如：run > outfile
tty命令可以指写输入输出的终端设备。如：tty /dev/ttyb

具体来说就是

例如一个程序名为prog 参数为 -l a -C abc
则，运行gcc/g++ -g  prog.c/cpp -o prog
就可以用gdb调试程序prog
#gdb prog
进入gdb调试界面
输入参数命令set args 后面加上程序所要用的参数，注意，不再带有程序名，直接加参数，如：
set args -l a -C abc
回车后输入
r
即可开始运

Coredump 是进程运行时在突然崩溃的那一刻的一个内存快照，包括了内存、寄存器状态、运行堆栈等信息。

在 Linux 中，可以使用 gdb、elfdump、objdump 等工具查看。

(gdb) backtrace      # bt   查看当前调用栈
(gdb) frame 1        # f 1  切换到Frame #1
(gdb) disassemble    # 该函数的反汇编
(gdb) info frame     # Frame信息
(gdb) info register  # 寄存器信息
(gdb) info args      # 入参信息
(gdb) info locals    # 本地参数信息
(gdb) info variables # 所有的全局变量

所有的Core场景
http://www.voidcn.com/article/p-cjgkidhy-xp.html

(gdb) info threads 运行的线程信息
(gdb) thread apply all bt 所有线程的栈信息


## GDB VS. Variadic

简单来说，`va_list` 是 `char *` 的同义，而 `va_start`、`va_end`、`va_arg` 是宏定义，如下是常见示例：

va_list args; /* 定义 char * 类型的变量 */

 are the macros needed.
The very first step is to create a pointer to point to the first element of the variable argument list. (va_list myListPointer;)
use va_start(myListPointer, numargs) to actually make myListPointer point to the first variable. (You need to at least step past this line in order to start inspecting memory).
The rest involves looping through and printing/calculating the values.

函数调用传递

void log_snprintf(const char *fmt, va_list ap){
	va_list args;

	va_copy(args, ap);
	vsnprintf(buffer, 10, fmt, args);
}

注意，对于 AMD64 采用另外的实现方式，其中 `va_list` 是一个大小为 1 的数组，其中的成员列表如下：

.gp_offset 第一个参数距离reg_save_area的字节数
.fp_offset
.overflow_arg_area
.reg_save_area 第一个参数的地址

那么，如果已知第一参数是 `int` 类型，那么可以通过如下方式打印其对应的值。

(gdb) p *(int *)(((char *)arglist[0].reg_save_area)+arglist[0].gp_offset)

p (((char *)arglist[0].reg_save_area)+arglist[0].gp_offset)
https://moythreads.com/wordpress/2008/05/25/a-tale-of-two-bugs/
https://sourceware.org/ml/gdb/2010-07/msg00075.html
https://www.anintegratedworld.com/how-to-view-va_list-variables-via-gdb/




### Peephole Optimization

这个是针对汇编代码的优化方式，会利用目标 CPU 的指令集特性，所进行的局部优化。

https://blog.csdn.net/liumf2005/article/details/8858102
http://blog.yajun.info/?p=7394
https://www.kancloud.cn/itfanr/i-100-gdb-tips/81888
https://www.cs.swarthmore.edu/~newhall/unixhelp/gdb_pthreads.php
https://sourceware.org/gdb/onlinedocs/gdb/Threads.html
https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_24.html
http://crossbridge.io/docs/gdb_nonstop.html
https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/developer_guide/gdbthreads




https://github.com/rouming/dla
http://cwndmiao.github.io/programming%20tools/2013/11/26/Dwarf/
https://blog.csdn.net/tenfyguo/article/details/6623967
https://blog.csdn.net/luoyuyou/article/details/73498640


(gdb) bt
#0  0x00000000004004dc in foo ()
#1  0x00000000004004f8 in handler ()
#2  <signal handler called>
#3  0x000000000040050d in main ()
(gdb) info frame 1
Stack frame at 0x7f92f50c1fc0:
 rip = 0x44442f; saved rip 0x7f92f5fbb100
 called by frame at 0x7f92f50c2cc0, caller of frame at 0x7f92f50c1fa0
 Arglist at 0x7f92f50c1f98, args:
 Locals at 0x7f92f50c1f98, Previous frame's sp is 0x7f92f50c1fc0
 Saved registers:
  rbx at 0x7f92f50c1fa0, rbp at 0x7f92f50c1fa8, r12 at 0x7f92f50c1fb0, rip at 0x7f92f50c1fb8

也就是说在该栈中，参数 `arglist` 位于 `0x7f92f50c1f98` 处，因为是 64bit 机器，那么向下查看入参 `0x7f92f50c1f90`


在POSIX标准中定义了三种线程同步机制: Mutexes(互斥量), Condition Variables(条件变量)和POSIX Semaphores(信号量)。NPTL基本上实现了POSIX，而glibc又使用NPTL作为自己的线程库。因此glibc中包含了这三种同步机制的实现(当然还包括其他的同步机制，如APUE里提到的读写锁)。


线程的类型可以在 `pthread.h` 中设置，最常见的有如下的几种。

enum lock_type {
	PTHREAD_MUTEX_TIMED_NP,      // 当一个线程加锁后，其余请求锁的线程形成等待队列，在解锁后按优先级获得锁。
	PTHREAD_MUTEX_ADAPTIVE_NP       // 动作最简单的锁类型，解锁后所有线程重新竞争。
	PTHREAD_MUTEX_RECURSIVE_NP      // 允许同一线程对同一锁成功获得多次。当然也要解锁多次。其余线程在解锁时重新竞争。
	PTHREAD_MUTEX_ERRORCHECK_NP     // 若同一线程请求同一锁，返回EDEADLK，否则与PTHREAD_MUTEX_TIMED_NP动作相同。 此处特别注意linux和windows下的errno.h中的EDEADLK对应的宏的值有差别：linux下为35，windows下36
} type;

pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr); // 初始化attr为默认属性
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);

https://blog.csdn.net/jasmineal/article/details/8807635

#include <sys/time.h>
#include <linux/futex.h>

int futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3);

其中比较关键的参数是前三个。

* uaddr 用户态下共享内存的地址，保存了一个对齐的整型计数器。
* op 表示操作类型，总共有 5 种，常用的有 A) FUTEX_WAIT 原子检查uaddr中计数器的值是否为val，是则让进程休眠，直到 FUTEX_WAKE或超时；B) FUTEX_WAKE 最多唤醒val个等待在uaddr上进程。

注意，不要使用 futex 系统调用来实现进程同步，futex 的同步机制和 futex 系统调用是有区别的，futex 的同步机制还包括了用户态下的操作。

pthread_mutex_lock()   nptl/pthread_mutex_lock.c
 |-__pthread_mutex_lock()
   |-LLL_MUTEX_LOCK()    最主要的实现函数，也就是lll_lock()的宏定义
     |-__lll_lock()
       |-atomic_compare_and_exchange_bool_acq()	尝试从0变为1，成功返回0，否则返回>0
	   |-__lll_lock_wait() 返回的是非0，会调用futex并将值设置为2

对于 x86_64 来说，其实现在 `x86_64/lowlevellock.S` 中实现，对于第一个没有获得锁的线程进入 `while` 循环，并将 futex 赋值成为 2，然后等待 lock 被释放后成为 0 。

这第一个waiter被唤醒，atomic_exchange_acq则会赋予futex继续是2，但是返回0跳出获取到lock。

pthread_mutex_unlock()
 |-__pthread_mutex_unlock()
   |-__pthread_mutex_unlock_usercnt()
     |-lll_unlock()   // 将futex值赋为0，并对oldval比较，如果是2，说明有waiter，则futex_wake，1则不需要
	   |-lll_futex_wake()

DWARF 是一种调试信息的保存格式，独立于体系结构和操作系统，使用 gcc 时可以简单的添加上 `-g` 选项即可，此时会增加多个 `.debug.XXX` 的段，这里记录的就是 ELF 文件的调试信息。

各个字段的具体用途可以查看 http://dwarfstd.org/doc/DWARF4.pdf

/proc/[pid]/syscall

当前进程正在执行的系统调用。

$ cat /proc/2406/syscall
202 0xab3730 0x0 0x0 0x0 0x0 0x0 0x7ffff7f6ec68 0x455bb3

其中第一个参数代表了系统调用号，上面的 `202` 表示 `sys_futex`，后面为 6 个系统调用的参数值，最后两个值依次是堆栈指针和指令计数器的值。

如果当前进程虽然阻塞，但阻塞函数并不是系统调用，则系统调用号的值为 `-1`，后面只有堆栈指针和指令计数器的值。

如果进程没有阻塞，则这个文件只有一个 running 的字符串。


实际上最终调用的函数在 `sysdeps/unix/sysv/linux/x86_64/lowlevellock.S` 中实现。

## 死锁

可以通过 `pstack <PID>` 查看对应的栈信息，一般栈的最后为 `__lll_lock_wait()` 函数。


(gdb) info thread    # 查看栈信息
  Id   Target Id         Frame
  3    Thread 0x7fbb21e56700 (LWP 46832) "thread" __lll_lock_wait () at lowlevellock.S:135
  2    Thread 0x7fbb21655700 (LWP 46833) "thread" __lll_lock_wait () at lowlevellock.S:135
* 1    Thread 0x7fbb2264b740 (LWP 46831) "thread" 0x00007fbb2222cf47 in pthread_join() at pthread_join.c:90

(gdb) thread 2       # 切换到怀疑发生死锁的线程
(gdb) bt             # 查看调用栈信息
#0  __lll_lock_wait () at ../nptl/sysdeps/unix/sysv/linux/x86_64/lowlevellock.S:135
#1  0x00007fbb2222ddcb in _L_lock_883 () from /lib64/libpthread.so.0
#2  0x00007fbb2222dc98 in __GI___pthread_mutex_lock (mutex=0x7ffed182d540) at ../nptl/pthread_mutex_lock.c:78
#3  0x00000000004007e2 in thread2 (arg=0x7ffed182d540) at thread.c:32
#4  0x00007fbb2222bdd5 in start_thread (arg=0x7fbb21655700) at pthread_create.c:307
#5  0x00007fbb21f54ead in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:111

(gdb) p *(pthread_mutex_t *)0x7ffed182d540  # 查看等待的线程，本线程为46833而在等待46832所持有的锁
$6 = {__data = {__lock = 2, __count = 0, __owner = 46832, ...}

(gdb) frame 3        # 也可以切换到对应的帧以方便查看两个锁
(gdb) print d->mutex1
(gdb) print d->mutex2

可见 `d->mutex1` 当前被 PID 为 `46832` 线程所持有，而 `d->mutex2` 被 PID 为 `46833` 的线程，也就是当前线程所持有。

ps -o ruser,pid,ppid,lwp,psr,%cpu,%mem,vsz,rss,lstart,etime,comm Hp 46831


https://blog.csdn.net/lixungogogo/article/details/52156547

1. 上下文切换

如果对应的进程没有发生过切换，那么就可能意味着发生了死锁。

$ grep switches /proc/<pid>/status
voluntary_ctxt_switches:	168599
nonvoluntary_ctxt_switches:	21

grep switches /proc/78000/status

2. 确认系统API

其中 `sys_futex` 的系统调用号为 202 ，可以查看进程是否阻塞到该 API 。

# sudo cat /proc/*/task/*/syscall | grep '^202'
202 0x7fffc8b336dc 0x89 0x1 0x7fffc8b33658 ...
                        ^^^ pthread_cond_wait
202 0x7fff60dd3c80 0x80 0x0 0x0 ...
                        ^^^ sem_wait
202 0x601650 0x80 0x2 0x0 0x601650 ...
                  ^^^ pthread_mutex_lock

关于当前系统支持的 API 接口，可以通过 `/usr/include/asm/unistd.h` 头文件查看。


假设已知字符串的地址为 `0x539a2f` ，然后通过 `objdump -h <BIN-FILE>` 获取到 `.rodata` 的入口地址。

 15 .fini         00000016  0000000000535788  0000000000535788  00135788  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 16 .rodata       0003a7d8  00000000005357a0  00000000005357a0  001357a0  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA

获取到字符串表的首地址 0x5357a0 - 0x1357a0 = 0x400000 ，对应字符串的偏移为 `0x539a2f - 0x400000 = 0x139A2F`



可以通过调整 ulimits 允许进程在异常时生成 core 文件，或者是在 gdb 中通过 `gcore <file name>` 生成对应的 core 文件。

注意，通过 `backtrace()` 打印的栈，其显示的地址是将要执行的下个地址。

在 GDB 中可以通过 call 直接调用某个函数。
-->

## 暂停

通过 BreakPoint、WatchPoint、CatchPoint 来设置程序在某段代码处或者满足某个条件时停止。

如下是一个测试程序。

{% highlight c %}
#include <stdio.h>

int main(void)
{
        int i, sum = 0;

        for (i = 1; i <= 200; i++)
                sum += i;         // line #8
        printf("%d\n", sum);
        return 0;
}
{% endhighlight %}

### Breakpoint

直接在某个指定的位置停止。

{% highlight text %}
(gdb) break <FunctionName>                # 当前文件指定函数
(gdb) break <LineNumber>                  # 当前文件指定行数
(gdb) break <FileName:FunctionName>
(gdb) break <FileName:LineNumber>

(gdb) info breakpoints                    # 查看断点信息

(gdb) delete <NUM1 NUM2>                  # 根据上述的序号删除
(gdb) delete <Range>

(gdb) enable/disable

(gdb) clear <...>                         # 指定行删除，类似上述的break设置方式
{% endhighlight %}

另外，也可以通过 `tbreak` 设置临时断点，也就是在执行到一次之后会立即退出。

<!--
匿名空间设置断点
(gdb) b (anonymous namespace)::bar
-->

#### 指定地址

在调试汇编程序时，如果没有调试信息，就需要在程序地址上打断点，方法为 `b *ADDRESS` 。

#### 程序入口

当没有调试信息时，是无法通过 `start` 命令启动并在入口处暂停的，可以通过如下方式获取程序入口。

<!--
https://github.com/hellogcc/100-gdb-tips/blob/master/src/break-on-entry.md
-->

#### 保存断点

可以将断点保存在某个文件中，然后下次直接加载再次使用。

{% highlight text %}
(gdb) save breakpoints <FilenameToSave>
(gdb) source <FilenameToSave>
{% endhighlight %}

注意，如果通过行号指定，那么当源码修改之后可能会是非预期的。

#### 条件断点

可以通过 `break ... if cond` 设置条件断点，也就是当满足某个条件时，断点才会触发。在上述示例的第 8 行，根据变量 `i` 设置一个条件断点。

{% highlight text %}
(gdb) start
Temporary breakpoint 1 at 0x40059e: file main.c, line 5.
Starting program: /tmp/test/a.out
Temporary breakpoint 1, main () at main.c:5
5               int i, sum = 0;
(gdb) break 8 if i == 100
Breakpoint 2 at 0x4005ae: file main.c, line 8.
(gdb) c
Continuing.

Breakpoint 2, main () at main.c:8
8                       sum += i;
(gdb) print sum
$1 = 4950
(gdb) print i
$2 = 100
{% endhighlight %}

#### 忽略 N 次断点

通过命令 `ignore bnum count` 设置，意味着在接下来 `count` 次编号为 `bnum` 的断点触发都不会让程序中断，只有第 `count + 1` 次断点触发才会让程序中断。

{% highlight text %}
(gdb) start
Temporary breakpoint 3 at 0x40059e: file main.c, line 5.
Starting program: /tmp/foobar/a.out

Temporary breakpoint 3, main () at main.c:5
5               int i, sum = 0;
(gdb) break 8
Breakpoint 1 at 0x4005ae: file main.c, line 8.
(gdb) ignore 1 5
Will ignore next 5 crossings of breakpoint 1.
(gdb) c
Continuing.

Breakpoint 5, main () at main.c:8
8                       sum += i;
(gdb) p i
$4 = 6
{% endhighlight %}

如果想让断点下次就生效，可以将 `count` 置为 0 ，也就是 `ignore 1 0` 。

### Watchpoint

当某个变量或者表达式发生变化时暂停，可扩展为变量读、写时停止。

上述示例可以通过 `watch i` 设置，或者直接通过地址，假设地址为 `0x7fffffffdeec` ，那么也可以通过 `watch *(int *)0x7fffffffdeec` 设置观察点，两者作用相同。

当变量修改后，会打印老的以及新的值。

#### 硬件观测点

观测点分为两类：软件观察点 (Software Watchpoint) 和硬件观察点 (Hardware Watchpoint)。其使用软件观察点的方式就是单步执行程序同时测试变量的值，这样会导致程序的执行速度变慢。

现在很多 CPU 提供了硬件观测点，不过因为寄存器有限，所以只能设置有限观测点，可以通过 `disable` 关闭不需要的观测点。

当触发观测点后，才会告知是否启用硬件观测点，也就是 `Hardware watchpoint num` 信息。

{% highlight text %}
(gdb) show can-use-hw-watchpoints
(gdb) set can-use-hw-watchpoints 0
{% endhighlight %}

另外，可以设置读写观测点，不过只针对硬件生效。

* 读观测点 `rwatch`，只对硬件观测点生效。
* 读写观测点 `awatch`，只对硬件观测点生效。

#### 针对线程

可以通过 `watch expr thread threadnum` 设置观察点只针对特定线程生效。

<!--
https://github.com/hellogcc/100-gdb-tips/blob/master/src/set-watchpoint-on-specified-thread.md
-->

### Catchpoint

其作用是在发生某种事件时候停止运行，常用的有如下几类：

* C++ 异常，`throw` 抛出异常，`catch` 捕获异常；
* 程序调用，针对的是不同的系统 API 接口，包括了 `exec` `fork` `vfork`；
* 动态库，加载或者卸载动态库会停止，也可以指定动态库的名称 `load` `unload` ；
* 系统调用，可以指定系统调用号或名称 `syscall` 。

<!--
https://github.com/hellogcc/100-gdb-tips/blob/master/src/index.md
b. tcatch event: 设置只停一次的catchpoint，第一次生效后，该catchpoint被自动删除
-->

## init

在启动时，会在当前用户目录下寻找文件名为 `.gdbinit` 的文件，如果存在，会执行该文件中的所有命令，通常用于简单的配置命令。

<!--
https://blog.csdn.net/gatieme/article/details/63254211
-->

## 参考


{% highlight text %}
{% endhighlight %}
