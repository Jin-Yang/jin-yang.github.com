---
title: Agent 通用库
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

## MiniAgent

这里实际上是一个通用的示例模版。

### 常用脚本

简单列举常用的脚本，一般保存在 `contrib` 目录下。

#### generate_version.sh

依次从 `VERSION` 文件、`Git-Tag` 中读取，其命名格式为 `v1.2.3` ，默认是 `v0.1.0` 。

#### package.sh

将源码打包成 `tar.bz2` 包，同时将源码编译并打包成 RPM 安装包。

{% highlight text %}
./contrib/package.sh MiniAgent 1.2.1-1
{% endhighlight %}

如上的示例中，其中 `1.2.1` 为版本号，而 `1` 表示编译号，

该脚本同时会调用 `generate_version.sh` 脚本判断版本号是否匹配，不匹配则直接报错，可以通过 `-f` 跳过。

## 测试用例

其中单元测试使用的是 CMake 提供的机制，另外，针对。

## 常用库

### liblog

一个非常简单的日志库，支持简单的日志打印、级别设置、日志切割等功能，同时针对线程、进程编程进行了简单的优化。

为了尽量简单，并没有支持灵活的日志格式，但是如果需要调整为自己喜欢的格式，可以直接修改 `common.c` 文件中与日志格式化相关程序，其中 GCC 中可以使用的宏包括了 `__FILE__` `__FUNCTION__` `__LINE__` 等。

注意，在 CMake 中使用 `__FILE__` 时默认是文件的全路经，如果要使用相对路径可以参考 [CMake 自动编译]({{ site.production_url }}/post/linux-cmake-auto-compile-introduce.html) 中相关介绍。

#### TODO

* 日志压缩。

### libcron

类似于系统中的 crontab 的周期定义方式。

{% highlight text %}
 .------------------ second       (0~59) , - * /
 |  .--------------- minute       (0~59) , - * /
 |  |  .------------ hour         (0~23) , - * /
 |  |  |  .--------- day of month (1~31) , - * / ?
 |  |  |  |  .------ month        (1~12) , - * / jan,feb,mar,apr ...
 |  |  |  |  |  .--- day of week  (0~7) (Sunday=0/7) , - * / ? sun,mon,tue,wed,thu,fri,sat
 |  |  |  |  |  |
 *  *  *  *  *  * user-name  command to be executed
 0  1  2  3  4  5
{% endhighlight %}

其中 crontab 默认精确到分钟，这里的精度是秒，包含了 6 个通过空格分开的字段，其表示的时间段如上所示。

* `*` 任意值，如果秒域为 `*` 表示任一秒都会触发；
* `,` 列举，如果秒域为 `5,13,20` 则表示会在第 5 秒、第 13 秒、第 20 秒会触发；
* `-` 范围，如果秒域为 `5-9` 表示从第 5 秒到第 9 秒都会触发，等价于 `5,6,7,8,9` ；
* `/` 间隔，如果秒域为 `30/13` 表示从第 6 秒开始每隔 13 秒触发一次，也就是 `39 52 39 52 ...` ；
* `?` 只能用于日期或者星期中，如果设置了其中的一个，另外一个需要设置为 `?` ；

其中 `?` 只能用在 `Day of Month` 和 `Day of Week` 两个域，它也匹配域的任意值，但实际不会，因为两者会相互影响。假设，想每月 20 号触发，而不关心具体是星期几，则可以使用 `0 0 0 20 * ?` ，最后必须是 `?` ，否则会直接报错。

注意，对于 `Day of Week` 字段来说，在代码中实际使用的范围是 `0~6` 。

<!--
Java提供了更复杂的功能
http://www.cnblogs.com/sawyerlsy/p/7208321.html
-->

#### 示例

{% highlight text %}
"0 0 * * * *"                 the top of every hour of every day
"*/10 * * * * *"              every ten seconds
"0 0 8-10 * * *"              8, 9 and 10 o'clock of every day
"0 0/30 8-10 * * *"           8:00, 8:30, 9:00, 9:30 and 10 o'clock every day
"0 0 9-17 * * MON-FRI"        on the hour nine-to-five weekdays
"0 0 0 25 12 ?"               every Christmas Day at midnight


*/10 * * * *     echo "Ten minutes ago." >> /tmp/foo.txt    // 每十分钟执行一次
0 6 * * *        echo "Good morning." >> /tmp/foo.txt       // 每天早上6点
0 */2 * * *      echo "Have a break now." >> /tmp/foo.txt   // 每两个小时
45 4 1,10,22 * * echo "Restart server." >> /tmp/foo.txt     // 每月1、10、22日的4:45
0 23-7/2,8 * * * echo "Have a good dream." >> /tmp/foo.txt  // 晚上11点到早上8点之间每两个小时，早上八点
0 11 4 * 1-3     echo "Just kidding." >> /tmp/foo.txt       // 每月4号和每周的周一到周三的早上11点
45 11 * * 0,6    echo "Have a good lunch." >> /tmp/foo.txt  // 每周六、周日的11点45分
0 9 * * 1-5      echo "Work hard." >> /tmp/foo.txt          // 从周一到周五的9点
2 8-16/3 * * *   echo "Some examples." >> /tmp/foo.txt      // 8:02、11:02、14:02执行

0,30 18-23 * * *    echo "Same." >> /tmp/foo.txt            // 每天18:00到23:00之间每隔30分钟
0-59/30 18-23 * * * echo "Same." >> /tmp/foo.txt            // 同上
*/30 18-23 * * *    echo "Same." >> /tmp/foo.txt            // 同上
{% endhighlight %}

在表达式中，每位表示一个时间，例如秒、分、时等。

#### Tips

如果要测试类似夏令时、冬令时这类的问题，可以通过设置环境变量的方式调整时区，详见 `man 3 timegm` 中的介绍。

需要测试的场景包括了：夏令时、冬令时、闰年。

### libdown

默认直接覆盖。


### PIDFile

这里简单检查 PIDFile 文件，以及 `/proc` 文件系统中的值，并没有使用 flock 机制。

{% highlight text %}
int pidfile_check(const char *file);
    检查PIDFile对应的进程是否存在，会读取/proc/PID目录，同时校验/proc/PID/comm值。
返回值：
     0 不存在
     1 进程存在
    -1 检查异常，包括各类的异常，例如文件打开异常

int pidfile_update(const char *file);
    更新PIDFile文件值，也就是将当前进程的PID写入到文件中。

void pidfile_destory(const char *file);
    删除PIDFile文件。
{% endhighlight %}

也就是说只校验命令，而不校验参数。


<!--
strtod异常测试，包括NaN +-Inf
libev + SSL 使用
https://segmentfault.com/a/1190000005998141

Semantic version library written in ANSI C
https://github.com/h2non/semver.c
Socket基本库的封装
https://github.com/jb55/anet.c
这里有很多小而美的库介绍
https://github.com/clibs/clib/wiki/Packages

原子写入
https://yq.aliyun.com/articles/160854
https://liam.page/2018/04/28/debug-in-Linux-kernel-jprobe/
https://blog.csdn.net/dog250/article/details/78879600
-->

{% highlight text %}
{% endhighlight %}
