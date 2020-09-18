---
title: MySQL Performance Schema
layout: post
comments: true
language: chinese
category: [mysql,database]
---

这一存储引擎应该是在 MySQL 5.5 引入的，主要用于收集数据库服务器性能参数，提供等待事件的详细信息，包括锁、互斥变量、文件信息等；其中的表是只读的，用户不能创建或者修改。

在此简单介绍下。

<!-- more -->

## 简介

这一存储引擎应该是在 MySQL 5.5 引入的，主要用于收集数据库服务器性能参数，提供等待事件的详细信息，包括锁、互斥变量、文件信息等；其中的表是只读的，用户不能创建或者修改。

另外，需要注意的是，开启 PS 会有一定的性能损耗，不同的版本号，不同的分支可能略有不同，如果比较关注性能，应该以实测为准。

在该库中，数据表分为如下的几类：

1. setup table：设置表，配置监控选项；
2. current events table：记录当前那些 thread 正在发生什么事情；
1. history table  发生的各种事件的历史记录表；
1. summary table  对各种事件的统计表；
1. 杂项表，乱七八糟表。

<!--
    Current events
    event histories
    summaries
    object instances
    setup (configuration)
-->

很多资料也可以参考官方网站的介绍 [MySQL Performance Schema](http://dev.mysql.com/doc/refman/en/performance-schema.html) 。

### 开启

如果使用 Performance Schema，需要在编译时进行配置，可以通过如下命令查看是否支持。

{% highlight text %}
$ mysqld --verbose --help | grep performance_schema
  ... ...
  --performance-schema
                      Enable the performance schema.
  --performance-schema-accounts-size=#
                      Maximum number of instrumented user@host accounts. Use 0
                      to disable, -1 for automated sizing.
  ... ...
{% endhighlight %}

在 5.5.6 之前，默认是关闭的，之后默认是打开的，在配置文件中通过如下方式进行设置，注意，这个参数是静态参数，只能在 my.cnf 设置，不能动态修改。

{% highlight text %}
[mysqld]
performance_schema=ON
{% endhighlight %}

设置完重启后，可以查看相应变量，判断是否已经启动；若返回值为 ON，则说明性能数据库正常开启状态；如果是 OFF 可以查看日志为什么启动失败。

{% highlight text %}
mysql> SHOW VARIABLES LIKE 'performance_schema';
{% endhighlight %}

Performance Schema 通过存储引擎插件实现，也就意味着可以通过如下命令查看。

{% highlight text %}
mysql> SELECT * FROM INFORMATION_SCHEMA.ENGINES WHERE ENGINE='PERFORMANCE_SCHEMA'\G
*************************** 1. row ***************************
      ENGINE: PERFORMANCE_SCHEMA
     SUPPORT: YES
     COMMENT: Performance Schema
TRANSACTIONS: NO
          XA: NO
  SAVEPOINTS: NO

mysql> SHOW ENGINES\G
... ...
      Engine: PERFORMANCE_SCHEMA
     Support: YES
     Comment: Performance Schema
Transactions: NO
          XA: NO
  Savepoints: NO
... ...
{% endhighlight %}

对应的表存储在 performance_schema 库中，所包含的表可以通过如下两种方式查看，对应的表会随着新监控的添加慢慢增长。

{% highlight text %}
mysql> SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = 'performance_schema';
mysql> SHOW TABLES FROM performance_schema;
{% endhighlight %}

默认不是所有的 instrument 和 consumer 都会被 enable ，这也就意味着一开始不会收集所有的事件，可以通过执行如下 SQL 。

{% highlight text %}
mysql> UPDATE setup_instruments SET ENABLED = 'YES', TIMED = 'YES';
Query OK, 338 rows affected (0.12 sec)
mysql> UPDATE setup_consumers SET ENABLED = 'YES';
Query OK, 8 rows affected (0.00 sec)
{% endhighlight %}

如果想查看某个时刻的等待事件，可以查询 events_waits_current 表，它记录了每个 thread 最近的监控信息。










## setup table

通过如下方式可以查看所有的 setup 表。

{% highlight text %}
mysql> SELECT table_name FROM information_schema.tables WHERE \
       table_schema = 'performance_schema' AND table_name LIKE 'setup%';
+-------------------+
| table_name        |
+-------------------+
| setup_actors      |       配置用户纬度的监控，默认监控所有用户
| setup_consumers   |       消费者类型，即收集的事件写入到哪些统计表中
| setup_instruments |       该数据库的表名以及是否开启监控
| setup_objects     |       监控对象
| setup_timers      |       监控选项已经采样频率的时间间隔
+-------------------+
5 rows in set (0.00 sec)
{% endhighlight %}

### setup_actors

配置用户纬度的监控，默认监控所有用户。

{% highlight text %}
mysql> SELECT * FROM performance_schema.setup_actors;
+------+------+------+---------+---------+
| HOST | USER | ROLE | ENABLED | HISTORY |
+------+------+------+---------+---------+
| %    | %    | %    | YES     | YES     |
+------+------+------+---------+---------+
1 row in set (0.00 sec)
{% endhighlight %}


### setup_consumers

对于 setup_consumers 表，需要注意的是，如果要采集数据，需要确保其 ENABLED 列为 YES 才会收集，可以直接使用 UPDATE SQL 进行更新，更新完后立即生效。

{% highlight text %}
mysql> SELECT * FROM performance_schema.setup_consumers ;
+----------------------------------+---------+
| NAME                             | ENABLED |
+----------------------------------+---------+
| events_stages_current            | NO      |
| events_stages_history            | NO      |
| events_stages_history_long       | NO      |
| events_statements_current        | YES     |
| events_statements_history        | YES     |
| events_statements_history_long   | NO      |
| events_transactions_current      | NO      |
| events_transactions_history      | NO      |
| events_transactions_history_long | NO      |
| events_waits_current             | NO      |
| events_waits_history             | NO      |
| events_waits_history_long        | NO      |
| global_instrumentation           | YES     |
| thread_instrumentation           | YES     |
| statements_digest                | YES     |
+----------------------------------+---------+
15 rows in set (0.00 sec)
{% endhighlight %}


<!--
这个表的各个记录还存在层级关系，只有当上级的配置启用时才会考虑下级的配置。
层级关系为：

global_instrumentation
|----thread_instrumentation
|         |----events_waits_current
|         |           |-events_waits_history
|         |           |-events_waits_history_long
|         |----events_stages_current
|         |           |-events_stages_history
|         |           |-events_stages_history_long
|         |----events_statements_current
|                     |-events_statements_history
|                     |-events_statements_history_long
|-----statements_digest
当global_instrumentation启用时，"thread_instrumentation"和"statements_digest"的配置才有可能生效。其他的配置类推。
只有当"setup_instruments"，"setup_objects",”setup_consumers“和"threads"都某一项测量指标都启用时才能收集到它的信息。
-->

上述通过 UPDATE 配置后，服务器重启又会变回默认值，要永久生效需要在配置文件里添加。

{% highlight text %}
[mysqld]
performance_schema_consumer_events_waits_current      = ON
performance_schema_consumer_events_stages_current     = ON
performance_schema_consumer_events_statements_current = ON
performance_schema_consumer_events_waits_history      = ON
performance_schema_consumer_events_stages_history     = ON
performance_schema_consumer_events_statements_history = ON
{% endhighlight %}

也即在这些表的前面加上performance_schema_consumer_xxx。

其中 history 和 history_long 保存的是 current 表的历史记录条数，长度可以通过变量设置。

{% highlight text %}
mysql> SHOW VARIABLES LIKE 'performance_schema%history%size';
+----------------------------------------------------------+-------+
| Variable_name                                            | Value |
+----------------------------------------------------------+-------+
| performance_schema_events_stages_history_long_size       | 1000  |
| performance_schema_events_stages_history_size            | 10    |
| performance_schema_events_statements_history_long_size   | 1000  |
| performance_schema_events_statements_history_size        | 10    |
| performance_schema_events_transactions_history_long_size | 1000  |
| performance_schema_events_transactions_history_size      | 10    |
| performance_schema_events_waits_history_long_size        | 1000  |
| performance_schema_events_waits_history_size             | 10    |
+----------------------------------------------------------+-------+
8 rows in set (0.00 sec)
{% endhighlight %}

### setup_instruments

配置具体的 instrument，主要包含如下的几大类：idle、stage/xxx、statement/xxx、wait/xxx、memory/xxx、transaction 。

{% highlight text %}
mysql> SELECT name, count(1) FROM performance_schema.setup_instruments GROUP BY left(name,5);
+-------------------------------------------+----------+
| name                                      | count(1) |
+-------------------------------------------+----------+
| idle                                      | 1        |
| memory/performance_schema/mutex_instances | 375      |
| stage/sql/After create                    | 129      |
| statement/sql/select                      | 193      |
| transaction                               | 1        |
| wait/synch/mutex/sql/TC_LOG_MMAP::LOCK_tc | 314      |
+-------------------------------------------+----------+
6 rows in set (0.01 sec)
{% endhighlight %}

idle 表示 socket 空闲的时间；stage 类表示语句的每个执行阶段的统计；statement 类统计语句维度的信息；wait 类统计各种等待事件，比如 IO、mutux、spin_lock、condition 等。


### setup_objects

默认对 MySQL、performance_schema 和 information_schema 中的表都不监控，而其它 DB 的所有表都监控。



{% highlight text %}
mysql> SELECT * FROM performance_schema.setup_objects;
+-------------+--------------------+-------------+---------+-------+
| OBJECT_TYPE | OBJECT_SCHEMA      | OBJECT_NAME | ENABLED | TIMED |
+-------------+--------------------+-------------+---------+-------+
| EVENT       | mysql              | %           | NO      | NO    |
| EVENT       | performance_schema | %           | NO      | NO    |
| EVENT       | information_schema | %           | NO      | NO    |
| EVENT       | %                  | %           | YES     | YES   |
| FUNCTION    | mysql              | %           | NO      | NO    |
| FUNCTION    | performance_schema | %           | NO      | NO    |
| FUNCTION    | information_schema | %           | NO      | NO    |
| FUNCTION    | %                  | %           | YES     | YES   |
| PROCEDURE   | mysql              | %           | NO      | NO    |
| PROCEDURE   | performance_schema | %           | NO      | NO    |
| PROCEDURE   | information_schema | %           | NO      | NO    |
| PROCEDURE   | %                  | %           | YES     | YES   |
| TABLE       | mysql              | %           | NO      | NO    |
| TABLE       | performance_schema | %           | NO      | NO    |
| TABLE       | information_schema | %           | NO      | NO    |
| TABLE       | %                  | %           | YES     | YES   |
| TRIGGER     | mysql              | %           | NO      | NO    |
| TRIGGER     | performance_schema | %           | NO      | NO    |
| TRIGGER     | information_schema | %           | NO      | NO    |
| TRIGGER     | %                  | %           | YES     | YES   |
+-------------+--------------------+-------------+---------+-------+
20 rows in set (0.00 sec)
{% endhighlight %}


### setup_timers

配置每种类型指令的统计时间单位。MICROSECOND表示统计单位是微妙，CYCLE表示统计单位是时钟周期，时间度量与CPU的主频有关，NANOSECOND表示统计单位是纳秒。但无论采用哪种度量单位，最终统计表中统计的时间都会装换到皮秒。（1秒＝1000000000000皮秒）

{% highlight text %}
mysql> SELECT * FROM performance_schema.setup_timers;
+-------------+-------------+
| NAME        | TIMER_NAME  |
+-------------+-------------+
| idle        | MICROSECOND |
| wait        | CYCLE       |
| stage       | NANOSECOND  |
| statement   | NANOSECOND  |
| transaction | NANOSECOND  |
+-------------+-------------+
5 rows in set (0.00 sec)
{% endhighlight %}

## instance

### cond_instances

条件等待对象实例，表中记录了系统中使用的条件变量的对象，OBJECT_INSTANCE_BEGIN 为对象的内存地址。

### file_instances

记录了系统中打开了文件的对象，包括 ibdata、redo、binlog、用户的表文件等，open_count 显示当前文件打开的数目，如果没有打开过，不会出现在表中。

{% highlight text %}
mysql> SELECT * FROM performance_schema.file_instances;
+-------------------------------------------+--------------------------------------+------------+
| FILE_NAME                                 | EVENT_NAME                           | OPEN_COUNT |
+-------------------------------------------+--------------------------------------+------------+
| /opt/mysql-5.7/share/english/errmsg.sys   | wait/io/file/sql/ERRMSG              | 0          |
| /opt/mysql-5.7/share/charsets/Index.xml   | wait/io/file/mysys/charset           | 0          |
| /tmp/mysql-master/ibdata1                 | wait/io/file/innodb/innodb_data_file | 3          |
| /tmp/mysql-master/ib_logfile0             | wait/io/file/innodb/innodb_log_file  | 2          |
| /tmp/mysql-master/ib_logfile1             | wait/io/file/innodb/innodb_log_file  | 2          |
| /tmp/mysql-master/mysql/engine_cost.ibd   | wait/io/file/innodb/innodb_data_file | 3          |
+-------------------------------------------+--------------------------------------+------------+
269 rows in set (0.01 sec)
{% endhighlight %}

<!--
### mutex_instances

记录了系统中使用互斥量对象的所有记录，其中name为：wait/synch/mutex/*。LOCKED_BY_THREAD_ID显示哪个线程正持有mutex，若没有线程持有，则为NULL。

### rwlock_instances

： 读写锁同步对象实例

表中记录了系统中使用读写锁对象的所有记录，其中name为 wait/synch/rwlock/*。WRITE_LOCKED_BY_THREAD_ID为正在持有该对象的thread_id，若没有线程持有，则为NULL。READ_LOCKED_BY_COUNT为记录了同时有多少个读者持有读锁。（通过 events_waits_current 表可以知道，哪个线程在等待锁；通过rwlock_instances知道哪个线程持有锁。rwlock_instances的缺陷是，只能记录持有写锁的线程，对于读锁则无能为力）。
-->

### socket_instances

记录活跃会话对象实例，表中记录了 thread_id、socket_id、ip 和 port，其它表可以通过 thread_id 与 socket_instance 进行关联，获取 IP-PORT 信息，能够与应用对接起来。

{% highlight text %}
mysql> select * from socket_instances;
+----------------------------------------+-----------+-----------+------------------+-------+--------+
| EVENT_NAME                             | THREAD_ID | SOCKET_ID | IP               | PORT  | STATE  |
+----------------------------------------+-----------+-----------+------------------+-------+--------+
| wait/io/socket/sql/server_tcpip_socket | 1         | 33        | ::               | 3307  | ACTIVE |
| wait/io/socket/sql/server_unix_socket  | 1         | 34        |                  | 0     | ACTIVE |
| wait/io/socket/sql/client_connection   | 31        | 41        | ::ffff:127.0.0.1 | 54834 | ACTIVE |
| wait/io/socket/sql/client_connection   | 32        | 45        |                  | 0     | ACTIVE |
+----------------------------------------+-----------+-----------+------------------+-------+--------+
4 rows in set (0.00 sec)
{% endhighlight %}

event_name 主要包含 3 类：

* wait/io/socket/sql/server_unix_socket，服务端 unix 监听 socket；
* wait/io/socket/sql/server_tcpip_socket，服务端 tcp 监听 socket；
* wait/io/socket/sql/client_connection，客户端 socket 。

<!--
三：Wait表
1，events_waits_current：记录了当前线程等待的事件
2，events_waits_history：记录了每个线程最近等待的10个事件
3，events_waits_history_long：记录了最近所有线程产生的10000个事件

四：Stage 表
1，events_stages_current：记录了当前线程所处的执行阶段
2，events_stages_history：记录了当前线程所处的执行阶段10条历史记录
3，events_stages_history_long：记录了当前线程所处的执行阶段10000条历史记录

五：Statement 表
1，events_statements_current：通过 thread_id+event_id可以唯一确定一条记录。Statments表只记录最顶层的请求，SQL语句或是COMMAND，每条语句一行。event_name形式为statement/sql/*，或statement/com/*
2，events_statements_history
3，events_statements_history_long

六：Connection 表
1，users：记录用户连接数信息
2，hosts：记录了主机连接数信息
3，accounts：记录了用户主机连接数信息

七：Summary 表： Summary表聚集了各个维度的统计信息包括表维度，索引维度，会话维度，语句维度和锁维度的统计信息
-->










## 其它

### threads

监视服务端的当前运行的线程。




{% highlight text %}
----- 哪个SQL执行最多
mysql> SELECT schema_name,digest_text,count_star,sum_rows_sent,sum_rows_examined
          FROM events_statements_summary_by_digest ORDER BY count_star desc\G

----- 哪个SQL平均响应时间最多
mysql> SELECT schema_name,digest_text,count_star,avg_timer_wait,sum_rows_sent,sum_rows_examined
          FROM events_statements_summary_by_digest ORDER BY AVG_TIMER_WAIT desc LIMIT 1\G

{% endhighlight %}


## 源码解析

该引擎的源码保存在 storage/perfschema 目录下，

{% highlight text %}
mysqld_main()
 |-pre_initialize_performance_schema()      最先调用的函数
 |-initialize_performance_schema()
 |-initialize_performance_schema_acl()
 |-check_performance_schema()
{% endhighlight %}




<!--
status；
show status;
show engines;
show plugins;
show engine innodb status;
show master status;
show slave status;
show procedure status;
show table status;
show variables;

https://yq.aliyun.com/articles/59262
http://www.cnblogs.com/cchust/p/5061131.html
-->

{% highlight text %}
{% endhighlight %}
