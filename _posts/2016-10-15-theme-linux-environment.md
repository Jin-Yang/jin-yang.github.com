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

#### Miscellaneous

简单记录一些乱七八糟的东西。

* [CentOS 安装与配置](/post/centos-config-from-scratch.html)，简单介绍 CentOS 在安装时需要作的一些常用配置。
* [Linux 自动编译](/post/linux-package.html)，简单介绍 Linux 下的自动编译工具，如 Makefile、Autotools 等。
* [Linux 常用技巧](/post/linux-tips.html)，简单记录了一些在 Linux 中常用的技巧。
* [Linux 绘图工具](/post/linux-gnuplot.html)，这是一个命令行驱动的绘图工具，支持多个平台。
* [Linux 时间同步](/post/linux-sync-time.html)，介绍部分与时间相关的概念，例如时区、闰秒、夏令时、NTP 等。
* [Systemd 使用简介](/post/linux-systemd.html)，一般新发行版本采用的是 systemd，在此简单介绍下。
* [Rsync & Inotify](/post/rsync-inotify.html)，通过这两个命令可以快速实现文件的同步。


<!--
* [Bash 安全编程](/post/linux-bash-pitfalls_init.html)，
-->

## Bash 相关

* [Bash 相关内容](/post/linux-bash-related-stuff.html)，一些与 Bash 相关的内容，如命令执行顺序、配置文件等。
* [Bash 重定向](/post/linux-bash-redirect-details.html)，简单介绍 Bash 中 IO 重定相关的内容，包括其使用方法。
* [Bash 自动补全](/post/linux-bash-auto-completion-introduce.html)，介绍 Bash 的自动补全原理，以及如何实现自己的补全功能。

### 常用命令

* [Linux 文本处理](/post/linux-commands-text.html)，简单介绍下 Linux 常用的文本处理方式。
* [Linux AWK](/post/linux-commands-text-awk-introduce.html) 擅长从格式化报文、大文本文件中提取数据。
* [Linux SED](/post/linux-commands-text-sed-introduce.html) 一个精简的、非交互式的编辑器，可以提供类似 VIM 的编辑能力。
* [Linux 杂项](/post/linux-commands-tips.html)，常用命令，如 find 。

## 常用技巧

* [Linux 用户管理](/post/linux-user-management.html)，简单介绍 Linux 用户管理相关的内容。

## 开发环境

* [TMUX 简介](/post/tmux-introduce.html) 终端复用工具，类似 screen 但是更加方便使用，不过更加高端。
* [VIM 基本功能](/post/linux-vim-introduce.html) 一功能强大、高度可定制的文本编辑器，介绍其安装和基本用法。
* [VIM 插件使用](/post/linux-vim-third-plugins-introduce.html) 一些常用的三方组件安装、配置、使用方法。

## Linux 运维工具

* [RPM 包制作](/post/linux-create-rpm-package.html)，如何在 CentOS 中创建 RPM 包。
* [你所不知道的定时任务](/post/details-about-cronie.html)，也就是 Linux 中如何使用 crontab，以及常见错误。
* [Logrotate 使用方法](/post/logrotate-usage.html)，一个不错的日志切割管理程序。
* [Linux 后台服务管理](/post/linux-daemon-tools.html)，介绍目前常用的后台服务管理，例如 Monit、supervisor、goreman。

{% highlight text %}
{% endhighlight %}
