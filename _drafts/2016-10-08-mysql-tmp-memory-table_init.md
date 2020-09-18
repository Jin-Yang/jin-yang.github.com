---
title: MySQL 临时表和内存表
layout: post
comments: true
language: chinese
category: [mysql,database]
---

MySQL 在处理一些语句时，会自动创建临时表，当然这是客户端无法控制的。

<!-- more -->

## 简介

会话使用到临时表时，会自动分配，最大使用内存是 ```min(tmp_table_size, max_heap_table_size)``` ，如果内存中超出了限制，就会自动将内存中的数据转化到磁盘，存储在 tmpdir 目录下。

首先看下内存表的使用。

{% highlight text %}
----- TRANS A -------------------------------------+----- TRANS B --------------------------------
CREATE TABLE tmp_mem (i INT) ENGINE = memory;
Query OK, 0 rows affected (0.00 sec)

INSERT INTO tmp_mem VALUES(1);
Query OK, 1 row affected (0.00 sec)
                                                     CREATE TABLE tmp_mem (i INT) ENGINE = memory;
                                                     ERROR 1050 (42S01): Table 'tmp_mem' already exists

                                                     SELECT * FROM tmp_mem;
                                                     +------+
                                                     | i    |
                                                     +------+
                                                     | 1    |
                                                     +------+
                                                     1 row in set (0.00 sec)
{% endhighlight %}

内存表会将表结构信息 tmp_mem.frm 保存在磁盘上，而数据则保存在内存中，所以有如下特性：

1. 由于会在磁盘上保存表结构信息，所以多个会话，创建表的名字不能一样；
2. 一个会话创建表，写入数据之后，对其它会话也是可见的；
3. 服务器重启后内存表里的数据会丢失，但是表结构仍然存在；
4. 可以创建、删除索引，支持唯一索引；
5. 不影响主备复制，主库上插入的数据，备库也可以查到；
6. 可以通过 ```SHOW TABLES``` 查看表。

下面来看看临时表。

{% highlight text %}
----- TRANS A -------------------------------------+----- TRANS B --------------------------------
CREATE TEMPORARY TABLE tmp_tbl (i INT);
Query OK, 0 rows affected (0.00 sec)

INSERT INTO tmp_tbl VALUES(1);
Query OK, 1 row affected (0.00 sec)

SELECT * FROM tmp_tbl;
+------+
| i    |
+------+
| 1    |
+------+
1 row in set (0.00 sec)
                                                     CREATE TEMPORARY TABLE tmp_tbl (i INT);
                                                     Query OK, 0 rows affected (0.00 sec)

                                                     INSERT INTO tmp_tbl VALUES(2);
                                                     Query OK, 1 row affected (0.00 sec)

                                                     SELECT * FROM tmp_mem;
                                                     +------+
                                                     | i    |
                                                     +------+
                                                     | 2    |
                                                     +------+
                                                     1 row in set (0.00 sec)
{% endhighlight %}

可以看出来，临时表的表结构和数据都保存在内存中，有如下特性：

1. 不同的会话创建的表名可以相同；
2. 会话消失表结构和数据都消失；
3. 可以创建、删除索引；
4. 主库创建的表，备库查不到；
5. 通过 ```SHOW TABLES``` 无法看到表。


### 配置参数





max_tmp_tables 一个客户能同时保持打开的临时表的最大数量，这个值默认32，可以根据需要调整此值





在某些场景下，服务器会创建内部临时表，当然，这一行为用户是无法控制的，其中常见的场景如下：

* UNION 查询，部分不使用的场景后面介绍；
* 一些视图相关的使用，例如使用到了 TEMPTABLE、UNION、聚合；
* 处理衍生表时，也就是子查询在 FROM 中；
* 对于子查询或者 semi-join 时创建的表，可查看 [Optimizing Subqueries](https://dev.mysql.com/doc/refman/en/subquery-optimization.html)；
* ORDER BY 和 GROUP BY 子句不同，ORDER BY 和 GROUP BY 的列不是第一个表中列；
* DISTINCT 查询并且加上 ORDER BY 可能需要临时表；

<!--
* 使用了 SQL_SMALL_RESULT ???????
* 表连接中，ORDER BY 的列不是驱动表中的；
* SQL中用到SQL_SMALL_RESULT选项时；
-->

### 临时表





它规定了内部内存临时表的最大值，每个线程都要分配。（实际起限制作用的是tmp_table_size和max_heap_table_size的最小值。）如果内存临时表超出了限制，MySQL就会自动地把它转化为基于磁盘的MyISAM表，存储在指定的tmpdir目录下，默认:

mysql> show variables like "tmpdir";
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| tmpdir        | /tmp/ |
+---------------+-------+

优化查询语句的时候，要避免使用临时表，如果实在避免不了的话，要保证这些临时表是存在内存中的。如果需要的话并且你有很多group by语句，并且你有很多内存，增大tmp_table_size(和max_heap_table_size)的值。这个变量不适用与用户创建的内存表(memory table).

你可以比较内部基于磁盘的临时表的总数和创建在内存中的临时表的总数（Created_tmp_disk_tables和Created_tmp_tables），一般的比例关系是:

Created_tmp_disk_tables/Created_tmp_tables<5%

max_heap_table_size

这个变量定义了用户可以创建的内存表(memory table)的大小.这个值用来计算内存表的最大行数值。这个变量支持动态改变，即set @max_heap_table_size=#

,但是对于已经存在的内存表就没有什么用了，除非这个表被重新创建(create table)或者修改(alter table)或者truncate table。服务重启也会设置已经存在的内存表为全局max_heap_table_size的值。

这个变量和tmp_table_size一起限制了内部内存表的大小。

如果想知道更详细的信息，请参考“MySQL是怎样使用内部临时表的？”和“内存存储引擎”






## 临时表








内存表：
1. 参数控制：max_heap_table_size
2. 到达上线后报错。
3. 表定义保存在磁盘上，数据和索引保存在内存里面。
4. 不能包含TEXT,BLOB等字段。

临时表：
1. 参数控制：tmp_table_size。
2. 到达上线后创建文件在磁盘上。
3. 表定义和数据都在内存里。
4. 可以包含TEXT, BLOB等字段。








1、heap对所有用户的连接是可见的，这使得它非常适合做缓存。

2、仅适合使用的场合。heap不允许使用xxxTEXT和xxxBLOB数据类型；只允许使用=和<=>操作符来搜索记录（不允许<、>、<=或>=）；不支持auto_increment；只允许对非空数据列进行索引（not null）。
注：操作符 “<=>” 说明：NULL-safe equal.这个操作符和“=”操作符执行相同的比较操作，不过在两个操作码均为NULL时，其所得值为1而不为NULL，而当一个操作码为NULL时，其所得值为0而不为NULL。

3、一旦服务器重启，所有heap表数据丢失，但是heap表结构仍然存在，因为heap表结构是存放在实际数据库路径下的，不会自动删除。重启之后，heap将被清空，这时候对heap的查询结果都是空的。

4、如果heap是复制的某数据表，则复制之后所有主键、索引、自增等格式将不复存在，需要重新添加主键和索引，如果需要的话。

5、对于重启造成的数据丢失，有以下的解决办法：
　a、在任何查询之前，执行一次简单的查询，判断heap表是否存在数据，如果不存在，则把数据重新写入，或者DROP表重新复制某张表。这需要多做一次查询。不过可以写成include文件，在需要用该heap表的页面随时调用，比较方便。
　b、对于需要该heap表的页面，在该页面第一次且仅在第一次查询该表时，对数据集结果进行判断，如果结果为空，则需要重新写入数据。这样可以节省一次查询。
　c、更好的办法是在mysql每次重新启动时自动写入数据到heap，但是需要配置服务器，过程比较复杂，通用性受到限制。







有这么几种情况会出现临时内存表：

    包含 UNION 的query；
    使用了 UNION 或者聚合运算的视图，有个算法专门用于判定一个视图是不是会使用临时表： http://dev.mysql.com/doc/refman/5.0/en/view-algorithms.html ；
    如果query包含 order by 和 group by 子句，并且两个子句中的字段不一样；或者 order by 或者 group by 中包含除了第一张表中的字段；
    query中同时包含 distinct 和 order by 。

如果一个query的explain结果中的extra字段包含 using temporary ，说明这个query使用了临时表。

使用explain的时候extra里面如果输出 using temporary; 说明需要使用临时表，但是这里无论如何都看不出来是不是使用了磁盘临时表（因为是不是使用磁盘临时表要根据query结果的大小来判定，在SQL分析阶段是无法判定的）， using filesort 是说没办法使用索引进行排序（ order by 和 group by 都需要排序），只能对输出结果进行quicksort，参考 http://www.mysqlperformanceblog.com/2009/03/05/what-does-using-filesort-mean-in-mysql/

当MySQL处理query是创建了一个临时表（不论是内存临时表还是磁盘临时表）， created_tmp_tables 变量都会增加1。如果创建了一个磁盘临时表， created_tmp_disk_tables 会增加1。

可以通过 show session status like 'Created_tmp_%'; 来查看，如果要查看全局的临时表使用次数，将其中的 session 改为 global 即可。

一般磁盘临时表会比内存临时表慢，因此要尽可能避免出现磁盘临时表。下面几种情况下，MySQL会直接使用磁盘内存表，要尽可能避免：

    表中包含了 BLOB 和 TEXT 字段（MEMORY引擎不支持这两种字段）；
    group by 和 distinct 子句中的有超过512字节的字段；
    UNION 以及 UNION ALL 语句中，如果 SELECT 子句中包含了超过512（对于binary string是512字节，对于character是512个字符）的字段。





## 参考

[MySQL Reference Manual - Internal Temporary Table Use in MySQL](https://dev.mysql.com/doc/refman/en/internal-temporary-tables.html) 。


http://www.open-open.com/lib/view/open1436690608048.html


{% highlight text %}
{% endhighlight %}
