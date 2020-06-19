---
title: 【专题】Linux 监控专题
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## CentOS

![centos logo]({{ site.url }}/images/centos-logo.jpg "centos logo"){: .pull-center width="45%" }

Community Enterprise Operating System, CentOS 是 Linux 发行版之一，它是来自于 Red Hat Enterprise Linux 依照开源规定，而释出的源代码所编译而成。

因为出自同样的源代码，有些要求高度稳定性的服务器以 CentOS 替代商业版的 RedHat 使用。

这里简单记录一些乱七八糟的东西。

* [CentOS 安装配置](/post/centos-config-from-scratch.html) 安装时常用配置，包括了基础配置以及常用软件安装。
* [CentOS Yum 配置](/post/centos-config-yum-rpm-stuff-introduce.html) 关于 YUM 仓库的源配置以及 RPM 相关操作。
* [CentOS 内核升级](/post/centos-update-kernel-version.html) 如何使用三方仓库将内核升级到新版本。
* [Systemd 使用简介](/post/linux-systemd.html) 一般发行版本采用的是 systemd，在此简单介绍下。

### 基本概念

* [Linux 用户管理](/post/linux-user-management.html) 简单介绍 Linux 用户管理相关的内容。

### 常用工具

一些常用的工具，可以通过这些工具有效的提高处理效率。

* [RPM 包制作](/post/linux-create-rpm-package.html) 如何在 CentOS 中创建 RPM 包。
* [你所不知道的定时任务](/post/details-about-cronie.html) 也就是 Linux 中如何使用 crontab，以及常见错误。
* [Logrotate 使用方法](/post/logrotate-usage.html) 一个不错的日志切割管理程序。
* [Linux 后台服务管理](/post/linux-daemon-tools.html) 介绍目前常用的后台服务管理，例如 Monit、supervisor、goreman。
* [Linux 绘图工具](/post/linux-gnuplot.html) 一个通过命令行驱动的绘图工具，支持多个平台。
* [Rsync & Inotify](/post/rsync-inotify.html) 通过这两个命令可以快速实现文件的同步。
* [Linux Alternatives](/post/linux-tools-alternatives-command-introduce.html) 用来管理多个软件或者多个版本的实现。


### 其它

* [Linux 常用技巧](/post/linux-tips.html) 简单记录了一些在 Linux 中常用的技巧。

## Bash 相关

* [Bash 相关内容](/post/linux-bash-related-stuff.html)，一些与 Bash 相关的内容，如命令执行顺序、配置文件等。
* [Bash 重定向](/post/linux-bash-redirect-details.html)，简单介绍 Bash 中 IO 重定相关的内容，包括其使用方法。
* [Bash 自动补全](/post/linux-bash-auto-completion-introduce.html)，介绍 Bash 的自动补全原理，以及如何实现自己的补全功能。

<!--
* [Bash 安全编程](/post/linux-bash-pitfalls_init.html)，
-->

### 常用命令

* [Linux 文本处理](/post/linux-commands-text.html)，简单介绍下 Linux 常用的文本处理方式。
* [Linux AWK](/post/linux-commands-text-awk-introduce.html) 擅长从格式化报文、大文本文件中提取数据。
* [Linux SED](/post/linux-commands-text-sed-introduce.html) 一个精简的、非交互式的编辑器，可以提供类似 VIM 的编辑能力。
* [Linux 杂项](/post/linux-commands-tips.html)，常用命令，如 find 。

## 善用嘉软

<!--
https://www.calcurse.org/
http://todotxt.org/
https://www.moneymanagerex.org/news

https://www.gnucash.org/
https://sourceforge.net/projects/gnucash/
-->


{% highlight text %}
{% endhighlight %}
