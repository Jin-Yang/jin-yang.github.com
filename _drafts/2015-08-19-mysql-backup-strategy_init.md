---
title: MySQL 备份策略
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,备份,mysqldump,mysqlbinlog,xtrabackup
description: 为了保证数据安全，都会对硬件做高可用，防止出现单点故障，但是无论如何都无法取代备份，尤其对于数据库中所保存的数据而言。在此，介绍一下 MySQL 中常用的备份方法。
---


<!-- more -->


## 准备工作

在刚接触一个环境时，首先需要确认一下几点的数据：

1. 确定需要备份数据的大小，包括使用的存储引擎，例如 InnoDB、MyISAM、ARCHIVE 等；
2. 如果要获取一致性读，可以使用什么样的锁策略，当然这也根上述的存储引擎相关；
3. 备份时会花费多长时间；
4. 是否有维护时间窗口，通常是没有对外提供 7*24 服务的业务。

#### 确定备份大小

可以通过如下方式查看当前某个库的大小。

{% highlight text %}
mysql> SELECT ROUND(SUM(data_length+index_length)/1024/1024) AS total_mb,
              ROUND(SUM(data_length)/1024/1024) AS data_mb,
              ROUND(SUM(index_length)/1024/1024) AS index_mb
       FROM information_schema.tables
       WHERE  table_schema NOT IN
              ('information_schema', 'performance_schema', 'mysql', 'sys');
{% endhighlight %}

<!--
mysql> SELECT ROUND(SUM(data_length+index_length)/1024) AS total_kb,
              ROUND(SUM(data_length)/1024) AS data_kb,
              ROUND(SUM(index_length)/1024) AS index_kb
       FROM information_schema.tables
       WHERE  table_schema NOT IN
              ('information_schema', 'performance_schema', 'mysql', 'sys');
-->

这里计算的大小会与实际导出的文件有些许区别，不过相差不大。

#### 使用锁策略

锁的策略决定了备份时是否会影响到在线业务的读写，例如对于 mysqldump 命令可以通过 \-\-lock-tables 参数开启表锁，也就是 ```LOCK TABLES``` 命令；显然，这会影响到线上业务，不过对于 MyISAM 存储引擎来说，如果要获取到一致性的读，这是必须的。

对于 mysqldump 命令，同时也提供了一个 \-\-single-transaction 参数在备份时开启一个事务，当然这需要存储引擎支持多版本事务，例如 InnoDB ，当使用该选项时，则会自动关闭 \-\-lock-tables 。

可以通过如下命令查看每个数据库包含的存储引擎。

{% highlight text %}
mysql> SELECT table_schema,engine,count(1) AS tables
       FROM   information_schema.tables
       WHERE  table_schema NOT IN
              ('information_schema', 'performance_schema', 'mysql', 'sys')
       GROUP BY table_schema, engine
       ORDER BY 3 DESC;
{% endhighlight %}

#### 备份时间

这个实际上比较难测量，包括使用的内存、备份的并发等等，所以在此就不过多介绍了。

#### 总结

可以将当前库的大小、存储引擎信息通过如下 SQL 一次查看。

{% highlight text %}
mysql> SELECT table_schema, engine,
              ROUND(SUM(data_length+index_length)/1024/1024) AS total_mb,
              ROUND(SUM(data_length)/1024/1024) AS data_mb,
              ROUND(SUM(index_length)/1024/1024) AS index_mb,
              count(1) AS tables
       FROM information_schema.tables
       WHERE  table_schema NOT IN
              ('information_schema', 'performance_schema', 'mysql', 'sys')
       GROUP BY table_schema, engine
       ORDER BY 3 DESC;
{% endhighlight %}

收集到信息之后，接下来就是要决定如何备份、什么时候执行备份；在备份时需要检查是否备份成功，备份的大小等信息。

{% highlight text %}
----- 执行备份，并计算备份的时间
$ time mysqldump -uroot -p --all-databases > backup.sql
----- 查看备份的返回值，确认返回0，否则备份时有异常
$ echo $?
----- 查看备份的大小
$ ls -lh backup.sql
{% endhighlight %}

当然，备份完成之后，可以通过 rsync 同步一份到其它的服务器上，防止数据损坏。




Point In Time Recovery, PITR











## 参考

[Effective MySQL: Backup and Recovery](http://apprize.info/php/effective/index.html)



{% highlight text %}
{% endhighlight %}
