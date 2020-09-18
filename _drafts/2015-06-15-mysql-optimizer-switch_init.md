---
Date: October 19, 2013
title: MySQL 优化开关
layout: post
comments: true
language: chinese
category: [mysql,database]
---


<!-- more -->

![mysql optimizer]({{ site.url }}/images/databases/mysql/optimizer-logo.png "mysql optimizer"){: .pull-center width="80%" }

## 简介

其中，MySQL 存在一系列的优化器开关，可以通过如下命令查看以及设置。

{% highlight text %}
----- 查看优化器选项
mysql> SHOW VARIABLES LIKE 'optimizer_switch';

----- 关闭ICP选项，其它优化器选项不变
mysql> SET optimizer_switch='index_condition_pushdown=off';
{% endhighlight %}


### 源码相关

MySQL 优化器相关的开关，与源码相关的主要涉及 sql/{sql_const.h,sql_class.h,sys_vars.cc} 几个文件，相关内容分别如下。

{% highlight c %}
/* @@optimizer_switch flags. These must be in sync with optimizer_switch_typelib */
#define OPTIMIZER_SWITCH_INDEX_MERGE               (1ULL << 0)
#define OPTIMIZER_SWITCH_INDEX_MERGE_UNION         (1ULL << 1)
#define OPTIMIZER_SWITCH_INDEX_MERGE_SORT_UNION    (1ULL << 2)
#define OPTIMIZER_SWITCH_INDEX_MERGE_INTERSECT     (1ULL << 3)
#define OPTIMIZER_SWITCH_ENGINE_CONDITION_PUSHDOWN (1ULL << 4)
#define OPTIMIZER_SWITCH_INDEX_CONDITION_PUSHDOWN  (1ULL << 5)
#define OPTIMIZER_SWITCH_MRR                       (1ULL << 6)
#define OPTIMIZER_SWITCH_MRR_COST_BASED            (1ULL << 7)
#define OPTIMIZER_SWITCH_BNL                       (1ULL << 8)
#define OPTIMIZER_SWITCH_BKA                       (1ULL << 9)
#define OPTIMIZER_SWITCH_MATERIALIZATION           (1ULL << 10)
#define OPTIMIZER_SWITCH_SEMIJOIN                  (1ULL << 11)
#define OPTIMIZER_SWITCH_LOOSE_SCAN                (1ULL << 12)
#define OPTIMIZER_SWITCH_FIRSTMATCH                (1ULL << 13)
#define OPTIMIZER_SWITCH_DUPSWEEDOUT               (1ULL << 14)
#define OPTIMIZER_SWITCH_SUBQ_MAT_COST_BASED       (1ULL << 15)
#define OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS      (1ULL << 16)
#define OPTIMIZER_SWITCH_COND_FANOUT_FILTER        (1ULL << 17)
#define OPTIMIZER_SWITCH_DERIVED_MERGE             (1ULL << 18)
#define OPTIMIZER_SWITCH_LAST                      (1ULL << 19)
{% endhighlight %}


{% highlight c %}
/** Tells whether the given optimizer_switch flag is on */
inline bool optimizer_switch_flag(ulonglong flag) const
{
  return (variables.optimizer_switch & flag);
}
{% endhighlight %}

{% highlight c %}
static const char *optimizer_switch_names[]=
{
  "index_merge", "index_merge_union", "index_merge_sort_union",
  "index_merge_intersection", "engine_condition_pushdown",
  "index_condition_pushdown" , "mrr", "mrr_cost_based",
  "block_nested_loop", "batched_key_access",
  "materialization", "semijoin", "loosescan", "firstmatch", "duplicateweedout",
  "subquery_materialization_cost_based",
  "use_index_extensions", "condition_fanout_filter", "derived_merge",
  "default", NullS
};
static Sys_var_flagset Sys_optimizer_switch(
       "optimizer_switch",
       "optimizer_switch=option=val[,option=val...], where option is one of "
       "{index_merge, index_merge_union, index_merge_sort_union, "
       "index_merge_intersection, engine_condition_pushdown, "
       "index_condition_pushdown, mrr, mrr_cost_based"
       ", materialization, semijoin, loosescan, firstmatch, duplicateweedout,"
       " subquery_materialization_cost_based"
       ", block_nested_loop, batched_key_access, use_index_extensions,"
       " condition_fanout_filter, derived_merge} and val is one of "
       "{on, off, default}",
       SESSION_VAR(optimizer_switch), CMD_LINE(REQUIRED_ARG),
       optimizer_switch_names, DEFAULT(OPTIMIZER_SWITCH_DEFAULT),
       NO_MUTEX_GUARD, NOT_IN_BINLOG, ON_CHECK(NULL), ON_UPDATE(NULL));
{% endhighlight %}

## 优化详解

数据仍然参考 [MySQL 基本介绍](/post/mysql-basic.html) 中的第一个示例，为了测试方便，需要添加一个索引。

{% highlight text %}
mysql> CREATE INDEX idx_contacter ON customers (contactLastName, contactFirstName);
{% endhighlight %}

接下来，就挨个看看各种优化选项。

### Index Condition Pushdown, ICP

ICP 是 MySQL 5.6 新增的特性，是一种在存储引擎层使用索引过滤数据的优化方式。

{% highlight text %}
----- 查看优化器选项
mysql> SHOW VARIABLES LIKE 'optimizer_switch';

----- 关闭ICP选项，其它优化器选项不变
mysql> SET optimizer_switch='index_condition_pushdown=off';

----- ICP示例
mysql> EXPLAIN SELECT * FROM customers WHERE contactLastName = 'Young' AND contactFirstName LIKE 'J%';
+----+-------------+-----------+------------+-------+---------------+---------------+---------+------+------+----------+-----------------------+
| id | select_type | table     | partitions | type  | possible_keys | key           | key_len | ref  | rows | filtered | Extra                 |
+----+-------------+-----------+------------+-------+---------------+---------------+---------+------+------+----------+-----------------------+
|  1 | SIMPLE      | customers | NULL       | range | idx_contacter | idx_contacter | 104     | NULL |    1 |   100.00 | Using index condition |
+----+-------------+-----------+------------+-------+---------------+---------------+---------+------+------+----------+-----------------------+
1 row in set, 1 warning (0.01 sec)
{% endhighlight %}

在源码 sql/sql_select.cc 文件中，有如下的相关注释，介绍在什么条件下使用 ICP 。

{% highlight c %}
/*
  We will only attempt to push down an index condition when the
  following criteria are true:
  0. The table has a select condition
  1. The storage engine supports ICP.
  2. The index_condition_pushdown switch is on and
     the use of ICP is not disabled by the NO_ICP hint.
  3. The query is not a multi-table update or delete statement. The reason
     for this requirement is that the same handler will be used
     both for doing the select/join and the update. The pushed index
     condition might then also be applied by the storage engine
     when doing the update part and result in either not finding
     the record to update or updating the wrong record.
  4. The JOIN_TAB is not part of a subquery that has guarded conditions
     that can be turned on or off during execution of a 'Full scan on NULL
     key'.
     @see Item_in_optimizer::val_int()
     @see subselect_single_select_engine::exec()
     @see TABLE_REF::cond_guards
     @see setup_join_buffering
  5. The join type is not CONST or SYSTEM. The reason for excluding
     these join types, is that these are optimized to only read the
     record once from the storage engine and later re-use it. In a
     join where a pushed index condition evaluates fields from
     tables earlier in the join sequence, the pushed condition would
     only be evaluated the first time the record value was needed.
  6. The index is not a clustered index. The performance improvement
     of pushing an index condition on a clustered key is much lower
     than on a non-clustered key. This restriction should be
     re-evaluated when WL#6061 is implemented.
  7. The index on virtual generated columns is not supported for ICP.
*/
{% endhighlight %}



a 当关闭ICP时,index 仅仅是data access 的一种访问方式，存储引擎通过索引回表获取的数据会传递到MySQL Server 层进行where条件过滤。
b 当打开ICP时,如果部分where条件能使用索引中的字段,MySQL Server 会把这部分下推到引擎层,可以利用index过滤的where条件在存储引擎层进行数据过滤,而非将所有通过index access的结果传递到MySQL server层进行where过滤.
优化效果:ICP能减少引擎层访问基表的次数和MySQL Server 访问存储引擎的次数,减少io次数，提高查询语句性能。



{% highlight text %}
index_read()/general_fetch()
  |-row_search_mvcc()
    |-row_search_idx_cond_check()
      |-innobase_index_cond()

JOIN::optimize()
  |-make_join_readinfo()
    |-push_index_cond()               将索引条件下发到存储引擎
{% endhighlight %}





### Multi-Range Read, MRR

MySQL 5.6 版本，对于二级索引的范围扫描并且需要回表的情况，进行的优化；主要是减少随机 IO，并且将随机 IO 转化为顺序 IO，提高查询效率。

MRR 适用于以下两种情况：

1. range access；
2. ref and eq_ref access, when they are using Batched Key Access 。

另外，使用索引的等值 JOIN 查询也可以进行优化。

#### 原理

详细来说，其原理是，将多个需要回表的二级索引根据主键进行排序，然后一起回表，将原来的回表时进行的随机 IO，转变成顺序 IO。

![optimizer switch mrr]({{ site.url }}/images/databases/mysql/optimizer_switch_mrr_1.png "optimizer switch mrr"){: .pull-center }

如上是没有开启 MRR 的情况下，MySQL 执行查询的伪代码大致如下：

{% highlight text %}
1. 根据where条件中的二级索引获取二级索引与主键的集合，结果集为result。
   select key_column, pk_column from table where key_column=XXX order by key_column
2. 通过第一步获取的主键来获取对应的值。
   for each pk_column value in result do:
      select non_key_column from table where pk_column=val
{% endhighlight %}

由于 MySQL 存储数据的方式导致二级索引的存储顺序与主键的存储顺序不一致，如果根据二级索引获取的主键来访问表中的数据会导致大量的随机 IO，尤其当不同主键不在同一个 page 里面时，必然导致多次 IO 和随机读。

![optimizer switch mrr]({{ site.url }}/images/databases/mysql/optimizer_switch_mrr_2.png "optimizer switch mrr"){: .pull-center }

上图是在使用 MRR 优化特性的情况下，此时 MySQL 对于基于二级索引的查询策略为：

{% highlight text %}
1. 根据where条件中的二级索引获取二级索引与主键的集合，结果集为result。
   select key_column, pk_column from table where key_column=XXX order by key_column
2. 将结果集result放在缓存中(read_rnd_buffer_size大小直到buffer满了)，
   然后对结果集result按照pk_column排序，得到结果集是result_sort
3. 利用已经排序过的结果集，访问表中的数据，此时是顺序IO
   select non_key_column from table where pk_column in (result_sort)
{% endhighlight %}

MySQL 根据二级索引获取的结果集根据主键排序，将乱序化为有序，在用主键顺序访问基表时，将随机读转化为顺序读，多页数据记录可一次性读入或根据此次的主键范围分次读入，以减少 IO 操作，提高查询效率。

#### 配置使用

关于该优化选项的查看和配置使用方式如下：

{% highlight text %}
----- 查看mrr/mrr_cost_based是否开启
mysql> SHOW VARIABLES LIKE 'optimizer_switch';

----- 打开开关
mysql> SET GLOBAL optimizer_switch ='mrr=on,mrr_cost_based=on';
{% endhighlight %}

mrr=on 表示启用 MRR 优化；mrr_cost_based 是否通过 cost base 的方式来启用 MRR；如果选择 mrr=on,mrr_cost_based=off 则表示总是开启 MRR 优化； 参数 read_rnd_buffer_size 用来控制键值缓冲区的大小。

<!--
当开启MRR时
MySQL > explain select * from tbl where tbl.key1 between 1000 and 2000;
+----+-------------+-------+-------+---------------+------+---------+------+------+-------------------------------------------+
| id | select_type | table | type  | possible_keys | key  | key_len | ref  | rows | Extra                                     |
+----+-------------+-------+-------+---------------+------+---------+------+------+-------------------------------------------+
| 1  | SIMPLE      | tbl   | range | key1          | key1 | 5       | NULL | 960  | Using index condition; Using MRR          |
+----+-------------+-------+-------+---------------+------+---------+------+------+-------------------------------------------+
1 row in set (0.03 sec)

http://blog.itpub.net/22664653/viewspace-1673682   【MySQL】MySQL5.6新特性之Multi-Range Read
https://www.percona.com/blog/2012/03/21/multi-range-read-mrr-in-mysql-5-6-and-mariadb-5-5/
-->





<!--
order by的实现有两种方式，主要就是按用没用到索引来区分：
1. 根据索引字段排序，利用索引取出的数据已经是排好序的，直接返回给客户端；
2. 没有用到索引，将取出的数据进行一次排序操作后返回给客户端。
下面通过示例来介绍这两种方式间的差异，首先利用索引进行order by操作，使用explain分析得出的执行计划：
EXPLAIN SELECT m.id,m.subject,c.content FROM group_message m,group_message_content c WHERE m.group_id = 1 AND m.id = c.group_msg_id ORDER BY m.user_id\G;
-->






## 参考




http://dev.mysql.com/doc/refman/en/mrr-optimization.html




{% highlight text %}
{% endhighlight %}
