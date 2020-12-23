---
title: Linux Direct IO
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: 
description:
---


<!-- more -->

## 使用

在 Linux 中使用 DirectIO 很简单，在通过 `open(2)` 打开文件时，在 `flags` 参数中添加上 `O_DIRECT` 即可，以告诉内核想对该文件进行直接 IO 操作。

当然，上述的 `O_DIRECT` 宏依赖于 `_GNU_SOURCE` 的宏定义。

### 注意事项

因为涉及到直接操作磁盘，那么需要保证 **存放数据缓存区的起始位置** 以及每次 **读写数据长度** 必须是 **磁盘逻辑块大小的整数倍**，如果不满足就会报 `EINVAL` 的错误。

磁盘逻辑块的大小可以通过 `statfs()` 函数获取，对应了 `f_bsize` 字段，一般是 512 字节，如果是 SSD 可能是 4096 大小，也可以通过如下命令获取。

{% highlight text %}
----- 硬盘的最小存储单位，一般是扇区，本身并没有Block概念
# fdisk -l
----- 文件系统Block大小，首先查看类型，然后再选择对应的工具
# df -T
/dev/sda2   ext4      51474912 16553932  32283156  34% /home
# tune2fs -l /dev/sda2 | grep "Block size"
Block size:               4096
{% endhighlight %}

<!--
/*
 * gcc directio.c -o directio -D_GNU_SOURCE
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>

#define BUF_SIZE 1024

int main(void)
{
        int fd, i;
        unsigned char *buff;

        if (posix_memalign((void **)&buff, 512, BUF_SIZE)) {
                fprintf(stderr, "malloc memory align failed, %d:%s.",
                        errno, strerror(errno));
                return -1;
        }
        memset(buff, 'c', BUF_SIZE);

        fd = open("direct_io.data", O_WRONLY | O_DIRECT | O_CREAT, 0644);
        //fd = open("direct_io.data", O_WRONLY | O_CREAT, 0644);
        if (fd < 0){
                fprintf(stderr, "open file failed, %d:%s.",
                        errno, strerror(errno));
                close(fd);
                return -1;
        }

        for (i = 0; i < 102400; i++) {
                if (write(fd, buff, BUF_SIZE) < 0) {
                        fprintf(stderr, "write to file failed, %d:%s.",
                                errno, strerror(errno));
                        break;
                }
        }
        fsync(fd);

        free(buff);
        close(fd);

        return 0;
}

但是这个测试程序的性能下降好多，

$ time ./directio
real    0m0.316s
user    0m0.027s
sys     0m0.202s

$ time ./directio 
real    1m31.724s
user    0m0.085s
sys     0m2.159s


#define _GNU_SOURCE
#include <stdlib.h>
int posix_memalign (void **memptr, size_t alignment, size_t size);

会返回 size 字节大小的动态内存，并且这块内存的地址是 alignment 的倍数，而且参数 alignment 必须是 2 的幂，是 `void *` 指针大小的倍数。

有些应用会自动管理文件的 Cache ，那么此时就需要绕开 Linux VFS 提供的缓存机制，也就是 Direct IO 提供的能力。
-->


{% highlight text %}
{% endhighlight %}
