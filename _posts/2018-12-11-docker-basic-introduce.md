---
title: Docker 使用简介
layout: post
comments: true
language: chinese
category: [program]
keywords: lock free,queue
description:
---


<!-- more -->



{% highlight text %}
# yum install docker
# systemctl start docker
{% endhighlight %}

可以通过 `man docker` 查看帮助，利用 `man docker-run` 查看子命令帮助文档。

{% highlight text %}
----- 查找镜像
# docker search centos
----- 下载镜像，可以通过 :XX.X 指定版本号
# docker pull docker.io/hello-world
# docker pull docker.io/centos
----- 查看镜像
# docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
docker.io/centos    latest              67fa590cfc1c        4 weeks ago         202 MB
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

   -i, --interactive=true|false 保持STDIN打开，也就是交互方式
   -t, --tty=true|false 是否分配一个tty终端
   -d, --detach=true|false 在后台运行
   -p, --publish=[] 端口映射，可以查看帮助
   -v, --volume=[] 文件映射，可以查看帮助
   --name 指定运行时的容器名称

{% endhighlight %}



<!--
docker建立最简单最小的helloworld镜像
https://blog.csdn.net/u012819339/article/details/80007919
-->

{% highlight text %}
{% endhighlight %}
