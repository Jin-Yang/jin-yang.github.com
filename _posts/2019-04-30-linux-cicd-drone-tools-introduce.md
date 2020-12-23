---
title: Drone 使用
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords:
description:
---


<!-- more -->

## 简介

这里采用 Gogs + Drone + Exec 作为一套系统。

### Runner

这里采用本地的执行模块，添加如下的配置 `/etc/drone-runner-exec/config` 文件。

{% highlight text %}
DRONE_RPC_PROTO=http
DRONE_RPC_HOST=drone.cargo.com
DRONE_RPC_SECRET=ALQU2M0KdptXUdTPKcEw
DRONE_LOG_FILE=/var/log/drone-runner-exec/log.txt
{% endhighlight %}

然后安装服务。

{% highlight text %}
# mkdir -p /var/log/drone-runner-exec
# drone-runner-exec service install
# drone-runner-exec service start
{% endhighlight %}

会新建一个 `/etc/systemd/system/drone-runner-exec.service` 文件，所以，可以通过如下命令查看相关服务的信息。

{% highlight text %}
----- 查看当前的状态信息
# systemctl status drone-runner-exec

----- 取消开机自动启动
# systemctl disable drone-runner-exec
{% endhighlight %}



## 使用

直接选择相关的仓库激活，在 Gogs 中的 `Settings -> Webhooks` 会自动生成相关的配置。


## 参考

* [docs.drone.io](https://docs.drone.io/) 官方的参考，可以直接查看相关的安装方式。

<!--
可以使用 Docker-Compose 进行简单的编排，这里采用 Podman 的类似编排工具。

{% highlight text %}
# podman pull drone/drone:1
{% endhighlight %}

https://github.com/containers/podman-compose
-->

{% highlight text %}
{% endhighlight %}
