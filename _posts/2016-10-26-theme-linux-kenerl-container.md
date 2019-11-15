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

在 Linux 上提供了 cgroup 以及 namespace 的基本隔离手段，然后在此基础上，提供了更上层的封装，包括了 Docker、rkt、PouchContainer 以及最新的 Podman 等等。


## Kubernetes

![k8s logo]({{ site.url }}/images/docker/k8s-logo.jpg "k8s logo"){: .pull-center width="40%" }

一个开源用于管理容器化应用的平台，其目标是让部署容器化的应用简单并且高效，提供了应用部署、规划、更新、维护。

* [K8S 使用简介](/post/kubernets-basic-introduce.html)
* [K8S Pod 简介](/post/kubernets-pod-info-introduce.html)


## Docker

![docker logo]({{ site.url }}/images/docker/docker-logo.png "docker logo"){: .pull-center width="30%" }

一个开源的应用容器引擎，可以让开发者将应用及其依赖包打包到一个可移植的镜像中。

* [Docker 使用简介](/post/docker-basic-introduce.html)
* [Dockerfile 简介](/post/docker-basic-concept-dockfile-introduce.html)
* [Docker Compose 简介](/post/docker-compose-tools-introduce.html)
* [Docker RunC 简介](/post/docker-component-runc-introduce.html)
* [Docker Containerd 简介](/post/docker-component-containerd-introduce.html)

## Podman

![podman logo]({{ site.url }}/images/docker/podman-logo.svg "podman logo"){: .pull-center width="50%" }

这也是最新的容器实现方式，在 RedHat-8 或者 CentOS-8 中提供的默认方式，其操作方式与 Docker 基本相同。

* [Podman 使用简介](/post/podman-container-basic-introduce.html)

## container

实际上现在很火的 Docker 的底层是基于容器的，这部分也比较复杂。

* [LXC 简介](/post/linux-container-lxc-introduce.html) 对 Linux Container 的简单介绍，包括如何安装、新建、启动容器等操作。
* [LXC sshd 单进程启动](/post/linux-container-lxc-sshd.html) 介绍如何启动一个单进程，对于资源隔离有很大的参考意义。
* [容器之 CGroup](/post/linux-container-cgroup-introduce.html) 介绍 CentOS 中 systemd 以及 cgroup-tools 相关的内容。
* [cgroup 之 cpuset 简介](/post/linux-cgroup-cpuset-subsys-introduce.html) 简单介绍 cpuset 的使用方法。
* [网络 namespace](/post/linux-namespace-network-introduce.html) 与网络相关的 namespace 介绍。

<!--
* [LXC 网络设置相关](/post/linux-container-lxc-network.html)，关于 Container 中网络的介绍，主要介绍 veth、vlan、macvlan 等概念。
-->

## 其它

* [Linux 资源限制](/post/linux-resource-limit-introduce.html) 在 Linux 中最原始的一种资源限制方法，也就是 ulimits 。
* [Linux Chroot](/post/linux-chroot.html) 这实际上是做目录隔离的方法，也是最初的一种方式。
* [LVM 使用简介](/post/linux-logical-volume-manager-introduce.html)
* [KVM 虚拟平台](/post/linux-virtual-kvm-introduce.html)


{% highlight text %}
{% endhighlight %}
