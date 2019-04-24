---
title: C 代码覆盖率
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: c,coverage
description: 我们已经提供了一些测试用例，但是这些测试用例的好坏如何评估？是否已经覆盖了所有的函数？函数中的分支以及边界条件是否都已经覆盖？ 这就需要通过代码覆盖率进行查看，这里简单介绍其使用方法。
---

我们已经提供了一些测试用例，但是这些测试用例的好坏如何评估？是否已经覆盖了所有的函数？函数中的分支以及边界条件是否都已经覆盖？

这就需要通过代码覆盖率进行查看，这里简单介绍其使用方法。

<!-- more -->

## 简介

在 GCC 中，提供了一个代码覆盖率的分析工具 `gcov` ，除此之外，还可以使用 `lcov` `genhtml` 等工具生成最终的展示页面。

CentOS 中可以通过如下命令安装。

{% highlight text %}
# yum install --enablerepo=epel lcov
{% endhighlight %}


### 使用

在使用 `gcc` 或者 `g++` 后面添加参数 `--fprofile-arcs` `--ftest-coverage` ，必须同时在编译器和链接器上设置，在 `Makefile` 里面可以加在 `CFLAGS` 和 `LDFLAGS` 上。

编译完成之后，会生成相关的 `*.gcno` 文件，运行之后生成 `*.gcda` 文件，然后可以使用 `gcov *.c` 生成 `*.c.gcov` 代码覆盖信息，参数如下。

不过这个是文本的，可以将生成的文件 `main.c.gcov` 通过编辑器打开，会发现在代码前有很多的符号标记，简介如下：

* `#####` 没有被执行过。
* `-` 不计入统计。
* `N` 被调用执行的此时。

文件文件观察起来不太方便，可以通过 `lcov` 生成图形化的代码覆盖数据。

{% highlight text %}
$ lcov -d . -t 'Main Test' -o 'main_test.info' -b . -c
{% endhighlight %}

会生成一个 `main_test.info` 文件，接着可以通过 `gethtml` 生成 html 图形文件，

{% highlight text %}
$ genhtml -o main_test.html main_test.info
{% endhighlight %}

然后在浏览器中打开 index 文件即可。

## 示例

{% highlight c %}
/* main.c */
#include <stdio.h>
#include <stdlib.h>

void test(int count);

int main(int argc, char *argv[])
{
        int i = 10;

        if(argc == 2)
                i = atoi(argv[1]);
        printf("arg is %d\n", i);
        test(i);

        return 0;
}
{% endhighlight %}

{% highlight c %}
/* test.c */
#include <stdio.h>

void test(int count)
{
        int i;

        for (i = 1; i < count; i++) {
                if (i % 3 == 0)
                        printf ("%d is divisible by 3\n", i);
                if (i % 11 == 0)
                        printf ("%d is divisible by 11\n", i);
                if (i % 13 == 0)
                        printf ("%d is divisible by 13\n", i);
        }
}
{% endhighlight %}

{% highlight makefile %}
# Makefile
GCOV_FLAGS=-fprofile-arcs -ftest-coverage
CFLAGS+=-g $(GCOV_FLAGS)
#LDFLAGS+=$(GCOV_FLAGS)

target=main
all : $(target)

main : test.o main.o
        $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.c
        $(CC) -c $^ -o $@ $(CFLAGS) $(DEFINES)

.PHONY : clean
clean :
        rm -rf *.o
        rm -rf $(target)
        rm -rf  *.gcov *.gcda *.gcno
{% endhighlight %}

如果连接的时候出现 `undefined reference to '__gcov_init'` 错误，则还要加上 `-lgocv` 。

<!--
cmake -DWITH_CONVERAGE=ON -DWITH_UNIT_TESTS=ON ..
lcov --directory . --zerocounters
ctest
lcov --directory . --capture --output-file foobar.info
  -d/--directory 指定gcno gcda文件所在的目录
  -c/--capture 获取覆盖率信息
  -o/--output-file 指定输出文件名称

----- 生成html格式的覆盖率报告，保存到result目录下
$ genhtml -o result out.info
-->

## 其它

### 数据后处理

最常见的是一些三方库是不需要统计覆盖率信息，或者只需要统计某些文件的覆盖率，就需要对输出的结果进行筛选。

{% highlight text %}
----- 查看覆盖率统计信息
$ lcov --list foobar.info
----- 移除指定目录的覆盖率统计信息
$ lcov --remove foobar.info '/src/include/*' '/user/bin/*' -o foobar.info.cleaned
----- 只保留固定目录的统计信息
$ lcov --extract foobar.info '*/src/*' '*/lib/*' -o foobar.info.reserved
{% endhighlight %}

注意，在 CMake 中上述使用的是绝对路径，这也就意味着，如果需要删除测试用例的覆盖，则应该使用 `*/tests/*` 。

### CMake

详细的可以参考示例，通过如下方式编译。

{% highlight text %}
$ cmake .. -DWITH_UNIT_TESTS=ON -DWITH_CONVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
{% endhighlight %}

然后提供一个简单的 HTTP 服务器。

{% highlight text %}
$ python -m SimpleHTTPServer 8080
{% endhighlight %}

上述的命令无法指定静态文件的目录，所以需要先切换到相应的目录，然后在执行上述的命令。最后访问 [http://localhost:8080](http://localhost:8080) 即可以看到报告。

![cmake coverage]({{ site.url }}/images/programs/lcov-coverage-example.png "cmake coverage"){: .pull-center width="90%" }


<!--
lcov --remove foobar.info '*/libmock/*' '*/tests/*' -o foobar.info.cleaned
lcov --list foobar.info.cleaned
-->

<!--
http://blog.lucode.net/backend-development/c-unittest-framework-cmocka.html
https://blog.microjoe.org/2017/unit-tests-c-cmocka-coverage-cmake.html
-->


{% highlight text %}
{% endhighlight %}
