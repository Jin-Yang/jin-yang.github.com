---
title: Docker Volume 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: docker,dockerfile
description:
---

所谓的 Docker 数据持久化，也就是数据不会随着容器的结束而删除，有两种：A) Bind Mount 保存在主机上的某个目录；B) Volume 通过 Docker 自己管理的 Volume 。

<!-- more -->

## Bind Mount

在很早就已经开始使用了，用于将主机的目录挂载到容器中，不过在不同的类型的操作系统中无法移植，所以不会出现在 Dockerfile 中。

{% highlight text %}
# docker run -it -v $(pwd)/host-data:/container-data alpine sh
{% endhighlight %}

将主机上当前目录下的 `host-data` 目录挂载到容器的 `/container-data` 目录。

注意，本地的目录必须使用绝对路径，否则会被识别为 Volume 。

## Volume

同样也是绕过容器的文件系统，直接将数据写到主机上，只是 volume 是被容器管理的，默认所有的 volume 都在主机的 `/var/lib/docker/volumes` 目录下。

{% highlight text %}
# docker run -it -v host-volume:/container-data alpine sh
{% endhighlight %}

将 `host-volume` 挂载到容器的 `/container-data` 目录，此时会默认创建一个 `host-volume` ，然后，可以通过如下命令查看 Volume 信息。

{% highlight text %}
# docker volume inspect host-volume
[
    {
        "CreatedAt": "2019-7-29T11:18:24+08:00",
        "Driver": "local",
        "Labels": null,
        "Mountpoint": "/var/lib/docker/volumes/host-volume/_data",
        "Name": "host-volume",
        "Options": null,
        "Scope": "local"
    }
]
{% endhighlight %}

如果在启动时只使用 `-v /container-data` 参数，那么会创建一个匿名的 Volume ，一般默认的路径为 `/var/lib/docker/volumes/<ID>/_data` 。

### 常用命令

{% highlight text %}
----- 创建、删除、查看Volume
# docker volume create
# docker volume rm
# docker volume ls

----- 查看详细参数
# docker volume inspect

----- 清理不需要的Volumes
# docker volume prune
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
