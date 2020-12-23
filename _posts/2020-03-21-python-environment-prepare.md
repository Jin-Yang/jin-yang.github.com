---
title: Python 基本环境准备
layout: post
comments: true
language: chinese
tag: [Program, Python]
keywords: Python, virtualenv, pip, Windows, Linux, 虚拟环境
description: Python 底层通过虚拟机屏蔽掉了 OS 的差异，所以，需要在不同的平台上安装 Python 虚拟机的基础环境，而且，安装完之后，为了方便使用可以很容易虚拟出独立的开发环境，这里详细介绍其使用方式。
---

与 Java 类似，Python 底层通过一个虚拟机运行，而且已经移植到了多个平台上，也就是，需要根据不同的平台安装虚拟机，实际上最主要的就是 python 可执行文件。

现在大多数安装的都是 CPython ，也就是 C 语言实现的 Python ，通常，直接使用安装包安装即可。

这里详细介绍如何安装环境，使用虚拟环境等等。

<!-- more -->

## 简介

可以从官方 [www.python.org](https://www.python.org/downloads/) 下载所需平台安装包，当然，如果需要也可以从 [Source Releases](https://www.python.org/downloads/source/) 源码安装。

对于 Linux 来说，Python 是最基础的软件包，很多的一些系统工具都需要依赖 Python ，所以，必定存在 Python 环境，关键是是否是所需的版本。

而 Windows 默认是不会安装的，可以下载安装包，按照正常的软件包安装即可。

### 多版本

对于 CentOS-7 来说，官方默认仓库是没有对应的 RPM 包的，一般来说是需要用户从源码开始编译安装的，这里直接从 epel 安装对应的包，当然可能不是最新的。

#### 二进制安装

通过如下命令查看并安装对应的包，如下以 3.6 版本为例。

{% highlight text %}
# yum list --enablerepo=epel all | grep python3
# yum install --enablerepo=epel python36 python36-pip
{% endhighlight %}

此时，对应的二进制文件为 `/usr/bin/python36` ，库安装到了 `/usr/lib64/python3.6/` 目录下。而默认使用版本，实际上是通过符号链接指定的，例如如下的示例。

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

在升级 Python3 的时候，因为语法不支持，需要评估对系统工具的影响，例如对于 YUM 来说，因为其要用到 python2 才能执行，直接覆盖会导致 yum 不能正常使用。

{% highlight text %}
# cat /usr/bin/yum
#!/usr/bin/python ----> #!/usr/bin/python2

# cat /usr/libexec/urlgrabber-ext-down
#!/usr/bin/python ----> #!/usr/bin/python2
{% endhighlight %}

## 虚拟环境

在开发 Python 程序时，经常会遇到要使用不同版本的问题，例如 Python 的 3.4、2.7 甚至是 2.6 版本，甚至是多个应用场景，例如机器学习、Web 框架等等，而通过 pip 安装时，一般只会安装到标准路径下，这会导致包特别混乱，而 Python 构建独立环境又特别方便，所以，这里介绍下如何使用独立的虚拟环境。

这里所谓的虚拟环境，保证了其 Python 可执行文件的版本独立，而且安装的包也在该环境对应目录下，所以也是相对独立的。

其原理详细可以查看 [Python 虚拟环境工作原理](/post/python-virtual-environment-details.html) 中的介绍。

### 环境准备

在 Python 2 中，通常使用 pyenv 用于在多个不同系统 Python 版本之间切换，而使用 virtualenv 用于创建独立的开发环境。

其中 pyenv 是一堆的 Bash 脚本，安装方式可以参考 [github pyenv](https://github.com/pyenv/pyenv) 中的介绍，而在 Linux 中，也可以通过上述的 alternatives 工具进行切换，所以，这里就不详细介绍了。

### virtualenv

对于 Python 2 来说，可以通过 virtualenv 来为一个应用创建一套 "隔离" 的 Python 运行环境。

直接通过 `pip install virtualenv` 安装相应的最新版本，假设要开发一个新的项目，需要一套独立的 Python 运行环境，那么就可以按照如下步骤操作：

{% highlight text %}
----- 创建目录
$ mkdir /tmp/project && cd /tmp/project

----- 创建一个独立的Python运行环境，命名为foobar
$ virtualenv --no-site-packages foobar
New python executable in /tmp/project/foobar/bin/python2
Also creating executable in /tmp/project/foobar/bin/python
Installing setuptools, pip, wheel...done.
{% endhighlight %}

通过命令 `virtualenv` 可以创建一个独立的 Python 运行环境，参数 `--no-site-packages` 表示已安装到系统环境中的所有第三方包都不会复制过来，这样，我们就得到了一个不带任何第三方包的 "干净" 的 Python 运行环境。

新建的 Python 环境被放到当前目录下的 foobar 目录，此时可以通过 `source foobar/bin/activate` 切换到这个 Python 环境；当然，不同的终端如 csh、fish 需要执行不同的脚本。

此时的终端提示符会添加 `(foobar)` 前缀，然后可以通过 `pip install jinja2` 类似的命令安装三方包，需要退出时执行 `deactivate` 命令。

注意，默认会使用 Python2 的版本，可以通过 `--python=python3.6` 参数指定版本号。

### venv

在 Python 3.3 以上的版本，在基础安装包中已经集成了基础的虚拟环境创建工具 [venv](https://docs.python.org/3/library/venv.html) ，可以代替之前的 virtualenv 工具。

```
----- 在子目录下创建名称为machine的虚拟环境
$ python -m venv machine
```

有些参数可以通过 `python -m venv -h` 查看详细的帮助信息，接着是要激活创建的环境。

```
----- 对于Linux、Unix、Mac等操作系统
$ source machine/bin/activate

----- 如果是在Windows CMD中
D:> machine/Scripts/activate.bat

----- 如果在Windows PowerShell下
PS D:> machine/Scripts/Activate.ps1
```

在激活虚拟环境后，命令行会提示当前虚拟环境的名称。

## 包管理

Python 中可以通过 PIP 来自动管理依赖包，如果机器上没有安装，可以通过包管理工具安装，例如 CentOS 中的 yum 。

如果是离线，那么可以直接从 [pypi.org](https://pypi.org/project/pip/) 上下载源码包，然后通过 `python setup.py install` 命令安装，其中依赖的 `setuptools` 也可以直接通过上述的源码安装方式。

### Python 3

如上的环境准备过程中，当通过 `yum install python36-pip` 安装了对应 V3 版本，如果没有连接到 pip 命令，此时对应的命令行是 `pip3` 而非 `pip` ，也就是需要保证 pip 是相同的版本。

{% highlight text %}
# pip3 install --upgrade pip3
{% endhighlight %}

关于 PIP 的详细使用方式可以参考 [Python 包安装工具](/post/python-install-package-tools.html) 。

<!--
https://anjingwd.github.io/AnJingwd.github.io/2017/08/03/python%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA/
-->

{% highlight text %}
{% endhighlight %}
