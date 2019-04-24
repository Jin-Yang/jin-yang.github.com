---
title: PostgreSQL 源码分析
layout: post
comments: true
language: chinese
category: [database,postgresql,linux]
keywords: postgresql
description:
---

<!-- more -->


<!--
在 src/tools 目录下有 make_ctags 脚本，可以用来生成 tags 文件，直接执行 tools/make_ctags 即可。

{% highlight text %}
contrib/    已经打包到源码中的三方插件

src/backend/
           |-main/    会检查进程的参数，并传递给postmaster或者postgres
{% endhighlight %}


# pg_ctl

其源码在 bin/pg_ctl 目录下，通常需要执行如下的操作，而该文件实际上就是通过调用其它程序执行。

{% highlight text %}
----- 初始化一个cluster，等价于initdb -D /pg/data/dir
$ pg_ctl -D /pg/data/dir initdb

----- 启动
$ pg_ctl -D /pg/data/dir -l logfile start

----- 停止
$ pg_ctl -D /pg/data/dir stop
{% endhighlight %}

以启动为例。

pg_ctl start
 |-do_start()
   |-start_postmaster()
     |-system("postgres")
-->





## Backend

### postmaster

通常这个是执行 postgres 默认启动的程序，真正的入口在 `main` 目录下。

PG 启动后 postmaster 进入无限循环，等待客户端请求并为之提供服务，通过 select 定期检查是否有客户端服务请求，如果没有则继续循环，有就创建一个 postgres 子进程为其提供服务。

{% highlight text %}
PostmasterMain()
 |-AllocSetContextCreate()               # 初始化内存上下文
 |-InitializeGUCOptions()                # 初始化GUC
 |-SelectConfigFiles()                   # 查找PGDATA目录，加载配置文件
 | |-ProcessConfigFile()
 |   |-ProcessConfigFileInternal()
 |     |-ParseConfigFile()               # 这个是通过flex生成的文件
 |       |-ParseConfigFp()
 |
 |-SysLogger_Start()                     # 启动日志收集进程
 |
 |-StartupDataBase()                     # 各种启动进程实际都是宏定义
 | |-StartChildProcess(StartupProcess)   # 启动startup进程
 |   |-AuxiliaryProcessMain()            # 包括walwriter, walreceiver等进程的入口函数
 |     |-StartupProcessMain()            # startup进程入口
 |       |-StartupXLOG()
 |
 |-ServerLoop()
   |-select()
   |-ConnCreate()
   |-BackendStartup()                    # 启动一个服务进程
   | |-fork_process()
   | |-InitPostmasterChild()             # 子进程
   | |-ClosePostmasterPorts()
   | |-BackendInitialize()
   | |-BackendRun()
   |   |-PostgresMain()
   |
   |-StreamClose()
   |-ConnFree()
{% endhighlight %}

<!-- 关于配置文件的解析  http://blog.chinaunix.net/uid-24774106-id-3565790.html -->

<!--
## postgres

{% highlight text %}
PostgresMain()
  |-for(;;)                      ....
  |-exec_simple_query()            # Q，简单查询
{% endhighlight %}


## checkpointer

该进程同样时 postmaster 负责创建的，当该进程挂掉之后，为了保证数据的一致性，会杀掉所有 backend 进程，然后类似系统初始化一样逐一恢复。


CheckpointerShmemStruct;




PG 中有资源管理器的说法，会把各种需要记录日志的数据分类，并分配相应的资源管理器号，在恢复或者读取日志记录时，能够通过资源管理器号和日志头信息中的 xl_info 字段（高4位），知道数据库对源数据做了哪种操作，从而正确的调用相应的响应函数，在 rmgrlist.h 中定义，如下：
{% highlight c %}
PG_RMGR(RM_XLOG_ID, "XLOG", xlog_redo, xlog_desc, xlog_identify, NULL, NULL)
PG_RMGR(RM_XACT_ID, "Transaction", xact_redo, xact_desc, xact_identify, NULL, NULL)
PG_RMGR(RM_SMGR_ID, "Storage", smgr_redo, smgr_desc, smgr_identify, NULL, NULL)
PG_RMGR(RM_CLOG_ID, "CLOG", clog_redo, clog_desc, clog_identify, NULL, NULL)
{% endhighlight %}

可以通过 pg_xlogdump --rmgr=list 命令查看当前系统支持的资源管理器。







看过的一篇文章，pg_xlog 目录增长到了 4G，开始可能会怀疑是日志归档过慢，导致堆积在 pg_xlog 目录下，未被清除导致。但是，在 archive_status 目录下每个日志都有对应的 .done 文件，也就是都被归档完成了。

ls -lhrt pg_xlog
-rw------- 1 xxx xxx 16M Jun 14 18:39 0000000100000035000000DE
-rw------- 1 xxx xxx 16M Jun 14 18:39 0000000100000035000000DF
-rw------- 1 xxx xxx 16M Jun 14 18:39 0000000100000035000000E0
drwx------ 2 xxx xxx 72K Jun 14 18:39 archive_status

ls -lrt pg_xlog/archive_status
-rw------- 1 xxx xxx 0 Jun 14 18:39 0000000100000035000000DE.done
-rw------- 1 xxx xxx 0 Jun 14 18:39 0000000100000035000000DF.done
-rw------- 1 xxx xxx 0 Jun 14 18:39 0000000100000035000000E0.done

另外，奇怪的是，pg_xlog 里有些日志文件对应了还没产生的日志号，如上在被写入的日志号为 100000035000000E0 左右，却出现了名为 1000000360000000C 的日志文件名，而且修改时间并非最新创建或修改过。

ls -lrt pg_xlog
-rw------- 1 xxx xxx 16777216 Jun 14 17:37 00000001000000360000000C
-rw------- 1 xxx xxx 16777216 Jun 14 17:37 000000010000003600000014

接下来看看，为什么会出现这种情况？正常的日志空间大小是多少？

## PG日志创建以及清理机制

首先，需要先摸清 PG 日志的创建、保持和清理机制，与此相关的包括了：日志写入 (WAL writer) 进程、日志归档 (archiver) 进程、检查点 (checkpointer) 进程和日志发送进程 (WAL sender) 。

其中 WAL writer 负责异步把 WAL 日志刷入磁盘；与此同时，其他普通后台进程，也可能会同步的将WAL日志刷入磁盘，我们先从分析它们入手。从代码里面不难看出，它们将日志写入新的日志文件时，有如下函数调用：

XLogWrite()
 |-XLogFileInit()
   |-BasicOpenFile()

BasicOpenFile() 负责打开一个新的日志文件，如果文件不存在，则新建文件。在 XLogFileInit() 代码中，有如下的注释，"Try to use existent file (checkpoint maker may have created it already)"，文件可能已经被 checkpointer 进程创建了。

那么 checkpointer 的处理流程是什么样子的呢？其主函数 CheckpointerMain()，在 for(;;) 循环中的逻辑如下：

检查是否有checkpoint request信号；
检查是否checkpoint timeout时间已到；
调用CreateCheckPoint做检查点操作；
调用WaitLatch等待checkpoint timeout或checkpoint request信号。

重点内容都在 CreateCheckPoint() 函数中，其逻辑如下：

检查上次检查点后是否有WAL日志写入，如果没有直接返回；
调用CheckPointGuts将WAL日志fsync到磁盘；注意其中的CheckPointBuffers函数，会根据checkpoint_completion_target的值做一定的delay，使fsync操作的完成时间占两个检查点之间时间间隔的比例，约为checkpoint_completion_target；
在WAL中插入检查点日志信息；
取系统前一次检查点的日志位置指针，即此指针之前的日志文件，都可以删除了；
由KeepLogSeg根据wal_keep_segments和replication slot的情况计算要额外保留的日志；
由RemoveOldXlogFiles做真正的日志删除，而神奇的是RemoveOldXlogFiles并未实际删除文件，而是将其回收，即将老文件rename成新文件，做了日志文件预分配；
完成检查点返回。

可以看到，在这里出现日志删除、预分配等逻辑。也就是说PG的日志文件可能是在做检查点操作时预分配的！预分配的文件名使用了“未来”的目前还不存在的日志号，这就解释了我们之前遇到的“幽灵日志”情况，也回答了我们的第一个问题。

当然，需要说明的是，日志的保留和删除还和是否被archiver进程归档成功有关。

## 日志空间大小

继续看第二个问题。前面提到的日志空间暴增让我们如临大敌，那么PG日志到底最多会占用多少空间？我们遇到的涨到3G情况正常吗？

从日志清理逻辑（重点是KeepLogSeg和RemoveOldXlogFiles函数）的分析，我们得到下面的结论：

日志的删除和预分配只在检查点刚完成时进行；
删除时，保证上一次检查点之后到现在的日志不会被删除；
保证从目前日志位置往前wal_keep_segments个日志文件不会删除；
预分配的过程是，对所有不再需要的旧文件重命名为一个未来的日志号，直到预分配的文件数量达到XLOGfileslop，即2*checkpoint\_segments + 1。checkpoint_segments为一个可配置的参数，控制了两个检查点间产生的日志文件数量。

另外，为讨论方便，下面我们先做如下假设：

有足够多（即大于2*checkpoint_segments + 1）的不再需要的旧日志文件，可以用于预分配；
每次检查点操作完成的时间，正好占两个检查点之间时间间隔的checkpoint_completion_target（线上目前我们设为0.9）。

设某次检查点操作完成时的时间点为A，则此时做日志预分配的情形如下图所示：

检查点完成时的日志预分配

候选被回收的文件是在时间点C之前的、并且大于wal_keep_segments个文件间隔的文件；这些文件将重命名为预分配文件，文件号为从A对应的日志开始递增，直到达到2*checkpoint_segments + 1个文件为止。

做检查点操作过程中，是不断产生新日志文件的，而且两次检查点之间的日志文件数为一个稳定的值，即checkpoint_segments。因此，在时间点B到A之间产生的日志数约为checkpoint_segments * checkpoint_completion_target。

待A时间点预分配完日志文件，并删除其他不需要的日志后，新产生的日志将使用预分配空间，日志空间不会增大，日志空间大小达到一个稳定状态。而此时日志的空间至少为：保留的日志空间 + 预分配空间 + 正在被写入的那个文件，即为：

max(wal_keep_segments, checkpoint_segments + checkpoint_segments*checkpoint_completion_target) + 2 * checkpoint_segments + 1 + 1

这就是在日志大小达到稳定状态时，所能达到的最大值。所谓“稳定状态”是指，一旦达到这个状态，优先使用预分配空间，一般不会增大；即使日志文件继续增加，也会被删除（如果archiver和wal sender都正常工作的话）。而日志大小也不会明显减少，因为处于预分配状态的日志数量、前一次检查点到当前时间点的日志量都没有大的变化。

回到我们的问题，PG的日志空间占用的正常值，可以用上面的公式计算出来。如果wal_keep_segments为80，checkpoint_segments为64，checkpoint_completion_target为0.9，那么根据公式计算结果为4.02G。即日志空间增加到4G也是正常的。并且可以通过减小checkpoint_segements的值，减少日志空间占用。
几个问题

通过上面分析得出的公式，我们在处理日志时遇到的一些问题就迎刃而解了，例如：

Q: 增加wal_keep_segments会增大日志空间吗？
A: 如果增加wal_keep_segments后，其值仍小于（checkpoint_segments + checkpoint_segments * checkpoint_completion_target），则增加wal_keep_segments并不会增大日志占用空间。

Q: checkpoint_segments与日志空间大小有什么关系？
A: 在wal_keep_segments较小时，checkpoint_segments对日志空间占用有至关重要的影响。日志空间大小基本上可以用4倍checkpoint_segments来估算出来。但当wal_keep_segments较大时，比如是checkpoint_segments的10倍，则checkpoint_segments对日志空间大小的影响相对就小很多了。
思考题

上面的分析中，我们做了两点假设。一个是系统中有足够多的旧日志可供回收，这种情况会出现吗（提示：archiver进程或replication slot对日志删除的影响）？另一个是，检查点操作会及时完成，那么如果检查点操作未及时完成，会出现什么情况？会导致日志空间占用比我们的公式更大吗？









# CLOG (Commit Log)

事务状态日志（pg_clog


clog 用来记录事物最终状态的日志，是 pg_xlog 的辅助日志，放在数据库目录的 pg_clog 下，其中事务号是一个 32bits 整数，有三个是比较特殊的：

{% highlight c %}
#define InvalidTransactionId  ((TransactionId) 0)
#define BootstrapTransactionId  ((TransactionId) 1)
#define FrozenTransactionId   ((TransactionId) 2)
#define FirstNormalTransactionId ((TransactionId) 3)
#define MaxTransactionId   ((TransactionId) 0xFFFFFFFF)
{% endhighlight %}

事务有四种状态。

#define TRANSACTION_STATUS_IN_PROGRESS  0x00
#define TRANSACTION_STATUS_COMMITTED  0x01
#define TRANSACTION_STATUS_ABORTED   0x02
#define TRANSACTION_STATUS_SUB_COMMITTED 0x03



事务提交时，就可以把事务的状态存在一个独立的日志文件中，文件中每两位在文件中的位置对应事务ID

实际的实现中，是通过一个LRU算法，从共享内存分出8个页面来实现的。






## PG用户管理

PG View管理
http://www.postgresqltutorial.com/postgresql-views/
创建Monitor用户所需权限
https://pganalyze.com/docs/install/amazon_rds/02_create_monitoring_user

----- 通过内部表或者命令查看当前所有用户
SELECT * FROM pg_roles;
SELECT * FROM pg_user;
\du




gsql -W Manager@123 postgres


gsql -U monitor -W UAgent@GauSSdb#321


 postgres -c "show archive_command"


----- 常见用户管理操作
CREATE ROLE monitor WITH LOGIN NOSUPERUSER NOCREATEDB NOCREATEROLE PASSWORD 'your-password' CONNECTION LIMIT 10;
GRANT CONNECT ON DATABASE dbname TO monitor;
ALTER USER monitor WITH PASSWORD 'your-password';
REVOKE ALL ON pg_stat_activity FROM monitor;
DROP USER monitor;

----- 查看数据库及其对应的用户权限
\l

----- 注意其中的部分字段需要superuser权限，否则无法查看
SELECT * FROM pg_stat_activity;
\d pg_stat_activity;

----- 对于monitor用户，可以通过如下的方式进行规避
CREATE FUNCTION get_stat_activity() RETURNS SETOF pg_stat_activity AS 'SELECT * FROM pg_stat_activity;' LANGUAGE 'SQL' SECURITY DEFINER;
CREATE FUNCTION get_stat_replication() RETURNS SETOF pg_stat_replication AS 'SELECT * FROM pg_stat_replication;' LANGUAGE 'SQL' SECURITY DEFINER;
SELECT XACT_START, STATE FROM get_pg_stats();
GRANT ALL on FUNCTION get_pg_stats() TO monitor;
SELECT * FROM pg_stat_replication;
\sf get_stat_activity
\sf get_stat_replication

----- 当前PG的连接数，可以通过\d查看表结构
SELECT * FROM pg_stat_activity;
SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE state='idle';
----- 最大连接数限制
SHOW max_connections;
----- 超级用户保留的连接数
SHOW superuser_reserved_connections;

rpm -ivh --force uagent-1.

ALTER ROLE monitor SUPERUSER;
ALTER USER monitor SET default_transaction_read_only = ON;

在 backend/utils/fmgrtab.c 代码中，内置了对应的函数，例如 pg_stat_get_backend_activity() 。


## PG源码解析

PG 使用了一堆的 HOOK 函数，例如 ExecutorStart_hook() ，通过该机制可以允许用户切换到 PG 的内部运行机制，运行中断、增加或修改原来程序逻辑。

### 修改pg_stat_activity视图

可以通过 `\d+ pg_stat_activity` 查看视图的定义，这里尝试在 pg_stat_activity 视图中添加一个 queryid 字段，以方便与 pg_stat_statements 视图做关联。

视图实际通过 pg_stat_get_activity() 函数获取内存中的所有会话信息，而在 src/include/nodes/parsenodes.h 中的 struct Query 结构体中有 queryId 但是没有添加，只能是通过插件完成。

在 pg_analyze_and_rewrite_params() 函数中，可以通过调用钩子函数 post_parse_analyze_hook() 完成特定的动作，


src/backend/utils/adt/pgstatfuncs.c:641
if (superuser() || beentry->st_userid == GetUserId()) 代码写死判断，只有 superuser 或者本用户创建的链接才有权限查询，这也就意味着，无法通过类似 GRANT SELECT ON pg_catalog.pg_stat_activity TO monitor; 这样的 SQL 去添加读取权限。


# 参考

PG 及其代码结构
http://blog.csdn.net/beiigang/article/details/7596222
-->



{% highlight text %}
{% endhighlight %}
