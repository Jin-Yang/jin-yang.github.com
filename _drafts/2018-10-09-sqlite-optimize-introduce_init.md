---
title: SQLite 优化实践
layout: post
comments: true
language: chinese
category: [database,program,linux]
keywords: SQLite,optimize
description: SQLite 的写入性能与磁盘的性能有很大的关系，例如在 SATA 上可能只有 100 左右，在 SSD 上大概有 1K 左右，如果直接使用 RAMDisk 测试能达到 1W 以上。除此之外，还可以通过一些方法进行相关的优化。
---

SQLite 的写入性能与磁盘的性能有很大的关系，例如在 SATA 上可能只有 100 左右，在 SSD 上大概有 1K 左右，如果直接使用 RAMDisk 测试能达到 1W 以上。除此之外，还可以通过一些方法进行相关的优化。

<!-- more -->

<!--
![SQLite Execute Routine]({{ site.url }}/images/databases/sqlite/execute-routine.jpg "SQLite Execute Routine"){: .pull-center width="50%" }
-->

## 修改配置

在介绍如何修改参数前，提前说明下相关的命令。

`PRAGMA` 语句是 SQLite 的扩展，主要用于修改数据库的配置等。

1. 不同版本的特性可能会被删除或者新增，无法保证其兼容性。
2. 未知的命令不会有错误消息出现，它只是简单的忽略。
3. 有些 PRAGMA 只在 SQL 执行的部分阶段起作用，例如编译阶段`sqlite3_prepare`、执行阶段`sqlite3_step`。

#### 配置

可以通过 `PRAGMA` 命令查询或者设置当前系统的配置参数。

{% highlight text %}
----- 要查询当前PRAGMA的值
sqlite> PRAGMA pragma_name;
----- 设置为新值
sqlite> PRAGMA pragma_name = value;
{% endhighlight %}

设置模式，可以是名称或等值的整数，但返回的值将始终是一个整数。

### 常用参数

主要包含了两个：A) `synchronous` 定义了何时写入磁盘；B) `journal_mode` 日志写入模式。

#### synchronous

{% highlight text %}
sqlite> PRAGMA synchronous;
sqlite> PRAGMA synchronous = 2;
{% endhighlight %}

用于设置磁盘的写入时机。

* `0/OFF` 不进行同步设置，完全由操作系统进行控制；
* `1/NORMAL` V2默认设置，在关键操作后刷新磁盘，小几率导致数据库损坏；
* `2/FULL` V3默认设置，每次关键操作都会刷新磁盘，性能差但是可以保证数据库不损坏。

对于磁盘来说，此时的吞吐量会增加很多，1/2 差异不大。

<!--
如果使用 RAMDISK 测试实际上差别不大。
40 -> 55 -> 6K
-->

#### journal_mode

控制日志文件如何存储和处理的日志模式，可以设置数据库级别。

其中 `journal` 为数据库事务提供 `rollback` 操作，当事务写入时，首先写入 `journal` 文件中，在提交时，根据 `journal-mode` 来处理 `journal` 日志文件。

若在提交之前由于断电等原因造成无法提交，当再次启动时，通过 `journal` 文档做回滚操作，保证数据库的完整性和一致性。

{% highlight text %}
sqlite> PRAGMA journal_mode;
sqlite> PRAGMA journal_mode = mode;
sqlite> PRAGMA database.journal_mode;
sqlite> PRAGMA database.journal_mode = mode;
{% endhighlight %}

这里支持五种日志模式：

* `DELETE` 默认，在事务结束时，日志文件将被删除；
* `WAL` 使用 Write Ahead Log；
* `MEMORY` 日志记录保留在内存中，而不是磁盘上，断电会丢失；
* `OFF` 不保留任何日志记录；

<!--
TRUNCATE	日志文件被阶段为零字节长度。
PERSIST	日志文件被留在原地，但头部被重写，表明日志不再有效。
-->


DISK 40->120
RAMDISK 1W->4W


### 关于 WAL

在 `V3.7.0` 版本支持，修改不直接写入数据库文件中，而是直接一个 WAL 的文件中，若事务失败，WAL 记录被忽略；若事务成功，随后在某个 checkpoint 时间点写回数据库。

此时读写、读读可完全并发执行，不会互相阻塞(写之间不能并发)。除了数据库文件，同时会增加 `name.db-shm` `name.db-wal` 两个文件。

为了避免读取的数据不一致，查询时也需要读取 WAL 文件，这样的代价就是读取会变得稍慢，但是写入会变快很多。要提高查询性能的话，可以减小 WAL 文件的大小，但写入性能也会降低。

另外，需要注意各个版本间的 WAL 可能不兼容，但是数据库文件是通用的。

## 参考

<!--
优化(通过修改配置完成)
https://zhuanlan.zhihu.com/p/36769649
https://www.sqlite.org/optoverview.html


SQLite3源码学习（8）Pager模块概述及初始化
https://blog.csdn.net/pfysw/article/details/79121815
-->

[微信 iOS SQLite 源码优化实践](https://github.com/WeMobileDev/article/blob/master/%E5%BE%AE%E4%BF%A1iOS%20SQLite%E6%BA%90%E7%A0%81%E4%BC%98%E5%8C%96%E5%AE%9E%E8%B7%B5.md) 。

{% highlight text %}
{% endhighlight %}
