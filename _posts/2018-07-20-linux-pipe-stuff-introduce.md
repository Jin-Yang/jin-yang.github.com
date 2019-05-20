---
title: Linux PIPE 相关介绍
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,auto,completion
description:
---


<!-- more -->

## 容量限制

在 Linux 中管道的能力是有限的，对于一个应用来说，如果管道满，那么会导致 `write()` 系统调用被阻塞 (依赖于程序是否设置了 `O_NONBLOCK` 标志)。

不同的系统对应的 Pipe 能力不同，所以应用应该做到尽快读取 Pipe 中的数据，以防止阻塞。

### 最大容量

在 `man 7 pip` 中 `Pipe capacity` 有相关的介绍，对于 2.6.11 之前的版本标准是一个页的大小，一般是 4K ，而在之后最大是 64K 。

实际上通过，`ulimit -a` 查看 Pipe 的大小是 `512 * 8 = 4K` ，而实际上内核会动态分配最大到 16 倍的缓冲，也就是最大到 64K 。

注意，其中的 16 倍是在源码的 `linux/pipe_fs_i.h` 文件中通过 `PIPE_DEF_BUFFERS` 宏定义写死的，所以，如果想要扩展 Pipe 的最大能力，就需要修改宏重新编译内核。

### 内核实现

在 Linux 中，管道的实现并没有使用专门的数据结构，而是借助了文件系统的 `struct file` 和 VFS 的索引节点 `struct inode` ，通过将两个 file 结构指向同一个临时的 VFS 索引节点，而这个 VFS 索引节点又指向一个物理页面而实现的。

有两个 file 数据结构，但它们定义文件操作例程地址是不同的，其中一个是向管道中写入数据的例程地址，而另一个是从管道中读出数据的例程地址。这样，用户程序的系统调用仍然是通常的文件操作，而内核却利用这种抽象机制实现了管道这一特殊操作。

## 原子操作

所谓的原子操作，简单来说就是这次写入要么成功要么失败，不会存在着中间状态。

对于 PIPE 来说，可以参考 [Atomic Operations with Pipes](https://www.tldp.org/LDP/lpg/node13.html) 中的介绍，也就是说 POSIX 规定了 `512` 字节的写入是原子的，而 Linux 实际上是 `4096` 。

Linux 在 `linux/limits.h` 头文件中，通过 `PIPE_BUF` 宏定义了该值。

## popen

简单来说，`popen()` 会 `fork()` 一个子进程，然后建立管道，并通过管道读取相关的数据。

注意处理时需要注意如下的异常场景。

#### 1. 提前关闭

读取部分数据后直接调用了 `pclose()` ，提前关闭了一段的管道，那么接下来的行为要依赖调用命令了，列举如下：

1. 如果调用的命令没有输出任何内容，那么对应的命令可以正常运行。
2. 调用命令同时输出了，因为此时管道的对端关闭了，那么命令输出时实际会接收 `SIGPIPE` 信号，默认是直接退出。

#### 2. 异常处理

因为是标准的 C 库，实际上可以使用 `strerror(errno)` 打印错误信息，包括了 `popen()` `pclose()` ，当然对于 `popen()` 内存申请失败，实际上是不会设置 `errno` 的。

在循环读取数据时，建议使用 `fgets()`，理由如下。

`fgets()` 如果获取的数据超过了 buffer 的大小，实际获取的数据是 `sizeof(buffer) - 1` ，而且最后会填充一个 `'\0'` 作为字符串的结束。

### 总结

不是特别喜欢用类似 `popen()` 这样的封装，理由如下：

1. 同步命令。目前为了保证性能大部分的代码用的是异步调用方式，这样的话 `popen()` 实际使用场景有限。
2. 超时处理。如果要设置命令执行的超时，那么在调用命令的时候需要添加 `timeout` 命令，一般建议为 `timeout --signal=INT --kill-after=10 YOUR CMD` 这种方式。
3. 标准错误输出。该函数只读取标准输出的内容，不会读取标准错误输出，所以调用脚本时，需要将标准错误输出重定向，例如 `command 2>&1`。

在通过 `pclose()` 返回时，如果 `wait4()` 没有报错，那么实际上会返回进程的退出码，这样就可以通过该值获取更多的有效信息，例如退出码、是否core、是否收到信号等等，然后根据这些信息去做相关处理。

{% highlight bash %}
#!/bin/bash
trap "echo 'pipe error' > /tmp/pipe.err" PIPE
for i in {1..100}; do
        #echo "Current count ${i}" >> /tmp/pipe.out
        echo "Current count ${i}"
        sleep 1
done
{% endhighlight %}

{% highlight c %}
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void)
{
        FILE *fp;
        int count = 0;
        char buff[10];

        fp = popen("bash /tmp/pipe.sh", "r");
        if (fp == NULL) {
                fprintf(stderr, "popen() failed, %s.\n", strerror(errno));
                return -1;
        }

        fprintf(stdout, "Output BEGIN\n");
        while(fgets(buff, sizeof(buff), fp) != NULL) {
                fprintf(stdout, "xxx%s", buff); /* including '\n' for fgets() */
                if (++count > 10)
                        break;
        }
        fprintf(stdout, "Output -END-\n");
        if (pclose(fp) < 0)
                fprintf(stderr, "pclose() failed, %s.\n", strerror(errno));


        return 0;
}
{% endhighlight %}





## 其它

### Broken Pipe

一般来说就是尝试写入数据的时候，对端已经关闭，包括了 Socket 以及 Pipe 。

## 示例

{% highlight c %}
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NUM_PRODUCE    5
#define NUM_CONSUME    1

#include <stdarg.h>
#include <pthread.h>

#define MOD "(main) "

#define LVL_INFO   1
#define LVL_ERROR  2

void log_it(int severity, const char *const fmt, ...)
{
        int rc, len;
        va_list ap;
        char buff[128], *ptr, *end;

        end = buff + sizeof(buff);
        if (severity == LVL_INFO)
                memcpy(buff, "INFO  ", sizeof("INFO  "));
        else
                memcpy(buff, "ERROR ", sizeof("ERROR "));
        ptr = buff + sizeof("ERROR "); /* 6 */
        len = end - ptr; /* including '\n' */

        va_start(ap, fmt);
        rc = vsnprintf(ptr, len, fmt, ap);
        va_end(ap);

        if (rc >= len) {
                buff[sizeof(buff) - 1] = '\n';
                buff[sizeof(buff) - 2] = '.';
                buff[sizeof(buff) - 3] = '.';
                buff[sizeof(buff) - 4] = '.';
                len = sizeof(buff);
        } else {
                *(ptr + rc) = '\n';
                len = ptr - buff + 1 + rc;
        }
        write(1, buff, len);
}

#define log_info(...)  log_it(LVL_INFO, __VA_ARGS__);
#define log_error(...) log_it(LVL_ERROR, __VA_ARGS__);

void *producer_func(void *args)
{
        int fd, nbytes, i;
        char buff[1024];

        fd = *(int *)args;
        log_info(MOD "producer thread #%lx start, write fd %d.", pthread_self(), fd);
        nbytes = snprintf(buff, sizeof(buff), "thread %ld\n", pthread_self());
        for (i = 0; i < 1000000; i++) {
                if (write(fd, buff, nbytes) < 0) {
                        log_error(MOD "write to pipe %d times failed, %s.",
                                        i, strerror(errno));
                        break;
                }
        }

        log_info(MOD "producer thread #%lx quit.", pthread_self());
        return NULL;
}
void *consumer_func(void *args)
{
        int fd, rc, count = 0;
        char buff[1024], *ptr, *end, *last, *tail, *str;

        fd = *(int *)args;
        last = ptr = buff;
        end = buff + sizeof(buff);
        log_info(MOD "consumer thread #%lx start, read fd %d.", pthread_self(), fd);
        while (1) {
                rc = read(fd, ptr, end - ptr);
                if (rc < 0) {
                        log_error(MOD "read from pipe failed, %s.", strerror(errno));
                        break;
                } else if (rc == 0) {
                        log_error(MOD "peer has closed, just quit.");
                        break;
                }
                tail = ptr + rc;
                for (ptr = buff, last = buff; ptr < tail; ptr++) {
                        if (*ptr == '\n') {
                                *ptr = 0;

                                str = strchr(last, ' ');
                                *str = 0;
                                if (strcmp(last, "thread") == 0)
                                        count++;

                                //log_info(MOD "got a line: %s", last);
                                last = ptr + 1;
                        }
                }
                if (last == buff) {
                        log_error(MOD "buffer overflow.");
                        break;
                }
                for (ptr = buff; last < tail; last++, ptr++)
                        *ptr = *last;
        }

        log_info(MOD "consumer thread #%lx quit, got %d.", pthread_self(), count);
        return NULL;
}

int main(void)
{
        int pipefd[2], i;
        pthread_t consumers[NUM_CONSUME], producers[NUM_PRODUCE];

        //log_info("123456789");  /* INFO  12345... */
        //log_info("12345678");   /* INFO  12345678 */
        //log_info("1234567");    /* INFO  1234567 */
        //log_it(LVL_INFO, "12345678");

        if (pipe(pipefd) < 0) {
                log_error(MOD "create pipe failed, %s.", strerror(errno));
                return -1;
        }

        for (i = 0; i < NUM_CONSUME; i++)
                pthread_create(&consumers[i], NULL, consumer_func, &pipefd[0]);
        for (i = 0; i < NUM_PRODUCE; i++)
                pthread_create(&producers[i], NULL, producer_func, &pipefd[1]);

        log_info(MOD "wait producers quit.");
        for (i = 0; i < NUM_PRODUCE; i++)
                pthread_join(producers[i], NULL);
        log_info(MOD "all producers quit now.");
        close(pipefd[1]);

        sleep(1);

        log_info(MOD "wait consumers quit.");
        for (i = 0; i < NUM_CONSUME; i++)
                pthread_join(consumers[i], NULL);
        log_info(MOD "all consumers quit now.");

        close(pipefd[0]);

        return 0;
}
{% endhighlight %}


{% highlight text %}
{% endhighlight %}
