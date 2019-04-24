---
title: Linux 进程退出码
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,process,exit code,退出码
description: Linux 下进程的退出包括了正常退出和异常退出，正常退出包括了 A) `main()` 函数中通过 `return` 返回；B) 调用 `exit()` 或者 `_exit()` 退出。异常退出包括了 A) `abort()` 函数；B) 收到了信号退出。不管是哪种退出方式，系统最终都会执行内核中的同一代码，并将进程的退出方式以返回码的方式保存下来。
---

Linux 下进程的退出包括了正常退出和异常退出，正常退出包括了 A) `main()` 函数中通过 `return` 返回；B) 调用 `exit()` 或者 `_exit()` 退出。异常退出包括了 A) `abort()` 函数；B) 收到了信号退出。

不管是哪种退出方式，系统最终都会执行内核中的同一代码，并将进程的退出方式以返回码的方式保存下来。

<!-- more -->

## 简介

当进程正常或异常终止时，内核就向其父进程发送 `SIGCHLD` 信号，对于 `wait()` 以及 `waitpid()` 进程可能会出现如下场景：

* 如果其所有子进程都在运行则阻塞；
* 如果某个子进程已经停止，则获取该子进程的退出状态并立即返回；
* 如果没有任何子进程，则立即出错返回。

如果进程由于接收到 `SIGCHLD` 信号而调用 wait，则一般会立即返回；但是，如果在任意时刻调用 wait 则进程可能会阻塞。

### 等待子进程退出

父进程可以通过 `wait()` 或者 `waitpid()` 获取子进程的状态码，详细可以通过 `man 3 wait` 查看，其声明如下。

如果下面参数中的 status 不是 `NULL`，那么会把子进程退出时的状态返回，该返回值保存了是否为正常退出、正常结束的返回值、被那个信号终止等。

{% highlight c %}
#include <sys/wait.h>

pid_t wait(int *status);
pit_t waitpid(pid_t pid, int *status, int options);
{% endhighlight %}

当要等待一特定进程退出时，可调用 `waitpid()` 函数，其中第一个入参 `pid` 的入参含义如下：

* `pid=-1` 等待任一个子进程，与 wait 等效。
* `pid>0` 等待其进程 ID 与 pid 相等的子进程。
* `pid==0` 等待其进程组 ID 等于调用进程组 ID 的任一个子进程。
* `pid<-1` 等待其进程组 ID 等于 pid 绝对值的任一子进程。

`waitpid` 返回终止子进程的进程 ID，并将该子进程的终止状态保存在 status 中，其中 `waitpid()` 第三个入参指定了一些行为，如下是常见的参数列表：

* `WNOHANG` 没有任何已经结束的子进程则立刻返回，不等待。
* `WUNTRACED` 子进程进入暂停执行情况则马上返回, 不会关心子进程的推出状态。

<!---
通常使用如下的宏定义。

{% highlight text %}
WIFEXITED(status)
    用于判断子进程是否正常退出，正常退出返回0；否则返回一个非零值。

WEXITSTATUS(status)
    当WIFEXITED返回非零时，可以通过这个宏获取子进程的返回值，如果exit(5)则返回5。
    注意，如果WIFEXITED返回零时，这个返回值则无意义。
{% endhighlight %}

waitpid的返回值比wait稍微复杂一些，一共有3种情况：
    当正常返回的时候，waitpid返回收集到的子进程的进程ID；
    如果设置了选项WNOHANG，而调用中waitpid发现没有已退出的子进程可收集，则返回0；
    如果调用中出错，则返回-1，这时errno会被设置成相应的值以指示错误所在；
当pid所指示的子进程不存在，或此进程存在，但不是调用进程的子进程，waitpid就会出错返回，这时errno被设置为ECHILD；

WNOHANG 如果没有任何已经结束的子进程则马上返回，此时返回 0；
WUNTRACED 如果子进程进入到了 Trace 状态，那么则直接返回 0；

WIFEXITED(status)：如果子进程正常结束则为非0 值.
WEXITSTATUS(status)：取得子进程exit()返回的结束代码, 一般会先用WIFEXITED 来判断是否正常结束才能使用此宏.
WIFSIGNALED(status)：如果子进程是因为信号而结束则此宏值为真
WTERMSIG(status)：取得子进程因信号而中止的信号代码, 一般会先用WIFSIGNALED 来判断后才使用此宏.
WIFSTOPPED(status)：如果子进程处于暂停执行情况则此宏值为真. 一般只有使用WUNTRACED时才会有此情况.
WSTOPSIG(status)：取得引发子进程暂停的信号代码, 一般会先用WIFSTOPPED 来判断后才使用此宏.
-->


## 退出码

子进程结束后，其最终的状态信息保存在 status ，在 `sys/wait.h` 中有对相关宏定义的实现，其中退出码有效为 `16Bits` 主要由三部分组成，包括了：

* `Bits 8~15` 通过 `exit()` 接口退出进程，也就是意味着错误码最大为 255 ，如果是 256 那么实际上是 0 ；
* `Bit 7` 用来标示是否有生成 core 文件。
* `Bits 0~6` 对应了接受到的信号，注意这里还通过 `127 0x7f` 定义了 STOP 状态。 

头文件中提供的宏定义包括了：

* `WIFEXITED` 判断是否通过 `exit()` `return` 正常退出，然后通过 `WEXITSTATUS` 获取具体的退出码。
* `WIFSIGNALED` 判断是否是因为接受到了信号而停止，包括 core 的方式实际上也是通过信号完成，此时可通过 `WTERMSIG` 读取信号，`WCOREDUMP` 是否生成 coredump 文件。
* `WIFSTOPPED` 判断子进程是否处于暂停执行状态，一般只有使用 `WUNTRACED` 参数时会返回该值。

其中示例代码如下。

{% highlight c %}
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#define log_info(...)  do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0);
#define log_error(...) do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);

#define REPO_PATH "/tmp/examples/linux/process/exitcode"

int main(void)
{
        int status;
        pid_t pid;
        char *argv[] = {(char *)"/bin/bash", (char *)"-c", (char *)"exit 1", NULL};

        pid = fork();
        if (pid < 0) {
                log_error("fork failed, %s.", strerror(errno));
                exit(EXIT_FAILURE); /* 1 */
        } else if (pid == 0) { /* child */
                //ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                if (execvp(argv[0], argv) < 0) {
                        log_error("execl failed, %s.", strerror(errno));
                        return 0;
                }
        }

        if (waitpid(pid, &status, WUNTRACED) < 0) {
                log_error("waitpid error, %s.", strerror(errno));
                return 0;
        }
        log_info("process #%d exit with %d.", pid, status);
        if (WIFEXITED(status)) {
                log_info("normal termination, exit status = %d.", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
                log_info("abnormal termination, signal number = %d%s.",
                        WTERMSIG(status), WCOREDUMP(status) ? " (core file generated)" : "");
        } else if (WIFSTOPPED(status)) {
                log_info("child stopped, signal number = %d.", WSTOPSIG(status));
        }

        exit(0);
}
{% endhighlight %}


## 测试场景

如下是一些测试的用例，示例可以参考 [github linux process]({{ site.example_repository }}/linux/process/exitcode)。

### 1. 正常退出

满足 `WIFEXITED` 宏定义的条件，也就是通过 `exit()` 或者 `return` 这种接口退出，此时可以直接通过 `WEXITSTATUS` 获取具体的错误码。

可以使用 `bash -c "exit 1"` 或者提供的 [github exitcode]({{ site.example_repository }}/linux/process/exitcode/exitcode.c) 作为测试。

{% highlight c %}
char *argv[] = {(char *)"/bin/bash", (char *)"-c", (char *)"exit 1", NULL};
char *argv[] = {(char *)REPO_PATH "/exitcode", (char *)"1", NULL};
{% endhighlight %}

注意，退出码只能是 `8bits` 也就是说 `256` 和 `0` 是相同的。

### 2. 异常退出

满足 `WIFSIGNALED()` 宏定义的判断条件。

#### Core 掉

常见的是除零错误 [github coredump]({{ site.example_repository }}/linux/process/exitcode/coredump.c) ，这里会有两种方式：A) 生成了 Core 文件，对应的返回码是 `136 = 1000 1000`；B) 没有生成 Core 文件，则是 `8 = 1000` 。

{% highlight c %}
char *argv[] = {(char *)REPO_PATH "/coredump", NULL};
{% endhighlight %}

这里的 `8` 实际上对应了 `SIGFPE` 信号量。

> 是否生成 core file 文件可以通过 ulimit 命令进行查看 (ulimit -c)、开启 (ulimit -c unlimited)、关闭 (ulimit -c 0)。

#### 发送信号

通过执行 `sleep 1000` 命令，然后通过 `kill -SIGTERM <PID>` 或者 `kill -15 <PID>` 手动发送信号。

注意，如果注册了信号的回调函数，而在回调函数里是通过 `exit()` 退出的，那么实际上仍然被认为是 `exit()` 的退出方式。

### 3. 停止执行

实际上对应了 `WIFSTOPPED()` 宏定义，此时需要在子进程中调用 `ptrace()` 接口，默认返回的信号是 `SIGTRAP` 。

示例如下。

{% highlight c %}
char *argv[] = {(char *)"/usr/bin/sleep", (char *)"1", NULL};

pid = fork();
if (pid < 0) {
	log_error("fork failed, %s.", strerror(errno));
	exit(EXIT_FAILURE); /* 1 */
} else if (pid == 0) { /* child */
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	if (execvp(argv[0], argv) < 0) {
		log_error("execl failed, %s.", strerror(errno));
		return 0;
	}
}
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
