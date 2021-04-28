---
title: Python 包管理机制详解介绍
layout: post
comments: true
language: chinese
tag: [SoftWare, Python]
keywords: Python, 虚拟环境, virtalenv
description: Python 中可以通过 PIP 来自动管理依赖包，当然，也可以下载源码或者二进制包，有些源码包需要编译环境，最好直接下载安装二进制包。
---

Python 中可以通过 PIP 来自动管理依赖包，当然，也可以下载源码或者二进制包，有些源码包需要编译环境，最好直接下载安装二进制包。

<!-- more -->

## 包安装

Python 中可以通过 PIP 来自动管理依赖包，如果机器上没有安装，可以通过包管理工具安装，例如 CentOS 中的 yum 。

对于三方库，如果机器上没有安装，那么可以直接从 [pypi.org](https://pypi.org/project/pip/) 上下载源码包，然后通过 `python setup.py install` 命令安装，其中依赖的 `setuptools` 也可以直接通过上述的源码安装方式。

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

Python 中的包管理工具，常见操作如下，可以包含一个 `requirements.txt` 文件，用于保存当前依赖包及其精确的版本号。

{% highlight text %}
----- 列出已安装的包，freeze一般是老版本使用，可以用于导出requirements.txt
# pip freeze > requirements.txt
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

在 `*nix` 和 `MacOS` 中，配置文件为 `$HOME/.pip/pip.conf` ，对于 Windows 保存在 `C:\Users\<USRNAME>\pip\pip.ini` 文件中，一般的内容如下。

{% highlight text %}
[global]
timeout = 6000
index-url = https://pypi.douban.com/simple
{% endhighlight %}

常用的源有：

* 豆瓣 `https://pypi.douban.com/simple`
* 中国科学技术大学 `https://mirrors.ustc.edu.cn/pypi/web/simple/`
* 清华大学TUNA `https://pypi.tuna.tsinghua.edu.cn/simple`

如果要临时指定，那么可以通过类似 `--trusted-host pypi.douban.com -i http://pypi.douban.com/simple` 参数。

<!--
查看当前源配置
pip config list
修改源之前可以将PIP升级到最新版本
pip install --upgrade pip -i + 临时源
pip config set global.index-url + 源地址
-->

#### Python 3

如上的环境准备过程中，当通过 `yum install python36-pip` 安装了对应 V3 版本，如果没有连接到 pip 命令，此时对应的命令行是 `pip3` 而非 `pip` ，也就是需要保证 pip 是相同的版本。

{% highlight text %}
# pip3 install --upgrade pip3
{% endhighlight %}

关于 PIP 的详细使用方式可以参考 [Python 包安装工具](/post/python-install-package-tools.html) 。

<!--
https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple/
-->

#### 代理设置

可以通过 `export https_proxy='https://USR:PASS@HOSTNAME:PORT'` 设置代理，注意在使用时密码尽量不要带有类似 `#` `$` `@` 等这种特殊的字符，否则可能会鉴权失败。

## 包命名格式

在安装 Python 包的时候，通常是使用 PIP 直接解决依赖问题，也就是 Wheel 包，其命名格式为。

```
{distribution}-{version}(-{build tag})?-{python tag}-{abi tag}-{platform tag}.whl
```

其中 `distribution` 为包名称，可以使用下划线，`version` 对应了版本号，剩余参数会比较麻烦，介绍如下。通过 `python tag` 标识具体 Python 的实现，有如下几种：

* `py` 只依赖 Python 语言，一般是 py2.py3 ，也就是兼容；或者 py2、py3 只兼容特定版本。
* `cp` CPython 通常使用的实现，大多数平台，使用最广泛。
* `ip` IronPython 跑在 Windows CLI 平台上的 Python 实现。
* `pp` PyPy 带 JIT 的 Python 实现。
* `jy` Jython 跑在 JVM 上的 Python 实现。

接着 `api tag` 为 API 标签，如 `cp27mu`，后面 `platform tag` 为平台信息，例 `linux_aarch64`、`linux_x86_64` 等。

#### cp27m VS. cp27mu

在 CPython 3.3 之前的版本，编译时有两个选项 `--enable-unicode=ucs2` 和 `--enable-unicode=ucs4` ，所以，很多的一些发行版本，为了兼容会对应提供 `cp27m` 和 `cp27mu` 两个版本。

### 当前支持标签

可以通过如下的方式查看当前 Python 环境所支持的 wheel 包。

```
# x86_32 查看
import wheel.pep425tags
print(wheel.pep425tags.get_supported())

# x86_64 查看
import pip
print(pip.pep425tags.get_supported())
```

其输出为 `('python Tag','abi Tag','platform tag')` 。

## 离线安装包

有时候需要离线安装包，可以直接从仓库下载二进制文件即可，也可以省去编译过程。从仓库下载对应的 `XXX.whl` 安装包，并通过 `pip install XXX.whl` 即可。另外，如果发现有 `XXX.egg` 文件，那么可以通过 `easy_install XXX.egg` 命令安装。

```
----- 先离线下载包，可以指定包或者文件
$ pip download -d /opt/pip/tmp ansible
$ pip download -d /opt/pip/tmp -r requirement.txt

----- 安装
$ pip install --no-index --find-links="/opt/pip/tmp" ansible
$ pip install --no-index --find-links="/opt/pip/tmp" -r requirements.txt
```

另外，我们可能需要在 `x86` 机器上下载 `arm` 的包，那么就需要使用如下的参数，如下是常见的方式。

```
--only-binary=:all:       \   # 只下载二进制包
--python-version 27       \   # Python 版本，也就是 2.7
--implementation cp       \   # 使用 cpython 实现，一般都是这个
--platform linux_aarch64  \   # 可以指定多个，也就是 Linux ARM 64Bits
--abi cp27mu              \   # 可以指定多个，支持的 ABI
```

<!--
pip download -r ../requirement.txt --only-binary=:all: --python-version 27 --implementation cp --platform linux_aarch64 --abi cp27mu --abi none
-->

<!--
https://www.jianshu.com/p/1855e997510c

如何自己执着PIP
https://dzone.com/articles/executable-package-pip-install


----- 列出包所有的版本号
pip install --no-deps sqlalchemy==
pip install --no-deps psutil==


到目前为止没有找到可以直接通过 PIP 查看远端仓库中包的信息，所以，只能下载之后再查看具体的信息了。

pip install --no-deps sqlalchemy==1.3.2
pip show -v sqlalchemy


貌似没有办法直接


pip show sqlalchemy
pip search sqlalchemy

https://pydist.com/blog/pip-install
https://www.python.org/dev/peps/pep-0440/
https://www.python.org/dev/peps/pep-0425/


到目前为止没有找到可以直接通过 PIP 查看远端仓库中包的信息，可以使用 yolk3k 这个三方包。

Yolk 是一个 Python 工具，可用于获取有关已安装或者 PyPI 的包信息，yolk3k 添加 Python 3 支持，同时也维护 Python 2 支持，当然，它还增加了附加功能。






yolk -l -f License,Author sqlalchemy

==1.0

https://www.jianshu.com/p/bae942d89b37




在开发一个 Python 开源项目，或者需要在多台服务器上部署，那么最好的方式是发布到 PyPI 上，这是一个 Python 的包管理平台，然后就可以通过 `pip install PKG` 的方式进行安装。

https://packaging.python.org/
https://www.jianshu.com/p/f3a5392b8e25
-->

## 参考

* [pip download](https://pip.pypa.io/en/stable/cli/pip_download/) 关于 PIP 下载命令的详细介绍，包括了一些常见的使用方式。
