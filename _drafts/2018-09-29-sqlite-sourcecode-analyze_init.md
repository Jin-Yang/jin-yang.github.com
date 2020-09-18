---
title: SQLite 源码解析
layout: post
comments: true
language: chinese
category: [program,database,linux]
keywords: SQLite,database
description: SQLite 嵌入到使用它的应用程序中，它们共用相同的进程空间，而不是单独的一个进程，从外部看，它并不像一个 RDBMS，但在进程内部，它却是完整的，自包含的数据库引擎。核心大约有 3W 行标准 C 代码，这些代码都是模块化的，很容易阅读，如下简单介绍。
---

SQLite 嵌入到使用它的应用程序中，它们共用相同的进程空间，而不是单独的一个进程，从外部看，它并不像一个 RDBMS，但在进程内部，它却是完整的，自包含的数据库引擎。

核心大约有 3W 行标准 C 代码，这些代码都是模块化的，很容易阅读，如下简单介绍。

<!-- more -->

## 架构

SQLite 采用了模块的设计，它由三个子系统，包括 8 个独立的模块构成。

![SQLite Architecture]({{ site.url }}/images/databases/sqlite/architecture.png "SQLite Architecture"){: .pull-center width="70%" }

#### 接口 Interface

接口由 SQLite C API 组成，也就是说不管是程序、脚本语言还是库文件，最终都是通过它与 SQLite 交互。

#### 编译器 Compiler

在编译器中，分词器（Tokenizer）和分析器(Parser)对SQL进行语法检查，然后把它转化为底层能更方便处理的分层的数据结构-语法树，然后把语法树传给代码生成器(code generator)进行处理。而代码生成器根据它生成一种针对SQLite的汇编代码，最后由虚拟机(Virtual Machine)执行。

<!--

####2.3、虚拟机(Virtual Machine)
架构中最核心的部分是虚拟机，或者叫做虚拟数据库引擎(Virtual Database Engine,VDBE)。它和Java虚拟机相似，解释执行字节代码。VDBE的字节代码由128个操作码(opcodes)构成，它们主要集中在数据库操作。它的每一条指令都用来完成特定的数据库操作(比如打开一个表的游标)或者为这些操作栈空间的准备(比如压入参数)。总之，所有的这些指令都是为了满足SQL命令的要求(关于VM，后面会做详细介绍)。
-->

#### 后端 Backend

后端由 B-tree、页缓存 (PageCache, Pager) 和操作系统接口构成，通过 B-tree 和 Pager 共同对数据进行管理。

B-tree 的主要功能就是索引，它维护着各个页面之间的复杂的关系，便于快速找到所需数据。而 Pager 的主要作用就是通过 OS 接口在 B-tree 和 Disk 之间传递页面。


![SQLite Execute Routine]({{ site.url }}/images/databases/sqlite/execute-routine.jpg "SQLite Execute Routine"){: .pull-center width="50%" }

实现的源码都保存在 `src` 目录下，当删除调 test 文件之后，实际上真正有效的代码文件没有太多，但是各个文件中为了适配不同的平台有很多的宏定义。

{% highlight text %}
configure             Linux中的编译脚本
ext/                  扩展代码
 |-fts3.c             全文搜索引擎支持
 |-rtree.c            RTree索引支持
tool/                 一些常用的工具
test/                 测试代码
{% endhighlight %}



事务执行过程
https://www.sqlite.org/atomiccommit.html

SQLite优化过程
https://www.sqlite.org/optoverview.html



## VDBE

其全称为 Virtual DataBase Engine 虚拟数据库引擎，类似于 Python、JAVA 中的虚拟机，主要为了实现其跨平台的特性，经过编译器生成的 VDBE 指令最终通过解释引擎解释执行。

如下是一个 SQL 语句生成的字节码指令。

{% highlight text %}
sqlite> CREATE TABLE foobar(id INTEGER PRIMARY KEY, value TEXT);
sqlite> .explain
sqlite> EXPLAIN INSERT INTO foobar(id, value) VALUES(1, 'Micheal');
addr  opcode         p1    p2    p3    p4             p5  comment
----  -------------  ----  ----  ----  -------------  --  -------------
0     Trace          0     0     0                    00
1     Goto           0     15    0                    00
2     OpenWrite      0     2     0     2              00
3     Integer        1     1     0                    00
4     NotNull        1     6     0                    00
5     NewRowid       0     1     0                    00
6     MustBeInt      1     0     0                    00
7     Null           0     2     0                    00
8     String8        0     3     0     Micheal        00
9     NotExists      0     11    1                    00
10    Halt           1555  2     0     PRIMARY KEY must be unique  00
11    MakeRecord     2     2     4     da             00
12    Insert         0     4     1     foobar         13
13    Close          0     0     0                    00
14    Halt           0     0     0                    00
15    Transaction    0     1     0                    00
16    VerifyCookie   0     1     0                    00
17    TableLock      0     2     1     foobar         00
18    Goto           0     2     0                    00
{% endhighlight %}

<!--
通过 explain SELECT * FROM foobar; 可以查看最终生成的字节码。

sqlite> .mode column;
sqlite> .header on;
-->






通过 `.explain` 命令使得显示字节码时会进行格式化显示，更加清晰。


## Atomic Commit

对于支持事务的 DB 引擎来说原子提交是基本的能力，SQLite 提供了两种方式：A) [Rollback Mode](https://www.sqlite.org/atomiccommit.html)；B) [Write-Ahead Logging ](https://www.sqlite.org/wal.html) 。


## Pager

解析 SQL 后需要对磁盘中的数据进行操作，可通过 B-Tree 进行查找，不过 B-Tree 不会直接读写磁盘，会通过 Pager 模块来获取所需的页面或修改页面，该模块可以看作是 B-Tree 和磁盘读写的中间代理。

Pager 作为事务管理器，实现了数据库的 ACID 特性，会依赖三个模块：锁管理、缓存管理和日志管理。


{% highlight text %}
sqlite3_open()
 |-openDatabase()
   |-sqlite3MallocZero()
   |-createCollation()    字符比较方式包括了BINARY NOCASE RTRIM 三种
   |-sqlite3BtreeOpen()
     |-sqlite3MallocZero()
     |-sqlite3PagerOpen()
     |-sqlite3PagerSetBusyhandler()
   |-sqlite3BtreeEnter()
sqlite3_exec()
sqlite3_close()
{% endhighlight %}



## 压测

跟磁盘的性能有很大关系，单线程简单更新，如果使用 RAMDISK 可以达到 `1W` 的 TPS 。

### 单线程

当多个线程使用不同的链接句柄时极易发生冲突，可以将所有线程共有一个 SQLite Handle，并用线程锁避免多线程问题。当多线程并发时，各线程的数据库操作同步顺序进行，这就导致后来的线程会被阻塞较长的时间。

SQLite的多句柄方案及Busy Retry方案

SQLite实际是支持多线程（几乎）无锁地并发操作。只需

* 在编译时开启 `SQLITE_THREADSAFE=2` 选项，然后通过 `PRAGMA` 进行设置；
* 确保同一个句柄同一时间只有一个线程在操作。

Multi-thread. In this mode, SQLite can be safely used by multiple threads provided that no single database connection is used simultaneously in two or more threads.

如果此时再开启 `Write Ahead Log, WAL` 模式，多线程的并发性将得到进一步的提升。

此时写操作会先 append 到 wal 文件末尾，而不是直接覆盖旧数据；而读操作开始时，会记下当前的 WAL 文件状态，并且只访问在此之前的数据，有点类似 MVCC 操作。这就确保了多线程读与读、读与写之间可以并发地进行。


## 参考


<!--
SQLite 源码解析
http://huili.github.io/sqlite/sqliteintro.html
http://huili.github.io/vdbemem/vdbememsqliteintro.html

一篇很不错的文章(包含源码以及图示)
https://www.kancloud.cn/kangdandan/sqlite/64352
-->

{% highlight text %}
{% endhighlight %}
