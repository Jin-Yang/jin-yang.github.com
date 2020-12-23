---
title: Linux Alternatives 命令使用
layout: post
comments: true
language: chinese
tag: [Linux,DevOps]
keywords: linux,alternatives
description: 通过 Linux 提供的 Alternatives 可以很方便的管理多个软件版本，方便在多版本之间相互切换，例如 Python 可以是 2.X 和 3.X 等，这里详细介绍如何进行配置。
---

在 Linux 中，对于相同的功能可能会存在多种的实现，例如查看文件可以使用 head、tail、cat 等；而且，即使相同软件可能会有多个版本，例如 Python 有 2.X、3.X 版本，等等。

为了统一命令、方便管理，就可以使用 Linux 中的 alternatives 命令，这里简单介绍。

<!-- more -->

## 简介

在 `chkconfig` 包中，包含了一个 `update-alternatives` 命令 (与 `alternatives` 相同)，可以对某个工具的多个版本进行管理，可以很方便设置系统默认使用哪个版本。

以编辑器为例，在 Linux 上支持 nvi、vim、emacs、nano 等等，假设希望通过 `editor` 可以直接调用一个编辑器，而且无需关心是那个。

## 示例

通过如下方式创建一个 `editor` 的命令。

{% highlight text %}
alternatives --install <link> <name> <path> <priority>
  <link> 使用的绝对路径，例如/bin/editor
  <name> 在/etc/alternative中的名字，例如editor
  <path> 可选的程序所在路径，例如/bin/vim
  <priority> 优先级，越高越好

alternatives --install /bin/editor editor /bin/vim 10
alternatives --install /bin/editor editor /bin/cat 5
{% endhighlight %}

此时，会存在一个 `/bin/editor` 的符号链接，指向 `/etc/alternatives/editor` 符号链接，而该路径指向 `/bin/vim` 文件。

{% include ads_content01.html %}

## 常用命令

{% highlight text %}
----- 查看所有可以作为editor的命令
alternatives --display editor

----- 选择其中一个命令作为editor，此时会变成manual，包括了交互和直接设置
alternatives --config editor
alternatives --set editor /bin/vim

----- 同时再次修改为auto模式
alternatives --auto editor

----- 删除cat或者所有
alternatives --remove editor /bin/cat
alternatives --remove-all editor

----- 查看当前所有默认指向命令
alternatives --list
{% endhighlight %}

另外，有些软件包含有多个命令，那么就可以使用 `--slave` 参数，如下是一个 `java` 的示例。

{% highlight text %}
alternatives --install /usr/bin/java java /opt/jdk/bin/java 10 \
    --slave /usr/bin/jar     jar     /opt/jdk-10/bin/jar       \
    --slave /usr/bin/javac   javac   /opt/jdk-10/bin/javac     \
    --slave /usr/bin/javadoc javadoc /opt/jdk-10/bin/javadoc
{% endhighlight %}

也就是，可以同时配置多个。


{% highlight text %}
{% endhighlight %}
