---
title: 【专题】Linux 容器专题
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## container

实际上现在很火的 Docker 的底层是基于容器的，这部分也比较复杂。

* [LXC 简介](/post/linux-container-lxc-introduce.html)，对 Linux Container 的简单介绍，包括如何安装、新建、启动容器等操作。
* [LXC sshd 单进程启动](/post/linux-container-lxc-sshd.html)，介绍如何启动一个单进程，对于资源隔离有很大的参考意义。
* [容器之 CGroup](/post/linux-container-cgroup-introduce.html)，介绍 CentOS 中 systemd 以及 cgroup-tools 相关的内容。

<!--
* [LXC 网络设置相关](/post/linux-container-lxc-network.html)，关于 Container 中网络的介绍，主要介绍 veth、vlan、macvlan 等概念。
* [Bootstrap](/post/bootstrap-etc.html)，一个来自 Twitter 的前端框架，同时包括了一些 css、javascript 相关的内容介绍。
-->

## 其它

* [Linux 资源限制](/post/linux-resource-limit-introduce.html) 在 Linux 中最原始的一种资源限制方法。
* [Linux Chroot](/post/linux-chroot.html) 这实际上是做目录隔离的方法，也是最初的一种方式。


{% highlight text %}
{% endhighlight %}
