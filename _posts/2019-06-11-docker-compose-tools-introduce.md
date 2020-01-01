---
title: Docker Compose 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: docker,dockerfile
description:
---


Docker Compose 是 Docker 官方编排项目，用于快速的部署应用。通过 DockerFile 可以快速生成一个容器，如果要多个容器配合使用，那么就可以通过该编排工具完成。

<!-- more -->

## 简介

通过一个单独的 `docker-compose.yml` 模板文件定义一组相关联的应用容器作为一个项目，其中有两个比较重要的概念：

* 服务 Service，一个应用容器，可以包括若干运行相同镜像的容器实例。
* 项目 Project，由一组关联的应用容器组成的一个完整业务单元，在配置文件中定义。

Compose 项目由 Python 编写，通过调用 Docker 服务提供的 API 来对容器进行管理。

在 [Github-Compose](https://github.com/docker/compose/releases) 同时提供了相关的二进制，暂时不太清楚一个 Python 怎么转换为二进制的，下载时需要注意与 Docker 版本的配套依赖关系。

### 安装

因为是 Python 编写，可以利用 PIP 进行安装，也就是 `pip install -U docker-compose` 。

<!--
另外，还提供了自动补全工具
contrib/completion/bash/docker-compose
/etc/bash_completion.d/docker-compose
https://yeasy.gitbooks.io/docker_practice/compose/
-->

### 示例

这里使用的是 [Dockerfile 简介](/post/docker-basic-concept-dockfile-introduce.html) 中的示例镜像，然后创建 `docker-compose.yml` 文件，内容如下。

{% highlight text %}
version: '3'
services:
  foobar:
    image: "foobar"
    ports:
      - "3030:2000"
{% endhighlight %}

最后，通过 `docker-compose up` 命令启动即可，可以通过 `-f` 参数指定文件。

<!--
## 参考
https://docs.docker.com/compose/gettingstarted/
-->

{% highlight text %}
{% endhighlight %}
