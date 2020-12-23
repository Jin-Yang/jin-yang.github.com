---
title: Linux 内存磁盘
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,ram disk
description: 简单来说，我们在做一些压测时，可能会由于磁盘性能的限制无法得到极限的压测结果，此时可以使用 RAM DISK 进行测试。内存磁盘是把一部分内存模拟成磁盘，可以把它当成一块高速的硬盘使用。
---

简单来说，我们在做一些压测时，可能会由于磁盘性能的限制无法得到极限的压测结果，此时可以使用 RAM DISK 进行测试。

内存磁盘是把一部分内存模拟成磁盘，可以把它当成一块高速的硬盘使用。

<!-- more -->

## 使用

除了上述的场景，例如浏览器的缓存也可以使用，可以降低对硬盘的读写，在一定程度上保护了硬盘，尤其是 SSD 。

当然，缺点的话就是无法做持久化。

### 创建

在 Windows 上创建 RAM DISK 需要用到第三方软件，而 Linux 只要几条命令即可。

{% highlight text %}
----- 新建RAM DISK的挂载点
$ mkdir /tmp/ramdisk
----- 查看可用内存
$ free -h
----- 创建并挂载RAM DISK
$ mount -t tmpfs -o size=128M ramdisk /tmp/ramdisk
----- 查看挂载是否成功
$ df
$ mount
{% endhighlight %}

此时创建了 128M 的 RAM DISK，文件格式为 `tmpfs`，挂载目录是 `/tmp/ramdisk` 。

### 压测

直接使用系统自带的 `dd` 进行写入测试。

{% highlight text %}
----- RAM DISK
$ dd if=/dev/zero of=/tmp/ramdisk/test bs=1024 count=102400 conv=fdatasync

----- 普通硬盘
$ dd if=/dev/zero of=~/test bs=1024 count=102400 conv=fdatasync
{% endhighlight %}

## 其它

{% highlight text %}
----- 卸载RAM DISK，释放内存空间
$ umount /tmp/ramdisk
----- 编辑fstab文件，设置开机启动
$ vim /etc/fstab
ramdisk /tmp/ramdisk tmpfs defaults,size=1G,x-gvfs-show 0 0
{% endhighlight %}

其中 `x-gvfs-show` 选项会在文件管理器中显示挂载的 RAM DISK 。



{% highlight text %}
{% endhighlight %}
