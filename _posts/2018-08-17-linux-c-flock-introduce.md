---
title: Linux C Flock 使用
layout: post
comments: true
language: chinese
category: [linux,program]
keywords: linux,program,c,flock
description: 在某些场景下，例如需要保证单个进程运行，通常的做法是生成一个 PID 文件，并将当前的进程 PID 写入，每次进程启动时检查文件以及进程是否存在。如果进程异常崩溃没有删除文件，而 Linux 中 PID 可以复用，那么就可能会导致误认为进程存在，虽然概率很低。其实在 Linux 中可以通过 flock 实现。
---

在某些场景下，例如需要保证单个进程运行，通常的做法是生成一个 PID 文件，并将当前的进程 PID 写入，每次进程启动时检查文件以及进程是否存在。

如果进程异常崩溃没有删除文件，而 Linux 中 PID 可以复用，那么就可能会导致误认为进程存在，虽然概率很低。

其实在 Linux 中可以通过 flock 实现。

<!-- more -->

## 简介

Linux 中与文件锁相关的函数有 `fcntl()` `lockf()` `flock()` 三个，其中 `lockf()` 是对 `fcntl()` 函数的封装，其底层实现是相同的。而 `flock()` 和 `fcntl()` 是两个不同的系统 API ，对应了不同的实现。

## Fcntl

其中 `fcntl()` 函数的声明如下。

{% highlight c %}
#include <fcntl.h>
#include <unistd.h>

int fcntl(int fd, int cmd, ... /* arg */ );
{% endhighlight %}

当使用建议锁时，其对应入参的结构体如下。

{% highlight c %}
struct flock {
	// ...
	short l_type;    /* Type of lock: F_RDLCK, F_WRLCK, F_UNLCK */
	short l_whence;  /* 与l_start配合，类似于seek决定从文件什么位置开始 SEEK_SET, SEEK_CUR, SEEK_END */
	off_t l_start;   /* 对文件加锁时的开始位置 */
	off_t l_len;     /* 加锁的长度，其中 0 表示整个文件 */
	pid_t l_pid;     /* PID of process blocking our lock (F_GETLK only) */
	// ...
};
{% endhighlight %}

简单来说，`fcntl()` 的功能很强大，既支持共享锁又支持排他锁，即可以锁住整个文件，又能只锁文件的某一部分。

### 参数

其中 `fnctl()` 函数的 cmd 可以是如下的三个参数：

* F_SETLK 在指定的字节范围获取锁 (F_RDLCK F_WRLCK) 或者释放锁 (F_UNLCK)，如果锁冲突则返回 -1 ，并将 errno 设置为 EACCES 或 EAGAIN 。
* F_SETLKW 行为等同 F_SETLK，但是当不能获取锁时会睡眠等待，如果在等待的过程中接收到信号，会立即返回并将 errno 置为 EINTR 。
* F_GETLK 获取文件锁信息。
* F_UNLCK 释放文件锁。

注意，为了设置读锁，文件必须以读的方式打开。为了设置写锁，文件必须以写的方式打开。为了设置读写锁，文件必须以读写的方式打开。

## Flock

在 Linux 中有个简单的实现，也就是 `flock()` ，这是一个建议性锁，不具备强制性。

也就是说，一个进程使用 flock 将文件锁住，另一个进程仍然可以操作正在被锁的文件，修改文件中的数据，这也就是所谓的建议性锁的内核处理策略。

{% highlight c %}
#include <sys/file.h>

int flock (intfd, int operation);
{% endhighlight %}

其中 `flock()` 主要有三种操作类型：

* `LOCK_SH` 共享锁，多个进程可以使用同一把锁，常被用作读共享锁；
* `LOCK_EX` 排他锁，同时只允许一个进程使用，常被用作写锁；
* `LOCK_UN` 释放锁。

默认的操作是阻塞，可以通过 `LOCK_NB` 设置为非阻塞。

### 脚本

另外，在命令行中，也可以通过类似如下的方式进行测试。

{% highlight text %}
$ flock -xn /tmp/foobar.lock -c "echo 'Hi world'"
{% endhighlight %}

常见的如 crontab 。


### 注意事项

在使用如下测试时，需要保证 `/tmp/foobar.txt` 存在。

#### 同一进程

在同一个进程中可以多次进行加锁而不会阻塞，可以通过如下方式进行测试。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>

int main(void)
{
        int rc, fd;

        fd = open("/tmp/foobar.txt", O_RDWR);
        printf("current fd: %d\n", fd);
        rc = flock(fd, LOCK_EX);
        printf("get lock rc: %d\n", rc);
        rc = flock(fd, LOCK_EX);
        printf("get lock again, rc: %d\n", rc);

	sleep(1000);

	return 0;
}
{% endhighlight %}

当启动第二个程序时就会被阻塞掉。

#### 文件描述符

`flock` 创建的锁是和文件打开表项 `struct file` 相关联的，而不是文件描述符 `fd`。

也就是通过 `fork()` 或者 `dup()` 复制 `fd` 后，可以通过这两个 `fd` 同时操作锁，但是关闭其中一个 `fd` 锁并不会释放，因为 `struct file` 并没有释放，只有关闭所有复制出的 `fd`，锁才会释放。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>

int main(void)
{
        int rc, fd1, fd2;

        fd1 = open("/tmp/foobar.txt", O_RDWR);
        fd2 = dup(fd1);
        close(fd1);

        printf("current fd: %d\n", fd2);
        rc = flock(fd2, LOCK_EX);
        printf("get lock2, ret: %d\n", rc);
        sleep(10);
        close(fd2);
        printf("release\n");

        sleep(10000);

        return 0;
}
{% endhighlight %}

如上，在关闭掉所有的文件描述符之后才会释放掉文件锁。

#### 子进程

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>

int main(void)
{
        int rc, pid, fd;

        fd = open("/tmp/foobar.txt", O_RDWR);

        pid = fork();
        if (pid == 0) { /* child */
                rc = flock(fd, LOCK_EX);
                printf("chile get lock, fd: %d, ret: %d\n", fd, rc);
                sleep(10);
                printf("child exit\n");
                exit(0);
        }

        rc = flock(fd, LOCK_EX);
        printf("parent get lock, fd: %d, ret: %d\n", fd, rc);
        sleep(12);
        printf("parent exit\n");

        return 0;
}
{% endhighlight %}

子进程持有锁，并不影响父进程通过相同的 fd 获取锁，反之亦然。

#### 多次打开

当使用 `open()` 两次打开同一个文件，得到的两个 `fd` 是独立的，内核会使用两个 `struct file` 对象，通过其中一个加锁，通过另一个无法解锁，并且在前一个解锁前也无法上锁。

{% highlight c %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>

int main(void)
{
        int rc, fd1, fd2;

        fd1 = open("/tmp/foobar.txt", O_RDWR);
        fd2 = open("/tmp/foobar.txt", O_RDWR);
        printf("fd1: %d, fd2: %d\n", fd1, fd2);

        rc = flock(fd1, LOCK_EX);
        printf("get lock1, ret: %d\n", rc);
        close(fd1);

        rc = flock(fd2, LOCK_EX);
        printf("get lock2, ret: %d\n", rc);

        return 0;
}
{% endhighlight %}

上述代码中，如果注释掉 `close(fd1)` 会被阻塞。

<!--
(3) 使用exec后，文件锁的状态不变。
(4) flock不能再NFS文件系统上使用，如果要在NFS使用文件锁，请使用fcntl。
(5) flock锁可递归，即通过dup或者或者fork产生的两个fd，都可以加锁而不会产生死锁。
-->

## PIDFile

默认进程在使用 flock 尝试锁文件时，如果文件已经被其它进程锁住，进程会被阻塞直到锁被释放掉。也可以使用 `LOCK_NB` 参数，此时如果被锁，那么会直接返回错误，对应的 `errno` 为 `EWOULDBLOCK`。

简单来说，也就是阻塞、非阻塞两种工作模式。

flock 可以通过 `LOCK_UN` 显示的释放锁，也可以直接通过关闭 fd 的来释放文件锁，这意味着 flock 会随着进程的关闭而被自动释放掉。

<!--
注意，`flock()` 可以被 `fork()` 继承。

也就是以排它非阻塞的方式运行。
http://blog.jobbole.com/102538/

#include <stdio.h>
#include <sys/file.h>

int check_pidfile(char *file)
{
        int fd, rc;

        if (file == NULL)
                return -1;

        fd = open(file, O_RDWR);
        if (fd < 0)
                return -2;

        rc = flock(fd, LOCK_EX | LOCK_NB);
        if (rc < 0)
                return -3;

        return 0;
}
-->

## 其它

### /proc/locks

可以从 `/proc/locks` 文件查看所有的文件锁信息，示例如下：

{% highlight text %}
1: POSIX  ADVISORY  READ  3633 08:08:1612502 1073741826 1073742335
2: POSIX  ADVISORY  READ  5181 08:08:1441890 128 128
3: FLOCK  ADVISORY  WRITE 2367 08:07:2236373 0 EOF
{% endhighlight %}

<!--
1) POSIX FLOCK 这个比较明确，就是哪个类型的锁。flock系统调用产生的是FLOCK，fcntl调用F_SETLK，F_SETLKW或者lockf产生的是POSIX类型，有次可见两种调用产生的锁的类型是不同的；

2) ADVISORY表明是劝告锁；

3) WRITE顾名思义，是写锁，还有读锁；

4) 18849是持有锁的进程ID。当然对于flock这种类型的锁，会出现进程已经退出的状况。

5) 08:02:852674表示的对应磁盘文件的所在设备的主设备好，次设备号，还有文件对应的inode number。

6) 0表示的是所的其实位置

7) EOF表示的是结束位置。 这两个字段对fcntl类型比较有用，对flock来是总是0 和EOF。 
-->



{% highlight text %}
{% endhighlight %}
