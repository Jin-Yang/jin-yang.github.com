---
title: CMake 自动编译
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,automake,configure,cmake
description: 在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。这里简单介绍下这种构建方式的细节，以及开发者如何利用 autotools 创建兼容 GNU 构建系统的项目。
---

在 Linux 中，经常使用 GNU 构建系统，也就是利用脚本和 make 程序在特定平台上构建软件，这种方式几乎成为一种习惯，被广泛使用。

这里简单介绍下 CMake 的使用。

<!-- more -->

## 简介

在介绍示例之前，先说明一下 CMake 有两种编译方式：内部构建和外部构建。内部构建直接在源码目录下执行 `cmake .`，外部构建则会在一个目录下构建，不会影响原源码的结构。

其基本结构可以简单描述为：

1. 依赖 `CMakeLists.txt` 文件，项目主目标一个，主目录中可指定包含的子目录；
2. 在项目 `CMakeLists.txt` 中使用 `PROJECT` 指定项目名称，通过 `ADD_SUBDIRECTORY` 添加子目录；
3. 子目录 `CMakeLists.txt` 将从父目录 `CMakeLists.txt` 继承设置。

另外，上述通过 `PROJECT` 设置好工程后，可以通过 `${hello_SOURCE_DIR}` 引用，注意大小写。

因为 `CMakeLists.txt` 可执行脚本并通过脚本生成一些临时文件，因此 CMake 无法跟踪到底产生了那些临时文件，因此，没有办法提供一个可靠的 `make distclean` 方案，为此可以使用外部编译。

另外，也没提供 `make uninstall` 命令，卸载可以通过 `cat install_manifest.txt | sudo xargs rm`  命令执行删除或卸载。

## 简单示例

最简单示例单文件输出 `Hello World!`，只需创建两个文件：`main.c` 和 `CMakeLists.txt`；然后通过 `cmake . && make` 进行编译，要查看详细信息可以 `make VERBOSE=1` 或者 `VERBOSE=1 make` 。

{% highlight text %}
$ cat CMakeLists.txt
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(hello)                                              # 项目名称
SET(SRC_LIST main.c)                                        # 添加源码，可忽略
MESSAGE(STATUS "This is BINARY dir " ${HELLO_BINARY_DIR})   # 打印信息
MESSAGE(STATUS "This is SOURCE dir " ${HELLO_SOURCE_DIR})
ADD_EXECUTABLE(hello ${SRC_LIST})

$ cat main.c
#include <stdio.h>
int main()
{
    printf("Hello World!\n");
    return 0;
}
{% endhighlight %}

在上述 `MESSAGE()` 中，会打印变量用于调试，该变量是通过 `PROJECT()` 默认设定的变量，详见 `PROJECT()` 的使用。

调试的话可以使用 `cmake . -DCMAKE_BUILD_TYPE=Debug` ，不过此时使用的绝对地址。

## 配置文件

通常 `autotools` 会根据平台动态生成 `config.h` 文件，在 CMake 中同样可以生成。

除了支持脚本外，还包括了一些常见的模块 (Modules)，例如 CentOS 中保存在 `/usr/share/cmake/Modules/` 目录下，在使用时需要先通过 `INCLUDE()` 指令包含相应的模块。

{% highlight text %}
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
INCLUDE (CheckIncludeFiles)

CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES ("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
{% endhighlight %}

在执行时，会将结果保存在 `CMakeCache.txt` 文件中，类似如下；如果要重新生成则需要删除。

{% highlight text %}
//Have include HAVE_MALLOC_H
HAVE_MALLOC_H:INTERNAL=1
{% endhighlight %}

在 `config.h.in` 中添加如下内容。

{% highlight text %}
#cmakedefine HAVE_MALLOC_H 1
#cmakedefine HAVE_SYS_MOUNT_H
{% endhighlight %}

在检测后，会将 `#cmakedefine` 替换掉，如下。

{% highlight text %}
#define HAVE_MALLOC_H 1
#define HAVE_SYS_MOUNT_H

/* #undef HAVE_MALLOC_H 1 */
/* #define HAVE_SYS_MOUNT_H */
{% endhighlight %}

详细内容可以参考 [CMake:How To Write Platform Checks](https://itk.org/Wiki/CMake:How_To_Write_Platform_Checks) 。

### 常见示例

<!--
CHECK_FUNCTION_EXISTS(backtrace HAVE_BACKTRACE)
CHECK_LIBRARY_EXISTS(pthread pthread_setname_np "" HAVE_PTHREAD_SETNAME_NP)
-->

#### 头文件检查

{% highlight text %}
CHECK_INCLUDE_FILE("regex.h" HAVE_REGEX_H)
CHECK_INCLUDE_FILES("sys/prctl.h;sys/others.h" HAVE_SYS_PRCTL_H)
{% endhighlight %}

其中前者只能检查一个，后者可以检查多个头文件。

#### 函数检查

{% highlight text %}
CHECK_FUNCTION_EXISTS(backtrace HAVE_BACKTRACE)
{% endhighlight %}

其中 `CHECK_FUNCTION_EXISTS()` 仅能判断在连接时能找到的函数，一般也就是标准头文件、标准库中的函数，如果是非标准的可以通过 `CHECK_SYMBOL_EXISTS()` 检查。

#### 变量、宏检查

{% highlight text %}
CHECK_SYMBOL_EXISTS(ENAMETOOLONG errno.h HAVE_ENAMETOOLONG)
{% endhighlight %}

用来检查函数、变量或宏存在，其中文件列表可以是 `"foo.h;bar.h"` 这类的方式，注意无法识别类型或者枚举，此时可以考虑使用 `CheckTypeSize()` 或 `CheckCSourceCompiles()` 。

其中 `CMakeLists.txt` 示例文件如下。

{% highlight text %}
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(hello)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILE("regex.h" HAVE_REGEX_H)

INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(backtrace HAVE_BACKTRACE)

INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(pthread pthread_setname_np "" HAVE_PTHREAD_SETNAME_NP)

INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(ENAMETOOLONG errno.h HAVE_ENAMETOOLONG)

CONFIGURE_FILE(config.h.in config.h @ONLY)
{% endhighlight %}

其中 `config.h.in` 文件内容如下。

{% highlight text %}
#ifndef CONFIG_H_
#define CONFIG_H_

#cmakedefine HAVE_REGEX_H            1
#cmakedefine HAVE_BACKTRACE          1
#cmakedefine HAVE_PTHREAD_SETNAME_NP 1
#cmakedefine HAVE_ENAMETOOLONG       1

#endif
{% endhighlight %}

### 安装

CMake 默认会在与源码目录相同的路径下生成二进制文件或者库文件，实际上可以通过如下方式进行设置。

{% highlight text %}
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
{% endhighlight %}

当然，对于单个可执行文件，也可以通过 `set_target_properties()` 指令设置如下的变量覆盖全局的参数。

{% highlight text %}
RUNTIME_OUTPUT_DIRECTORY
LIBRARY_OUTPUT_DIRECTORY
ARCHIVE_OUTPUT_DIRECTORY
{% endhighlight %}

#### 安装指令

在编译完成之后，可以通过 `make install` 进行安装，通过 `cat install_manifest.txt | xargs rm` 删除即可，不过需要提前指定安装规则。

{% highlight text %}
INSTALL(TARGETS foobar DESTINATION bin)
INSTALL(FILES foobar.h DESTINATION include)
{% endhighlight %}

如上的配置会将 `foobar` 和 `foobar.h` 安装到 `/usr/local/{bin,include}` 目录下即可，实际上对于安装目录而言，也可以直接使用 `INCLUDE(GNUInstallDirs)` 指定的目录。

对于 `TARGETS` 的配置，主要有三个参数，分别为 `ARCHIVE`、`LIBRARY`、`RUNTIME` ，一般会类似如下的方式编写。

{% highlight text %}
INSTALL(TARGETS targets...
    ARCHIVE
        DESTINATION     <dir>
        PERMISSIONS     permissions...
        CONFIGURATIONS  [Debug|Release|...]
        COMPONENT       <component>
    RUNTIME
        DESTINATION     <dir>
        PERMISSIONS     permissions...
        CONFIGURATIONS  [Debug|Release|...]
        COMPONENT       <component>
    PUBLIC_HEADER
        DESTINATION     <dir>
        PERMISSIONS     permissions...
        CONFIGURATIONS  [Debug|Release|...]
        COMPONENT       <component>
)
{% endhighlight %}

其中上述的 `targets` 可以指定多个，而且可以是不同的类型，如二进制、动态库、静态库、头文件等。

上述指定的 `DESTINATION` 一般是相对路径，可以通过 `CMAKE_INSTALL_PREFIX` 指定其前缀，对于 Linux 默认是 `/usr/local/`；另外，其前面还可以指定 `DESTDIR` 目录。

除了使用 `TARGETS` 外，还可以使用 `FILES` 或者 `DIRECTORY` ，对于 `DIRECTORY` 使用比较灵活，常见需要注意的关键点如下。

##### 后缀符号

需要注意其后缀的 `/` 符号。

{% highlight text %}
#----- 会将目录复制成为 dst/src/{subdirs and files...}
install(DIRECTORY   myproj/src DESTINATION dst)

#----- 会将目录复制成为 dst/{subdirs and files...}
install(DIRECTORY   myproj/src/ DESTINATION dst)
{% endhighlight %}

##### 文件过滤

可以通过参数 `FILES_MATCHING` 用于指定操作档案的条件，可以使用 `PATTERN` 或 `REGEX` 两种匹配方式，要注意 `PATTERN` 会比对全路径而不只是文件名。

{% highlight text %}
INSTALL(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")
{% endhighlight %}

以上会把 `src/` 底下所有文件后缀为 `.h` 的文件复制到 `include` 文件夹下，并且会保留原本目录树的结构。

另外，还可以在匹配条件后面通过 `EXCLUDE` 排除符合条件的文件或目录。

{% highlight text %}
INSTALL(DIRECTORY myapp/ mylib DESTINATION myproj PATTERN ".git" EXCLUDE)
{% endhighlight %}

### 打包

可以通过 CPack 进行打包，也就是将编译后的二进制进行打包，当然也可以打包源码，分别通过如下命令进行打包。

{% highlight text %}
make package_source
make package
{% endhighlight %}

### 宏和函数

同大多数脚本语言一样，CMake 中也有宏和函数的概念，关键字分别为 `macro` 和 `function`，具体用法如下：

{% highlight text %}
MACRO( [arg1 [arg2 [arg3 ...]]])
	 COMMAND1(ARGS ...)
	 COMMAND2(ARGS ...)
	 ...
ENDMACRO()

FUNCTION( [arg1 [arg2 [arg3 ...]]])
	 COMMAND1(ARGS ...)
	 COMMAND2(ARGS ...)
	 ...
ENDFUNCTION()
{% endhighlight %}

如下是一个求和的宏定义。

{% highlight text %}
macro(sum outvar)
	set(_args ${ARGN})
	list(LENGTH _args argLength)
	if(NOT argLength LESS 4)　# 限制不能超过4个数字
		message(FATAL_ERROR "to much args!")
	endif()
	set(result 0)

	foreach(_var ${ARGN})
		math(EXPR result "${result}+${_var}")
	endforeach()

	set(${outvar} ${result})
endmacro()

sum(addResult 1 2 3 4 5)
message("Result is :${addResult}")
{% endhighlight %}

`${ARGN}` 是 CMake 中的一个变量，指代宏中传入的多余参数；上述宏只定义了一个参数 `outvar`，其余需要求和的数字都是不定形式传入的，所以需要先将多余的参数传入一个单独的变量中。

函数与宏的区别是，函数中的变量是局部的，不能直接传出。


## 常用示例

### 内置变量

如下是设置 C 编译器的参数，对于 CPP 则将 C 替换为 CXX 即可。

{% highlight text %}
set(CMAKE_C_COMPILER      "gcc" )               # 显示指定使用的编译器
set(CMAKE_C_FLAGS         "-std=c99 -Wall")     # 设置编译选项，也可以通过add_definitions添加编译选项
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g" )            # 调试包不优化
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG " )     # release包优化
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-sign-compare")   # 忽略某些告警
{% endhighlight %}

内置变量可在 cmake 命令中使用，如 `cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug` 。

<!--
EXECUTABLE_OUTPUT_PATH：可执行文件的存放路径
LIBRARY_OUTPUT_PATH：库文件路径
CMAKE_BUILD_TYPE:：build 类型(Debug, Release, ...)，
BUILD_SHARED_LIBS：Switch between shared and static libraries
-->


### Build Type

除了上述 `CMAKE_C_FLAGS_DEBUG` 指定不同类型的编译选项外，还可以通过如下方式指定。

{% highlight text %}
set(CMAKE_BUILD_TYPE Debug CACHE STRING "set build type to debug")
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
	SET(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    set(LIBRARIES Irrlicht_S.lib)
else()
    set(LIBRARIES Irrlicht.lib)
endif()
{% endhighlight %}

可以在命令行中通过如下方式编译 `cmake -DCMAKE_BUILD_TYPE=Debug ..`，最终的编译选项可以查看 `CMakeFiles/SRCFILE.dir/flags.make` 中的 `C_FLAGS` 选项，一般是 `CMAKE_C_FLAGS+CMAKE_C_FLAGS_MODE` 。

### 测试用例

可以通过如下方式添加测试用例。

{% highlight text %}
option(WITH_UNIT_TESTS "Compile with unit tests" OFF)

# Setup testing
IF(WITH_UNIT_TESTS)
        ENABLE_TESTING()
        ADD_SUBDIRECTORY(test)
ENDIF()

# test/CMakeLists.txt
FILE(GLOB SRCS *.c)
ADD_EXECUTABLE(testfoo ${SRCS})
TARGET_LINK_LIBRARIES(testfoo libs)
ADD_TEST(
    NAME testfoo
    COMMAND testfoo your arguments
)
{% endhighlight %}

然后在编译时通过 `cmake .. -DWITH_UNIT_TESTS=ON` 执行，并通过 `make test` 进行测试，实际等价于在 `build` 目录下运行 `ctest` 命令。

另外，可以指定测试的参数，以及输出的匹配。

{% highlight text %}
ADD_TEST(test foobar 10 5)
SET_TESTS_PROPERTIES(test PROPERTIES PASS_REGULAR_EXPRESSION "ok")
{% endhighlight %}


<!---
# 测试 1 + 3 , 是否输出打印'is 4'
add_test(test_1_plus_3 my_test.elf 1 3)
set_tests_properties (test_1_plus_3
PROPERTIES PASS_REGULAR_EXPRESSION "is 4")  #
-->

对于 valgrind，如果输出是 reachable 类型，那么实际上是依赖于 OS 的内存回收，此时及时已经配置了 `--error-exitcode=1` 也不会返回 0 ，尤其是对于全局变量。

为此，对于 CMake 而言，就需要配置 `FAIL_REGULAR_EXPRESSION "reachable"`，同时对于其它的内存泄露也会报错。

### 文件渲染

可以通过如下脚本，动态生成。

{% highlight text %}
$ cat CMakeLists.txt
CONFIGURE_FILE(
    "${PROJECT_SOURCE_DIR}/gcc_debug_fix.sh.in"
    "${PROJECT_BINARY_DIR}/gcc_debug_fix.sh"
    @ONLY)
... ...
ADD_EXECUTABLE (my_exe ...)
... ...
SET_TARGET_PROPERTIES(my_exe PROPERTIES RULE_LAUNCH_COMPILE "${PROJECT_BINARY_DIR}/gcc_debug_fix.sh")

$ cat gcc_debug_fix.sh.in
#!/bin/sh
PROJECT_BINARY_DIR="@PROJECT_BINARY_DIR@"
PROJECT_SOURCE_DIR="@PROJECT_SOURCE_DIR@"

# shell script invoked with the following arguments
# $(CXX) $(CXX_DEFINES) $(CXX_FLAGS) -o OBJECT_FILE -c SOURCE_FILE

# extract parameters
SOURCE_FILE="${@: -1:1}"
OBJECT_FILE="${@: -3:1}"
COMPILER_AND_FLAGS=${@:1:$#-4}

# make source file path relative to project source dir
SOURCE_FILE_RELATIVE="${SOURCE_FILE:${#PROJECT_SOURCE_DIR} + 1}"

# make object file path absolute
OBJECT_FILE_ABSOLUTE="$PROJECT_BINARY_DIR/$OBJECT_FILE"

cd "$PROJECT_SOURCE_DIR"

# invoke compiler
exec $COMPILER_AND_FLAGS -c "${SOURCE_FILE_RELATIVE}" -o "${OBJECT_FILE_ABSOLUTE}"
{% endhighlight %}

通过 `CONFIGURE_FILE()` 将文件中的变量替换，然后在 `set_target_properties()` 设置，将在编译之前将绝对路径转换为相对路径，注意 `gcc_debug_fix.sh.in` 需要为可以执行文件。

### 库参数

在编译不同的库时，可能需要使用不同的编译参数，那么此时就可以使用如下的方式修改。


{% highlight text %}
ADD_LIBRARY(unity unity/src/unity.c)

# Disable -Werror for Unity
IF (FLAG_SUPPORTED_Werror)
	IF ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")
    ELSE()
        TARGET_COMPILE_OPTIONS(unity PRIVATE "-Wno-error")
    ENDIF()
ENDIF()
# Disable -fvisibility=hidden for Unity
IF (FLAG_SUPPORTED_fvisibilityhidden)
    IF ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=default")
    ELSE()
        TARGET_COMPILE_OPTIONS(unity PRIVATE "-fvisibility=default")
    ENDIF()
ENDIF()
# Disable -fsanitize=float-divide-by-zero for Unity
#     (GCC bug on x86 https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80097)
IF (FLAG_SUPPORTED_fsanitizefloatdividebyzero AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    IF ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-sanitize=float-divide-by-zero")
    ELSE()
        TARGET_COMPILE_OPTIONS(unity PRIVATE "-fno-sanitize=float-divide-by-zero")
    ENDIF()
ENDIF()
{% endhighlight %}

### 添加 flex bison

默认是不支持 flex 和 bison 的，可以通过如下方式添加。


{% highlight text %}
#----- 生成的中间文件保存在${CMAKE_CURRENT_BINARY_DIR}目录下
include_directories(${CMAKE_CURRENT_BINARY_DIR} )

#----- 定义源码目录以及中间文件
SET(PARSER_DIR ${CMAKE_SOURCE_DIR}/driver/mysql)
SET(GEN_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
	${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
	${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
	${CMAKE_CURRENT_BINARY_DIR}/parser.l.h
)

#----- 添加一个custom_target用于构建依赖，该target依赖${GEN_SOURCES}指向的文件
ADD_CUSTOM_TARGET(
        GenServerSource
        DEPENDS ${GEN_SOURCES}
)

#----- 告知${GEN_SOURCES}指向的文件是编译过程中生成的，以避免执行cmake命令的时候报文件找不到
SET_SOURCE_FILES_PROPERTIES(${GEN_SOURCES} GENERATED)

#----- 需要注意的是OUTPUTS一定要和${GEN_SOURCES}中文件一致，并且target设置对否则无法确保执行顺序
ADD_CUSTOM_COMMAND(
	SOURCE ${PARSER_DIR}/parser.y
	COMMAND bison -d ${PARSER_DIR}/parser.y
	TARGET GenServerSource
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
	WORKING_DIRECTORY ${PARSER_DIR}
)
ADD_CUSTOM_COMMAND(
	SOURCE ${PARSER_DIR}/parser.l
	COMMAND flex  ${PARSER_DIR}/parser.l
	TARGET GenServerSource
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c ${CMAKE_CURRENT_BINARY_DIR}/parser.l.h
	WORKING_DIRECTORY ${PARSER_DIR}
)

#----- 这里将flex和bison生成的.c文件编译到程序的动态库中，需要注意指定路径
add_library(driver SHARED
	sourcefile1.cc sourcefile2.cc sourcefile3.cc
	${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
	${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
)

#----- 指明动态库driver所需要的依赖，所以可以保证cmake会先生成GenServerSource, 而GenServerSource又
#      依赖${GEN_SOURCES}指明的文件， 而这些文件又是由两个ADD_CUSTOM_COMMAND命令来生成的
#      (outputs中给出的)，所以会先执行两个ADD_CUSTOM_COMMAND命令
ADD_DEPENDENCIES(driver GenServerSource)
{% endhighlight %}

### 库使用

关于库的使用常见有如下的操作。

{% highlight text %}
----- 增加库的搜索路径
LINK_DIRECTORIES(./lib)

----- 生成库，可以是动态(SHARED)或者静态库(STATIC)
ADD_LIBRARY(hello SHARED ${SRC_LIST})

----- 指定生成对象时依赖的库
TARGET_LINK_LIBRARIES(hello A B.a C.so)

----- 自定义链接选项，单独对B.a使用--whole-archive选项
TARGET_LINK_LIBRARYIES(hello A -Wl,--whole-archive B.a -Wl,--no-whole-archive C.so)
{% endhighlight %}

在使用 `add_library(foo SHARED foo.c)` 时，不同平台输出有所区别，例如，`foo.dll(Windows)`、 `libfoo.so(Linux)`、 `libfoo.dylib(Mac)` 。

比如编写用于 lua 扩展的 C 模块，那么在进行 require 时，需要执行如下调用：

{% highlight lua %}
require 'libfoo'     --默认加载libfoo.[so|dll]，并且执行luaopen_libluafoo
require 'foo'        --加载foo.so,并且执行luaopen_luafoo
{% endhighlight %}

并且各个平台下各不相同，可以通过如下方式修改前缀以及后缀：

{% highlight text %}
SET_TARGET_PROPERTIES(foo PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(foo PROPERTIES SUFFIX "so")
{% endhighlight %}


<!--
 对了，吐槽一下mac的rpath也是件麻烦的事情


# enable @rpath in the install name for any shared library being built
set(CMAKE_MACOSX_RPATH 1)

# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
set_target_properties(bar PROPERTIES INSTALL_RPATH "@loader_path/../lib")
-->

### 文件路径

CMake 在编译时会使用到绝对路径，而在打印日志时可能会导致文件长度过大，有几种方式修改。

{% highlight c %}
#include <string.h>
#define __FILENAME__                             \
        (strrchr(__FILE__, '/') ?                \
	strrchr(__FILE__, '/') + 1 : __FILE__)
{% endhighlight %}

不过这种方式会在执行时计算，也可以使用如下宏定义，此时会使用相对路径。

{% highlight text %}
SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
{% endhighlight %}

或者只使用文件名。

{% highlight text %}
SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(notdir $(abspath $<))\"'")
{% endhighlight %}

### 常用命令

指令是大小写无关的，参数和变量是大小写相关的，参数使用空格或者分号隔开。如 `MESSAGE (STATUS "This is BINARY dir" ${HELLO_BINARY_DIR})` 和 `MESSAGE (STATUS "This is BINARY dir ${HELLO_BINARY_DIR}")` 相同。

变量使用 `${}` 方式取值，但是在 `IF` 控制语句中是直接使用变量名。

{% highlight text %}
PROJECT (projectname [CXX] [C] [Java])
  定义工程名称以及语言，会隐式定义 projectname_BINARY_DIR、projectname_SOURCE_DIR变量，
  同时也会定义 PROJECT_BINARY_DIR、PROJECT_SOURCE_DIR 与前两者相同，建议使用后者。

SET(VAR [VALUE] [CACHE TYPE DOCSTRING [FORCE]])
  可以用来显式的定义变量。

MESSAGE([SEND_ERROR | STATUS | FATAL_ERROR] "message to display" ...)
  向终端输出用户定义的信息，SEND_ERROR(产生错误，生成过程被跳过)，SATUS(输出前缀为-的信息)、
  FATAL_ERROR(立即终止所有cmake过程)。

ADD_DEFINITIONS(-DMICRO_1 ...)
  添加宏定义。

ADD_EXECUTABLE(hello ${SRC_LIST})
  该工程会生成一个文件名为 hello 的可执行文件，相关的源文件是 SRC_LIST 。

ADD_SUBDIRECTORY(source_dir [binary_dir] [EXCLUDE_FROM_ALL])
  指定当前工程的

CONFIGURE_FILE(intput output [COPYONLY] [ESCAPE_QUOTES] [@ONLY])
  将文件 input 拷贝到 output 然后替换文件内容中引用到的变量值；如果用相对路径，则 input 相对
  的是当前源码路径，output 相对于二进制文件路径。该命令替换掉在输入文件中，以 ${VAR} 格式或
  @VAR@ 格式引用的任意变量，如同它们的值是由CMake确定的一样。 如果一个变量还未定义，它会被替
  换为空。如果指定了COPYONLY选项，那么变量就不会展开。如果指定了ESCAPE_QUOTES选项，那么所有被
  替换的变量将会按照C语言的规则被转义。该文件将会以CMake变量的当前值被配置。

----- 查看当前支持的选项
cmake .. -LH
{% endhighlight %}

## 参考

关于 CMake 相关的文档可以参考 [CMake 实践](/reference/linux/CMake_Practice.pdf) ，或者参考 [CMake 入门实战](http://hahack.com/codes/cmake/) 以及 [CMake Tutorial](https://cmake.org/cmake-tutorial/)；搭建 GTest 环境可以参考 [Unit testing with GoogleTest and CMake](http://kaizou.org/2014/11/gtest-cmake/) 。

常见的命令参考 [cmake-commands](https://cmake.org/cmake/help/v3.0/manual/cmake-commands.7.html) 。


<!--
The new guide of the conversation in Autoconf and Make
http://www.edwardrosten.com/code/autoconf/

A tutorial for porting to autoconf & automake
http://mij.oltrelinux.com/devel/autoconf-automake/

运用Autoconf和Automake生成Makefile的学习之路
http://www.cnblogs.com/ericdream/archive/2011/12/09/2282359.html

如何根据configure.ac和Makefile.am为开源代码产生当前平台的Makefile
http://www.51cos.com/?p=1649

Colletcd的编译系统介绍
https://collectd.org/wiki/index.php/Build_system

AC_CHECK_FUNCS
AC_CHECK_LIB
https://autotools.io/autoconf/finding.html
概念：GNU构建系统和Autotool
http://www.pchou.info/linux/2016/09/16/gnu-build-system-1.html

CMake如何查找链接库
http://www.yeolar.com/note/2014/12/16/cmake-how-to-find-libraries/


http://huqunxing.site/2016/08/26/CMake%E5%85%A5%E9%97%A8%E6%8C%87%E5%8D%97/#编译32位和64位程序

CPACK_GENERATOR RPM TBZ2 ZIP

cpack -D CPACK_GENERATOR="ZIP;TGZ" /path/to/build/tree

 AC_PLUGIN([cpu],                 [$plugin_cpu],             [CPU usage statistics])


http://cmake.3232098.n2.nabble.com/Adding-a-custom-line-to-CMake-s-makefiles-td5984979.html


%defattr(file mode, user, group, dir mode)

默认的是 `%defattr(644, user, group, 755)` 或者 `%defattr(-, user, group)` ，对于普通文件是 644 而可执行文件以及目录使用 755 。
-->



{% highlight text %}
{% endhighlight %}
