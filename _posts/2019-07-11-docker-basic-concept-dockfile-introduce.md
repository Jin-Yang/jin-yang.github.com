---
title: Dockerfile 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: docker,dockerfile
description:
---

Dockerfile 可以用来创建一个镜像，允许在该文件中调用一些命令行中的命令，这样可以利用一个基础镜像，然后附加一些基础操作，最后直接自动生成新镜像。

<!-- more -->

## 简介

用于表示 docker 镜像生成过程的文件，如果在某个目录下有名为 Dockfile 的文件，那么通过 `docker build --tag name:tag .` 命令生成镜像，其中 `name` 是镜像名称，而 `tag` 就是镜像的版本或者是标签号，默认是 `lastest` 。

从 Docker 1.10 起，在执行 `COPY`、`ADD` 和 `RUN` 语句时，会在镜像中添加新层。

### 基本指令

基本指令有十三个。

{% highlight text %}
FROM <image>
    指定构建镜像的基础源镜像
MAINTAINER <name> <email>
    镜像的创建者和邮箱
RUN <command> <param1> ... <paramN>
    执行命令
CMD <command> <param1> ... <paramN>
    容器启动后执行的默认命令，可以在通过run命令启动的时候覆盖

EXPOSE、ENV、ADD、COPY、ENTRYPOINT、VOLUME、USER、WORKDIR、ONBUILD。下面对这些指令的用法一一说明。
{% endhighlight %}

## 示例

这里通过 BusyBox 中的 nc 命令作为一个 TCP 的 echo 服务器，通过本地的 `3030` 端口访问，容器内部监听 `2000` 。

对应的 Dockerfile 文件内容如下。

{% highlight text %}
FROM busybox
CMD ["nc", "-lk", "-p", "2000"]
{% endhighlight %}

然后可以通过如下方式进行测试。

{% highlight text %}
----- 构建镜像
# docker build -t foobar .
# docker images

----- 启动新创建的镜像后台运行，然后本地建立连接
# docker run -d -p 3030:2000 foobar
# nc 127.1 3030

----- 查看容器的标准输出
# docker logs -f e47ac47bd9af

----- 连接到容器，后者需要确保在执行sh命令
# docker exec -it e47ac47bd9af /bin/bash
# docker attach e47ac47bd9af
{% endhighlight %}


<!--
docker建立最简单最小的helloworld镜像
https://blog.csdn.net/u012819339/article/details/80007919

各个组件之间的关系
https://www.cnblogs.com/sparkdev/p/9129334.html
https://www.infoq.cn/article/2017/02/Docker-Containerd-RunC
-->

{% highlight text %}
{% endhighlight %}
