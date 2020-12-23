---
title: Linux 用户管理的最佳实践
layout: post
comments: true
language: chinese
tag: [Linux,DevOps]
keywords: linux,管理,用户,用户组,系统用户
description: 在 Linux 中，权限管理包括了用户、主用户组、从用户组，通过这三者的组织可以方便对权限的管理，尤其是对于文件的权限管理。这里，通过一个示例，详细介绍常见的场景，以及如何使用，以保证灵活性。
---

在 Linux 中，权限管理包括了用户、主用户组、从用户组，通过这三者的组织可以方便对权限的管理，尤其是对于文件的权限管理。这里假设一个运维中的使用场景，通过创建一系列的用户以及用户组，然后查看其权限管理的方案。

<!-- more -->

## 简介

这是在运维时经常使用到的场景，在设计 DevOps 系统时，需要通过各种各样 Agent 采集数据、执行任务等，这些账号不需要登陆，只需要作权限控制即可，常见的如：

1. BasicAgent 作为命令通道，用来执行命令，修改配置等基础操作。
2. MonitorAgent 采集 OS、进程、业务、MySQL 等监控数据。
3. LogAgent 采集日志信息，也可以转换成监控指标。

其它的可能还有安全、文件传输等类型的 Agent，这里仅列举最常见的这三个，对应了 basic、monitor、log 三个用户，并通过 devops 用户组进行管理。

另外，为了方便对各个 Agent 同时可以控制自己的属组，会同时创建与用户名相同的用户组，而 devops 作为次要用户组。

注意，这里使用的是系统用户，也就意味着只能从 root 切换过去，而且，如果要使用 Shell 执行一些命令测试，可以在通过 `su` 命令切换时添加 `-s /bin/bash` 覆盖默认的 Shell 。

### 用户规划

如上所述，这里分别创建三个用户 basic、monitor、log 以及同名的主用户组，同时会创建一个 devops 次用户组，各个用户的 UID 以及 GID 对应如下。

{% highlight text %}
devops             GID:60
basic       UID:63 GID:63
monitor     UID:66 GID:66
log         UID:69 GID:69
{% endhighlight %}

实际上，完全可以将所有用户以 devops 作为主用户组，因为目前还没有想到具体那些场景需要通过独立的用户组，所以，这里其实还有就是为了测试。

{% include ads_content01.html %}

## 创建用户/用户组

这里以 montor 用户为例，其它用户的创建可以将相关参数替换即可，如下是一个简单的创建用户的脚本。

{% highlight bash %}
#! /bin/sh

# NOTE: Command 'id -g monitor' will failed on some platform.
if ! (grep -E '^monitor\>:' /etc/group >/dev/null 2>&1) ; then
	echo "Group 'monitor' does not exist, try to create one"
	/usr/sbin/groupadd -g 66 -o -r monitor >/dev/null 2>&1 || :
else
	echo "Group 'monitor' already exists"
	exit 1
#	gid=`id -g monitor`
#	if [ "x${gid}" != "x66" ]; then
#		echo "Change group id(${gid}) to 66"
#		/usr/sbin/groupmod -g 66 monitor
#	fi
fi

if ! (grep -E '^monitor\>:' /etc/passwd >/dev/null 2>&1) ; then
	echo "User 'monitor' does not exist, try to create one"
	/usr/sbin/useradd -M -g monitor -o -r -d /your/home/path -s /sbin/nologin \
		-c "Monitor Agent" -u 66 monitor >/dev/null 2>&1 || :
else
	echo "User 'monitor' already exists"
	exit 2
#	uid=`id -u monitor`
#	if [ "x${uid}" != "x66" ]; then
#		echo "Change user id(${uid}) to 66"
#		/usr/sbin/usermod -u 66 monitor
#	fi
fi
{% endhighlight %}

其中的注释为，当 UID 不满足时，直接通过命令进行修改，强制修改可能会导致一些已经创建的文件异常，其对应的属主因为查询不到用户，只会显示源 UID ，所以不建议修改，当然，可以根据自己的需求决定是否进行修改。

另外，注意使用的两个参数：A) `-o` 不再限制 UID/GID 唯一，可以出现用户名/组名不同，但是对应 UID/GID 相同；B) `-r` 创建系统用户，不允许登陆。

### 常用命令

如上所述，因为添加的是系统用户，而且没有设置有效的 Shell ，如果要执行一些常见的命令，可以使用类似如下的方式。

{% highlight text %}
# su - monitor -s /bin/bash -c "ls -alh /"
{% endhighlight %}

注意，执行时使用的是系统用户。

如果想登陆 Shell 执行交互命令，可以使用如下命令。

{% highlight text %}
# su - monitor -s /bin/bash
$ id
uid=66(monitor) gid=66(monitor) groups=66(monitor)
{% endhighlight %}

如上的命令只是创建了 monitor 用户，创建 devops 用户组的方式类似，创建完之后可以通过如下命令添加附加组。

{% highlight text %}
# usermod -G devops monitor
# grep -re devops /etc/group
devops:x:60:monitor
# su - monitor -s /bin/bash
$ id
uid=66(monitor) gid=66(monitor) groups=66(monitor),60(devops)
{% endhighlight %}

可以看到，切换之后具有了两个用户组，包括了主用户组 monitor 以及辅助用户组 devops 。

### 测试命令整理

如下将创建上述用户以及用户组的命令简单整理，可以直接复制执行。

{% highlight text %}
----- 创建用户
groupadd -g 60 -o -r devops
useradd -M -g devops -o -r -d /tmp -s /sbin/nologin -c "DevOps Group" -u 60 devops
groupadd -g 63 -o -r basic
useradd -M -g basic -o -r -d /tmp -s /sbin/nologin -c "Basic Agent" -u 63 basic
groupadd -g 66 -o -r monitor
useradd -M -g monitor -o -r -d /tmp -s /sbin/nologin -c "Monitor Agent" -u 66 monitor
groupadd -g 69 -o -r log
useradd -M -g log -o -r -d /tmp -s /sbin/nologin -c "Log Agent" -u 69 log

----- 删除用户
userdel devops
groupdel devops
userdel basic
groupdel basic
userdel monitor
groupdel monitor
userdel log
groupdel log
{% endhighlight %}

{% include ads_content02.html %}

## 常见场景

虽然上面创建了三个用户，实际测试的时候只需要 monitor 和 devops 即可。

### 统一设置 devops 组权限

对应的场景为，为了方便管理，所有 Agent 相关的配置文件保存在固定的目录下，而且所有 Agent 都可以读取固定配置文件。

{% highlight text %}
# su -s /bin/bash devops -c "echo 'config' > /tmp/host.cnf"
# ls -l /tmp/host.cnf
-rw------- 1 devops devops 7 Oct 18 16:43 /tmp/host.cnf
# chmod 664 /tmp/host.cnf -l
# ls -l /tmp/host.cnf
-rw-rw-r-- 1 devops devops 7 Oct 18 16:43 /tmp/host.cnf
{% endhighlight %}

通过如上命令创建了一个 `/tmp/host.cnf` 配置文件，然后可以通过如下命令进行测试。

{% highlight text %}
# su - -s /bin/bash monitor
$ id                                  当前没有添加到devops辅助组
uid=66(monitor) gid=66(monitor) groups=66(monitor)
$ cat /etc/host.cnf                   文件允许其它用户读取
config
$ echo "new config" > /tmp/host.cnf   但是不允许写入
-bash: /tmp/host.cnf: Permission denied
{% endhighlight %}

接下来，通过 `usermod -G devops monitor` 命令将 monitor 用户添加到 devops 辅助组中。

{% highlight text %}
# su - -s /bin/bash monitor
$ id                                  确认已经添加到devops辅助组
uid=66(monitor) gid=66(monitor) groups=66(monitor),60(devops)
# ls -l /tmp/host.cnf                 确保辅助组有权限
-rw-rw-r-- 1 devops devops 7 Oct 18 16:43 /tmp/host.cnf
$ echo "new config" > /tmp/host.cnf   现在可以写入成功了
$ cat /etc/host.cnf
new config
{% endhighlight %}

也就是说，主用户组和辅助用户组的组权限是相同的，前提是文件允许组进行访问。

### 读取其它用户信息

对于监控来说，需要采集 MySQL 的监控指标，在本地就需要通过 Unix Domain Socket 进行连接，那么可以通过如下方式查看是否有权限。

{% highlight text %}
----- 查看是否有访问权限
# su - monitor -s /bin/bash -c "ls -alh /var/lib/mysql/mysql.sock"

----- 查看是否有写权限
# su - monitor -s /bin/bash -c "touch /var/lib/mysql/mysql.sock"
{% endhighlight %}

访问其它用户的文件时需要保证：A) 保证文件对其它用户是可读写的；B) 整个路径目录对其它用户是可执行的。

如果在通过上述的方式检查时发现没有权限，那么需要通过如下命令修改 monitor 用户的属组，同时要保证 相关的文件以及目录有该属组的访问权限 。

如下示例仍以 MySQL 用户为例。

{% highlight text %}
----- 设置monitor用户的辅助属组
# usermod -G mysql monitor

----- 检查是否修改成功，注意后面的属组
$ id monitor
uid=66(monitor) gid=66(monitor) groups=66(monitor),60(devops)
{% endhighlight %}

这样设置完之后，在切换用户的时候需要同时切换到主用户组和辅助用户组。

{% highlight text %}
{% endhighlight %}
