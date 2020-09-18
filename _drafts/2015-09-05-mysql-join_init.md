---
title: MySQL 笛卡尔积
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,笛卡尔积,join
description: 笛卡尔积实际上就是等值连接操作，MySQL 最初采用的是 Nested-Loop Join，也就是简单的嵌套循环查询，不过针对这种方式，进行了很多优化，在此就看看与 JOIN 相关的一些操作。
---

笛卡尔积实际上就是等值连接操作，MySQL 最初采用的是 Nested-Loop Join，也就是简单的嵌套循环查询，不过针对这种方式，进行了很多优化，在此就看看与 JOIN 相关的一些操作。

<!-- more -->

## 简介

关系数据库中的查询中，JOIN 是将两个数据集合按照某个条件进行合并形成新的数据集合的操作，其理论基础是关系代数，由一个迪卡尔积运算和一个选择运算构成。

连接类型主要有两大类：内连接 (INNER JOIN, 也称为自然连接) 和外连接 (OUTER JOIN)；其中外连接又包括左外连接 (LEFT OUTER JOIN)、右外连接 (RIGHT OUTER JOIN) 和全外连接 (FULL OUTER JOIN)，不过 MySQL 不支持全外连接。

另外，连接条件有三种：自然连接(NATURAL JOIN)、条件连接(ON 谓词条件)和指定属性的连接(USING 属性)。自然连接就是以两个集合的公共属性作为条件进行等值连接，也就是对于 A NATURAL JOIN B 来说，等价于 A JOIN B USING(A1, A2)，其中 A1、A2 是 A 和 B 的所有公共属性。还有一种连接类型叫交叉连接 (CROSS JOIN)，它等价于无连接条件的 NATURAL INNER JOIN。

![mysql joins]({{ site.url }}/images/databases/mysql/joins.svg "mysql joins"){: .pull-center width="90%" }

INNER JOIN 又称为等值连接，实际是笛卡尔积；LEFT JOIN 和 RIGHT JOIN 在概念上基本相同，只是 LEFT JOIN 会显示左侧的所有数据，如果存在不符合条件的记录则右侧的为 NULL ，通过这一特征可以确定右数据表中缺少了那些数据行 (注意，此时要求判断右行本身为 NOT NULL)。

在 MySQL 中没有 FULL JOIN 的概念，但是可以将 LEFT/RIGHT 的结果做 UNION 操作；另外，在 MySQL 中，INNER JOIN、CROSS JOIN、JOIN 的操作相同。


### 示例

假设我们有两张表，Table A 是左边的表，Product；Table B 是右边的表，Product Details；通过如下命令创建示例表：

{% highlight text %}
CREATE TABLE tablea (
     id int(10) unsigned NOT NULL auto_increment,
     amount int(10) unsigned default NULL,
     PRIMARY KEY (id)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
CREATE TABLE tableb (
     id int(10) unsigned NOT NULL,
     weight int(10) unsigned default NULL,
     exist int(10) unsigned default NULL,
     PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO tablea (id,amount) VALUES (1,100),(2,200),(3,300),(4,400);
INSERT INTO tableb (id,weight,exist) VALUES (2,22,0),(4,44,1),(5,55,0),(6,66,1);

 id  amount     id  weight  exist
 --  ------     --  ------  -----
  1     100      2      22      0
  2     200      4      44      1
  3     300      5      55      0
  4     400      6      66      1
{% endhighlight %}

INNER JOIN 、CROSS JOIN、JOIN 以及 ',' 功能相似，需要注意的是 ',' 不能使用 ON 和 USING 子句。下面是简单的示例:

{% highlight text %}
----- 其中tablea为内循环，tableb为外循环
mysql> SELECT * FROM tablea INNER JOIN tableb;
+----+--------+----+--------+-------+
| id | amount | id | weight | exist |
+----+--------+----+--------+-------+
|  1 |    100 |  2 |     22 |     0 |
|  2 |    200 |  2 |     22 |     0 |
|  3 |    300 |  2 |     22 |     0 |
|  4 |    400 |  2 |     22 |     0 |
|  1 |    100 |  4 |     44 |     1 |
|  2 |    200 |  4 |     44 |     1 |
|  3 |    300 |  4 |     44 |     1 |
|  4 |    400 |  4 |     44 |     1 |
|  1 |    100 |  5 |     55 |     0 |
|  2 |    200 |  5 |     55 |     0 |
|  3 |    300 |  5 |     55 |     0 |
|  4 |    400 |  5 |     55 |     0 |
|  1 |    100 |  6 |     66 |     1 |
|  2 |    200 |  6 |     66 |     1 |
|  3 |    300 |  6 |     66 |     1 |
|  4 |    400 |  6 |     66 |     1 |
+----+--------+----+--------+-------+
16 rows in set (0.00 sec)


----- 等值JOIN，两者结果集相同，只是USING会自动合并一个id，而且使用USING时列名需相同
mysql> SELECT * FROM tablea a INNER JOIN tableb b ON a.id = b.id;
+----+--------+----+--------+-------+
| id | amount | id | weight | exist |
+----+--------+----+--------+-------+
|  2 |    200 |  2 |     22 |     0 |
|  4 |    400 |  4 |     44 |     1 |
+----+--------+----+--------+-------+
2 rows in set (0.00 sec)

MYSQL>  SELECT * FROM tablea a INNER JOIN tableb b USING(id);
+----+--------+--------+-------+
| id | amount | weight | exist |
+----+--------+--------+-------+
|  2 |    200 |     22 |     0 |
|  4 |    400 |     44 |     1 |
+----+--------+--------+-------+
2 rows in set (0.00 sec)


----- 首先确认tableb.id均为非NULL；然后利用左连接，查找a在b中不存在的id
mysql> SELECT * FROM tableb where id is NULL;
Empty set (0.00 sec)

mysql> SELECT * FROM tablea a LEFT JOIN tableb b ON a.id = b.id where b.id is NULL;
+----+--------+------+--------+-------+
| id | amount | id   | weight | exist |
+----+--------+------+--------+-------+
|  1 |    100 | NULL |   NULL |  NULL |
|  3 |    300 | NULL |   NULL |  NULL |
+----+--------+------+--------+-------+
2 rows in set (0.00 sec)


----- 使用自连接
mysql> SELECT a.id, b.id FROM tablea a INNER JOIN tablea b WHERE a.id > b.id;
+----+----+
| id | id |
+----+----+
|  2 |  1 |
|  3 |  1 |
|  3 |  2 |
|  4 |  1 |
|  4 |  2 |
|  4 |  3 |
+----+----+
6 rows in set (0.01 sec)

----- 获取FULL JOIN
mysql> SELECT * FROM tablea a LEFT JOIN tableb b ON a.id = b.id WHERE b.id IS NULL
       UNION
       SELECT * FROM tablea a RIGHT JOIN tableb b ON a.id = b.id WHERE a.id IS NULL;
+------+--------+------+--------+-------+
| id   | amount | id   | weight | exist |
+------+--------+------+--------+-------+
| 1    |   100  | NULL |   NULL |  NULL |
| 3    |   300  | NULL |   NULL |  NULL |
| NULL |   NULL | 5    |   55   |  0    |
| NULL |   NULL | 6    |   66   |  1    |
+------+--------+------+--------+-------+
4 rows in set (0.01 sec)
{% endhighlight %}

在 MySQL 中，尽量用 INNER JOIN 避免 LEFT JOIN 和 NULL。

### ON vs. WHERE

在 MySQL 中，ON 子句与 WHERE 子句是有所区别的，"A LEFT JOIN B ON 条件表达式" 中的 ON 用来决定如何从 B 表中检索数据行。如果 B 表中没有任何一行数据匹配 ON 的条件，将会额外生成一行所有列为 NULL 的数据。

在匹配阶段 WHERE 子句的条件都不会被使用，仅在匹配阶段完成以后，WHERE 子句条件才会被使用，它将从匹配阶段产生的数据中检索过滤。

可以通过如下的例子查看 ON 子句和 WHERE 子句的区别。

{% highlight text %}
mysql> SELECT * FROM tablea LEFT JOIN tableb ON (tablea.id = tableb.id) AND tableb.id=2;
+----+--------+------+--------+-------+
| id | amount | id   | weight | exist |
+----+--------+------+--------+-------+
|  1 |    100 | NULL |   NULL |  NULL |
|  2 |    200 |    2 |     22 |     0 |
|  3 |    300 | NULL |   NULL |  NULL |
|  4 |    400 | NULL |   NULL |  NULL |
+----+--------+------+--------+-------+
4 rows in set (0.00 sec)

mysql> SELECT * FROM tablea LEFT JOIN tableb ON (tablea.id = tableb.id) WHERE tableb.id=2;
+----+--------+----+--------+-------+
| id | amount | id | weight | exist |
+----+--------+----+--------+-------+
|  2 |    200 |  2 |     22 |     0 |
+----+--------+----+--------+-------+
1 row in set (0.01 sec)
{% endhighlight %}

第一条查询使用 ON 条件决定了从 LEFT JOIN 的 tableb 表中检索条件符合 (两个条件都需要符合) 的所有数据行，不符合条件的右侧显示为NULL。

第二条查询做了简单的 LEFT JOIN ，然后使用 WHERE 子句从 LEFT JOIN 的数据中过滤掉不符合条件的数据行。

{% highlight text %}
mysql> SELECT * FROM tablea LEFT JOIN tableb ON tablea.id = tableb.id AND tablea.amount=100;
+----+--------+------+--------+-------+
| id | amount | id   | weight | exist |
+----+--------+------+--------+-------+
|  1 |    100 | NULL |   NULL |  NULL |
|  2 |    200 | NULL |   NULL |  NULL |
|  3 |    300 | NULL |   NULL |  NULL |
|  4 |    400 | NULL |   NULL |  NULL |
+----+--------+------+--------+-------+
4 rows in set (0.00 sec)
{% endhighlight %}

所有来自 tablea 表的数据行都被检索到了，但没有在 tableb 表中匹配到记录 (tablea.id = tableb.id AND product.amount=100 条件并没有匹配到任何数据，同上需要满足所有的条件)。

<!-- http://blog.csdn.net/taylor_tao/article/details/7068511 -->

## 实现

数据库中 JOIN 操作的实现主要有三种：嵌套循环连接 (Nested Loop Join)、归并连接 (Merge Join) 和哈希连接 (Hash Join)。其中嵌套循环连接又视情况又有两种变形：块嵌套循环连接和索引嵌套循环连接。

在 MySQL 中，采用的是 Nested Loop Join，也就是通过驱动表的结果集作为循环基础数据，然后一条一条的通过该结果集中的数据作为过滤条件到下一个表中查询数据，然后合并结果；如果还有第三个，则如此往复。

因此，MySQL 可以用来做一些 "简单" 的分析查询，当数据量比较大时，即使支持 Hash Join 的传统 MPP 架构的关系型数据库也不太合适，这类需求应该交给更为专业的 Hadoop 集群来计算。

> 数仓中使用的 Massively Parallel Processing 模型，是将任务并行的分散到多个服务器和节点上，在每个节点上计算完成后，将各自部分的结果汇总在一起得到最终的结果；与 Hadoop 类似，不过前者使用的是 SQL 后者时 MapReduce 程序。

接下来看看各个方法是如何实现的，例如，有两张表 R 和 S，R 共占有 M 页，S 共占有 N 页。r 和 s 分别代表元组，而 i 和 j 分别代表第 i 或者第 j 个字段，也就是后文提到的 JOIN 字段。




## 原理详解

表连接的方式包括了 join, semi-join, outer-join, anti-join/anti-semi-join；实现方式包括了 nested loop, merge, hash，下面使用简单的 nested loop 介绍。

其他的连接方式还有 cross join，用于获取记录的完整笛卡尔积，不能使用 ON 谓词，通常使用不多。

<ul><li>
inner-join<br>
自然连接。查看申请表中相关学校的信息。
<pre style="font-size:0.8em; face:arial;">
select * from College c inner join Apply a on c.name = a.name;

for x in ( select * from tablea )
    for y in ( select * from tableb )
        if ( x.id == y.id )
            OutPut_Record(x.*, y.*)
        end if
    end loop
end loop
</pre>
内连接满足交换律："A inner join B" 和 "B inner join A" 是相等的。</li><br><li>

outer-join<br>
２ 个数据源键值一一比较，返回相互匹配的；但如果在另外一个 row source 没有找到匹配的也返回记录，不过是 NULL。
<pre style="font-size:0.8em; face:arial;">
select * from tablea a left join tableb b on a.id = b.id;

for x in ( select * from tablea )
    find_flag=false;
    for y in ( select * from tableb)
        if ( x.id == y.id )
            OutPut_Record(x.*, y.*)
            find_flag=true
        end if
    end loop
    if ( find_flag == false )
        OutPut_Record(x.*, null)
    end if
end loop
</pre></li><br><li>

semi-join<br>
从一个表中返回的行与另一个表中数据行进行不完全联接查询，查找到匹配的数据行就返回，不再继续查找。<br><br>

与 join 和 outer join 不同，semi-join 和 anti-semi-join 没有明确的语法实现，可以通过 exits 和 not exits 实现。
<pre style="font-size:0.8em; face:arial;">
select * from Student where exists (select null from Apply where Apply.id = Student.id);
select * from Student where exists (select * from Apply where Apply.id = Student.id);

for x in ( select * from tablea )
    for y in ( select * from tableb)
        if ( x.id == y.id )
            OutPut_Record(x)
            Break;
        end if
    end loop
end loop
</pre></li><br><li>


anti-join<br>
现在要求我们找出还没有申请学校的学生信息
<pre style="font-size:0.8em; face:arial;">
select * from Student where not exists (select * from Apply where Apply.id = Student.id);

for x in ( select * from Student )
    for y in ( select * from Apply )
        if ( x.deptno != y.deptno )
            OutPut_Record(x.dname,y.deptno)
        End if
    end loop
end loop
</pre>
</li></ul>
<!-- http://www.cnblogs.com/rush/archive/2012/03/27/2420246.html -->
</p>



### 其它

尽量避免子查询，而用 JOIN。

STRAIGHT JOIN 完全等同于 INNER JOIN，不过可以实现强制多表的载入顺序，从左到右。默认，JOIN 语法是根据 "哪个表的结果集小，就以哪个表为驱动表" 来决定谁先载入的，而 STRAIGHT JOIN 会强制选择其左边的表先载入。

使用 NATURAL JOIN 时，MySQL 将表中具有相同名称的字段自动进行记录匹配，而这些同名字段类型可以不同。因此，NATURAL JOIN 不用指定匹配条件。








### Nested-Loop Join, NLJ

从第一个表每次读一行数据，传递到一个嵌套循环（处理join中的下一个表）。
这个处理重复次数跟join中涉及的表相同?
例子：
Table   Join Type
t1      range
t2      ref
t3      ALL
逻辑处理：
for each row in t1 matching range {
  for each row in t2 matching reference key {
    for each row in t3 {
      if row satisfies join conditions,
      send to client
    }
  }
}

### Block Nested-Loop Join, BNL

通过缓存外层循环读的行，来降低内层表的读取次数。比如： 10行数据读入到buffer中，
然后buffer被传递到内层循环，内层表读出的每一行都要跟这个缓存的10行依次做对比，
这样就降低了内层表数据的读取次数。
使用条件：
1] join_buffer_size决定了每一个join buffer的大小
2] 只有当join type是 all or index（没有合适的索引，使用全索引或者全表扫描的场景）,
   range的时候才会使用。5.6中，外连接也可以用buffer了。
3] 每一个需要buffer的join都会申请一个独立的buffer，也就是说一个查询可能使用多个join buffer。
4] 第一个非常量表是不会使用join buffer的。
5] join buffer在执行join之前申请，在查询完成后释放。
6] join buffer只保存跟join有关的列，而不是整行

explain 列显示：Using join buffer (Block Nested Loop)

逻辑处理：
for each row in t1 matching range {
  for each row in t2 matching reference key {
    store used columns from t1, t2 in join buffer
    if buffer is full {
      for each row in t3 {
        for each t1, t2 combination in join buffer {
          if row satisfies join conditions,
          send to client
        }
      }
      empty buffer
    }
  }
}

if buffer is not empty {
  for each row in t3 {
    for each t1, t2 combination in join buffer {
      if row satisfies join conditions,
      send to client
    }
  }
}

### Multi-Range Read, MRR

当一个表很大，不能存储到存储引擎的缓存的时候，使用二级索引做范围扫描会引起大量磁盘随机读。
MRR的存在就是为了优化这些随机读。mysql开始只扫描跟行相关的索引和收集key，然后把这些key排序，
最后根据排好序的primary key来从基础表获取数据。  MRR的目的是，降低随机的磁盘IO，替换成相对更
有顺序的IO。
MRR的好处：
1、随机IO转换成顺序IO。
2、批量处理请求

优化场景：
A： MRR可以用来做innodb，myiasm的索引范围扫描和等值join操作。
1、索引元组累积到一个buffer
2、buffer中的元组根据rowid排序
3、根据排序好的索引元组顺序去获取数据行
4、当不需要回表访问的时候，MRR就失去意义了（比如覆盖索引）

当使用MRR的时候 explain出现：Using MRR标志

存储引擎使用read_rnd_buffer_size 的值来确定MRR时的buffer大小。

### Batched Key Access, BKA

当使用索引访问第二个join对象的时候，跟BNL类似，BKA使用一个join buffer

来收集第一个操作对象生成的相关列值。BKA构建好key后，批量传给引擎层做索引
查找。key是通过MRR接口提交给引擎的，这样，MRR使得查询更有效率。
BKA使用join buffer size来确定buffer的大小，buffer越大，访问右侧表就越
顺序。
使用BAK的条件：
SET optimizer_switch='mrr=on,mrr_cost_based=off,batched_key_access=on';













<h3>嵌套循环连接</h3><p>
嵌套循环连接（Nested Loop Join），嵌套循环连接由两个 FOR 循环构成，其思路相当的简单和直接，对于关系 R 的每个元组 r 将其与关系 S 的每个元组 s 在 JOIN 条件的字段上直接比较并筛选出符合条件的元组。

Nest Loop Join的操作过程很简单，很想我们最简单的排序检索算法，两层循环结构。进行连接的两个数据集合（数据表）分别称为外侧表（驱动表）和内测表（被驱动表）。首先处理外侧表中每一行符合条件的数据，之后每一行数据和内测表进行连接匹配操作。最后可以获取到结果集合。

http://database.51cto.com/art/200904/117947.htm
<pre>
foreach tuple r in R do
    foreach tuple s in S do
        if ri == sj then add r,s to result
</pre>
嵌套循环连接比较通用，不需要索引，且不管是什么样的连接条件，该算法都适用。对于任何类型的JOIN，该算法都只需要做稍微的调整就能进行运算。如对于自然连接（Natural Join）该算法只需要在将（X, Y）加入结果之前消除其中的重复属性。<br><br>

但是，嵌套循环连接的代价比较大。因为该算法过程中需要逐一比较R和S中的每一个元组，当数据规模较大而不能完全放入内存中时其引起的磁盘与内存交换比较频繁，即使数据能够完全放入内存，则嵌套循环连接执行过程中也会引起CPU的CACHE命中率低下，从而严重影响系统效率。<br><br>

被联结的表所处内层或外层的顺序对磁盘I/O开销有着非常重要的影响。而CPU开销相对来说影响较小，主要是元组读入内存以后(in-memory)的开销，是 O (n * m)。对于I/O开销，根据 page-at-a-time 的前提条件，I/O cost = M + M * N，翻译一下就是 I/O的开销 = 读取M页的I/O开销 + M次读取N页的I/O开销。<br><br>

使用小结：<ul><li>
    适用于一个集合大而另一个集合小的情况(将小集合做为外循环)，I/O性能不错。</li><li>

    当外循环输入相当小而内循环非常大且有索引建立在JOIN字段上时，I/O性能相当不错。</li><li>

    当两个集合中只有一个在JOIN字段上建立索引时，一定要将该集合作为内循环。</li><li>

    对于一对一的匹配关系(两个具有唯一约束字段的联结)，可以在找到匹配元组后跳过该次内循环的剩余部分(类似于编程语言循环语句中的continue)。
</li></ul>
<br><br>

块嵌套循环连接（Nested Block Loop Join），块嵌套循环连接是采用了分块策略的嵌套循环连接，不过它有四层循环，最外面两层是块循环，里面两层是块内循环。对于关系R和S以及连接条件P，块嵌套循环连接的回想是将R和S分成块，对于逐一连接R和S中的每一块。其算法如下：
<pre>
For each block A in R do
    For each block B in S do
        For each tuple X in A do
            For each tuple Y in B do
                If P(X, Y)
                    Then Result=Result+(X, Y)
            End
        End
    End
End
</pre>
相比嵌套循环连接，块嵌套循环连接在时间复杂度上没有改进，但是由于块循环连接是以块为单位进行处理，减少了磁盘交换次数。需要说明的是，对于外层循环的R而言，它并没有节省磁盘交换次数，关键在于每取S中的一块，处理了R中一块的数据，而不是R中的一条数据。如果一块有N个元组，则块嵌套循环连接节省了N倍的磁盘交换次数。<br><br>

索引嵌套循环连接，对于关系R和S以及连接条件P，则如果S中存在一种索引，无论是临时和还是永久的，对于R中的任一元组X，能够通过该索引找到S中满足条件P的元组，则可以用索引查找替代文件扫描。这样的算法也更加简单：
<pre>
For each tuple T X in R do
    Find all tuples {Y} in S that P(X, Y)
    For each y in {Y} do
        Result=Result+(X,y)
    End
End
</pre>
索引嵌套循环连接在已经有索引的情况下使用，或者为了进行连接操作专门建立一个索引。如果已经有了索引，则其时间复杂度比较小，只需要扫描一个表，然后根据索引查找另一个表。

Nested loops join支持包括相等连接谓词和不等谓词连接在内的所有连接谓词。

Nested loops join支持什么类型的逻辑连接？

Nested loops join支持以下类型的逻辑连接：

* Inner join
* Left outer join
* Cross join
* Cross apply and outer apply
* Left semi-join and left anti-semi-join

Nested loops join不支持以下逻辑连接：

* Right and full outer join
* Right semi-join and right anti-semi-join



为什么Nested loops join 只支持左连接？

我们很容易扩展Nested loops join 算法来支持left outer 和semi-joins.例如，下边是左外连接的伪码。
我们可以写出相似的代码来实现 left semi-join 和 left anti-semi-join.

for each row R1 in the outer table
    begin
        for each row R2 in the inner table
            if R1 joins with R2
                return (R1, R2)
        if R1 did not join
            return (R1, NULL)
    end

这个算法记录我们是否连接了一个特定的外部行。如果已经穷尽了所有内部行,但是没有找到一个
符合条件的内部行,就把该外部行做为NULL扩展行输出。

那么我们为什么不支持right outer join呢。在这里,我们想返回符合条件的行对(R1,R2)
和不符合连接条件的(NULL,R2)。问题是我们会多次扫描内部表-对于外部表的每行都要扫描一次。
在多次扫描过程中我们可能会多次处理内部表的同一行。这样我们就无法来判断某一行到底符合
不符合连接条件。更进一步,如果我们使用index join,一些内部行可能都不会被处理,但是这些行在
外连接时是应该返回的。


幸运的是right outer join可以转换为left outer join,right semi-join可以转换为left semi-join,
所以right outer join和semi-joins是可以使用nested loops join的。但是,当执行转换的时候可能会
影响性能。例如,上边方案中的"Customer left outer join Sales",由于表内部表Sales有聚集索引,所以
我们在连接过程中可以使用索引探寻。如果"Customer right outer join Sales" 转换为 "Sales left outer
join Customer”,我们则需要在Customer表上具有相应的索引了。



full outer joins是什么情况呢？

nested loops join完全支持outer join.我们可以把"T1 full outer join T2"转换为"T1 left outer join T2
UNION T2 left anti-semi-join T1".可以这样来理解,将full outer join转换为一个左连接-包含T1和T2所有的
符合条件的连接行和T1表里没有连接的行,然后加上那些使用anti-semi-join从T2返回的行。下边是转换过程：



select *
from Customers C full outer join Sales S
on C.Cust_Id = S.Cust_Id

Rows Executes


5    1        |--Concatenation

4    1           |--Nested Loops(Left Outer Join, WHERE:([C].[Cust_Id]=[S].[Cust_Id]))

3    1           |    |--Table Scan(OBJECT:([Customers] AS [C]))

12   3           |    |--Clustered Index Scan(OBJECT:([Sales].[Sales_ci] AS [S]))

0    0           |--Compute Scalar(DEFINE:([C].[Cust_Id]=NULL, [C].[Cust_Name]=NULL))

1    1                |--Nested Loops(Left Anti Semi Join, OUTER REFERENCES:([S].[Cust_Id]))

4    1                  |--Clustered Index Scan(OBJECT:([Sales].[Sales_ci] AS [S]))

3    4                  |--Top(TOP EXPRESSION:((1)))

3    4                       |--Table Scan(OBJECT:([Customers] AS [C]), WHERE:([C].[Cust_Id]=[S].[Cust_Id]))


注意：在上边的例子中,优化器并选择了聚集索引扫描而不是探寻。这完全是基于成本考虑而做出的决定。表非常小(只有一页)
所以扫描或探寻并没有什么性能上的区别。



NL join好还是坏？

实际上,并没有所谓"最好"的算法,连接算法也没有好坏之分。每一种连接方式在正确的环境下性能非常好,
而在错误的环境下则非常差。因为nested loops join的复杂度是与驱动表大小和内部表大小乘积成比例的,所以在驱动表比较小
的情况下性能比较好。内部表不需要很小,但是如果非常大的话,在具有高选择性的连接列上建立索引将很有帮助。

一些情况下,Sql Server只能使用nested loops join算法,比如Cross join和一些复杂的cross applies,outer applies,
(full outer join是一个例外)。如果没有任何相等连接谓词的话nested loops join算法是Sql Server的唯一选择。
-->
</p>



<br><h3>归并连接 (Merge Join)</h3><p>
Nested Loop 一般在两个集合都很大的情况下效率就相当差了，而 Sort-Merge 在这种情况下就比它要高效不少，尤其是当两个集合的 JOIN 字段上都有聚集索引 (clustered index) 存在时，Sort-Merge 性能将达到最好。<br><br>

归并连接算法主要用于计算自然连接和等值连接，主要有两个步骤：A) 按 JOIN 字段进行排序；B) 对两组已排序集合进行合并排序，从来源端各自取得数据列后加以比较。<br><br>

假设有关系 R 和 S，则在进行连接之前先让 R 和 S 是有序的，然后分别对两个表进行扫描一遍即可完成。
<pre style="font-size:0.8em; face:arial;">
R、S 中元组按连接属性从小到大排序，i=j=0
while i&lt;len(R) AND j&lt;len(S) do
    while S[j] != R[i] do
        while S[j]&lt;R[i] do
            j=j+1
        end
        while S[j]&gt;R[i] do
            i=i+1
        end
    end
    m=i, n=j
    while S[m]=S[i] do
        M=m+1
    end
    while R[n]=R[j] do
        N=n+1
    end
    for each tuple X from R[i] to R[m - 1] do
        for each tuple Y from S[j] to S[n-1] do
            Result=Result+(X, Y)
        end
    end
end
</pre>
归并连接执行起来非常高效，其时间复杂度是线性的O(n)，其中 n 为 R 和 S 中元组数最多的那个关系的元组个数。但是它只能进行等值连接和自然连接，对于其它谓词的连接显得力不从心，并且还要求连接之前对元组进行全排序。
</p>


<br><h3>散列连接 (Hash Join)</h3><p>
<!--类似归并连接，散列连接可用于实现等值连接和自然连接。在散列连接算法中，散列函数H用于对两个关系进行划分。此算法的基本思想是把两个关系按连接属性划分成具有相同散列值的元组集合。

    假设H是将JoinAttrs值映射到{0，1，2…，max}的散列函数，并且具有良好的随机性和均匀性，其中JoinAttrs表示关系R与S中的公共属性组，或者连接属性组。Hr1，Hr2，…，Hrmax表示关系R的元组划分，Hs1，Hs2，…，Hsmax表示关系S的元组划分，则散列连接算法的思想如下：如果关系R的一个元组与关系S的一个元组满足连接条件，那么它们在连接属性上有相同的值。若该元组在函数H中映射为i，则关系R的那个元组必在Hri中，关系S的那个元组必在His中。因此，Hri中的元组只需与His中的元组进行比较，而无需与S中其它元组进行比较。

    散列连接不需要索引，并且与循环嵌套连接相比，散列连接更容易处理大结果集，唯一的遗憾就是它只能用作等值连接。

    Hash Join在本质上类似于两列都有重复值时的Sort-Merge的处理思想——分区(patitioning)。但它们也有区别：Hash Join通过哈希来分区(每一个桶就是一个分区)而Sort-Merge通过排序来分区(每一个重复值就是一个分区)。 
      值得注意的是，Hash Join与上述两种算法之间的较大区别同时也是一个较大限制是它只能应用于等值联结(equality join)，这主要是由于哈希函数及其桶的确定性及无序性所导致的。
    算法： 
      基本的Hash Join算法由以下两步组成： 
      (1) Build Input Phase： 基于JOIN字段，使用哈希函数h2为较小的S集合构建内存中(in-memory)的哈希表，相同键值的以linked list组成一个桶(bucket) 
      (2) Probe Input Phase： 在较大的R集合上对哈希表进行核对以完成联结。其中核对操作包括
      foreach tuple r Î R do    
    hash on the joining attribute using the hash function of step 1 to find a bucket in the hash table  
      if the bucket is nonempty  
      foreach tuple s in the found bucket    if ri == sj then add to result 
      代价： 
      值得注意的是对于大集合R的每个元组 r ，hash bucket中对应 r 的那个bucket中的每个元组都需要与 r进行比较，这也是算法最耗时的地方所在。 
      CPU开销是O (m + n * b) b是每个bucket的平均元组数量。 
      使用小结： 
      一般来说，查询优化器会首先考虑Nested Loop和Sort-Merge，但如果两个集合量都不小且没有合适的索引时，才会考虑使用Hash Join。 
      Hash Join也用于许多集合比较操作，inner join、left/right/full outer join、intersect、difference等等，当然了，需要保证都是等值联结。 

另外，Hash Join的变种能够移除重复和进行分组，它只使用一个输入，兼做Build和Probe的角色。 
   其实产品级的优化器一般都改进了这些基本算法，而改进过的版本的确有较大的性能提升。在这里只是给需要判断执行计划优劣或者研究查询优化器实现的兄弟提供原理方面的介绍，在实际应用中我们还得结合丰富的statistics作出准确的判断。 
-->
</p>





JOIN实现实例

本部分主要调研了市场主流数据库对于连接的实现方式。在介绍这些内容之前，首先分析一下索引对于JOIN操作的影响。

对于嵌入循环连接，索引有可能有用，但在归并连接和散列连接中，有无索引对JOIN操作并没有任何影响。

再总结一下各种JOIN适用的场景。嵌套循环连接比较通用，可用于任何类型的连接。而归并连接和歼列连接只能用于等值连接和自然连接。
MYSQL

在MYSQL中，只有一种JOIN算法，就是最简单最通用的嵌套循环连接。它没有散列连接和排序归并连接。MYSQL中JOIN操作操作最好避免，特别是对于大表的JOIN，速度特别低下。

但是对于效率，MYSQL社区谈到了HASH JOIN，但却把JOIN效率的提高寄希望于INTEL的SSD技术。即半导体硬盘带来的速度提高。
ORACLE

Oracle数据库中对三种连接算法都有实现，并分不同情况分别使用不同的JOIN算法。

嵌入循环连接是最慢的算法，只有在不得已时才使用。索引能够有效地提高其速度。

归并连接比嵌入循环连接快的多，但它也不是Oracle优先选择的算法。

Oracle Hash join 是一种非常高效的join 算法，主要以CPU(hash计算)和内存空间(创建hash table)为代价获得最大的效率。Hash join一般用于大表和小表之间的连接，我们将小表构建到内存中，称为Hash cluster，大表称为probe表。
SQL SERVER 7.0/2000

Microsoft SQL Server 7.0/2000的JOIN操作有三种类型：Nested-Loop Join， Merge Join和 Hash Join。

嵌套循环连接

Nested-Loop操作从关联的两个table中，选择一个作为外层循环，为每一条记录在另一个table中循环查找匹配的结果。作为外层循环的table为outer table，内层循环的table为inner table。在执行计划中，不管是图形还是文本显示方式，outer table位于上方，inner table位于下方。 SQL Server在自动选择join type时，大多数情况下使用Nested-Loop join的条件是：关联的两个Table中，有一个数据量比较小（记录数在2000左右以下），另外一个数据量大的Table又有对于关联条件可用的索引。

另外一种情况，假如查询语句类似如下：     SELECT [...] FROM A INNER JOIN B ON A.?=B.? WHERE A.?=? 如果A、B的记录数都是几万到几十万，B中有ON A.?=B.?可用的索引，A中有WHERE A.?=?可用的索引，并且A通过WHERE子句条件的过滤后记录数比较小（2000左右以下），这种情况下仍然会使用Nested-Loop join。但是如果WHERE子句中既包含A也包含B的限制条件，则会选择Merge或Hash join了。从Nested-Loop join的选择条件可以看出，Nested-Loop join的执行非常高效。outer table的记录数很小，因此外层循环次数少；在inner table中搜索匹配记录时使用索引，就算inner table的数据量非常大，搜索也是相当快而有效的。

在查询语句中，可以强制SQL Server使用Nested-Loop方式关联两个table，例如：SELECT A.PONO,B.VCODE,B.VNAME FROM TBLPO A INNER JOIN TBLVENDOR B ON A.VENDORID=B.ID OPTION (LOOP JOIN) 。如果没有十分的把握，让SQL Server Optimizer自动选择join type。Optimizer总是尝试合理的确定inner talbe和outer table。通常情况下总是选择有可用索引的一个作为inner table，即使这个table的数据量可能会比另外一个多。如果两个table都没有可用索引，则选择数据量较小的一个作为outer table。这种情况下，如果数据量大的table记录数太多或内存有限，无法将inner table的数据全部读入内存中，则SQL Server会尝试将数据量小的作为inner table，以使inner table的数据全部驻留内存中，提高inner table循环的速度。在进行Nested-Loop join操作时，SQL Server Optimizer可能会对inner table进行一次排序，以提高对inner table搜索的速度
</p>



























<h2>代码实现</h2><p>
入口函数为 handle_select()@sql/sql_select.cc，首先对union进行判断，如果查询SQL中不包含union关键字，函数直接执行mysql_select()函数处理；否则，将执行mysql_union()函数。<br><br>

mysql_select()函数是单个select查询的“入口函数”，该函数执行SQL优化的一些列操作。区别于mysql_select()函数，mysql_union()函数逐个执行union中单个select查询优化的一系列操作。执行SQL优化的过程调用相同的函数实现，仅在执行前期的处理过程不一致。<br><br>

在mysql_select()中执行如下的优化顺序，join->prepare()  join->optimize()  join->exec() select_lex->cleanup()。<br><br>

JOIN::optimize(); sql/sql_select.cc<br>
这是整个查询优化器的核心内容，查询优化主要对Join进行简化、优化where条件、优化having条件、裁剪分区partition（如果查询的表是分区表）、优化count()/min()/max()聚集函数、统计join的代价、搜索最优的join顺序、生成执行计划、执行基本的查询、优化多个等式谓词、执行join查询、优化distinct、创建临时表存储临时结果等操作。

<br><br><br>
<br><br><br>
将 outer-join 转换为 inner-join。
<pre style="font-size:0.75em; face:arial;">
SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a WHERE t2.b < 5         转化为如下
SELECT * FROM t1 INNER JOIN t2 ON t2.a=t1.a WHERE t2.b < 5        进一步转化为
SELECT * FROM t1, t2 ON t2.a=t1.a WHERE t2.b < 5 AND t2.a=t1.a
</pre>
<!-- 这篇文章对各种JOIN做了比较好的展示：http://blog.csdn.net/taylor_tao/article/details/7068511 -->
MySQL 解析完 SQL 后，会生成很多 item 类，而 MySQL 是将任何 select 都转换为 JOIN 来处理的。
<pre style="font-size:0.75em; face:arial;">
JOIN::optimize()         @sql/sql_optimizer.cc
  |-flatten_subqueries()                   优化IN子查询为semi-join
  |-handle_derived()                       处理衍生表
  |- ... ...                               对于limit的处理
  |-simplify_joins()                       如果有可能就将outer-join转换为inner-join
  |-record_join_nest_info()
  |-build_bitmap_for_nested_joins()        bitmap?????
  |-optimize_cond()                        优化查询条件
  |   |-build_equal_items()                等价传递
  |   |-propagate_cond_constants()         常量传递
  |   |-remove_eq_conds()                  剪除无效条件
  |
  |-optimize_cond()                        优化where、having查询条件，其中第二个参数为where条件，同上
  |-opt_sum_query()                        优化count(*),min(),max()
  |-make_tmp_tables_info()                 如果需要则构建临时表
  |-get_sort_by_table()                    是否有一个表在order by中，并且和group是相容的?
  |
  |-make_join_statistics()                 1. 计算最好的join顺序以及初始化join结构
  | |-update_ref_and_keys()                1.1 二级索引的ref查询
  | | |-add_key_fields()
  | |   |-add_key_equal_fields()
  | |
  | |-get_quick_record_count()             1.2 range查询优化，计算全表扫描以及possible_keys中代价最小的
  | | |-test_quick_select()
  | |   |-get_mm_tree()                    生成所有查询条件的一棵查询树
  | |   |-get_best_group_min_max()
  | |   |-get_key_scans_params()           获得最优的范围查询计划
  | |   |-get_best_ror_intersect()
  | |   |-get_best_index_intersect()
  | |   |-get_best_disjunct_quick()
  | |
  | |-pull_out_semijoin_tables()
  | |-optimize_semijoin_nests()
  | |-choose_plan()                        1.3 计算最优的join顺序
  |   |-optimize_straight_join()           如果采用STRAIGHT_JOIN的hint
  |   |-greedy_search()                    否则使用贪婪搜索
  |
  |-make_join_select()                     2. 对优化结果再次调整
  |
  |-ORDER_with_src()                       尝试优化distinct,group by,order by等
  |-test_if_subpart()
  |-plan_is_const()
  |
  |-make_join_readinfo()                   3. Plan Refinement Stage 根据表访问的最优路径，选择对应的执行方式。同时，进行索引覆盖扫描优化，将全表扫描转化为索引覆盖扫描。
    |-setup_semijoin_dups_elimination()    关于semi-join的优化
    |-check_join_cache_usage_for_tables()
  |-make_tmp_tables_info()
</pre>



scan_time= (double) records / TIME_FOR_COMPARE + 1;
read_time= (double) head->file->scan_time() + scan_time + 1.1;



<br><br>
range 的解析是通过 get_mm_tree() 函数实现，如果要查看 range 的代码，或者解析后的结果可以在 get_best_group_min_max() 设置断点。
<pre style="font-size:0.75em; face:arial;">
class SEL_ARG :public Sql_alloc


#0  get_mm_leaf ()                       at sql/opt_range.cc:8198
#1  get_mm_parts ()                      at sql/opt_range.cc:8166
#2  get_func_mm_tree ()                  at sql/opt_range.cc:7812
#3  get_full_func_mm_tree ()             at sql/opt_range.cc:7920
#4  get_mm_tree ()                       at sql/opt_range.cc:8107
#5  SQL_SELECT::test_quick_select ()     at sql/opt_range.cc:3125
#6  get_quick_record_count ()            at sql/sql_select.cc:3345
#7  make_join_statistics ()              at sql/sql_select.cc:3953
#8  JOIN::optimize_inner ()              at sql/sql_select.cc:1337
#9  JOIN::optimize ()                    at sql/sql_select.cc:1022
#10 mysql_select ()                      at sql/sql_select.cc:3294
#11 handle_select ()                     at sql/sql_select.cc:373
#12 execute_sqlcom_select ()i            at sql/sql_parse.cc:5274
#13 mysql_execute_command () at sql/sql_parse.cc:2562
#14 mysql_parse () at sql/sql_parse.cc:6529
#15 dispatch_command () at sql/sql_parse.cc:1308
#16 do_command () at sql/sql_parse.cc:999
#17 do_handle_one_connection () at sql/sql_connect.cc:1378
#18 handle_one_connection () at sql/sql_connect.cc:1293
#19 start_thread (arg=0x7f260df64700) at pthread_create.c:308
#20 clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:113




#0  add_key_field (join=0x7f25e1822c90, key_fields=0x7f260df62168, and_level=0, cond=0x7f25e1822a60, field=0x7f25e1856c20,
    eq_func=false, value=0x7f25e1822af8, num_values=1, usable_tables=18446744073709551615, sargables=0x7f260df622b8)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:4355
#1  0x00000000006831fc in add_key_equal_fields (join=0x7f25e1822c90, key_fields=0x7f260df62168, and_level=0, cond=0x7f25e1822a60,
        field_item=0x7f25e18228c8, eq_func=false, val=0x7f25e1822af8, num_values=1, usable_tables=18446744073709551615,
            sargables=0x7f260df622b8) at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:4549
#2  0x0000000000683d2b in add_key_fields (join=0x7f25e1822c90, key_fields=0x7f260df62168, and_level=0x7f260df62174,
                cond=0x7f25e1822a60, usable_tables=18446744073709551615, sargables=0x7f260df622b8)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:4771
#3  0x0000000000685237 in update_ref_and_keys (thd=0x7f25f3bfa070, keyuse=0x7f25e1822f90, join_tab=0x7f25e1823ab8, tables=1,
        cond=0x7f25e1822a60, normal_tables=18446744073709551615, select_lex=0x7f25f3bfe0b8, sargables=0x7f260df622b8)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:5250
#4  0x00000000006805e0 in make_join_statistics (join=0x7f25e1822c90, tables_list=..., conds=0x7f25e1822a60,
        keyuse_array=0x7f25e1822f90) at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:3590
#5  0x0000000000678b74 in JOIN::optimize_inner (this=0x7f25e1822c90)
            at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:1337
#6  0x0000000000677abc in JOIN::optimize (this=0x7f25e1822c90)
                at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:1022
#7  0x000000000067f76e in mysql_select (thd=0x7f25f3bfa070, rref_pointer_array=0x7f25f3bfe330, tables=0x7f25e18222c0, wild_num=1,
                    fields=..., conds=0x7f25e1822a60, og_num=0, order=0x0, group=0x0, having=0x0, proc_param=0x0, select_options=2147748608,
                        result=0x7f25e1822c70, unit=0x7f25f3bfd9c8, select_lex=0x7f25f3bfe0b8)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:3294
#8  0x0000000000675ce6 in handle_select (thd=0x7f25f3bfa070, lex=0x7f25f3bfd900, result=0x7f25e1822c70, setup_tables_done_option=0)
        at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_select.cc:373
#9  0x000000000064a5f7 in execute_sqlcom_select (thd=0x7f25f3bfa070, all_tables=0x7f25e18222c0)
            at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_parse.cc:5274
#10 0x00000000006429ba in mysql_execute_command (thd=0x7f25f3bfa070)
                at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_parse.cc:2562
#11 0x000000000064d172 in mysql_parse (thd=0x7f25f3bfa070,
                    rawbuf=0x7f25e1822088 "select * from employees where emp_no > 1000 limit 1", length=51, parser_state=0x7f260df636a0)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_parse.cc:6529
#12 0x000000000063fb5b in dispatch_command (command=COM_QUERY, thd=0x7f25f3bfa070,
        packet=0x7f25f97eb071 "select * from employees where emp_no > 1000 limit 1", packet_length=51)
    at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_parse.cc:1308
#13 0x000000000063eda0 in do_command (thd=0x7f25f3bfa070)
        at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_parse.cc:999
#14 0x000000000075beb8 in do_handle_one_connection (thd_arg=0x7f25f3bfa070)
            at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_connect.cc:1378
#15 0x000000000075bbfd in handle_one_connection (arg=0x7f25f3bfa070)
                at /home/andy/Workspace/databases/mariadb/mariadb-10.0.20/sql/sql_connect.cc:1293
#16 0x00007f260d96ddc5 in start_thread (arg=0x7f260df64700) at pthread_create.c:308
#17 0x00007f260c62828d in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:113

</pre>





<ul><li>
simplify_joins();  sql/sql_select.cc<br>
simplify_joins()函数简化join操作。该函数将可以用内连接替换的外连接进行替换，并对嵌套join使用的表、非null表、依赖表等进行计算。该函数的实现是一个递归过程，在递归中，所有的特征将被计算，所有可以被替换的外连接被替换，所有不需要的括号也被去掉。此外，该函数前的注释中，给出了多个例子，用于解释外连接替换为内连接的一些特征。
</li></ul>
Sample 1:
SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a WHERE t2.b < 5                   ==>
SELECT * FROM t1 INNER JOIN t2 ON t2.a=t1.a WHERE t2.b < 5                ==>
SELECT * FROM t1, t2 WHERE t2.b < 5 AND t2.a=t1.a
Sample 2:
SELECT * from t1 LEFT JOIN (t2, t3) ON t2.a=t1.a t3.b=t1.b WHERE t2.c < 5    ==>
SELECT * FROM t1, (t2, t3) WHERE t2.c < 5 AND t2.a=t1.a t3.b=t1.b
Sample 3:
SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a LEFT JOIN t3 ON t3.b=t2.b WHERE t3 IS NOT NULL
==>
SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a, t3 WHERE t3 IS NOT NULL AND t3.b=t2.b
==>
SELECT * FROM t1, t2, t3 WHERE t3 IS NOT NULL AND t3.b=t2.b AND t2.a=t1.a

build_bitmap_for_nested_joins();  sql/sql_select.cc
函数为每个嵌套join在bitmap中分配一位，该函数也是一个递归过程，返回第一个没有使用的bit。

optimize_cond()：(sql/sql_select.cc：9405)
分别对where条件和having条件建立多个等价谓词，并且删除可以被推导出的等价谓词。该函数调用build_equal_items()函数(sql/sql_select.cc：8273)用于该处理过程，该函数是一个递归过程，并在函数前，列举了多个实例，供参考和理解。
Sample 1:
SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t1.a=t3.a AND t2.a=t4.a WHERE t1.a=t2.a;
==>
SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON multiple equality (t1.a,t2.a,t3.a,t4.a)
Sample 2:
SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t1.a=t3.a AND t3.a=t4.a WHERE t1.a=t2.a
==>
SELECT * FROM (t1 LEFT JOIN (t3,t4) ON t1.a=t3.a AND t3.a=t4.a),t2 WHERE t1.a=t2.a
Sample 3:
SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t2.a=t4.a AND t3.a=t4.a WHERE t1.a=t2.a
==>
SELECT * FROM (t2 LEFT JOIN (t3,t4)ON t2.a=t4.a AND t3.a=t4.a), t1 WHERE t1.a=t2.a
    此外，该函数调用propagate_cond_constants()函数(sql/sql_select.cc：8763)，用于处理（a=b and b=1）为（a=1 and b=1）操作，该函数也是一个递归过程。调用remove_eq_conds()函数(sql/sql_select()：9617)，用于删除（a=a以及1=1、1=2）条件，该函数调用了internal_remove_eq_conds()递归处理函数(sql_select.cc：9462)。

prune_partitions()：(sql/opt_range.cc：2611)
prune_partitions()函数通过分析查询条件，查找需要的数据分区。该函数仅对分区表有效，普通的表不做处理。



make_join_statistics()：(sql/sql_select.cc：2651)
    make_join_statistics()函数用于计算最好的join执行计划。该过程较复杂，分为以下几个步骤：
首先如果join是外连接，利用Floyd Warshall（弗洛伊德）算法建立传递闭包依赖关系，该过程较复杂，具体参考该算法的一些内容。
之后进行主键或唯一索引的常量查询处理，该过程是一个循环过程，与执行计划中的ref类型有关。
接下来，计算每个表中匹配的行数以及扫描的时间，该过程得到的结果是估计的值，并非实际查询需要的记录数和时间。并且调用了一下的make_select()函数和get_quick_recond_count()函数，具体内容参照以下相应函数的分析。该过程从生成执行计划来看，与index和full类型有关。
最后，根据以上统计的记录数和时间，选择一个最优的join顺序。实际该处理逻辑，调用choose_plan()函数处理，参照以下choose_plan()函数分析。
该函数简化后的流程图如下图所示：






MySQL查询优化器源码分析   http://m.blog.chinaunix.net/uid-26896862-id-3218584.html


Optimizer阶段
execute_sqlcom_select()函数中，调用了handle_select()函数，而该函数正是处理SQL查询的“入口函数”
然后调用的是mysql_select()








词法分析MYSQLlex
    客户端向服务器发送过来SQL语句后，服务器首先要进行词法分析，而后进行语法分析，语义分析，构造执行树，生成执行计划；词法分析是第一阶段。
    词法分析即将输入的语句进行分词(token)，解析出每个token的意义。分词的本质便是正则表达式的匹配过程，比较流行的分词工具应该是lex，通过简单的规则制定，来实现分词。Lex一般和yacc结合使用。然而Mysql并没有使用lex来实现词法分析，但是语法分析却用了yacc，而yacc需要词法分析函数yylex。
    以select * from zzz where 为例。
    MySQL使用的是自己的词法分析，使用yacc作语法分析，而yacc需要词法分析函数yylex，故在sql_yacc.cc文件最前面我们可以看到如下的宏定义:
    #define yyparse         MYSQLparse
    #define yylex              MYSQLlex
    也就是yylex的实际入口是MYSQLlex(); sql/sql_lex.cc。lex_one_token()为主要的处理函数，先了解一下其中的几个重要的数据类型以及变量，分别是：
    Lex_input_stream *lip= &thd->m_parser_state->m_lip;
    uchar *state_map= cs->state_map;
    lip用来表示输入流，在sql/sql_lex.h中定义，暂时不知道在何处初始化的；state_map保存了词法分析机中的各种状态，state_map是通过init_state_maps(); mysys/charset.c用来初始化的，应该是在初始化字符集信息的时候完成初始化的。
    下面以insert into tablename values (2,‘Y’,2.3);为例对词法分析进行介绍
1) state的初始状态为MY_LEX_START，首先会跳过所有的空格，然后读取到第一个字母'i'，通过state_map数组返回的state状态为MY_LEX_IDENT。
2) 接下来状态机进如MY_LEX_IDENT分支，这时会将整个insert单词读完直到遇到空格，然后对读到的单词进行判定是否为关键字（判定函数为find_keyword），如果是关键字则会返回hash表中该单词所对应的关键字，由于insert是关键字那么返回该token，并将下一个状态设置为MY_LEX_START。至此insert这个token被识别并被传到MYSQLlex中，被后续的语法分析所用。
3) 重新进入lex_one_token函数，由于into同样是关键字，因此对于into的分析和insert是一样的，在此就不再赘述。
4) 再次进入lex_one_token函数，在经过初始处理之后，状态机会进入MY_LEX_IDENT分支。在读完tablename之后发现不是关键字，这时由get_token函数将读到的单词保存到yylval中(yylval->lex_str)并返回result_state，result_state的取值是根据单词是否存在非ASCII码字母来决定为IDENT或是IDENT_QUOTED（这两个都是yacc中的一种token类型）。
...........
可以参考http://blog.csdn.net/sfifei/article/details/9449629

    语法是自上而下的，实际的解析过程是自下而上的匹配过程，词法分析首先yacc送来SELECT关键字，但是为什么SELECT是关键字呢。
    在sql/sql_yacc.yy中可以找到如下一个定义，也就是说通过词法分析传过来的应该是SELECT_SYM这个宏。
        %token   SELECT_SYM              /* SQL-2003-R */
    在执行bison -d之后，实际会转换为一个宏SELECT_SYM，代表一个关键字，使用enum表示为一个宏定义，如下：
        enum yytokentype {...,  SELECT_SYM 687, ... }
    那么再MySQL中，是如何将关键字与xxx_SYM宏对应起来的呢？在sql/sql_lex.cc中的find_keyword()，通过该函数查找是否为关键字；而该函数实际调用的是get_hash_symbol()，该函数实际是在symbols_map中查找。这个数组在lex_hash.h中，从注释中可以看出该数组是由gen_lex_hash.cc产生的。
    在gen_lex_hash.cc中，看到了个main函数，里面是一些生成文件的操作，在generate_find_structs()函数中找到了insert_symbols()，这应该是初始化我们的symbols_map数组了吧。可以看到函数的实现是循环取数组symbols,找到symbols定义，在文件lex.h中，可以看到这个数组。
    { "SELECT", SYM(SELECT_SYM)},
    这就是将SELECT字符串与SELECT_SYM关联的地方了。
    再来捋一下SELECT解析的思路，词法分析解析到SELECT后，执行find_keyword去找是否是关键字，发现SELECT是关键字，于是给yacc返回SELECT_SYM用于语法分析。NOTE：如果我们想要加关键字，只需在sql_yacc.yy上面添加一个%token xxx,然后在lex.h里面加入相应的字符串和SYM的对应即可。
    select在sql/sql_yacc.yy中的语法节点执行流程为：
    query -> verb_clause -> statement -> select -> select_init(SELECT_SYM关键字) -> select_init2 -> select_part2 -> select_options(展开后为各个选项) -> select_item_list -> select_item(这里为指定的列，可以为*) -> select_alias
到此为止包括了select所包含的语义解析，包括了select [options] {*|columns}  AS ident。
    select_part2: select_options select_item_list {  } select_into select_lock_type ;
    当匹配 select_options select_item_list将执行{}中的内容，同时会继续匹配下面的内容，也就是将会匹配select_from等字段。
    解析时通过对应的action将相应的参数添加到列表中，也就是对add_item_to_list 和table_list的赋值。解析后对于需要查询的表(zzz)和字段(*)这些信息都写入到thd->lex这个结构体里了，例如其中thd->lex->query_tables就是表(zzz)的状况；thd->lex->current_select->with_wild 是表示该语句是否使用了*；sql_command为SQLCOM_SELECT；where子句保存在select_lex->where；table列表select_lex->table_list；字段列表select_lex->item_list等等。
    例如语句select table1.field1,'test',100 from table1 where table1.field1='field1' and (table1.field2=100 or table1.field2=200)
    1) 选择域的解析  table1.field1,'test',100
将被解析为一个List<Item>，其中List的第一个元素Item的子类Item_field，表示表中的列。
第二个元素为Item_string，表示字符串。由val_str方法可获得string值"test”,也可用tmp_table_field_from_field_type方法返回一个Field的子类Field_string的指针。
第三个元素为Item_int，表示整数。由val_int获得int值100,也可以用tmp_table_field_from_field_type方法返回一个Field的子类Field_longlong的指针。
2) Where域的解析where table1.field1='field1' and (table1.field2=100 or table1.field2=200)
将被解析为一个Item对象，这个对象的层次结构如下：

    下面列举了一些常见语句解析后的结果。
A) Select语句
    对select类型的语句解析后，将结果存放在SELECT_LEX类中，其中：
选择域存放在SELECT_LEX::item_list中，类型为LIST<Item>
where域存放在SELECT_LEX::wheret中，类型为Item*
having域存放在SELECT_LEX::having中，类型为Item*
order域存放在SELECT_LEX::order_list中，实际类型为ORDER*
group域存放在SELECT_LEX::group_list中，实际类型为ORDER*
limit域存放在SELECT_LEX::select_limit中，unsigned long
table名字域存放在SELECT_LEX::table_list中，实际类型为TABLE_LIST*
    where域和having域的解构请见上文中，其他几个域的解构类似于链表。
B) Update语句
    对update类型的语句解析后，将结果存放在SELECT_LEX类和LEX类中，其中：
更新域存放在SELECT_LEX::item_list中，类型为LIST<Item>
值域存放在LEX::value_list中,类型为LIST<Item>
where域存放在SELECT_LEX::wheret中，类型为Item*
table名字域存放在SELECT_LEX::table_list中，实际类型为TABLE_LIST*
    (其中更新域和值域的结构请见上文中4(1),where域的解构请见上文，table名字域的解构类似于链表)
C) Insert语句
    对insert类型的语句解析后，将结果存放在SELECT_LEX类和LEX类中，其中：
插入域存放在LEX::item_list中，类型为LIST<Item>
值域存放在LEX::many_values中,类型为LIST<LIST<Item>>
table名字域存放在SELECT_LEX::table_list中，实际类型为TABLE_LIST*
(其中插入域的结构请见上文中的4(1), 值域可以含有多个LIST<Item>, table名字域的解构类似于链表)
D) Delete语句
对delete类型的语句解析后，将结果存放在SELECT_LEX类中，其中：
where域存放在SELECT_LEX::wheret中，类型为Item*
limit域存放在SELECT_LEX::select_limit中，unsigned long
table名字域存放在SELECT_LEX::table_list中，实际类型为TABLE_LIST*
    (其中选择域的结构请见上文中的4(1),where域和having域的解构请见上文中的4(2), 其他几个域的解构类似于链表)

PS: 对于查找关键字多说一句。
    考虑到关键字是一个只读的列表，对它做一个只读的查找树可以改善查找的性能，
    产生查找树：A) 读取关键字数组，产生一个Trie树；B) 调整这棵树，并产生一个数组，也就是一个不用链表表示的树。
    相关的Makefile规则，在sql/CMakeFiles/sql.dir/build.make文件中：
sql/lex_hash.h: sql/gen_lex_hash
$(CMAKE_COMMAND) -E cmake_progress_report /home/zedware/Workspace/mysql/CMakeFiles $(CMAKE_PROGRESS_153)
@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating lex_hash.h"
cd /home/zedware/Workspace/mysql/sql && ./gen_lex_hash > lex_hash.h
    使用查找树：这个比较简单，直接看函数get_hash_symbol好了。


在mysql里建立一个sql语句，类似于show authors，在此添加show disk_usage命令。
1.  在sql/lex.h加入新语句的名称：disk_usage;
    static SYMBOL symbols[] = { .... ,  {"DISK_USAGE",  SYM(DISK_USAGE_SYM)},  ... }
2. 修改 sql/sql_lex.h
    增加一个助记符来标识这个命令。这个助记符将在解析器利用来创建和标识一个内部查询结构，并通过sql_parse.cc文件里的超大型switch语句的一个case分支对执行流程进行控制。
    enum enum_sql_command { ..., SQLCOM_SHOW_DISK_USAGE, ... }
    注意该变量上面的注释，增加完该命令后，也需要在sql/mysqld.cc文件中，为SHOW_VAR com_status_vars[] 增加一项。
    {"show_disk_usage",         (char*) offsetof(STATUS_VAR,
        com_stat[(uint) SQLCOM_SHOW_DISK_USAGE]), SHOW_LONG_STATUS},
3. 修改sql/sql_yacc.yy
    在这个文件里定义在lex.h里使用的新记号。
        %token DISK_USAGE_SYM
    还需要把新命令的语法加到解析器的YACC代码里，在show:标签下，
        show:
            SHOW DISK_USAGE_SYM
            {
                LEX * lex=Lex;
                lex->sql_command=SQLCOM_SHOW_DISK_USAGE;
            }
        |   SHOW ... ...
4. 然后就是到了比较熟悉的sql/sql_parse.cc中增加相应的路由信息。
    case SQLCOM_SHOW_AUTHORS:
        ... ...
    case SQLCOM_SHOW_DISK_USAGE:
        res = show_disk_usage_command(thd);
        break;
5. 接着我们在sql/sql_show.cc中加上show_disk_usage_command的实现，可在mysqld_show_authors()函数后面添加该函数。同时在sql/sql_show.h文件中增加声明bool show_disk_usage_command(THD *thd);。
bool show_disk_usage_command(THD *thd)
{
    List<Item> field_list;
    Protocol *protocol = thd->protocol;
    DBUG_ENTER(“show_disk_usage”);

    /* send fields */
    field_list.push_back(new Item_empty_string(“Database”, 50));
    field_list.push_back(new Item_empty_string(“Size (Kb)”, 30));
    if (protocol->send_result_set_metadata(&field_list,
            Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
        DBUG_RETURN(TRUE);
    /* send test data */
    protocol->prepare_for_resend();
    protocol->store(“test_row”, system_charset_info);
    protocol->store(“1024”, system_charset_info);
    if (protocol->write())
        DBUG_RETURN(TRUE);
    my_eof(thd);
    DBUG_RETURN(FALSE);
}
6. 在sql目录下生成sql_yacc.cc和sql_yacc.h，bison -y -d sql_yacc.yy，将生成的y.tab.c和y.tab.h替换掉即可。
    在sql_yacc.cc中添加如下内容。
    /* Pull parsers.  */
    #define YYPULL 1
    /* Using locations.  */
    #define YYLSP_NEEDED 0
    /* Substitute the variable and function names.  */
    #define yyparse         MYSQLparse
    #define yylex           MYSQLlex
    #define yyerror         MYSQLerror
    #define yylval          MYSQLlval
    #define yychar          MYSQLchar
    #define yydebug         MYSQLdebug
    #define yynerrs         MYSQLnerrs

7. 现在重新生成下lex hash.h，gen_lex_hash > lex_hash.h ，替换掉源文件即可。
8. 现在已经大功告成，重新编译一把mysqld工程，然后从client测试下。
mysql -uroot -h127.1
mysql> show disk_usage;
其它可以查看http://www.orczhou.com/index.php/2012/11/more-about-mysql-item/
http://www.orczhou.com/index.php/2012/11/mysql-source-code-data-structure-about-index/
http://www.orczhou.com/index.php/2012/11/mysql-innodb-source-code-optimization-1/








class THD在sql/sql_class.h中定义，在sql/sql_class.cc中实现，




update t2 set b=b*2-1, c=c/2+4 where id=5;
set左值集合：fields
thd->lex->select_lex.item_list
List<Item>
|-->Item_field(b)
|-->Item_field(c)
set右值集合：values：
thd->lex->value_list
list<Item>
|-->Item_func_minus
    |-->args[0]:Item_func_mul
        |-->args[0]:Item_field(b)
        |-->args[1]:Item_int(2)
    |-->args[1]:Item_int(1)
|-->Item_func_plus
    |-->args[0]:Item_func_div
        |-->args[0]:Item_field(c)
        |-->args[1]:Item_int(2)
    |-->args[1]:Item_int(4)
where 条件：
thd->lex->select_lex.where
Item_func_eq
|-->args[0]:Item_field(id)
|-->args[1]:Item_int(5)


</p>










## 参考

关于 MySQL 的语法可以参考 [MySQL Reference: JOIN Syntax](http://dev.mysql.com/doc/refman/en/join.html) 。


{% highlight text %}
{% endhighlight %}