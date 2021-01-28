---
title: DBDeployer 常用命令速查表
layout: post
comments: true
tag: [CheatSheet]
language: chinese
keywords: MySQL, DBDeployer, CheatSheet, 常用命令
description: DBDeployer 是一款十分强大的数据库测试环境部署工具，可实现一键部署不同架构、不同版本的数据库环境，这里介绍常用的一些命令。
---

DBDeployer 是一款十分强大的数据库测试环境部署工具，可实现一键部署不同架构、不同版本的数据库环境，之前已经详细介绍过其使用方法。

这里将一些常用的命令整理下，已方便查询。

<!-- more -->

## 环境准备

```
----- 设置实例和二进制包的安装路径，另外package保存离线包、backup保存备份
mkdir -p /opt/MySQL/{binary,home,package}
export SANDBOX_HOME=/opt/MySQL/home
export SANDBOX_BINARY=/opt/MySQL/binary
```

## 包管理

```
----- 支持下载的软件包列表，可以使用远程或者本地缓存
dbdeployer remote list
dbdeployer downloads list
----- 查看会下载的最新包信息，去掉--dry-run后会执行下载
dbdeployer downloads get-by-version 8.0 --newest --OS=linux --minimal --dry-run
----- 安装离线下载软件包
dbdeployer unpack mysql-8.0.21-rc-linux-glibc2.12-x86_64.tar.gz
```

## 创建

```
----- 这里仅列举常用参数，详细可通过如下命令查看
dbdeployer deploy -h
dbdeployer deploy single -h
dbdeployer deploy multiple -h
dbdeployer deploy replication -h
```

### 单实例

```
----- 部署单实例，默认名称为 msb_8_0_21 可通过 --sandbox-directory 修改
dbdeployer deploy single 8.0.21
dbdeployer deploy single 8.0.21 --sandbox-directory=YourSandBoxName

----- 手动启动，默认会自动启动，可通过 --skip-start 跳过启动实例过程
dbdeployer deploy single --skip-start 8.0.21
${SANDBOX_HOME}/msb_8_0_21/start
${SANDBOX_HOME}/msb_8_0_21/load_grants

----- 也可以部署多个独立的实例，默认三个节点，可通过 --nodes 参数指定数量
dbdeployer deploy multiple 8.0.21 --nodes=2
```

创建实例后，会在实例目录下包含一个 `my.sandbox.cnf` 配置文件，包含了所有的个性化设置。

### 主从模式

实际上都被称为 replication 模式，这里只是根据不同的场景进行了分类。

```
----- 默认是主从模式，一主两从，可通过 --topology 指定其它方式
dbdeployer deploy --topology=master-slave replication 8.0.21 --concurrent

----- 可以指定一主一备
dbdeployer deploy --topology=master-slave replication 8.0.21 --nodes 2 --concurrent

----- 使用GTID，而且端口从18600开始，也就是18601 18602 18603
dbdeployer deploy replication 8.0.21 --gtid --base-port=18600 --concurrent

----- 通过如下查看常用操作命令
dbdeployer usage multiple

----- 可以手动启动，注意，只有主库需要load_grants，备库会自动同步
dbdeployer deploy replication --topology=master-slave --skip-start 8.0.21 --concurrent
${SANDBOX_HOME}/rsandbox_8_0_21/start_all
${SANDBOX_HOME}/rsandbox_8_0_21/master/load_grants
${SANDBOX_HOME}/rsandbox_8_0_21/initialize_slaves
```

### 集群模式

```
----- 默认是多主模式，可通过 --single-primary 指定单主模式
dbdeployer deploy --topology=group replication 8.0.21 --concurrent
dbdeployer deploy --topology=group replication 8.0.21 --single-primary --concurrent

----- 同主备模式，也可以手动启动
dbdeployer deploy replication --topology=group --skip-start 5.7.21 --concurrent
${SANDBOX_HOME}/group_msb_8_0_21/start_all
${SANDBOX_HOME}/group_msb_8_0_21/node1/load_grants
${SANDBOX_HOME}/group_msb_8_0_21/node2/load_grants
${SANDBOX_HOME}/group_msb_8_0_21/node3/load_grants
${SANDBOX_HOME}/group_msb_8_0_21/initialize_nodes
```

### 其它模式

```
----- 使用多主模式
dbdeployer deploy replication --topology=fan-in --nodes=5 \
    --master-list="1,2 3" --slave-list="4,5" 8.0.21 --concurrent
```

### 其它命令

```
----- 清理数据并重新部署
${SANDBOX_HOME}/msb_8_0_21/wipe_and_restart
${SANDBOX_HOME}/msb_8_0_21/wipe_and_restart_all
```

## 执行命令

配置都保存在 `my.sandbox.cnf` 文件中，创建的用户信息可以查看 `grants.mysql`，密码默认都是 `msandbox` ，只有主备复制用的 `rsandbox` 密码是 `rsandbox`。

```
----- 需要在实例安装目录下执行，默认使用msandbox用户，可以通过-u指定
./use
./use -u root

----- 可以多个实例统一执行
dbdeployer global use "SELECT version()"
dbdeployer global status
dbdeployer global stop --version=5.7.27
dbdeployer global stop --short-version=8.0
dbdeployer global stop --short-version='!8.0' # or --short-version=no-8.0
dbdeployer global status --port-range=5000-8099
dbdeployer global start --flavor=percona
dbdeployer global start --flavor='!percona' --type=single
dbdeployer global metadata version --flavor='!percona' --type=single
```

也可以使用 MySQL 客户端支持的参数，例如 `-pYourPassword`，也可以使用常用的命令行。

```
----- 使用MySQL的二进制
${SANDBOX_BINARY}/8.0.21/bin/mysql -h 127.0.0.1 -P 19832 -u msandbox -pmsandbox -D test
${SANDBOX_BINARY}/8.0.21/bin/mysql --defaults-file=${SANDBOX_HOME}/msb_8_0_21/my.sandbox.cnf -u root
${SANDBOX_BINARY}/8.0.21/bin/mysqldump --defaults-file=${SANDBOX_HOME}/msb_8_0_21/my.sandbox.cnf -u root --databases employees --tables employees --where 'emp_no < 10010'

----- 从my.sandbox.cnf中读取配置，然后拼接参数
pt-show-grants -S /tmp/mysql_sandbox19832.sock -u root -pmsandbox
```

## 查看状态

```
----- 已经安装的二进制格式
dbdeployer versions
----- 当前已经创建的实例信息，建议使用第二个命令
dbdeployer sandboxes --header
dbdeployer sandboxes --full-info
----- 可以全局查看、停止、删除所有或者指定实例
dbdeployer global status
dbdeployer global stop msb_8_0_21
dbdeployer delete msb_8_0_21
```

## 锁定

```
----- 锁定后无法删除，即使是删除所有实例
dbdeployer admin lock SandboxName
dbdeployer admin unlock SandboxName
```

## 删除

```
----- 删除实例指定实例，指定ALL删除所有
dbdeployer delete SandboxName

----- 清除数据并重新启动
${SANDBOX_HOME}/msb_8_0_21/wipe_and_restart
${SANDBOX_HOME}/rsandbox_8_0_21/wipe_and_restart_all
```

## 配置

```
----- 将默认配置导出
dbdeployer defaults export ~/.dbdeployer/config.json

----- 查看默认的配置
dbdeployer defaults show

----- 通过命令行修改参数
dbdeployer defaults update sandbox-binary /opt/MySQL/sandboxes
dbdeployer defaults update reserved-ports '7001,10000,15000'
dbdeployer defaults update reserved-ports '1186,3306,33060,7001,10000,15000'
```

## 其它

### 支持特性

```
----- 包括了特性以及对应的分支，可利用分支、版本过滤
dbdeployer admin capabilities
dbdeployer admin capabilities mysql
dbdeployer admin capabilities mysql 5.7.11
```

### 杂项

```
----- 一些常见的示例程序
dbdeployer cookbook list

----- 导入测试库数据，以官方的employees为例，先切换到测试库目录下
cd /opt/MySQL/package/test_db-master
${SANDBOX_HOME}/msb_8_0_21/use -u root
mysql > source employees.sql
```

## 配置文件

会使用 `$HOME/.dbdeployer/config.json` 配置文件，默认不存在，可以手动生成。

```
----- 将默认配置导出
dbdeployer defaults export ~/.dbdeployer/config.json

----- 查看默认的配置
dbdeployer defaults show

----- 通过命令行修改参数
dbdeployer defaults update sandbox-binary /opt/MySQL/sandboxes
```

## 参考

* [配置信息](https://github.com/datacharmer/dbdeployer/wiki/sandbox-customization)
