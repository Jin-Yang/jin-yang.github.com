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

这里实际上是一个通用的示例模版，可以通过如下方式进行编译测试。

{% highlight text %}
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DWITH_EXAMPLES=true -DWITH_UNIT_TESTS=true
{% endhighlight %}

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

这里的通用库采用 git 的 submodule 方式添加到各个项目中，大致的使用方式如下。

{% highlight text %}
----- 以submodule方式添加库
$ git submodule add git@gogs.cargo.com:cargo/clib-liblog.git libs/liblog

----- 对于所有的submodule拉取最新的代码
$ git submodule foreach --recursive git pull

----- 可以简单查看当前各个模块的状态
$ git submodule
{% endhighlight %}

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

### libprotocol

这里包含了一些基本的协议，以及常用的 Socket 操作。

#### 简介

一般 Socket 的处理流程分为了发送和接收。

发送流程：

1. 业务生成数据结构，发送到缓冲队列 (可以是队列、环、优先队列)，并通知 Socket 线程；Tips#1
2. 在 Socket 线程中，进行序列化操作，保存到统一的发送缓冲区中。Tips#3
3. 然后写入，此时可以通过异步 IO 进行处理；Tips#2

接收流程：

1. 接收到数据后保存到接收缓冲区，判断是否接收到一个完整的包；
2. 添加到接收链表中，然后通知工作线程进行处理；
3. 处理完成后，工作线程向写队列中添加响应报文。

Tips

1. 默认是在 Socket 线程中进行序列化，如果说性能不满足，那么可以将序列化在发送缓冲队列前进行处理，这时，只需要简单的发送缓冲队列的数据即可。
2. 这里可能会出现只发送了缓冲队列中一部分数据的情况，需要注意如何进行处理。
3. 这里的序列化采用的是一个相同的缓冲区，那么可以节省内存使用，而 Tips#1 中的使用方式，实际上是以内存换取效率。


<!--
IO 线程可以和工作线程绑定，也可以分开，主要看通讯的压力。

如果绑定那么对应的处理流程为：A) Read IO；B) 处理业务逻辑，获取结果；C) Write IO。

这种方式的处理流程简单，解析好请求后就能直接在同一线程中处理，省去了线程切换的开销，非常适合业务处理流程简单的请求。

但是，当单个任务的处理时间过长时，就可能会导致后面的任务被阻塞，影响请求的响应。

http://oserror.com/backend/libeasy-all/


当与服务端建立连接之后，建议只使用一个 Socket 连接，然后通过异步 IO 提高连接上的吞吐量，同时，如果需要也可以进行流量控制，这样会方便很多。

如果说吞吐量不够，一般是 Socket 的处理逻辑封装了很多业务逻辑，此时可以将业务逻辑拆出来，然后使用单独线程只做 Socket 收发。


缓存超过之后的策略：

1. 本地持久化，并周期重试。注意：本地持久化需要限制大小。
2. 业务失败，当前不可用。
-->

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

## 其它

### 子进程处理

在调用子进程时，需要注意如下。

1. 通过 `setpgrp()` 设置进程组，在超时或者异常发送信号时，向进程组发送信号。
2. 使用 `prctl(PR_SET_PDEATHSIG, SIGKILL);` 确保父进程退出的时候，子进程同样可以接收到信号。
3. 如果不支持 Clous On Exec ，那么需要在子进程中将相关的文件描述符关闭，注意，为了防止继承系统需要在系统启动时通过 ulimits 进行设置。

正常，如果父进程直接退出，那么子进程会变为僵尸进程，通过 `prctl()` 可以确保父进程退出时，向子进程发送指定的信号。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>

int main(void)
{
        int i = 0;
        pid_t pid;

        pid = fork();
        if (pid < 0) {
                fprintf(stderr, "fork failed, %d:%s.\n",
                                errno, strerror(errno));
                return -1;
        } else if (pid == 0) { /* child */
                prctl(PR_SET_PDEATHSIG, SIGKILL);
				/* system("sleep 10000") */
                while (1) {
                    printf("child running...\n");
                    sleep(1);
                }
        }

        for (i = 0; i < 6; i++) {
                printf("father running...\n");
                sleep(1);
        }

        printf("main exit()\n");
        return 0;
}
{% endhighlight %}

注意，这里只会涉及到子进程，对于孙子进程实际上是不生效的，例如上述的 `system("sleep 10000");` 实际上不会退出。


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



## dump 进程

假设有这么一个场景，有个进程卡死在了某个系统调用，例如 `futex` 接口处。

gcore [-o filename] pid


例如可以按照如下步骤操作。

$ gcore -o /tmp/foobar.core 8059
$ gdb -c /tmp/foobar.core.8059
>

查找cache目录下不是html的文件
find ./cache ! -name '*.html' -type f

根据文件属性查找：
find . -type f -name "*config*" ! -path "./tmp/*" ! -path "./scripts/*" ! -path "./node_modules/*"

https://fedoraproject.org/wiki/Packaging:Debuginfo

https://xiaohui-p.iteye.com/blog/1171927

如果当进程出问题之后如何更好的打印日志信息。

键入info variables以列出“所有全局和静态变量名称”。
键入info locals以列出“当前堆栈框架的局部变量”(名称和值)，包括该函数中的静态变量。
键入info args以列出“当前堆栈框架的参数”(名称和值)。

#define BUG() do {
	log_fatal("BUG: failure at %s:%d/%s()!", __FILENAME__, __LINE__, __func__);
		panic("BUG!");
} while(0)
#define BUG_ON(cond)  do { if (unlikely(cond)) BUG(); } while(0)

https://zhuanlan.zhihu.com/p/31630417
https://blog.csdn.net/littlefang/article/details/42295803
https://www.cnblogs.com/muahao/p/7610645.html
https://www.cnblogs.com/welhzh/p/4813778.html
https://www.linuxjournal.com/article/6391
https://blog.csdn.net/astrotycoon/article/details/8142588
http://velep.com/archives/1032.html
https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/
https://www.quora.com/How-do-you-get-a-stack-trace-on-Linux
https://github.com/gustafsson/backtrace
https://github.com/daddinuz/panic



https://paper.seebug.org/481/
https://paper.seebug.org/

Stack Overflow 作者，里面包含了很多架构相关的内容
https://nickcraver.com/
https://riboseyim.github.io/2016/07/17/OpenSource-StackOverflow/
https://zhuanlan.zhihu.com/p/22353191

检查内存中的敏感数据
https://github.com/rek7/mXtract

http://cwndmiao.github.io/2014/03/10/percpu/
http://cwndmiao.github.io/programming%20tools/2013/11/26/Dwarf/
死锁检测
https://github.com/rouming/dla
https://cloud.tencent.com/developer/article/1176832
https://yq.aliyun.com/articles/579470


https://github.com/antirez/visitors
https://github.com/antirez/lloogg
Btree
https://github.com/antirez/otree

用来替换readline and libedit的工具集
https://github.com/antirez/linenoise

一个很简单的文本编辑器
https://github.com/antirez/kilo

字符串压缩库
https://github.com/antirez/smaz

命令行中的备忘录
https://github.com/cheat/cheat

模糊搜索工具，一个交互式的命令行搜索
https://github.com/junegunn/fzf

可以将文件保存到照片中
https://github.com/DimitarPetrov/stegify

代码行统计工具，以及要快100倍的工具
http://cloc.sourceforge.net/
https://github.com/cgag/loc

磁盘使用情况检查
https://dev.yorhel.nl/ncdu

https://www.zhihu.com/question/59227720
命令行中的TODO支持Getting Things Done
https://calcurse.org/


很不错的终端工具
https://www.zhihu.com/question/59227720

-->

{% highlight text %}
{% endhighlight %}
