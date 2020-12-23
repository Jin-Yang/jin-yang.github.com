---
title: C++ 命令行解析包 gflags 使用详解
layout: post
comments: true
language: chinese
tag: [C/C++, Program]
keywords: gflags,googel,c/c++,cpp,命令行解析,getopt
description: gflags 是 Google 提供的一个命令行参数处理的开源库，比传统的 getopt() 功能更加强大，允许在不同文件中定义参数，这里详细介绍其使用方法。
---

gflags 是 Google 提供的一个命令行参数处理的开源库，目前已经独立开源，比传统的 `getopt()` 功能更加强大，可以将不同的参数定义分布到各个源码文件中，不需要集中管理。

提供了 C++ 和 Python 两个版本，这里仅详细介绍 C++ 版本的使用方式。

<!-- more -->

## 简介

配置参数分开还是集中管理没有严格的约束，关键要看项目里的统一规范，只是，gflags 可以支持这两种方式，允许用户更加灵活的使用。

当将参数分布到各个源码文件中时，如果定义了相同的参数，那么在编译的时候会直接报错。

### 安装

很多发行版本会有自己相关的开发库，这里简单介绍使用 CMake 从源码进行编译，源码可以从 [GitHub gflags Releases](https://github.com/gflags/gflags/releases) 中选择相关的版本。

如下命令以最新的 `2.2.2` 版本为例。

```
$ tar xzf gflags-2.2.2.tar.gz
$ cd gflags-2.2.2
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
$ make test     # 单元测试，执行cmake时需要增加-DBUILD_TESTING=true参数
# make install  # 安装，一般需要root用户执行
```

默认会安装到 `/usr/local` 目录下，需要配置动态库、头文件路径等，通过上述的 `-DCMAKE_INSTALL_PREFIX=/usr` 参数修改该路径，使用系统默认路径，此时会安装如下的文件。

```
/usr/lib/libgflags.a
/usr/lib/libgflags_nothreads.a
/usr/include/gflags/gflags.h
/usr/include/gflags/gflags_declare.h
/usr/include/gflags/gflags_completions.h
/usr/include/gflags/gflags_gflags.h
/usr/lib/cmake/gflags/gflags-config.cmake
/usr/lib/cmake/gflags/gflags-config-version.cmake
/usr/lib/cmake/gflags/gflags-targets.cmake
/usr/lib/cmake/gflags/gflags-targets-release.cmake
/usr/lib/cmake/gflags/gflags-nonamespace-targets.cmake
/usr/lib/cmake/gflags/gflags-nonamespace-targets-release.cmake
/usr/bin/gflags_completions.sh
/usr/lib/pkgconfig/gflags.pc
```

详细的安装可以参考 [gflags install.md](https://github.com/gflags/gflags/blob/master/INSTALL.md) 中的介绍，可以使用 `ccmake` 选择配置项，或者使用上述的 `cmake` + 参数的方式配置。

### 示例

假设有个网络客户端代码，需要指定服务端的地址和端口，希望有默认参数，同时允许用户通过命令行来指定不同的值。

``` cpp
#include <iostream>
#include <gflags/gflags.h>

DEFINE_string(host, "localhost", "Server host address");
DEFINE_int32(port, 8080, "Server port");

int main(int argc, char **argv)
{
        gflags::ParseCommandLineFlags(&argc, &argv, true);
        std::cout << "Got '" << FLAGS_host << ":" << FLAGS_port << "'." << std::endl;
        return 0;
}
```

在代码开头通过 `DEFINE_XXX` 定义参数，包括了变量名、默认值、参数介绍等；主程序中使用 `gflags::ParseCommandLineFlags()` 函数解析参数；使用时，在变量名称前添加 `FLAGS_` 头即可。

通过如下命令行进行编译。

```
g++ main.cc -std=c++11 -o gflags -lgflags -lpthread
```

默认是需要 `pthread` 线程库的，暂时还不太确定没有使用多线程时，如何关闭该参数。

然后，可以通过如下方式指定参数。

```
----- 不指定参数，使用默认值
$ ./gflags
Got 'localhost:8080'.

----- 可以选择指定一个参数，或者多个参数
$ ./gflags -host www.foobar.com
Got 'www.foobar.com:8080'.
$ ./gflags -port 80
Got 'localhost:80'.
$ ./gflags -host www.foobar.com -port 80
Got 'www.foobar.com:80'.

----- 同时支持不同的参数指定方式
$ ./gflags --host www.foobar.com --port 80
Got 'www.foobar.com:80'.
$ ./gflags --host=www.foobar.com=--port 80
Got 'www.foobar.com:80'.
```

同时也可以使用 `--help` 参数查看帮助信息，包含了 gflags 库提供的参数，以及用户提供的参数，如下是输出的用户参数信息。

```
  Flags from main.cc:
    -host (Server host address) type: string default: "localhost"
    -port (Server port) type: int32 default: 8080
```

接着看看详细使用方式。

## 使用详解

包含了如何定义、解析等使用场景。

### 定义参数

在如上的示例中定义了两种类型的参数，分别为字符串 `string` 和整型 `int32` ，包括了变量名、默认值、参数介绍三个入参，三个参数都是必须的。

`gflags` 总共提供了六种定义方式 （或者说类型）。

```
DEFINE_bool     boolean
DEFINE_int32    32-bit integer
DEFINE_int64    64-bit integer
DEFINE_uint64   unsigned 64-bit integer
DEFINE_double   double
DEFINE_string   C++ string
```

可以定义到某个 NameSpace 下，这样在使用时也必须要带着 NameSpace 前缀。如果在不同的文件中定义，那么可以在某个集中的头文件中通过 `DECLARE_XXX(VAR)` 进行声明。

注意，不要定义相同名称的参数，即使在不同的 NameSpace 也不可以；还有几个保留参数，包括了 `flagfile` `fromenv` `tryfromenv` `undefok` 等等。

### 参数解析

在上述的示例中，通过 `ParseCommandLineFlags()` 函数解析参数，另外还有不带帮助文档的解析方式，两个函数的声明如下。

```
uint32 ParseCommandLineFlags(int* argc, char*** argv, bool remove_flags);
uint32 ParseCommandLineNonHelpFlags(int* argc, char*** argv, bool remove_flags);
```

其中 `remove_flags` 标识指定参数的处理方式，如果为 `true` ，那么解析时会将 flag 以及 flag 对应的值从 argv 中删除，并修改 argc ，也就是说，最后存放的是不包含 flag 的参数；如果为 false 则仅对参数重排，标志位参数放最前面。

#### 参数校验

为了检查参数的值是否合法，可以针对某个参数注册一个验证函数，当参数解析或者修改 (调用 `SetCommandLineOption()` 时) 该验证函数都会被调用，返回 true 表示校验成功，否则失败。

如下是对于 `port` 参数的校验。

``` cpp
#include <iostream>
#include <gflags/gflags.h>

DEFINE_string(host, "localhost", "Server host address.");
DEFINE_int32(port, 8080, "Server port.");

static bool ValidatePort(const char *flag, gflags::int32 value)
{
        if (value > 0 && value < 32768)
                return true;
        std::cerr << "Invalid value for --" << flag << ": " << value << std::endl;
        return false;
}
static const bool port_dummy = gflags::RegisterFlagValidator(&FLAGS_port, &ValidatePort);

int main(int argc, char **argv)
{
        gflags::ParseCommandLineFlags(&argc, &argv, true);
        std::cout << "Got '" << FLAGS_host << ":" << FLAGS_port << "'" << std::endl;
        return 0;
}
```

如果超过了指定的范围则会报错。

### 使用参数

提供了默认的 `--help` 查看帮助信息，指定参数可以使用 `-` 或者 `--` 符号，参数和值的分割可以使用 ` ` 或者 `=` ，如上的示例。

对于布尔类型，还可以使用 `--foobar` `--nofoobar` `--foobar=true` `--foobar false` 的方式指定。

#### 检查参数是否设置

在通过 `ParseCommandLineFlags()` 函数解析完参数之后，可以通过如下方法检查对应的变量是否被设置。

``` cpp
gflags::CommandLineFlagInfo info;
if (gflags::GetCommandLineFlagInfo("port", &info) && info.is_default) {
        std::cout << "port is not set." << std::endl;
} else {
        std::cout << "port is set." << std::endl;
}
```

这里不是直接比对的设置值与默认值相同，即使指定了与默认值相同的值，也会被认为参数被修改了。

#### 文件引入

可以通过 `--flagfile=FileName` 指定参数文件名，文件名也可以使用通配符 `*` 以及 `?`，在文件中每行标识一个参数，例如：

```
$ cat flags.txt
# This is the test server.
--host=www.foobar.com
--port=80
$ ./gflags --flagfile flags.txt
Got 'www.foobar.com:80'.
```

以 `#` 开头的为注释，也可以再次使用 `--flagfile=FileName` 包含一个参数配置文件。

#### 环境变量引入

可以使用 `--fromenv` 或者 `--tryfromenv` 从环境变量中引入参数，也可通过 `--fromenv=foo,bar` 指定读取的参数，当然，需要先设置好环境变量。

```
export FLAGS_foo=xxx; export FLAGS_bar=yyy   # sh
setenv FLAGS_foo xxx; setenv FLAGS_bar yyy   # tcsh
```

这种方式等价于在命令行指定 `--foo=xxx --bar=yyy` 参数。

其中 `--fromenv` 时如果环境变量不存在则会报错，而 `--tryfromenv` 当环境变量中不存在时会使用默认值。

#### 程序中指定

最常见的是允许用户动态进行配置，也就是说动态加载，可以调用 `SetCommandLineOption()` 函数来实现，函数的声明如下。

```
std::string SetCommandLineOption(const char* name, const char* value);
```

例如。

```
gflags::SetCommandLineOption("port", "9999");
```

成功则会返回 `port set to 9999` 字符串，否则会返回空字符串。

另外也可以通过 `bool GetCommandLineOption(const char* name, std::string* OUTPUT)` 函数获取 flag 的接口，如果没有指定，则会通过 `OUTPUT` 返回默认的值，只有当指定了一个未定义的 `flag` 名称时，会返回 `false` 。

正常读写都可以使用 `if (FLAGS_foo); FLAGS_Foo = bar` 的形式，但是如果需要线程安全的调用，建议使用这两个函数。

### 完整示例

如上设置的完整示例如下。

``` cpp
#include <iostream>
#include <gflags/gflags.h>

DEFINE_string(host, "localhost", "Server host address.");
DEFINE_int32(port, 8080, "Server port.");

static bool ValidatePort(const char *flag, gflags::int32 value)
{
        if (value > 0 && value < 32768)
                return true;
        std::cerr << "Invalid value for --" << flag << ": " << value << std::endl;
        return false;
}
static const bool port_dummy = gflags::RegisterFlagValidator(&FLAGS_port, &ValidatePort);

int main(int argc, char **argv)
{
        gflags::SetVersionString("1.1.0");
        gflags::SetUsageMessage("./gflags");
        gflags::ParseCommandLineFlags(&argc, &argv, true);

        std::cout << gflags::SetCommandLineOption("port", "999") << std::endl;

        std::cout << "Got '" << FLAGS_host << ":" << FLAGS_port << "'" << std::endl;
        return 0;
}
```

可以通过 `g++ main.cc -std=c++11 -o gflags -lgflags -lpthread` 命令编译。

## 其它

### 常用参数

gflags 中包含了几类默认的参数。

```
--help	       显示所有文件的所有flag，按文件、名称排序，显示flag名、默认值和帮助
--helpfull	   和 --help 相同，显示全部flag
--helpshort	   只显示执行文件中包含的flag，通常是 main() 所在文件
--helpxml	   类似 --help，但输出为xml
--helpon=FILE  只显示定义在 FILE.* 中得flag
--helpmatch=S  只显示定义在 *S*.* 中的flag
--helppackage  显示和 main() 在相同目录的文件中的flag
--version	   打印执行文件的版本信息

--undefok=flagname,flagname,...  后面列出的flag名，可以在无定义的情况下忽略而不报错

--fromenv --tryfromenv   从环境变量中引入
--flagfile               从文件中引入
```

### 定制版本和帮助信息

通过 `--version` 和 `--help` 默认会输出其对应的版本和帮助信息，也可以通过 `SetVersionString()` 设置版本信息，通过 `SetUsageMessage()` 设置帮助的开始软件信息 (帮助信息无法覆盖)。

``` cpp
gflags::SetVersionString("1.1.0");
gflags::SetUsageMessage("./gflags");
```

注意，参数的设置需要在调用 `ParseCommandLineFlags()` 之前。

### CMake 使用

最新版本的 gflags 已经可以支持 CMake 了，如上安装时所示，在安装时会在 `/usr/lib/cmake/gflags/` 目录下安装相关的文件，那么在项目中可以通过如下方式使用。

```
FIND_PACKAGE(gflags REQUIRED)
INCLUDE_DIRECTORIES(${gflags_INCLUDE_DIR})

ADD_EXECUTABLE(foo main.cc)
TARGET_LINK_LIBRARIES(foo gflags)
```

如果是将 CMake 安装到了其它路径下，那么可以将上述的文件复制到 CMake 的模块路径下再使用。

### 整型选择

在 `gflags/gflags_declare.h` 中定义了 `int32` `int64` 等类型，可以直接使用，不过编译时需要添加 `-std=c++11` 参数，否则在使用 `cstdint` 头文件时会报错。

### 其它

可以通过 `void GetAllFlags(std::vector<CommandLineFlagInfo>* OUTPUT)` 接口遍历所有的参数，更多接口可以查看 `gflags/gflags.h` 头文件。

## 参考

* 详细可以查看官方文档 [How To Use gflags](https://gflags.github.io/gflags/) 中的介绍。

{% highlight text %}
{% endhighlight %}
