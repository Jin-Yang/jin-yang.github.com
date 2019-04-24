---
title: Golang 常用工具
layout: post
comments: true
language: chinese
category: [program,golang,linux]
keywords: golang,go,简介
description: 这里简单介绍下 GoLang 中常用的工具，包括了 GVM、GDM、Goreman 等。
---

这里简单介绍下 GoLang 中常用的工具，包括了 GVM、GDM、Goreman 等。

<!-- more -->

简单介绍下一些常见的工具。

![golang logo]({{ site.url }}/images/go/golang-logo-2.png "golang logo"){: .pull-center width="50%" }

## GVM (Go Version Manager)

工程项目保存在 [Github moovweb/gvm](https://github.com/moovweb/gvm) 中，这是一套使用 Bash 编写的脚本工具，支持多个 go 编译器共存，方便进行切换。

首先，需要安装一些常见的工具，这个是在 scripts/gvm-check 脚本中检查。

{% highlight text %}
$ yum install git binutils bison gcc make curl
{% endhighlight %}

如果感兴趣，可以直接查看其源码，其实很简单，只是设置好环境变量即可，接下来看看如何安装，并使用。

{% highlight text %}
----- 下载安装脚本并直接执行
$ bash < <(curl -s -S -L https://raw.githubusercontent.com/moovweb/gvm/master/binscripts/gvm-installer)
{% endhighlight %}

默认会直接将上述的代码仓库 Clone 到 ~/.gvm 目录下，并在 ~/.bashrc 中添加一行脚本执行命令，用来设置环境变量。

### 安装

其中，每个子命令执行的脚本，实际是对应到 script 目录下相应名称的脚本；另外，建议可以直接下载该工具，然后下载相应的 go 版本。

{% highlight text %}
$ gvm install go1.4             ← 安装相应的版本
$ gvm use go1.4 --default       ← 设置使用相应的版本，可同时设置为默认
{% endhighlight %}

注意，go-1.5 之后的版本，将 C 编译器使用 go 编译器直接替换掉了，这也就意味着，如果你要安装 1.5 之后的版本，需要先安装 1.4 版的才可以。

这也就是说，需要如下两步：1) 用 gcc 编译生成 go-1.4；2) 用 go-1.4 编译大于 go-1.5 的版本。

当然，上述的方式是通过源码安装的，假设，你已经通过 yum 安装好了 go ，那么可以通过如下方式进行编译。实际上就是将 GOROOT_BOOTSTRAP 环境变量设置为安装目录，如下所示。

{% highlight text %}
$ export GOROOT_BOOTSTRAP=/usr/lib/golang
{% endhighlight %}

实际上，安装完之后，相关的数据会保存在如下的目录中 (假设版本为go1.5) 。

{% highlight text %}
~/.gvm/gos/go1.5
~/.gvm/pkgsets/go1.5
~/.gvm/environments/go1.5
{% endhighlight %}

简单来说，**需要设置环境变量，会保存在 ~/.gvm/environments/ 中**，可以用来设置相应的变量；例如，你可以将 GOPATH 设置为自己想放置的目录。

### 常用命令

如下统计常用的命令。

{% highlight text %}
$ gvm list                      ← 查看当前所有版本
$ gvm install go1.4             ← 安装
$ gvm use go1.4 --default       ← 设置当前使用版本
{% endhighlight %}

## GDM (Go Dependency Manager)

源码可以从 [Github sparrc/gdm](https://github.com/sparrc/gdm) 上下载，其工作原理很简单，可以通过 save 命令会生成 Godeps 文件，保存了相关的包及其版本。

在新的环境中，通过 restore 命令可以下载代码包。

## Goreman

Goreman 是一个 Foreman 的 Go 语言的克隆版本，一般在开发过程中的调试多个进程时使用。目前微服务、分布式架构很流行，应用依赖于其它应用或者集群化，这样本地调试时会很不方便。

通过 Goreman 同时批量管理多个进程，可以方便开发。

**注意** 一般用于开发环境，不要用于生产环境，如果要线上使用，建议用 monit、supervisor 等更成熟稳定的工具。

### 安装使用

可以直接从 [github mattn/goreman](https://github.com/mattn/goreman/releases) 上下载已经编译好的二进制文件。

在运行时，goreman 会独占当前 shell 窗口，可以通过 `Ctrl-C` 退出运行状态，如果要执行其它的命令，则需要重新开启一个 shell 执行。

常用操作如下。

{% highlight text %}
----- 检查配置文件是否正确
$ goreman check

----- 启动服务，默认使用当前目录下的配置文件，也可以通过-f指定
$ goreman start
$ goreman -f myprocfile start

----- 查看当前应用的运行状态，如果正在运行则会显示一个星号
$ goreman run status

----- 停止某个进程
$ goreman run stop PROCESS_NAME

----- 启动某个已停止进程
$ goreman run start PROCESS_NAME

----- 重启某个进程
$ goreman run restart PROCESS_NAME

----- 查看进程的状态
$ goreman run status
{% endhighlight %}



{% highlight text %}
{% endhighlight %}
