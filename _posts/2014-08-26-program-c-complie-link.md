---
title: C 编译链接
layout: post
comments: true
language: chinese
category: [program,misc]
keywords: c,编译链接
description: 详细介绍下与 C 语言相关的概念。
---

详细介绍下与 C 语言相关的概念。

<!-- more -->

## 链接过程

在程序由源码到可执行文件的编译过程实际有预处理 (Propressing)、编译 (Compilation)、汇编 (Assembly) 和链接 (Linking) 四步，在 `gcc` 中分别通过 `ccp` `cc1` `as` `ld` 四个命令来完成。

![compile link gcc details]({{ site.url }}/images/linux/compile-link-gcc-details.jpg "compile link gcc details"){: .pull-center }

<!--
关于链接方面可以直接从网上搜索 《linker and loader》。
-->

如下详细介绍编译连接的过程。

### 预编译

将源代码和头文件通过预编译成一个 `.i` 文件，相当与如下命令。

{% highlight text %}
$ gcc -E main.c -o main.i          # C
$ cpp main.c > main.i              # CPP
{% endhighlight %}

与编译主要是处理源码中以 `"#"` 开始的与编译指令，主要的处理规则是：

* 删除所有的 `"#define"` ，并且展开所有的宏定义。
* 处理所有条件预编译指令，比如 `"#if"`、`"#ifdef"`、`"#elif"`、`"#else"`、`"#endif"` 。
* 处理 `"#include"` ，将被包含的文件插入到该预编译指令的位置，该过程是递归的。
* 删除多有的注释 `"//"` 和 `"/* */"` 。
* 添加行号和文件名标识，如 `#2 "main.c" 2` ，用于编译时产生调试用的行号以及在编译时产生错误或警告时显示行号。
* 保留所有的 `"#pragma"` 编译器指令，因为编译器需要使用它们。

经过预编译后的 `.i` 文件不包含任何宏定义，因为所有的宏已经被展开，并且包含的文件也已经被插入到 `.i` 文件中。所以，当无法判断宏定义是否正确或头文件包含是否正确时，可以查看该文件。

### 编译

编译过程就是把预处理后的文件进行一系列的词法分析、语法分析、语义分析以及优化后生成相应的汇编代码文件，这个是核心部分，也是最复杂的部分。

gcc 把预编译和编译合并成一个步骤，对于 C 语言使用的是 `cc1` ，C++ 使用的是 `cc1obj` 。

{% highlight text %}
$ gcc -S hello.i -o hello.s
$ gcc -S main.c -o main.s
{% endhighlight %}

<!-- $ /usr/lib/gcc/i386-linux-gnu/4.7/cc1 main.c -->

### 汇编

汇编器是将汇编代码转化成机器码，每条汇编语句几乎都对应一条机器指令。汇编器不需要复杂的语法语义，也不用进行指令优化，只是根据汇编指令和机器指令的对照表一一翻译即可。

{% highlight text %}
$ gcc -c hello.s -o hello.o
$ as main.s -o main.o
$ gcc -c main.s -o main.o
$ gcc -c main.c -o main.o
{% endhighlight %}

### 链接

可以通过 `gcc hello.c -o hello -v` 查看。

{% highlight text %}
$ gcc hello.o -o hello.exe
{% endhighlight %}

## 静态链接库

库有动态与静态两种，Linux 中动态通常用 `.so` 为后缀，静态用 `.a` 为后缀，如：`libhello.so` `libhello.a`，静态链接库实际上就是将各个 `.o` 文件打包合并。

使用静态链接库时，连接器会找出程序所需的函数，然后将它们拷贝到执行文件，一旦连接成功，静态程序库也就不再需要了，缺点是占用的空间比较大，但是执行速度要快一些。

### 示例

如果编译时使用静态库，那么所有依赖的基础库都需要安装静态版本，否则链接会失败，例如对于 `libc` 基础库，在 CentOS 中可以通过 `yum install glibc-static` 命令安装。

现在假设有一个 hello 程序开发包，它提供一个静态库 `libhello.a`，一个动态库 `libhello.so`，一个头文件 `hello.h`，头文件中提供 `foobar()` 这个函数的声明。

下面这段程序 `main.c` 使用 hello 库中的 `foobar()` 函数。

{% highlight c %}
/* filename: foobar.c */
#include "hello.h"

void foobar(void)
{
	puts("FooBar!");
}
{% endhighlight %}

{% highlight c %}
/* filename: hello.c */
#include "hello.h"

void hello(void)
{
	puts("Hello world!");
}
{% endhighlight %}

{% highlight c %}
/* filename: hello.h */
#ifndef _HELLO_H__
#define _HELLO_H__
#include <stdio.h>

void hello();
void foobar();
#endif
{% endhighlight %}

{% highlight c %}
/* filename: main.c */
#include "hello.h"

int main(void)
{
	foobar();
	hello();
	return 0;
}
{% endhighlight %}

当生成静态库时，需要先对源文件进行编译，然后使用 `ar(archive)` 命令连接成静态库。

{% highlight text %}
$ gcc -c hello.c -o hello.o
$ gcc -c foobar.c -o foobar.o
$ ar crv libhello.a hello.o foobar.o
$ ar -t libhello.a                              // 查看打包的文件
{% endhighlight %}

`ar` 实际是一个打包工具，可以用来打包常见文件，不过现在被 `tar` 替代，目前主要是用于生成静态库，详细格式可以参考 [ar(Unix) wiki](http://en.wikipedia.org/wiki/Ar_(Unix)) 。

{% highlight text %}
$ echo "hello" > a.txt && echo "world" > b.txt
$ ar crv text.a a.txt b.txt
$ cat text.a
{% endhighlight %}

在与静态库连接时，需要指定库的路径，默认不会将当前目录添加到搜索目录中。

{% highlight text %}
$ gcc main.c -o test -lhello                    // 库在默认路径下，如/usr/lib
$ gcc main.c -lhello -L. -static -o main        // 通过-L指定库的路径

$ gcc main.o -o main -WI,-Bstatic -lhello       // 报错，显示找不到-lgcc_s
{% endhighlight %}




注意：这个特别的 `"-WI,-Bstatic"` 参数，实际上是传给了连接器 `ld`，指示它与静态库连接，如果系统中只有静态库可以不需要这个参数； 如果要和多个库相连接，而每个库的连接方式不一样，比如上面的程序既要和 `libhello` 进行静态连接，又要和 `libbye` 进行动态连接，其命令应为：

{% highlight text %}
$ gcc testlib.o -o test -WI,-Bstatic -lhello -WI,-Bdynamic -lbye
{% endhighlight %}

最好不要进行分别编译、链接，因为在生成可执行文件时往往需要很多的其他文件，可以通过 `-v` 选项进行查看，如果通过如下方式进行编译通常会出现错误。

{% highlight text %}
$ gcc -c main.c
$ ld main.o -L. -lhello
{% endhighlight %}

### 链接顺序

当一个项目中有多个静态库时，就可能会由于链接顺序不同导致 `undefined reference` 的报错，而实际上符号是有定义的，其根本原因是由于符号的查找算法引起的。

链接器查找符号针对的是单个目标文件 `.o` ，而非整个静态库，如果在某个目标文件中找到了所需的符号，那么就会将整个目标文件单独从静态库中提取出来，而非将整个链接库添加。

链接器在工作过程中，维护 3 个列表：A) 需要参与连接的目标文件列表 E；B) 一个未解析符号列表 U；C) 一个在 E 中所有目标文件定义过的所有符号列表 D 。

<!--
静态库的链接问题
https://segmentfault.com/a/1190000006911973
-->

## 动态链接库

动态链接就是在程序运行时对符号进行重定位，确定符号对应的内存地址的过程，默认采用的是 Lazy Mode ，只解析那些用得到的符号，如果不需要就不会查找。

### 依赖库

ELF 文件有一个特别的 Section `.dynamic`，存放了和动态链接相关的很多信息，例如动态链接器通过它找到该文件使用的动态链接库。

通过 `readelf -d | grep NEEDED` 可以找到该文件直接依赖的库，如果要查看所有依赖的库，那么可以通过 `ldd` 命令查看。

{% highlight text %}
$ readelf -d /bin/bash | grep NEED
 0x0000000000000001 (NEEDED)             Shared library: [libtinfo.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libdl.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

$ ldd /bin/bash
        linux-vdso.so.1 (0x00007ffd2438a000)
        libtinfo.so.6 => /lib64/libtinfo.so.6 (0x00007fe7d3745000)
        libdl.so.2 => /lib64/libdl.so.2 (0x00007fe7d3541000)
        libc.so.6 => /lib64/libc.so.6 (0x00007fe7d317e000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fe7d3c90000)
{% endhighlight %}

其中 `linux-vdso.so.1` 是一个虚拟的动态链接库，对应进程内存映像的内核部分，而 `/lib/ld-linux-x86_64.so.2` 正好是动态链接器，系统需要用它来进行符号重定位。

而链接器实际上是在 `.interp` 中指定的，使用的是绝对路径，会先被装载到内存中，然后由该文件再加载其它动态库。

{% highlight text %}
$ readelf -p .interp /bin/bash

String dump of section '.interp':
  [     0]  /lib64/ld-linux-x86-64.so.2
{% endhighlight %}

<!--
通过 LD_LIBRARY_PATH 参数，它类似 Shell 解释器中用于查找可执行文件的 PATH 环境变量，也是通过冒号分开指定了各个存放库函数的路径。该变量实际上也可以通过 /etc/ld.so.conf 文件来指定，一行对应一个路径名。为了提高查找和加载动态链接库的效率，系统启动后会通过 ldconfig 工具创建一个库的缓存 /etc/ld.so.cache 。如果用户通过 /etc/ld.so.conf 加入了新的库搜索路径或者是把新库加到某个原有的库目录下，最好是执行一下 ldconfig 以便刷新缓存。
-->

### 路径问题

如果动态库不在搜索路径中，则会报 `cannot open shared object file: No such file or directory` 的错误。可以通过 `gcc --print-search-dirs` 命令查看默认的搜索路径。

查找顺序通常为：

1. 查找程序编译指定的路径，保存在 `.dynstr` 段，其中包含了一个以冒号分割的目录搜索列表。
2. 查找环境变量 `LD_LIBRARY_PATH`，以冒号分割的目录搜索列表。
3. 查找 `/etc/ld.so.conf` 。
4. 默认路径 `/lib` 和 `/usr/lib` 。

为了让执行程序顺利找到动态库，有三种方法：

##### 1. 复制到指定路径

把库拷贝到查找路径下，通常为 `/usr/lib` 和 `/lib` 目录下，或者通过 `gcc --print-search-dirs` 查看动态库的搜索路径。

##### 2. 添加链接选项

编译时添加链接选项，指定链接库的目录，此时会将该路径保存在二进制文件中。

{% highlight text %}
$ gcc -o test test.o -L. -lhello -Wl,-rpath,/home/lib:.
$ readelf -d test | grep RPATH
$ objdump -s -j .dynstr test                     // 查看.dynstr段的内容
{% endhighlight %}

##### 3. 设置环境变量

执行时在 `LD_LIBRARY_PATH` 环境变量中加上库所在路径，例如动态库 `libhello.so` 在 `/home/test/lib` 目录下。

{% highlight text %}
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/test/lib
{% endhighlight %}

##### 4. 修改配置文件

修改 `/etc/ld.so.conf` 文件，把库所在的路径加到文件中，并执行 `ldconfig` 刷新配置。动态链接库通常保存在 `/etc/ld.so.cache` 文件中，执行 `ldconfig` 可以对其进行刷新。

### 版本管理

对于共享库更新时通常会有兼容更新和不兼容更新，其实指的是二进制接口 (Application Binary Interface, ABI)，为了保证共享库的兼容性， Linux 采用一套规则来命名系统中的共享库。

实际上就是语意版本，其规则如下 `libname.so.x.y.z`，`name` 库名称，`x` `y` `z` 含义如下：

* `x` 主版本号(Major Version Number)，重大升级，不同主版本不兼容。
* `y` 次版本号(Minor Version Number)，增量升级，增加了新接口，且保持原符号不变。
* `z` 发布版本号(Release Version Number)，错误修正、性能改进等，不添加、不修改接口。

由于历史的原因最基本的 C 语言库 glibc 动态链接库不使用这种规则，如 `libc-x.y.z.so` 、`ld-x.y.z.so` ，在  [Library Interface Versioning in Solaris and Linux](https://www.usenix.org/legacy/publications/library/proceedings/als00/2000papers/papers/full_papers/browndavid/browndavid_html/) 中，对 Salaris 和 Linux 的共享库版本机制和符号版本机制做了非常详细的介绍。

在 Linux 中采用 SO-NAME 的命名机制，每个库会对应一个 SO-NAME ，这个 SO-NAME 只保留主版本号，也即 SO-NAME 规定了共享库的接口。为了在同一系统中使用不同版本的库，可以在库文件名后加上版本号为后缀，例如：`libhello.so.1.0`，然后，使用时通过符号链接指向不同版本。

{% highlight text %}
# ln -s libhello.so.1.0 libhello.so.1
# ln -s libhello.so.1 libhello.so
{% endhighlight %}

### 示例程序

通过 `-fPIC` 参数生成与位置无关的代码，这样允许在任何地址被连接和装载。

{% highlight text %}
$ gcc -c -fPIC hello.c -o hello.o

$ gcc -shared -Wl,-soname,libhello.so.1 -o libhello.so.1.0 hello.o // 生成动态库，可能存在多个版本，通常指定版本号

$ ln -s libhello.so.1.0 libhello.so.1                           // 另外再建立两个符号连接
$ ln -s libhello.so.1 libhello.so

$ gcc -fPIC -shared -o libhello.so hello.c                         // 当然对于上述的步骤可以通过一步完成

$ readelf -d libhello.so.1.0 | grep SONAME                         // 查看对应的soname
$ nm -D libhello.so                                                // 查看符号
{% endhighlight %}

最重要的是传 `-shared` 参数使其生成是动态库而不是普通执行程序； `-Wl` 表示后面的参数也就是 `-soname,libhello.so.1` 直接传给连接器 `ld` 进行处理。

实际上，每一个库都有一个 `soname` ，当连接器发现它正在查找的程序库中有这样一个名称，连接器便会将 `soname` 嵌入连结中的二进制文件内，而不是它正在运行的实际文件名，在程序执行期间，程序会查找拥有 `soname` 名字的文件，而不是库的文件名，换句话说，`soname` 是库的区分标志。

其目的主要是允许系统中多个版本的库文件共存，习惯上在命名库文件的时候通常与 `soname` 相同 `libxxxx.so.major.minor` 其中，`xxxx` 是库的名字， `major` 是主版本号， `minor` 是次版本号。

## 动态库加载API

对于 Linux 下的可执行文件 ELF 使用如下命令查看，可以发现其中有一个 `.interp` 段，它指明了将要被使用的动态链接器 (`/lib/ld-linux.so`)。

{% highlight text %}
$ readelf -l EXECUTABLE
{% endhighlight %}

动态加载函数主要包括了下面的四个，依赖 `dlfcn.h` 头文件，定义在 `libdl.so` 库中。

{% highlight text %}
void *dlopen( const char *file, int mode );
  用来打开一个文件，使对象文件可被程序访问，同时还会自动解析共享库中的依赖项，这样，如果打开了一个
    依赖于其他共享库的对象，它就会自动加载它们，该函数返回一个句柄，该句柄用于后续的 API 调用。
  mode 参数通知动态链接器何时执行再定位，有两个可能的值：
    A) RTLD_NOW，表明动态链接器将会在调用 dlopen 时完成所有必要的再定位；
    B) RTLD_LAZY，只在需要时执行再定位。

void *dlsym( void *restrict handle, const char *restrict name );
  通过句柄和连接符名称获取函数名或者变量名。

char *dlerror();
  返回一个可读的错误字符串，该函数没有参数，它会在发生前面的错误时返回一个字符串，在没有错误发生时返回NULL。

char *dlclose( void *handle ); 
  通知操作系统不再需要句柄和对象引用了。它完全是按引用来计数的，所以同一个共享对象的多个用户相互间
    不会发生冲突（只要还有一个用户在使用它，它就会待在内存中）。
    任何通过已关闭的对象的 dlsym 解析的符号都将不再可用。
{% endhighlight %}

有了 ELF 对象的句柄，就可以通过调用 dlsym 来识别这个对象内的符号的地址了。该函数采用一个符号名称，如对象内的一个函数的名称，返回值为对象符号的解析地址。

下面是一个动态加载的示例 [github libdl.c]({{ site.example_repository }}/c_cpp/c/libdl.c)，通过如下的命令进行编译，其中选项 `-rdynamic` 用来通知链接器将所有符号添加到动态符号表中（目的是能够通过使用 dlopen 来实现向后跟踪）。

{% highlight text %}
$ gcc -rdynamic -o dl library_libdl.c -ldl        # 编译
$ ./dl                                            # 测试
> libm.so cosf 0.0
   1.000000
> libm.so sinf 0.0
   0.000000
> libm.so tanf 1.0
   1.557408
> bye
{% endhighlight %}

另外，可以通过如下方式简单使用。

{% highlight text %}
$ cat caculate.c                                     # 查看动态库源码
int add(int a, int b) {
    return (a + b);
}
int sub(int a, int b) {
    return (a - b);
}
$ gcc -fPIC -shared caculate.c -o libcaculate.so     # 生成动态库
$ cat foobar.c                                       # 测试源码
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

typedef int (*CAC_FUNC)(int, int);                           // 定义函数指针类型
int main(int argc, char** argv) {
    void *handle;
    char *error;
    CAC_FUNC cac_func = NULL;

    if ( !(handle=dlopen("./libcaculate.so", RTLD_LAZY)) ) { // 打开动态链接库
        fprintf(stderr, "!!! %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    cac_func = dlsym(handle, "add");                         // 获取一个函数
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "!!! %s\n", error);
        exit(EXIT_FAILURE);
    }
    printf("add: %d\n", (cac_func)(2,7));

    dlclose(handle);                                         // 关闭动态链接库
    exit(EXIT_SUCCESS);
}
$ gcc -rdynamic -o foobar foobar.c -ldl              # 编译测试
{% endhighlight %}

<!-- https://www.ibm.com/developerworks/cn/linux/l-elf/part1/index.html -->

## 其它

### not a dynamic executable

一般是由于不同的平台导致，例如在 x86 上查看 arm，或者在 64 位机器上查看 32 位。

`ldd` 命令会通过默认的解析器进行解析，一般来说为 `ld-linux-aarch64.so.1` 或者 `ld-linux-x86-64.so.2` 。

{% highlight text %}
$ readelf -l <EXEC_FILE> | grep 'program interpreter'
{% endhighlight %}

如果存在 `locate` 命令，可以直接通过 `locate ld-linux` 查找当前机器所有类似文件。注意，可以通过 `updatedb` 更新。

而这个二进制文件所支持的平台可以通过 `file <EXEC_FILE>` 或者 `readelf -h <EXEC_FILE>` 命令查看。

### 库版本

如果在高版本机器上编译二进制文件，然后复制到低版本 (主要是动态库) 上执行，那么就可能会出现类似 `version 'GLIBC_2.12' not found` 的报错，完整报错信息如下。

{% highlight text %}
<EXEC_NAME>: /lib64/libpthread.so.0: version `GLIBC_2.12' not found (required by <EXEC_NAME>)
{% endhighlight %}

这里就是因为依赖的 glibc 版本太低导致，通过 `strings /lib64/libc.so.6 | grep GLIBC` 查看当前库支持版本号，最高版本可以通过 `ldd --verion` 或者 `/lib64/libc.so.6` 查看。

最简单的，就是在编译的时候只依赖低版本的 glibc ，这样在高版本上也可以使用。

#### 原因

从 `glibc 2.1` 开始，引入了 [Symbol Versioning](https://akkadia.org/drepper/symbol-versioning) 的机制，每个符号都会对应一个版本号，例如：

{% highlight text %}
$ nm /lib64/libc.so.6 | grep " memcpy"
{% endhighlight %}

当前二进制文件所有依赖的版本号，可以通过如下命令查看。

{% highlight text %}
$ nm <exec file> | awk -F '@' '/@@GLIBC/{ print $3}' | sort -t. -k 2 -nur
{% endhighlight %}

当在一台高版本 glibc 上编译包是无法在一个低版本 glibc 的机器上运行的，通常有几种办法：A) 升级 glibc 库；B) 重新在低版本 glibc 上编译；C) 修改二进制文件；

<!--
更改引用高版本glibc的程序到引用低版本的glibc
https://blog.csdn.net/Mr_HHH/article/details/83104485
glibc和Symbol Versioning和如何链接出低版本glibc可运行的程序
https://blog.blahgeek.com/glibc-and-symbol-versioning/
version `GLIBC_2.14' not found 解决方法.
https://blog.csdn.net/force_eagle/article/details/8684669
-->

### 杂项

#### 1. 静态库生成动态库

可以通过多个静态库生成动态库，而实际上静态库是一堆 `.o` 库的压缩集合，而生成动态库需要保证 `.o` 编译后是与地址无关的，也就是添加 `-fPIC` 参数。

#### 2. 关于 PreLoad

正常来说 `ld-linux(8)` 会查找一个程序需要加载的库，然后解析执行，通过 `LD_PRELOAD` 或者 `/etc/ld.so.preload` 可以提前加载一些动态库。

<!--
https://sourceware.org/binutils/docs/ld/VERSION.html

Yubikey一个物理的USBKey，也可以参考Google开源的OpenSK，以及SOLO
https://github.com/google/OpenSK
https://github.com/solokeys/solo
个人安全信息模型
https://blog.blahgeek.com/personal-security-model/

## Symbol Versioning
https://blog.blahgeek.com/glibc-and-symbol-versioning/


## GDB

基于 ptrace 实现

(gdb) info variables   查看全局和静态变量
(gdb) info locals      当前栈的局部变量
(gdb) info args        当前栈的参数

bt full  显示各个函数的局部变量

strip -d
strip -s 同时删除.symtab符号表 .strtab字符串表

软件调试的艺术 Linker and Loader
https://github.com/Jessicahust/books/blob/master/%E8%BD%AF%E4%BB%B6%E8%B0%83%E8%AF%95%E7%9A%84%E8%89%BA%E6%9C%AF.pdf
https://github.com/yuanyiyixi/book/blob/master/C-book/linkers%20and%20loaders-%E4%B8%AD%E6%96%87%E7%89%88.pdf
一个不依赖glibc的程序
https://www.cnblogs.com/softfair/p/hello-from-a-glibc-free-world.html
-->

{% highlight text %}
{% endhighlight %}
