---
title: Yum 配置以及常用 RPM 命令
layout: post
comments: true
language: chinese
tag: [Linux, DevOps, SoftWare]
keywords: yum,rpm
description: RPM 作为一个通用的软件包管理工具，可以用来安装、删除、升级、检查、查询等功能，而 YUM 在此基础上提供了依赖的管理。这里介绍常用的管理命令，基本的 RPM 规范，常见的错误处理，例如镜像地址配置、本地源设置、强制升级包等等。
---

在 CentOS 中，会通过 YUM 或者 RPM 进行软件包的管理，不过 RPM 不能很好的解决软件之间的依赖关系，所以目前使用较多的是 YUM 源。

这里介绍常用的管理命令，基本的 RPM 规范，常见的错误处理，例如镜像地址配置、本地源设置、强制安装包等等。

<!-- more -->

![centos power logo](/{{ site.imgdir }}/linux/centos-power-logo.jpg "centos power logo")

## 简介

其中 Redhat Package Manager, RPM 是由 RedHat 公司提供的软件包管理器，可以实现软件包的安装、查询、卸载、升级以及校验等，相关的数据会保存在 `/var/lib/rpm` 目录下，但 rpm 不能很好的解决软件之间的依赖关系。

<!--
There are five basic modes for RPM command
    Install : It is used to install any RPM package.
    Remove : It is used to erase, remove or un-install any RPM package.
    Upgrade : It is used to update the existing RPM package.
    Verify : It is used to query about different RPM packages.
    Query : It is used for the verification of any RPM package.-->

在安装时，通常系统文件存放在 `/bin`、`/sbin`、`/lib` 目录下，而第三方软件会存放在 `/usr/local/bin`、`/usr/local/sbin`、 `/usr/local/lib`、`/usr/share/man` 和 `/usr/local/share/man` (后两者为帮助文件) 。

RPM 包通常有一个通用的命名方式：`name-version-release.arch.rpm` 。

* name: 表示包的名称，包括主包名和分包名；
* version: 表示包的版本信息；
* release: 用于标识 rpm 包本身的发行号，可还包含适应的操作系统；
* arch: 表示主机平台，noarch 表示此包能安装到所有平台上面。

如 `gd-devel-2.0.35-11.el6.x86_64.rpm` ，`gd` 是这个包的主包名；`devel` 是这个包的分包名；`2.0.35` 是表示版本信息，`2` 为主版本号，`0` 表示次版本号，`35` 为源码包的发行号也叫修订号； `11.el6` 中的 `11` 是 rpm 的发行号，`el6` 表示 RHEL6；`x86_64` 是表示适合的平台。

为了解决各个包之间的依赖关系，可以采用 Yellowdog Updater Modified, YUM 进行管理，该软件是由 Seth Vidal 开发，用于管理 RPM 包。

### 常用命令

如下列举一些常用的命令。

{% highlight text %}
----- 查看仓库源列表信息，可以先将/var/cache/yum/中的Cache清理
# yum repolist                                            // 可以是all disabled enabled
# yum repoinfo                                            // 以及源的信息

----- 查看软件信息
# rpm -q kernel                                           // 查看安装包的全名
# rpm -ql kernel                                          // 查看已安装软件包含有的文件
# rpm -qi kernel                                          // 查看已安装软件包的摘要信息
# rpm -qa | grep software                                 // 查看是否安装了software
# rpm -qf /usr/sbin/ifcfg                                 // 查看某文件属于那个包
# rpm -qc iproute                                         // 查看生成了那些配置文件
# rpm -qd iproute                                         // 查看生成了那些帮助文件
# rpm -qpi iproute.rpm                                    // 查看rpm包的信息
# rpm -qpl iproute.rpm                                    // 查看rpm包中含有的软件列表
# yum info mysql                                          // 查看包信息

----- 搜索软件
# yum search package                                      // 搜索package

----- 安装软件
# rpm --checksig foobar.rpm                               // 校验PGP Signature，查看完整性和来源
# rpm -qpR foobar.rpm                                     // 查看依赖
# rpm -ivh --nodeps --force foobar.rpm                    // 强制安装，如果其它软件包未安装，则不能工作
# yum -y install foobar                                   // 默认为 yes ，通常用于脚本文件，不需要交互

----- 只下载软件，如下命令包含在yum-utils包中
# yumdownloader PACK                                      // 使用yum自带软件
# yum -y install yum-downloadonly                         // 通过yum-downloadonly插件
# yum -y install --downloadonly --downloaddir=/tmp/ PACK  // 直接使用

----- 安装软件包群，很多被打包成Group的软件，可以通过如下命令查看、安装
# yum grouplist
# yum groupinstall 'MySQL Database'
# yum groupupdate 'MySQL Database'
# yum groupremove 'MySQL Database'

----- 卸载软件
# rpm -evv --nodeps foobar                                // 不需要指定到foobar.rpm
# yum -y remove foobar

----- 升级软件包
# rpm -Uvh foobar.rpm                                     // 如果新版本不工作，仍可以使用之前的版本
# yum check-update                                        // 查看可升级的软件包
# yum update foobar                                       // 升级特定的软件
# yum update                                              // 升级所有软件

----- 清除/更新Cache
# yum clean all                                           // 默认会保存在/var/cache/yum
# yum makecache                                           // 通常在修改/etc/yum/repos.d目录下配置文件之后

----- 获得源码，需要安装yum-utils工具，提供了yumdownloader
# vi /etc/yum.repos.d/CentOS-Source.repo                  // 将enabled设置为1
# yum clean all                                           // 清空缓存
# yum makecache                                           // 使修改生效
# rpm -qf `which mysql`                                   // 查看对应安装包
# yumdownloader --source mariadb                          // 下载源码包，不加--source则只下载
# rpm2cpio coreutils-8.4-19.el6_4.2.src.rpm | cpio -ivd   // 解压rpm包

----- 列出软件
# yum list | less                                         // 列出所有可安装的软件包
# yum list updates                                        // 列出所有可更新的软件包
# yum list installed                                      // 列出所有已安装的软件包
# yum list extras                                         // 列出所有已安装但不在 Yum Repository 內的软件包
# yum list [package]                                      // 列出所指定的软件包

----- 其它
# yum shell                                               // 交互环境，可以执行多条命令
# yum history                                             // 查看历史
# rpm -Vp xxx.rpm                                         // 与数据库中的版本校验 (Verify)
# rpm -Va                                                 // 校验所有的
# rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-6      // 导入 GPG key
----- 查询未安装软件包的依赖关系
$ rpm -qRp vim-common-6.3.046-2.el4.1.x86_64.rpm
----- 查询已安装软件包的依赖关系
$ rpm -qR vim-common-6.3.046-2.el4.1

----- 数据库出错，通过下述命令重建
# rm /var/lib/rpm/__db*
# rpm --rebuilddb
# rpmdb_verify Packages
{% endhighlight %}

`yum makecache` 会把服务器的包信息下载到本地电脑缓存起来，配合 `yum -C search xxx` 使用，不用上网检索就能查找软件的相关信息。

### 问题排查

#### Yum Lock

如果在通过 yum 安装软件时出现，`Another app is currently holding the yum lock` 错误，主要是为了防止 RPM 许同时运行造成冲突。

如果确认没有 yum 同时运行，那么可以通过 `cat /var/run/yum.pid` 查看占用的进程的 PID ，通过 `rm -f /var/run/yum.pid` 删除，重新安装。

{% include ads_content01.html %}

## 源配置

CentOS 中官方的源只包含了有限的安装包，为此需要安装一些常用的三方源。当然，也可以自己手动创建 Yum 仓库，主要采用了 createrepo，用于生成 repodata 软件信息仓库。

可以通常如下命令查看源列表的配置。

{% highlight text %}
# yum repolist                                            // 查看现在可用的源列表
# yum repolist all                                        // 查看所有的源列表
# yum --enablerepo=epel install mysql                     // 从指定源列表下载
{% endhighlight %}

接下来看看如何配置第三方的源。

### 使用三方源

配置完成后可以通过 `yum repolist all` 命令查看三方源是否生效，通过 `yum list software` 查看相应的软件。对于一些 RPM 包，也可以从 [pkgs.org](http://pkgs.org/search/) 或者 [www.rpmfind.net](http://www.rpmfind.net) 查找相应的软件包，以及与该软件包相关的依赖。

注意：如果在安装过程中出现 `error: Failed dependencies` 可以使用 `--nodeps` `--force` 选项强制安装，不过此时有可能导致部分功能失效。

另外，关于 CentOS 的第三方源，可以参考 [CentOS Wiki](http://wiki.centos.org/zh/AdditionalResources/Repositories) 中给出的参考意见。

#### 国内官方源

也就是一些 CentOS 的镜像，常见的有 [mirrors.aliyun.com](https://mirrors.aliyun.com/)、[centos.ustc.edu.cn](http://centos.ustc.edu.cn/)、[mirrors.163.com](http://mirrors.163.com/centos/)、[mirrors.sohu.com](http://mirrors.sohu.com/centos/)，只需要修改基本数据源中的 URL 配置选项。

阿里云提供了很多的三方仓库，例如 EPEL RPMFusion 等。

#### EPEL, Extra Packages for Enterprise Linux

EPEL是由 Fedora 社区打造，为 RHEL 及衍生发行版如 CentOS、Scientific Linux 等提供高质量软件包的项目，详细内容可以参考 [EPEL-Wiki](https://fedoraproject.org/wiki/EPEL/zh-cn) 。

安装源，其中对应的版本需要根据当前的版本自行选择。

{% highlight text %}
# rpm -Uvh http://mirrors.ustc.edu.cn/epel/beta/7/x86_64/epel-release-7-0.2.noarch.rpm
# rpm -Uvh http://dl.fedoraproject.org/pub/epel/beta/7/x86_64/epel-release-7-0.2.noarch.rpm
# rpm -Uvh https://mirrors.aliyun.com/epel/epel-release-latest-8.noarch.rpm
{% endhighlight %}

接下来时导入证书，当然这步也可以在通过 yum 安装时根据提示自动导入。

{% highlight text %}
# rpm -import /etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-7
{% endhighlight %}


#### rpmforge

可以从 [pkgs.repoforge.org](http://pkgs.repoforge.org/rpmforge-release/) 或者 [apt.sw.be](http://apt.sw.be/redhat/el7/en/x86_64/rpmforge/RPMS/) 下载。

{% highlight text %}
# rpm -Uvh http://apt.sw.be/redhat/el7/en/x86_64/rpmforge/RPMS/rpmforge-release-xxx.rpm
{% endhighlight %}

<!-- https://mirrors.aliyun.com/rpmfusion/free/el/rpmfusion-free-release-8.noarch.rpm -->

#### nux-dextop

直接从 [nux-dextop-release*rpm](http://li.nux.ro/download/nux/dextop/el7/x86_64/) 上查找安装最新的配置，通常 mplayer 会包含在该三方源中。


### 使用本地源

可以通过如下方法使用本地源，也就是下载的包含安装包的 ISO 镜像。

对于 VMware 需要通过如下方法挂载，`[Setting] -> [Hardware] -> [CD/DVD] -> 右边 Device 里勾选 Connected`，在 Use ISO image file 里选择 ISO 文件后确定即可。

CentOS 中使用的 yum 源配置文件保存在 `/etc/yum.repos.d` 目录下，主要包括了两个配置文件 `CentOS-Base.repo` 和 `CentOS-Media.repo`；其中，前者是配置网络 yum 源的，而后者是用来配置本地 yum 源。

{% highlight text %}
----- 对于VM来说，也可以使用Share Folder
# mount /dev/cdrom /media/cdrom
# mount -o loop /mnt/hgfs/Share/CentOS-xxx-xxx-bin-DVD.iso /media/cdrom/
# vi /etc/yum.repos.d/iso.repo
[c6-media]
name=CentOS-$releasever - Media      # 自定义名称
baseurl=file:///media/cdrom/         # 可以指定多个路径
        file:///media/cdrom2/
gpgcheck=1
enabled=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-6

----- 清除缓存
# yum clean all

----- 证书也可以使用如下方式安装
# rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-5
# yum --enablerepo=c6-media install mysql-server                   # 本地和网络
# yum --disablerepo=\* --enablerepo=c6-media install mysql-server  # 只使用本地光盘
{% endhighlight %}

#### 原生本地源

如上的 ISO 镜像文件中，已经包含了索引文件 (位于 `repodata` 文件夹)，如果自己创建本地镜像，例如平时收藏的 RPM 软件包或者集成测试等，此时需要通过 `createrepo` 创建索引文件。

当然，如过没有 `createrepo` 命令，则需要安装 `createrepo.xxx.rpm` 包。

{% highlight text %}
----- 1. 创建本地yum仓库目录
$ mkdir -p /share/CentOS/7/local/x86_64/RPMS

----- 2. 创建索引&更新缓存
$ createrepo /share/CentOS/7/local/x86_64
$ yum makecache

----- 3. 创建本地repo文件
$ cat<<-"EOF">/etc/yum.repos.d/CentOS-Local.repo
[local]
name=CentOS-$releasever - local packages for $basearch
baseurl=file:///share/CentOS/$releasever/local/$basearch
enabled=1
gpgcheck=0
protect=1
EOF
{% endhighlight %}

另外，可以参考 [How to create public mirrors for CentOS](https://wiki.centos.org/HowTos/CreatePublicMirrors)、[Create Local Repos](https://wiki.centos.org/HowTos/CreateLocalRepos) 。

{% include ads_content02.html %}

## 其它

### 优先级

在使用时，最好先安装 `yum-priorities` 插件，该插件用来设置 yum 在调用软件源时的顺序，因为官方提供的软件源，都是比较稳定和被推荐使用的，因此，官方源的顺序要高于第三方源的顺序。

{% highlight text %}
# yum install yum-priorities
{% endhighlight %}

安装完后需要设置 `/etc/yum.repos.d/` 目录下的 `*.repo` 相关文件，例如 `CentOS-Base.repo`、`epel.repo`、`nux-dextop.repo` 等，在这些文件中插入顺序指令 `priority=N` (N为1到99的正整数，数值越小优先级越高)，一般第三的软件源设置的优先级大于 10 。

### Baseurl VS. MirrorList

在 `/etc/yum.repo.d` 目录下，基本上每个仓库的配置文件都包含了这两个配置项，例如：

{% highlight text %}
mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=BaseOS&infra=$infra
baseurl=http://mirror.centos.org/$contentdir/$releasever/BaseOS/$basearch/os/
{% endhighlight %}

另外，在 `/etc/yum.conf` 文件中看到如下配置项，实际上就是缓存。

{% highlight text %}
cachedir=/var/cache/yum/$basearch/$releasever
{% endhighlight %}

如果打开 `mirrorlist` 的配置项，实际上是一堆的 `baseurl`；而 `baseurl` 必须要指向 YUM 仓库上的 `repodata` 目录，该目录保存了 RPM 安装时的依赖信息。

当需要配置三方的仓库时，就需要修改 `baseurl` 的值。

### 强制升级包

当一个软件打上 Patch 并制作 RPM 包之后，如果希望本地安装、测试，那么就可能会遇到如下的一些问题。

#### 同名同版本

已经安装好的包，需要重新安装同名而且同版本的包，那么可以使用下面的命令强制安装。

```
rpm -ivh --replacepkgs package-1.2.3-34.el7.x86_64.rpm
```

#### 回退到老包

也就是需要安装一个版本号较老的包。

```
rpm -Uvh --oldpackage package-1.2.1-34.el7.x86_64.rpm
```

当然，其实也可以使用 YUM 安装，这样就不需要再记住上面的包，也就是直接使用 `yum localinstall package-1.2.3-34.el7.x86_64.rpm` 命令即可。

{% highlight text %}
{% endhighlight %}
