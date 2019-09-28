---
Date: October 19, 2013
title: MySQL 优化详解
layout: post
comments: true
language: chinese
category: [mysql,database]
---

优化应该是 MySQL 中比较复杂的一部分了。

<!-- more -->

![mysql optimizer]({{ site.url }}/images/databases/mysql/optimizer-logo.png "mysql optimizer"){: .pull-center width="80%" }


## 执行计划

可以通过 explain 查看 MySQL 的执行计划。

{% highlight text %}
mysql> [explain|describe|desc] [extended|partitions] select_clause;
{% endhighlight %}

使用 extended 参数后，会包含了优化器优化后的 select 查询语句，可以通过 show warnings 查看；partitions 是用于分区表的查询语句；示例如下：

{% highlight text %}
mysql> explain select emp_no from salaries where salary = 90930;
+----+-------------+----------+------------+------+---------------+------+---------+------+---------+----------+-------------+
| id | select_type | table    | partitions | type | possible_keys | key  | key_len | ref  | rows    | filtered | Extra       |
+----+-------------+----------+------------+------+---------------+------+---------+------+---------+----------+-------------+
|  1 | SIMPLE      | salaries | NULL       | ALL  | NULL          | NULL | NULL    | NULL | 2838718 |    10.00 | Using where |
+----+-------------+----------+------------+------+---------------+------+---------+------+---------+----------+-------------+
1 row in set, 1 warning (0.00 sec)
{% endhighlight %}

执行计划包含的信息如下：

* **id** 识别符，也就是查询序列号；表示查询中执行 select 子句或操作表的顺序，id 相同，执行顺序由上至下，如果是子查询，id 的序号会递增，id 值越大优先级越高，越先被执行。
* **select_type** 类型，可以为以下任何一种:
    1. SIMPLE: 简单 SELECT，没有使用 UNION 或子查询。
* **table** 输出的行所引用的表。
* **type** 联接类型。下面给出各种联接类型,按照从最佳类型到最坏类型进行排序:
    1. ALL: Full Table Scan，遍历全表以找到匹配的行。
    2. index: Full Index Scan，与 ALL 的区别在于只会遍历索引树，通常比 ALL 快，因为索引文件通常比数据文件小。
    3. range: 索引的范围扫描，对索引的扫描开始于某一点，返回匹配值域的行，常见于 in、&lt;、&gt;、between 等查询。
    4. ref: 非唯一性索引扫描，返回匹配某个值的所有行；常见于使用非唯一索引或者唯一索引的非唯一前缀进行的查找。
    5. eq_ref: 唯一性索引扫描，对于每个索引键，表中只有一条记录与之匹配，常见于主键或唯一索引扫描。
    6. const, system: 优化后可以转换为一个常量时，如主键查询；其中，system 是 const 类型的特例，当查询的表只有一行的情况下，即为 system 。
* **possible_keys** 能使用哪个索引在该表中找到行，查询涉及到的字段上若存在索引，则该索引将被列出，但不一定被查询使用。
* **key** 实际决定使用的索引，如果没有选择索引，则是 NULL 。
* **key_len** 使用的键长度，为最大可能长度；如果键是 NULL，则长度为 NULL 。
* **ref** 显示使用哪个列或常数与 key 一起从表中选择行。
* **rows** 认为它执行查询时必须检查的行数，多行之间的数据相乘可以估算要处理的行数。
* **filtered** 显示了通过条件过滤出的行数的百分比估计值。
* **Extra** 该列包含 MySQL 解决查询的详细信息。

数据库可以参考 [MySQL 基本介绍](/post/mysql-basic.html) 中的第一个示例数据库，另外，需要添加一个索引。

{% highlight text %}
mysql> CREATE INDEX idx_contacter ON customers (contactLastName, contactFirstName);
{% endhighlight %}

接下来看看一些常见的示例。

{% highlight text %}
----- select_type(SIMPLE)
mysql> EXPLAIN SELECT * FROM employees WHERE employeeNumber = 1002;
+----+-------------+-----------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
| id | select_type | table     | partitions | type  | possible_keys | key     | key_len | ref   | rows | filtered | Extra |
+----+-------------+-----------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
|  1 | SIMPLE      | employees | NULL       | const | PRIMARY       | PRIMARY | 4       | const |    1 |   100.00 | NULL  |
+----+-------------+-----------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
1 row in set, 1 warning (0.00 sec)
{% endhighlight %}















<!--
###### select_type的说明
UNION
当通过union来连接多个查询结果时，第二个之后的select其select_type为UNION。
explain select * from t_order where order_id=100 union select * from t_order where order_id=200;

DEPENDENT UNION与DEPENDENT SUBQUERY
当union作为子查询时，其中第二个union的select_type就是DEPENDENT UNION，第一个子查询的select_type则是DEPENDENT SUBQUERY。
explain select * from t_order where order_id in (select order_id from t_order where order_id=100 union select order_id from t_order where order_id=200);

SUBQUERY
子查询中的第一个select其select_type为SUBQUERY。
explain select * from t_order where order_id=(select order_id from t_order where order_id=100);

DERIVED
当子查询是from子句时，其select_type为DERIVED。
explain select * from (select order_id from t_order where order_id=100) a;

PRIMARY: 包含复杂的子部分，最外面的 SELECT 为该类型。
<pre style="font-size:0.8em; face:arial;">
explain SELECT customerNumber, checkNumber, amount FROM payments WHERE amount = (
        SELECT MAX(amount) FROM payments);
</pre></li><li>

SUBQUERY: 在 SELECT 或 WHERE 列表中包含了子查询，示例同上。</li><li>

UNION: UNION 中的第二个或后面的 SELECT 语句，最后一个为 NULL。
<pre style="font-size:0.8em; face:arial;">
explain SELECT customerNumber id, contactLastname name FROM customers
        UNION
        SELECT employeeNumber id,firstname name FROM employees
</pre>

            DEPENDENT UNION:UNION中的第二个或后面的SELECT语句,取决于外面的查询</li><li>
            UNION RESULT:UNION 的结果</li><li>
            DEPENDENT SUBQUERY:子查询中的第一个SELECT,取决于外面的查询</li><li>
DERIVED: 在 from 列表中包含的子查询。
<pre style="font-size:0.8em; face:arial;">
explain SELECT max(items), min(items), floor(avg(items)) FROM (
    SELECT count(orderNumber) AS items FROM orderdetails GROUP BY orderNumber) AS lineitems;
</pre>

##### type的说明
system，const
见上面DERIVED的例子。其中第一行的type就是为system，第二行是const，这两种联接类型是最快的。

eq_ref
在t_order表中的order_id是主键，t_order_ext表中的order_id也是主键，该表可以认为是订单表的补充信息表，他们的关系是1对1，在下面的例子中可以看到b表的连接类型是eq_ref，这是极快的联接类型。
explain select * from t_order a,t_order_ext b where a.order_id=b.order_id;

ref
下面的例子在上面的例子上略作了修改，加上了条件。此时b表的联接类型变成了ref。因为所有与a表中order_id=100的匹配记录都将会从b表获取。这是比较常见的联接类型。
explain select * from t_order a,t_order_ext b where a.order_id=b.order_id and a.order_id=100;

ref_or_null
user_id字段是一个可以为空的字段，并对该字段创建了一个索引。在下面的查询中可以看到联接类型为ref_or_null，这是mysql为含有null的字段专门做的处理。在我们的表设计中应当尽量避免索引字段为NULL，因为这会额外的耗费mysql的处理时间来做优化。
explain select * from t_order where user_id=100 or user_id is null;

index_merge
经常出现在使用一张表中的多个索引时。mysql会将多个索引合并在一起，如下例:
explain select * from t_order where order_id=100 or user_id=10;

unique_subquery
该联接类型用于替换value IN (SELECT primary_key FROM single_table WHERE some_expr)这样的子查询的ref。注意ref列，其中第二行显示的是func，表明unique_subquery是一个函数，而不是一个普通的ref。
explain select * from t_order where order_id in (select order_id from t_order where user_id=10);

index_subquery
该联接类型与上面的太像了，唯一的差别就是子查询查的不是主键而是非唯一索引。
explain select * from t_order where user_id in (select user_id from t_order where order_id>10);

range
按指定的范围进行检索，很常见。
explain select * from t_order where user_id in (100,200,300);

index
在进行统计时非常常见，此联接类型实际上会扫描索引树，仅比ALL快些。
explain select count(*) from t_order;

ALL
完整的扫描全表，最慢的联接类型，尽可能的避免。
explain select * from t_order;
    ref_or_null:该联接类型如同ref,但是添加了MySQL可以专门搜索包含NULL值的行。
    index_merge:该联接类型表示使用了索引合并优化方法。
    unique_subquery:该类型替换了下面形式的IN子查询的ref: value IN (SELECT primary_key FROM single_table WHERE some_expr) unique_subquery是一个索引查找函数,可以完全替换子查询,效率更高。
    index_subquery:该联接类型类似于unique_subquery。可以替换IN子查询,但只适合下列形式的子查询中的非唯一索引: value IN (SELECT key_column FROM single_table WHERE some_expr)

Distinct:MySQL发现第1个匹配行后,停止为当前的行组合搜索更多的行。</li><li>
Using temporary:为了解决查询,MySQL需要创建一个临时表来容纳结果。通常是由于group by和order by采用了不同的列。</li><li>

Using index：使用覆盖索引，可以是非前缀匹配，也就是可以只从索引中获取相应的信息，不需要再搜索原表来检索表中的列信息。
<pre style="font-size:0.8em; face:arial;">
select salesRepEmployeeNumber from customers where salesRepEmployeeNumber=1401;
</pre></li><li>

Using index condition：使用索引需要回表获取数据，仍有可能会全表扫描。
<pre style="font-size:0.8em; face:arial;">
select * from employees e join dept_manager d on e.emp_no = d.emp_no where dept_no = 'd009';
</pre>



    Not exists:MySQL能够对查询进行LEFT JOIN优化,发现1个匹配LEFT JOIN标准的行后,不再为前面的的行组合在该表内检查更多的行。
    range checked for each record (index map: #):MySQL没有发现好的可以使用的索引,但发现如果来自前面的表的列值已知,可能部分索引可以使用。
    Using filesort:MySQL需要额外的一次传递,以找出如何按排序顺序检索行。
    Using where:WHERE 子句用于限制哪一个行匹配下一个表或发送到客户。
    Using sort_union(...), Using union(...), Using intersect(...):这些函数说明如何为index_merge联接类型合并索引扫描。
    Using index for group-by:类似于访问表的Using index方式,Using index for group-by表示MySQL发现了一个索引,可以用来查 询GROUP BY或DISTINCT查询的所有列,而不要额外搜索硬盘访问实际的表。--></li></ul>

###### extra的说明
Distinct
MySQL发现第1个匹配行后,停止为当前的行组合搜索更多的行。对于此项没有找到合适的例子，求指点。

Not exists
因为b表中的order_id是主键，不可能为NULL，所以mysql在用a表的order_id扫描t_order表，并查找b表的行时，如果在b表发现一个匹配的行就不再继续扫描b了，因为b表中的order_id字段不可能为NULL。这样避免了对b表的多次扫描。
explain select count(1) from t_order a left join t_order_ext b on a.order_id=b.order_id where b.order_id is null;

Range checked for each record
这种情况是mysql没有发现好的索引可用，速度比没有索引要快得多。
explain select * from t_order t, t_order_ext s where s.order_id>=t.order_id and s.order_id<=t.order_id and t.express_type>5;

-->

详细内容可以参考 [MySQL Explain](/reference/databases/mysql/mysqlexplain.pdf) 中的介绍。










### 源码解析

EXPLAIN 相关的源码在 sql/opt_explain.cc 文件中，主要处理流程如下。

{% highlight text %}
mysql_parse()
 |-mysql_execute_command()
   |-execute_sqlcom_select()
     |-handle_query()
       |-explain_query()
{% endhighlight %}



## 配置参数

优化器的搜索空间越小，实际上对应的优化时间越小；同时可能会由于忽略了一些方案导致失去了最佳方案。对于搜索空间，可以通过如下的两个系统变量来控制：

* optimizer_prune_level [1]

    用来告诉优化器,可以通过评估每个表被访问记录的数量来忽略某些方案. 经验显示,这种类型的"学习猜想(educated guess)",很少会错失掉最佳方案,并会很明显地减少查询编译次数.这也是为什么MYSQL默认会设置:optimizer_prune_level=1的原因. 当然我们也可以将optimizer_prune_level=0,但需要承担编译查询可能会持续很长时间的危险.

* optimizer_search_depth [62]

    用来告诉优化器,对一个不完整方案的最大评估深度是多少.其值越小,那么查询编译的次数(时间)就会越少.如果此值为0,那么优化器会自动决定其值.


如果 search_depth = 0 ，那么实际上是由 MySQL 决定搜索深度的，通过 determine_search_depth() 实现，实际上就是 min(number of tables, 7)。另外，需要主要注意的是，表的数量是不包含常量表的。

实际上，在 MySQL 中，当超过了 7 个表的 JOIN 之后，优化器所带来的成本就非常高了，如 (7!=5040, 8!=40320, 9!=362 880)

<!--
http://lists.mysql.com/internals/37635   对search_depth的讲解
-->










<br><br><h2>Index Merge Optimization 索引合并优化</h2><p>
5.0 之前，每条单表查询只能使用一个索引，之后推出的索引合并优化，可以让 MySQL 在查询中对一个表使用多个索引，对它们同时扫描，并且合并结果。需要注意的是，该优化只能用于单表。<br><br>

对于该类型的优化，可以通过 explain 查看 type 字段为 index_merge，在 Extra 中显示具体的类型，包括了三种：unions、intersections、unions-of-intersections；详细可以参考 <a href="http://dev.mysql.com/doc/refman/5.7/en/index-merge-optimization.html">Index Merge Optimization</a> 。
<pre style="font-size:0.8em; face:arial;">
mysql> create index idx_firstname on employees(first_name);
mysql> create index idx_lastname on employees(last_name);
</pre>
<ol><li>
The Index Merge Intersection Access Algorithm<br>
不同的索引的 range 查询，并通过 AND 链接。
<pre style="font-size:0.8em; face:arial;">
mysql> explain select * from employees where first_name = 'Georgi' and last_name = 'Facello';
</pre></li><br><li>

The Index Merge Union Access Algorithm<br>
不同的索引的 range 查询，并通过 OR 链接。
<pre style="font-size:0.8em; face:arial;">
mysql> explain select * from employees where first_name = 'Georgi' or last_name = 'Facello';
</pre></li><br><li>


The Index Merge Sort-Union Access Algorithm<br>
</li></ol>
<!--
二 使用场景
Index merge算法有 3 种变体：例子给出最基本常见的方式：
2.1 对 OR 取并集
In this form, where the index has exactly N parts (that is, all index parts are covered):
1 key_part1=const1 AND key_part2=const2 ... AND key_partN=constN
2 Any range condition over a primary key of an InnoDB table.
3 A condition for which the Index Merge method intersection algorithm is applicable.
root@odbsyunying 02:34:41>explain  select count(*) as cnt from `order` o  WHERE  o.order_status = 2  or  o.buyer_id=1979459339672858 \G
*************************** 1. row ***************************
           id: 1
  select_type: SIMPLE
        table: o
         type: index_merge
possible_keys: buyer_id,order_status
          key: order_status,buyer_id
      key_len: 1,9
          ref: NULL
         rows: 8346
        Extra: Using union(order_status,buyer_id); Using where
1 row in set (0.00 sec)
当 where 条件中 含有对两个索引列的 or 交集操作时 ，执行计划会采用 union merge 算法。


2.2 对 AND 取交集：
”In this form, where the index has exactly N parts (that is, all index parts are covered):
key_part1=const1 AND key_part2=const2 ... AND key_partN=constN
Any range condition over a primary key of an InnoDB table.“
root@odbsyunying 02:33:59>explain  select count(*) as cnt from `order` o  WHERE  o.order_status = 2  and o.buyer_id=1979459339672858 \G
*************************** 1. row ***************************
           id: 1
  select_type: SIMPLE
        table: o
         type: index_merge
possible_keys: buyer_id,order_status
          key: buyer_id,order_status
      key_len: 9,1
          ref: NULL
         rows: 1
        Extra: Using intersect(buyer_id,order_status); Using where; Using index
1 row in set (0.00 sec)
当where条件中含有索引列的and操作时，执行计划会采用intersect 并集操作。

2. 3 对 AND 和 OR 的组合取并集。
root@odbsyunying 02:42:19>explain  select count(*) as cnt from `order` o  WHERE  o.order_status > 2  or  o.buyer_id=1979459339672858 \G
*************************** 1. row ***************************
           id: 1
  select_type: SIMPLE
        table: o
         type: index_merge
possible_keys: buyer_id,order_status
          key: order_status,buyer_id
      key_len: 1,9
          ref: NULL
         rows: 4585
        Extra: Using sort_union(order_status,buyer_id); Using where
1 row in set (0.00 sec)

The difference between the sort-union algorithm and the union algorithm is that the sort-union algorithm must first fetch row IDs for all rows and sort them before returning any rows.

三 Index merge的 限制
MySQL在5.6.7之前，使用index merge有一个重要的前提条件：没有range可以使用。这个限制降低了MySQL index merge可以使用的场景。理想状态是同时评估成本后然后做出选择。因为这个限制，就有了下面这个已知的bad case ：
SELECT * FROM t1 WHERE (goodkey1 < 10 OR goodkey2 < 20) AND badkey < 30;
优化器可以选择使用goodkey1和goodkey2做index merge，也可以使用badkey做range。因为上面的原则，无论goodkey1和goodkey2的选择度如何，MySQL都只会考虑range，而不会使用index merge的访问方式。这是一个悲剧...（5.6.7版本针对此有修复)
-->


get_key_scans_params() -> check_quick_select() -> sel_arg_range_seq_next() -> is_key_scan_ror() 用于判断是否为 Rowid-Ordered Retrieval, ROR
</p>














<!--
*************************** 3. row ***************************
Variable_name: optimizer_selectivity_sampling_limit
Value: 100
*************************** 4. row ***************************
Variable_name: optimizer_switch
Value: index_merge=on,index_merge_union=on,index_merge_sort_union=on,index_merge_intersection=on,index_merge_sort_intersection=off,engine_condition_pushdown=off,index_condition_pushdown=on,derived_merge=on,derived_with_keys=on,firstmatch=on,loosescan=on,materialization=on,in_to_exists=on,semijoin=on,partial_match_rowid_merge=on,partial_match_table_scan=on,subquery_cache=on,mrr=off,mrr_cost_based=off,mrr_sort_keys=off,outer_join_with_cache=on,semijoin_with_cache=on,join_cache_incremental=on,join_cache_hashed=on,join_cache_bka=on,optimize_join_buffer_size=off,table_elimination=on,extended_keys=on,exists_to_in=on
*************************** 5. row ***************************
Variable_name: optimizer_use_condition_selectivity
1
-->


## Profile

这一参数可以在全局和 session 级别来设置，该参数开启后，后续执行的 SQL 语句都将记录其资源开销，常见的有 IO、上下文切换、CPU、Memory 等，可以通过 help profile 查看帮助。

{% highlight text %}
----- 查看profile是否开启
mysql> show variables like '%profiling%';
+------------------------+-------+
| Variable_name          | Value |
+------------------------+-------+
| have_profiling         | YES   |   # 只读变量，是否支持profiling
| profiling              | OFF   |   # 是否开启了语句刨析功能
| profiling_history_size | 15    |   # 保留剖析结果的数目，默认为15[0~100]，为0时禁用
+------------------------+-------+
3 rows in set (0.00 sec)

----- 打开会话级的剖析功能
mysql> set profiling = on;
Query OK, 0 rows affected (0.00 sec)

----- 关闭会话级的剖析功能
mysql> set profiling = off;
Query OK, 0 rows affected (0.00 sec)

----- 查看当前会话保存的profiling记录
mysql> show profiles;
+----------+-------------+--------------------------------------------------+
| Query_ID | Duration    | Query                                            |
+----------+-------------+--------------------------------------------------+
| 1        | 11.24294018 | select emp_no from salaries where salary = 90930 |
+----------+-------------+--------------------------------------------------+
1 rows in set (0.00 sec)

----- 列出具体消耗时间的详细信息，1 表示query_id
mysql> show profile for query 1;
+----------------------+-----------+
| Status               | Duration  |
+----------------------+-----------+
| starting             | 0.000095  |
| checking permissions | 0.000008  |
| Opening tables       | 0.000034  |
| After opening tables | 0.000007  |
| System lock          | 0.000006  |
| Table lock           | 0.000011  |
| init                 | 0.000020  |
| optimizing           | 0.000017  |
| statistics           | 0.000023  |
| preparing            | 0.000025  |
| executing            | 0.000003  |
| Sending data         | 11.242564 |
| end                  | 0.000014  |
| query end            | 0.000012  |
| closing tables       | 0.000005  |
| Unlocking tables     | 0.000017  |
| freeing items        | 0.000013  |
| updating status      | 0.000059  |
| cleaning up          | 0.000009  |
+----------------------+-----------+
19 rows in set (0.00 sec)

----- 其它的还支持查看cpu、memory、block io等方式查看，如下为查看CPU
mysql> show profile cpu for query 1;
{% endhighlight %}

<!--
SELECT STATE, SUM(DURATION) AS Total_R,ROUND( 100 * SUM(DURATION) / (SECT SUM(DURATION)
-> FROM INFORMATION_SCHEMA.PROFILING  WHERE QUERY_ID = 1), 2) AS Pct_R, COT(*) AS Calls,SUM(DURATION) / COUNT(*) AS "R/Call"
-> FROM INFORMATION_SCHEMA.PROFILING  WHERE QUERY_ID = 1 GROUP BY STATE  ORER BY Total_R DESC;
-->




## Optimizer Trace

在 MySQL5.6 中，支持将执行的 SQL 的查询计划树记录下来，该功能可以通过一个会话级的参数 optimizer_trace 进行控制，默认是关闭的，因为这会影响查询的性能。

{% highlight text %}
----- 1. 查看相关变量是否打开，如果没有则打开
mysql> SHOW VARIABLES LIKE 'optimizer_trace%';
mysql> SET optimizer_trace="enabled=on";

----- 2. 执行一些查询，注意：只保留最后一次的结果
... ...

mysql> SELECT * FROM INFORMATION_SCHEMA.OPTIMIZER_TRACE;   # 查询结果，或者如下导入到文件中
mysql> SELECT TRACE INTO DUMPFILE "xx.trace" FROM INFORMATION_SCHEMA.OPTIMIZER_TRACE;
{% endhighlight %}

注意，上述的 dumpfile 会将该文件 dump 到该库所在的目录下。

常见的相关参数如下。

* optimizer_trace，开关控制。

    有两个字段，"enabled=on,one_line=off"，前者表示是否打开 optimizer_trace，后者表示打印的查询计划是以一行显示，还是以 json 树的形式显示，默认为 json 数显示。

* optimizer_trace_limit、optimizer_trace_offset，保存以及显示的记录数。

    前者是正整数，后者为整数，默认值为 1/-1，也就是只会保存并显示一条记录；注意，重设变量会导致 trace 被清空。

* optimizer_trace_max_mem_size，内存使用大小。

    trace 数据会存储在内存中，通过该参数设置最大的内存使用数；该参数是 session 级别，不应该设置的过大，而且不应该超过 OPTIMIZER_TRACE_MAX_MEM_SIZE，默认是 16K。

* optimizer_trace_features，控制打印查询计划树的选项。

    当不关心某些查询计划选项时，可以将其关闭掉，只打印关注的，这样可以减小查询计划树的输出。


### 源码解析

在源码中，通过 trace_wrapper()、trace_prepare()、trace_steps() 等函数实现。如下是一个很简单的查询的执行计划 select * from customers limit 100, 2; 。


{% highlight text %}
{
  "steps": [
    {"join_preparation": {                                       在JOIN::prepare()中
        "select#": 1,
        "steps": [
          {
            "expanded_query": "/* select#1 */..."
          }
        ]
      }
    },

    {"join_optimization": {                                      在JOIN::optimize()中
        "select#": 1,
        "steps": [
          {
            "table_dependencies": [
              {
                "table": "`customers`",
                "row_may_be_null": false,
                "map_bit": 0,
                "depends_on_map_bits": [
                ]
              }
            ]
          },
          {
            "rows_estimation": [
              {
                "table": "`customers`",
                "table_scan": {
                  "rows": 122,
                  "cost": 3
                }
              }
            ]
          },
          {
            "considered_execution_plans": [
              {
                "plan_prefix": [
                ],
                "table": "`customers`",
                "best_access_path": {
                  "considered_access_paths": [
                    {
                      "access_type": "scan",
                      "rows": 122,
                      "cost": 27.4,
                      "chosen": true
                    }
                  ]
                },
                "cost_for_plan": 27.4,
                "rows_for_plan": 122,
                "chosen": true
              }
            ]
          },
          {
            "attaching_conditions_to_tables": {
              "original_condition": null,
              "attached_conditions_computation": [
              ],
              "attached_conditions_summary": [
                {
                  "table": "`customers`",
                  "attached": null
                }
              ]
            }
          },
          {
            "refine_plan": [
              {
                "table": "`customers`",
                "access_type": "table_scan"
              }
            ]
          }
        ]
      }
    },

    {
      "join_execution": {
        "select#": 1,
        "steps": [
        ]
      }
    }
  ]
}
{% endhighlight %}

可以查看官方的介绍 [Tracing the Optimizer](http://dev.mysql.com/doc/internals/en/optimizer-tracing.html)，以及其中的示例。
















## 参考

关于 MySQL 的大部分耗时都消耗在那里了，以及作者是如何通过 HandlerSocket 插件大幅度提高访问性能的，可以参考 [Using MySQL as a NoSQL - A story for exceeding 750,000 qps on a commodity server](http://yoshinorimatsunobu.blogspot.com/2010/10/using-mysql-as-nosql-story-for.html)，或者直接参考 [本地版本](/reference/mysql/Using MySQL as a NoSQL.mhtml) 。


<!--
https://dev.mysql.com/doc/refman/5.7/en/order-by-optimization.html

Chapter 12 How MySQL Performs Different Selects
https://dev.mysql.com/doc/internals/en/selects.html
-->

{% highlight text %}
{% endhighlight %}
