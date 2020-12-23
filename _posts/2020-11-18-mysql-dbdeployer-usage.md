---
title: 详细介绍 DBDeployer 部署工具使用
layout: post
comments: true
language: chinese
tag: [MySQL, Database]
keywords: MySQL, Percona, MariaDB, Sandbox, 测试, dbdeployer, 工具, 本地
description: 通过 DBDeployer 可以在本地快速搭建一个强大的测试环境，包括了不同版本、不同架构的数据库环境，例如 MySQL 主从复制、GTID 模式复制、MySQL 组复制等等。通常可以用作本地测试，非常方便使用。
---

通过 DBDeployer 可以在本地快速搭建一个强大的测试环境，包括了不同版本、不同架构的数据库环境，例如 MySQL 主从复制、GTID 模式复制、MySQL 组复制等等。

支持几乎当前所有版本的 MySQL 发行版本，除了官方的，还有 Percona MySQL、MariaDB、MySQL NDB Cluster、Percona XtraDB Cluster 等等，还有 MySQL-Shell 的安装，甚至还有 TiDB ，估计是 TiDB 团队贡献的代码吧。

这里会详细介绍其使用方法。

<!-- more -->

## 简介

在该工具之前，还存在一个 [MySQL SandBox](/post/mysql-sandbox.html) 的类似工具，好像是同一个作者，不过 MySQL-SandBox 是使用 Perl 语言编写的，而 DBDeployer 则是采用 GoLang 开发，如果是新手，建议还是使用 DBDeployer 这个工具了。

直接从安装部署一个简单的单实例开始。

### 安装

直接从 [GitHub Release](https://github.com/datacharmer/dbdeployer/releases) 上下载相应 OS 的版本，都是一些独立的二进制文件，所以，在多数 OS 中，直接添加到 `PATH` 环境变量所在路径即可，例如 `/usr/bin` 目录下。

然后，可以通过如下命令检查是否安装成功。

```
dbdeployer --version
```

另外，该工具还提供了命令行的自动补齐功能，在源码库中提供了 `docs/dbdeployer_completion.sh` 脚本，直接通过如下方式复制到对应目录即可。

```
cp dbdeployer_completion.sh /etc/bash_completion.d
source /etc/bash_completion.d/dbdeployer_completion.sh
```

在输入命令时可以尝试下 `Tab` 键的自动补齐功能。

### 配置环境

使用时，可以通过 `SANDBOX_BINARY` 和 `SANDBOX_HOME` 两个环境变量配置路径，前者用来安装不同的 MySQL 二进制版本，后者是 MySQL 运行实例的路径。

```
mkdir /opt/MySQL/{binary,home,package}
export SANDBOX_HOME=/opt/MySQL/home
export SANDBOX_BINARY=/opt/MySQL/binary
```

这里同时增加一个 `package` 目录，用来存放下载的 MySQL 压缩包。

```
----- 查看环境变量等信息，会显示初始化环境的步骤，但是没有执行
dbdeployer init --dry-run
```

所谓的初始化环境，其实就是创建所需目录，下载对应的 MySQL 版本，可以通过 `dbdeployer init -h` 查看具体步骤。

如果可以联网，那么通过 `dbdeployer init` 会自动完成初始化环境，而离线，无非就是创建所需目录，添加上述的补全文件，以及如下的准备二进制文件。

#### 下载二进制包

可以通过 `dbdeployer` 自动下载，也可以从 [downloads.mysql.com](https://downloads.mysql.com/archives/community/) 上获取所需的版本包，选择 `版本号` + `Linux - Generic` + `All` 即可。

```
----- 查看当前支持的列表
dbdeployer downloads list
----- 貌似是下载并解压了，也可以分成get和unpack两步
dbdeployer downloads get-unpack mysql-5.7.31-linux-glibc2.12-x86_64.tar.gz
```

假设手动下载了版本 `mysql-8.0.21-linux-glibc2.17-x86_64-minimal.tar.xz` 包，那么可以通过如下命令安装。

```
dbdeployer unpack mysql-8.0.21-linux-glibc2.17-x86_64-minimal.tar.xz
```

### 启动实例

接着看看如何启动单个实例。

```
----- 启动单个实例
dbdeployer deploy single 8.0.21
```

此时已经启动了 MySQL 服务进程，对应的包会安装在 `$SANDBOX_HOME/msb_8_0_21` 目录下，可以通过该目录下的 `use` 脚本启动。

```
$ cd /opt/MySQL/home/msb_8_0_21
$ ./use
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 12
Server version: 8.0.21 MySQL Community Server - GPL

Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql [localhost:8021] {msandbox} ((none)) >
```

除了 `use` 脚本之外，还包括了 `start` `status` `stop` `restart` 等常用的启停脚本，以及 `show_log` `show_binlog` 等查看日志的脚本。

### 常用命令

```
----- 当前版本所支持的数据库类型及其版本等信息
dbdeployer admin capabilities

----- 已经安装的组件版本
dbdeployer versions

----- 当前已经配置的沙箱
dbdeployer sandboxes --header

----- 删除某个沙箱
dbdeployer delete msb_8_0_21
```

<!--
 部署一个单节点的MySQL，开启GTID并指定字符集 ## GTID和字符等参数也可在部署完成后在MySQL配置文件中指定 ## 注意：部署的数据库默认自动运行，可以指定--skip-start参数只初始化但不启动 shell> dbdeployer deploy single 8.0.17 --gtid --my-cnf-options="character_set_server=utf8mb4" ## 部署一个主从复制MySQL(默认初始化3个节点，一主两从) shell> dbdeployer deploy replication 8.0.17 --repl-crash-safe --gtid --my-cnf-options="character_set_server=utf8mb4" ## 部署一个单主模式的MGR(默认初始化3个节点) shell> dbdeployer deploy --topology=group replication 8.0.17 --single-primary ## 部署一个多主模式的MGR(默认初始化3个节点) shell> dbdeployer deploy --topology=all-masters replication 8.0.17
dbdeployer deploy multiple 8.0.21

## 配置文件

默认会使用 `~/.dbdeployer/config.json` 配置，

multiple    创建多个独立的mysql
replication 创建复制环境的mysql
single      创建单节点的mysql
-->

## 参考

* [GitHub DBDeployer](https://github.com/datacharmer/dbdeployer) 官方的仓库地址，同时包含了手册文档。
* [downloads.mysql.com/archives](https://downloads.mysql.com/archives/community/) MySQL 官方历史版本的下载路径。
