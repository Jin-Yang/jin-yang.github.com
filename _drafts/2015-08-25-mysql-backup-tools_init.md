---
title: MySQL 备份工具
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,备份,mysqldump,mysqlbinlog,xtrabackup
description: 为了保证数据安全，都会对硬件做高可用，防止出现单点故障，但是无论如何都无法取代备份，尤其对于数据库中所保存的数据而言。在此，介绍一下 MySQL 中常用的备份方法。
---

为了保证数据安全，都会对硬件做高可用，防止出现单点故障，但是无论如何都无法取代备份，尤其对于数据库中所保存的数据而言。

在此，介绍一下 MySQL 中常用的备份方法。

<!-- more -->

## 简介

MySQL 自带了 mysqldump、mysqlbinlog 两个备份工具，percona 又提供了强大的 XtraBackup，加上开源的 mydumper，还有基于主从同步的延迟备份、从库冷备等方式，以及基于文件系统快照的备份，其实选择已经多到眼花缭乱。

备份本身是为了能在出现故障时，可以迅速、准确恢复到原有状态，如果同时可以简单实用，那就最好不过了。接下来，我们看看这几种常用的备份工具。

## mysqldump mydumper

mysqldump 是最简单的逻辑备份方式，在备份 myisam 表的时候，如果要得到一致的数据，就需要锁表。

备份 innodb 表时，加上 \-\-master-data=1 \-\-single-transaction 选项，在事务开始记录下 binlog pos 点，然后利用 mvcc 来获取一致的数据，由于是一个长事务，在写入和更新量很大的数据库上，将产生非常多的 undo，显著影响性能，所以要慎用。

这种方式可以针对单表备份，可以导出表结构。

mydumper 是 mysqldump 的开源加强版，支持压缩，支持并行备份和恢复，速度比要快很多，但是由于是逻辑备份，仍不是很快。

<!--
2. 基于文件系统的快照

基于文件系统的快照，是物理备份的一种。在备份前需要进行一些复杂的设置，在备份开始时刻获得快照并记录下binlog pos点，然后采用类似copy-on-write的方式，把快照进行转储。转储快照本身会消耗一定的IO资源，而且在写入压力较大的实例上，保存被更改数据块的前印象也会消耗IO，最终表现为整体性能的下降。而且服务器还要为copy-on-write快照预留较多的磁盘空间，这本身对资源也是一种浪费。因此这种备份方式我们使用的不多。


3. Xtrabackup

这或许是最为广泛的备份方式。percona之所以家喻户晓，Xtrabackup应该功不可没。它实际上是物理备份+逻辑备份的组合。在备份innodb表的时候，它拷贝ibd文件，并一刻不停的监视redo log的变化，append到自己的事务日志文件。在拷贝ibd文件过程中，ibd文件本身可能被写"花"，这都不是问题，因为在拷贝完成后的第一个prepare阶段，Xtrabackup采用类似于innodb崩溃恢复的方法，把数据文件恢复到与日志文件一致的状态，并把未提交的事务回滚。如果同时需要备份myisam表以及innodb表结构等文件，那么就需要用flush tables with lock来获得全局锁，开始拷贝这些不再变化的文件，同时获得binlog位置，拷贝结束后释放锁，也停止对redo log的监视。
它的工作原理如下：

由于mysql中不可避免的含有myisam表，同时innobackup并不备份表结构等文件，因此想要完整的备份mysql实例，就少不了要执行flush tables with read lock，而这个语句会被任何查询(包括select)阻塞，在阻塞过程中，它又反过来阻塞任何查询(包括select)。如果碰巧备份实例上有长查询先于flush tables with read lock执行，数据库就会hang住。而当flush tables with read lock获得全局锁后，虽然查询可以执行，但是仍会阻塞更新，所以，我们希望flush tables with read lock从发起到结束，持续的时间越短越好。
为了解决这个问题，有两种比较有效的方法：
1. 尽量不用myisam表。
2. Xtrabackup增加了–rsync选项，通过两次rsync来减少持有全局锁的时间。

优化后的备份过程如下：

    优点：在线热备，全备+增备+流备，支持限速，支持压缩，支持加密。
    缺点：需要获取全局锁，如果遇到长查询，等待时间将不可控，因此要做好监控，必要时杀死长查询或自杀；遇到超大的实例，备份过程较长，redo log太大会影响恢复速度，这种情况下最好采用延迟备份。
-->

## XtraBackup

这是 percona 开发的一个物理备份工具，可以在线对 InnoDB/XtraDB 引擎的表进行物理备份，相比于 mysqldump 来说，效率很不错。

XtraBackup 包含了两个主要的工具 xtrabackup、innobackupex，其中 xtrabackup 只能备份 InnoDB 和 XtraDB 两种数据表；innobackupex 则封装了 xtrabackup 同时可以备份 MyISAM 数据表。

如果通过源码进行编译，需要 boost 库才可以，为了简单起见，可以直接安装二进制文件。

{% highlight text %}
# yum install libev
# rpm -ivh percona-xtrabackup-xxx.rpm
{% endhighlight %}

如果是源码安装，可以直接从 [Github - Percona XtraBackup](https://github.com/percona/percona-xtrabackup) 下载。

{% highlight text %}
innobackupex
xbcrypt
xbstream
xtrabackup
{% endhighlight %}

实际上，在 2.3 版本之前，innobackupex 是通过 perl 实现的，而在此之后则采用 C 进行了重写；为了保持向前兼容，该文件实际上是指向 xtrabackup 的一个符号连接。

### apply-log

一般情况下，在备份完成后，数据尚且不能用于恢复操作，因为备份的数据中可能会包含尚未提交的事务或已经提交但尚未同步至数据文件中的事务。也就是说，此时数据文件仍处理不一致状态；需要回滚未提交的事务及同步已经提交的事务至数据文件，使得数据文件处于一致性状态。

innobakupex 命令通过的 \-\-apply-log 选项可用于实现上述功能。

## 备份

如 [MySQL 复制](/blog/mysql-replication.html) 中所示，假设搭建了一个 Master 服务器。

### 全量备份

可以通过如下步骤进行全量备份。

{% highlight text %}
----- 全量备份到/tmp/percon目录下
$ innobackupex --defaults-file=/tmp/my-master.cnf --user=root \
     --socket=/tmp/mysql-master.sock /tmp/percon
... ...
MySQL binlog position: filename 'mysql-bin.000001', position '1687589', GTID of the last change '0-1-5439'
161205 20:56:49 [00] Writing backup-my.cnf
161205 20:56:49 [00]        ...done
161205 20:56:49 [00] Writing xtrabackup_info
161205 20:56:49 [00]        ...done
xtrabackup: Transaction log of lsn (4397451) to (4397451) was copied.
161205 20:56:49 completed OK!
{% endhighlight %}

当结尾为 __completed OK!__ 时，说明备份成功。备份完成之后，在目录下会有几个比较重要的元数据文件，简单介绍如下。

{% highlight text %}
$ cat xtrabackup_checkpoints
backup_type = full-backuped          # 备份类型
from_lsn = 0
to_lsn = 4397451
last_lsn = 4397451                   # LSN号
compact = 0
recover_binlog_info = 0

$ cat xtrabackup_binlog_info         # 备份时binlog文件及其位置
mysql-bin.000001        1687589 0-1-5439

$ cat xtrabackup_binlog_pos_innodb   # 同上，不包括GTID信息
mysql-bin.000001        1687589
{% endhighlight %}

在使用 innobackupex 进行备份时，可以使用 \-\-no-timestamp 选项来阻止命令自动创建一个以时间命名的目录；此时，该命令将会创建一个 BACKUP-DIR 目录来存储备份数据。


### 全量恢复

假设将其中的一个数据库删除掉，然后可以通过上述的备份恢复该数据库。

{% highlight text %}
----- 关闭数据库
$ mysqladmin -uroot -S /tmp/mysql-master.sock shutdown

----- 备份源数据库，并新建相同目录
$ mv /tmp/mysql-master /tmp/mysql-master-bak
$ mkdir /tmp/mysql-master

----- 准备备份的全量数据
$ innobackupex --apply-log /tmp/percon/2015-08-23_20-55-32/
... ...
xtrabackup: starting shutdown with innodb_fast_shutdown = 1
InnoDB: FTS optimize thread exiting.
InnoDB: Starting shutdown...
InnoDB: Shutdown completed; log sequence number 4406415
161205 21:27:15 completed OK!

----- 恢复全量数据
$ innobackupex --defaults-file=/tmp/my-master.cnf --copy-back --rsync /tmp/percon/2015-08-23_20-55-32/
... ...
161205 21:34:36 completed OK!

----- 启动服务器
$ /opt/mariadb/bin/mysqld --defaults-file=/tmp/my-master.cnf --basedir=/opt/mariadb \
       --datadir=/tmp/mysql-master
{% endhighlight %}

### 增量备份

注意，增量备份需要先有一个全量的备份，假设仍使用上述的备份。

{% highlight text %}
mysql> INSERT INTO test.foobar VALUES(5, "Hobart"), (6, "Raymond");
Query OK, 2 rows affected (0.01 sec)
Records: 2  Duplicates: 0  Warnings: 0

----- 执行增量备份
$ innobackupex --defaults-file=/tmp/my-master.cnf --user=root \
   --socket=/tmp/mysql-master.sock --incremental /tmp/percon/ \
    --incremental-basedir=/tmp/percon/2015-08-23_20-55-23/ --parallel=2
... ...
xtrabackup: Transaction log of lsn (4408143) to (4408143) was copied.
161205 21:49:34 completed OK!


mysql> INSERT INTO test.foobar VALUES(7, "Jeremy"), (8, "Philip");
Query OK, 2 rows affected (0.01 sec)
Records: 2  Duplicates: 0  Warnings: 0

$ innobackupex --defaults-file=/tmp/my-master.cnf --user=root \
   --socket=/tmp/mysql-master.sock --incremental /tmp/percon/ \
    --incremental-basedir=/tmp/percon/2015-08-23_21-48-23/ --parallel=2
... ...
xtrabackup: Transaction log of lsn (4408743) to (4408743) was copied.
161205 21:59:20 completed OK!
{% endhighlight %}

### 增量备份恢复

对于如上的增量备份，恢复时需要执行如下的 3 个步骤。

1. 准备完全备份；

2. 将增量备份恢复到完全备份，开始恢复的增量备份要添加 \-\-redo-only 参数，到最后一次增量备份要去掉该参数；

3. 对整体的完全备份进行恢复，回滚未提交的数据。

{% highlight text %}
mysql> DROP DATABASE TEST;

----- 将增量1应用到完全备份
$ innobackupex --apply-log --redo-only /tmp/percon/2015-08-23_20-55-23/ \
    --incremental-dir=/tmp/percon/2015-08-23_21-48-23/

----- 将增量2应用到完全备份
$ innobackupex --apply-log /tmp/percon/2015-08-23_21-48-23 --incremental-dir=/tmp/pecon/2015-08-23_21-58-40/

----- 把所有合在一起的完全备份整体进行一次apply操作，回滚未提交的数据
$ innobackupex --apply-log /tmp/percon/2015-08-23_20-55-23/

----- 恢复数据
$ innobackupex --defaults-file=/tmp/my-master.cnf --copy-back --rsync /tmp/percon/2015-08-23_20-55-23/
{% endhighlight %}

另外，可以对数据进行压缩，在此暂不讨论。


## 执行流程

如下是 xtrabackup 执行的详细流程。

![mysql percona xtrabackup procedure]({{ site.url }}/images/databases/mysql/pxb-backup-procedure.png){: .pull-center}

<!--

1. 在启动后，会先 fork 一个进程，启动 xtrabackup进程，然后就等待 xtrabackup 备份完 ibd 数据文件；
xtrabackup 在备份 InnoDB 相关数据时，是有2种线程的，1种是 redo 拷贝线程，负责拷贝 redo 文件，1种是 ibd 拷贝线程，负责拷贝 ibd 文件；redo 拷贝线程只有一个，在 ibd 拷贝线程之前启动，在 ibd 线程结束后结束。xtrabackup 进程开始执行后，先启动 redo 拷贝线程，从最新的 checkpoint 点开始顺序拷贝 redo 日志；然后再启动 ibd 数据拷贝线程，在 xtrabackup 拷贝 ibd 过程中，innobackupex 进程一直处于等待状态（等待文件被创建）。
xtrabackup 拷贝完成idb后，通知 innobackupex（通过创建文件），同时自己进入等待（redo 线程仍然继续拷贝）;
innobackupex 收到 xtrabackup 通知后，执行FLUSH TABLES WITH READ LOCK (FTWRL)，取得一致性位点，然后开始备份非 InnoDB 文件（包括 frm、MYD、MYI、CSV、opt、par等）。拷贝非 InnoDB 文件过程中，因为数据库处于全局只读状态，如果在业务的主库备份的话，要特别小心，非 InnoDB 表（主要是MyISAM）比较多的话整库只读时间就会比较长，这个影响一定要评估到。
当 innobackupex 拷贝完所有非 InnoDB 表文件后，通知 xtrabackup（通过删文件） ，同时自己进入等待（等待另一个文件被创建）；
xtrabackup 收到 innobackupex 备份完非 InnoDB 通知后，就停止 redo 拷贝线程，然后通知 innobackupex redo log 拷贝完成（通过创建文件）；
innobackupex 收到 redo 备份完成通知后，就开始解锁，执行 UNLOCK TABLES；
最后 innobackupex 和 xtrabackup 进程各自完成收尾工作，如资源的释放、写备份元数据信息等，innobackupex 等待 xtrabackup 子进程结束后退出。




在上面描述的文件拷贝，都是备份进程直接通过操作系统读取数据文件的，只在执行 SQL 命令时和数据库有交互，基本不影响数据库的运行，在备份非 InnoDB 时会有一段时间只读（如果没有MyISAM表的话，只读时间在几秒左右），在备份 InnoDB 数据文件时，对数据库完全没有影响，是真正的热备。

InnoDB 和非 InnoDB 文件的备份都是通过拷贝文件来做的，但是实现的方式不同，前者是以page为粒度做的(xtrabackup)，后者是 cp 或者 tar 命令(innobackupex)，xtrabackup 在读取每个page时会校验 checksum 值，保证数据块是一致的，而 innobackupex 在 cp MyISAM 文件时已经做了flush（FTWRL），磁盘上的文件也是完整的，所以最终备份集里的数据文件都是写入完整的。
增量备份

PXB 是支持增量备份的，但是只能对 InnoDB 做增量，InnoDB 每个 page 有个 LSN 号，LSN 是全局递增的，page 被更改时会记录当前的 LSN 号，page中的 LSN 越大，说明当前page越新（最近被更新）。每次备份会记录当前备份到的LSN（xtrabackup_checkpoints 文件中），增量备份就是只拷贝LSN大于上次备份的page，比上次备份小的跳过，每个 ibd 文件最终备份出来的是增量 delta 文件。

MyISAM 是没有增量的机制的，每次增量备份都是全部拷贝的。

增量备份过程和全量备份一样，只是在 ibd 文件拷贝上有不同。
恢复过程

如果看恢复备份集的日志，会发现和 mysqld 启动时非常相似，其实备份集的恢复就是类似 mysqld crash后，做一次 crash recover。

恢复的目的是把备份集中的数据恢复到一个一致性位点，所谓一致就是指原数据库某一时间点各引擎数据的状态，比如 MyISAM 中的数据对应的是 15:00 时间点的，InnoDB 中的数据对应的是 15:20 的，这种状态的数据就是不一致的。PXB 备份集对应的一致点，就是备份时FTWRL的时间点，恢复出来的数据，就对应原数据库FTWRL时的状态。

因为备份时 FTWRL 后，数据库是处于只读的，非 InnoDB 数据是在持有全局读锁情况下拷贝的，所以非 InnoDB 数据本身就对应 FTWRL 时间点；InnoDB 的 ibd 文件拷贝是在 FTWRL 前做的，拷贝出来的不同 ibd 文件最后更新时间点是不一样的，这种状态的 ibd 文件是不能直接用的，但是 redo log 是从备份开始一直持续拷贝的，最后的 redo 日志点是在持有 FTWRL 后取得的，所以最终通过 redo 应用后的 ibd 数据时间点也是和 FTWRL 一致的。

所以恢复过程只涉及 InnoDB 文件的恢复，非 InnoDB 数据是不动的。备份恢复完成后，就可以把数据文件拷贝到对应的目录，然后通过mysqld来启动了。


-->


## mysqlbinlog

上述备份方式，都只能把数据库恢复到备份时的时间点：A) mysqldump、mydumper、snapshot 是备份开始的时间点；B) XtraBackup 是备份结束的时间点。

如果要实现 point in time 恢复，还必须备份 binlog；开启 binlog 非常简单，可以通过修改如下配置文件打开。

{% highlight text %}
$ vi /etc/my.cnf
binlog_format = mixed
log-bin       = mysql-bin
{% endhighlight %}

其中 mysql 5.6 提供了远程备份 binlog 的能力，我么可以直接通过如下命令远程备份。

{% highlight text %}
$ mysqlbinlog --raw --read-from-remote-server --stop-never
{% endhighlight %}

该命令会伪装成 mysql 从库，从远程获取 binlog 然后进行转储，不过这样状态监控需要单独部署；当然，还可以通过 blackhole 来备份全量的 binlog 。

## 常用解析

通过 mysqlbinlog 工具可以将 binlog 或者 relay-log 文件解析成文本文件，两者的格式相同。

{% highlight text %}
$ mysqlbinlog binlog.0000003
{% endhighlight %}

如上命令将会解析 binlog ，并输出该文件中的所有语句及其相关信息，例如每个语句花费的时间、客户发出的线程ID、发出线程时的时间戳等等。

也可以通过 mysql 客户端查看 binlog 相关信息。

{% highlight text %}
mysql> SHOW BINARY LOGS;                           # 查看binlog日志
+-----------------+-----------+
| Log_name        | File_size |
+-----------------+-----------+
| mysql-bin.000001|       409 |
| mysql-bin.000002|       346 |
+-----------------+-----------+
2 rows in set (0.00 sec)

mysql> SHOW BINLOG EVENTS;                         # 查看执行的SQL，默认是1
mysql> SHOW BINLOG EVENTS IN 'mysql-bin.000002';   # 查看指定binlog文件的内容
mysql> SHOW BINLOG EVENTS FROM 213;                # 指定位置binlog的内容
{% endhighlight %}

接下来看看如何通过 mysqlbinlog 方式提取 binlog 日志。

{% highlight text %}
----- 提取整个binlog日志，压缩，执行
$ mysqlbinlog mysql-bin.000001
$ mysqlbinlog mysql-bin.000001 | gzip > foobar.sql.gz
$ mysqlbinlog mysql-bin.000001 | mysql -uroot -p

----- 提取指定位置的binlog日志
$ mysqlbinlog --start-position="120" --stop-position="332" mysql-bin.000001

----- 指定数据库、字符集、开始时间、结束时间的binlog并输出到日志文件
$ mysqlbinlog --database=test --set-charset=utf8 \
   --start-datetime="2015-08-23 21:15:23" --stop-datetime="2015-08-23 22:15:23" \
   --result-file=foobar.sql mysql-bin.000002 mysql-bin.000003

----- 远程提取日志，指定结束时间
# mysqlbinlog -uroot -p -h127.1 -P3306 --stop-datetime="2015-08-23 22:30:23" \
   --read-from-remote-server mysql-bin.000033 | less

----- 远程提取使用row格式的binlog日志并输出到本地文件
# mysqlbinlog -uroot -p -P3306 -h127.1 --read-from-remote-server -vv \
   mysql-bin.000005 > foobar.sql

常用参数：
   -p, --password[=name]      # 密码
   -P, --port[=num]           # 端口号
   -u, --user=name            # 登陆用户名
   -h, --host=name            # 主机地址
   -S, --socket=name          # 套接字文件
   -d, --database=name        # 只列出该数据库的条目(只适用本地日志)
   --protocol=name            # 连接协议
   --server-id[=num]          # 仅提取指定id的binlog日志

   --set-charset=name         # 添加SET NAMES character_set到输出
   -r, --result-file=name     # 输出指向给定文件
   -s, --short-form           # 只显示包含的语句

   --start-datetime=name      # 等于或大于该值的事件
   --stop-datetime=name       # 小于或等于该值的事件

   -j, --start-position[=num] # 从指定位置开始读取事件
   --stop-position[=num]      # 到该位置停止
   -o, --offset[=num]         # 跳过前num个条目

   -f, --force-read
       无法识别二进制事件，打印警告并忽略该事件继续；否则停止
   -H, --hexdump
       在注释中显示日志的十六进制转储，可以用于调试
{% endhighlight %}
<!--
   -l, --local-load=name
     为指定目录中的LOAD DATA INFILE预处理本地临时文件。

   -R, --read-from-remote-server|--read-from-remote-master=name
     从MySQL服务器读二进制日志。如果未给出该选项，任何连接参数选项将被忽略，即连接到本地。
     这些选项是–host、–password、–port、–protocol、–socket和–user。

   -t, --to-last-log
     在MySQL服务器中请求的二进制日志的结尾处不停止，而是继续打印直到最后一个二进制日志的结尾。
     如果将输出发送给同一台MySQL服务器，会导致无限循环。该选项要求–read-from-remote-server。

   -D, --disable-log-bin
     禁用二进制日志。如果使用–to-last-logs选项将输出发送给同一台MySQL服务器，可以避免无限循环。
     该选项在崩溃恢复时也很有用，可以避免复制已经记录的语句。注释：该选项要求有SUPER权限。

   -v, --verbose
     用于输出基于row模式的binlog日志，-vv为列数据类型添加注释
-->

mysqlbinlog 可以基于 server_id，以及基于数据库级别提取日志，不支持表级别。




## 参考

关于 XtraBackup 可以直接参考官方网站 [Percona XtraBackup - Documentation](https://www.percona.com/doc/percona-xtrabackup/index.html)。


{% highlight text %}
{% endhighlight %}
