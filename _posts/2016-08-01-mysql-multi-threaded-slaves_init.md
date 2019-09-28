---
title: MySQL 并行复制
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,replication,parallel,binlog,并行,复制
description: 众所周知，MySQL 复制延迟是一直被诟病的问题之一，然而在 MySQL 5.7 版本已经支持 "真正" 的并行复制功能，官方称为 Enhanced Multi-Threaded Slave, MTS，因此复制延迟问题已经得到了极大的改进。结下来，详细看看 MySQL 的实现。
---

众所周知，MySQL 复制延迟是一直被诟病的问题之一，然而在 MySQL 5.7 版本已经支持 "真正" 的并行复制功能，官方称为 Enhanced Multi-Threaded Slave, MTS，因此复制延迟问题已经得到了极大的改进。

结下来，详细看看 MySQL 的实现。

<!-- more -->

## 简介

在 MySQL 5.6 之前，备库上只有两个线程 (IO线程和SQL线程)；IO 线程负责接收二进制日志 (更准确的说是二进制日志的 event)，SQL 线程进行回放二进制日志。

在 MySQL 5.6 版本中，也支持所谓的并行复制，但是其并行只是基于库 (或者schema) 的；所以，只有当用户的 MySQL 数据库实例中存在多个库时，才会对于从机复制的速度有所帮助。

5.6 开启并行复制功能后，SQL 线程就变为了 coordinator 线程，该线程主要负责如下功能：

* 若判断可以并行执行，那么选择 worker 线程执行事务的二进制日志；
* 若判断不可以并行执行 (如 DDL、事务跨 schema 操作)，则等待所有 worker 线程执行完成之后，再执行当前的日志；

这意味着 coordinator 线程并不是仅将日志发送给 worker 线程，自己也可以回放日志，但是所有可以并行的操作交付由 worker 线程完成。

<!--
上述机制实现了基于schema的并行复制存在两个问题，首先是crash safe功能不好做，因为可能之后执行的事务由于并行复制的关系先完成执行，那么当发生crash的时候，这部分的处理逻辑是比较复杂的。从代码上看，5.6这里引入了Low-Water-Mark标记来解决该问题，从设计上看（WL#5569），其是希望借助于日志的幂等性来解决该问题，不过5.6的二进制日志回放还不能实现幂等性。另一个最为关键的问题是这样设计的并行复制效果并不高，如果用户实例仅有一个库，那么就无法实现并行回放，甚至性能会比原来的单线程更差。而单库多表是比多库多表更为常见的一种情形。

诚然，MySQL 5.6版本也支持所谓的并行复制，但是其并行只是基于schema的，也就是基于库的。如果用户的MySQL数据库实例中存在多个schema，对于从机复制的速度的确可以有比较大的帮助。MySQL 5.6并行复制的架构如下所示：
-->

### 5.7并行复制

MySQL 5.7 才可称为真正的并行复制：从库的回放与主库是一致的，即主库是怎么并行执行的备库上就怎样进行并行回放，不再有库的并行复制限制。

其并行复制的思路简单易懂，简言之：一个组提交的事务都是可以并行回放，因为这些事务都已进入到事务的 prepare 阶段，则说明事务之间没有任何冲突，否则就不可能提交。

> To provide parallel execution of transactions in the same schema, MariaDB 10.0 and MySQL 5.7 take advantage of the binary log group commit optimization.
>
> 主库依据 Group Commit 的并行性，在二进制日志中进行标记，备库利用主库提供的信息并行执行事务。

为了兼容 5.6 基于库的并行复制，5.7 引入了新变量 slave-parallel-type：

* DATABASE：默认值，基于库的并行复制方式；
* LOGICAL_CLOCK：基于组提交的并行复制方式。

其并行复制可以通过如下参数配置：

{% highlight text %}
slave_parallel_type=logical_clock
    默认database，也就是使用DB并行方式；或者logical_clock使用基于组提交的并行模式；
slave_parallel_workers=16 设置worker线程数
binlog_group_commit_sync_delay 和 binlog_group_commit_sync_no_delay_count 在master延时事务提交，增加group commit事务数
{% endhighlight %}

接下来看看，如何判断事务是否在一组中，因为原版的 MySQL 并没有提供这样的信息。

### 并行复制原理

在 MySQL 5.7 版本中，其设计方式是将组提交的信息存放在 GTID 中；对于没有开启 GTID 功能的用户，5.7 又引入了称之为 Anonymous_Gtid 的二进制日志 event 类型。

{% highlight text %}
----- 查看是否存在Event_Type为Anonymous_Gtid的事件
mysql> SHOW BINLOG EVENTS IN 'mysql-bin.000001';
{% endhighlight %}

也就是说，即使 5.7 不开启 GTID，每个事务开始前也是会存在一个 Anonymous_Gtid，而这 GTID 中就存在着组提交的信息。

在上述命令中，通过 ```SHOW BINLOG EVENTS``` 查看时，并没有发现有关组提交的任何信息；但实际上可以通过 mysqlbinlog 工具查看组提交的内部信息。

{% highlight text %}
$ mysqlbinlog mysql-bin.0000006 | grep last_committed
#160720 22:13:41 server id 1 end_log_pos 259 CRC32 0x4ead9ad6 GTID last_committed=0 sequence_number=1
#160720 22:13:41 server id 1 end_log_pos 1483 CRC32 0xdf94bc85 GTID last_committed=0 sequence_number=2
#160720 22:13:41 server id 1 end_log_pos 2708 CRC32 0x0914697b GTID last_committed=0 sequence_number=3
#160720 22:13:41 server id 1 end_log_pos 3934 CRC32 0xd9cb4a43 GTID last_committed=0 sequence_number=4
#160720 22:13:41 server id 1 end_log_pos 5159 CRC32 0x06a6f531 GTID last_committed=0 sequence_number=5
#160720 22:13:41 server id 1 end_log_pos 6386 CRC32 0xd6cae930 GTID last_committed=0 sequence_number=6
#160720 22:13:41 server id 1 end_log_pos 7610 CRC32 0xa1ea531c GTID last_committed=6 sequence_number=7
#160720 22:13:41 server id 1 end_log_pos 8834 CRC32 0x96864e6b GTID last_committed=6 sequence_number=8
#160720 22:13:41 server id 1 end_log_pos 10057 CRC32 0x2de1ae55 GTID last_committed=6 sequence_number=9
#160720 22:13:41 server id 1 end_log_pos 11280 CRC32 0x5eb13091 GTID last_committed=6 sequence_number=10
#160720 22:13:41 server id 1 end_log_pos 12504 CRC32 0x16721011 GTID last_committed=6 sequence_number=11
#160720 22:13:41 server id 1 end_log_pos 13727 CRC32 0xe2210ab6 GTID last_committed=6 sequence_number=12
{% endhighlight %}

与之前的 binlog 相比，日志中多了 last_committed 和 sequence_number，其中 last_committed 表示事务提交的时候，上次事务提交的编号，如果事务具有相同的 last_committed，表示这些事务都在一组内，可以进行并行的回放。

上述的 last_committed+sequence_number 就是所谓的 LOGICAL_CLOCK。



## 源码解析






{% highlight cpp %}
class MYSQL_BIN_LOG: public TC_LOG
{
... ...
public:
  /* Committed transactions timestamp */
   Logical_clock max_committed_transaction;
  /* "Prepared" transactions timestamp */
   Logical_clock transaction_counter;
... ...
}
{% endhighlight %}

可以看到在类 MYSQL_BIN_LOG 中定义了两个 Logical_clock 变量：

* max_committed_transaction<br>上次组提交时的 logical_clock，也就是上述 binlog 中的 last_committed；
* transaction_counter<br>当前组提交中各事务的 logcial_clock，也就是上述的 sequence_number 。

<!--
接下来看看源码中对于 LOGICAL_CLOCK 的定义：

class Logical_clock
{
  private:
  int64 state;
  /*
  Offset is subtracted from the actual "absolute time" value at
  logging a replication event. That is the event holds logical
  timestamps in the "relative" format. They are meaningful only in
  the context of the current binlog.
  The member is updated (incremented) per binary log rotation.
  */
  int64 offset;
  ......

state是一个自增的值，offset在每次二进制日志发生rotate时更新，记录发生rotate时的state值。其实state和offset记录的是全局的计数值，而存在二进制日志中的仅是当前文件的相对值。
-->





## 其它


### 调优

master_info_repository

开启MTS功能后，务必将参数master_info_repostitory设置为TABLE，这样性能可以有50%~80%的提升。这是因为并行复制开启后对于元master.info这个文件的更新将会大幅提升，资源的竞争也会变大。在之前InnoSQL的版本中，添加了参数来控制刷新master.info这个文件的频率，甚至可以不刷新这个文件。因为刷新这个文件是没有必要的，即根据master-info.log这个文件恢复本身就是不可靠的。在MySQL 5.7中，Inside君推荐将master_info_repository设置为TABLE，来减小这部分的开销。

slave_parallel_workers

若将slave_parallel_workers设置为0，则MySQL 5.7退化为原单线程复制，但将slave_parallel_workers设置为1，则SQL线程功能转化为coordinator线程，但是只有1个worker线程进行回放，也是单线程复制。然而，这两种性能却又有一些的区别，因为多了一次coordinator线程的转发，因此slave_parallel_workers=1的性能反而比0还要差，在Inside君的测试下还有20%左右的性能下降，如下图所示：



这里其中引入了另一个问题，如果主机上的负载不大，那么组提交的效率就不高，很有可能发生每组提交的事务数量仅有1个，那么在从机的回放时，虽然开启了并行复制，但会出现性能反而比原先的单线程还要差的现象，即延迟反而增大了。聪明的小伙伴们，有想过对这个进行优化吗？
Enhanced Multi-Threaded Slave配置

### 配置与监控

只需要在备库添加如下配置。

{% highlight text %}
slave-parallel-type       = LOGICAL_CLOCK
slave-parallel-workers    = 16
master_info_repository    = TABLE
relay_log_info_repository = TABLE
relay_log_recovery        = ON
{% endhighlight %}

并行复制监控仍然可以通过 ```SHOW SLAVE STATUS\G``` 查看，但是 MySQL 5.7 在 performance_schema 架构下多了以下这些元数据表，用户可以更细力度的进行监控。

{% highlight text %}
mysql> SHOW TABLES LIKE 'replication%';
+---------------------------------------------+
| Tables_in_performance_schema (replication%) |
+---------------------------------------------+
| replication_applier_configuration           |
| replication_applier_status                  |
| replication_applier_status_by_coordinator   |
| replication_applier_status_by_worker        |
| replication_connection_configuration        |
| replication_connection_status               |
| replication_group_member_stats              |
| replication_group_members                   |
+---------------------------------------------+
8 rows in set (0.00 sec)
{% endhighlight %}













<!--
mariadb参数
slave_parallel_threads=16 也支持多源复制
slave_parallel_mode=conservative 10.1.3开始支持optimistic模式，通过启发性方式减少冲突，如果发生冲突，将事务进行回滚 ；Conservative默认值，使用group commit发现潜在的并行事件，在一个group commit中的事务写到binlog时拥有相同的commit id(cid)；minimal仅仅commit阶段是并行，其他事务应用发生是串行的，同时也关闭out-of-order(使用不同domain id)并行复制
binlog_commit_wait_count 和 binlog_commit_wait_usec 调整master进行group commit的事务数
slave_parallel_max_queued 限制每个线程的队列事件数，提高worker线程处理能力
slave_domain_parallel_threads worker线程由所有多源master连接共享，建议大于slave_parallel_threads值

查看状态

show processlist 检查worker线程的状态
mariadb的状态变量BINLOG_COMMITS 和 BINLOG_GROUP_COMMITS
-->







<!--
http://mysqlhighavailability.com/stop-slave-improvements-for-multi-threaded-slaves/
http://blog.csdn.net/mawming/article/details/52055008
-->

{% highlight text %}
{% endhighlight %}
