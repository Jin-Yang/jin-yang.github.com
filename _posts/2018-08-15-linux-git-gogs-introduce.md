---
title: Gogs 仓库使用
layout: post
comments: true
language: chinese
category: [misc]
keywords: git,ssh,gogs,openssh
description: Gogs 的功能类似于 GitHub 或者 GitLab ，不过相比来说是一款极易搭建的 Git 服务。其目标是打造一个最简单、最快速和最轻松的方式搭建自助 Git 服务，通过 Go 语言开发，使得 Gogs 能够通过独立二进制分发，支持平台包括了 Linux、Mac OS、Windows 以及 ARM 。
---

Gogs 的功能类似于 GitHub 或者 GitLab ，不过相比来说是一款极易搭建的 Git 服务。

其目标是打造一个最简单、最快速和最轻松的方式搭建自助 Git 服务，通过 Go 语言开发，使得 Gogs 能够通过独立二进制分发，支持平台包括了 Linux、Mac OS、Windows 以及 ARM 。

<!-- more -->

![git gogs logo]({{ site.url }}/images/linux/git-gogs-logo.jpg "git gogs logo"){: .pull-center width="70%" }

## 简介

这里使用的是独立的 OpenSSH 服务，而非内置的 ssh ，同时需要创建一个独立的 git 帐号用来保存管理。

{% highlight text %}
----- 启动OpenSSH服务以及Gogs
# systemctl start sshd
# systemctl start gogs
{% endhighlight %}

### 创建用户

在使用时增加 git 用户，用于处理 ssh 相关的内容，直接执行 `adduser git` 命令即可。

### 安装

可以直接从 [gogs.io](https://gogs.io/docs/installation/install_from_binary) 下载相关的二进制包，然后解压到 `/usr/local/gogs` 目录下。

{% highlight text %}
# cd /usr/local/gogs
# cp scripts/systemd/gogs.service /usr/lib/systemd/system/
{% endhighlight %}

然后编辑 `gogs.service` 文件，示例如下。

{% highlight text %}
[Unit]
Description=Gogs
After=syslog.target network.target

[Service]
Type=simple
User=git
Group=git
WorkingDirectory=/usr/local/gogs
ExecStart=/usr/local/gogs/gogs web
Restart=always
Environment=USER=git HOME=/home/git

[Install]
WantedBy=multi-user.target
{% endhighlight %}

通过 `systemctl daemon-reload` 命令加载配置文件，然后使用 `systemctl start gogs` 启动进程，此时会监听 `3000` 端口，直接通过 [http://localhost:3000](http://localhost:3000) 访问即可。

### 配置

首次启动时，会启动一个配置页面用来修改配置，如果仓库不大，直接使用 SQLite 即可。

其中服务端相关的配置如下。

{% highlight text %}
[server]
DOMAIN           = gogs.cargo.com
HTTP_PORT        = 3000
ROOT_URL         = http://gogs.cargo.com:3000/
DISABLE_SSH      = false
SSH_PORT         = 22
START_SSH_SERVER = false
OFFLINE_MODE     = false
{% endhighlight %}

注意，在创建时需要设置一个 admin 用户用于管理，例如 root 用户。

### ssh

gogs 提供了一个基本的 ssh 服务端，不过不太好用，所以还是用 OpenSSH 吧。在使用时，需要通过 `systemctl start sshd` 启动 ssh 服务端。

<!--
除此之外还需要提供如下的配置。

{% highlight text %}
SSH_ROOT_PATH    = /root/.ssh  # 因为使用的时root用户
REWRITE_AUTHORIZED_KEYS_AT_START = false
{% endhighlight %}

Host gogs.cargo.com
    #HostName gogs.cargo.com
    Port 22
    Identityfile ~/.ssh/id_git_gogs

-->

免密码登陆可以参考 [Git 使用杂项]({{ site.production_url }}/post/git-tips.html) 中的相关介绍。

## 参考

<!--
https://segmentfault.com/a/1190000008733238
-->

{% highlight text %}
{% endhighlight %}
