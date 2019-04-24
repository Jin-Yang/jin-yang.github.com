---
title: Python 环境准备
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

## 包管理

Python 中可以通过 PIP 来自动管理依赖包，如果机器上没有安装，可以通过包管理工具安装，例如 CentOS 中的 yum 。

如果是离线，那么可以直接从 [pypi.org](https://pypi.org/project/pip/) 上下载源码包，然后通过 `python setup.py install` 命令安装，其中依赖的 `setuptools` 也可以直接通过上述的源码安装方式。

### Windows 安装

在 [www.python.org/downloads](https://www.python.org/downloads/) 中选择相关的 Windows 版本下载，PIP 的安装与 Linux 中类似，直接从 [pypi.python.org](https://pypi.python.org/pypi/pip) 上下载，然后在命令行中采用上述的方式安装。

{% highlight text %}
{% endhighlight %}
