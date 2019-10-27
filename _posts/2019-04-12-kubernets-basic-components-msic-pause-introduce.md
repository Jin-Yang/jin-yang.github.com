---
title: K8S Pause
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---

在 K8S 的 Slave 节点上，除了本身启动的容器外，还会看到很多的 Pause 容器，一般是一个 Pod 会对应一个 Pause 。

那么这个 Pause 的用途是什么？

<!-- more -->

## 简介

Pod 底层的 runtime 可以支持多种，例如 Docker、rkt 等，而这些 runtime 底层一般会基于内核的 cgroup 以及 namespace 机制，在一个 Pod 中，各个进程会有独立 cgroup 配置，但是会使用相同的 namespace 。

Pause 会作为所有容器的 "父容器" ，会创建 Linux 中的 namespace ，并作为 PID 为 1 的进程，回收其它容器产生的僵尸进程。

### NameSpace

在 Linux 中，默认子进程会继承父进程的 namespace ，如果子进程想使用新的 namespace ，那么就可以通过 unshare 命令。

{% highlight text %}
# unshare --pid --uts --ipc --mount -f chroot rootfs /bin/sh
{% endhighlight %}

当创建了一个新的 namespace 之后，可以通过 setns 系统调用，将进程添加到已经存在的 namespace 中。

<!--
https://jimmysong.io/posts/what-is-a-pause-container/
https://zhuanlan.zhihu.com/p/52938305
https://o-my-chenjian.com/2017/10/17/The-Pause-Container-Of-Kubernetes/
https://www.ianlewis.org/en/almighty-pause-container
https://blog.sctux.com/2018/12/07/k8s-ji-qun-zhongpause-rong-qi-shi-gan-ma-de/

https://github.com/kubernetes/kubernetes/tree/master/build/pause
-->


{% highlight text %}
{% endhighlight %}
