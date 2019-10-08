---
title: Docker RunC 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

OCI 定义了容器运行时标准，runC 是 Docker 按照开放容器格式标准 Open Container Format, OCF 制定的一种具体实现。

该项目从 Docker 的 libcontainer 中迁移来，实现了容器启停、资源隔离等功能，所以，可以直接通过该工具运行容器。

<!-- more -->

## 简介

所谓容器运行时 (Container Runtime) 包括了容器管理和容器镜像，Open Container Initiative, OCI 是容器运行时的工业标准，包括了：A) 运行时标准 [runtime-spec](https://github.com/opencontainers/runtime-spec)；B) 容器镜像标准 [image-spec](https://github.com/opencontainers/image-spec) 。

简单来说，容器镜像标准定义了容器镜像的打包形式，而运行时标准定义了如何去运行一个容器。

RunC 是一个遵循 OCI 标准的用来运行容器的命令行工具，它也是一个 Runtime 的实现。

### Docker

老版本里，使用需要调用 `docker-runc` ，而最新的 CE 版本里，直接修改为了 `runc` 命令。

![docker runc]({{ site.url }}/images/docker/docker-runc-arch.png "docker runc"){: .pull-center width="70%" }

RunC 作为容器的最底层运行环境，其上层通过 Docker 进行管理。

## 独立运行

RunC 作为容器的运行态，不包含镜像的管理，如果直接使用，需要先准备好镜像，这里直接使用已经构建好的 BusyBox 模板。

### OCI Bundle

OCI Bundle 是指满足 OCI 标准的一系列文件，这些文件包含了运行容器所需要的所有数据，它们存放在一个共同的目录，该目录包含以下两项：

* `config.json` 包含容器运行的配置数据；
* 容器的 root filesystem 。

如果主机上安装了 Docker，那么可以使用 `docker export` 命令将已有镜像导出为 OCI Bundle 的格式。

### 生成 rootfs

直接使用 Docker 中的 BusyBox 模板。

{% highlight text %}
----- 下载最新的版本
$ docker pull busybox

----- 创建rootfs
$ mkdir rootfs
$ docker export $(docker create busybox) | tar -C rootfs -xvf -
{% endhighlight %}

### 配置文件

在 `rootfs` 目录下，包含了常见的二进制文件，除了根目录之外，还需要一个 `config.json` 配置文件，可以通过 `runc spec` 命令生成配置模板。

详细的配置可以参考 [Open Container Initiative Runtime Specification](https://github.com/opencontainers/runtime-spec/blob/master/spec.md) 。

将配置文件中的 `"terminal": true` 修改为 `false` ，否则会在创建的时候会宝 `cannot allocate tty if runc will detach without setting console socket` 报错。

另外，因为关闭了终端，所以将命令修改为 `"args": [ "sleep", "10000" ],` 参数。

### 启动

{% highlight text %}
----- 创建容器并查看状态
# runc create mybusybox
# runc list

----- 启动容器，并查看状态
# runc start mybusybox

----- 删除容器
# runc delete mybusybox
{% endhighlight %}

在启动了之后，可以通过 `ps aux | grep sleep` 看到具体的命令。

## 常用命令

{% highlight text %}
----- 查看容器状态
# runc state mybusybox

----- 容器内运行进程
# runc ps mybusybox

----- 在容器中执行命令
# runc exec mybusybox ls

----- 停止容器内的任务
# runc kill mybusybox

----- 暂停容器内的所有进程
# runc pause mybusybox

----- 恢复容器内进程的执行
# runc resume mybusybox

----- 获取容器的资源使用情况
# runc events mybusybox
{% endhighlight %}

## 热迁移

就是将一个容器进行 CheckPoint 操作，并获得一系列文件，然后再在其它机器上启动。


<!--
https://cizixs.com/2017/11/05/oci-and-runc/
https://segmentfault.com/a/1190000017543294
-->

{% highlight text %}
{% endhighlight %}
