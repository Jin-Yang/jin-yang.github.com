---
title: MySQL 内存配置
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,memory,内存
description:
---


<!-- more -->

![Monitor Logo]({{ site.url }}/images/databases/mysql/monitor-logo.png "Monitor Logo"){: .pull-center }


如果应用程序使用内存过多，可能会导致操作系统通过 oom-killer 将其杀死，通常会在 syslog 日志中有所体现。MySQL 内存分为了两部分，内存使用量可以查看 [MySQL 内存计算器](http://www.mysqlcalculator.com/) 。

这里的内存配置，可以直接通过 SHOW VARIABLES LIKE 'variable-name' 查看。

### 全局级别


#### key_buffer_size
#### query_cache_size
#### tmp_table_size
#### innodb_buffer_pool_size

InnoDB Buffer Pool 的内存大小。

#### innodb_additional_mem_pool_size

InnoDB 中其它内存使用。

#### innodb_log_buffer_size










### 会话级别

通过 max_connections 可以设置最大连接数，然后计算单个连接最大的内存消耗，就可以计算出与会话相关的最大内存消耗。

需要注意的是 read_buffer_size, sort_buffer_size, read_rnd_buffer_size, tmp_table_size 这些参数在需要的时候才分配，操作后释放；而且不管使用多少都分配该 size 的值所对应的内存数，即使实际需要远远小于这些 size。

另外，每个线程可能会不止一次需要分配 buffer，例如子查询，每层都需要有自己的 read_buffer, sort_buffer, tmp_table_size 等。

#### sort_buffer_size

排序使用最大内存，在 Linux 下，如果超过 256KB 和 2MB 建议使用会话级变量。

#### read_buffer_size

#### read_rnd_buffer_size

#### join_buffer_size

#### thread_stack

#### binlog_cache_size

{% highlight text %}
mysql> SHOW VARIABLES LIKE '%binlog_cache_size%';
+-----------------------+----------------------+
| Variable_name         | Value                |
+-----------------------+----------------------+
| binlog_cache_size     | 32768                |
| max_binlog_cache_size | 18446744073709547520 |
+-----------------------+----------------------+
2 rows in set (0.00 sec)
{% endhighlight %}





### sort_buffer_size

mysql> SHOW VARIABLES LIKE '%sort_buffer_size%';
mysql> SET GLOBAL sort_buffer_size = 1024*1024;
mysql> SHOW GLOBAL STATUS LIKE '%sort%';

sort_buffer_size   每个会话都会分配，通用排序优化与存储引擎无关。注意: 需要最少存储15条记录。如果秒级的 SHOW GLOBAL STATUS LIKE 'Sort_merge_passes' 参数较大，可以适当增加该参数，从而优化 ORDER BY, GROUP BY 的性能。   另外，优化器可能会分配过多的内存，进而影响其它查询，所以最好针对需要较大内存的查询设置会话变量，减小影响。在 Linux 中，有 256KB 和 2MB 两个边界值，可能会导致内存申请效率降低。max_sort_length   在执行 GROUP BY, ORDER BY, DISTINCT 操作时，比较时判断多少字节；如果需要增加该值，则需要同时调整 sort_buffer_size 。innodb_sort_buffer_size   ???官方文档提供了一个计算方法，没有看懂???   创建 InnoDB 索引的时候可以使用的排序缓存大小，索引创建完成后释放，???同时也会影响到 Online DDL???。创建索引时分为了 sort+merge 两个阶段，增加该值可以减小迭代次数，从而加快索引创建速度。myisam_sort_buffer_size   MyISAM 在 REPAIR TABLE(Sorting Index), CREATE INDEX(Creating Index), ALTER TABLE(Creating Index) 时使用内存数目。


默认256K每个session 需要做一个排序分配的一个buffer,sort_buffer_size 不指定任何的存储引擎,适用于一般的方式进行优化如果你看到很多的ort_merge_passes per second你可以考虑增加sort_buffer_size 来加速ORDER BY 或者GROUP BY 操作,不能通过查询或者索引优化的。在MySQL 5.6.4 优化器尝试解决需要多少空间,但可以分配更多,达到极限。 在MySQL 5.6.4, 优化器分配整个buffer 即使如果根本不需要所有。在任何情况下, 设置它大于需要的全局会减慢很多的查询。最后是作为一个会话设置来增加,只有对需要大量的内存的会话, 在Linux上,有阀值为256KB 和2MB ,大的值可能显著的减慢内存分配,因此你应该考虑下面中的一个值。Index Condition Pushdown (EXPLAIN: Using Index Condition)首先 MySQL-Server 和 Storage-Engine 是两个组件，Server 负责 SQL 解析、优化、执行；Storage Engine 真正的负责 data+index 的读写。入入. 以前是这样: server 命令 storage engine 按 index 把相应的 数据 从 数据表读出, 传给server, server来按 where条件 做选择; 现在 ICP则是在 可能的情况下, 让storage engine 根据index 做判断, 如果不符合 条件 则无须 读 数据表. 这样 节省了disk IO. https://dev.mysql.com/doc/refman/5.6/en/index-condition-pushdown-optimization.html•不用index 因为 你 是 select *, 而且你的where 是 >=, mysql 如果用index查找 则 会有 太多的 random disk IO. 所以它选择了 全表读.如何避免全表扫描通过 EXPLAIN 查看时，如果在 type 字段中有 ALL 的话，就表示为全表扫描，通常为如下条件：• 表比较小(小于10行，行比较短)，使用索引反而会浪费时间；• 在 ON 和 WHERE 子句中，没有有效的约束条件；• 区分度(cardinality) 过低，全表扫描效率可能更高；可以通过 ANALYZE TABLE tbl_name 更新，或者使用 FORCE INDEX 指定索引。SELECT * FROM t1, t2 FORCE INDEX (idx_for_column) WHERE t1.col_name=t2.col_name;另外，可以设置 max_seeks_for_key=1000 ，也就是告诉优化器，所有的 key scan 不会超过 1000 次。？？？？？？Cardinality is the count of how many items in the index are unique.Reducing max_seeks_for_key to 1,000 is like telling MySQL that you want it to use indexes when the cardinality of the index is over 1,000. I’ve seen this variable reduced to as low as 1 on some servers without any issues.https://major.io/2007/08/03/obscure-mysql-variable-explained-max_seeks_for_key/


'Using index condition' VS. 'Using where; Using index'Using index condition : where condition contains indexed and non-indexed column and the optimizer will first resolve the indexed column and will look for the rows in the table for the other condition (index push down)Using where; Using index : 'Using index' meaning not doing the scan of entire table. 'Using where' may still do the table scan on non-indexed column but it will use if there is any indexed column in the where condition first more like using index conditionWhich is better? 'Using where; Using index' would be better then 'Using index condition' if query has index all covering.When you see Using Index in the Extra part of an explain it means that the (covering) index is adequate for the query. In your example: SELECT id FROM test WHERE id = 5; the server doesn't need to access the actual table as it can satisfy the query (you only access id) only using the index (as the explain says). In case you are not aware the PK is implemented via a unique index. When you see Using Index; Using where it means that first the index is used to retrieve the records (an actual access to the table is not needed) and then on top of this result set the filtering of the where clause is done. In this example: SELECT id FROM test WHERE id > 5; you still fetch for id from the index and then apply the greater than condition to filter out the records non matching the condition


https://dev.mysql.com/doc/refman/5.6/en/index-condition-pushdown-optimization.html





ORDER BY 优化   未完成???MySQL 中部分场景可以通过索引将排序优化消除掉。CREATE TABLE IF NOT EXISTS foobar (    id INT NOT NULL,    first_name VARCHAR(30) NOT NULL,    last_name VARCHAR(30) NOT NULL,    address VARCHAR(150) NOT NULL,    INDEX idx_name (first_name, last_name),    PRIMARY KEY(id))""")CREATE TABLE IF NOT EXISTS foobar (    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,    first_name VARCHAR(30) NOT NULL,    last_name VARCHAR(30) NOT NULL,    address VARCHAR(150) NOT NULL,    INDEX idx_name (first_name, last_name));INSERT INTO foobar(first_name, last_name, address) VALUES ('Andy', 'Ddufresne', 'The Shawshank Redemption');SELECT * FROM foobar WHERE first_name='Andy' and last_name='Ddufresne';// key_part1,key_part2 前缀匹配，排序方式相同EXPLAIN SELECT * FROM foobar ORDER BY first_name,first_name LIMIT 10;EXPLAIN SELECT * FROM foobar ORDER BY first_name DESC, first_name DESC LIMIT 10;+----+-------------+--------+-------+---------------+----------+---------+------+------+-------+| id | select_type | table  | type  | possible_keys | key      | key_len | ref  | rows | Extra |+----+-------------+--------+-------+---------------+----------+---------+------+------+-------+|  1 | SIMPLE      | foobar | index | NULL          | idx_name | 184     | NULL |   10 | NULL  |+----+-------------+--------+-------+---------------+----------+---------+------+------+-------+1 row in set (0.00 sec)// key_part1=constant,key_part2 开始为常量  ???为什么第一个采用了Using index conditionEXPLAIN SELECT * FROM foobar WHERE first_name='Andy' ORDER BY last_name;+----+-------------+--------+------+---------------+----------+---------+-------+-------+------------------------------------+| id | select_type | table  | type | possible_keys | key      | key_len | ref   | rows  | Extra                              |+----+-------------+--------+------+---------------+----------+---------+-------+-------+------------------------------------+|  1 | SIMPLE      | foobar | ref  | idx_name      | idx_name | 92      | const | 19244 | Using index condition; Using where |+----+-------------+--------+------+---------------+----------+---------+-------+-------+------------------------------------+1 row in set (0.00 sec)EXPLAIN SELECT * FROM foobar WHERE first_name='Andy' ORDER BY last_name DESC;+----+-------------+--------+------+---------------+----------+---------+-------+-------+-------------+| id | select_type | table  | type | possible_keys | key      | key_len | ref   | rows  | Extra       |+----+-------------+--------+------+---------------+----------+---------+-------+-------+-------------+|  1 | SIMPLE      | foobar | ref  | idx_name      | idx_name | 92      | const | 19244 | Using where |+----+-------------+--------+------+---------------+----------+---------+-------+-------+-------------+1 row in set (0.00 sec)// key_part1 > const ???为什么采用filesortEXPLAIN SELECT * FROM foobar WHERE first_name > 'Andy' ORDER BY last_name ASC;+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+| id | select_type | table  | type | possible_keys | key  | key_len | ref  | rows  | Extra                       |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+|  1 | SIMPLE      | foobar | ALL  | idx_name      | NULL | NULL    | NULL | 99391 | Using where; Using filesort |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+1 row in set (0.00 sec)// key_part1 < const ???为什么采用filesortEXPLAIN SELECT * FROM foobar WHERE first_name < 'Andy' ORDER BY last_name DESC;+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+| id | select_type | table  | type | possible_keys | key  | key_len | ref  | rows  | Extra                       |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+|  1 | SIMPLE      | foobar | ALL  | idx_name      | NULL | NULL    | NULL | 99391 | Using where; Using filesort |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+1 row in set (0.00 sec)EXPLAIN SELECT * FROM foobar WHERE first_name = 'Andy' AND last_name > 'Ddufresne' ORDER BY last_name;+----+-------------+--------+-------+---------------+----------+---------+------+------+-----------------------+| id | select_type | table  | type  | possible_keys | key      | key_len | ref  | rows | Extra                 |+----+-------------+--------+-------+---------------+----------+---------+------+------+-----------------------+|  1 | SIMPLE      | foobar | range | idx_name      | idx_name | 184     | NULL |    1 | Using index condition |+----+-------------+--------+-------+---------------+----------+---------+------+------+-----------------------+1 row in set (0.00 sec)使用两个不同索引EXPLAIN SELECT * FROM foobar ORDER BY id,first_name LIMIT 10;+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+| id | select_type | table  | type | possible_keys | key  | key_len | ref  | rows  | Extra          |+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+|  1 | SIMPLE      | foobar | ALL  | NULL          | NULL | NULL    | NULL | 99391 | Using filesort |+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+1 row in set (0.00 sec)非前缀匹配EXPLAIN SELECT * FROM foobar WHERE last_name='Ddufresne' ORDER BY first_name; +----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+| id | select_type | table  | type | possible_keys | key  | key_len | ref  | rows  | Extra                       |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+|  1 | SIMPLE      | foobar | ALL  | NULL          | NULL | NULL    | NULL | 99391 | Using where; Using filesort |+----+-------------+--------+------+---------------+------+---------+------+-------+-----------------------------+1 row in set (0.00 sec)采用不同排序策略EXPLAIN SELECT * FROM foobar ORDER BY first_name DESC, last_name ASC LIMIT 10;+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+| id | select_type | table  | type | possible_keys | key  | key_len | ref  | rows  | Extra          |+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+|  1 | SIMPLE      | foobar | ALL  | NULL          | NULL | NULL    | NULL | 99391 | Using filesort |+----+-------------+--------+------+---------------+------+---------+------+-------+----------------+1 row in set (0.00 sec)数据过滤与排序采用不同索引 ？？？？ 貌似部分情况可以使用EXPLAIN SELECT * FROM foobar WHERE first_name='Andy' ORDER BY id;+----+-------------+--------+------+---------------+----------+---------+-------+-------+----------------------------------------------------+| id | select_type | table  | type | possible_keys | key      | key_len | ref   | rows  | Extra                                              |+----+-------------+--------+------+---------------+----------+---------+-------+-------+----------------------------------------------------+|  1 | SIMPLE      | foobar | ref  | idx_name      | idx_name | 92      | const | 19244 | Using index condition; Using where; Using filesort |+----+-------------+--------+------+---------------+----------+---------+-------+-------+----------------------------------------------------+1 row in set (0.00 sec)排序字段不只有字段，可能有函数处理SELECT * FROM t1 ORDER BY ABS(key);SELECT * FROM t1 ORDER BY -key;• The query joins many tables, and the columns in the ORDER BY are not all from the first nonconstant table that is used to retrieve rows. (This is the first table in the EXPLAIN output that does not have a const join type.) • The query has different ORDER BY and GROUP BY expressions. • There is an index on only a prefix of a column named in the ORDER BY clause. In this case, the index cannot be used to fully resolve the sort order. For example, if only the first 10 bytes of a CHAR(20) column are indexed, the index cannot distinguish values past the 10th byte and a filesort will be needed. • The index does not store rows in order. For example, this is true for a HASH index in a MEMORY table.








{% highlight text %}
{% endhighlight %}
