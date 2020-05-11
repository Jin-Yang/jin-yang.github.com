---
title: CentOS 安装与配置
layout: post
comments: true
language: chinese
category: [linux]
keywords: linux,centos,安装,配置
description: CentOS (Community Enterprise Operating System) 也即社区版的企业操作系统，是 Linux 的发行版本之一，来自于 Red Hat Enterprise Linux 所开放的源码编译而成。因此，对稳定性较高的服务器通常以 CentOS 替代商业版的 Red Hat 。两者的不同在于 CentOS 并不包含封闭源代码软件。下面会介绍桌面版的 CentOS 在安装时需要作的一些常用配置。
---

CentOS (Community Enterprise Operating System) 也即社区版的企业操作系统，是 Linux 的发行版本之一，来自于 Red Hat Enterprise Linux 所开放的源码编译而成。因此，对稳定性较高的服务器通常以 CentOS 替代商业版的 Red Hat 。

两者的不同在于 CentOS 并不包含封闭源代码软件。下面会介绍桌面版的 CentOS 在安装时需要作的一些常用配置。

<!-- more -->

![centos logo]({{ site.url }}/images/linux/centos-logo.png "centos logo"){: .pull-center }

## 安装 CentOS

对于 CentOS 桌面版的安装也比较简单，完全是基于图形界面的，安装时可以从 [www.centos.org](https://www.centos.org/download/) 上下载相应版本，一般用 LiveGNOME 或者 LiveKDE 。

当然，也可以从国内的一些镜像网站上下载，例如 [阿里云镜像](http://mirrors.aliyun.com/)、[搜狐镜像](http://mirrors.sohu.com/)、[网易镜像](http://mirrors.163.com/) 上下载。

下载了 ISO 镜像之后可以通过如下方式安装到 U 盘中，如果有 Linux 发行版，安装过程将很简单；如下是通过 dd 命令直接复制即可。

{% highlight text %}
----- 查看U盘的设备号
# fdisk -l
... ...
Disk /dev/sdc: 4002 MB, 4002910208 bytes, 7818184 sectors
... ...

----- 将ISO镜像原样复制到U盘中
# dd if=xxx.iso of=/dev/sdb

----- 从另外一个终端执行，查看复制到U盘的进度
# while killall -USR1 dd; do sleep 5; done
{% endhighlight %}

在重启之后进入到 BIOS 启动界面，然后选择从 USB 启动进入安装界面。

接下来就看看如何配置一些常用的环境。

## 系统配置

一些常见的系统配置。

### 安装编译环境

如下是安装 C/C++ 编译工具。

{% highlight text %}
# yum install gcc gcc-c++ make cmake
{% endhighlight %}

### 中文输入

在安装时可能误选英文环境，或者在使用英文环境的同时，需要支持中文输入法。

可以安装 `ibus-libpinyin` 包，然后在 `Applications -> System Tools -> Setting -> Regin & Language` 中进行设置，一般是 `Chinese(Intelligent Pinyin)` 。

注意，正常来说，安装完包之后不需要重启，但是没有找到恢复的手段。

### 支持 NTFS 文件系统

可以在安装完第三方软件源 EPEL 之后通过如下命令安装。

{% highlight text %}
# wget -O /etc/yum.repos.d/epel.repo http://mirrors.aliyun.com/repo/epel-7.repo
# yum --enablerepo=epel install ntfs-3g
{% endhighlight %}

但是如果是最新版本的 CentOS，可能在三方库中没有相应的安装包，那么此时就需要从源码编译安装。直接从 [www.tuxera.com](http://www.tuxera.com/community/ntfs-3g-download/) 下载源码，然后通过如下方式编译。

{% highlight text %}
$ tar -zxvf ntfs-3g_ntfsprogs-2014.2.15.tgz
$ cd ntfs-3g_ntfsprogs-2014.2.15
$ ./configure
$ make
# make install                # 完成安装，需要root权限
{% endhighlight %}

接下来配置为自动挂载，假设对应的磁盘为 /dev/sda5 。

{% highlight text %}
# blkid /dev/sda5             # 查看UUID
# vi /etc/fstab               # 添加如下的内容
UUID=xxxxxxxxxxx /media/disk  ntfs-3g defaults 0 0
{% endhighlight %}

其中的 UUID 可以通过 `blkid` 查看。

### Gnome-Terminal 配置

一些常见的配置选项。

##### 取消声音

有声音实在是太烦了，可以通过如下方式配置。Edit->Preferences->Profiles->选择对应配置文件[Edit]->General->取消Terminal bell。

##### 设置启动快捷键

在 CentOS 的系统菜单中选择 Applications -> System Tools -> Keyboard -> Shortcuts -> Custom Shortcuts 设置命令为 `gnome-terminal --hide-menubar --maximize`，详细参数可以参考 `gnome-terminal --help-window-options` 。

##### 颜色设置

个人比较喜欢的颜色配置，文本颜色 `#dbfef8` 背景颜色 `#2f4f4f` 。

##### 设置为半透明

首先尝试在 Edit->Preferences 菜单中设置，如果不生效，则在 `~/.bashrc` 文件中添加如下内容，其中 80 对应不同的透明度。

{% highlight bash %}
if [ -n "$WINDOWID" ]; then
    TRANSPARENCY_HEX=$(printf 0x%x $((0xffffffff * 80 / 100)))
    xprop -id "$WINDOWID" -f _NET_WM_WINDOW_OPACITY 32c -set _NET_WM_WINDOW_OPACITY "$TRANSPARENCY_HEX"
fi
{% endhighlight %}

另外，可以将 bash 替换为 zsh 。

{% highlight text %}
# chsh -s /bin/zsh     # 修改默认shell
{% endhighlight %}

还有些弹出式终端工具，例如 guake ，很酷，不过感觉不太实用。

## 常用软件配置

{% highlight text %}
MPlayer VLC   视频
Deluge        BT客户端
Thunderbird   邮件客户端
Pidgin        即时消息
ClamAV        病毒扫描
Audacity      音频编辑
{% endhighlight %}


### 安装 Flash 插件

直接从如下网站 [www.adobe.com](http://get.adobe.com/cn/flashplayer/) 获得，主要有两种方法。

A) YUM 安装。下载时选择 "YUM，适用于Linux(YUM)"，实际会下载一个 RPM 安装包，用来安装 adobe 的 yum 源配置文件。

{% highlight text %}
# rpm -ivh adobe-release-i386-1.0-1.noarch.rpm        安装Adobe的源
# yum install flash-plugin                            安装falsh插件
{% endhighlight %}

B）RPM 安装。在下载页面选择 ".rpm，适用于其它Linux"，此时将会直接下载 RPM 安装包，可以直接通过 RPM 进行安装。

{% highlight text %}
# rpm -ivh flash-plugin-11.2.202.297-release.i386.rpm
{% endhighlight %}

### 音频/视频软件

在 CentOS 中，默认是 `Rythmbox/Totem`，不过使用有点麻烦，还是用 `Mplayer/Audacious` 比较方便，不过需要依赖 nux-dextop 源，当然也可以从 [pkgs.org](http://pkgs.org/search/) 上下载相关的二进制文件。

{% highlight text %}
# yum --enablerepo=nux-dextop install mplayer audacious plugins-freeworld-mp3
{% endhighlight %}

对于 Mplayer，如果使用时无法缩放，可以在 `~/.mplayer/config` 中添加 `zoom=yes` 配置项。

其中 `plugins-freeworld-mp3` 是 Audacious 中的 MP3 解码器。不过默认的外观不太好看，不过还好支持其它主题，可以从 [gnome-look.org](http://gnome-look.org) 中的 XMMS Themes 中选择主题，保存在 `/usr/share/audacious/Skins` 目录下，然后可以从 Audacious 的 Settings 窗口中看到。

对于中文，在主窗口中右击，选择 `Settings->Playlist->Compalibility[Fallback...]`，设置为 cp936 (比其它的要更通用)，重新加载播放列表即可。

另外，除上述的 GUI 播放器之外，还有些终端播放器，如 [Console Music](https://github.com/cmus/cmus)、[Music On Console](https://moc.daper.net/) ([Github](https://github.com/sagitter/moc)) 。

### 绘图软件

inkscape 用于绘制矢量图，另一个比较简单的是 [xfig](http://www.xfig.org/userman/) ，一款 old style 的画图工具。

{% highlight text %}
# yum install inkscape
{% endhighlight %}

另外一个就是 GIMP，同样可以通过 YUM 安装，方式同上。


### 虚拟机

常用的是 VirtualBox，可以直接从 [www.virtualbox.org](https://www.virtualbox.org/) 上下载相应的安装包，也就是 CentOS 的版本，然后通过如下方式安装。

{% highlight text %}
# yum install kernel-devel                      # 编译内核模块时需要该包
# rpm -ivh VirtualBox-x.x.x.rpm                 # 安装
# /sbin/vboxconfig                              # 重新编译内核模块
{% endhighlight %}

安装时可以直接参考网上的文章。


### 浏览器设置

主要包括了比较常用的 FireFox 以及 Chrome，其中前者是默认安装的。

#### Firefox

可以安装常用的插件，如 Regular Expressions Tester (一个正则表达式的测试工具)、Vimperator (将对火狐的部分操作改为VIM模式)、Mozilla Archive Format (用来保存查看mhtml格式的文件)。

##### 安装最新版本

有部分的开发工具需要最新版本的 firefox ，可以从 [Firefox Download](https://www.mozilla.org/en-US/firefox/new/) 下载最新版本，然后通过如下步骤安装。

{% highlight text %}
----- 1. 删除老版本
# yum remove firefox
# unlink /usr/bin/firefox

----- 2. 下载并将压缩包解压到/usr/local目录下
# ln -s /usr/local/firefox/firefox /usr/bin/firefox
{% endhighlight %}

#### Chrome

也可以从 [www.google.cn](https://www.google.cn/chrome/) 上下载 RPM 安装即可。

### CHM 阅读器

Linux 中常见的 chm 阅读器有 xchm、kchmiewer 等，在 CentOS 可以直接安装 xchm 。

{% highlight text %}
# yum --enablerepo=nux-dextop,epel install xchm
{% endhighlight %}


### 笔记类

比较悲剧，Linux 下面没有发现很好用的笔记软件，其中 WizNote 算是比较好用的，不过还是有 BUGs 。

#### tagspace

直接从官网 [www.tagspaces.org](http://www.tagspaces.org/) 下载，然后解压直接运行即可。

#### 为知笔记

一个跨平台的笔记 [www.wiz.cn](http://www.wiz.cn/)，安装方法可以查看 [GitHub](https://github.com/wizteam/wizqtclient)，可以查看相关的开源依赖 [为知笔记中使用的开源组件和协议](http://blog.wiz.cn/wiznote-opensource.html) 。

#### MarkDown

一些网页的工具， [StackEdit](https://stackedit.io/editor)、[Markable](https://markable.in/)、[Cmd Markdown](https://www.zybuluo.com/mdeditor)、[MaHua](http://mahua.jser.me/)、[马克飞象](https://maxiang.io/) 。

#### Haroopad

一个 MarkDown 软件，使用 Chromium 作为 UI，可以参考 [官方文档](http://pad.haroopress.com/) 。

<!--
$ tar -zxvf haroopad-v0.12.2_amd64.tar.gz
$ tar -zxvf data.tar.gz
# cp -r --link usr /usr                         # 创建硬链接
$ tar -zxvf control.tar.gz
# ./postinst
# vi /usr/share/applications/Haroopad.desktop   # 修改ICON
... ...
Icon=/usr/share/icons/hicolor/128x128/apps/haroopad.png
... ...
-->

#### 其它

另外，两个在 Mac 上很经典的软件 Mou 以及 [MacDown](http://macdown.uranusjr.com/features/)，对于 MacDown 源码可以参考 [Github](https://github.com/MacDownApp/macdown) 。


## 参考

官方的镜像列表，可以参考 [List of CentOS Mirrors](https://www.centos.org/download/mirrors/) 。

<!--
gpg签名
/etc/pki/rpm-gpg/RPM*
rpm 安装时可能会报 NOKEY 的错误信息 --nogpgcheck nosignature



安装 MPlayer 时，没有 `enca` 包，暂时安装 CentOS-7 版本的包，在 EPEL 仓库中。
-->

{% highlight text %}
{% endhighlight %}
