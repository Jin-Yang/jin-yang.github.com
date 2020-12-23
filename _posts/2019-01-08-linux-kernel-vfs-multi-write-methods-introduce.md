---
title: Linux Write API 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,vfs,write,pwrite,writev
description: 之前其实已经介绍过，Linux 中有条设计原则，"一切都是文件" 。 对于文件的操作除了 `open()` `close()` 之外，使用比较多的就是 `read()` 和 `write()` 函数了，为了适配不同的应用场景，实际上，Linux 还提供了 `writev()` `pwrite()` 之类的函数。那么，这些函数的区别、使用场景是什么。
---

之前其实已经介绍过，Linux 中有条设计原则，"一切都是文件" 。

对于文件的操作除了 `open()` `close()` 之外，使用比较多的就是 `read()` 和 `write()` 函数了，为了适配不同的应用场景，实际上，Linux 还提供了 `writev()` `pwrite()` 之类的函数。

那么，这些函数的区别、使用场景是什么。

<!-- more -->

## 简介

可以通过 `man 3 write` 或者 `man 2 writev` 查看详细的介绍，这里仅简单将 `write` 相关的函数摘抄出来。

{% highlight text %}
#include <unistd.h>
ssize_t write(int fildes, const void *buf, size_t nbyte);
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);

#include <sys/uio.h>
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
{% endhighlight %}

三类都是将数据写入到文件，包括了磁盘、网络等等，只要是可以抽象成文件的都可以。

其区别是针对不同的应用场景，用户可以根据场景选择相应的函数使用。

### write VS. pwrite

可以直接通过 `man 3 write` 查看，其中关于 `pwrite` 的部分摘抄如下。

> The pwrite() function shall be equivalent to write(), except that it writes into a given position without changing the file pointer. The first three arguments to pwrite() are the same as write() with the addition of a fourth argument offset for the desired position inside the file.

也就是说，`pwrite()` 会直接跳转到 `offset` 处，然后继续读取 `nbyte` 个字节，不过这不会影响到原文件的偏移量。这对于多线程的读写会比较友好，此时不会相互影响读写文件时的 offset 。

### write VS. writev

简单来说，`write()` 用来写入连续数据块，而 `writev` 会写入分散的数据块，当然，两个函数的最终结果都是将内容写入连续的空间。

注意，使用 `writev()` 类似的函数时，为了保证兼容性，其中 iovcnt 参数最好不要超过 `IOV_MAX` 大小，该值在 `<limits.h>` 头文件中定义。


## 问题

通过 `writev()` 的出发点是好的，减少了系统调用的次数，提高了效率。

但是在使用时需要注意，对于 socket 来说，如果单次无法写完，对于非阻塞来说，此时会返回 `EAGAIN` 错误，需要在下次写入时重新计算基址和长度。

对于 `write()` 函数来说很简单，而 `writev()` 返回的是字节数，但是入参却是 `iovec` ，也就意味着此时需要重新计算 `iovec` 了，包括了写入的数据可能会在某个 `iovec` 的中间位置。

## 总结

一般来说 `write()` 函数要比 `writev()` 函数的效率高。

那么，最为高效的方法是预先分配好连续的内存空间，然后直接通过 `write()`函数写入。

当然，如果预先无法评估所需的最大内存，那么当超过当前内存时可以通过 `realloc()` 再次分配，如此一来，会造成内存空间利用率低的问题，且 `realloc()` 很可能带来潜在的内存拷贝开销。


{% highlight text %}
{% endhighlight %}
