---
title: Fuzzing 测试
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords:
description: Fuzzing 一般指模糊测试，是一种基于黑盒的测试技术，通过自动化生成并执行大量的随机测试用例来发现产品或协议的未知漏洞。同时，也作为了当前最为强大而有效的漏洞挖掘技术。
---

Fuzzing 一般指模糊测试，是一种基于黑盒的测试技术，通过自动化生成并执行大量的随机测试用例来发现产品或协议的未知漏洞。

同时，也作为了当前最为强大而有效的漏洞挖掘技术。

<!-- more -->

## AFL

American Fuzzy Lop, AFL 是一种开源的模糊测试器，由谷歌的 Michal Zalewski 开发。

可以在源码编译时添加，或者使用 QEMU 模式，也就是 QEMU-(User Mode) ，在执行时注入部分代码进行测试。

目前使用 AFL 有两种方式：

1. 开源软件，在编译的时候同时进行插桩；
2. 闭源软件，配合QEMU直接对闭源的二进制代码进行fuzz测试。

### 安装

直接从 [ALF Release](http://lcamtuf.coredump.cx/afl/releases/) 下载相关的版本，然后执行 `make` 即可。

默认安装在 `/usr/local` 相对目录下，可以通过 `PREFIX=/usr && make` 修改。

### 使用

<!--
ASAN 是GCC支持的一个性能，所以，在使用ALF对软件进行编译之前，只需要设置环境变量即可，指令如下：
export AFL_USE_ASAN=1
-->

如果程序使用 `autoconf` 工具链构建，在执行 `configure` 脚本时，添加如下参数即可。

{% highlight text %}
$ ./configure --disable-shared CC="afl-gcc" CXX="afl-g++"
{% endhighlight %}

其中 `--disable-shared` 选项可以允许通过 `LD_LIBRARY_PATH` 变量，让程序加载经过 AFL 插桩的 `.so` 文件，进行静态构建而不是动态链接。

如果是其它的 `Makefile` 文件，可以直接修改引用的编译器。

另外，为了后期更好的分析 crash 可以开启 Address Sanitizer, ASAN 检测工具，此工具可以更好的检测出缓存区溢出、UAF 等内存漏洞。

{% highlight text %}
$ AFL_USE_ASAN=1 ./configure CC=afl-gcc CXX=afl-g++ LD=afl-gcc--disable-shared
$ AFL_USE_ASAN=1 make
{% endhighlight %}

不使用 AFL 编译插桩时，可使用以下方式开启 Address Sanitizer。

{% highlight text %}
$ ./configure CC=gcc CXX=g++ CFLAGS="-g -fsanitize=address"
$ make
{% endhighlight %}

<!--
afl-fuzz -m 300 -i fuzz_in -o fuzz_out ./foobar -f

常见参数含义如下。

-m 分配的内存空间
-i 测试样本的路径
-o 输出结果的路径

-f参数表示：testcase的内容会作为afl_test的stdin
-t：设置程序运行超时值，单位为 ms
-M：运行主(Master) Fuzzer
-S：运行从属(Slave) Fuzzer
-->

### 输出解读

其中有几个比较关键的输出。

* `last new path` 报错需要进行修正；
* `cycles done`  变绿就说明后面即使继续fuzz，出现crash的几率也很低了
* `uniq crashes` 由于相同类型导致crash的数量

状态窗口中的 "cycles done" 字段，其颜色会由洋红色逐步变为黄色、蓝色、绿色，当为绿色的时候就很难有新发现了。

## 语料库

AFL 需要一些初始输入数据作为 Fuzzing 的起点，这些输入甚至可以是毫无意义的数据，AFL 可以通过启发式算法自动确定文件格式结构。

<!--
通过Hello字符串凭空生成了大量的JPGE图像
https://lcamtuf.blogspot.com/2014/11/pulling-jpegs-out-of-thin-air.html

* 有效输入。虽然无效的输入也可以，但是有效输入可以更快的找到更多的执行路径；
* 尽量小的体积。可以节省内存以及处理时间，建议小于 1KB ，可以参考 perf_tips.txt 。

https://www.freebuf.com/articles/system/191536.html
http://zeroyu.xyz/2019/05/15/how-to-use-afl-fuzz/
https://stfpeak.github.io/2017/06/11/Finding-bugs-using-AFL/
https://stfpeak.github.io/2017/06/12/AFL-Cautions/
https://paper.seebug.org/841/
-->

## 参考

* [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl/) AFL 的官方网站，包括了基本介绍、下载路径等。

<!--
使用QEMU进行测试
http://www.gandalf.site/2019/01/aflafl-qemufuzz.html
https://cool-y.github.io/2019/07/09/afl-first-try/

https://github.com/google/syzkaller
https://github.com/xxg1413/fuzzer/tree/master/iFuzz

http://bobao.360.cn/news/detail/3354.html
http://www.jianshu.com/p/015c471f5a9d
http://ele7enxxh.com/Use-AFL-For-Stagefright-Fuzzing-On-Linux.html
http://www.freebuf.com/articles/system/133210.html
http://www.hackdig.com/07/hack-24522.htm
总结
https://zhuanlan.zhihu.com/p/43432370

二进制分析方面主要利用技术包括：动态分析(Dynamic Analysis)、静态分析(Static Analysis)、符号化执行(Symbolic Execution)、Constraint Solving、资讯流追踪技术(Data Flow Tracking)以及自动化测试(Fuzz Testing)
-->

{% highlight text %}
{% endhighlight %}
