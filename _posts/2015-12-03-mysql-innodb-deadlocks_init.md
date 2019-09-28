---
title: InnoDB 死锁处理
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,innodb,deadlock,死锁
description: 当两个或以上的事务相互持有并请求锁，当形成一个循环的依赖关系时，就会产生死锁；InnoDB 会自动检测事务死锁，立即回滚其中代价最小的某个事务，并且返回一个错误。通常，在一个事务系统中，死锁是确切存在并且是不能完全避免的，偶然发生的死锁不必担心，但死锁频繁出现的时候就要引起注意了。接下来，我们看看 InnoDB 对死锁的处理，以及诊断方式。
---

当两个或以上的事务相互持有并请求锁，当形成一个循环的依赖关系时，就会产生死锁；InnoDB 会自动检测事务死锁，立即回滚其中代价最小的某个事务，并且返回一个错误。

通常，在一个事务系统中，死锁是确切存在并且是不能完全避免的，偶然发生的死锁不必担心，但死锁频繁出现的时候就要引起注意了。

接下来，我们看看 InnoDB 对死锁的处理，以及诊断方式。

<!-- more -->

## 简介

InnoDB 引擎采用两种方法处理死锁：A) 被动等待超时，通过参数 innodb_lock_wait_timeout 控制；B) 主动通过 Wait for Graph 算法检测，每当加锁请求无法立即满足需要并进入等待时，该算法就会被触发。

Wait for Graph 就是当事务 A 需要等待事务 B 的资源时，就会生成一条有向边指向 B，依次类推，最后形成一个有向图；然后检测这个有向图是否出现环路即可，如果出现环路则发生了死锁！

简单来说，产生死锁的必要条件：

1. 两个及以上的多个并发事务；
2. 每个事务都持有了锁，或者是在等待锁；
3. 每个事务为了完成业务逻辑都需要再继续持有锁；
4. 事务之间产生加锁的循环等待，形成死锁。

InnoDB 有一个后台的锁监控线程，该线程负责查看可能的死锁问题，并自动告知用户。

另外，死锁与事务隔离级别、是否为二级索引相关。


### 死锁诊断

在 MySQL 5.6 之前，只有最新的死锁信息可以使用 ```show engine innodb status``` 命令查看；也可以使用 Percona Toolkit 工具包中的 [pt-deadlock-logger](https://www.percona.com/doc/percona-toolkit/2.2/pt-deadlock-logger.html) 保存死锁信息，可以写入文件或者表中，该工具也是通过上述的命令查看。

对于 MySQL 5.6 及以上版本，可以通过参数 innodb_print_all_deadlocks 把 InnoDB 中发生的所有死锁信息都记录在错误日志里面。

死锁发生以后，只有部分或者完全回滚其中一个事务，才能打破死锁；InnoDB 目前处理死锁的方法就是将持有最少行级排他锁的事务进行回滚，这也是相对比较简单的死锁回滚方式。

### 死锁处理

首先死锁是正常的，不必过于担心；当然，有如下的方法，可以处理死锁。

#### 避免死锁

最好的方式是在业务逻辑层做调整，也就是修改 SQL 的操作顺序；另外，尽量缩短长事务。

#### 死锁检测

InnoDB 会记录事务所维持以及等待的锁，然后通过 Wait for Graph 算法进行检测。

#### 等锁超时

也就是锁的持有时间，如果说一个事务持有锁超过设置时间的话，就直接抛出一个错误，InnoDB 不会回滚该事务。如果业务逻辑中有超长的事务，就需要把锁超时时间设置为大于事务执行时间。

### 常用命令

与锁相关的环境变量，可以通过如下命令查看。

{% highlight text %}
----- 加锁的超时时间，单位为秒
mysql> SHOW VARIABLES LIKE 'innodb_lock_wait_timeout';

----- 对于MySQL5.6之后可以进行设置，将死锁打印到错误日志
mysql> SHOW VARIABLES LIKE 'innodb_print_all_deadlocks';
----- 开启死锁日志，如果有死锁，则可以查看如下日志
mysql> SET GLOBAL innodb_print_all_deadlocks = 1;
... ...
2016-02-16 22:10:52 InnoDB: transactions deadlock detected, dumping detailed information.
... ...

----- 查看当前连接的事务ID
mysql> SELECT trx_id FROM information_schema.innodb_trx WHERE trx_mysql_thread_id = connection_id();
{% endhighlight %}

注意，对于上述的事务 ID 查看命令，如果是只读的，启动时不会分配事务 ID 。


### 测试

创建环境。

{% highlight text %}
mysql> CREATE TABLE account(id INT PRIMARY KEY, money INT NOT NULL);
mysql> INSERT INTO account values(1,1000), (2, 1000);
{% endhighlight %}

接下来，分别在两个终端各启动一个事务。

{% highlight text %}
----- TRANS A -----------------------------+----- TRANS B -----------------------------
### ID=9249                                |  ### ID=9250
START TRANSACTION;                         |  START TRANSACTION;
UPDATE account SET money=2000 WHERE id=1;  |
                                           |  UPDATE account SET money=2000 WHERE id=2;
### 由于id=2记录加了X锁，此时会被卡住          |
UPDATE account SET money=3000 WHERE id=2;  |
                                           |  ### 监测到发生死锁，直接回滚该事务
                                           |  UPDATE account SET money=3000 WHERE id=1;
{% endhighlight %}

对于事务 A ，当尝试更新 id=2 时，如果此时加锁超时，会返回客户端报错信息，不过 **该事务不会被回滚**；而事务 B 尝试更新 id=1 时，此时会回滚该事务。

上述事务 B 当检测到死锁后，会抛出如下的报错信息，即发生了死锁。

{% highlight text %}
ERROR 1213 (40001): Deadlock found when trying to get lock; try restarting transaction
{% endhighlight %}

然后，通过 ```show engine innodb status``` 命令查看死锁信息，内容如下。

{% highlight text linenos %}
------------------------
LATEST DETECTED DEADLOCK
------------------------
2016-02-05 12:39:02 0x7f398fe72700
*** (1) TRANSACTION:
TRANSACTION 9249, ACTIVE 255 sec starting index read
mysql tables in use 1, locked 1
LOCK WAIT 3 lock struct(s), heap size 1160, 3 row lock(s), undo log entries 1
MySQL thread id 2, OS thread handle 139885204420352, query id 60 localhost root updating
UPDATE account SET money=3000 WHERE id=2
*** (1) WAITING FOR THIS LOCK TO BE GRANTED:
RECORD LOCKS space id 54 page no 3 n bits 72 index PRIMARY of table `test`.`account` trx id 9249 lock_mode X locks rec but not gap waiting
Record lock, heap no 3 PHYSICAL RECORD: n_fields 4; compact format; info bits 0
 0: len 4; hex 80000002; asc     ;;
 1: len 6; hex 000000002422; asc     $";;
 2: len 7; hex 37000001840b6a; asc 7     j;;
 3: len 4; hex 800007d0; asc     ;;

*** (2) TRANSACTION:
TRANSACTION 9250, ACTIVE 155 sec starting index read
mysql tables in use 1, locked 1
3 lock struct(s), heap size 1160, 2 row lock(s), undo log entries 1
MySQL thread id 3, OS thread handle 139885204154112, query id 61 localhost root updating
UPDATE account SET money=3000 WHERE id=1
*** (2) HOLDS THE LOCK(S):
RECORD LOCKS space id 54 page no 3 n bits 72 index PRIMARY of table `test`.`account` trx id 9250 lock_mode X locks rec but not gap
Record lock, heap no 3 PHYSICAL RECORD: n_fields 4; compact format; info bits 0
 0: len 4; hex 80000002; asc     ;;
 1: len 6; hex 000000002422; asc     $";;
 2: len 7; hex 37000001840b6a; asc 7     j;;
 3: len 4; hex 800007d0; asc     ;;

*** (2) WAITING FOR THIS LOCK TO BE GRANTED:
RECORD LOCKS space id 54 page no 3 n bits 72 index PRIMARY of table `test`.`account` trx id 9250 lock_mode X locks rec but not gap waiting
Record lock, heap no 2 PHYSICAL RECORD: n_fields 4; compact format; info bits 0
 0: len 4; hex 80000001; asc     ;;
 1: len 6; hex 000000002421; asc     $!;;
 2: len 7; hex 3600000183026d; asc 6     m;;
 3: len 4; hex 800007d0; asc     ;;

*** WE ROLL BACK TRANSACTION (2)
{% endhighlight %}

第 4 行是死锁发生的时间，可以通过该时间戳与应用程序的日志报错时间匹配，这样可以找到事务中对应的语句。

<!--
第 3 和 12 行，注意事务的序号和活跃时间。如果你定期地把 show engine innodb status 的输出信息记录到日志文件(这是一个很好的做法)，那么你就可以使用事务编号在之前的输出日志中查到同一个事务中所希望看到的更多的语句。活跃时间提供了一个线索来判断这个事务是单个语句的事务，还是包含多个语句的事务。

第 4 和 13行，使用到的表和锁只是针对于当前的语句。因此，使用到一张表，并不意味着事务仅仅涉及到一张表。

第 5 和 14 行，这里的信息需要重点关注，因为它告诉我们事务做了多少的改变，也就是 “undo log entries”；”row lock(s)” 则告诉我们持有多少行锁。这些信息都会提示我们这个事务的复杂程度。

第 6 和 15 行，留意线程 ID、连接主机和用户。如果你在不同的应用程序中使用不同的 MySQL 用户，这将是另外一个好的习惯，这样你就可以根据连接主机和用户来定位到事务来自于哪个应用程序。

第 9 行，对于第一个事务，它只是显示了处于锁等待状态，在这个例子中，是表 t3 的 X 锁。其他的可能：共享锁(S)，有间隙锁(gap lock)的排他锁(X)，及没有间隙锁(gap lock)的排他锁(X)，及AUTO_INC 。

第 9 和 10 行：”space id” 是表空间ID，”page no” 指出了这个表空间里面记录锁所在的数据页，”n bits” 不是数据页偏移量，而是锁位图里面的 bits 数。在第 10 行记录的 “heap no” 是数据页偏移量。然后第 10 行下面的数据显示了记录数据的十六进制编码。字段 0 表示聚集索引（即主键），忽略最高位，值为 5。字段 1 表示最后修改这条记录的事务的ID号，上面实例中的十进制值是 25205268，即是 TRANSACTION (2)。字段 2 表示回滚指针。从字段 3 开始，表示的是余下的行数据。通过阅读这些信息，我们可以准确知道哪一行被锁了，哪些是当前值。

第 17 和 18 行，对于第二个事务，显示了它持有的锁，在本示例中，是事务1 (TRANSACTION (1)) 所请求并等待中的 X 锁。

第 20 和 21 行，显示了事务2 (TRANSACTION (2)) 所等待的锁的信息。在本例中，是t3表产生的 X 锁。
-->

## 死锁案例

<!--
事务1与事务2第一条语句都是查询A，然后事务1对A进行更新操作，但是由于事务2的读锁对A还没有释放，所以事务1要等待；如果此时事务2也要对A进行更新操作，由于事务1对A的读锁还没有释放，所以事务2要等待。此时发生了什么？就是死锁。

那么为什么我们数据库里面不会出现这种情况呢？其实原因就是我们使用了U锁，U锁很简单就是会提前判断你这个事务中有没有针对一个事务的写操作，如果检查到有写锁，那么它会提前在你申请锁的时候把原来的读锁变成写锁。当锁变成写锁之后其他的读写操作都无进来这个事务内了，也就避免了死锁。





另外，在InnoDB中有几种少数情况会产生共享记录锁：

1) 使用了 SELECT … LOCK IN SHARE MODE 的语句

2) 外键引用记录

3) 源表上的共享锁，使用了 INSERT INTO… SELECT 的语句

这些信息结合着其他数据可以帮助开发人员定位到那个事务。

我们还可以从哪里找到事务之前的语句？

除了应用程序日志和之前的 show engine innodb status 的输出信息外，还可以利用 binlog、low log，甚至是general log。

通过 binlog，如果 binlog_format = statement，binlog 中的每个 event 都会拥有一个 thread_id。只有已提交的事务会被记录到 binlog 中，


类似于上述示例中的死锁
Example 1: Two Transactions Updating Two Records In Two Tables
Example 2: Two Transactions Updating Two Records In One Table





锁跟索引的关系

这时我们要注意到，money表虽然没有添加索引，但是InnoDB存储引擎会使用隐式的主键来进行锁定。对于没有索引或主键的表来说，那么MySQL会给整张表的所有数据行的加行锁。这里听起来有点不可思议，但是当sql运行的过程中，MySQL并不知道哪些数据行是id=1（没有索引嘛），如果一个条件无法通过索引快速过滤，存储引擎层面就会将所有记录加锁后返回，再由MySQL Server层进行过滤。但在实际使用过程当中，MySQL做了一些改进，在MySQL Server过滤条件，发现不满足后，会调用unlock_row方法，把不满足条件的记录释放锁 (违背了二段锁协议的约束)。这样做，保证了最后只会持有满足条件记录上的锁，但是每条记录的加锁操作还是不能省略的。可见即使是MySQL，为了效率也是会违反规范的。这种情况同样适用于MySQL的默认隔离级别RC。所以对一个数据量很大的表做批量修改的时候，如果无法使用相应的索引，MySQL过滤数据的的时候特别慢，就会出现虽然没有修改某些行的数据，但是它们还是被锁住了的现象。


http://www.ywnds.com/?p=4949
http://www.gpfeng.com/?p=426
http://www.cnblogs.com/LBSer/p/5183300.html
-->



{% highlight text %}
{% endhighlight %}
