---
title: Python 常用工具
layout: post
comments: true
language: chinese
category: [python]
keywords: python,virtualenv,pyenv,pip
description: 简单介绍一下 Python 中常用的一些工具，及其使用方法。
---

简单介绍一下 Python 中常用的一些工具，及其使用方法。

<!-- more -->

## 包安装

Python 中可以通过 PIP 来自动管理依赖包，如果机器上没有安装，那么可以直接从 [pypi.org](https://pypi.org/project/pip/) 上下载源码包，然后通过 `python setup.py install` 命令安装，其中依赖的 `setuptools` 也可以直接通过上述的源码安装方式。

很多 Python 包可以直接从 [www.pypi.org](https://pypi.org/project/) 上获取源码包，例如 ipython 包，当然需要注意版本号，部分可能只支持 Python3。

例如，如果 python 的环境为 2.X 版本，那么可以通过 `pip install ipython==5.8.0` 安装。

### 安装方式

如上，最简单的是直接通过 `pip` 命令行安装，不过需要联网才可以，否则无法下载。

{% highlight text %}
# pip install scikit-learn
# easy_install scikit-learn
{% endhighlight %}

另外，一种方式是直接离线下载完之后，通过如下命令安装。

{% highlight text %}
$ python setup.py build
$ python setup.py install
{% endhighlight %}

### PIP 简介

Python 中的包管理工具，常见操作如下。

{% highlight text %}
----- 列出已安装的包，freeze一般是老版本使用，可以用于导出requirements.txt
# pip freeze
# pip list

----- 在线安装，也可以指定具体的版本要求
# pip install <PKG-Name>
# pip install -r requirements.txt
# pip install "django==1.9"
# pip install "django>1.9"
# pip install "django><1.9"

----- 安装本地包
# pip install <Your/Source/Directory>

----- 卸载包
# pip uninstall <PKG-Name>
# pip uninstall -r requirements.txt

----- 升级包，可以自升级
# pip install -U <PKG-Name>

----- 显示包所在的目录
# pip show -f <PKG-Name>

----- 搜索包
# pip search <KEY-Word>

----- 查询可升级的包
# pip list -o

----- 只下载包而不安装
# pip install <PKG-Name> -d <Directory>
# pip install -d <Directory> -r requirements.txt

----- 指定下载的源地址
# pip install <PKG-Name> -i http://pypi.v2ex.com/simple
{% endhighlight %}

在查看已经安装包时，不会显示 `python setup.py install` 这种方式安装的包，可通过 `python -c "help('modules')"` 查看已经安装的包，不过没有版本号；据说有个 `yolk` 工具可以使用，没有验证过。

#### 指定全局安装源

在 `*nix` 和 `MacOS` 中，配置文件为 `$HOME/.pip/pip.conf` ，一般的内容如下。

{% highlight text %}
[global]
timeout = 6000
index-url = http://pypi.douban.com/simple
{% endhighlight %}

常用的源有：

* 豆瓣 https://pypi.douban.com/simple
* 中国科学技术大学 https://mirrors.ustc.edu.cn/pypi/web/simple/
* 清华大学TUNA https://pypi.tuna.tsinghua.edu.cn/simple

<!--
https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple/
-->

#### 代理设置

可以通过如下方法设置其代理，也就是 `export https_proxy='https://USR:PASS@HOSTNAME:PORT'`，注意在使用时密码尽量不要带有类似 `#` `$` `@` 等这种特殊的字符，否则可能会鉴权失败。

## pyenv & virtualenv

一般会使用着两个工具搭建多 Python 环境，其中 pyenv 用于在多个不同系统 Python 版本之间切换，而 virtualenv 用于创建独立的开发环境。

### pyenv

这个是一堆的 Bash 脚本，安装方式可以参考 [github pyenv](https://github.com/pyenv/pyenv) 中的介绍。

### virtualenv

在开发 Python 程序时，经常会遇到要使用多个环境甚至时不同版本的问题，例如 Python 的 3.4、2.7 甚至是 2.6 版本，而通过 pip 安装时，一般只会安装到标准路径下。

virtualenv 就是用来为一个应用创建一套 "隔离" 的 Python 运行环境。

可以直接通过 `pip install virtualenv` 安装相应的最新版本，假设要开发一个新的项目，需要一套独立的 Python 运行环境，那么就可以按照如下步骤操作：

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

<!--
## Fabric
http://www.fabfile.org/
-->


{% highlight python %}
{% endhighlight %}
