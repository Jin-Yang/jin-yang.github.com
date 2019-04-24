---
title: SQLite 使用简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: sqlite,c language,sample
description: SQLite 是一个开源的嵌入式关系数据库，一个简单无需配置的小型数据库，支持事物，在嵌入式设备或者小型应用中使用较多，例如 Android、Chrome、WeChat 等系统。它在 2000 年由 D. Richard Hipp 发布，用来减少应用程序管理数据的开销，可移植性好、很容易使用、高效而且可靠。

---

SQLite 是一个开源的嵌入式关系数据库，一个简单无需配置的小型数据库，支持事物，在嵌入式设备或者小型应用中使用较多，例如 Android、Chrome、WeChat 等系统。

它在 2000 年由 D. Richard Hipp 发布，用来减少应用程序管理数据的开销，可移植性好、很容易使用、高效而且可靠。

<!-- more -->

![SQLite logo]({{ site.url }}/images/databases/sqlite/logo.png "SQLite logo"){: .pull-center width="70%" }

## 简介

### 源码编译

最新版本的代码可以直接从 [www.sqlite.org](https://www.sqlite.org/download.html) 上下载，在 [README.md](https://sqlite.org/src/doc/trunk/README.md) 中有相关的介绍，例如编译、源码概览等。

SQLite 使用了 Fossil 做版本管理，历史版本可以通过 [taglist](https://sqlite.org/src/taglist) 中下载，选择对应的版本，进入 `check-in` 中的版本号，然后下载 ZIP 格式包即可。

{% highlight text %}
----- 暂时关闭TCL的支持，否则需要提供tclsh命令
./configure --disable-tcl
{% endhighlight %}

如上是关闭了 tcl 支持，如果需要，那么在编译前需要安装 `tcl` 包，在 CentOS 中可以通过 `yum install tcl` 安装。

### 安装

在 CentOS 中可以可以直接通过如下方式安装。

{% highlight text %}
# yum install sqlite
{% endhighlight %}

### 常见命令

{% highlight text %}
----- 直接新建一个文件名为foobar.db的数据库
$ sqlite foobar.db

----- 新建一个表
sqlite> create table foobar(id integer primary key, value text);

----- 直接插入部分数据
sqlite> insert into foobar(id, value) values(1, 'Micheal'), (2, 'Jenny'), (3, 'Francis');

----- 查看数据
sqlite> select * from foobar;
1|Micheal
2|Jenny
3|Francis

----- 设置查询返回的结果
sqlite> .mode column;                         # 按照列格式显示
sqlite> .header on;                           # 显示列名称
sqlite> select * from foobar;
id          value
----------- -------------
1           Micheal
2           Jenny
3           Francis

----- 添加列
sqlite> alter table foobar add column email text not null '' collate nocase;

----- 创建视图
sqlite> create view nameview as select value from foobar;
sqlite> select * from nameview;

----- 创建索引
sqlite> create index idx_value on foobar(value);

----- 查看帮助
sqlite> .help

----- 查看所有表，包括视图
sqlite> .tables

----- 显示表结构
sqlite> .schema [table|view]

----- 获取指定表的索引列表
sqlite > .indices [table]
idx_value

----- 导出数据库到SQL文件
sqlite > .output [filename ]
sqlite > .dump
sqlite > .output stdout

----- 备份、恢复数据库
$ sqlite foobar.db .dump > backup.sql
$ cat backup.sql | sqlite3 foobar-restore.db
$ sqlite3 foobar-restore.db < backup.sql
{% endhighlight %}

## C 编程

{% highlight c %}
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#define SQL_CREATE_TBL_DAEMON                                     \
	"CREATE TABLE IF NOT EXISTS daemon("                      \
		"name CHAR(64) PRIMARY KEY NOT NULL, "            \
		"version CHAR(64) NOT NULL, "                     \
		"gmt_modify NOT NULL DEFAULT CURRENT_TIMESTAMP, " \
		"gmt_create NOT NULL DEFAULT CURRENT_TIMESTAMP"   \
	");"

static int callback(void *non, int argc, char **argv, char **cols)
{
	(void) non;
	int i;

	for (i = 0; i < argc; i++)
		printf("%s = %s\n", cols[i], argv[i] ? argv[i] : "NULL");
	printf("\n");

	return 0;
}

int main(void)
{
	int rc;
	sqlite3 *db;
	sqlite3_stmt *res;
	char *sql, *errmsg;

	printf("Current SQLite version: %s\n", sqlite3_libversion());

	/* OR ":memory:" to create a memory database */
	rc = sqlite3_open("daemon.db", &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	printf("Opened SQLite handle successfully.\n");

	rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	rc = sqlite3_step(res);
	if (rc == SQLITE_ROW)
		printf("Got version from SQL: %s\n", sqlite3_column_text(res, 0));
	sqlite3_finalize(res);

	/* "INSERT INTO XXXX VALUES(1, 'Audi', 52642);" */
	rc = sqlite3_exec(db, SQL_CREATE_TBL_DAEMON, NULL, NULL, &errmsg);
	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
	printf("Create daemon table successfully.\n");

	sql = "INSERT INTO daemon(name, version) VALUES "
		"('MonitorAgent', 'V1.0.0'), ('SecurityAgent', 'V1.0.2');";
	rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
        printf("Insert data successfully\n");
	int lastid = sqlite3_last_insert_rowid(db);
	printf("The last Id of the inserted row is %d\n", lastid);


	/* SELECT DATETIME(gmt_modify, "localtime") FROM daemon; */
	sql = "SELECT * FROM daemon";
	rc = sqlite3_exec(db, sql, callback, NULL, &errmsg);
	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}

	char **result;
	int nrow, ncol;
	rc = sqlite3_get_table( db , sql , &result , &nrow , &ncol , &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
	for(rc = 0; rc < (nrow + 1 ) * ncol ; rc++ )
		printf( "result[%d] = %s\n", rc, result[rc]);
	sqlite3_free_table(result);


	rc = sqlite3_prepare_v2(db, "SELECT * FROM daemon", -1, &res, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	while((rc = sqlite3_step(res)) == SQLITE_ROW)
		printf("Got version from SQL: %s\n", sqlite3_column_text(res, 0));
		//printf("Got version from SQL: %d\n", sqlite3_column_int(res, 0));
	sqlite3_finalize(res);

	sql = "UPDATE daemon SET version = 'V2.0.1', gmt_modify = CURRENT_TIMESTAMP WHERE name = ?1;";
	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	sqlite3_bind_text(res, 1, "MonitorAgent", -1, SQLITE_STATIC);
        //sqlite3_bind_int(res, 1, 3);
	rc = sqlite3_step(res);
	if (rc == SQLITE_ROW)
		printf("%s: ", sqlite3_column_text(res, 0));
	sqlite3_finalize(res);

	sql = "UPDATE daemon SET version = 'V2.0.1', gmt_modify = CURRENT_TIMESTAMP WHERE name = 'MonitorAgent';";
	rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}

	sql = "DELETE FROM daemon";
	rc = sqlite3_exec(db, sql, NULL, 0, &errmsg);
	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
	sqlite3_close(db);

	return 0;
}
{% endhighlight %}

#### 关闭

在通过 `sqlite3_close()` 关闭时，如果有 `stmt` 没有关闭则会返回报错，可以通过如下方式关闭。

{% highlight c %}
void db_destroy(void)
{
        int rc = 0;
        sqlite3_stmt *stmt;

        if (db == NULL)
                return;

        rc = sqlite3_close(db);
        while(rc == SQLITE_BUSY) {
                stmt = sqlite3_next_stmt(db, NULL);
                if (stmt == NULL) {
                        NEW_INFO(LOG_CATE_DAEMON, "Destroy SQLite error, unexpect NULL");
                        sqlite3_close(db);
                        break;
                }

                if (sqlite3_finalize(stmt) == SQLITE_OK)
                        rc = sqlite3_close(db);
        }
        NEW_INFO(LOG_CATE_DAEMON, "Destroy SQLite(0x%lx) structure.", db);

        db = NULL;
}
{% endhighlight %}


## 并发访问控制

简单来说，在使用 SQLite 时，如果采用的是多线程或者多进程访问，一旦并发访问量过大，而又没有做并发控制，经常会遇到 `database is locked SQLITE_BUSY(5)` 的报错。

官网提供了 SQLite 并发模型的介绍，可以参考 [Using SQLite In Multi-Threaded Applications](https://www.sqlite.org/threadsafe.html) 。

<!--
### 多线程模型

SQLite 提供了 `Single Thread` `Multi Thread` 和 `serialized`(默认) 三种不同的线程安全模式，可以分别在编译(源码编译库)、启动(打开数据库)、运行(建立连接) 时进行控制。

可以参考...
-->

对于 SQLite 来说，只支持库级锁，也就是说，即使有两个事务分别对不同的表进行操作，那么这两个事物也是无法同时运行的，更不要说是元组级别的了。

当多个线程可以同时读数据库，但多个线程(多个链接)写入时就会发生冲突，也就是说 SQLite 实现的是多读单写。

### 重试机制

SQLite 提供了 Busy Retry 的方案，即发生阻塞时，会触发 Busy Handler，此时可以让线程休眠一段时间后，重新尝试操作，当重试一定次数依然失败后，则返回 `SQLITE_BUSY` 错误码。

提供两个 busy handle 函数 `sqlite3_busy_timeout()` `sqlite3_busy_handle()` 在并发访问失败时，可以进行重试，详细可以参考 [Register A Callback To Handle SQLITE_BUSY Errors](http://www.sqlite.org/c3ref/busy_handler.html) 。

也可以通过 `PRAGMA` 命令进行设置 `sqlite3_exec(db, "PRAGMA busy_timeout=times", 0, 0, err);` 等同于调用第一个 API 。

注意，对于同一个链接来说，只能有一个，两个会相互影响。

当然，这只能降低出现 `SQLITE_BUSY` 的概率，而不能彻底消除。

### BusyHandler

实际上，SQLite 内部提供了默认的处理函数 `sqliteDefaultBusyCallback` ，这里简单介绍对用户提供的 API 接口。

<!--
static int sqliteDefaultBusyCallback(
 void *ptr,               /* Database connection */
 int count                /* Number of times table has been busy */
){
  sqlite3 *db = (sqlite3 *)ptr;
  int timeout = ((sqlite3 *)ptr)->busyTimeout;
  if( (count+1)*1000 > timeout ){
    return 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  return 1;
}
-->

{% highlight c %}
int sqlite3_busy_handler(sqlite3 *, int (*)(void *, int), void *);
{% endhighlight %}

默认回调函数为 NULL ，此时会在申请不到锁时直接返回 BUSY。设置回调之后，当返回非 0 时会自动重试，否则返回 BUSY 。其中设置回调函数的第二个入参表示当前因 BUSY 事件调用该函数的次数。

{% highlight c %}
int sqlite3_busy_timeout(sqlite3*, int ms);
{% endhighlight %}

上述函数用来设置等待 BUSY 的超时时间，SQLite 会 sleep 并重试当前操作，如果失败则返回 BUSY ，这里的时间必须要大于 1s ，否则无效。为了方便自己控制时间，最好的方式还是用第一种。

### 总结

在 Retry 过程中，休眠时间的长短和重试次数，是决定性能和操作成功率的关键。

根据不同的场景，其最优值也会有所区别，若休眠时间太短或重试次数太多，会空耗 CPU 的资源；若休眠时间过长，会造成等待的时间太长；若重试次数太少，则会降低操作的成功率。


## 参考

[SQLite C tutorial](http://zetcode.com/db/sqlitec)

[Command Line Shell For SQLite](https://sqlite.org/cli.html) 。

<!--
一本不错的书  SQLite Database System Design and Implementation

SQLite加密方案实现
http://foggry.com/blog/2014/05/19/jia-mi-ni-de-sqlite/
https://www.jianshu.com/p/208200e0c465








## 线程锁 VS. 进程锁

作为有着十几年发展历史且被广泛认可的数据库，任何方案选择都是有其原因的，所以在完全理解由来之前，切忌盲目自信、直接上手修改。


因此，首先要了解SQLite是如何控制并发的。

SQLite是一个适配不同平台的数据库，不仅支持多线程并发，还支持多进程并发。它的核心逻辑可以分为两部分：

Core层。包括了接口层、编译器和虚拟机。通过接口传入SQL语句，由编译器编译SQL生成虚拟机的操作码opcode。而虚拟机是基于生成的操作码，控制Backend的行为。
Backend层。由B-Tree、Pager、OS三部分组成，实现了数据库的存取数据的主要逻辑。

在架构最底端的OS层是对不同操作系统的系统调用的抽象层。它实现了一个VFS（Virtual File System），将OS层的接口在编译时映射到对应操作系统的系统调用。锁的实现也是在这里进行的。

SQLite通过两个锁来控制并发。第一个锁对应DB文件，通过5种状态进行管理；第二个锁对应WAL文件，通过修改一个16-bit的unsigned short int的每一个bit进行管理。尽管锁的逻辑有一些复杂，但此处并不需关心。这两种锁最终都落在OS层的sqlite3OsLock、sqlite3OsUnlock和sqlite3OsShmLock上具体实现。

它们在锁的实现比较类似。以lock操作在iOS上的实现为例：

通过pthread_mutex_lock进行线程锁，防止其他线程介入。然后比较状态量，若当前状态不可跳转，则返回SQLITE_BUSY
通过fcntl进行文件锁，防止其他进程介入。若锁失败，则返回SQLITE_BUSY
而SQLite选择Busy Retry的方案的原因也正是在此－－－文件锁没有线程锁类似pthread_cond_signal的通知机制。当一个进程的数据库操作结束时，无法通过锁来第一时间通知到其他进程进行重试。因此只能退而求其次，通过多次休眠来进行尝试。

但是，SQLite尽量延迟申请X锁，直到数据块真正写盘时才申请X锁，这是非常巧妙而有效的。

连接串中加入 "Journal Mode=WAL;"可以缓解并发压力，可是客户生产环境仍然出现“database is locked”错误。


简单来说，从 3.3.1 版本开始 SQLite 是支持线程安全的，详细可以查看 [Is SQLite threadsafe?](https://www.sqlite.org/faq.html#q6) 中的介绍，也就是说需要在编译是添加 SQLITE_THREADSAFE 参数，详细可以查看 [Compile-time Options](https://www.sqlite.org/compile.html)。

当 SQLITE_THREADSAFE 为 0 时，是非线程安全的，需要从程序上保证对其访问是穿行的；当使用 1/2 时

Under Unix, you should not carry an open SQLite database across a fork() system call into the child process. ？？？？？

SQLITE_THREADSAFE

1. 多线程使用同一个链接时不会报错，此时需要保证SQLite库本身支持线程安全。
2. 多线程使用不同链接时会报错，

[Using SQLite In Multi-Threaded Applications](https://www.sqlite.org/threadsafe.html)
SQLite事务和锁
https://www.jianshu.com/p/3f60e73c3919

SQLite临时文件及其编译参数
https://www.cnblogs.com/liangxiaxu/archive/2012/09/09/2677339.html
SQLite线程模式探讨
https://wereadteam.github.io/2016/08/19/SQLite/
SQLite 线程安全和并发
https://github.com/xiangwangfeng/xiangwangfeng.github.io/wiki/SQLite-%E7%BA%BF%E7%A8%8B%E5%AE%89%E5%85%A8%E5%92%8C%E5%B9%B6%E5%8F%91
https://blog.csdn.net/guofu8241260/article/details/36378291
http://iihero.iteye.com/blog/1222539
https://www.sqlite.org/cvstrac/wiki?p=MultiThreading
https://www.cnblogs.com/jaejaking/p/5383403.html
http://www.360doc.com/content/10/1214/12/87000_77984300.shtml

压测
https://blog.csdn.net/thomashtq/article/details/45029889

https://medium.com/@gwendal.roue/four-different-ways-to-handle-sqlite-concurrency-db3bcc74d00e
https://www.linkedin.com/pulse/make-multi-thread-write-operation-sqlite-almost-same-time-muradov
http://charlesleifer.com/blog/multi-threaded-sqlite-without-the-operationalerrors/

解决方法有：
1。使用进程或线程间的同步机制以避免同时操作；如用信号量，互斥锁等（pthread_mutex_lock，pthread_mutex_unlock）,如果你的项目工程较大要求较高的话建议用此方法自行封装函数处理同步
2。使用sqlite提供的两个busy handler函数，但对于一个连接来说，只能有一个busy handle，两个函数会相互影响，设置一个的同时会清除另一个，应根据需要来选择。



https://www.sqlite.org/cintro.html
https://www.sqlite.org/errlog.html
http://www.cnblogs.com/stephen-liu74/archive/2012/01/21/2328483.html
https://sskaje.me/2015/12/sniff-unix-domain-socket/
-->


{% highlight text %}
{% endhighlight %}
