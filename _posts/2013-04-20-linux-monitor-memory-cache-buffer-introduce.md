---
title: Cache 能否回收
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,monitor,memory,cache
description:
---

<!-- more -->

我们分析了cache能被回收的情况，那么有没有不能被回收的cache呢？当然有。我们先来看第一种情况：

### tmpfs

Linux 中有个 tmpfs 作为临时文件系统，可以将内存中的一部分空间作为文件系统使用，常见的是 `/dev/shm` 这个目录，也可以手动创建。

{% highlight text %}
# mkdir /tmp/ramdisk
# mount -t tmpfs -o size=500M none /tmp/ramdisk
# umount /tmp/ramdisk
{% endhighlight %}

因为是文件系统，那么正常来说使用的应该是 Page Cache ，可以通过如下命令进行测试。

{% highlight text %}
# echo 3 > /proc/sys/vm/drop_caches
# free -m
              total        used        free      shared  buff/cache   available
Mem:          15359        4946        9437         686         974        9371
Swap:          4095         427        3668
# dd if=/dev/zero of=/tmp/ramdisk/file bs=1M count=400
400+0 records in
400+0 records out
419430400 bytes (419 MB) copied, 0.243054 s, 1.7 GB/s
# free -m
              total        used        free      shared  buff/cache   available
Mem:          15359        4947        9033        1086        1378        8969
Swap:          4095         427        3668
{% endhighlight %}

可以看到 `shared` `buff/cache` 有所增加，而 `free` `available` 同时减少，如果再次尝试释放内存仍然是无效的，只有当文件被释放掉之后才能被使用。

{% highlight text %}
# free -m
             total       used       free     shared    buffers     cached
Mem:          3833        750       3083          0          0         49
-/+ buffers/cache:        700       3133
Swap:         4091        410       3681
# dd if=/dev/zero of=/tmp/ramdisk/file bs=1M count=400
400+0 records in
400+0 records out
419430400 bytes (419 MB) copied, 0.208524 s, 2.0 GB/s
# free -m
             total       used       free     shared    buffers     cached
Mem:          3833       1151       2682          0          0        450
-/+ buffers/cache:        700       3133
Swap:         4091        410       3681
{% endhighlight %}

### 共享内存

这是一种常用的进程间通信 (IPC) 方式，不过不能在 Shell 申请和使用，如下是一个简单的测试用例。

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MEMSIZE 100 * 1024 * 1024

int main(void)
{
        int shmid, rc;
        struct shmid_ds buf;
        pid_t pid;
        char *ptr;

        shmid = shmget(IPC_PRIVATE, MEMSIZE, 0600);
        if (shmid < 0) {
                perror("shmget()");
                exit(1);
        }
        rc = shmctl(shmid, IPC_STAT, &buf);
        if (rc < 0) {
                perror("shmctl()");
                exit(1);
        }
        printf("shmid: %d\n", shmid);
        printf("shmsize: %ld\n", buf.shm_segsz);

        pid = fork();
        if (pid < 0) {
                perror("fork()");
                exit(1);
        } else if (pid == 0) {
                ptr = (char *)shmat(shmid, NULL, 0);
                if (ptr == (void *)-1) {
                        perror("shmat()");
                        exit(1);
                }
                bzero(ptr, MEMSIZE);
                strcpy(ptr, "Hello!");
                exit(0);
        } else {
                wait(NULL);
                ptr = (char *)shmat(shmid, NULL, 0);
                if (ptr == (void *)-1) {
                        perror("shmat()");
                        exit(1);
                }
                puts(ptr);
                exit(0);
        }

        return 0;
}
{% endhighlight %}

程序很简单，就是申请一块约 400M 的共享内存，然后打开一个子进程对这段共享内存做一个初始化操作，父进程等子进程初始化完之后输出一下共享内存的内容，然后退出。

注意，上述的程序在退出之前并没有删除这段共享内存，同样可以通过 `free` 命令查看其内存的使用情况。

{% highlight text %}
# free -m
              total        used        free      shared  buff/cache   available
Mem:          15359        5329        9051         686         977        8987
Swap:          4095         427        3668
# ./test
shmid: 491522
shmsize: 419430400
Hello!
# free -m
              total        used        free      shared  buff/cache   available
Mem:          15359        5330        8627        1086        1401        8574
Swap:          4095         427        3668
{% endhighlight %}

简单来说，这段共享内存即使没人使用，仍然会长期存放在 cache 中，直到其被删除。删除方法有两种，一种是程序中使用 `shmctl(IPC_RMDI)` 去删除，或者使用 ipcrm 命令。

{% highlight text %}
----- 查看当前系统的所有共享内存
# ipcs -m
----- 删除指定的共享内存
# ipcrm -m 491522
{% endhighlight %}

实际上，内核底层在实现共享内存 (shm)、消息队列 (msg) 和信号量 (sem) 这些 IPC 机制时，使用的都是 tmpfs，这也是为什么共享内存的操作逻辑与 tmpfs 类似的原因。

只是，一般情况下 shm 占用的内存更多。

### mmap

该系统函数将一个文件映射到进程的虚拟内存地址，然后就可以用类似操作内存的方式对文件的内容进行操作。

实际上，其使用范围要更广泛些，例如：A) 通过 `malloc()` 申请内存时，小段内存内核使用 `sbrk()` 处理，而大段内存使用 `mmap()`；B) 通过系统调用 `exec()` 族函数执行时，其本质上是将一个可执行文件加载到内存执行，同样使用 `mmap()`；C) 同时也可以作为共享内存申请使用。

同样，如下是一个简单的测试程序：

{% highlight text %}
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/mman.h>

#define MEMSIZE  400 * 1024 * 1024
#define MMAPFILE "/tmp/mmapfile"

int main()
{
        void *ptr;
        int fd;

        fd = open(MMAPFILE, O_RDWR);
        if (fd < 0) {
                perror("open()");
                exit(1);
        }

        ptr = mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, fd, 0);
        if (ptr == NULL) {
                perror("malloc()");
                exit(1);
        }
        printf("%p\n", ptr);
        bzero(ptr, MEMSIZE);
        sleep(100);
        munmap(ptr, MEMSIZE);

        close(fd);
        exit(1);
}
{% endhighlight %}

上述功能很简单，申请一块 400M 的内存，然后等待 100s 后释放，可以用同样的方式查看。

注意，在使用前要先创建一个相应大小的文件 `dd if=/dev/zero of=/tmp/mmapfile bs=1M count=400`。

同样，在程序退出之后 cached 占用的空间被释放。当使用 `mmap()` 申请标志状态为 `MAP_SHARED` 的内存时，内核也是使用的 cache 进行存储的。

实际上，`mmap()` 的 `MAP_SHARED` 方式申请的内存，在内核中也是由 `tmpfs` 实现的。

### 总结

简单来说，`shmget` `mmap` 的共享内存，在内核层都是通过 `tmpfs` 实现的，而 `tmpfs` 实现的存储用的都是 cache ，这样只有当使用的空间被删除之后对应的内存才可能会被删除。


<!--
Linux内存中的Cache真的能被回收么？
http://liwei.life/2016/04/26/linux%e5%86%85%e5%ad%98%e4%b8%ad%e7%9a%84cache%e7%9c%9f%e7%9a%84%e8%83%bd%e8%a2%ab%e5%9b%9e%e6%94%b6%e4%b9%88%ef%bc%9f/
-->



{% highlight text %}
{% endhighlight %}
