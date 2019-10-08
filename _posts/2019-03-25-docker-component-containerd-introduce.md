---
title: Docker Containerd 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

这是在容器技术标准化之后的产物，为了能够兼容 OCI 标准，将容器运行时及其管理功能从 Docker Daemon 剥离。从理论上来说，即使不运行 dockerd，也能够直接通过 containerd 来管理容器。

<!-- more -->

## 简介

在宿主机中管理完整的容器生命周期，包括了：

* 管理容器的生命周期，从创建到销毁；
* 拉取、推送容器镜像；
* 存储管理，包括了管理镜像及容器数据的存储；
* 调用 runC 运行容器，并与其进行交互；
* 管理容器网络接口及网络。


![docker containerd]({{ site.url }}/images/docker/docker-containerd-arch.png "docker containerd"){: .pull-center width="70%" }


安装包里包含了如下的几个组件：

* `containerd` 容器运行时，通过 gRPC 协议提供满足 OCI 标准的 API；
* `containerd-shim` 每个容器的运行时载体；
* `ctr` 一个简单的 CLI 接口，用作 containerd 本身的一些调试用途。

### 配置文件

默认配置文件保存在 `/etc/containerd/config.toml`，可以通过 `containerd config default > /etc/containerd/config.toml` 命令生成一个默认的配置文件。




## 参考

* [containerd.io](https://containerd.io/)

<!--
https://www.cnblogs.com/sparkdev/p/9063042.html
-->

{% highlight text %}
{% endhighlight %}
