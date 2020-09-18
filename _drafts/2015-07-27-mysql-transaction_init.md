---
title: MySQL 事务处理
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,事务,源码,XA,TM,RM
description: 事务处理是当前大多数数据库所需的，而不同的数据库实现方式又略有区别，在此介绍一下 MySQL 的实现方式，包括了 XA、。
---

事务处理是当前大多数数据库所需的，而不同的数据库实现方式又略有区别，在此介绍一下 MySQL 的实现方式，包括了 XA、。


<!-- more -->

## 简介

首先，介绍一下一些常见的概念，也就是 XA 。

XA (transaction accordant) 是由 X/Open 组织提出的分布式事务的规范，主要定义了全局事务管理器 (Transaction Manager, TM) 和局部资源管理器 (Resource Manager, RM)之间的接口。

MySQL XA 分为了内部 XA 与外部 XA 两类，内部 XA 用于同一实例下跨多个引擎的事务，由 binlog 作为协调者；外部 XA 用于跨多 MySQL 实例的分布式事务，需要应用层介入作为协调者，包括崩溃时的悬挂事务、全局提交还是回滚。

通常会将事务的提交分成了两个阶段，也就是两阶段提交 (tow phase commit, 2PC)：

1. Prepare 阶段<br>第一阶段，事务管理器向所有涉及到的数据库服务器发出 "准备提交"(prepare) 请求，数据库收到请求后执行数据修改和日志记录等处理，处理完成后只是把事务的状态改成 "可以提交"，然后把结果返回给事务管理器。

2. Commit 阶段<br>第二阶段，在第一阶段中所有数据库都提交成功，那么事务管理器向数据库服务器发出 "确认提交" 请求，数据库服务器把事务的 "可以提交" 状态改为 "提交完成" 状态，然后返回应答。

如果在第一阶段内有任何一个数据库的操作发生了错误，或者事务管理器收不到某个数据库的回应，则认为事务失败，回撤所有数据库的事务；数据库服务器收不到第二阶段的确认提交请求，也会把 "可以提交" 的事务回滚。


## 外部 XA 事务

也就是 MySQL 的跨库事务，或者称之为分布式事务，注意必须使用 serializable 隔离级别，而且只有 Innodb 支持。

{% highlight text %}
XA {START | BEGIN} xid         // 开始一个事务，并将事务置于ACTIVE状态，此后执行的SQL语句都将置于该事务中
XA END xid                     // 将事务置于IDLE状态，表示事务内SQL操作完成
XA COMMIT xid [ONE PHASE]      // 事务最终提交，完成持久化，事务完成；ONE PHASE将prepare和commit一步完成
XA PREPARE xid                 // 实现事务提交的准备工作，事务状态置于PREPARED状态
XA ROLLBACK xid                // 事务回滚并终止
{% endhighlight %}

对于一个应用大致的操作示例如下。

{% highlight text %}
xa begin 'xa-trans';           // A1，两个数据库分别创建两个分布式事务
xa start 'xa-trans';           // A2

... ...                        // 向两个库表中分别写入数据

xa end 'xa-trans';             // A1，SQL操作完成
xa end 'xa-trans';             // A2

xa prepare 'xa-trans';         // A1，事务准备提交
xa prepare 'xa-trans';         // A2

xa commit 'xa-trans';          // A1，事务最终提交
xa commit 'xa-trans';          // A2
{% endhighlight %}


## 内部 XA 事务

MySQL 为了兼容其它非事物引擎的复制，在 server 层引入了 binlog, 它可以记录所有引擎中的修改操作，因而可以对所有的引擎使用复制功能。

另外，由于 MySQL 采用插件式存储架构，导致开启 binlog 后，事务提交实际是采用 2PC 来保证 binlog 和 redolog 的一致性，以及事务提交和回滚等；这也就是内部 XA 事务。

MySQL 通过 WAL (Write-Ahead Logging) 方式，来保证数据库事务的一致性 (consistent) 和持久性 (durability)；这是一种实现事务日志的标准方法，具体而言就是修改记录前，一定要先写日志；事务提交过程中，一定要保证日志先落盘，才能算事务提交完成。

在提交过程中，主要做了如下的几件事情：

1. 是清理 undo 段信息，对于 innodb 存储引擎的更新操作来说，undo 段需要 purge，这里的 purge 主要职能是，真正删除物理记录。在执行delete或update操作时，实际旧记录没有真正删除，只是在记录上打了一个标记，而是在事务提交后，purge线程真正删除，释放物理页空间。因此，提交过程中会将undo信息加入purge列表，供purge线程处理。

2. 然后是释放锁资源，mysql通过锁互斥机制保证不同事务不同时操作一条记录，事务执行后才会真正释放所有锁资源，并唤醒等待其锁资源的其他事务；

3. 再就是刷redo日志，前面我提到了，mysql实现事务一致性和持久性的机制。通过redo日志落盘操作，保证了即使修改的数据页没有即使更新到磁盘，只要日志是完成了，就能保证数据库的完整性和一致性；

4. 最后就是清理保存点列表，每个语句实际都会有一个savepoint(保存点)，保存点作用是为了可以回滚到事务的任何一个语句执行前的状态，由于事务都已经提交了，所以保存点列表可以被清理了。

其中相关的包括了 MySQL 的锁机制、purge 原理、redo 日志、undo 段等内容，其实都是数据库的核心。


1. prepare，第一阶段<br>binlog 不作任何操作。InnoDB prepare，包括的操作有 A) 持有 prepare_commit_mutex；B) 并且 write/sync redo log；C) 将回滚段设置为 Prepared 状态。

2. commit，第二阶段<br>包括 write/sync Binlog。InnoDB commit，在写入 COMMIT 标记后释放 prepare_commit_mutex。

其中，以 binlog 的写入与否作为事务提交成功与否的标志，innodb commit 标志并不是事务成功与否的标志。此时，事务崩溃恢复过程如下：

1. 崩溃恢复时，扫描最后一个 Binlog 文件，提取其中的 xid。

2. InnoDB 维持了状态为 prepare 的事务链表，将这些事务的 xid 和 binlog 中记录的 xid 做比较，如果在 binlog 中存在，则提交，否则回滚事务；从而可以让 InnoDB 和 binlog 中的事务状态保持一致。

在 prepare 阶段或者在 write/sync binlog 阶段崩溃，也会回滚；在写入 innodb commit 标志时崩溃，则恢复时，会重新对 commit 标志进行写入。


## 组提交

MySQL 5.1 中，如果同时打开 sync_binlog=1、innodb_flush_log_at_trx_commit=1，TPS 将会下降到几十；这是因为 InnoDB 提交事务时，不仅需要将 REDO 刷盘，还需要将 Binlog 刷盘，每个事务都需要 2 次 sync 操作。而机械磁盘的 IOPS 也就为几百，所以导致 InnoDB 的性能极差。

<!-- 当 sync_binlog=1 时，第二阶段中的 write/sync binlog 将会成为瓶颈，而且由于 innodb 还持有全局大锁 (prepare_commit_mutex: prepare 和 commit共用一把锁)，这会导致性能急剧下降。-->

为了保证主从库的一致性，必须要保证 Binlog 和 InnoDB 的一致性，即如果一个事务写入了 Binlog，InnoDB 中就必须提交该事务；相反，如果一个事务没有写入 Binlog，InnoDB 就不能提交该事务。

为此，在 MySQL 中的做法是：A) InnoDB 执行 Prepare，将 Redo 日志写磁盘；B) 将 Binlog 写磁盘；C) InnoDB 执行 Commit，将事务标记为提交。这样，可以保证 Binlog 和 InnoDB 的一致性，可以分三种情况考虑：

1. 在 InnoDB Prepare 阶段 Crash。MySQL 在启动时做崩溃恢复，InnoDB 会回滚这些事务，同时由于事务也没有写到 binlog，InnoDB 和 Binlog 一致。

2. 在 Binlog 写磁盘阶段 Crash。MySQL 在启动做崩溃恢复时，会扫描未成功提交的事务，和当时未成功关闭的 binlog 文件，如果事务已经 Prepare 了，并且也已经在 Binlog 中了，InnoDB 会提交该事务；相反，如果事务已经在 Prepare 中了，但是不在 Binlog 中，InnoDB 会回滚该事务。结果就是 InnoDB 和 Binlog 一致。

3. 在 InnoDB 执行 Commit 阶段 Crash。和 2 类似，由于事务已经成功 Prepare，并且存在 Binlog 文件中，InnoDB 在崩溃恢复时，仍然会提交该事务，确保 Binlog 和 InnoDB 一致。

也就是说，统一按照 binlog 的为准。在实现时，将 mysql_bin_log 作为 2 阶段提交的协调者 (详见 ha_commit_trans)。
<!-- 内部分别调用tc_log->prepare()和tc_log->commit()实现2阶段提交，这里的tc_log就是MySQL源码中的全局对象mysql_bin_log。 -->

于是，对 binlog 采用组提交 (注意，prepare 阶段没有变)，将该过程分为三个阶段。

1. flush stage，将各个线程的 binlog 从 cache 写到文件中;

2. sync stage 对binlog做fsync操作（如果需���的话；最重要的就是这一步，对多个线程的binlog合并写入磁盘）；

3. commit stage 为各个线程做引擎层的事务commit(这里不用写redo log，在prepare阶段已写)。每个stage同时只有一个线程在操作。(分成三个阶段，每个阶段的任务分配给一个专门的线程，这是典型的并发优化)

这种实现的优势在于三个阶段可以并发执行，从而提升效率。
<!-- (另外：5.7中引入了MTS：多线程slave复制，也是通过binlog组提交实现的，在binlog组提交时，给每一个组提交打上一个seqno，然后在slave中就可以按照master中一样按照seqno的大小顺序，进行事务组提交了。)-->




# 事务操作

MySQL 提供了多种方式来开启一个事务，最简单的就是以一条 BEGIN 语句开始，也可以以 START TRANSACTION 开启事务，你还可以选择开启一个只读事务还是读写事务。所有显式开启事务的行为都会隐式的将上一条事务提交掉。

所有显示开启事务的入口函数均为 trans_begin()，如下列出了几种常用的事务开启方式。

### BEGIN

与该命令等效的命令还有 "BEGIN WORK" 及 "START TRANSACTION"，下面仍以 BEGIN 为例。

当以 BEGIN 开启一个事务时，首先会去检查是否有活跃的事务还未提交，如果没有提交，则调用 ha_commit_trans() 提交之前的事务，并释放之前事务持有的 MDL() 锁。

执行 BEGIN 命令并不会真的去引擎层开启一个事务，仅仅是为当前线程设定标记，表示为显式开启的事务。


### START TRANSACTION READ ONLY

使用该选项开启一个只读事务，当以这种形式开启事务时，会为当前线程的 thd->tx_read_only 设置为 true。当服务端接受到任何数据更改的 SQL 时，都会直接拒绝请求，会返回如下的错误码，不会进入引擎层。

下面的文件会在编译的时候生成。

{% highlight cpp %}
// build/include/mysqld_ername.h
{ "ER_CANT_EXECUTE_IN_READ_ONLY_TRANSACTION", 1792, "Cannot execute statement in a READ ONLY transaction." },

// build/include/mysqld_error.h
#define ER_CANT_EXECUTE_IN_READ_ONLY_TRANSACTION 1792

// build/include/sql_state.h
{ ER_CANT_EXECUTE_IN_READ_ONLY_TRANSACTION,"25006", "" },
{% endhighlight %}

这个选项可以强约束一个事务为只读的，而只读事务在引擎层可以走优化过的逻辑，相比读写事务的开销更小，例如不用分配事务id、不用分配回滚段、不用维护到全局事务链表中。

这种方式从 5.6 版本开始引入，在该版本中将全局事务链表拆成了两个链表：一个用于维护只读事务，一个用于维护读写事务。这样我们在构建一个一致性视图时，只需要遍历读写事务链表即可。

不过在 5.6 版本中，InnoDB 并不具备事务从只读模式自动转换成读写事务的能力，因此需要用户显式的使用以下两种方式来开启只读事务：A) 执行 START TRANSACTION READ ONLY；B) 将变量 tx_read_only 设置为 true 。

5.7 版本引入了模式自动转换的功能，但该语法依然保留了。

<!--
另外一个有趣的点是，在5.7版本中，你可以通过设置session_track_transaction_info变量来跟踪事务的状态，这货主要用于官方的分布式套件(例如fabric)，例如在一个负载均衡系统中，你需要知道哪些 statement 开启或处于一个事务中，哪些 statement 允许连接分配器调度到另外一个 connection。只读事务是一种特殊的事务状态，因此也需要记录到线程的Transaction_state_tracker中。

关于Session tracker，可以参阅官方WL#6631。
-->

### START TRANSACTION READ WRITE

用于开启读写事务，这也是默认的事务模式；如果当前实例的 read_only 打开了且当前连接不是超级账户，则显示开启读写事务会报错。

<!--
同样的事务状态TX_READ_WRITE也要加入到Session Tracker中。另外包括上述几种显式开启的事务，其标记TX_EXPLICIT也加入到session tracker中。

读写事务并不意味着一定在引擎层就被认定为读写事务了，5.7版本InnoDB里总是默认一个事务开启时的状态为只读的。举个简单的例子，如果你事务的第一条SQL是只读查询，那么在InnoDB层，它的事务状态就是只读的，如果第二条SQL是更新操作，就将事务转换成读写模式。
-->





<!--

START TRANSACTION WITH CONSISTENT SNAPSHOT
和上面几种方式不同的是，在开启事务时还会顺便创建一个视图（Read View），在InnoDB中，视图用于描述一个事务的可见性范围，也是多版本特性的重要组成部分。

这里会进入InnoDB层，调用函数innobase_start_trx_and_assign_read_view，注意只有你的隔离级别设置成REPEATABLE READ（可重复读）时，才会显式开启一个Read View，否则会抛出一个warning。

使用这种方式开启事务时，事务状态已经被设置成ACTIVE的。

状态变量TX_WITH_SNAPSHOT会加入到Session Tracker中。



AUTOCOMMIT = 0
当autocommit设置成0时，就无需显式开启事务，如果你执行多条SQL但不显式的调用COMMIT（或者执行会引起隐式提交的SQL）进行提交，事务将一直存在。通常我们不建议将该变量设置成0，因为很容易由于程序逻辑或使用习惯造成事务长时间不提交。而事务长时间不提交，在MySQL里简直就是噩梦，各种诡异的问题都会纷纷出现。一种典型的场景就是，你开启了一条查询，但由于未提交，导致后续对该表的DDL堵塞住，进而导致随后的所有SQL全部堵塞，简直就是灾难性的后果。

另外一种情况是，如果你长时间不提交一个已经构建Read View的事务，purge线程就无法清理一些已经提交的事务锁产生的undo日志，进而导致undo空间膨胀，具体的表现为ibdata文件疯狂膨胀。我们曾在线上观察到好几百G的Ibdata文件。

TIPS：所幸的是从5.7版本开始提供了可以在线truncate undo log的功能，前提是开启了独立的undo表空间，并保留了足够的 undo 回滚段配置（默认128个），至少需要35个回滚段。其truncate 原理也比较简单：当purge线程发现一个undo文件超过某个定义的阀值时，如果没有活跃事务引用这个undo文件，就将其设置成不可分配，并直接物理truncate文件。
-->

<!--其中与事务相关的数据结构如下。
<pre style="font-size:0.8em; face:arial;">
struct trx_t{

        trx_rseg_t* rseg;       /*!< rollback segment assigned to the
                    transaction, or NULL if not assigned

        trx_undo_t* insert_undo;    /*!< pointer to the insert undo log, or
                    NULL if no inserts performed yet */
    trx_undo_t* update_undo;    /*!< pointer to the update undo log, or
                    NULL if no update performed yet */
    const char* mysql_log_file_name;
                    /*!< if MySQL binlog is used, this field
                    contains a pointer to the latest file
                    name; this is NULL if binlog is not
                    used */
    ib_int64_t  mysql_log_offset;
                    /*!< if MySQL binlog is used, this
                    field contains the end offset of the
                    binlog entry */
}


/* The rollback segment memory object */
struct trx_rseg_t{
    /* Fields for update undo logs */
    UT_LIST_BASE_NODE_T(trx_undo_t) update_undo_list;
                    /* List of update undo logs */
    UT_LIST_BASE_NODE_T(trx_undo_t) update_undo_cached;
                    /* List of update undo log segments
                    cached for fast reuse */
    /*--------------------------------------------------------*/
    /* Fields for insert undo logs */
    UT_LIST_BASE_NODE_T(trx_undo_t) insert_undo_list;
                    /* List of insert undo logs */
    UT_LIST_BASE_NODE_T(trx_undo_t) insert_undo_cached;
                    /* List of insert undo log segments
                    cached for fast reuse */
}
</pre>
-->


## 源码详解

其中与事务提交相关的涉及到两个重要的参数，innodb_flush_log_at_trx_commit 和 sync_binlog，不同的模式区别在于，写文件调用 write 和落盘 fsync 调用的频率不同，所导致的后果是 mysqld 或 os crash 后，不严格的设置可能会丢失事务的更新。

双一模式是最严格的模式 (也就是上述参数均为一)，这种设置情况下，单机在任何情况下不会丢失事务更新；不过同时也会带来极大的性能影响。

所有显示开启事务的入口函数均为 trans_begin() 。

### 初始化

在实例启动时，会选择使用哪种 XA 方式，默认的就是 BINLOG 和 ENGINE 做 XA，如果 BINLOG 禁止了，则只用引擎做 XA。

{% highlight cpp %}
static int init_server_components()  // 初始化tc_log变量
{
    ... ...
    if (total_ha_2pc > 1 || (1 == total_ha_2pc && opt_bin_log))
    {
      if (opt_bin_log)
        tc_log= &mysql_bin_log;      // 打开binlog时，使用mysql_bin_log做2PC协调者
      else
        tc_log= &tc_log_mmap;        // 没有打开binlog，且存在超过两个支持2PC的引擎时
    }
    else
      tc_log= &tc_log_dummy;         // 不存在支持2PC的引擎，或只有1个支持2PC的引擎且没有打开binlog时
    ... ...
}
{% endhighlight %}

其中 total_ha_2pc 代表了支持 2PC 引擎的数目，在 ha_initialize_handlerton() 初始化各个存储接口时，如果发现引擎的 prepare() 函数被定义，就会 total_ha_2pc++。

需要注意的是 binlog 模块也算是支持 2PC 的引擎，也就是只要又一个存储引擎执行 preprare() 正常，则 total_ha_2pc 值就大于 1 。

* TC_LOG_DUMMY<br>直接调用引擎接口做 PREPARE/COMMIT/ROLLBACK，基本不做任何协调。

* MYSQL_BIN_LOG<br>实际上 binlog 接口不做 prepare，只做 commit，也就所有写入 binlog 文件的事务，在崩溃恢复时，都应该能够提交。

* TC_LOG_MMAP<br>使用 mmap() 方式映射到内存中，实现事务日志的策略。

上述类都是从 TC_LOG 这个纯虚类继承而来的，该类以及相关处理方法是日志子系统中为两阶段事务 (2PC) 提供的，并且在 binlog 中继承了该结构。

其中比较重要的方法包括了如下的几种：open() 打开日志文件、close() 关闭日志文件、log_xid() 记录 2PC 事务日志 id、unlog() 删除 2PC 事务日志 id。


### TC_LOG_MMAP

在此重点介绍 TC_LOG_MMAP，其中一个重要的结构体就是 st_page，使得日志按照页的方式进行组织和操作。

{% highlight cpp %}
class TC_LOG_MMAP: public TC_LOG {
  typedef struct st_page {
    struct st_page *next;            // 指向下一个日志页，链接成一个FIFO队列
    my_xid         *start, *end;     // 指向页的第一个和最后一个位置
    my_xid         *ptr;             // 下一个要记录xid的位置
    int            size, free;       // 日志页可以存储xid的数目以及空闲的存储空间数目
    int            waiters;          // 当前页处于条件等待的操作数目
    PAGE_STATE     state;            // 当前页的状态，主要包括PS_POOL、PS_ERROR、PS_DIRTY三种状态
    mysql_mutex_t  lock;             // 用于控制日志页的并发操作
    mysql_cond_t   cond;             // 等待sync()
  } PAGE;

  char           logname[FN_REFLEN];               // mmap的日志文件名
  File           fd;                               // 文件描述符
  my_off_t       file_length;                      // 日志文件的大小
  uint           npages;                           // 日志文件的页数，其中日志文件是按照页进行组织
  uint           inited;                           // 一个流程状态参数，用于表明当前操作所处的状态
  uchar          *data;                            // 用于存储xid数据内容
  struct st_page *pages;                           // 用于组织数据页
  struct st_page *syncing, *active, *pool;         // 日志页分为syncing、active、pool三种状态存在
  struct st_page **pool_last_ptr;                  // 该指针指向pool队列中的最后一个元素
  mysql_mutex_t  LOCK_active, LOCK_pool, LOCK_sync;// 三个锁结构，用于并发控制访问日志
  mysql_cond_t   COND_pool, COND_active;           // 该条件变量分别用于并发控制中，唤醒pool队列和active页访问
};
#define TC_LOG_PAGE_SIZE   8192
#define TC_LOG_MIN_SIZE    (3*TC_LOG_PAGE_SIZE)
{% endhighlight %}

其中日志页按照队列的方式进行组织，其中 TC_LOG 日志文件默认每页大小为 8K，文件最小为 3 页，可以根据配置文件和启动参数指定日志文件大小。

<!-- http://blog.chinaunix.net/uid-26896862-id-3527584.html -->

### 执行流程

无论对于 DML (update, delete, insert)、TCL (commit, rollback) 语句，MySQL 提供了公共接口 mysql_execute_command()，基本流程如下：

{% highlight cpp %}
mysql_execute_command(THD *thd) {
    switch (lex->sql_command) {
        case SQLCOM_INSERT:
            mysql_insert();
            break;
        case SQLCOM_UPDATE:
            mysql_update();
            break;
        case SQLCOM_BEGIN:
            trans_begin()
            break;
        case SQLCOM_COMMIT:
            trans_commit();
            break;
        ......
    }

    if (thd->is_error() || ...)  // 语句执行错误
        trans_rollback_stmt(thd);
    else
        trans_commit_stmt(thd);
}
{% endhighlight %}

可以看到执行任何语句最后，都会执行 trans_rollback/commit_stmt()，这两个调用分别是语句级提交和语句级回滚。

语句级提交，对于非自动模式提交情况下，主要作两件事情，一是释放autoinc锁，这个锁主要用来处理多个事务互斥地获取自增列值，因此，无论最后该语句提交或是回滚，该资源都是需要而且可以立马放掉的。二是标识语句在事务中位置，方便语句级回滚。

{% highlight text %}
CREATE TABLE a (
     id int(10) unsigned NOT NULL,
     weight int(10) unsigned default NULL,
     PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE TABLE b (
     id int(10) unsigned NOT NULL,
     weight int(10) unsigned default NULL,
     PRIMARY KEY (id)
) ENGINE=TokuDB;

INSERT INTO a VALUES(1, 70),(2, 80);
INSERT INTO b VALUES(1, 70),(2, 80);

SET autocommit = 0;
SHOW VARIABLES LIKE 'log_bin';
SET SESSION debug = 'd:t:i:o,/tmp/mysqld.trace';

######################################################################
### 最常见的，也即打开binlog，且只使用了InnoDB引擎
### 此时tc_log使用的是mysql_bin_log
log-bin = 1, a (innodb)

mysql> START TRANSACTION | BEGIN;
mysql_execute_command()                         SQLCOM_BEGIN
  |-trans_begin()
    |-trans_check_state()

mysql> INSERT INTO a VALUES(9, 100);
Query OK, 1 row affected (0.01 sec)
mysql_execute_command()                         SQLCOM_INSERT
 |-insert_precheck()
 | |-check_access()
 | |-check_grant()
 |-mysql_insert()

mysql> COMMIT;
Query OK, 0 rows affected (0.00 sec)
mysql_execute_command()                         SQLCOM_COMMIT
 |-trans_commit()
   |-trans_check_state()
   |-ha_commit_trans()                          stmt(false)
   | |-commit_owned_gtids()
   | |-tc_log->prepare()                        1. MYSQL_BINLOG::prepare()
   | | |-ha_prepare_low()
   | |   |-##### for()中循环中遍厉所有接口，在此也就是binlog+InnoDB
   | |   |-ht->prepare()                        调用存储引擎handlerton->prepare()
   | |   |                                      ### 注意，实际调用如下的两个函数
   | |   |-binlog_prepare()
   | |   |-innobase_xa_prepare()
   | |     |-check_trx_exists()
   | |     |-innobase_trx_init()
   | |
   | |-tc_log->commit()                         2. MYSQL_BIN_LOG::commit()
   |   |-ordered_commit()                       在此涉及到了binlog的组提交，详细《日志相关》
   |     |-change_stage()                       2.1 转为FLUSH_STAGE，写入binlog缓存
   |     |-process_flush_stage_queue()
   |     |-flush_cache_to_file()
   |     |
   |     |-change_stage()                       2.2 转为SYNC_STAGE，调用fsync()刷入磁盘
   |     |-sync_binlog_file()                   调用fsync()写入磁盘
   |     |
   |     |-change_stage()                       2.3 转为COMMIT_STAGE，提交阶段
   |     |-process_commit_stage_queue()
   |     | |-ha_commit_low()
   |     |   |-##### for()中循环中遍厉所有接口，在此也就是binlog+InnoDB
   |     |   |-ht->commit()                     调用存储引擎handlerton->commit()
   |     |   |                                  ### 注意，实际调用如下的两个函数
   |     |   |-binlog_commit()
   |     |   |-innobase_commit()
   |     |-process_after_commit_stage_queue()
   |     |-finish_commit()
   |
   |-trans_track_end_trx()











++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

log-bin = 0, a (innodb), b (tokudb)

mysql> START TRANSACTION | BEGIN;
Query OK, 0 rows affected (0.00 sec)
mysql_execute_command()                          SQLCOM_BEGIN
 |-trans_begin()                                 开启事务
 | |-trans_check_state()
 | |-ha_commit_trans()                           如果有未提交的事务，先提交掉
 |
 |-ha_commit_trans()
 |   |-ha_commit_trans()
 |       |-tc_log->commit()                       实际调用的是TC_LOG_MMAP::commit()
 |-MDL_context::release_transactional_locks()
 |- ... ...                                       将thd.server_status设置为SERVER_STATUS_IN_TRANS

mysql> UPDATE a SET weight = 30 WHERE id = 1;
Query OK, 1 row affected (0.00 sec)
Rows matched: 1  Changed: 1  Warnings: 0
<font color="blue">mysql_execute_command()                     SQLCOM_UPDATE</font>
  |-update_precheck()
  |-mysql_update()

mysql> UPDATE b SET weight = 30 WHERE id = 1;
mysql> COMMIT;
<font color="blue">mysql_execute_command()                     SQLCOM_COMMIT</font>
  |-trans_commit()



######################################################################
log-bin = 0, a (innodb)

mysql> START TRANSACTION | BEGIN;
Query OK, 1 row affected (0.01 sec)
mysql> INSERT INTO a VALUES(4, 100);
Query OK, 1 row affected (0.01 sec)
mysql_execute_command()                     SQLCOM_COMMIT
 |-insert_precheck()
 |-mysql_insert()
 | |-open_tables_for_query()
 | | |-open_tables()
 | |   |-open_and_process_table()
 | |     |-open_table()
 | |       |-my_hash_first_from_hash_value()
 | |       |-innobase_trx_init()
 | |       |-column_bitmaps_signal()
 | |
 | |-write_record()
 |   |-handler::ha_write_row()
 |     |-ha_innobase::write_row()
 |       |-row_ins
 |         |-row_ins_index_entry_step
 |           |-row_ins_clust_index_entry
 |             |-row_ins_clust_index_entry_low
 |
 |-trans_commit_stmt()
   |-ha_commit_trans()
     |-ha_commit_low()
       |-innobase_commit()

mysql> COMMIT;
Query OK, 1 row affected (0.01 sec)
mysql_execute_command()                     SQLCOM_COMMIT
 |-trans_commit
   |-commit_owned_gtids(...)
   |-ha_commit_trans
     |-ha_commit_low
       |-innobase_commit
         |-innobase_trx_init
         | |-THD::get_trans_pos
         |-innobase_commit_low()
         |-trx_commit_complete_for_mysql()





///////////////
log-bin = 1
sync_binlog = 1


------------------------------------------
mysql> start transaction;
Query OK, 0 rows affected (0.00 sec)
    trans_begin()
    trans_commit_stmt

mysql> insert into x4 values (10);
Query OK, 1 row affected (1 min 1.69 sec)
    trans_commit_stmt
        ha_commit_trans
              binlog_prepare
              innobase_xa_prepare
            ha_commit_one_phase
                binlog_commit(看起来什么也没做，仅仅做了cache_mngr->trx_cache.set_prev_position(MY_OFF_T_UNDEF);)
                    cache_mngr->trx_cache.set_prev_position(MY_OFF_T_UNDEF);
                innobase_commit(同样仅仅是做一些标记)

mysql> commit ;
    trans_commit
        ha_commit_trans
            binlog_prepare
            innodb_xa_prepare
                trx_prepare_for_mysql
                mysql_mutex_lock(&prepare_commit_mutex);
            MYSQL_BIN_LOG::log_xid
            ha_commit_one_phase
                binlog_commit(似乎木有做写binlog,莫非已经被group commit了。。。= =！ log_xid..)
                innobase_commit
                    innobase_commit_low
                    mysql_mutex_unlock(&prepare_commit_mutex);
                    trx_commit_complete_for_mysql(trx);
    trans_commit_stmt












trans_commit_stmt()
  |-ha_commit_trans()
      |-tc_log->commit()               未开启binlog和gtid，则调用TC_LOG_MMAP::commit()
          |                            开启binlog后，则调用MYSQL_BIN_LOG::commit()
          |
          |-check_trx_exists()         获取innodb层对应的事务结构
          |-innobase_trx_init()
          |
          |   #### 通过if判断需要事务提交
          |-innobase_commit_low()
          | |-trx_commit_for_mysql()
          |   |-trx_commit()
          |     |-trx_commit_low()
          |       |-trx_write_serialisation_history()  更新binlog位点，设置undo状态
          |       | |-trx_undo_update_cleanup()        供purge线程处理，清理回滚页
          |       |-trx_commit_in_memory()             释放锁资源，清理保存点列表，清理回滚段
          |         |-trx_flush_log_if_needed()        刷日志
          |
          |-trx_deregister_from_2pc()
          |-trx_commit_complete_for_mysql()         确定事务对应的redo日志是否落盘
          |   |-trx_flush_log_if_needed()<font color='red'>
          |       |-trx_flush_log_if_needed_low()
          |           |-log_write_up_to()           根据参数决定刷磁盘的时机</font>
          |
          |   #### 单个语句，且非自动提交
          |-lock_unlock_table_autoinc()      释放自增列占用的autoinc锁资源
          |-trx_mark_sql_stat_end()          标识sql语句在事务中的位置，方便语句级回滚


MYSQL_BIN_LOG::prepare()
   |-ha_prepare_low()
       |-ht->prepare()             实际调用如下的三个函数
       |-binlog_prepare()          engine
       |-innobase_xa_prepare()     engine
       |-trx_prepare_for_mysql()   mysql
           |-trx_start_if_not_started_xa()
           |   |-trx_start_if_not_started_xa_low()
           |       |-trx_start_low()
           |-trx_prepare()
               |-trx_undo_set_state_at_prepare()   设置undo段的标记为TRX_UNDO_PREPARED
               |-trx->state=TRX_STATE_PREPARED     设置事务状态为TRX_STATE_PREPARED
               |-trx_flush_log_if_needed()         将产生的redolog刷入磁盘






xa start 'pp'
    trans_xa_start
        trans_begin (隐含的commit任何当前的事务并释放锁)
            trans_check :(/* Conditions under which the transaction state must not change. */)
                (likely(value)、unlikely(value))
            ha_commit_trans()
            thd->mdl_context.release_transactional_locks()
    trans_commit_stmt

mysql> insert into x4 values(5);
    trans_commit_stmt
        ha_commit_trans
            binlog_prepare：空函数
            innobase_xa_prepare
                对于xa事务，仅仅标记为ended
        ……
            ha_commit_one_phase
                binlog_commit(该函数在每执行一次statement后)
                    cache_mngr->trx_cache.set_prev_position(MY_OFF_T_UNDEF)
                    //binlog_commit_flush_trx_cache：将binlog cache中的内容写到文件中。
                        //binlog_flush_cache
                innobase_commit （对这条语句而言，仅仅标记该语句结束了，而不做commit）

mysql> xa end  'pp';
    trans_commit_stmt
        after_commit


mysql> xa prepare 'pp';
    trans_xa_prepare
        ha_prepare
            binlog_prepare
            innobase_xa_prepare
                ……
                trx_prepare_for_mysql
                    trx_prepare_off_kernel-------根据my.cnf的选项决定是否刷新log到磁盘----做一次group commit

                当不为XA_PREPARE语句时。。。
                if (thd_sql_command(thd) != SQLCOM_XA_PREPARE
                    pthread_mutex_lock(&prepare_commit_mutex);
    trans_commit_stmt
        after_commit

mysql> xa commit 'pp';
    trans_xa_commit
92     /*
693     *  Acquire metadata lock which will ensure that COMMIT is blocked
694     * by active FLUSH TABLES WITH READ LOCK (and vice versa COMMIT in
695     *  progress blocks FTWRL).We allow FLUSHer to COMMIT; we assume FLUSHer knows what it does.
    */
        加锁……
        ha_commit_one_phase(thd, 1)
            binlog_commit
                binlog_commit_flush_trx_cache
                    binlog_flush_cache
            innobase_commit
                innobase_commit_low
                    trx_commit_for_mysql
                        trx_commit_off_kernel(看起来似乎是真的要提交了。。。。)


            if (trx->declared_to_be_inside_innodb) { //不知道为什么没有持有prepare_commit_mutex


            trx_commit_complete_for_mysql


log_write_up_to：（log0log.c）
实现group commit，这里仅考虑flushto_disk为真的情况，即需要向磁盘中fsync log
loop:
    当ut_dulint_cmp(log_sys->flushed_to_disk_lsn, lsn) >= 0
    即lsn小于或等于全局变量log_sys中的记录的上次刷新的lsn时，直接返回

    log_sys->n_pending_writes > 0
        if (flush_to_disk
           && ut_dulint_cmp(log_sys->current_flush_lsn, lsn)
           >= 0) {
            /* The write + flush will write enough: wait for it to
            complete  */


            goto do_waits;
        }



{% endhighlight %}


## 常见配置项


##### flush_log_at_trx_commit

在 trx_commit_complete_for_mysql() 函数中，会根据 flush_log_at_trx_commit 参数，确定 redo 日志落盘方式。

##### binlog_max_flush_queue_time(0) 单位为us

设置从 flush 队列中取事务的超时时间，防止并发事务过高，导致某些事务的 RT 上升，详细的实现参考 MYSQL_BIN_LOG::process_flush_stage_queue()。

##### binlog_order_commits(ON)
事务和binlog是否以相同顺序提交<br>

<!-- opt_binlog_order_commits -->
不以顺序提交时可以稍微提升点性能，但并不是特别明显。</li><br><li>

innodb_support_xa(ON) 是否启用 innodb 的 XA<br>
它会导致一次额外的磁盘flush(prepare阶段flush redo log). 但是我们必须启用，而不能关闭它。因为关闭会导致binlog写入的顺序和实际的事务提交顺序不一致，会导致崩溃恢复和slave复制时发生数据错误。如果启用了log-bin参数，并且不止一个线程对数据库进行修改，那么就必须启用innodb_support_xa参数。






## 事务ID

在 InnoDB 中一直维护了一个不断递增的整数，存储在 trx_sys->max_trx_id 中，该事务 ID 可以看做一个事务的唯一标识；每次开启一个新的读写事务时，都将该 ID 分配给事务，同时递增全局计数。

{% highlight text %}
trx_start_low()                # innobase/trx/trx0trx.cc
  |-trx_sys_get_new_trx_id()   # innobase/include/trx0sys.ic
{% endhighlight %}

在 MySQL5.6 及之前的版本中，总是为事务分配 ID，而实际上只有做过数据更改的读写事务，才需要去根据事务 ID 判断可见性。因此在 MySQL5.7 版本中，只有读写事务才会分配事务 ID，只读事务默认为 0 。

<!--
那么问题来了，怎么去区分不同的只读事务呢？这里在需要输出事务ID时（例如执行SHOW ENGINE INNODB STATUS 或者查询INFORMATION_SCHEMA.INNODB_TRX表），使用只读事务对象的指针或上一个常量来标识其唯一性，具体的计算方式见函数trx_get_id_for_print。所以如果你show出来的事务ID看起来数字特别庞大，千万不要惊讶。
-->

对于全局最大事务 ID，每做 TRX_SYS_TRX_ID_WRITE_MARGIN (256) 次修改后，就持久化一次到 ibdata 的事务页 (TRX_SYS_PAGE_NO) 中。

<!--
已分配的事务ID会加入到全局读写事务ID集合中（trx_sys->rw_trx_ids），事务ID和事务对象的map加入到trx_sys->rw_trx_set中，这是个有序的集合(std::set)，可以用于通过trx id快速定位到对应的事务对象。

事务分配得到的ID并不是立刻就被使用了，而是在做了数据修改时，需要创建或重用一个undo slot时，会将当前事务的ID写入到undo page头，状态为TRX_UNDO_ACTIVE。这也是崩溃恢复时，InnoDB判断是否有未完成事务的重要依据。

在执行数据更改的过程中，如果我们更新的是聚集索引记录，事务ID + 回滚段指针会被写到聚集索引记录中，其他会话可以据此来判断可见性以及是否要回溯undo链。
对于普通的二级索引页更新，则采用回溯聚集索引页的方式来判断可见性（如果需要的话）。关于MVCC，后文会有单独描述。
-->






{% highlight cpp %}
/** The transaction system central memory data structure. */
struct trx_sys_t{

    ib_mutex_t      mutex;      /*!< mutex protecting most fields in
                    this structure except when noted
                    otherwise */
    ulint       n_prepared_trx; /*!< Number of transactions currently
                    in the XA PREPARED state */
    ulint       n_prepared_recovered_trx; /*!< Number of transactions
                    currently in XA PREPARED state that are
                    also recovered. Such transactions cannot
                    be added during runtime. They can only
                    occur after recovery if mysqld crashed
                    while there were XA PREPARED
                    transactions. We disable query cache
                    if such transactions exist. */
    trx_id_t    max_trx_id; /*!< The smallest number not yet
                    assigned as a transaction id or
                    transaction number */
#ifdef UNIV_DEBUG
    trx_id_t    rw_max_trx_id;  /*!< Max trx id of read-write transactions
                    which exist or existed */
#endif
    trx_list_t  rw_trx_list;    /*!< List of active and committed in
                    memory read-write transactions, sorted
                    on trx id, biggest first. Recovered
                    transactions are always on this list. */
    trx_list_t  ro_trx_list;    /*!< List of active and committed in
                    memory read-only transactions, sorted
                    on trx id, biggest first. NOTE:
                    The order for read-only transactions
                    is not necessary. We should exploit
                    this and increase concurrency during
                    add/remove. */
    trx_list_t  mysql_trx_list; /*!< List of transactions created
                    for MySQL. All transactions on
                    ro_trx_list are on mysql_trx_list. The
                    rw_trx_list can contain system
                    transactions and recovered transactions
                    that will not be in the mysql_trx_list.
                    There can be active non-locking
                    auto-commit read only transactions that
                    are on this list but not on ro_trx_list.
                    mysql_trx_list may additionally contain
                    transactions that have not yet been
                    started in InnoDB. */
    trx_rseg_t* const rseg_array[TRX_SYS_N_RSEGS];
                    /*!< Pointer array to rollback
                    segments; NULL if slot not in use;
                    created and destroyed in
                    single-threaded mode; not protected
                    by any mutex, because it is read-only
                    during multi-threaded operation */
    ulint       rseg_history_len;/*!< Length of the TRX_RSEG_HISTORY
                    list (update undo logs for committed
                    transactions), protected by
                    rseg->mutex */
    UT_LIST_BASE_NODE_T(read_view_t) view_list;
                    /*!< List of read views sorted
                    on trx no, biggest first */
};

{% endhighlight %}



事务子系统维护了三个不同的链表，用来管理事务对象。

trx_sys->mysql_trx_list
包含了所有用户线程的事务对象，即使是未开启的事务对象，只要还没被回收到trx_pool中，都被放在该链表上。当session断开时，事务对象从链表上摘取，并被回收到trx_pool中，以待重用。

trx_sys->rw_trx_list
读写事务链表，当开启一个读写事务，或者事务模式转换成读写模式时，会将当前事务加入到读写事务链表中，链表上的事务是按照trx_t::id有序的；在事务提交阶段将其从读写事务链表上移除。

trx_sys->serialisation_list
序列化事务链表，在事务提交阶段，需要先将事务的undo状态设置为完成，在这之前，获得一个全局序列号trx->no，从trx_sys->max_trx_id中分配，并将当前事务加入到该链表中。随后更新undo等一系列操作后，因此进入提交阶段的事务并不是trx->id有序的，而是根据trx->no排序。当完成undo更新等操作后，再将事务对象同时从serialisation_list和rw_trx_list上移除。

这里需要说明下trx_t::no，这是个不太好理清的概念，从代码逻辑来看，在创建readview时，会用到序列化链表，链表的第一个元素具有最小的trx_t::no，会赋值给ReadView::m_low_limit_no。purge线程据此创建的readview，只有小于该值的undo，才可以被purge掉。

总的来说，mysql_trx_list包含了rw_trx_list上的事务对象，rw_trx_list包含了serialisation_list上的事务对象。

事务ID集合有两个：

trx_sys->rw_trx_ids
记录了当前活跃的读写事务ID集合，主要用于构建ReadView时快速拷贝一个快照

trx_sys->rw_trx_set
这是<trx_id, trx_t>的映射集合，根据trx_id排序，用于通过trx_id快速获得对应的事务对象。一个主要的用途就是用于隐式锁转换，需要为记录中的事务id所对应的事务对象创建记录锁，通过该集合可以快速获得事务对象


















{% highlight cpp %}
struct trx_t{
    ulint       magic_n;
    ib_mutex_t  mutex;
    trx_id_t    id;     /*!< transaction id */
};
{% endhighlight %}



http://blog.chinaunix.net/uid-26896862-id-3527584.html
http://mysql.taobao.org/monthly/2015/12/01/
http://mysql.taobao.org/monthly/2015/11/04/

<!--
https://www.felix021.com/blog/read.php?2102
-->

{% highlight text %}
{% endhighlight %}
