---
title: ProxySQL 简单介绍
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,linux,database,proxysql
description: 一个强大灵活的 MySQL 代理层，支持读写分离、Query 路由、配置动态加载、故障切换等等。
---

一个强大灵活的 MySQL 代理层，支持读写分离、Query 路由、配置动态加载、故障切换等等。

<!-- more -->

![proxysql logo]({{ site.url }}/images/databases/proxysql-logo.png "proxysql logo"){: .pull-center width="70%" }

## 安装

ProxySQL 中有些脚本是通过 Perl 编写的，而 Perl 在连接数据库时需要安装 DBI 模块和相应数据库的 DBD 驱动，Linux 上默认安装的 Perl 是没有该模块的，所以要手动安装。

在 CentOS 中可以通过 `yum list all | grep DB` 查看并安装依赖的包。

### 配置文件

相关的配置文件保存在 `/etc/proxysql.cnf` 文件中，需要注意的是，该配置文件只有在第一次时生效，而后面的配置会保存到一个 SQLiteDB 中，如果两个文件同时存在，也就是非第一次启动，会读取 SQLiteDB 中的内容，而忽略该配置文件。

{% highlight text %}
admin_variables=
{
        admin_credentials="admin:admin"
        mysql_ifaces="127.0.0.1:6032;/tmp/proxysql_admin.sock"
}
{% endhighlight %}

ProxySQL 提供了类似 MySQL 的操作，可以直接通过 MySQL 客户端建立连接，并修改相关的配置，如上定义了 `admin` 也即是管理用户的相关配置。

### 启动

在安装包中有一个 `/etc/init.d/proxysql` 脚本可以用来操作 ProxySQL ，简单来说就是设置环境变量，并在执行前进行状态检查。

{% highlight text %}
/etc/init.d/proxysql start
/etc/init.d/proxysql stop
/etc/init.d/proxysql status
{% endhighlight %}

如果想要通过 systemd 管理 ProxySQL，可以添加 `proxysql.service` 文件，添加如下内容：

{% highlight text %}
# vim /usr/lib/systemd/system/proxysql.service
[Unit]
Description=High Performance Advanced Proxy for MySQL
After=network.target

[Service]
Type=simple
User=mysql
Group=mysql
PermissionsStartOnly=true
LimitNOFILE=102400
LimitCORE=1073741824
ExecStartPre=/bin/mkdir -p /var/lib/proxysql
ExecStartPre=/bin/chown mysql:mysql -R /var/lib/proxysql /etc/proxysql.cnf
ExecStart=/usr/bin/proxysql -f
Restart=always
{% endhighlight %}

为绝大多数配置都可以在线修改，所以一般 ProxySQL 很少停止或重启。

### 连接

连接方式与 MySQL 客户端相同，所以一些 MySQL 相关的配置也可以应用到 ProxySQL 中，例如在 `~/.my.cnf` 文件中添加如下内容，用于设置命令行提示符：

{% highlight text %}
[client]
prompt = "\\u@\\h:\\d \\r:\\m:\\s> "
{% endhighlight %}

然后对于上述的配置可以通过如下的命令行登录。

{% highlight text %}
$ mysql -uadmin -padmin -h127.0.0.1 -P6032
admin@127.0.0.1:(none) 10:53:38> SHOW DATABASES;
+-----+---------------+-------------------------------------+
| seq | name          | file                                |
+-----+---------------+-------------------------------------+
| 0   | main          |                                     |
| 2   | disk          | /var/lib/proxysql/proxysql.db       |
| 3   | stats         |                                     |
| 4   | monitor       |                                     |
| 5   | stats_history | /var/lib/proxysql/proxysql_stats.db |
+-----+---------------+-------------------------------------+
5 rows in set (0.01 sec)
admin@127.0.0.1:(none) 10:53:38> SHOW TABLES FROM main;
{% endhighlight %}

其中包括了如下的几个核心的配置：

* main 内存配置数据库，包括了后端 DB 实例信息、用户验证、路由规则等。
* disk 持久化到硬盘的配置，使用的是 SQLite 数据库。
* stats 包含的是统计信息，包括了命令执行次数、流量、Processlist、执行时间等等。
* monitor 对后端 DB 的健康、延迟检查等信息。

在 `main` 数据库中，有大量以 `runtime_` 开头的表，标示了 ProxySQL 当前运行的配置内容，不能通过 DML 语句直接修改，只能修改对应的非 `runtime_` 开头的表 (同样在 `main` 数据库)，然后 LOAD 使其生效， SAVE 使其存到硬盘以供下次重启加载。

## 配置

ProxySQL 有一个非常完善的配置系统，支持配置修改后的动态生效，整个配置系统分三层设计，其主要目的是 A) 自动更新；B) 支持配置的动态修改；C) 回滚错误配置。

整个配置系统的示例如下，这三层是独立配置的，可以同时用于三份不同的配置，配置项可以相同，也可以不同。

{% highlight text %}
+-------------------------+
|         RUNTIME         |
+-------------------------+
       /|\          |
        |           |
    [1] |       [2] |
        |          \|/
+-------------------------+
|         MEMORY          |
+-------------------------+ _
       /|\          |      |\
        |           |        \
    [3] |       [4] |         \ [5]
        |          \|/         \
+-------------------------+  +-------------------------+
|          DISK           |  |       CONFIG FILE       |
+-------------------------+  +-------------------------+
{% endhighlight %}

#### RUNTIME

代表了 ProxySQL 当前正在使用的配置，无法直接修改，需要从下一层加载进来，因为直接回影响到生产，所以加载前需要三思。其中配置包括了 `global_variables` `mysql_servers` `mysql_users` `mysql_query_rules` 。

#### MEMORY

这是一个中间层，可以随意修改而不影响运行配置参数，或者持久化到磁盘。修改完成确认无误后，可以再接入到生产或者持久化到磁盘。

也就是说，通过 MySQL 客户端可以直接连接到 ProxySQL 代理，然后查询并修改 MEMORY 中的配置，修改不会直接影响生产，也不影响磁盘中保存的数据。

通过此接口可以修改 `mysql_servers` `mysql_users` `mysql_query_rules` `global_variables` 等。

#### DISK

也就是持久化的部分。

### 加载顺序

当 ProxySQL 启动时，首先读取配置文件 (默认是 `/etc/proxysql.cnf` )，然后从该配置文件中读取 `datadir` 配置项，在该目录下保存了 SQLite 数据库。如果存在 SQLite 数据库文件，那么会将其中的配置项加载到 RUNTIME ，用来初始化 ProxySQL 的运行。

如果第一次启动时，此时没有 SQLite 数据库文件，那么会直接拿配置文件中的配置来初始化 ProxySQL 的运行态参数。

### 修改配置

大部分的配置在修改后可以动态生效，目前有几个配置在修改后需要重启才会生效，例如 A) `mysql-threads` 工作线程数；B) `mysql-stacksize` 线程栈的大小。

在配置文件中，除了上述的管理面接口之外，同时还提供了 Proxy 接口，对应配置文件中的如下内容。

{% highlight text %}
mysql_variables=
{
	interfaces="0.0.0.0:6033"
}
{% endhighlight %}

## 示例 MySQL

也就是安装一个简单的 MySQL 库，用来进行测试。

在 CentOS 中可以通过 `yum install mariadb-server` 进行安装，默认安装的还是比较老的版本，貌似是 `5.5.60` ，也可以直接下载更高版本，然后按照如下步骤进行配置。

{% highlight text %}
$ cat /opt/mysql/my3307.cnf
[mysqld]
port = 3307
server-id = 3307
basedir = /usr
socket = /opt/mysql/3307/mysql.sock
pid-file = /opt/mysql/3307/mysql.pid
datadir = /opt/mysql/3307
{% endhighlight %}

### 初始化

{% highlight text %}
----- 初始化DB目录
mysql_install_db --user=mysql --defaults-file=/opt/mysql/my3307.cnf
{% endhighlight %}

新建一个 `mariadb3307.service` 文件，将 `ExecStartPre` 和 `ExecStartPost` 删除，并将 `ExecStart` 选项修改为如下：

{% highlight text %}
ExecStart=/usr/bin/mysqld_safe --defaults-file=/opt/mysql/my3307.cnf --basedir=/usr
{% endhighlight %}

修改完后，通过 `systemctl daemon-reload` 命令重新加载，然后通过 `systemctl start mariadb3308` 启动进程。

接着通过如下命令修改密码、登录 MySQL 。

{% highlight text %}
$ /usr/bin/mysqladmin -h 127.1 -P3307 -u root password 'new-password'
$ mysql -uroot -h127.0.0.1 -P3307 -p'new-password'
{% endhighlight %}

## 简单示例

一个最简单的示例，在 ProxySQL 后端挂一个 MySQL 服务器，并通过 ProxySQL 代理进行访问。这里在 CentOS 中，在本地启动一个 MySQL 服务器，操作步骤如下。

#### 添加主机信息

登陆 ProxySQL 添加后端主机组，其中的 HostGroup 会与后面的用户信息相关联。

{% highlight text %}
INSERT INTO mysql_servers(hostgroup_id, hostname, port) VALUES(1, '127.0.0.1', 3307);
SELECT * FROM mysql_servers;
{% endhighlight %}

然后使配置生效。

{% highlight text %}
LOAD MYSQL SERVERS TO RUNTIME;
SAVE MYSQL SERVERS TO DISK;
{% endhighlight %}

#### 添加账号

其中包括了两种，分别是：A) 可以访问主机的业务账号；B) 查看 DB 健康情况的监控账号。

{% highlight text %}
----- DB 中添加相关的访问账号，然后可以通过mysql客户端判断是否可以访问
mysql> GRANT ALL ON *.* TO 'proxysql'@'%' IDENTIFIED BY '123456';
$ mysql -uproxysql -h127.0.0.1 -P3307 -p'123456'
{% endhighlight %}

然后在 ProxySQL 主机 `mysql_users` 表中添加刚才创建的账号，ProxySQL 客户端需要使用这个账号来访问数据库。

{% highlight text %}
INSERT INTO mysql_users(username, password, default_hostgroup, transaction_persistent)
		VALUES('proxysql', '123456', 1, 1);
{% endhighlight %}

然后使配置生效。

{% highlight text %}
LOAD MYSQL USERS TO RUNTIME;
SAVE MYSQL USERS TO DISK;
{% endhighlight %}

#### 登录

默认 ProxySQL 通过 6033 端口提供服务，对于上述的配置，可以通过如下方式登录。

{% highlight text %}
$ mysql -uproxysql -p123456 -h127.0.0.1 -P6033
{% endhighlight %}

## 参考

[Mini HOW TO on ProxySQL Configuration](https://github.com/sysown/proxysql/wiki/ProxySQL-Configuration)

<!--
各个表的详细配置可以参考
http://www.cnblogs.com/zhoujinyi/p/6829983.html

https://github.com/sysown/proxysql/wiki/Frequently-Asked-Questions

DBProxy介绍
http://www.cnblogs.com/zhoujinyi/p/6697141.html
-->

{% highlight text %}
{% endhighlight %}
