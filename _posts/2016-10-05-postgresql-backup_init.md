---
Date: October 19, 2013
title: PG 备份
layout: post
comments: true
language: chinese
category: [sql]
---

<!-- more -->

# Dump

通过 pg_dump 导出数据时，需要读权限，dump 时不会阻塞其它操作，当然不包括需要排他锁的操作，如 ALTER TABLE 。

{% highlight text %}
----- 可以按照不同的级别备份
$ pg_dump -U postgres dbname > outfile.sql
$ pg_dump -U postgres -n schema-name > outfile.sql
$ pg_dump -U postgres -t table-name > outfile.sql

----- 在一个事务中恢复，如果出错则退出
$ psql --single-transaction --set ON_ERROR_STOP=on dbname < infile.sql

----- 重新生成统计数据
postgres=# ANALYZE

----- 将数据导出到另外一个数据库
$ pg_dump -h host1 dbname | psql -h host2 dbname
{% endhighlight %}

pg_dump 只会备份一个数据库，不会备份 roles、tablespaces 等信息，为此可以通过 pg_dumpall 导出。

{% highlight text %}
----- 全部备份
$ pg_dumpall > outfile
----- 只备份全局数据
$ pg_dumpall --globals-only > outfile

----- 恢复数据
$ psql -f infile postgres
{% endhighlight %}



# PITR (Point In Time Recovery)

{% highlight text %}
----- 1. 创建表，并插入100W条记录
postgres=# CREATE TABLE foobar(id INT);
CREATE TABLE
postgres=# INSERT INTO foobar VALUES(generate_series(1, 1000000));
INSERT 0 1000000

----- 2. 通过如下两种方式可以查看现在数据库的大小
postgres=# SELECT oid FROM pg_database WHERE datname='postgres';
  oid
-------
 13294
(1 row)
postgres=# SELECT pg_size_pretty(pg_database_size('postgres'));
 pg_size_pretty
----------------
 42 MB
(1 row)
$ cd $PGDATA/base/13294 && du -sh
42M    .

----- 3. 在$PGDATA/postgresql.conf配置文件中添加如下内容，设置日志归档
wal_level = archive
archive_mode = on
archive_timeout = 300   # 单位是秒，5分钟强制归档
archive_command = 'cp %p /tmp/archive/%f'
{% endhighlight %}

archive_command 会在每次 WAL 日志 16MB 段满的时候才执行，即把其拷贝到 /tmp/archive 目录下，或者是超时同样会做归档。配置完后重启服务器，使各个参数生效，然后开始准备实验。

### 1. 做一次基础备份

{% highlight text %}
----- 建立一个目录用于存储基础备份
$ mkdir -p /tmp/backup

----- 执行SQL，开始备份，在archive目录下生成00000001000000000000000B.00000028.backup文件
postgres=# SELECT pg_start_backup('bak20160824');
 pg_start_backup
-----------------
 0/B000028
(1 row)

----- 将$PGDATA/base目录下的数据保存
$ tar -czvf /tmp/backup/base.tar.gz /var/lib/pgsql/data

----- 执行如下SQL
postgres=# SELECT pg_stop_backup();
NOTICE:  pg_stop_backup complete, all required WAL segments have been archived
 pg_stop_backup
----------------
 0/B0000C0
(1 row)

----- 切换日志，可以多执行几次
postgres=# SELECT pg_switch_xlog();
 pg_switch_xlog
----------------
 0/C0000B0
(1 row)
{% endhighlight %}

正常来说，现在已经备份成功，可以查看 /tmp/archive 目录中已经有了备份的 wal 日志段。


接下来进行测试，在原表中再次插入 100W 条记录，并假设此时由于某种原因数据库出问题了，现在的问题是，能否利用之前的“基础备份”+“新产生的WAL日志”恢复数据库。

{% highlight text %}
----- 原表中再次插入100W条记录
postgres=# INSERT INTO foobar VALUES(generate_series(1, 1000000));
INSERT 0 1000000

----- 关闭数据库，并将$PGDATA/data目录改名，模拟故障
# mv /var/lib/pgsql/{data,data.bak}

----- 使用postgres用户，尝试将之前的基础备份拷贝到$PGDATA/base目录下，并解压
$ cp /tmp/backup/base.tar.gz /var/lib/pgsql/base
$ tar -xzvf base.tar.gz

----- 进入解压得到的data目录，删除pg_xlog文件夹，创建pg_xlog/archive_status文件夹
$ rm -rf pg_xlog
$ mkdir -p pg_xlog/archive_status

----- 删除data下的postmaster.pid文件：
$ rm -rf postmaster.pid

----- 从安装的postgresql95-server的包中复制一份recovery.conf
$ cp /usr/pgsql-9.5/share/recovery.conf.sample recovery.conf

----- 修改recovery.conf，添加如下内容
restore_command =  'cp /tmp/archive/%f %p'
{% endhighlight %}

然后启动数据库，查看表中的数据确实已经恢复了。


# recovery.conf

这个文件是在恢复的时候使用的，一旦开始恢复便不能使用。可以直接从安装的 postgresql95-server 包中复制一份 recovery.conf 。

{% highlight text %}
$ cp /usr/pgsql-9.5/share/recovery.conf.sample recovery.conf
{% endhighlight %}

如下的配置参数，用于恢复过程中每个阶段的 HOOK 调用函数。

{% highlight text %}
restore_command = 'cp /mnt/server/archivedir/%f "%p"'
    获取归档文件的 shell 命令，其中归档恢复 (archive recovery) 必须，流复制 (streaming replication) 可选。
      %f : 需要获取的归档文件名。
      %p : 目的地的路径名+文件名，路径为当前工作目录 (PGDATA) 的相对路径。
      %r : 包含最近一次重启点，也就是能够恢复的最早的文件，可以用来清除多余数据。

archive_cleanup_command ( string )
    这是一个可选配置，用在每次重启点 (restartpoint) 执行。主要用于像 standby 服务器清除不需要的 WAL 文件，
    所有 %r 之前的文件都可以删除，通常用于温备。需要注意的是，如果是多个 standby，需要确保所有的服务器都不
    需要才可以删除。

recovery_end_command ( string )
    可选，用于恢复后需要执行的命令。
{% endhighlight %}

PG 默认会恢复到最后的 WAL 日志，当然也可以通过如下的参数指定想要恢复到的日志点，如果配置了多个，那么会选择最新的日志点。

{% highlight text %}
recovery_target = ’immediate’
    目前只有一个值，表示恢复到最新的一致日志点。

recovery_target_name ( string )
    恢复到通过 pg_create_restore_point() 函数指定的时间点。

recovery_target_time ( timestamp )
    指定恢复到的时间点，与 recovery_target_inclusive 配合使用。

recovery_target_xid ( string )
    指定恢复到的事务 ID，不过需要注意的是，事务 ID 是在开始的赋予的，而保存的顺序可能有所不同，同样
    需要与 recovery_target_inclusive 配合使用。
{% endhighlight %}

如下的选项，可以与上述的方式配合使用。

{% highlight text %}
recovery_target_inclusive ( boolean )
    用于指定到了指定的对象后，需要恢复(true)，还是忽略该值 (false)，默认是 true 。

recovery_target_timeline ( string )
    指定恢复的时间线，默认采用与基础备份相同的时间线。

recovery_target_action ( enum )
    指定恢复完成之后采取的操作，默认为 pause 。
      pause :  暂停
      promote : 恢复完成后准备接收新连接
      shutdown : 恢复完后停止
{% endhighlight %}

对于 standby 模式的参数如下。

{% highlight text %}
standby_mode ( boolean )
    指定是否作为 standby 模式，为 on 时，当恢复到了归档 WAL 日志之后仍然会尝试获取新的日志进行恢复。
    获取可以通过 restore_command 或者 primary_conninfo 的配置。

primary_conninfo ( string )
    指定连接到主服务器的连接方式。

primary_slot_name ( string )
    当通过流复制方式连接到主使用的 slot，如果 primary_conninfo 设置了则该配置是无效的。

trigger_file ( string )

recovery_min_apply_delay ( integer )
    默认standby会收到 WAL 后立即恢复，也可以通过该参数添加延迟时间。
{% endhighlight %}











# Warm-Backup







为 了简化本文，关于HA的基础就到此为止，下面看看如何使用warm-standby（基于拷贝WAL文件的方法）来实施HA。首先我们来看看warm- standby的含义，根据http://www.postgresql.org/docs/9.1/interactive/high- availability.html，其中的一段话：

Servers that can modify data are called read/write, master or primary servers. Servers that track changes in the master are called standby or slave servers. A standby server that cannot be connected to until it is promoted to a master server is called a warm standby server, and one that can accept connections and serves read-only queries is called a hot standby server.

从中我们知道，warm-standby在系统出现fault的时候，可以提升为master，即可以接受客户端的connect连接，并提供数据库的读写，角色如同一个master一样。而根据25.5的说明（如下），hot-standby则在为archive recovery 或 standby mode时，只能接受可读的query。

Hot Standby is the term used to describe the ability to connect to the server and run read-only queries while the server is in archive recovery or standby mode. This is useful both for replication purposes and for restoring a backup to a desired state with great precision（这一句什么意思？）.

# 参考

[PostgreSQL9.1 Warm-Standby ---之基于拷贝WAL文件的方法](http://blog.sciencenet.cn/home.php?mod=space&uid=419883&do=blog&quickforward=1&id=539178)








## pg_standby

{% highlight text %}
-d   打印详细的调试信息到stderr。
-s   每隔几秒查看归档目录中是否已经准备好。
-t   指定trigger file，当该文件出现时会进入 failover 。
{% endhighlight %}



restore_command = 'pg_standby -d -s 2 -t /tmp/pgsql.trigger.5442 /tmp/archive %f %p %r 2>>standby.log'


如果没有 recovery.conf 文件，则表示 PG 没有处于 standby 模式。







# Warm-Standby

## 基于拷贝WAL文件的方法

可以分别通过 pg_controldata 命令查看 master 和 standby 的状态，如果正常，则这两者会分别处于 in production 和 in archive recovery 状态。

可以在 master 里插入一些新的数据，然后检验 standby 服务端是否是几乎同时在用 WAL 日志恢复。

{% highlight text %}
CREATE TABLE foobar (id INT);
INSERT INTO foobar VALUES(generate_series(1, 1000000));
{% endhighlight %}

### Failover 阶段

在 Failover 时，master 由于某种原因宕掉。

{% highlight text %}
pg_ctl stop -m fast -D /var/lib/pgsql/master
{% endhighlight %}

确定 master 关机后，需要首先在提升 standby 为 master 之前做一些准备，即修改 standby 的主配置文件中的三个参数，一定要注意新增了 archive_failover 目录。此时配置的目的是让后面 Switchover 阶段时，原来的 master 可以使用新 master 的日志。

{% highlight text %}
wal_level = archive
archive_mode = on
archive_command = 'cp %p /tmp/archive_failover/%f'
{% endhighlight %}

结下来通过如下方式提升 standby 为 master 。

{% highlight text %}
----- 1. 创建上述指定的trigger文件，可以为空或者"smart"
$ > /tmp/pgsql.trigger.5433

----- 2. 发送promote命令，通产需要十几秒钟才能切换成master
$ pg_ctl promote -D /var/lib/pgsql/standby
{% endhighlight %}

然后重新启动新的 master 库，让 postgresql.conf 生效。

{% highlight text %}
$ pg_ctl restart -D /var/lib/pgsql/standby
{% endhighlight %}

最后需要维护原 master 库，把 fault 修正后，进入下一阶段。


### Switchover

这个阶段是将原来的 master 重新启动为 standby 模式。


<!--
把新master做一次基础备份（基础备份data目录）：
[postgres@localhost pgsql]$ /home/postgres/db/standby/pgsql/bin/createdb mydb --port=6432

    mydb=# SELECT pg_start_backup('bak20120220');
    [postgres@localhost pgsql]$ cd /home/postgres/db/standby/pgsql/
    [postgres@localhost pgsql]$ tar czvf /home/postgres/base/base_data_switchover.tar.gz data/
    mydb=# SELECT pg_stop_backup();
    mydb=# select pg_switch_xlog();

然后拷贝到老master那里：

    [postgres@localhost pgsql]$ cd /home/postgres/db/master/pgsql/
    [postgres@localhost pgsql]$ mv data data_bk
    [postgres@localhost pgsql]$ tar -xzvf /home/postgres/base/base_data_switchover.tar.gz
    [postgres@localhost pgsql]$ rm -r data/pg_xlog/
    [postgres@localhost pgsql]$ mkdir -p data/pg_xlog/archive_status
    [postgres@localhost pgsql]$ rm data/postmaster.pid
    [postgres@localhost pgsql]$ cp /home/postgres/db/master/pgsql/data_bk/postgresql.conf /home/postgres/db/master/pgsql/data/postgresql.conf

然后把postgresql.conf下面的几行都注释掉：

    #wal_level = archive
    #archive_mode = on
    #archive_command = 'cp %p /home/postgres/archive_failover/%f'

并为原有的master新建recovery.conf文件，内容和原来的standby的recovery.conf文件类似（只是使用不同的/home/postgres/archive_failover/目录,trigger也变为 /home/postgres/trigger/pgsql.trigger.5432）：

    standby_mode = on
    restore_command = '/home/postgres/db/master/pgsql/bin/pg_standby -d -s 2 -t /home/postgres/trigger/pgsql.trigger.5432 /home/postgres/archive_failover %f %p %r'
    recovery_end_command = 'rm -f /home/postgres/trigger/pgsql.trigger.5432'

然后启动原master，此时以standby模式运行：
[postgres@localhost develop]$ /home/postgres/db/master/pgsql/bin/postmaster -D /home/postgres/db/master/pgsql/data
查看是否一切正常。即你在新的master里做DML如插入新数据操作时，在新的standby里是否看到在用日志恢复。

（D）如果有必要，我们再来一次主库和备库角色切换的操作（推荐，但此处不再演示）。即关闭新主库，激活老主库。

写到这里你可能还有一些疑问，应用服务器有什么机制可以探测master down掉？有没自动化工具来对Failover和Switchover阶段自动执行？PostgreSQL9.1的25.3. Failover中只是简单的说明了一下，本身不是很详细，本文不再讨论，有兴趣的朋友可以专门写写。
至此完毕。

-->


## 基于流复制的方法


## 基于同步复制的方法


PostgreSQL standby 可以通过两种方法来激活成为主库：

trigger file，配置在recovery.conf中。
pg_ctl promote发送SIGUSR1信号给postmaster进程。

同时，PostgreSQL支持快速激活（fast promote）和非快速激活(fallback promote)：

fast promote 开启数据库读写前，不需要做检查点。而是推到开启读写之后执行一个CHECKPOINT_FORCE检查点。

pg_ctl promote发送SIGUSR1信号给postmaster进程。



# Hot-Standby





# 何时做归档

{% highlight text %}
wal_level = archive
archive_mode = on
archive_command = 'cp %p /tmp/archive/%f'
{% endhighlight %}

归档的隐含前提是，WAL 中仍有未归档的日志，通常有三种方式：

1. 执行 SELECT pg_switch_xlog() 。

2. 日志写满 16M，可以在编译的时候修改。

3. 超过了 archive_command 设置的时间。

# 流复制

PG 9.0 引入了主备流复制机制，备库不断的从主库同步相应的数据，并在备库 apply 每个 WAL record，这里的流复制每次传输单位是WAL日志的record。而PostgreSQL9.0之前提供的方法是主库写完一个WAL日志文件后，才把WAL日志文件传送到备库，这样的方式导致主备延迟特别大。同时PostgreSQL9.0之后提供了Hot Standby，备库在应用WAL record的同时也能够提供只读服务，大大提升了用户体验。

## 主备总体结构

PG 主备流复制的核心部分由 walsender、walreceiver 和 startup 三个进程组成。

walsender 进程在主库中，用来发送 WAL 日志记录的，执行顺序如下。

{% highlight text %}
PostgresMain()
 |-exec_replication_command()
   |-StartReplication()
     |-WalSndLoop()
       |-XLogSendPhysical()
{% endhighlight %}

walreceiver 进程是用来接收 WAL 日志记录的，执行顺序如下。

{% highlight text %}
sigusr1_handler()
 |-StartWalReceiver()
   |-AuxiliaryProcessMain()
     |-WalReceiverMain()
       |-walrcv_receive()
{% endhighlight %}

startup进程是用来apply日志的，执行顺序如下：

{% highlight text %}
PostmasterMain()
 |-StartupDataBase()
   |-AuxiliaryProcessMain()
     |-StartupProcessMain()
       |-StartupXLOG()
{% endhighlight %}





![streaming replication architecture]({{ site.url }}/images/databases/postgresql/streaming-replication-architecture.png){: .pull-center width="800"}


walsender 和 walreceiver 交互主要分为以下几个步骤。

{% highlight text %}

                                WalReceiverMain()
                                 |-walrcv_connect()
                                 |                             for()
                                 |-walrcv_identify_system()            # 执行
                                 |-WalRcvFetchTimeLineHistoryFiles()

XLogPageRead()
 |-WaitForWALToBecomeAvailable()
   |-RequestXLogStreaming()         # 启动walreceiver进程
{% endhighlight %}


walreceiver启动后通过recovery.conf文件中的primary_conninfo参数信息连向主库，主库通过连接参数replication=true启动walsender进程；

walreceiver执行identify_system命令，获取主库systemid/timeline/xlogpos等信息，执行TIMELINE_HISTORY命令拉取history文件；

执行wal_startstreaming开始启动流复制，通过walrcv_receive获取WAL日志，期间也会回应主库发过来的心跳信息(接收位点、flush位点、apply位点)，向主库发送feedback信息(最老的事务id)，避免vacuum删掉备库正在使用的记录；

执行walrcv_endstreaming结束流复制，等待startup进程更新receiveStart和receiveStartTLI，一旦更新，进入步骤2。

















startup进程进入standby模式和apply日志主要过程：

读取pg_control文件，找到redo位点;读取recovery.conf，如果配置standby_mode=on则进入standby模式。

如果是Hot Standby需要初始化clog、subtrans、事务环境等。初始化redo资源管理器，比如Heap、Heap2、Database、XLOG等。
读取WAL record，如果record不存在需要调用XLogPageRead->WaitForWALToBecomeAvailable->RequestXLogStreaming唤醒walreceiver从walsender获取WAL record。

对读取的WAL record进行redo，通过record->xl_rmid信息，调用相应的redo资源管理器进行redo操作。比如heap_redo的XLOG_HEAP_INSERT操作，就是通过record的信息在buffer page中增加一个record：









# pg_basebackup

通常在搭建备库时，需要执行如下的步骤：

1. select pg_start_backup();
2. 复制 $PGDATA
3. select pg_end_backup();

如果利用 pg_basebackup，可以直接一步搞定。

# pg_restore


<!--
SimianArmy+Chaos Monkey
What if I were a cloud?
asciimatics
man 7 signal


建议重新再看一遍
http://blog.sciencenet.cn/home.php?mod=space&uid=419883&do=blog&quickforward=1&id=539178
-->


# 参考
[gearman 搭建主备的测试脚本](/reference/databases/postgresql/gearman)
{% highlight text %}
{% endhighlight %}
