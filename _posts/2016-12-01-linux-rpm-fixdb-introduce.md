---
title: Linux RPM DB 修复
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: linux,rpm,db,fix
description: 在一些 Linux 的发行版本中，例如 CentOS、SUSE 中，会采用 RPM 来管理软件包，不过在使用过程中，经常会出现由于各种各样的原因导致 RPM DB 被损坏。这里简单介绍一下其修复方法。
---

在一些 Linux 的发行版本中，例如 CentOS、SUSE 中，会采用 RPM 来管理软件包，不过在使用过程中，经常会出现由于各种各样的原因导致 RPM DB 被损坏。

这里简单介绍一下其修复方法。

<!-- more -->

## 简介

RPM 会使用自己的 DB 保存元数据，经常在宕机、`kill -9` 时会损坏 DB ，那么就可能会导致已经安装的包丢失、更新 RPM 被 hang 住、查询包信息失败，失败信息一般如下。

{% highlight text %}
rpmdb: /var/lib/rpm/Packages: unexpected file type or format
error: cannot open Packages index using db3 - Invalid argument (22)
{% endhighlight %}

{% highlight text %}
rpmdb: Thread/process 14357/139885775857408 failed: Thread died in Berkeley DB library
error: db3 error(-30974) from dbenv->failchk: DB_RUNRECOVERY: Fatal error, run database recovery
error: cannot open Packages index using db3 -  (-30974)
error: cannot open Packages database in /var/lib/rpm
rpmdb: Thread/process 14357/139885775857408 failed: Thread died in Berkeley DB library
error: db3 error(-30974) from dbenv->failchk: DB_RUNRECOVERY: Fatal error, run database recovery
error: cannot open Packages database in /var/lib/rpm
{% endhighlight %}

## 修复过程

接下来需要着手进行修复。

##### 1. 首先确保没有RPM进程

{% highlight text %}
# ps -ef | grep -i rpm
{% endhighlight %}

在修复时，如果存在其它进程同时在使用，那么可能会同样导致失败，如果有必要可以通过 `kill -9` 关闭进程。

##### 2. 删除锁文件

{% highlight text %}
# rm -rf /var/lib/rpm/__db*
# /usr/lib/rpm/rpmdb_verify Packages
{% endhighlight %}

删除存留的锁文件，并校验 DB 是否有损坏，此时就可以检查 RPM 相关的命令是否可以执行。

##### 3. 备份 RPM 数据库

{% highlight text %}
# cd /var/lib
# mkdir rpm-backup
# rsync -av ./rpm/. ./rpm-backup/.
{% endhighlight %}

##### 4. 重新构建 RPM 数据库

{% highlight text %}
# rpm -vv --rebuilddb > /tmp/rpmrebuilddb.log 2>&1
{% endhighlight %}

##### 5. 最后重新校验

{% highlight text %}
# /usr/lib/rpm/rpmdb_verify /var/lib/rpm/Packages
{% endhighlight %}


<!--
----- 清除并重新构建缓存
# yum clean all
# yum makecache


# mkdir /backups/
# tar -zcvf /backups/rpmdb-$(date +"%d%m%Y").tar.gz  /var/lib/rpm

在第 4 步重建过程中，也可以通过如下步骤重新生成相关的 DB 。

# cd /var/lib/rpm/
# mv Packages Packages.back
# /usr/lib/rpm/rpmdb_dump Packages.back | /usr/lib/rpm/rpmdb_load Packages
# /usr/lib/rpm/rpmdb_verify Packages

最后可以通过 `rpm -qa >/dev/null` 命令查看是否有问题。
-->


## 参考

[RPM Database Recovery](http://rpm.org/user_doc/db_recovery.html) 官方提供的修复方案。

<!--
RPM 一些相关的内容，包括了如何打包
https://segmentfault.com/a/1190000015337919

FaceBook的一个检查修复RPM的实现
https://github.com/facebookincubator/dcrpm
https://github.com/facebookincubator/oomd
https://www.cyberciti.biz/tips/rebuilding-corrupted-rpm-database.html

此时需要清理以下rpm的临时文件

# rm -rf /var/lib/rpm/__db.*
-->



{% highlight text %}
{% endhighlight %}
