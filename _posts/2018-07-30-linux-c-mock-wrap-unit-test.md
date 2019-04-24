---
title: Linux C Mock Wrap
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: cmocka,mock,wrap
description: 编写高效松耦合的模块，体现的是功力，而完善的测试用例则是习惯。包括了一些异常场景的积累，代码重构时的验证等等，编写有效的测试用例就尤为重要。而 C 语言，由于其偏向于底层，导致不能像 Java、Python、GO 那样提供了成熟的测试框架。这里简单介绍一下一个基于 cmocka 修改的测试框架，会通过一些宏定义处理部分问题，当然，真正使用时还需要一些其它的技巧。
---

编写高效松耦合的模块，体现的是功力，而完善的测试用例则是习惯。包括了一些异常场景的积累，代码重构时的验证等等，编写有效的测试用例就尤为重要。

而 C 语言，由于其偏向于底层，导致不能像 Java、Python、GO 那样提供了成熟的测试框架。

这里简单介绍一下一个基于 cmocka 修改的测试框架，会通过一些宏定义处理部分问题，当然，真正使用时还需要一些其它的技巧。

<!-- more -->

## cmocka

可以查看官网 [cmocka.org](https://cmocka.org/) ，基于的是 Google 的 cmockery ，如下简单介绍。

### 示例

可以从 [cmocka.org/files](https://cmocka.org/files/) 上下载。

{% highlight text %}
----- 解压
$ tar -xvf cmocka-1.1.3.tar.xz
----- 编译安装
$ cd cmocka-1.1.3 && mkdir build && cd build
{% endhighlight %}

### 整体框架


{% highlight text %}
/* A test case that does nothing and succeeds. */
static void null_test_success(void **state)
{
	(void) state; /* unused */
}
int main(void)
{
    const struct unit_test tests[] = {
        mockx_unit_test(null_test_success, /* setup */ NULL, /* teardown */ NULL),
    };
    return cmocka_run_group_tests(tests, /* group setup */ NULL, /* group teardown */ NULL);
}
{% endhighlight %}

在 `main()` 中定义了一个 `struct unit_test` 类型的数组，包含了多个测试用例，然后通过 `cmocka_run_group_tests()` 函数启动测试用例。无论是多个测试用例，还是单个，都可以通过 `setup()` 或者 `teardown()` 来初始化或者清理某些资源。

## Mock

所谓的 mock 测试，简单来说就是仿造一个函数，一般来说这个函数可能是系统调用、某个库的函数，有可能需要建立连接等等。

对于一个函数来说，如果不考虑其本身带来的副作用 (实际上可以在测试时通过宏进行屏蔽)，一般只需要关注其输入和输出即可，那么对于 mock 而言，也就是如何校验入参，并构造输出内容。

### 使用方法

在使用时基本上可以分为两步：A) 根据测试场景，明确期望的输入以及输出参数；B) 在 mock 后的函数中检查相关的参数，并构造输出。

#### 测试用例

每个测试用例可以作为一个单独的调用过程，可以通过 `expect_XXX()` 确定某个函数对应的入参值；并使用 `will_return()` 确定 mock 后函数的返回值信息。

然后在 mock 后的函数中进行模拟，例如通过 `check_expected()` 检查在测试用例中通过 `expect_XXX()` 函数设置的入参检查值；通过 `mock()` 返回之前通过 `will_return()` 设置的返回值。

在使用 `mock()` 函数时，因为该函数可以保存指针、数值、浮点数等信息，可以按照参数的返回信息进行强制转换，例如 `return (int) mock();` 。

### 实现细节

`expect_XXX()` 类型的函数会向 `global_function_parameter_map_head` 链表中添加节点，一般是 `funcation` + `parameter` 两个参数，其中保存的对象为 `struct check_para_event` 。

`will_return()` 函数会向 `global_function_result_map_head` 链表中添加节点，包含的只是 `funcation` 一个参数，其中保存的对象为 `struct symbol_value` 。

在上述两种方式在添加到列表时，会新建一个 `struct symbol_map_value` 对象来保存值，主要是为了实现层级嵌套关系。


## API

返回值检查。

{% highlight text %}
will_return(func, value)                # 只返回一次mock值，等价于will_return_count(func, value, 1)
will_return_count(func, value, count)   # 可以返回count次的mock值，大于count次将会报错，如果有剩余没有返回同样报错
will_return_always(func, value)         # 总是返回数据，等价于will_return_count(func, value, -1)
{% endhighlight %}

参数检查。

{% highlight text %}
expect_check()                          # 是其它expect_XXX类型函数的最终调用接口，区别在与检查是否相等的函数不同
expect_string(func, para, str)          # 设置某个函数字符串参数的预期值
expect_value(func, para, value)         # 数值类型检查
expect_in_set(func, para, array)        # 检查值是否在数组中，注意数组应该是maxint_t []
expect_in_range(func, para, min, max)   # 检查值是否在 [min, max] 范围内
{% endhighlight %}

在 mock 测试时，隐含了一个引用次数的计数，可以通过 `expect_XXX_count()` 函数设置引用的次数，表示被几次函数调用。每次，如果有些参数没有被检查到，那么实际上也意味着错误。

## 模拟接口

对于一些函数的单元测试很简单，实际上就是构建测试用例，然后判断函数返回结果即可。但是有的时候 A 函数调用 B ，然后调用 C ... ...，我们希望，在完成 C 的单元测试之后，mock 函数 C 的功能，然后在针对 B 编写单元测试。

这也就意味着需要针对函数提供一些相关的 mock 调用，主要包括了：A) 系统函数调用，模拟系统调用失败；B) 已单元测试过的函数，用于解耦测试用例。

### 系统调用

通常的系统调用包括了 `open()` `write()` `read()` `close()` 等等，在模拟时可以检查这些参数，然后返回成功或者是失败。

当 mock 系统调用的时候，实际上有几种方式可供参考。

#### GCC Linker

也就是使用的 GCC 的连接器选项，不过一般会直接使用 `gcc` 命令而非 `ld` ，此时可以通过 `-Wl,--wrap=...` 将参数通过 `gcc` 传递给 `ld` 。

例如，使用 `-Wl,--wrap=open` 参数进行编译，此时，GCC 实际上会将 `open()` 替换为 `__wrap_open()` 函数，也就是说连接的是该函数。如果用户希望调用真正的函数，那么就可以使用 `__real_open()` 函数。

使用示例如下，如果要调用真正的 `open()` 函数，可以直接使用 `__real_open()` 函数。

{% highlight c %}
#include <stdio.h>
#include <fcntl.h>

int __real_open(const char *path, int flags);
int __wrap_open(const char *path, int flags)
{
        fprintf(stdout, "===> Open path '%s' flags '0x%x'\n", path, flags);
        return -15;
}

int main(void)
{
        int rc;

        rc = open("/tmp/1", O_CLOEXEC);
        if (rc < 0) {
                fprintf(stderr, "Open file failed, rc %d.\n", rc);
                return -1;
        }

        return 0;
}
{% endhighlight %}

### 函数替换

对于在同一个文件中的函数，GCC 在编译时不会将其标记为 `unresolved` ，那么上述的 wrap 机制也就无效。

这一问题，GCC 提供了一个简单的 `weak symbols` 机制，默认是 `strong symbols` ，对于后者如果再定义一个相同名称的函数，那么就会报 `multiple defination` 的错误。

设置 `weak symbols` 有两种方式：A) 使用 GCC 编译时传递相关的参数；B) 在函数实现时增加一个 `__attribute__((weak))` 的注释。前者适用于不方便修改代码的场景，例如在使用一些三方的库。

注意，如果在使用 B 方式的时候，其他用户实际上可以直接替换掉我们实现的函数，实际上这会引入一些问题，所以建议使用宏进行定义。

#### 示例

glibc 中大部分的接口都是使用的 `weak symbols` ，仍然以 `open()` 函数为例。

在 glibc 中的实现在 `sysdeps/unix/sysv/linux/open.c` 文件中，代码如下：

{% highlight c %}
int
__libc_open (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__OPEN_NEEDS_MODE (oflag))
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  return SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag, mode);
}
libc_hidden_def (__libc_open)

weak_alias (__libc_open, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open, open)
{% endhighlight %}

也就是说，可以在用户的代码中直接定义一个相同的函数 (函数名+参数相同) ，编译器在连接时会自动将弱引用覆盖。

{% highlight c %}
#include <stdio.h>
#include <fcntl.h>

int open(const char *path, int flags, ...)
{
        fprintf(stdout, "===> Open path '%s' flags '0x%x'\n", path, flags);
        return -15;
}

int main(void)
{
        int rc;

        rc = open("/tmp/1", O_CLOEXEC);
        if (rc < 0) {
                fprintf(stderr, "Open file failed, rc %d.\n", rc);
                return -1;
        }
        return 0;
}
{% endhighlight %}

也就是说，glibc 中实现的 `open()` 函数实际上是支持多个参数的。

注意，如果这里需要使用 glibc 的动态库中的函数，那么就需要通过 `dlsym(RTLD_NEXT, "open")` 动态查找到相关的实现，然后再调用。

<!--
{% highlight text %}
int (*glibc_open)(const char *path, int flags, ...);
glibc_open = (int (*)(const char *, int, ...))dlsym(RTLD_NEXT, "open");
{% endhighlight %}

## FAQ

#### 1. 如何 mock 一个 static 函数

一般来说，如果发现你的代码需要 mock 一个静态函数，通常意味着代码耦合度太高、不容易复用、不容易测试，最好将函数单独拆出来作为一个接口，然后测试这个接口即可。

正常来说不会出现 `ERROR` 错误，此时一般意味着使用方式有问题导致异常。

_cmocka_run_group_tests()
 |-check_point_allocated_blocks()
 |-cmocka_run_one_tests()
   |-cmocka_run_one_test_or_fixture()
   |-fail_if_blocks_allocated()

check_expected()
在mock中检查是否是合法的，其中校验的参数在测试函数中初始化完成。
报错示例如下
tload.c:21 Check of <func/para '__wrap_open'/'path'> failed, @tload.c:35
其中前者源码+行号表示在哪行出的错误，而检查的函数以及参数在行尾处设置，函数名和参数如上。

注意，因为涉及到通过 `setjmp()` 和 `longjmp()` 进行跳转，一些错误信息无法通过函数参数进行传递，所以实现时采用的是一个全局的变量。

这里参考的是cmocka的代码，包括了一些详细的测试
http://blog.lucode.net/backend-development/c-unittest-framework-cmocka.html


## 简介

当前的特性有：

* 无三方库依赖；
* 提供基本的抽象，支持 setup() 和 teardown() 函数，集成 mock ；   ???Test Fixture的使用场景是???
* 基本的内存检测，包括内存泄露、缓存的上下溢检测。

提供对于信号的处理
支持多种格式输出(编译时指定)


SQLite数据库完整性检查以及数据库备份
https://www.cnblogs.com/huahuahu/p/jian-chasqlite-shu-ju-ku-wan-zheng-xing.html
https://www.sqlite.org/backup.html

http://www.dapenti.com/blog/more.asp?name=xilei&id=139296
http://www.dapenti.com/blog/more.asp?name=xilei&id=139450
http://www.dapenti.com/blog/more.asp?name=xilei&id=139752
http://www.dapenti.com/blog/more.asp?name=xilei&id=139844

GCC 中的建议性参数定义
__attribute__ ((deprecated))
__attribute__ ((__format__ (__printf__, a, b)))


单元测试 (Unit Test) 一般测试的是单个函数的功能，如果在该函数中还会调用其它函数，那么被调用的函数应该是要 mock 的。否则，就会出现为了测试 A ，而实际 A 又会调用 B ，然后 B 又调用 C ，... ... 这样实际上就是功能测试了 (Functionnal Tests)。

最近发现两个很牛掰的库用来作单元测试以及Server+Client测试，正式我所需的内容
-->

## 参考

<!--
测试场景：
1. 如果有剩余的参数没有校验，相关的报错信息。

https://cwrap.org/
https://cmocka.org/
https://github.com/clibs/cmocka
CMocka源自于Google的cmockery
https://github.com/google/cmockery
https://lwn.net/Articles/558106/
https://stackoverflow.com/questions/44073243/how-to-mock-socket-in-c
https://github.com/martinpitt/umockdev
-->

介绍如何写 C 的单元测试 [Unit testing C code with CMocka](https://blog.microjoe.org/2017/unit-tests-c-cmocka-coverage-cmake.html) 。

{% highlight text %}
{% endhighlight %}
