---
Date: October 19, 2013
title: MySQL MyISAM
layout: post
comments: true
language: chinese
category: [mysql,database]
---



<!-- more -->

* 文件比较小；
* 支持 FULLTEXT、GIS，不过性能不太好；
* 支持复合索引以及自增主健（不建议使用）；
* 表级锁，对性能影响较大；
* 不支持事务；
* 不支持表恢复（通过 REPAIR TABLE）；
* update 效率较低，容易产生碎片。



.MYI 索引
.MYD 数据


--- MyISAM file structure

记录在数据文件中依次存放
   Delete -> Gap
   Gap -> Next Insert, or Append
FIXED vs. DYNAMIC vs. COMPRESSED
   Fragmentation (String extension)

Index: record Number (fixed), byte offset (DYNAMIC)
    Data pointer size variable
    myisam_data_pointer_size=6（256TB）
alter table t min_rows=n max_rows=m avg_row_size=p

--- MyISAM update storm
update t set sth = concat(sth, sth)
split record = seek
move to end of table = all indices updated
either way: fragmentation, seeks


--- MyISAM file structure
update t set count=count+1, and server crash?
some records update, you do not know which.
recover from unchrashed slave, or have invalid data.
also, the table is marked as crashed.

--- Compressed MyISAM
Close all filedhandles: FLUSH TABLES
run myisampack to compress data file: invalid index.
run myisamchk to rebuild index.
data footprint approx. 50% smaller.
   read-only

--- MyISAM locking
table level locks: shared(read), exclusive(write)
many concurrent select statements, serialized write statements.
writes have precedence
running statements have precedence.

select high_priority ...
write (insert update delete, ...)
select ...
low priority (update low_priority ...)

many selects, some slow (4s runtime)
foreach $i (@stmt) { $dbh->do($i) }
    insert blocks subsequent reads.
    insert blocked by runing（slow) read.
one insert every 4s.


--- Explicit locking
lock table t1 read, t2 read, t3 write.
all locks taken at once: avoids deadlocks.
unable to touch unlocked tables during lock: ensures correctness and completeness.
unlock tables (or disconnect)

how does explicit locking affect the myisam interlock scenario?
locking can actually make things fasster.
but will increase latency

--- Tuning MyISAM
MyISAM caches myi data, but no myd data.
system must have sufficient buffer cache: mysqld size must be limited.
key_buffer_size=...
ideally: sum of all myi files.

key_cache_division_limit = 100
the division point between the hot and warm sublists of the key cache buffer list(percent)

alter a server restart, the key cache is cold.
   load index into cache <indexname>, <indexname2>, ...
this preloads the cache w/ linear disk accesses.
much faster than normal warmup.

handling myisam index update storms:
    delayed_key_write = OFF|ON|ALL
    create table t (...) delay_key_write = 1;
status variable key_blocks_not_flushed.
large number of unflushed blocks + crash = monster repair table.

myisam_recover_options = force,backup
    may not be enough in the face of delayed key writes
myisam_repair_threads = 1
    used to be broken with > 1

--- monitoring myisam
table_locks_immediate
    table locks granted without waiting
table_locks_waited
    table locks granted after wait.
1%=dangerous, 3%=deadly

--- myisam maintenance
show table status - data_free
optimize table to reclaim data_free
    this is very slow and x-locks.
analyze tables - optimizer statistics
   also very slow (in myisam)



## 文件

总共有三类文件：.MYD (数据文件，MySQL Data)、.MYI (索引文件，MySQL Index)、.frm (格式文件，Format)；而数据文件的行格式按照不同的类型分为了 fixed、dynamic、packed 三种。

### fixed format

这个时默认格式，当表不含变长字段（varchar/varbinary/blob/text）时使用，每行固定，所以很容易获取行在页上的具体位置，存取效率高，但是占用磁盘空间较大。

* Page Size：MyISAM 没有像其它数据库一样保存数据时采用页存储，这也就意味着，你不会看到行间的填充字符。

*

{% highlight text %}
CREATE TABLE foobar (c1 CHAR(1), c2 CHAR(1), c3 CHAR(1)) ENGINE=MyISAM;

INSERT INTO foobar VALUES ('a', 'b', 'c');
INSERT INTO foobar VALUES ('d', NULL, 'e');
{% endhighlight %}





# 锁机制

MySQL 有三种粒度的锁：表级锁、行级锁和页面锁；其中 InnoDB 支持表锁和行级锁，MyISAM 支持表级锁，表锁分为读锁 (read lock) 和写锁 (write lock)，接下来我们看看 MyISAM 引擎的表锁。

为了测试，首先创建如下的表。

{% highlight text %}
CREATE TABLE foobar (id INT, name CHAR(30)) ENGINE=MyISAM;
INSERT INTO foobar VALUES (1, 'foo'), (2, 'bar');

CREATE TABLE test (id INT, name CHAR(30)) ENGINE=MyISAM;
INSERT INTO test VALUES (1, 'test1'), (2, 'test2');
{% endhighlight %}

## 读锁 (read lock)

当一个会话加读锁后，其它会话是可以继续读取该表的，但所有更新、删除和插入将会阻塞，直到将表解锁。

{% highlight text %}
mysql[s1]> LOCK TABLE foobar READ;                 # s1中对表加读锁
Query OK, 0 rows affected (0.00 sec)

mysql[s2]> SELECT * FROM foobar;                   # s2中可以对表正常读取
+------+------+
| id   | name |
+------+------+
|    1 |  foo |
|    2 |  bar |
+------+------+
2 rows in set (0.00 sec)
mysql[s2]> INSERT INTO foobar VALUES (3, 'test');  # s2中的写入被阻塞

mysql[s1]> UNLOCK TABLES;                          # s1中释放锁，s2写入成功
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

MyISAM 会在执行 SELECT 时会自动给相关表加读锁，在执行 UPDATE、DELETE 和 INSERT 时会自动给相关表加写锁；对于表级读锁有几点需要特别注意的地方 (InnoDB相同)：

### 1. 锁表时要将所有表加锁

锁表时一定要把所有需要访问的表都锁住，因为锁表之后无法访问其他未加锁的表。

{% highlight text %}
mysql> LOCK TABLE foobar READ;
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT * FROM test;
ERROR 1100 (HY000): Table 'test' was not locked with LOCK TABLES
{% endhighlight %}

### 2. 读锁后不能更新

当前会话对表加读锁之后，该会话只能读锁住的表，而无法对其进行 UPDATE、DELETE 和 INSERT 操作。

{% highlight text %}
mysql> LOCK TABLE foobar READ;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO foobar VALUES (4, 'again');
ERROR 1099 (HY000): Table 'foobar' was locked with a READ lock and can't be updated
{% endhighlight %}

### 3. 对表多次加锁

同一个表如果在 SQL 语句里面如果出现了 N 次，那么就要锁定 N 次，否则会出错。

{% highlight text %}
mysql> LOCK TABLE foobar READ;
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT * FROM foobar f1, foobar f2 WHERE f1.id = f2.id;
ERROR 1100 (HY000): Table 'f1' was not locked with LOCK TABLES

mysql> UNLOCK TABLES;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

应该使用如下的方法：

{% highlight text %}
mysql> LOCK TABLE foobar AS f1 READ, foobar AS f2 READ;
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT * FROM foobar f1, foobar f2 WHERE f1.id = f2.id;
+------+------+------+------+
| id   | name | id   | name |
+------+------+------+------+
|    1 | foo  |    1 | foo  |
|    2 | bar  |    2 | bar  |
|    3 | test |    3 | test |
+------+------+------+------+
3 rows in set (0.00 sec)

mysql> UNLOCK TABLES;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

## 写锁 (write lock)

当一个会话给表加写锁后，其它会话所有读取、更新、删除和插入将会阻塞，直到将表解锁。

{% highlight text %}
mysql[s1]> LOCK TABLE foobar WRITE;                # s1中对表加写锁
Query OK, 0 rows affected (0.00 sec)

mysql[s2]> SELECT * FROM foobar;                   # s2的读请求将会被阻塞，直到锁释放
{% endhighlight %}

同上，锁表的时候一定要把所有需要访问的表都锁住，因为锁表之后无法访问其他未加锁的表。

## concurrent_insert、local 操作

注意，这两个操作操作是 MyISAM 引擎所特有，InnoDB 无此功能。如上，当给表加锁之后，其它会话的写入将会被阻塞，通过 local 关键字，可以让其它会话也能添加数据。

上面所说的功能还需要配合 concurrent_insert 全局变量使用，该变量有三个值：

* NEVER(0)：加读锁后，不允许其它会话并发写入。
* AUTO(1)：加读锁后，在表里没有空洞(就是没有删除过行)的条件下，允许其会话并发写入。
* ALWAYS(2)：加读锁后，允许其它会话并发写入。

可以通过如下命令可以查看当前数据库的设置：

{% highlight text %}
mysql> SHOW GLOBAL VARIABLES LIKE 'concurrent_insert';
{% endhighlight %}

接下来测试一下。


{% highlight text %}
mysql[s1]> SET GLOBAL concurrent_insert=ALWAYS;
Query OK, 0 rows affected (0.00 sec)

mysql[s1]> LOCK TABLE foobar READ LOCAL;
Query OK, 0 rows affected (0.00 sec)

mysql[s2]> INSERT INTO foobar VALUES (4, 'again');
Query OK, 1 rows affected (0.00 sec)
mysql[s2]> SELECT * FROM foobar;
+------+-------+
| id   | name  |
+------+-------+
|    1 | foo   |
|    2 | bar   |
|    3 | test  |
|    4 | again |
+------+-------+
4 rows in set (0.00 sec)


mysql[s1]> SELECT * FROM foobar;
+------+-------+
| id   | name  |
+------+-------+
|    1 | foo   |
|    2 | bar   |
|    3 | test  |
+------+-------+
3 rows in set (0.00 sec)
{% endhighlight %}

注意，此时 s1 对 s2 写入的数据是不可见的，只有当 s1 释放锁之后才可见。

## MyISAM 锁的调度机制

默认写锁优于读锁，当有大量的对同一个表的读写请求时，只有在所有的写请求执行完成后，读请求才能获得执行机会。着就会导致，一个大量更新的表，将会无法读取数据。

### 手动设置优先级

此时可以使用 LOW_PRIORITY、HIGH_PRIORITY 和 DELAYED 关键字，来缓解这一问题；在执行 DELETE、INSERT、UPDATE、LOAD DATA 和 REPLACE 的时候可以使用 LOW_PRIORITY 来降低该更新语句的优先级，让读取操作能够执行。而在执行 SELECT 时，可以使用 HIGH_PRIORITY 来提高该语句的优先级，让读取操作能够执行。

另外，可以在执行 insert 和 replace 的时候可以使用 DELAYED 让 MySQL 返回 OK 状态给客户端，并且修改也是对该会话可见的。

不过，上述方法并非已经将数据插入表，而是存储在内存等待队列中，当能够获得表的写锁再插入。优点时客户无序等待，提高写入速度；坏处是，无法返回自增 ID，系统崩溃时会导致数据丢失。

### set LOW_PRIORITY_UPDATES = 1

让所有支持 LOW_PRIORITY 选项的语句都默认地按照低优先级来处理。

### MAX_WRITE_LOCK_COUNT

该变量默认为 int 最大值，表示当一个表的写锁数量达到设定的值后，就降低写锁的优先级，让读锁有机会执行。

### 查看锁竞争

直接通过如下命令查看。

{% highlight text %}
mysql> SHOW STATUS LIKE 'table_locks%'
{% endhighlight %}

# 参考

相关内容可以参考官方文档 [MySQL Internal Manual - MyISAM Storage Engine](http://dev.mysql.com/doc/internals/en/myisam.html)、[MySQL Reference Manual - The MyISAM Storage Engine](http://dev.mysql.com/doc/refman/en/myisam-storage-engine.html) 。


{% highlight text %}
{% endhighlight %}
