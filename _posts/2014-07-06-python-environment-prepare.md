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

## 多版本

默认使用 Python ，如果要使用 Python2 则需要显示指定。

官方默认仓库是没有对应的 RPM 包的，一般来说是需要用户从源码开始编译安装的，这里直接从 epel 安装对应的包，当然可能不是最新的。

### 二进制安装

通过如下命令查看并安装对应的包，如下以 3.6 版本为例。

{% highlight text %}
# yum list --enablerepo=epel all | grep python3
# yum install --enablerepo=epel python36 python36-pip
{% endhighlight %}

此时，对应的二进制文件为 `/usr/bin/python36` ，库安装到了 `/usr/lib64/python3.6/` 目录下。

默认使用的版本，实际上是通过符号链接指定的，例如如下的示例。

{% highlight text %}
# ls /usr/bin/python* -alh
lrwxrwxrwx 1 root root    7 Jun 27 21:52 /usr/bin/python -> python2
lrwxrwxrwx 1 root root    9 Jun 27 21:52 /usr/bin/python2 -> python2.7
-rwxr-xr-x 1 root root 7.1K Jun 21 04:28 /usr/bin/python2.7
-rwxr-xr-x 1 root root 1.8K Jun 21 04:27 /usr/bin/python2.7-config
lrwxrwxrwx 1 root root   16 Jun 27 21:52 /usr/bin/python2-config -> python2.7-config
lrwxrwxrwx 1 root root    9 Jul  9 23:29 /usr/bin/python3 -> python3.6
lrwxrwxrwx 1 root root   18 Jul  9 23:29 /usr/bin/python36 -> /usr/bin/python3.6
-rwxr-xr-x 2 root root  12K Apr 26 05:05 /usr/bin/python3.6
-rwxr-xr-x 2 root root  12K Apr 26 05:05 /usr/bin/python3.6m
lrwxrwxrwx 1 root root   14 Jun 27 21:52 /usr/bin/python-config -> python2-config
{% endhighlight %}

在替换完之后，可以通过 `python --version` 查看确认当前使用的版本号。

注意，Python3 和 Python2 所使用库保存的路径不同，所以，之前在 Python2 中安装的库，在 Python3 中无法使用，如果要使用 PIP 安装，则需要安装对应的版本。

可以通过 `strace python -c 'import numpy'` 命令确定其搜索路径。

### 其它

#### 老版本依赖

更改 yum 配置，因为其要用到 python2 才能执行，否则会导致 yum 不能正常使用。

{% highlight text %}
# cat /usr/bin/yum
#!/usr/bin/python ----> #!/usr/bin/python2

# cat /usr/libexec/urlgrabber-ext-down
#!/usr/bin/python ----> #!/usr/bin/python2
{% endhighlight %}

## 包管理

Python 中可以通过 PIP 来自动管理依赖包，如果机器上没有安装，可以通过包管理工具安装，例如 CentOS 中的 yum 。

如果是离线，那么可以直接从 [pypi.org](https://pypi.org/project/pip/) 上下载源码包，然后通过 `python setup.py install` 命令安装，其中依赖的 `setuptools` 也可以直接通过上述的源码安装方式。

#### Windows 安装

在 [www.python.org/downloads](https://www.python.org/downloads/) 中选择相关的 Windows 版本下载，PIP 的安装与 Linux 中类似，直接从 [pypi.python.org](https://pypi.python.org/pypi/pip) 上下载，然后在命令行中采用上述的方式安装。

### PIP

如上的环境准备过程中，会通过 `yum install python36-pip` 安装了对应 V3 版本，此时对应的命令行是 `pip3` 而非 `pip` 。

{% highlight text %}
# pip3 install --upgrade pip3
{% endhighlight %}

关于 PIP 的详细使用方式可以参考 [Python 常用工具](/post/python-most-useful-tools.html) 。

<!--
https://anjingwd.github.io/AnJingwd.github.io/2017/08/03/python%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA/
-->

{% highlight text %}
{% endhighlight %}
