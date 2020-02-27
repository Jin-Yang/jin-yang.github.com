---
title: Podman 使用简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

Docker 是一个以 root 身份在系统上运行的守护程序，然后利用 Linux 内核的功能来管理正在运行的容器，包括了容器镜像。

而 Podman 提供与 Docker 类似的功能，但是不再以 root 用户运行，而且非常驻进程。

<!-- more -->

![podman logo]({{ site.url }}/images/docker/podman-logo.svg "podman logo"){: .pull-center width="80%" }

## 简介

{% highlight text %}
----- 查看版本信息
# podman version

----- 环境信息
# podman info

----- 尝试拉取镜像，会尝试registry.redhat.io quay.io docker.io几个镜像
# podman search busybox
# podman pull busybox
# podman run busybox /bin/echo "Computing for Geeks"
# podman run -it busybox sh
# podman ps

----- 镜像操作
# podman images
# podman tag 19485c79a9bb busybox
# podman rmi busybox
# podman inspect 19485c79a9bb

----- 删除容器
# podman ps -a
# podman rm 024a277cc474
# podman rm $(podman ps -a -q)
{% endhighlight %}



## 参考

* [podman.io](https://podman.io/)

{% highlight text %}
{% endhighlight %}
