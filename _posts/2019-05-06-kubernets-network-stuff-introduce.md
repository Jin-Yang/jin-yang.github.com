---
title: K8S 网络简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

#################################
## 网络
#################################

K8S 中的网络以及 IP 地址都分了三层，分别为：

* Node Network 各个 Node 之间的通讯方式，可以是物理网络或者虚拟网络；
* Pod Network 各个 Pod 之间通讯，Pod 可以在 Node 内部，也可以是跨 Node ；
* Service Network 也就是 Pod 对外部暴露的接口，创建 Service 时会新建一个 IP 。

与之对应的 IP 地址包括了：

* Node IP 每个 Node 之间通讯时，分配的 IP 地址；
* Pod IP 每个 Pod 会分配一个 IP 地址，一般由 Pause 容器创建，一个 Node 中可以包含多个 Pod ；
* Cluster IP 对应了 Service 内部 IP ，作为服务的入口。

K8S 支持 Flannel、Calico、Weave Network 等多种 CNI 网络驱动。

----- 获取Cluster IP信息
# kubectl get services
----- 各个Pod IP信息
# kubectl get pod -o wide

使用flannel做通讯
https://tonybai.com/2017/01/17/understanding-flannel-network-for-kubernetes/
https://jiayi.space/post/kubernetescong-ru-men-dao-fang-qi-3-wang-luo-yuan-li
https://blog.csdn.net/huwh_/article/details/77922093
CoreOS
https://www.jianshu.com/p/c8684f943a7c

## Docker 网络

在 Docker 的后台服务启动之后，会创建一个 docker0 的网关，容器默认会分配在一个以 docker0 为默认网关的虚拟子网中。

ifconfig docker0
docker inspect nginx | grep IPAddress

为此，Docker 使用了 Linux 中的 Bridge、Network Namespace、VETH 几种方案。

* Bridge 工作在二层网络的虚拟网桥，可以配置 IP ，也就是上述的 docker0 网关；
* Network Namespace 内核实现的网络命名空间，可以建立一些完全隔离的网络栈，可以通过 `docker network create XXX` 命令创建；
* VETH 虚拟网卡，可以连接不同 Network Namespace 中的通信。

也就是说，通过 Namespace 做容器之间的网络隔离，利用 Bridge 建立容器之间、容器和宿主机之间建立一个网关，然后再用 VETH 将容器和宿主机两个网络通讯连接起来。

## Pod Network


{% highlight text %}
{% endhighlight %}
