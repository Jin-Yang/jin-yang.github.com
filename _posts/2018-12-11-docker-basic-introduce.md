---
title: Docker 使用简介
layout: post
comments: true
language: chinese
category: [program]
keywords: lock free,queue
description:
---

从 Docker 1.11 开始，Docker 容器运行已经不是简单的通过 Docker Daemon 启动，而是集成了 containerd、runC 等多个组件。

在 Docker 服务启动之后，可以看到系统上启动了 dockerd、containerd 等进程。

<!-- more -->

## 简介

在安装了 Docker 之后，比较重要的组件主要有如下几种：

* `docker` 一个客户端工具，用来把用户的请求发送给 dockerd；
* `dockerd` 一般也会被称为 docker engine，在后台运行；
* `docker-containerd` 管理 Docker 的核心组件；
* `docker-containerd-shim` 容器的运行时载体；
* `runc` 真正运行的容器。

各个模块的组件如下。

![docker runc]({{ site.url }}/images/docker/docker-runc-arch.png "docker runc"){: .pull-center width="70%" }

### 安装

可以通过安装 YUM 安装。

{% highlight text %}
----- 如有需要，删除老版本
# yum remove docker docker-client docker-client-latest docker-common docker-latest \
	docker-latest-logrotate docker-logrotate docker-engine

----- 配置阿里云的源
# yum install -y yum-utils device-mapper-persistent-data lvm2
# yum-config-manager --add-repo https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo

----- 安装包
# yum install docker-ce docker-ce-cli containerd.io
{% endhighlight %}

在 CentOS 8 中，如果使用 `yum-config-manager` 需要安装 `dnf-utils` 包。

#### 二进制

或者下载 [静态二进制文件](https://mirrors.aliyun.com/docker-ce/linux/static/stable/x86_64/)，直接复制到 `/usr/bin` 目录下，并将 [docker.service](/reference/linux/docker.service) [docker.socket](/reference/linux/docker.socket) [containerd.service](/reference/linux/containerd.service) 几个服务添加到 `/usr/lib/systemd/system/` 目录下。

正常来说，可以将二进制文件放到 `/opt/docker` 目录下，但是在执行 `run` 命令的时候会报 `shim` 命令在 `PATH` 中不存在。

### 启动服务

{% highlight text %}
# yum install docker
# systemctl start docker
{% endhighlight %}

然后通过 `docker run hello-world` 进行测试。


### 仓库地址

在国内的仓库 `registry.docker-cn.com` 中包含了流行的公有镜像。私有镜像仍需要从 Docker Hub 镜像库中拉取。

{% highlight text %}
----- 通过命令行拉取
$ docker pull registry.docker-cn.com/library/ubuntu:16.04
{% endhighlight %}

也可以在启动的时候通过 `--registry-mirror` 参数指定，然后直接使用 `library/ubuntu` 获取。

还可以在配置文件 `/etc/docker/daemon.json` 文件并添加上 `registry-mirrors` 配置。

{% highlight text %}
{
	"registry-mirrors": ["https://registry.docker-cn.com"]
}
{% endhighlight %}

### 启动参数

可以在启动时通过参数定制，例如.

* `--graph=/data/docker` 指定存储位置，默认在 `/var/lib/docker` 目录下；
* `--storage-driver=overlay2` 使用存储驱动；
* `--registry-mirror=https://foobar.com` 镜像地址；
* `--insecure-registry=http://foobar.com` 指定非安全的镜像仓库地址。

## 常用命令

可以通过 `man docker` 查看帮助，利用 `man docker-run` 查看子命令帮助文档。

Docker 提供了一个 Hello-World 镜像可以用来测试基本环境安装是否成功，另外，还要一些常用的较小镜像，例如 BusyBox(1M+)、Alpine Linux (使用 musl libc 和 busybox 减小体积，只有 5M+ ) 。

{% highlight text %}
----- 查找镜像
# docker search centos

----- 下载镜像，可以通过 :NAME.TAG 指定版本号以及Tag信息
# docker pull docker.io/hello-world
# docker pull docker.io/centos

----- 查看镜像，及其详细信息
# docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
docker.io/centos    latest              67fa590cfc1c        4 weeks ago         202 MB
# docker inspect 67fa590cfc1c

----- 镜像的构建信息，包括了含有几层
# docker history busybox

----- 删除镜像，可以通过REPO或者ID指定
# docker image rm 67fa590cfc1c

----- 重命名镜像，通过ID指定
# docker tag 67fa590cfc1c centos


----- 运行容器，指定名称为REPOSITORY:TAG
# docker run hello-world
# docker run -it docker.io/centos /bin/bash

----- 后台运行
# docker run -d -it --name centos centos /bin/bash

----- 查看正在运行的容器
# docker ps

----- 链接到运行的容器，需要指定CONTAINER ID
# docker start 5328dfa90416
# docker stop 5328dfa90416
# docker attach 5328dfa90416
# docker container rm 5328dfa90416

----- 将已有的镜像保存，然后本地加载镜像
$ docker save -o busybox.tar busybox:latest
$ docker load -i busybox.tar
{% endhighlight %}

在通过 `run` 命令启动时，常用的参数如下。

{% highlight text %}
-i, --interactive=true|false   保持STDIN打开，也就是交互方式
-t, --tty=true|false           是否分配一个tty终端
-d, --detach=true|false        在后台运行
-p, --publish=[]               端口映射，可以查看帮助
-v, --volume=[]                文件映射，可以查看帮助
--name                         指定运行时的容器名称
{% endhighlight %}


## 镜像管理

镜像是由多层组成，而且是只读的，例如如下。

![docker images layer]({{ site.url }}/images/docker/docker-image-layers.png "docker images layer"){: .pull-center width="50%" }

在三层镜像之上的 Container 部分才是可以动态修改的，不过一旦容器销毁，对应的数据会丢失。

### 目录结构

镜像会保存在 `/var/lib/docker` 目录下，可以通过 `docker info | grep 'Docker Root Dir'` 查看，各个目录的含义介绍如下。

{% highlight text %}
builder/
buildkit/
containers/
image/                   存储镜像管理数据的目录
  |-overlay2/            子目录对应了存储驱动
    |-distribution/      远端拉到本地的镜像相关元数据
    |-imagedb/           镜像数据库
    | |-content/
    |   |-sha256/        镜像的ID信息，images命令的前12字节来自这里
    |-leveldb/           镜像每个Layer的元数据
    | |-sha256/          Layer的ChainID
    |-repositories.json  通过JSON保存的相关元数据
network/
overlay2/
plugins/
runtimes/
swarm/
tmp/
trust/
volumes/
{% endhighlight %}

其中 image 由多个 layer 组合而成的，那么有可能多个 image 会指向同一个 layer ，一个 image 指向了那些 layer 可以通过 imagedb 查看。

### 存储驱动

也就是 GraphDriver ，默认支持多种，包括了 VFS、DeviceMapper、aufs、Overlay 等，开始最常用的是 aufs ，不过随着 Linux 3.18 将 Overlay 纳入内核，默认的驱动一般就是 Overlay 。

当前的驱动可以通过 `docker info | grep 'Storage Driver'` 查看，最新的一般是 `overlay2` ，这里只需要关心 root 目录下的 image 和 overlay2 目录即可。

#### Overlay2

Overlay 也是一种 Union FS，不过只有两层：A) Upper 文件系统；B) Lower 文件系统；分别代表 Docker 的镜像层和容器层，当需要修改一个文件时，使用 CoW 将文件从只读的 Lower 复制到可写的 Upper 进行修改，结果也保存在 Upper 层。

![docker overlayfs]({{ site.url }}/images/docker/docker-overlayfs.png "docker overlayfs"){: .pull-center width="90%" }

在挂载完成之后，可以通过 `mount | grep overlay` 查看其挂载的参数。

## 私有镜像

所有镜像最终以 `tar.gz` 方式静态存储在服务器端，这种存储适用于对象存储而不是块存储。

默认的私有镜像通过可以通过 `docker-registry` 包进行安装，详细的安装步骤可以参考 [Deploy a registry server](https://docs.docker.com/registry/deploying/) 中的详细介绍。

{% highlight text %}
----- 下载仓库镜像
# docker pull registry

----- 运行镜像，并将本机的/opt/docker/registry映射到容器中
# docker run -d -p 5000:5000 --restart=always --name=registry \
    -v /opt/docker/registry:/var/lib/registry registry

----- 验证是否创建成功，正常应该返回{}
# curl http://127.0.0.1:5000/v2/
{% endhighlight %}

Docker 在与 Docker Registry 进行交互时默认使用 https，但是当前的镜像仓库仅提供了 http ，需要将本地仓库添加为不安全仓库。

{% highlight text %}
# cat /etc/docker/daemon.json
{
	"registry-mirrors": [
		"https://registry.docker-cn.com"
	],
	"insecure-registries": [
		"127.0.0.1:5000"
	]
}
{% endhighlight %}

然后将本地的 busybox 添加到新搭建的仓库。

{% highlight text %}
----- 查看当前的镜像
# docker images

----- 修改某个镜像的tag，例如busybox，并将其添加到本地
# docker tag busybox:latest 127.0.0.1:5000/busybox:latest
# docker push 127.0.0.1:5000/busybox:latest
{% endhighlight %}

然后调用 REST API 查看镜像信息。

{% highlight text %}
----- 查询私有仓库镜像列表
# curl http://127.0.0.1:5000/v2/_catalog

----- 查询busybox镜像的标签列表
# curl http://127.0.0.1:5000/v2/busybox/tags/list

----- 从私有仓库拉取镜像
# docker pull 127.0.0.1:5000/busybox:latest
{% endhighlight %}


## 资源清理

随着 Docker 的运行，会占用越来越多的资源，比较常见的是磁盘资源，对于一些无用的镜像、容器、网络、数据卷实际可以直接删除。

可以通过如下命令查看当前使用的资源。

{% highlight text %}
----- 所有的容器信息，不带-a参数之列举出运行的容器
# docker container ls -a

----- 镜像信息，通过-a会列举出中间的镜像层
# docker image ls -a

----- 列出数据卷、网络信息、系统信息
# docker volume ls
# docker network ls
# docker info
{% endhighlight %}

默认提供了 `docker system prune` 命令来删除已经停止的容器、dangling 镜像、为被容器引用的 network 以及构建过程中的 cache 。

注意，为了安全，默认是不会删除为被任何容器引用的数据卷，如果需要删除，则应该显示指定 `--volumns` 参数。

{% highlight text %}
# docker system prune --all --force --volumns
{% endhighlight %}

### dangling images

所谓的 dangling images ，可以被理解为未被任何镜像引用的镜像，当通过 `docker image ls` 查看时，会在 `TAG` 中显示 `<none>` 。

例如，重新构建镜像之后，之前依赖的镜像就变成了 dangling images 。

另外，还有 `REPOSITOY` 和 `TAG` 都是 `<none>` 的镜像，一般是其它镜像的依赖层。

## 常见问题



### 容器已经在运行

报错详细信息为 `The container name "XXX" is already in use by container` 。

{% highlight text %}
----- 查看所有的容器信息，并根据ID删除
# docker ps -a
# docker rm 36bceeae4b22
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
