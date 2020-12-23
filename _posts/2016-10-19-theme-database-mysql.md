---
title: 【专题】MySQL 关系型数据库
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---

<!-- more -->

![MySQL Logo]({{ site.url }}/images/databases/mysql/mysql-mariadb-percona-logo.png "MySQL Logo"){: .pull-center}

MySQL 是一款最流行的开源关系型数据库，最初由瑞典的 MySQL AB 公司开发，目前已被 Oracle 收购，现在比较流行的开源分支包括了 MariaDB 和 Percona。

其中 MariaDB 由 MySQL 创始人 Michael Widenius 主导开发，主要原因之一是：Oracle 收购了 MySQL 后，有将 MySQL 闭源的潜在风险，因此社区采用分支的方式来避开这个风险。为了与原 MySQL 区分，不再使用原来的版本号，而是采用新的 10.0。

Percona 是最接近官方 MySQL Enterprise 发行版的版本，也就是说它提供了一些 MySQL 企业版采用的功能，并且包括了一些比较好用的常用工具。其中的缺点是，为了确保对产品中所包含功能的控制，他们自己管理代码，并不接受社区开发人员的贡献。

### 文章

* [MySQL 写在开头](/post/mysql-begin.html)，主要保存一些经常使用的 MySQL 资源。
* [MySQL 简单介绍](/post/mysql-introduce.html)，简单介绍 MySQL 常见的使用方法，包括安装启动、客户端使用、调试等。
* [MySQL 基本概念](/post/mysql-basic.html)，介绍 MySQL 中一些基本的概念，包括了 SQL、JOIN、常见测试库等。
* [MySQL 监控指标](/post/mysql-monitor.html)，包括了一些 MySQL 常见的监控指标及其含义等。
* [MySQL Handler 监控](/post/mysql-handler.html)，主要介绍下监控指标中与 handler_read_* 相关的内容。
* [MySQL 用户管理](/post/mysql-users.html)，一些用户相关的操作，包括了用户管理、授权、密码恢复等。
* [MySQL 通讯协议](/post/mysql-protocol.html)，简单介绍 MySQL 的服务器与客户端是如何进行通讯的。
* [MySQL 语法解析](/post/mysql-parser.html)，SQL 的处理过程包括了词法分析、语法分析、语义分析、构造执行树等。
* [MySQL 插件详解](/post/mysql-plugin.html)，关于 MySQL 中一些插件功能的实现，主要是一些通用插件的介绍。
* [MySQL 存储引擎](/post/mysql-storage-engine-plugin.html)，介绍下与存储引擎相关的内容，包括了提供的接口，实现方法等。
* [MySQL 线上部署](/post/mysql-deploy-online.html)，简单记录一些线上部署时常见的配置内容。
* [MySQL 执行简介](/post/mysql-executor.html)，简单介绍 MySQL 中的查询最终是如何执行的。
* [MySQL 自带工具](/post/mysql-tools-internal.html)，简单介绍下 MySQL 中自带的工具集。
* [MySQL 常用工具](/post/mysql-tools.html)，一些运维过程中常见的三方工具，包括压测工具。
* [MySQL Sandbox](/post/mysql-sandbox.html)，本地搭建多个 MySQL 实例的工具，包括主备、循环复制、一主多备等等。
* [MySQL 启动脚本](/post/mysql-mysqld-safe.html)，详细介绍下 mysqld_safe 脚本的执行流程。
* [MySQL Core 文件](/post/mysql-core-file.html)，一些关于 CoreDump 文件以及 debuginfo 的介绍。
* [MySQL 关闭过程](/post/mysql-shutdown.html)，简单分析下 mysqld 进程关闭的过程，以及关闭过程中执行的操作。
* [MySQL 杂项](/post/mysql-tips.html)，简单记录下 MySQL 常见的一些操作。

### 高可用

* [MySQL 日志相关](/post/mysql-log.html)，一些常见的日志介绍，同时也包括了 binlog 的详细介绍。
* [MySQL 组提交](/post/mysql-group-commit.html)，主要是关于 binlog 的组提交实现，介绍各个阶段的实现原理。
* [MySQL 复制方式](/post/mysql-replication.html)，数据复制方法，也是一些高可用解决方案的基础，介绍概念、配置方式。
* [MySQL 复制源码解析](/post/mysql-replication-sourcecode.html)，从源码的角度看看复制是如何执行的。
* [MySQL 半同步复制](/post/mysql-semisync.html)，关于半同步复制的详细解析，包括了源码的实现方式。
* [MySQL GTID 简介](/post/mysql-gtid.html)，主要介绍下 GTID 配置、实现方式，有那些限制，运维场景等。
* [MySQL Crash-Safe 复制](/post/mysql-crash-safe-replication.html)，在主备复制时，如何保证数据的一致性，当然主要是备库。
* [MySQL 主备数据校验](/post/mysql-replication-pt-table-checksum.html)，由于各种原因，主从架构可能会出现数据不一致，简单介绍校验方式。
* [MySQL 高可用 MHA](/post/mysql-replication-mha.html)，相对成熟的方案，能做到30秒内自动故障切换，且尽可能保证数据一致性。
* [MySQL 组复制](/post/mysql-group-replication.html)，也就是基于 Paxos 协议变体实现，提供了一种高可用、强一致的实现。

### InnoDB

* [InnoDB 简单介绍](/post/mysql-innodb-introduce.html)，介绍一下与 InnoDB 相关的资料。
* [InnoDB 隔离级别](/post/mysql-innodb-isolation-level.html)，主要介绍下 InnoDB 中如何使用事务的隔离级别。
* [InnoDB Redo Log](/post/mysql-innodb-redo-log.html)，介绍 redo log 相关。
* [InnoDB Checkpoint](/post/mysql-innodb-checkpoint.html)，介绍 InnoDB 中与 checkpoint 相关内容，包括如何写入、何时写入。
* [InnoDB Double Write Buffer](/post/mysql-innodb-double-write-buffer.html)，介绍为什么会有 dblwr 机制，以及 InnoDB 中如何实现。
* [InnoDB 崩溃恢复](/post/mysql-innodb-crash-recovery.html)，简单介绍下在服务器启动的时候执行崩溃恢复的流程。


<!--
* [MySQL 配置文件](/post/mysql-config.html)，关于配置相关的内容。
* [MySQL 链接方式](/post/mysql-connection.html)，实际上就是线程与链接的处理方式，主要包括了三种。
* [MySQL Handler 监控](/post/mysql-handler.html)，实际上时监控中的 handler 相关的内容。
* [MySQL MyISAM](/post/mysql-myisam.html)，关于 MySQL 中经典的 MyISAM 的介绍。
* [MySQL 代码导读](/post/mysql-skeleton.html)，也就是代码脉络的大致导读。
* [MySQL 事务处理](/post/mysql-transaction.html)，也就是 MySQL 中的事务处理方法。
* [MySQL 存储引擎](/post/mysql-storage-engine-plugin.html)，实际是插件的一个特例，不过使用比较复杂，所以就单独作为一篇。
* [MySQL 备份](/post/mysql-backup.html)，介绍 MySQL 一些常见的备份方法。
* [MySQL 高可用](/post/mysql-high-availability.html)，介绍 MySQL 中的常用高可用解决方案。
* [MySQL 安全设置](/post/mysql-security.html)，也就是一些对 MySQL 进行加固的方法。

InnoDB:

* [InnoDB 线程](/post/mysql-innodb-threads.html)，介绍下 InnoDB 中与线程相关的资料。
* [InnoDB Buffer Pool](/post/mysql-innodb-buffer-pool.html)，
* [InnoDB Insert Buffer](/post/mysql-innodb-insert-buffer.html)，
-->

{% highlight text %}
{% endhighlight %}
