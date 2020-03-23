---
title: C 加载过程
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,c,加载过程
description: 利用动态库，可以节省磁盘、内存空间，而且可以提高程序运行效率；不过同时也导致调试比较困难，而且可能存在潜在的安全威胁。这里主要讨论符号的动态链接过程，即程序在执行过程中，对其中包含的一些未确定地址的符号进行重定位的过程。
---


这里主要讨论符号的动态链接过程，即程序在执行过程中，对其中包含的一些未确定地址的符号进行重定位的过程。

<!-- more -->

## Shell 执行

在 Linux 中，可以在 Bash 中直接调用一个执行命令，甚至是一个脚本，脚本中甚至不需要指定解析器，这里简单介绍下 Bash 的执行过程。

目前一般使用的是虚拟终端，可以在当前终端中通过 `tty` 命令查看，假设为 `/dev/pts/27`，然后再通过 `ps -ef | grep pts/27 | grep bash | grep -v grep` 查看对应的 PID 。

打开另一个终端，通过 `pstack PID` 即可看到对应的调用堆栈。

{% highlight text %}
main()                               ← 各种初始化工作，shell.c
 |-reader_loop()                     ← 死循环，通过read_command()读取命令，eval.c
   |-read_command()                  ← 通过parse_command()和yyparse()解析输入的命令，eval.c
   |-execute_command()               ← 开始执行命令，execute_cmd.c
     |-execute_command_internal()
       |-execute_simple_command()
         |-execute_disk_command()
           |-execve()                ← 通过系统调用执行
{% endhighlight %}

其中词法语法解析通过 `flex-biso` 解析，涉及的文件为 `parse.y`，没有找到词法解析的文件。

{% highlight text %}
$ rpm -qf `which bash`                      // 查看所属包
$ yumdownloader --source bash               // 下载源码
$ rpm2cpio bash-version.src.rpm | cpio -id  // 解压源码
{% endhighlight %}

通过 `strace ./hello` 查看系统调用，定位到 `execve()` ，也就是通过该函数执行。

## 内核加载

上述的 `execve()` 实际上就是内核提供的一个系统调用，会将 ELF 文件加载到内存中，如果涉及到动态链接，那么还会在用户态中解析相关函数的地址，这也就是解析器的功能。

一般解析器是 `ld-linux.so.2`，解释器的装入/启动也由内核负责，详细可以查看 [内存-用户空间](/post/kernel-memory-management-from-userspace-view.html) 中的介绍，在此只介绍 ELF 的加载过程。

### 内核模块

如果要支持不同的执行格式，需要在内核中添加注册模块，每种类型通过 `struct linux_binfmt` 格式表示，其定义以及 ELF 的定义如下所示：

{% highlight c %}
struct linux_binfmt {
    struct list_head lh;
    struct module *module;
    int (*load_binary)(struct linux_binprm *);
    int (*load_shlib)(struct file *);
    int (*core_dump)(struct coredump_params *cprm);
    unsigned long min_coredump; /* minimal dump size */
};

static struct linux_binfmt elf_format = {
    .module        = THIS_MODULE,
    .load_binary   = load_elf_binary,
    .load_shlib    = load_elf_library,
    .core_dump     = elf_core_dump,
    .min_coredump  = ELF_EXEC_PAGESIZE,
};
{% endhighlight %}

其中的 `load_binary` 函数指针指向的就是一个可执行程序的处理函数，要支持 ELF 文件的运行，则必须通过 `register_binfmt()` 向内核登记这个数据结构，加入到内核支持的可执行程序的队列中。

当要运行程序时，则扫描该队列，让各对象所提供的处理程序 (ELF中即为`load_elf_binary()`)，逐一前来认领，如果某个格式的处理程序发现相符后，便执行该格式映像的装入和启动。

### 内核加载

内核执行 `execv()` 或 `execve()` 系统调用时，会通过 `do_execve()` 调用，该函数先打开目标映像文件，并读入文件的头部信息，也就是开始 128 字节。

然后，调用另一个 `search_binary_handler()` 函数，该函数中会搜索上面提到的 Linux 支持的可执行文件类型队列，让各种可执行程序的处理程序前来认领和处理。

如果类型匹配，则调用 `load_binary` 函数指针所指向的处理函数来处理目标映像文件，对于 ELF 文件也就是 `load_elf_binary()` 函数，下面主要就是分析 `load_elf_binary()` 的执行过程。

{% highlight c %}
static int load_elf_binary(struct linux_binprm *bprm)
{
    /* 上述已经读取 128 字节的头部信息 */
    loc->elf_ex = *((struct elfhdr *)bprm->buf);

    retval = -ENOEXEC;
    /* 首先校验ELF的头部信息，也就是"\177ELF" */
    if (memcmp(loc->elf_ex.e_ident, ELFMAG, SELFMAG) != 0)
        goto out;
    /* 只能是可执行文件(ET_EXEC)或者动态库(ET_DYN) */
    if (loc->elf_ex.e_type != ET_EXEC && loc->elf_ex.e_type != ET_DYN)
        goto out;

    /* 读取ELF的头部信息，也就是整个Program Header Table，一个可执行程序必须至少有一个段，且不能超过64K */
    retval = kernel_read(bprm->file, loc->elf_ex.e_phoff, (char *)elf_phdata, size);
    if (retval != size) {
        if (retval >= 0)
            retval = -EIO;
        goto out_free_ph;
    }

    /* 在for循环中，用来寻找和处理目标映像的"解释器"段，可以通过 ???????? 查看 */
    for (i = 0; i < loc->elf_ex.e_phnum; i++) {
        if (elf_ppnt->p_type == PT_INTERP) {
            /* 找到后就根据其位置的p_offset和大小p_filesz把整个"解释器"段的内容读入缓冲区，实际上
             * 这个"解释器"段实际上只是一个字符串，即解释器的文件名，如"/lib/ld-linux.so.2"
             */
            retval = kernel_read(bprm->file, elf_ppnt->p_offset,
                         elf_interpreter, elf_ppnt->p_filesz);
            if (retval != elf_ppnt->p_filesz) {
                if (retval >= 0)
                    retval = -EIO;
                goto out_free_interp;
            }

            /* 通过open_exec()打开这个文件，再通过kernel_read()读入开始的128个字节，也就是解释器的头部 */
            interpreter = open_exec(elf_interpreter);
            retval = PTR_ERR(interpreter);
            if (IS_ERR(interpreter))
                goto out_free_interp;
            retval = kernel_read(interpreter, 0, bprm->buf, BINPRM_BUF_SIZE);
            if (retval != BINPRM_BUF_SIZE) {
                if (retval >= 0)
                    retval = -EIO;
                goto out_free_dentry;
            }
            break;
        }
        elf_ppnt++;
    }

    /* 从目标映像的程序头中搜索类型为PT_LOAD的段；在二进制映像中，只有类型为PT_LOAD的段才是需要加载的。
     * 只是在加载前，需要确定加载的地址，包括页对齐、该段的p_vaddr域的值；在确定了装入地址后，就会通过
     * elf_map()建立用户空间虚拟地址空间与目标映像文件中某个连续区间之间的映射，其返回值就是实际映射的
     * 起始地址。
     */
    for(i = 0, elf_ppnt = elf_phdata; i < loc->elf_ex.e_phnum; i++, elf_ppnt++) {
        if (elf_ppnt->p_type != PT_LOAD)
            continue;

        error = elf_map(bprm->file, load_bias + vaddr, elf_ppnt, elf_prot, elf_flags, total_size);
        if (BAD_ADDR(error)) {
            retval = IS_ERR((void *)error) ?
                PTR_ERR((void*)error) : -EINVAL;
            goto out_free_dentry;
        }
    }

    /* 若存在PT_INTERP段，则通过load_elf_interp()加载，并把进入用户空间的入口地址设置成load_elf_interp()
     * 的返回值，即解释器映像的入口地址；如果不装入解释器，那么这个入口地址就是目标映像本身的入口地址，
     * 也就是静态编译的程序。
     */
    if (elf_interpreter) {
        elf_entry = load_elf_interp(&loc->interp_elf_ex, interpreter, &interp_map_addr, load_bias);
        if (!IS_ERR((void *)elf_entry)) {
            interp_load_addr = elf_entry;
            elf_entry += loc->interp_elf_ex.e_entry;
        }
        if (BAD_ADDR(elf_entry)) {
            retval = IS_ERR((void *)elf_entry) ?  (int)elf_entry : -EINVAL;
            goto out_free_dentry;
        }
        reloc_func_desc = interp_load_addr;
    } else {
        elf_entry = loc->elf_ex.e_entry;
    }

    /* 完成加载且启动用户空间的映像运行之前，还需要为目标映像和解释器准备好一些有关的信息，这些信息包括常
     * 规的argc、envc等等，还有一些"辅助向量(Auxiliary Vector)"。这些信息需要复制到用户空间，使它们在CPU
     * 进入解释器或目标映像的程序入口时出现在用户空间堆栈上。这里的create_elf_tables()就起着这个作用。
     */
    retval = create_elf_tables(bprm, &loc->elf_ex, load_addr, interp_load_addr);
    if (retval < 0)
        goto out;

    /* 最后，通过start_thread()宏将eip和esp改成新的地址，就使得CPU在返回用户空间时就进入新的程序入口。如果存
     * 在解释器映像，那么这就是解释器映像的程序入口(动态链接)，否则就是目标映像的程序入口(静态链接)。
     */
    start_thread(regs, elf_entry, bprm->p);
}
{% endhighlight %}

## 用户态执行

内核加载完之后，那么就交给了用户态执行，在 `main()` 运行之前通常会先执行一段代码，运行这些代码的函数称为 **入口函数** 或 **入口点** ，大致的步骤如下：

* 操作系统创建进程后，把控制权交给程序入口，这个入口往往是运行库中的某个入口函数。
* 入口函数对运行库和程序运行环境进行初始化，包括堆、I/O、线程、全局变量构造等。
* 入口函数在完成初始化之后，调用 `main()` 函数，正式开始执行程序主体部分。
* `main()` 执行完后，返回到入口函数，入口函数进行清理工作，包括全局变量析构、堆销毁、关闭 IO 等，然后进行系统调用结束进程。


{% highlight text %}
$ readelf -h main | grep 'Entry point'
  Entry point address:               0x4004a0
{% endhighlight %}



`ld.so` (Dynamic Linker/Loader) 和 `ldd` 都会使用到 ELF 格式中的 `.dynstr` (dynamic linking string table) 字段，如果通过 `strip -R .dynstr hello` 命令将该字段删除，那么 `ldd` 就会报错。











### 常见概念

解释器 `.interp` 分区用于指定程序动态装载、链接器 `ld-linux.so` 的位置，而过程链接表 `plt`、全局偏移表 `got`、重定位表则用于辅助动态链接过程。

#### 符号

对于可执行文件除了编译器引入的一些符号外，主要就是用户自定义的全局变量、函数等，而对于可重定位文件仅仅包含用户自定义的一些符号。

{% highlight text %}
----- 生成可重定位文件，并通过nm命令查看ELF文件的符号表信息
$ gcc -c main.c
$ nm main.o
0000000000000000 B global
0000000000000000 T main
                 U printf
{% endhighlight %}

上面包含全局变量、自定义函数以及动态链接库中的函数，但不包含局部变量，而且发现这三个符号的地址都没有确定。

{% highlight text %}
----- 生成可执行文件
$ gcc -o main main.o
$ nm main | egrep "main$| printf|global$"
0000000000601038 B global
000000000040052d T main
                 U printf@@GLIBC_2.2.5
{% endhighlight %}

经链接之后，`global` 和 `main` 的地址都已经确定了，但是 `printf` 却还没，因为它是动态链接库 `glibc` 中定义函数，需要动态链接，而不是这里的静态链接。

也就是说 main.o 中的符号地址没有确定，而经过链接后部分符号地址已经确定，也就是对符号的引用变成了对地址的引用，这样程序运行时就可通过访问内存地址而访问特定的数据。对于动态链接库，也就是上述的 `printf()` 则需要在运行时通过动态链接器 ld-linux.so 进行重定位，即动态链接。

另外，除了 nm 还可以用 `readelf -s` 查看 `.dynsym` 表或者用 `objdump -tT` 查看。

{% highlight text %}
$ nm -D /lib64/libc-2.17.so | grep "\ printf$"
{% endhighlight %}

注意，在部分新系统上，如果不使用参数 `-D` ，那么可能会无法查看符号表，因为 nm 默认打印 `.symtab` 和 `.strtab`，不过一般在打包时会通过 strip 删除掉，只保留了动态符号 (在 `.dynsym` 和 `.dynstr` 中)，以便动态链接器在执行程序时寻址这些外部用到的符号。

<!-- https://stackoverflow.com/questions/9961473/nm-vs-readelf-s -->

<!--
## 动态链接

动态链接就是在程序运行时对符号进行重定位，确定符号对应的内存地址的过程。为了提高效率，Linux 下符号的动态链接默认采用 Lazy Mode 方式，也就是在程序运行过程中用到该符号时才去解析它的地址。

不过这种默认是可以通过设置 LD_BIND_NOW 为非空来打破的（下面会通过实例来分析这个变量的作用），也就是说如果设置了这个变量，动态链接器将在程序加载后和符号被使用之前就对这些符号的地址进行解析。

## 动态链接库

在程序中，保存了依赖的库信息。

$ readelf -d main | grep NEEDED
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

在 `.dynamic` 分区中保存了和动态链接相关的信息，当然只有版本信息，而没有保存绝对路径，其搜索路径如上所述。

### FIXME: 获取动态链接器

动态链接器保存在 `.interp` 分区，可以通过 `readelf -x .interp main` 命令查看。


注意，与 libc.so 之类的动态库不同，ld-linux.so 的路径是绝对路径，而类似于 libc.so 库则只包含了文件名。

这是因为，程序执行时 ld-linux.so 最先被加载到内存，没有其他程序知道去哪里查找 ld-linux.so，所以它的路径必须是绝对的；当 ld-linux.so 被装载以后，由它来去装载可执行文件和相关的共享库，它会根据下面介绍的流程去加载。

### 动态链接器

可以通过 `man ld-linux` 获取与动态链接器相关的资料，包括各种相关的环境变量和文件都有详细的说明。

对于环境变量，除了上面提到过的 LD_LIBRARY_PATH 和 LD_BIND_NOW 变量外，还有其他几个重要参数，比如 LD_PRELOAD 用于指定预装载一些库，以便替换其他库中的函数，从而做一些安全方面的处理 [6]，[9]，[12]，而环境变量 LD_DEBUG 可以用来进行动态链接的相关调试。
对于文件，除了上面提到的 ld.so.conf 和 ld.so.cache 外，还有一个文件 /etc/ld.so.preload 用于指定需要预装载的库。

实际上，ELF 格式可以从两个角度去看，包括链接和执行，分别通过 `Section Header Table` 和 `Program Header Table` 表示。

SHT 保存了 ELF 所包含的段信息，可以通过 `readelf -S /bin/bash` 查看，其中比较重要的有 REL sections (relocations), SYMTAB/DYNSYM (symbol tables), VERSYM/VERDEF/VERNEED sections (symbol versioning information).

#### 1. 加载到内存

在 ELF 文件的文件头中就指定了该文件的入口地址，程序的代码和数据部分会相继映射到对应的内存中。

$ readelf -h /bin/bash | grep Entry
  Entry point address:               0x41d361
-->

## 加载过程

依赖动态库时，会在加载时根据可执行文件的地址和动态库的对应符号的地址推算出被调用函数的地址，这个过程被称为动态链接。

假设，现在使用的是 Position Independent Code, PIC 模型，动态库加载的过程以及一些环境变量可以通过 `man ld-linux` 查看。



#### 1. 获取动态链接器

首先，读取 ELF 头部信息，解析出 `PT_INTERP` 信息，确定动态链接器的路径，可以通过 `readelf -l foobar` 查看，一般是 `/lib/ld-linux.so.2` 或者 `/lib64/ld-linux-x86-64.so.2` 。

#### 2. 加载动态库

关于加载的详细顺序可以查看 `man ld` 中 rpath-link 的介绍，一般顺序为：

1. 链接时 `-rpath-link` 参数指定路径，只用于链接时使用，编译时通过 `-Wl,rpath-link=` 指定；
2. 链接时通过 `-rpath` 参数指定路径，除了用于链接时使用，还会在运行时使用，编译时可利用 `-Wl,rpath=` 指定，会生成 `DT_RPATH` 或者 `DT_RUNPATH` 定义，可以通过 `readelf -d main | grep -E (RPATH|RUNPATH)` 查看；
3. 查找 `DT_RUNPATH` 或者 `DT_RPATH` 指定的路径，如果前者存在则忽略后者；
3. 依次查看 `LD_RUN_PATH` 和 `LD_LIBRARY_PATH` 环境变量指定路径；
4. 查找默认路径，一般是 `/lib` 和 `/usr/lib` ，然后是 `/etc/ld.so.conf` 文件中的配置。

另外，需要加载哪些库通过 `DT_NEEDED` 字段来获取，每条对应了一个动态库，可以通过 `readelf -d main | grep NEEDED` 查看。

### 示例程序

利用如下的示例程序。

{% highlight c %}
/* filename: foobar.c */
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

int foobar(void)
{
    Dl_info dl_info;
    dladdr((void*)foobar, &dl_info);
    fprintf(stdout, "load .so at: %s\n", dl_info.dli_fname);
    return 0;
}
{% endhighlight %}

{% highlight c %}
/* filename: main.c */
int foobar(void);
int main(int argc, char **argv)
{
  foobar();
  return 0;
}
{% endhighlight %}

{% highlight text %}
# filename: Makefile
all:
    gcc --shared -fPIC foobar.c -o libfoobar.so -ldl
    gcc main.c -o main -Wl,-rpath-link=/foobar -ldl -lfoobar -L./
    readelf -d main | grep -E (RPATH|RUNPATH)
{% endhighlight %}

然后可以通过依次设置如上的加载路径进行测试。**注意**，在对 `/etc/ld.so.conf` 文件设置后需要通过 `ldconfig` 更新 cache 才会生效。

另外，推荐使用 `DT_RUNPATH` 而非 `DT_RPATH` ，此时，在编译时需要用到 `--enable-new-dtags` 参数。

<!--
通过C语言直接读取rpath参数
#include <stdio.h>
#include <elf.h>
#include <link.h>

int main()
{
  const ElfW(Dyn) *dyn = _DYNAMIC;
  const ElfW(Dyn) *rpath = NULL;
  const char *strtab = NULL;
  for (; dyn->d_tag != DT_NULL; ++dyn) {
    if (dyn->d_tag == DT_RPATH) {
      rpath = dyn;
    } else if (dyn->d_tag == DT_STRTAB) {
      strtab = (const char *)dyn->d_un.d_val;
    }
  }

  if (strtab != NULL && rpath != NULL) {
    printf("RPATH: %s\n", strtab + rpath->d_un.d_val);
  }
  return 0;
}
-->

### 版本管理

不同版本的动态库可能会不兼容，那么如果程序在编译时指定动态库是某个低版本，运行是用的一个高版本，可能会导致无法运行。

假设有如下的示例：

{% highlight c %}
/* filename:hello.c */
#include <stdio.h>
void hello(const char* name)
{
    printf("hello %s!\n", name);
}
{% endhighlight %}

{% highlight c %}
/* filename:hello.h */
void hello(const char* name);
{% endhighlight %}

{% highlight text %}
# filename: Makefile
all:
    gcc hello.c -fPIC -shared -Wl,-soname,libhello.so.0 -o libhello.so.0.0.1
{% endhighlight %}

需要注意是，参数 `-Wl,soname` 中间没有空格，`-Wl` 选项用来告诉编译器将后面的参数传递给链接器，而 `-soname` 则指定了动态库的 `soname`。运行后在当前目录下会生成一个 `libhello.so.0.0.1` 文件，当运行 `ldconfig -n .` 命令时，当前目录会多一个符号连接。

这个软链接是根据编译生成 `libhello.so.0.0.1` 时指定的 `-soname` 生成的，会保存到编译生成的文件中，可以通过 `readelf -d foobar` 查看依赖的库。

所以关键就是这个 soname，它相当于一个中间者，当我们的动态库只是升级一个小版本时，可以让它的 soname 相同，而可执行程序只认 soname 指定的动态库，这样依赖这个动态库的可执行程序不需重新编译就能使用新版动态库的特性。

#### 测试程序

示例程序如下。

{% highlight c %}
/* filename:main.c */
#include "hello.h"
int main()
{
    hello("foobar");
    return 0;
}
{% endhighlight %}

然后可以通过 `gcc main.c -L. -lhello -o main` 编译，不过此时会报 `cannot find -lhello.so.0` 错误，也就是找不到对应的库。

在 Linux 中，编译时指定 `-lhello` 时，链接器会去查找 `libhello.so` 这样的文件，如果当前目录下没有这个文件，那么就会导致报错；此时，可以通过 `ln -s libhello.so.0.0.1 libhello.so` 建立这样一个软链接。

通过 `ldd` 查看时，发现实际依赖的是 `libhello.so.0` 而非 `libhello` 也不是 `libhello.so.0.0.1` ，其实在生成 main 程序的过程有如下几步：

1. 链接器通过编译命令 `-L. -lhello` 在当前目录查找 `libhello.so` 文件；
2. 读取 `libhello.so` 链接指向的实际文件，这里是 `libhello.so.0.0.1`；
3. 读取 `libhello.so.0.0.1` 中的 `SONAME`，这里是 `libhello.so.0`；
4. 将 `libhello.so.0` 记录到 `main` 程序的二进制数据里。

也就是说 `libhello.so.0` 是已经存储到 main 程序的二进制数据里的，不管这个程序在哪里，通过 `ldd` 查看它依赖的动态库都是 `libhello.so.0` 。

那么，在部署时，只需要安装 `libhello.so.0` 即可。

#### 版本更新

假设动态库需要做一个小小的改动。

{% highlight c %}
/* filename:hello.c */
#include <stdio.h>
void hello(const char* name)
{
    printf("hello %s, welcom to our world!\n", name);
}
{% endhighlight %}

由于改动较小，编译动态库时仍然指定相同的 soname 。

{% highlight text %}
$ gcc hello.c -fPIC -shared -Wl,-soname,libhello.so.0 -o libhello.so.0.0.2
{% endhighlight %}

然后重新运行 `ldconfig -n .` 即可，会发现链接指向了新版本，然后直接运行即可。

同样，假如我们的动态库有大的改动，编译动态库时指定了新的 soname，如下：

{% highlight text %}
$ gcc hello.c -fPIC -shared -Wl,-soname,libhello.so.1 -o libhello.so.1.0.0
{% endhighlight %}

将动态库文件拷贝到运行目录，并执行 `ldconfig -n .`，不过此时需要重新编译才可以。


### 问题排查

可以通过如下方法查看。

{% highlight text %}
LD_TRACE_LOADED_OBJECTS=1 LD_BIND_NOW=1 LD_TRACE_PRELINKING=t /lib/ld-linux.so.2 python.so | grep ^undefined
{% endhighlight %}

<!--
Unhandled python exception in importing module: ImportError: /usr/lib64/python2.7/lib-dynload/_functoolsmodule.so: undefined symbol: PyExc_TypeError
LD_TRACE_LOADED_OBJECTS=1 LD_BIND_NOW=1 LD_TRACE_PRELINKING=t ./daemon/cloudagent_monitor -f
https://blog.csdn.net/jq0123/article/details/1340839
https://stackoverflow.com/questions/11643666/python-importerror-undefined-symbol-for-custom-c-module?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
-->

{% highlight text %}
{% endhighlight %}
