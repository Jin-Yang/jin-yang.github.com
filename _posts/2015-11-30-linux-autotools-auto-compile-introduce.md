---
title: Linux 自动编译 AutoTools
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,automake,configure,cmake
description: 在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。这里简单介绍下这种构建方式的细节，以及开发者如何利用 autotools 创建兼容 GNU 构建系统的项目。
---

在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。

这里简单介绍下 AutoTools 一系列工具的使用。

<!-- more -->

## Autotools

在 Linux 平台上，经常使用 `configure->make->make install` 从源码开始编译安装，也就是 GNU 构建系统，利用脚本和 make 程序在特定平台上编译软件，也就是利用 autotools 创建构建系统的项目。

注意，有些程序虽然也是采用上述的三步，但并未采用 autotools 实现，如 nginx 是作者编写的构建程序。

在 CentOS 中可以直接通过 `yum install automake autoconf` 安装相关的文件；利用 autotools 生成 Makefile 的过程如下图所示。

![autotools]({{ site.url }}/images/linux/autotools-process-1.png "autotools"){: .pull-center }

其中用到的核心工具包括了 Autoconf 和 Automake ，首先从用户和开发者角度看看两者的区别。关于 autotools 的简单处理流程可以参考 [automake](http://www.gnu.org/software/automake/manual/automake.html) 中的 `Setup Explained` 内容。

### 用户视角

`configure` 脚本是由软件开发者维护并发布给用户使用的 shell 脚本，该脚本作用是检测系统环境，最终目的是生成 Makefile 和 config.h 两个文件。

开发者在分发源码包时，除了源代码 (.c .h)，还有许多支撑软件构建的文件和工具，其中最重要的文件就是 Makefile.in 和 config.h.in 两个，在 configure 脚本执行成功后，将为每个 `*.in` 文件处理成对应的非 `*.in` 文件。

#### configure

configure 脚本会检查当前系统，而检查项的多少取决于开发者，一般来说，主要检查当前目标平台的程序、库、头文件、函数等的兼容性，而检查结果将作用于 config.h 和 Makefile 文件的生成，从而影响最终的编译。

用户可通过参数定制软件所需要包含的组件、安装路径等，一般会被五部分，可以通过 `--help` 参数查看当前软件提供了那些配置参数。

<!--
    *安装路径相关配置。最常见的是--prefix。
    *程序名配置。例如--program-suffix可用于为生成的程序添加后缀。
    *跨平台编译。不太常用。
    *动态库静态库选项。用于控制是否生成某种类型的库文件。
    程序组件选项。用于配置程序是否将某种功能编译到程序中，一般形如--with-xxx。这可能是最常用的配置，而且由软件开发者来定义。
（*表示这是几乎所有软件都支持的配置，因为这些配置是autotool生成的configure脚本默认支持的。）
-->

在 configure 在执行过程中，除了生成 Makefile 外，还会生成如下的临时文件：

* config.log 日志文件；
* config.cache 缓存，以提高下一次 configure 的速度，需通过 -C 来指定才会生成；
* config.status 实际调用编译工具构建软件的 shell 脚本。

如果软件通过 libtool 构建，还会生成 libtool 脚本，关于 libtool 脚本如何生成，详见如下。


### 开发者视角

开发者除了编写软件本身的代码外，还需要负责生成构建软件所需要文件和工具，通过 autotools 工具可以解决一些常见的平台问题，但是编写依旧复杂。为了生成 configure 脚本和 Makefile.in 等文件，开发者需要创建并维护一个 configure.ac 文件，以及一系列的 Makefile.am 文件。<!--autoreconf程序能够自动按照合理的顺序调用autoconf automake aclocal等程序。-->

#### Autoconf

该工具用于生成一个可在类 Unix 系统下工作的 Shell 脚本，该脚本可在不同 *nix 平台下自动配置软件源代码包，也就是 configure 文件，生成脚本后与 autoconf 再无关系。

<!--
对于每个使用了Autoconf的软件包，Autoconf从一个列举了该软件包需要的，或者可以使用的系统特征的列表的模板文件中生成配置脚本。在 shell代码识别并响应了一个被列出的系统特征之后，Autoconf允许多个可能使用（或者需要）该特征的软件包共享该特征。 如果后来因为某些原因需要调整shell代码，就只要在一个地方进行修改； 所有的配置脚本都将被自动地重新生成以使用更新了的代码。
-->

#### Automake

该工具通过 Makefile.am 文件自动生成 Makefile.in 文件，而 Makefile.am 基本上是一系列 make 宏定义，该文件需要手动编辑，也就是将对 Makefile 的编辑转义到了 Makefile.am 文件。

<!--
Automake 支持三种目录层次： “flat”、“shallow”和“deep”。一个flat（平）包指的是所有文件都在一个目录中的包。为这类包提供的`Makefile.am' 缺少宏SUBDIRS。这类包的一个例子是termutils。一个deep（深）包指的是所有的源代码都被储存在子目录中的包；顶层 目录主要包含配置信息。GNU cpio 是这类包的一个很好的例子，GNU tar也是。deep包的顶层`Makefile.am'将包括 宏SUBDIRS，但没有其它定义需要创建的对象的宏。一个shallow（浅）包指的是主要的源代码储存在顶层目录中，而 各个部分（典型的是库）则储存在子目录中的包。Automake本身就是这类包（GNU make也是如此，它现在已经不使用automake）。
-->

### 常见文件

接下来看下一些常用的文件。

#### configure.ac

用于通过 autoconf 命令生成 configure 脚本，如下是一个 configure.ac 的示例：

{% highlight text %}
AC_PREREQ([2.69])
AM_INIT_AUTOMAKE(hello, 1.0)
AC_INIT([hello], [1.0], [www.douniwan.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.
AC_CONFIG_FILES([Makefile                     # 主要是通过*.in模板生成响应的文件
                 src/Makefile
                 src/a/Makefile
                 src/b/Makefile])
AC_OUTPUT
{% endhighlight %}

以 `AC_` 开头的是一些宏调用，与 C 中的宏概念类似，会被替换展开；很多以 AC_PROG_XXX 开头的宏用于检查所需要的程序是否存在，详细可以查看 [Particular Program Checks](https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Particular-Programs.html)；对于一些特殊的函数或者文件则可以通过 [Generic Program and File Checks](https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Generic-Programs.html) 中定义的宏进行检查。

而 m4 是一个经典的宏工具，autoconf 正是构建在 m4 之上，可以简单理解为 autoconf 预先实现了大量用于检测系统可移植性的宏，这些宏在展开后就是大量的 shell 脚本。所以编写 configure.ac 需要对这些宏熟练掌握，并且合理调用，有时，甚至可以自己实现自己的宏。

#### configure.scan

通过 autoscan 命令可以得到一个初始化的 configure.scan 文件，然后重命名为 configure.ac 后，在此基础上编辑 configure.ac ，而 autoscan 通常 **只用于首次初始化 configure.ac** 。

autoscan 会扫描源码，并生成一些通用的宏调用、输入的声明以及输出的声明。

#### config.h.in

可以通过 autoheader 命令扫描 configure.ac 中的内容，并生成 config.h.in 文件；每当 configure.ac 文件有所变化，都可以再次执行 autoheader 更新 config.h.in 。

在 configure.ac 中通过 `AC_CONFIG_HEADERS([config.h])` 告诉 autoheader 应当生成 config.h.in 的路径；在最后的实际编译阶段，生成的编译命令会加上 `-DHAVE_CONFIG_H` 定义宏。

{% highlight text %}
/bin/sh ../../libtool --tag=CC --mode=compile gcc -DHAVE_CONFIG_H ...
{% endhighlight %}

于是在代码中，可以通过下面代码安全的引用 config.h 。

{% highlight c %}
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
{% endhighlight %}

config.h 包含了大量的宏定义，其中包括软件包的名字等信息，程序可以直接使用这些宏；更重要的是，程序可以根据其中的对目标平台的可移植性相关的宏，通过条件编译，动态的调整编译行为。

#### Makfile.am

手工编写 Makefile 是一件相当烦琐的事情，而且，如果项目复杂的话，编写难度将越来越大；为此，可以通过 automake+Makefile.am 生成 Makefile.in 文件，通常一个 Makefile.am 的示例如下。

{% highlight text %}
SUBDIRS         = a b
bin_PROGRAMS    = st
st_SOURCES      = main.c
st_LDADD        = $(top_builddir)/src/a/liba.la $(top_builddir)/src/b/libb.la
{% endhighlight %}

通过 SUBDIRS 声明了两个子目录，子目录的中的构建需要靠 a/Makefile.am 和 b/Makefile.am 来进行，这样多目录组织起来就方便多了。

bin_PROGRAMS 声明一个可执行文件，st_SOURCES 指定所依赖的源代码文件，st_LDADD 声明了可执行文件在连接时，需要依赖的 Libtool 库文件。

由于 automake 晚于 autoconf，所以 automake 是作为 autoconf 的扩展来实现的，在 configure.ac 中需要通过声明 AM_INIT_AUTOMAKE 告诉 autoconf 需要配置和调用 automake 。


#### aclocal

如上所述，configure.ac 是依靠宏展开来得到 configure 的，因此，能否成功生成取决于宏定义能否找到；默认 autoconf 会从安装路径下来寻找事先定义好了宏，而对于像 automake、libtool 和 gettext 等第三方扩展宏，甚至是开发者自行编写的宏就一无所知了。

于是，存在这个工具 aclocal，将在 configure.ac 同一目录下生成 aclocal.m4，在扫描 configure.ac 的过程中，将第三方扩展和开发者自己编写的宏定义复制进去；这样，autoconf 在遇到不认识的宏时，就会从 aclocal.m4 中查找。


#### libtool

libtool 试图解决不同平台下库文件的差异，实际是一个 shell 脚本，实际工作过程中，调用了目标平台的 cc 编译器和链接器，以及给予合适的命令行参数，libtool 可以单独使用。

automake 支持 libtool 构建声明，在 Makefile.am 中，普通的库文件目标写作 xxx_LIBRARIES ：

{% highlight text %}
noinst_LIBRARIES = liba.a
liba_SOURCES = ao1.c ao2.c ao3.c
{% endhighlight %}

而对于 libtool 目标，写作 xxx_LTLIBRARIES，并以 .la 作为后缀声明库文件。

{% highlight text %}
noinst_LTLIBRARIES = liba.la
liba_la_SOURCES = ao1.c ao2.c ao3.c
{% endhighlight %}

在 configure.ac 中需要声明 LT_INIT：

{% highlight text %}
...
AM_INIT_AUTOMAKE([foreign])
LT_INIT
...
{% endhighlight %}

有时，如果要用到 libtool 中的某些宏，则推荐将这些宏复制到项目中。首先，通过 `AC_CONFIG_MACRO_DIR([m4])` 指定使用 m4 目录存放第三方宏；然后在最外层的 Makefile.am 中加入 `ACLOCAL_AMFLAGS = -I m4` 。

<!--
https://segmentfault.com/a/1190000006915719


Configure、Makefile.am、Makefile.in、Makefile之间的关系
http://www.51cos.com/?p=1649

可以参考官方文档
http://inti.sourceforge.net/tutorial/libinti/autotoolsproject.html


通过--with-libmysql=/opt/mysql/lib指定MySQL库的路径；--enable-mysql编译MySQL插件；

需要手动编译：
1. Makefile.am
用于生成Makefile.in文件，需要手动书写。

[s]bin_PROGRAMS  需要生成的可执行文件的文件名，多个文件以空格分割；
foobar_SOURCES 指定生成命令所需要的文件，包括源码以及头文件；

1. autoscan(autoconf)
扫描源代码以搜寻普通的可移植性问题，比如检查编译器，库，头文件等，生成文件configure.scan，然后手动编辑生成configure.ac。
your source files --- [autoscan*] --- [configure.scan] -<edit>- configure.ac
常用函数

#### AC_ARG_WITH()

AC_ARG_WITH (package, help-string, [action-if-given], [action-if-not-given])
此时就是可以命令行中使用--with-package或者--without-package进行配置，其中第三个参数需要检查yes/no；配置完成之后可以通过`./configure --help`查看。
AC_ARG_WITH(libmysql, [AS_HELP_STRING([--with-libmysql@<:@=PREFIX@:>@], [Path to libmysql.])],
[
  if test "x$withval" = "xno"; then
    with_libmysql="no"
  else if test "x$withval" = "xyes"; then
    with_libmysql="yes"
  else
    if test -f "$withval" && test -x "$withval";
    then
      with_mysql_config="$withval"
    else if test -x "$withval/bin/mysql_config"
    then
      with_mysql_config="$withval/bin/mysql_config"
    fi; fi
    with_libmysql="yes"
  fi; fi
],
[
  with_libmysql="yes"
])
# 接下来可以通过shell脚本再进行一些检查，也可以使用AC_CHECK_LIB()、AC_CHECK_HEADERS()函数，失败则可以用AC_MSG_FAILURE()打印信息，暂时省略
if test "x$with_libmysql" = "xyes"
then
  BUILD_WITH_LIBMYSQL_CFLAGS="$with_mysql_cflags"
  AC_SUBST(BUILD_WITH_LIBMYSQL_CFLAGS)
  AC_SUBST(BUILD_WITH_LIBMYSQL_LIBS, "$with_mysql_libs")
fi
# 设置条件变量，可以再Makefile.am中使用
AM_CONDITIONAL(BUILD_WITH_LIBMYSQL, test "x$with_libmysql" = "xyes")
-->



### Flat

这一模式就是所有的文件，包括源码、头文件等，都在同一目录下。

如下示例中，目录下存在三个文件：hello.h (声明hello()方法)、hello.c (实现hello()方法)、main.c (主文件调用了hello()方法)。执行方法如下：

<ol><li>
autoscan<br>在目录下，直接执行该命令，然后会生成 configure.scan 和 autoscan.log (可删除) 两个文件，将 configure.scan 重命名为 <font color="blue">configure.ac</font> 文件 (注意，不是 configure.in，这个是老版本)，修改文件的内容。
{% highlight text %}
AC_PREREQ([2.69])                             # 设置Autoconf的最小版本
AM_INIT_AUTOMAKE(hello, 1.0)                  # 表示用于automake调用，可为AM_INIT_AUTOMAKE
AC_INIT([hello], [1.0], [www.douniwan.com])   # 设置包名称、版本、bug报告地址
AC_CONFIG_SRCDIR([main.c])                    # 可为包中的任意源码，用于判断是否存在源码
AC_CONFIG_HEADERS([config.h])                 # 表示在调用automake时依赖config.h.in文件，如果不定义则会
                                              # 在编译时会有大量的-DXXX=xxxd的定义参数
AC_PROG_CC                                    # 检测是否存在C编译器
AC_PROG_CXX                                   # 检测是否存在C++编译器
AC_OUTPUT(Makefile)                           # 指定configure输出的文件
{% endhighlight %} </li><li>

aclocal<br>
是一个 perl 脚本，用于生成 aclocal.m4 文件供 automake 和 autoconf 调用，详见 info aclocal。</li><li>

autoheader<br>用于产生 config.h.in 文件，感觉应该是配置通过 configure 输入 config.h 文件时需要定义那些宏。</li><li>

automake --add-missing<br>
新建文件<font color="blue">Makefile.am</font>文件，通过Makefile.am生成Makefile.in供configure脚本调用生成Makefile。

{% highlight text %}
AUTOMAKE_OPTIONS=foreign                     # 不遵循gnu的标准
bin_PROGRAMS=hello                           # 最后生成的程序，多个空格间隔
hello_LDFLAGS = -Wall                        # 编译选项，只用于可执行文件以及shared library
hello_SOURCES=main.c hello.c hello.h

noinst_LIBRARIES = libfoo.a
libfoo_a_CPPFLAGS = -g -Wall
libfoo_a_LIBADD = lpthread -lm            // 用于静态链接库
libfoo_a_SOURCES =  cmds.c cmds.h \
                    app.c app.h
foo_SOURCES = main.c
foo_LDADD = libfoo
{% endhighlight %}

autoconf<br>
主要是用来检测所依赖的条件是否成立，检测的结果可以保存为文件供其它应用使用，其脚本实际使用shell和M4(可以C语言中宏的加强版)组成，还有部分宏是通过aclocal生成的，保存在aclocal.m4中。<br><br>
在生成configure脚本的时候同时会保存日志方便调试，第一次执行完之后可以通过config.status更新。通常是用来生成一个配置文件，否则通过编译参数进行配置，此时编译选项将非常大。</li><br><li>

configure<br>
到现在已经生成了 configure 文件，通过 ./configure 即可以生成Makefile文件。</li><br><li>

Makefile<br>
主要包括了如下的选项，install安装，clean清除，dist打包软件发布，distcheck对发布的包进行检测。
</li></ol>
编辑 configure.ac 时，可以使用 case；其中 dnl 用于表示注释。<br><br>



### 其它

在正式执行 configure 之前，会调用 `AC_DEFUN()` 执行一些函数的扩展，然后调用 `AC_INIT()` 执行初始化操作；在 `AC_INIT()` 宏中，会初始化一些变量参数值，包括 `AC_PACKAGE_NAME`、`PACKAGE_NAME` 等。

{% highlight text %}
AC_INIT (package, version, [bug-report], [tarname], [url])
AC_INIT ([collectd], [m4_esyscmd(./version-gen.sh)])
{% endhighlight %}

<!--
AC_PACKAGE_NAME, PACKAGE_NAME
  Exactly package.
AC_PACKAGE_TARNAME, PACKAGE_TARNAME
  Exactly tarname, possibly generated from package.
AC_PACKAGE_VERSION, PACKAGE_VERSION
  Exactly version.
AC_PACKAGE_STRING, PACKAGE_STRING
  Exactly ‘package version’.
AC_PACKAGE_BUGREPORT, PACKAGE_BUGREPORT
  Exactly bug-report, if one was provided.
AC_PACKAGE_URL, PACKAGE_URL
  Exactly url, if one was provided. If url was empty, but package begins with ‘GNU ’, then this defaults to ‘http://www.gnu.org/software/tarname/’, otherwise, no URL is assumed.
-->



#### 修改configure.ac

需要执行 `autoreconf -ivf` 命令来更新 configure 脚本文件，autoreconf 可以看做是 autoconf、autoheader、acloacl、automake、libtoolize、autopoint 的组合体，而且会以合适的顺序来执行。

注意，在修改了 acinclude.m4、configure.ac 之后，就需要通过 aclocal 重新生成 aclocal.m4 文件；如果直接通过 autoconf 配置，部分库会报 `aclocal-1.14: command not found` 错误。据说 [是由于时间戳导致](http://stackoverflow.com/questions/33278928/how-to-overcome-aclocal-1-15-is-missing-on-your-system-warning-when-compilin) 的，不过安装了 automake、autoconf 工具后，最简单的方式是执行 autoreconf 命令。

#### 修改Makefile.am

需要在源码的跟目录下执行 automake 命令更新，然后在 build 目录下直接重新执行 make 命令即可。


#### 非标准库

如果有些库没有安装在标准路径下，或者需要使用不同的版本库，那么就需要通过参数指定；以使用 MySQL 库为例，通常可以通过 `--with-libmysql=/opt/mysql/lib` 参数指定特定库路径。

当然，这需要在制作 configure.ac 文件时通过 `AC_ARG_WITH()` 函数指定；如果使用 `./configure --help` 查看时会发现没有提供该 `--with-libmysql` 参数，这时候就需要通过如下方式指定。

{% highlight text %}
----- 通过指定环境变量方式添加库以及头文件搜索路径
env CPPFLAGS="-I/include/path"  LDFLAGS="-L/lib/path"  ./configure --prefix=/opt/foobar

----- 也可以把env省掉，作用相同
CPPFLAGS="-I/include/path"  LDFLAGS="-L/lib/path"  ./configure --prefix=/...
{% endhighlight %}

另外，还可以通过 `LIBRARY_PATH` 环境变量指定编译期间搜索 lib 库的路径，使用冒号分割，此时会先搜索该变量指定的路径，如果找不到则搜索系统默认搜索路径；而 `LD_LIBRARY_PATH` 则用于指定程序运行期间查找so动态链接库的搜索路径。

#### 测试功能

automake 提供了简单测试功能，在运行程序时返回非 0 则认为失败，否则认为成功；示例如下，需要在 `Makefile.am` 中添加如下内容，然后通过 `make check` 执行检查，相关可以参考 [Tests](https://www.gnu.org/software/automake/manual/html_node/Tests.html) 。

{% highlight text %}
TESTS = check_money
check_PROGRAMS = check_money
check_money_SOURCES = check_money.c $(top_builddir)/src/money.h
check_money_CFLAGS = @CHECK_CFLAGS@
check_money_LDADD = $(top_builddir)/src/libmoney.la @CHECK_LIBS@
{% endhighlight %}

另外，可以在 `Makefile.am` 中，可以增加其它功能测试，例如内存泄露，增加如下内容，详细参考 [Parallel Test Harness](https://www.gnu.org/software/automake/manual/html_node/Parallel-Test-Harness.html) 。

<!-- LOG_COMPILER = env VALGRIND="@VALGRIND@" $(abs_top_srcdir)/testwrapper.sh  -->

#### LIBADD VS. LDADD

简单来说 `LIBADD` 用于库，`LDADD` 用于可执行文件。

例如，要通过 libtool 生成一个 libfoo.la 静态库，那么可以使用如下方式。

{% highlight text %}
libfoo_la_LIBADD = libbar.la
{% endhighlight %}

如果使用了非 libtool 库，那么就需要通过 `-L` 和 `-l` 参数选项。

{% highlight text %}
libfoo_la_LIBADD = libbar.la -L/opt/local/lib -lpng
{% endhighlight %}

不过一般来说，回通过 configure 脚本查找所依赖的库，然后通过 `AC_SUBST` 函数将其导出，然后使用如下方式引用。

{% highlight text %}
libfoo_la_LIBADD = libbar.la $(EXTRA_FOO_LIBS)
{% endhighlight %}

而对于一个程序则使用如下方式。

{% highlight text %}
myprog_LDADD = libfoo.la # links libfoo, libbar, and libpng to myprog.
{% endhighlight %}

<!--
Sometimes the boundaries are a bit vague. $(EXTRA_FOO_LIBS) could have been added to myprog_LDADD. Adding dependencies to a libtool (.la) library, and using libtool do all the platform-specific linker magic, is usually the best approach. It keeps all the linker metadata in the one place.
-->

### 总结

需要手动维护 configure.ac 和 Makefile.am 文件。

## 参考

可以查看官方文档 [automake](https://www.gnu.org/software/automake/manual/automake.html)，[GNU Autoconf - Creating Automatic Configuration Scripts](https://www.gnu.org/software/autoconf/manual/) 。

{% highlight text %}
{% endhighlight %}
