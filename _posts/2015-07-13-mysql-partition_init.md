---
title: MySQL 分区表
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,plugin,插件
description: 在 MySQL 中，为了提高其灵活性，很多的功能都是通过插件来实现的，常见的比如 semi-sync、存储引擎、登陆认证等等。因为 MySQL 是 C/C++ 实现的，对于插件来说实际为动态链接库，保存在 plugin_dir 变量对应的目录下。在此介绍一下 MySQL 的插件实现。
---


<!-- more -->


= 水平分区（根据列属性按行分）=
举个简单例子：一个包含十年发票记录的表可以被分区为十个不同的分区，每个分区包含的是其中一年的记录。

=== 水平分区的几种模式：===
* Hash（哈希） – 这中模式允许DBA通过对表的一个或多个列的Hash Key进行计算，最后通过这个Hash码不同数值对应的数据区域进行分区，。例如DBA可以建立一个对表主键进行分区的表。
* Key（键值） – 上面Hash模式的一种延伸，这里的Hash Key是MySQL系统产生的。
* List（预定义列表） – 这种模式允许系统通过DBA定义的列表的值所对应的行数据进行分割。例如：DBA建立了一个横跨三个分区的表，分别根据2004年2005年和2006年值所对应的数据。
* Composite（复合模式） - 很神秘吧，哈哈，其实是以上模式的组合使用而已，就不解释了。举例：在初始化已经进行了Range范围分区的表上，我们可以对其中一个分区再进行hash哈希分区。
= 垂直分区（按列分）=
举个简单例子：一个包含了大text和BLOB列的表，这些text和BLOB列又不经常被访问，这时候就要把这些不经常使用的text和BLOB了划分到另一个分区，在保证它们数据相关性的同时还能提高访问速度。




{% highlight sql %}
CREATE TABLE foobar_partition (
  c1 INT DEFAULT NULL,
  c2 VARCHAR(30) DEFAULT NULL,
  c3 DATE DEFAULT NULL
) ENGINE=InnoDB PARTITION BY RANGE (year(c3)) (
  PARTITION p0 VALUES LESS THAN (1995),
  PARTITION p1 VALUES LESS THAN (1996) ,
  PARTITION p2 VALUES LESS THAN (1997) ,
  PARTITION p3 VALUES LESS THAN (1998) ,
  PARTITION p4 VALUES LESS THAN (1999) ,
  PARTITION p5 VALUES LESS THAN (2000) ,
  PARTITION p6 VALUES LESS THAN (2001) ,
  PARTITION p7 VALUES LESS THAN (2002) ,
  PARTITION p8 VALUES LESS THAN (2003) ,
  PARTITION p9 VALUES LESS THAN (2004) ,
  PARTITION p10 VALUES LESS THAN (2010),
  PARTITION p11 VALUES LESS THAN MAXVALUE
);
CREATE TABLE foobar_normal (
  c1 INT DEFAULT NULL,
  c2 VARCHAR(30) DEFAULT NULL,
  c3 DATE DEFAULT NULL
) ENGINE=InnoDB;

TRUNCATE TABLE load_partition;
DELIMITER EOF
drop procedure if exists `test`.`load_partition` EOF
CREATE PROCEDURE load_partition(IN number INT(11))
  BEGIN
    DECLARE i INT DEFAULT 1;
    -- such as 1-200,200-400,....
    WHILE i <= number DO
      IF mod(i, 20)=1 THEN
        SET @sqltext=concat("(", i, ", '", concat('testing partitions ', i), "', '",
                            adddate('1995-01-01',(rand(i)*36520) mod 3652), "')");
      ELSEIF mod(i, 20)=0 THEN
        SET @sqltext=concat(@sqltext, ",(", i, ", '", concat('testing partitions ', i), "', '",
                            adddate('1995-01-01',(rand(i)*36520) mod 3652), "')");
        SET @sqltext=concat("INSERT INTO foobar_partition VALUES", @sqltext);
        -- SELECT @sqltext;
        SELECT i AS count;
        PREPARE stmt FROM @sqltext;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        SET @sqltext='';
      ELSE
        SET @sqltext=concat(@sqltext, ",(", i, ", '", concat('testing partitions ', i), "', '",
                            adddate('1995-01-01',(rand(i)*36520) mod 3652), "')");
      END IF;
      SET i = i + 1;
    END WHILE;
    -- process when number is not be moded by 2000, such as 201,402,1520,...
    IF @sqltext<>'' THEN
      SET @sqltext=concat("INSERT INTO foobar_partition VALUES", @sqltext);
      -- SELECT @sqltext;
      PREPARE stmt FROM @sqltext;
      EXECUTE stmt;
      DEALLOCATE PREPARE stmt;
      SET @sqltext='';
    END IF;
  END
EOF
DELIMITER ;
CALL load_partition(80);
{% endhighlight %}

MySQL 中，可以将分区设置为独立的数据、索引存放目录，同时，这些目录所在的物理磁盘分区可能也都是完全独立的，从而可以提高磁盘 IO 吞吐量；需要注意的是 MERGE, CSV, FEDERATED 存储引擎不支持分区。

### RANGE/RANGE COLUMNS

可以将数据划分不同的区间，这些区间要连续且不能重叠，使用 ```VALUES LESS THAN``` 操作符定义分区；例如，将一个表通过年份划分成三个分区，80 年代 (1980's) 的数据，90 年代 (1990's) 的数据以及任何在 2000 年 (包括2000年) 后的数据。

RANGE 分区主要是基于整数的分区，对于非整形的字段需要利用表达式将其转换成整形。

{% highlight text %}
CREATE TABLE users ( 
  uid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '', 
  email VARCHAR(30) NOT NULL DEFAULT '' 
) PARTITION BY RANGE (uid) (
  PARTITION p0 VALUES LESS THAN (3000000) 
  DATA DIRECTORY = '/data0/data' 
  INDEX DIRECTORY = '/data1/idx', 

  PARTITION p1 VALUES LESS THAN (6000000) 
  DATA DIRECTORY = '/data2/data' 
  INDEX DIRECTORY = '/data3/idx', 
 
  PARTITION p2 VALUES LESS THAN (9000000) 
  DATA DIRECTORY = '/data4/data' 
  INDEX DIRECTORY = '/data5/idx', 
 
  PARTITION p3 VALUES LESS THAN MAXVALUE  必须
  DATA DIRECTORY = '/data6/data'  
  INDEX DIRECTORY = '/data7/idx' 
);
{% endhighlight %}

<!--
CREATE TABLE `test`.`partition_t3`( 
  `id` INT UNSIGNED NOT NULL,
  `username` VARCHAR(30) NOT NULL,
  `email` VARCHAR(30) NOT NULL,
  `birth_date` DATE NOT NULL
) ENGINE=MYISAM
PARTITION BY RANGE COLUMNS(birth_date)(
   PARTITION t31 VALUES LESS THAN ('1996-01-01'),
   PARTITION t32 VALUES LESS THAN ('2006-01-01'),
   PARTITION t33 VALUES LESS THAN ('2038-01-01')
);
-->

MySQL 5.5 改进 range 分区，提供 range columns 分区支持非整数分区。

COLUMNS分区：可以无需通过表达式进行转换直接对非整形字段进行分区，同时COLUMNS分区还支持多个字段组合分区，只有RANGELIST存在COLUMNS分区，COLUMNS是RANGE和LIST分区的升级
RANGE COLUMNS是RANGE分区的一种特殊类型，它与RANGE分区的区别如下：
1. RANGE COLUMNS不接受表达式，只能是列名。而RANGE分区则要求分区的对象是整数。
2. RANGE COLUMNS允许多个列，在底层实现上，它比较的是元祖（多个列值组成的列表），而RANGE比较的是标量，即数值的大小。
3. RANGE COLUMNS不限于整数对象，date，datetime，string都可作为分区列。
同RANGE分区类似，它的区间范围必须是递增的，有时候，列涉及的太多，不好判断区间的大小，可采用下面的方式进行判断：
SELECT (5,10) < (5,12), (5,11) < (5,12), (5,12) < (5,12);


### LIST/LIST COLUMNS

每个分区的定义和选择是基于某列的值或者一个返回整数的表达式，是基于列出的枚举值列表进行分区。

CREATE TABLE category ( 
  cid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '' 
) 
PARTITION BY LIST (cid) ( 
  PARTITION p0 VALUES IN (0,4,8,12) 
  DATA DIRECTORY = '/data0/data'  
  INDEX DIRECTORY = '/data1/idx', 
   
  PARTITION p1 VALUES IN (1,5,9,13) 
  DATA DIRECTORY = '/data2/data' 
  INDEX DIRECTORY = '/data3/idx', 
   
  PARTITION p2 VALUES IN (2,6,10,14) 
  DATA DIRECTORY = '/data4/data' 
  INDEX DIRECTORY = '/data5/idx', 
   
  PARTITION p3 VALUES IN (3,7,11,15) 
  DATA DIRECTORY = '/data6/data' 
  INDEX DIRECTORY = '/data7/idx' 
);
LIST COLUMNS分区同样是LIST分区的一种特殊类型，它和RANGE COLUMNS分区较为相似，同样不接受表达式，同样支持多个列支持string,date和datetime类型。
CREATE TABLE customers_2 (
    first_name VARCHAR(25),
    last_name VARCHAR(25),
    street_1 VARCHAR(30),
    street_2 VARCHAR(30),
    city VARCHAR(15),
    renewal DATE
)
PARTITION BY LIST COLUMNS(city,last_name,first_name) (
    PARTITION pRegion_1 VALUES IN (('Oskarshamn', 'Högsby', 'Mönsterås'),('Nässjö', 'Eksjö', 'Vetlanda')),
    PARTITION pRegion_2 VALUES IN(('Vimmerby', 'Hultsfred', 'Västervik'),('Uppvidinge', 'Alvesta', 'Växjo'))
);


### HASH/LINEAR HASH

   TODODO: hash值是如何计算的

基于给定的分区个数，将数据分配到不同的分区，HASH分区只能针对整数进行HASH，对于非整形的字段只能通过表达式将其转换成整数。

CREATE TABLE users ( 
  uid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '', 
  email VARCHAR(30) NOT NULL DEFAULT '' 
) 
PARTITION BY HASH (uid) PARTITIONS 4 ( 
  PARTITION p0 
  DATA DIRECTORY = '/data0/data' 
  INDEX DIRECTORY = '/data1/idx', 
 
  PARTITION p1 
  DATA DIRECTORY = '/data2/data' 
  INDEX DIRECTORY = '/data3/idx', 
 
  PARTITION p2 
  DATA DIRECTORY = '/data4/data' 
  INDEX DIRECTORY = '/data5/idx', 
 
  PARTITION p3 
  DATA DIRECTORY = '/data6/data' 
  INDEX DIRECTORY = '/data7/idx' 
);
它的优点是在数据量大的场景，譬如TB级，增加、删除、合并和拆分分区会更快，缺点是，相对于HASH分区，它数据分布不均匀的概率更大。
http://dev.mysql.com/doc/refman/5.6/en/partitioning-linear-hash.html
CREATE TABLE `test`.`partition_t5`( 
`id` INT UNSIGNED NOT NULL,
`username` VARCHAR(30) NOT NULL,
`email` VARCHAR(30) NOT NULL,
`birth_date` DATE NOT NULL
) ENGINE=MYISAM
PARTITION BY LINEAR HASH(id)
PARTITIONS 5;

### KEY

hash分区允许用户自定义的表达式，而key分区不允许使用用户自定义的表达式。

hash分区只支持整数分区，key分区支持除了blob或text类型之外的其他数据类型分区。
与hash分区不同，创建key分区表的时候，可以不指定分区键，默认会选择使用主键/唯一键作为分区键，没有主键/唯一键,必须指定分区键。
KEY分区和HASH分区的算法不一样，PARTITION BY HASH (expr)，MOD取值的对象是expr返回的值，而PARTITION BY KEY (column_list)，基于的是列的MD5值。
CREATE TABLE users ( 
  uid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '', 
  email VARCHAR(30) NOT NULL DEFAULT '' 
) 
PARTITION BY KEY (uid) PARTITIONS 4 ( 
  PARTITION p0 
  DATA DIRECTORY = '/data0/data' 
  INDEX DIRECTORY = '/data1/idx', 
   
  PARTITION p1 
  DATA DIRECTORY = '/data2/data'  
  INDEX DIRECTORY = '/data3/idx', 
   
  PARTITION p2  
  DATA DIRECTORY = '/data4/data' 
  INDEX DIRECTORY = '/data5/idx', 
   
  PARTITION p3  
  DATA DIRECTORY = '/data6/data' 
  INDEX DIRECTORY = '/data7/idx' 
);


子分区
子分区是针对 RANGE/LIST 类型的分区表中每个分区的再次分割。再次分割可以是 HASH/KEY 等类型。例如：
CREATE TABLE users ( 
  uid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '', 
  email VARCHAR(30) NOT NULL DEFAULT '' 
) 
PARTITION BY RANGE (uid) SUBPARTITION BY HASH (uid % 4) SUBPARTITIONS 2( 
  PARTITION p0 VALUES LESS THAN (3000000) 
  DATA DIRECTORY = '/data0/data' 
  INDEX DIRECTORY = '/data1/idx', 
 
  PARTITION p1 VALUES LESS THAN (6000000) 
  DATA DIRECTORY = '/data2/data' 
  INDEX DIRECTORY = '/data3/idx' 
); 

对 RANGE 分区再次进行子分区划分，子分区采用 HASH 类型。
CREATE TABLE users ( 
  uid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  name VARCHAR(30) NOT NULL DEFAULT '', 
  email VARCHAR(30) NOT NULL DEFAULT '' 
) 
PARTITION BY RANGE (uid) SUBPARTITION BY KEY(uid) SUBPARTITIONS 2( 
  PARTITION p0 VALUES LESS THAN (3000000) 
  DATA DIRECTORY = '/data0/data' 
  INDEX DIRECTORY = '/data1/idx', 
 
  PARTITION p1 VALUES LESS THAN (6000000) 
  DATA DIRECTORY = '/data2/data' 
  INDEX DIRECTORY = '/data3/idx' 
); 

对 RANGE 分区再次进行子分区划分，子分区采用 KEY 类型。

= 分区管理 =

删除分区 
ALERT TABLE users DROP PARTITION p0; 

重建分区
----- RANGE分区重建，将原来的 p0,p1 分区合并起来，放到新的 p0 分区中
ALTER TABLE users REORGANIZE PARTITION p0,p1 INTO (PARTITION p0 VALUES LESS THAN (6000000));
----- LIST分区重建，将原来的 p0,p1 分区合并起来，放到新的 p0 分区中。
ALTER TABLE users REORGANIZE PARTITION p0,p1 INTO (PARTITION p0 VALUES IN(0,1,4,5,8,9,12,13)); 
----- HASH/KEY分区重建，分区数只能减小不能增加，增加可以通过ADD PARTITION方法
ALTER TABLE users REORGANIZE PARTITION COALESCE PARTITION 2; 

新增分区
----- 新增LIST分区  
ALTER TABLE category ADD PARTITION (PARTITION p4 VALUES IN (16,17,18,19) 
   DATA DIRECTORY = '/data8/data' 
   INDEX DIRECTORY = '/data9/idx'); 
----- 新增HASH/KEY分区，将分区总数扩展到8个
ALTER TABLE users ADD PARTITION PARTITIONS 8; 



[ 给已有的表加上分区 ]
[sql] view plain copy

    alter table results partition by RANGE (month(ttime))  
    (PARTITION p0 VALUES LESS THAN (1), 
    PARTITION p1 VALUES LESS THAN (2) , PARTITION p2 VALUES LESS THAN (3) , 
    PARTITION p3 VALUES LESS THAN (4) , PARTITION p4 VALUES LESS THAN (5) , 
    PARTITION p5 VALUES LESS THAN (6) , PARTITION p6 VALUES LESS THAN (7) , 
    PARTITION p7 VALUES LESS THAN (8) , PARTITION p8 VALUES LESS THAN (9) , 
    PARTITION p9 VALUES LESS THAN (10) , PARTITION p10 VALUES LESS THAN (11), 
    PARTITION p11 VALUES LESS THAN (12), 
    PARTITION P12 VALUES LESS THAN (13) );  



默认分区限制分区字段必须是主键（PRIMARY KEY)的一部分，为了去除此
限制：
[方法1] 使用ID
[sql] view plain copy

    mysql> ALTER TABLE np_pk 
        ->     PARTITION BY HASH( TO_DAYS(added) ) 
        ->     PARTITIONS 4; 

ERROR 1503 (HY000): A PRIMARY KEY must include all columns in the table's partitioning function

However, this statement using the id column for the partitioning column is valid, as shown here:

[sql] view plain copy

    mysql> ALTER TABLE np_pk 
        ->     PARTITION BY HASH(id) 
        ->     PARTITIONS 4; 

Query OK, 0 rows affected (0.11 sec)
Records: 0 Duplicates: 0 Warnings: 0

[方法2] 将原有PK去掉生成新PK
[sql] view plain copy

    mysql> alter table results drop PRIMARY KEY; 

Query OK, 5374850 rows affected (7 min 4.05 sec)
Records: 5374850 Duplicates: 0 Warnings: 0

[sql] view plain copy

    mysql> alter table results add PRIMARY KEY(id, ttime); 

Query OK, 5374850 rows affected (6 min 14.86 sec)

Records: 5374850 Duplicates: 0 Warnings: 0

http://www.cnblogs.com/chenmh/p/5623474.html

http://www.cnblogs.com/martinzhang/p/3467232.html

http://www.simlinux.com/archives/133.html

http://blog.csdn.net/tjcyjd/article/details/11194489

http://blog.51yip.com/mysql/1013.html

https://segmentfault.com/a/1190000006812384

查看EXPLAIN命令

EXPLAIN PARTITIONS

查看各个分区当前的数据量

SELECT partition_name part, partition_method method, partition_expression expr, partition_description descr, table_rows
  FROM INFORMATION_SCHEMA.partitions WHERE TABLE_SCHEMA = schema() AND TABLE_NAME='foobar_partition';

http://phpzf.blog.51cto.com/3011675/793775

L 180/100A 胸围 104 腰围 102 肩阔 45 后中长 70 袖长 44
























https://www.byvoid.com/zhs/blog/string-hash-compare





{% highlight text %}
{% endhighlight %}
