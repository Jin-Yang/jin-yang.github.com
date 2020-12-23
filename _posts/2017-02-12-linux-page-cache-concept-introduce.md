---
title: Linux Page Cache
layout: post
comments: true
language: chinese
category: [linux,program,misc]
keywords: page cache
description:
---


<!-- more -->

## Buffer Cache VS. Page Cache

> The term, Buffer Cache, is often used for the Page Cache. Linux kernels up to version 2.2 had both a Page Cache as well as a Buffer Cache. As of the 2.4 kernel, these two caches have been combined. Today, there is only one cache, the Page Cache.

也即是说目前只剩下了 Page Cache 。

<!--
https://www.thomas-krenn.com/en/wiki/Linux_Page_Cache_Basics

其中 Page Cache 也就对应了 free 命令中的 Cached 项，当向文件写入数据的时候，首先会更新 Page Cache 并将对应的页设置为脏页。

然后可以通过 `sync` `fsync` 手动或者系统周期性的自动将数据写入到磁盘，可以通过如下示例测试。

$ dd if=/dev/zero of=foobar.txt bs=1M count=10
10+0 records in
10+0 records out
10485760 bytes (10 MB) copied, 0,0121043 s, 866 MB/s
$ cat /proc/meminfo | grep Dirty
Dirty:             10588 kB
$ sync
$ cat /proc/meminfo | grep Dirty
Dirty:                 4 kB

在 2.6.31 版本(含)之前，会通过 `pdflush` 内核线程将脏页周期性的刷新到磁盘。

因为 `pdflush` 在性能上会有些问题，在 2.6.32 版本之后每个设备都会起一个内核线程来处理刷脏操作。

对于读取同样会更新 Page Cache，一般可以看到第二次读取数据的速度往往会明显快于第一次的读取。

有个很不错的图片，绘制了整个磁盘的使用方式
https://en.wikipedia.org/wiki/Page_cache
-->


{% highlight text %}
{% endhighlight %}
