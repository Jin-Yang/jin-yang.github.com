---
title: InnoDB Buffer Pool
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,innodb,buffer pool
description: 无论哪种数据库，缓存都是提高数据库性能的关键技术，通过缓存可以在读写方面大幅提高数据库的整体性能，InnoDB 中就是通过 Buffer Pool 实现的。在此，简单介绍下 Buffer Pool 中的实现。
---

无论哪种数据库，缓存都是提高数据库性能的关键技术，通过缓存可以在读写方面大幅提高数据库的整体性能，InnoDB 中就是通过 Buffer Pool 实现的。

在此，简单介绍下 Buffer Pool 中的实现。

<!-- more -->

## 简介

我们知道 InnoDB 使用 buffer pool 来缓存从磁盘读取到内存的数据页，BP 通常由数个内存块加上一组控制结构体对象组成，BP 每块内存通过 mmap 分配内存，而这些大片的内存块又按照 16KB 划分为多个 frame，用于存储数据页。

多数情况下 BP 是以 16KB 来存储数据页，但有一种例外：使用压缩表时，需要在内存中同时存储压缩页和解压页，对于压缩页，会使用 Binary buddy allocator 算法来分配内存空间。

例如我们读入一个 8KB 的压缩页，就从 BP 中取一个 16KB 的 block，取其中 8KB，剩下的 8KB 放到空闲链表上；如果紧跟着另外一个 4KB 的压缩页读入内存，就可以从这 8KB 中分裂 4KB，同时将剩下的 4KB 放到空闲链表上。

BP 通过 Least Recently Used, LRU 进行组织，当需要插入新数据时，将最近最少使用的 block 去除，并将新 block 插入到列表的中间，也就是 ```midpoint insertion strategy``` 。

头部的子链表，表示最近读取过；尾部的子链表表示最近很少读取；也就是说经常使用的 blocks 会放在头部，而需要去除的会放在尾部。

### 算法详解

单纯的 LRU 算法有一个缺点：如果有某一个查询做了一次全表扫描，如备份、DDL 等，都可能会导致整个 LRU 链表中的数据块都被替换了，甚至很多热点数据也会被替换，而这些新进的数据块可能在这一次查询之后就再也不会被读到了；此时也就是说 BP 被污染了。

即使采用上述的 midpoint 方法，也就是说当数据块需要从数据文件中读取时 (也包括了预读)，首先会放到 old sublist 的头部
(midpoint)。然后，如果有对这个数据块的访问，那么就将这个数据块放到 new sublist 的首部。

一般来说，一个数据块被取出后，立刻会有读取，也就很快会被放到 new sublist 的头部。一种糟糕的情况是，如果是 mysqldump 访问全部数据块，也就会导致所有的数据块被放到 new sublist；这样 BP 也会被全部污染。

为了解决这个问题，可以设置 ```innodb_old_blocks_time(ms)``` 参数，当页被插入到 midpoint 后，必须要在 old sublist 的头部停留超过上述的时间后，才有可能被转移到 new sublist。

参数 ```innodb_old_blocks_time``` 可以动态设置，在执行一些全表扫描时，可以将上述参数设置为比较大的值，操作完成之后再恢复为 0 。

{% highlight text %}
SET GLOBAL innodb_old_blocks_time = 1000;
... perform queries that scan tables ...
SET GLOBAL innodb_old_blocks_time = 0;
{% endhighlight %}

还有一种情况时，希望数据加载的缓存中。例如，在进行性能测试时，通常会执行一次全表扫描，将磁盘中的数据加载到内存中，那么，此时就应该将上述的参数设置为 0 。

### 多实例配置

<!--Multiple Buffer Pool Instances-->

当服务器的内存比较大时，如果多个线程读取 BP 数据，会导致瓶颈；此时，可以将 BP 分为几个实例 (instance)，从而提高并发，减少资源冲突。每个实例管理自己的 free lists、flush lists、LRUs、互斥锁 (mutex) 以及其它与 BP 相关的结构。

可以通过 innodb_buffer_pool_instances 参数配置实例个数，默认为 1，最大可以配置为 64；只有当 innodb_buffer_pool_size 的值大于 1G 时才会生效，而且每个实例均分 BP 缓存。

通过多个 BP 可以减小竞争，每个页(page)将通过一个 hash 函数随机分配到 BP 中。

### 参数、监控

InnoDB 主索引是聚簇索引，索引与数据共用表空间，也就是说，对于 InnoDB 而言，数据就是索引，索引就是数据。也就是说，Innodb 和 MyISAM 缓存的最大区别就在于前者不仅缓存索引，同时还会缓存数据。

#### 参数设置

通过 innodb_buffer_pool_size 参数来设置 InnoDB 缓存池大小，也就是缓存用户表及索引数据的最主要缓存空间，甚至其它管理数据 (例如元数据信息，行级锁信息)；对于专用的数据库服务器上通常为物理内存的 70% ~ 80%；5.7.5 版本后可以动态调整，调整时按 Block 进行。

因此，对 Innodb 整体性能影响也最大。可通过如下命令查看当前 buffer pool 相关的参数。

{% highlight text %}
----- 当前缓存的大小
mysql> SHOW VARIABLES LIKE 'innodb%pool%';
+-----------------------------------------+----------------+
| Variable_name                           | Value          |
+-----------------------------------------+----------------+
| innodb_buffer_pool_chunk_size           | 134217728      |  动态调整时Block的大小
| innodb_buffer_pool_dump_at_shutdown     | ON             |
| innodb_buffer_pool_dump_now             | OFF            |
| innodb_buffer_pool_dump_pct             | 25             |
| innodb_buffer_pool_evict                |                |
| innodb_buffer_pool_filename             | ib_buffer_pool |
| innodb_buffer_pool_instances            | 1              |  BP缓存的实例数，只有当BP大于1G时才会生效
| innodb_buffer_pool_load_abort           | OFF            |
| innodb_buffer_pool_load_at_startup      | ON             |
| innodb_buffer_pool_load_now             | OFF            |
| innodb_buffer_pool_size                 | 134217728      |  缓存大小，包括了数据和索引
| innodb_disable_resize_buffer_pool_debug | ON             |
+-----------------------------------------+----------------+
12 rows in set (0.00 sec)

----- 当前缓存页的大小
mysql> SHOW VARIABLES LIKE 'innodb_page_size';
+------------------+-------+
| Variable_name    | Value |
+------------------+-------+
| innodb_page_size | 16384 |
+------------------+-------+
1 row in set (0.00 sec)
{% endhighlight %}

如上所述，通常建议设置为系统内存的 70%~80%，不过也必须要对具体项目具体分析，比如最大链接数、是否有 MyISAM 引擎等。可以按照如下的值进行分配，仅做参考：

{% highlight text %}
操作系统:
    800M~1G
线程独享:
    2GB  = 线程数(500) * (1MB  + 1MB  + 1MB  + 512KB  + 512KB)
    sort_buffer_size(1MB) + join_buffer_size(1MB) + read_buffer_size (1MB)
    read_rnd_buffer_size(512KB) + thread_statck(512KB)
MyISAM Key Cache:
    1.5GB
Innodb Buffer Pool:
    8GB(系统内存) - 800MB - 2GB - 1.5GB = 3.7GB
{% endhighlight %}

修改配置文件 /etc/my.cnf，并添加如下字段，然后重启 mysqld 即可。

{% highlight text %}
[mysqld]
innodb_buffer_pool_size = 3G
{% endhighlight %}

除了上述的参数之外，与之相关的还有如下配置参数：

* innodb_old_blocks_pct<br>old-sublist 的分割点，可以为 5~95，默认为 37 也就是 3/8；
* innodb_old_blocks_time (ms)<br>指定页必须在 old-sublist 中待多长时间之后，才有可能被移动到新列表中；

<!--
    innodb_flush_method: 这个控制Innodb的IO形为，什么:fsync, O_DSYNC之类的，这里不做过多介绍， 建议使用: O_DIRECT, 这样减少操作系统级别VFS的缓存使用内存过多和Innodb本身的buffer的缓存冲突，同时也算是给操作系统减少点压力。
    innodb_max_dirty_pages_pct : 这个参数据控制脏页的比例如果是innodb_plugin或是MySQL5.5以上的版本，建议这个参数可以设制到75%-90%都行。如果是大量写入，而且写入的数据不是太活跃，可以考虑把这个值设的低一点。 如果写入或是更新的数据也就是热数据就可以考虑把这个值设为：95%
    innodb_log_file_size : 这个可以配置256M以上，建议有两个以前的日志文件（innodb_log_files_in_group). 如果对系统非常大写的情况下，也可以考虑用这个参数提高一下性能，把文件设的大一点，减少checkpiont的发生。 最大可以设制成：innodb_log_files_in_group * innodb_log_file_size < 512G(percona, MySQL 5.6) 建议设制成: 256M -> innodb_buffer_pool_size/innodb_log_file_in_group 即可。
    innodb_log_buffer_size : 如果没在大事务，控制在8M-16M即可。

其它对IO有影响的参数(以5.6为准）

    innodb_adaptive_flushing 默认即可
    innodb_change_buffer_max_size 如果是日值类服务，可以考虑把这个增值调到 50
    innodb_change_buffering 默认即可
    innodb_flush_neighors 默认是开的， 这个一定要开着，充分利用顺序IO去写数据。
    innodb_lru_scan_depth: 默认即可 这个参数比较专业。
    innodb_max_purge_lag 默认没启用，如果写入和读取都量大，可以保证读取优先，可以考虑使用这个功能。
    innodb_random_read_ahead 默认没开启，属于一个比较活跃的参数，如果要用一定要多测试一下。 对用passport类应用可以考虑使用
    innodb_read_ahead_threshold 默认开启：56 预读机制可以根据业务处理，如果是passprot可以考虑关闭。如果使用innodb_random_read_ahead,建议关闭这个功能
    sync_binlog 默认即可： 0
    innodb_rollback_segments 默认即可: 128
另外5.6的undo log也可以独立配置了，建议单独配置出来。
    innodb_read_io_threads 默认为：4 可以考虑8
    innodb_write_io_threads 默认为：4 可以考虑8
-->






#### 监控指标

可以通过如下命令查看一些 InnoDB Buffer Pool 监控值。

{% highlight text %}
mysql> SHOW STATUS LIKE 'Innodb_buffer_pool_%';
+-----------------------------------+-------+
| Variable_name                     | Value |
+-----------------------------------+-------+
| Innodb_buffer_pool_pages_data     | 70    |    已经使用的缓存页数
| Innodb_buffer_pool_pages_dirty    | 0     |    脏页数
| Innodb_buffer_pool_pages_flushed  | 0     |    刷新页数
| Innodb_buffer_pool_pages_free     | 1978  |    尚未使用的缓存页数
| Innodb_buffer_pool_pages_misc     | 0     |
| Innodb_buffer_pool_pages_total    | 2048  |    缓存页面总数，innodb_page_size(16k)
| Innodb_buffer_pool_read_ahead_rnd | 1     |
| Innodb_buffer_pool_read_ahead_seq | 0     |
| Innodb_buffer_pool_read_requests  | 329   |    读请求次数
| Innodb_buffer_pool_reads          | 19    |    从磁盘中读取数据的次数
| Innodb_buffer_pool_wait_free      | 0     |
| Innodb_buffer_pool_write_requests | 0     |
+-----------------------------------+-------+
12 rows in set (0.01 sec)
{% endhighlight %}

如果 Innodb_buffer_pool_pages_free 偏大的话，证明有很多缓存没有被利用到，这时可以考虑减小缓存；相反 Innodb_buffer_pool_pages_data 过大就考虑增大缓存。

对于缓存命中率可以通过如下方式计算。

{% highlight text %}
----- 缓存命中率，通常不低于99%
(Innodb_buffer_pool_read_requests - Innodb_buffer_pool_reads) / Innodb_buffer_pool_read_requests * 100%
(329 - 19) / 329 * 100%  = 94.22%。
{% endhighlight %}

<!--
当然，通过上面的数据，我们还可以分析出 write 命中率，可以得到发生了多少次 read_ahead_rnd，多少次 read_ahead_seq，发生过多少次 latch，多少次因为 Buffer 空间大小不足而产 生 wait_free 等等。

在 Innodb Buffer  Pool 中，还有一个非常重要的概念，叫做“预读”。一般来说，预读概念主要是 在一些高端存储上面才会有，简单来说就是通过分析数据请求的特点来自动判断出客户在请求当前数据 块之后可能会继续请求的数据快。通过该自动判断之后，存储引擎可能就会一次将当前请求的数据库和 后面可能请求的下一个（或者几个）数据库一次全部读出，以期望通过这种方式减少磁盘 IO 次数提高 IO 性能。在上面列出的状态参数中就有两个专门针对预读：
Innodb_buffer_pool_read_ahead_rnd，记录进行随机读的时候产生的预读次数； Innodb_buffer_pool_read_ahead_seq，记录连续读的时候产生的预读次数；
-->

另外，也可以通过 ```SHOW ENGINE INNODB STATUS\G``` 命令查看。

{% highlight text %}
mysql> SHOW ENGINE INNODB STATUS\G
----------------------
BUFFER POOL AND MEMORY
----------------------
Total large memory allocated 140836864
Dictionary memory allocated 538303
Buffer pool size   8192
Free buffers       7810
Database pages     382
Old database pages 0
Modified db pages  0
Pending reads      0
Pending writes: LRU 0, flush list 0, single page 0
Pages made young 0, not young 0
0.00 youngs/s, 0.00 non-youngs/s
Pages read 336, created 46, written 128
0.00 reads/s, 0.00 creates/s, 0.00 writes/s
No buffer pool page gets since the last printout
Pages read ahead 0.00/s, evicted without access 0.00/s, Random read ahead 0.00/s
LRU len: 382, unzip_LRU len: 0
I/O sum[0]:cur[0], unzip sum[0]:cur[0]
{% endhighlight %}

可以查看对应的源码。

{% highlight c %}
ibool srv_printf_innodb_monitor(FILE* file, ibool nowait,
    ulint* trx_start_pos, ulint*  trx_end)
{
    // ... ...
    fputs("----------------------\n"
          "BUFFER POOL AND MEMORY\n"
          "----------------------\n", file);
    fprintf(file,
        "Total large memory allocated " ULINTPF "\n"
        "Dictionary memory allocated " ULINTPF "\n",
        os_total_large_mem_allocated,
        dict_sys->size);

    buf_print_io(file);
    // ... ...
}

void buf_print_io( FILE* file)
{
    ulint           i;
    buf_pool_info_t*    pool_info;
    buf_pool_info_t*    pool_info_total;

    /* If srv_buf_pool_instances is greater than 1, allocate
    one extra buf_pool_info_t, the last one stores
    aggregated/total values from all pools */
    if (srv_buf_pool_instances > 1) {
        pool_info = (buf_pool_info_t*) ut_zalloc_nokey((
            srv_buf_pool_instances + 1) * sizeof *pool_info);

        pool_info_total = &pool_info[srv_buf_pool_instances];
    } else {
        ut_a(srv_buf_pool_instances == 1);

        pool_info_total = pool_info =
            static_cast<buf_pool_info_t*>(
                ut_zalloc_nokey(sizeof *pool_info));
    }

    for (i = 0; i < srv_buf_pool_instances; i++) {
        buf_pool_t* buf_pool;

        buf_pool = buf_pool_from_array(i);

        /* Fetch individual buffer pool info and calculate
        aggregated stats along the way */
        buf_stats_get_pool_info(buf_pool, i, pool_info);

        /* If we have more than one buffer pool, store
        the aggregated stats  */
        if (srv_buf_pool_instances > 1) {
            buf_stats_aggregate_pool_info(pool_info_total,
                              &pool_info[i]);
        }
    }

    /* Print the aggreate buffer pool info */
    buf_print_io_instance(pool_info_total, file);

    /* If there are more than one buffer pool, print each individual pool
    info */
    if (srv_buf_pool_instances > 1) {
        fputs("----------------------\n"
        "INDIVIDUAL BUFFER POOL INFO\n"
        "----------------------\n", file);

        for (i = 0; i < srv_buf_pool_instances; i++) {
            fprintf(file, "---BUFFER POOL " ULINTPF "\n", i);
            buf_print_io_instance(&pool_info[i], file);
        }
    }

    ut_free(pool_info);
}
{% endhighlight %}

上述方式，只能看到所有 BP 的总体统计值，如果要查看每个实例的值，可以通过如下命令查看。

{% highlight text %}
mysql> SELECT * FROM information_schema.innodb_buffer_pool_stats\G
{% endhighlight %}


## 磁盘预热

如果一台高负荷的机器重启后，内存中大量的热数据被清空；重启后，会重新从磁盘加载到 Buffer Pool 中，此时，性能就会变得很差，连接数就会很高。

在 MySQL5.6 后，添加了一个新特性避免的这种问题的出现，当正常关闭时会将 BP 中的数据写入到磁盘；对于 kill -9 或者宕机仍无法使用。

{% highlight text %}
----- 关闭时把热数据dump到本地磁盘。
innodb_buffer_pool_dump_at_shutdown = 1

innodb_buffer_pool_dump_now = 1

解释：采用手工方式把热数据dump到本地磁盘。

innodb_buffer_pool_load_at_startup = 1

解释：在启动时把热数据加载到内存。

innodb_buffer_pool_load_now = 1

解释：采用手工方式把热数据加载到内存。

{% endhighlight %}

在关闭 MySQL 时，会把内存中的热数据保存在磁盘里 ib_buffer_pool 文件中，位于数据目录下。




## 源码解析

MySQL 5.6 内存管理在 mem0pool.c 中实现，其文件的开头注释中都有说明，粗略的可以分成四部分，包含 9 大块。

{% highlight text %}
/* We would like to use also the buffer frames to allocate memory. This
would be desirable, because then the memory consumption of the database
would be fixed, and we might even lock the buffer pool to the main memory.
The problem here is that the buffer management routines can themselves call
memory allocation, while the buffer pool mutex is reserved.

The main components of the memory consumption are:

1. buffer pool,
2. parsed and optimized SQL statements,
3. data dictionary cache,
4. log buffer,
5. locks for each transaction,
6. hash table for the adaptive index,
7. state and buffers for each SQL query currently being executed,
8. session for each user, and
9. stack for each OS thread.

Items 1 and 2 are managed by an LRU algorithm. Items 5 and 6 can potentially
consume very much memory. Items 7 and 8 should consume quite little memory,
and the OS should take care of item 9, which too should consume little memory.

A solution to the memory management:

1. the buffer pool size is set separately;
2. log buffer size is set separately;
3. the common pool size for all the other entries, except 8, is set separately.

Problems: we may waste memory if the common pool is set too big. Another
problem is the locks, which may take very much space in big transactions.
Then the shared pool size should be set very big. We can allow locks to take
space from the buffer pool, but the SQL optimizer is then unaware of the
usable size of the buffer pool. We could also combine the objects in the
common pool and the buffers in the buffer pool into a single LRU list and
manage it uniformly, but this approach does not take into account the parsing
and other costs unique to SQL statements.

The locks for a transaction can be seen as a part of the state of the
transaction. Hence, they should be stored in the common pool. We still
have the problem of a very big update transaction, for example, which
will set very many x-locks on rows, and the locks will consume a lot
of memory, say, half of the buffer pool size.

Another problem is what to do if we are not able to malloc a requested
block of memory from the common pool. Then we can request memory from
the operating system. If it does not help, a system error results.

Because 5 and 6 may potentially consume very much memory, we let them grow
into the buffer pool. We may let the locks of a transaction take frames
from the buffer pool, when the corresponding memory heap block has grown to
the size of a buffer frame. Similarly for the hash node cells of the locks,
and for the adaptive index. Thus, for each individual transaction, its locks
can occupy at most about the size of the buffer frame of memory in the common
pool, and after that its locks will grow into the buffer pool. */
{% endhighlight %}

### BP 管理概述

为了管理 buffer pool，每个 buffer pool instance 使用如下几个链表来管理：

* LRU 链表包含所有读入内存的数据页；
* Flush_list 包含被修改过的脏页；
* unzip_LRU 包含所有解压页；
* Free list 上存放当前空闲的 block 。

另外为了避免查询数据页时扫描 LRU，还为每个 buffer pool instance 维护了一个 page hash，可以通过 space id 和 page no 直接找到对应的 page 。

一般情况下，当需要读入一个 Page 时，首先会根据 space id 和 page no 找到对应的 buffer pool instance；然后查询 page hash，如果 page hash 中没有，则表示需要从磁盘读取。

在读盘前，我们需要为即将读入内存的数据页分配一个空闲的 block，当 free list 上存在空闲的 block 时，可以直接从 free list 上摘取；如果没有，就需要从 unzip_lru 或者 lru 上淘汰 page 。

在淘汰页时会遵循一定原则，可参考 ```buf_LRU_get_free_block()->buf_LRU_scan_and_free_block()```：

1. 首先尝试从 unzip_lru 上驱逐解压页，可以参考 ```buf_LRU_free_from_unzip_LRU_list()```；
2. 如果没有，再尝试从 lru 链表上淘汰 Page，可以参考 ```buf_LRU_free_from_common_LRU_list()``` ；
3. 如果还是无法从 lru 上获取到空闲 block，用户线程就会参与刷脏，尝试做一次 SINGLE PAGE FLUSH，单独从 lru 上刷掉一个脏页，然后再重试。

BP 中的页被修改后，不是立刻写入磁盘，而是由后台线程定时写入，和大多数数据库系统一样，脏页的写盘遵循日志先行 WAL 原则，因此在每个 block 上都记录了一个最近被修改时的 lsn，写数据页时需要确保当前写入日志文件的 redo 不低于这个 lsn。

然而基于 WAL 原则的刷脏策略可能带来一个问题：当数据库的写入负载过高时，产生 redo log 的速度极快，redo log 可能很快到达同步 checkpoint 点，这时候需要进行刷脏来推进 lsn 。

由于这种行为是由用户线程在检查到 redo log 空间不够时触发，大量用户线程将可能陷入到这段低效的逻辑中，产生一个明显的性能拐点。

### 数据结构


{% highlight cpp %}

class buf_page_t {
    unsigned    old:1;    // 是否在LRU_list的old部分
};

struct buf_block_t {
};

struct buf_buddy_free_t {
};


struct buf_pool_t{
    /** @name General fields */
    /* @{ */
    ib_mutex_t  mutex;      /*!< Buffer pool mutex of this
                    instance */
    ib_mutex_t  zip_mutex;  /*!< Zip mutex of this buffer
                    pool instance, protects compressed
                    only pages (of type buf_page_t, not
                    buf_block_t */
    ulint       instance_no;    /*!< Array index of this buffer
                    pool instance */
    ulint       old_pool_size;  /*!< Old pool size in bytes */
    ulint       curr_pool_size; /*!< Current pool size in bytes */
    ulint       LRU_old_ratio;  /*!< Reserve this much of the buffer
                    pool for "old" blocks */
#ifdef UNIV_DEBUG
    ulint       buddy_n_frames; /*!< Number of frames allocated from
                    the buffer pool to the buddy system */
#endif
#if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
    ulint       mutex_exit_forbidden; /*!< Forbid release mutex */
#endif
    ulint       n_chunks;   /*!< number of buffer pool chunks */
    buf_chunk_t*    chunks;     /*!< buffer pool chunks */
    ulint       curr_size;  /*!< current pool size in pages */
    hash_table_t*   page_hash;  /*!< hash table of buf_page_t or
                    buf_block_t file pages,
                    buf_page_in_file() == TRUE,
                    indexed by (space_id, offset).
                    page_hash is protected by an
                    array of mutexes.
                    Changes in page_hash are protected
                    by buf_pool->mutex and the relevant
                    page_hash mutex. Lookups can happen
                    while holding the buf_pool->mutex or
                    the relevant page_hash mutex. */
    hash_table_t*   zip_hash;   /*!< hash table of buf_block_t blocks
                    whose frames are allocated to the
                    zip buddy system,
                    indexed by block->frame */
    ulint       n_pend_reads;   /*!< number of pending read
                    operations */
    ulint       n_pend_unzip;   /*!< number of pending decompressions */

    time_t      last_printout_time;         // when buf_print_io was last time called
    buf_buddy_stat_t buddy_stat[BUF_BUDDY_SIZES_MAX + 1];
                    /*!< Statistics of buddy system,
                    indexed by block size */
    buf_pool_stat_t stat;       /*!< current statistics */
    buf_pool_stat_t old_stat;   /*!< old statistics */

    /* @} */

    /** @name Page flushing algorithm fields */

    /* @{ */

    ib_mutex_t  flush_list_mutex;/*!< mutex protecting the
                    flush list access. This mutex
                    protects flush_list, flush_rbt
                    and bpage::list pointers when
                    the bpage is on flush_list. It
                    also protects writes to
                    bpage::oldest_modification and
                    flush_list_hp */
    const buf_page_t*   flush_list_hp;/*!< "hazard pointer"
                    used during scan of flush_list
                    while doing flush list batch.
                    Protected by flush_list_mutex */
    UT_LIST_BASE_NODE_T(buf_page_t) flush_list;
                    /*!< base node of the modified block
                    list */
    ibool       init_flush[BUF_FLUSH_N_TYPES];
                    /*!< this is TRUE when a flush of the
                    given type is being initialized */
    ulint       n_flush[BUF_FLUSH_N_TYPES];
                    /*!< this is the number of pending
                    writes in the given flush type */
    os_event_t  no_flush[BUF_FLUSH_N_TYPES];
                    /*!< this is in the set state
                    when there is no flush batch
                    of the given type running */
    ib_rbt_t*   flush_rbt;  /*!< a red-black tree is used
                    exclusively during recovery to
                    speed up insertions in the
                    flush_list. This tree contains
                    blocks in order of
                    oldest_modification LSN and is
                    kept in sync with the
                    flush_list.
                    Each member of the tree MUST
                    also be on the flush_list.
                    This tree is relevant only in
                    recovery and is set to NULL
                    once the recovery is over.
                    Protected by flush_list_mutex */
    ulint       freed_page_clock;/*!< a sequence number used
                    to count the number of buffer
                    blocks removed from the end of
                    the LRU list; NOTE that this
                    counter may wrap around at 4
                    billion! A thread is allowed
                    to read this for heuristic
                    purposes without holding any
                    mutex or latch */
    ibool       try_LRU_scan;   /*!< Set to FALSE when an LRU
                    scan for free block fails. This
                    flag is used to avoid repeated
                    scans of LRU list when we know
                    that there is no free block
                    available in the scan depth for
                    eviction. Set to TRUE whenever
                    we flush a batch from the
                    buffer pool. Protected by the
                    buf_pool->mutex */
    /* @} */

    /** @name LRU replacement algorithm fields */
    /* @{ */

    UT_LIST_BASE_NODE_T(buf_page_t) free;
                    /*!< base node of the free
                    block list */

    UT_LIST_BASE_NODE_T(buf_page_t) withdraw;
                    /*!< base node of the withdraw
                    block list. It is only used during
                    shrinking buffer pool size, not to
                    reuse the blocks will be removed */

    ulint       withdraw_target;/*!< target length of withdraw
                    block list, when withdrawing */

    /** "hazard pointer" used during scan of LRU while doing
    LRU list batch.  Protected by buf_pool::mutex */
    LRUHp       lru_hp;

    /** Iterator used to scan the LRU list when searching for
    replacable victim. Protected by buf_pool::mutex. */
    LRUItr      lru_scan_itr;

    /** Iterator used to scan the LRU list when searching for
    single page flushing victim.  Protected by buf_pool::mutex. */
    LRUItr      single_scan_itr;

    UT_LIST_BASE_NODE_T(buf_page_t) LRU;
                    /*!< base node of the LRU list */

    buf_page_t* LRU_old;    /*!< pointer to the about
                    LRU_old_ratio/BUF_LRU_OLD_RATIO_DIV
                    oldest blocks in the LRU list;
                    NULL if LRU length less than
                    BUF_LRU_OLD_MIN_LEN;
                    NOTE: when LRU_old != NULL, its length
                    should always equal LRU_old_len */
    ulint       LRU_old_len;    /*!< length of the LRU list from
                    the block to which LRU_old points
                    onward, including that block;
                    see buf0lru.cc for the restrictions
                    on this value; 0 if LRU_old == NULL;
                    NOTE: LRU_old_len must be adjusted
                    whenever LRU_old shrinks or grows! */

    UT_LIST_BASE_NODE_T(buf_block_t) unzip_LRU;
                    /*!< base node of the
                    unzip_LRU list */

    /* @} */
    /** @name Buddy allocator fields
    The buddy allocator is used for allocating compressed page
    frames and buf_page_t descriptors of blocks that exist
    in the buffer pool only in compressed form. */
    /* @{ */
#if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
    UT_LIST_BASE_NODE_T(buf_page_t) zip_clean;
                    /*!< unmodified compressed pages */
#endif /* UNIV_DEBUG || UNIV_BUF_DEBUG */

    UT_LIST_BASE_NODE_T(buf_buddy_free_t) zip_free[BUF_BUDDY_SIZES_MAX];
                    /*!< buddy free lists */

    buf_page_t*         watch;   ?????TODODO:到底是干吗的
                    /*!< Sentinel records for buffer
                    pool watches. Protected by
                    buf_pool->mutex. */
};




struct buf_chunk_t{   // include/buf0buf.ic
    ulint       mem_size;   /*!< allocated size of the chunk */
    ulint       size;       /*!< size of frames[] and blocks[] */
    void*       mem;        /*!< pointer to the memory area which
                    was allocated for the frames */
    buf_block_t*    blocks;     /*!< array of buffer control blocks */
};
{% endhighlight %}




## Buffer Pool





### 预读策略

预读其实是基于这样的判断，在读取磁盘数据时，接下来的请求，有很大可能要读取周围的数据；为此，InnoDB 会异步读取该页周围的数据，从而减小 IO 。

InnoDB 提供了顺序和随机两种策略。

#### 顺序预读

<!--
buf_read_ahead_linear()

也就是 linear read-ahead ，会根据
https://dev.mysql.com/doc/refman/5.7/en/innodb-performance-read_ahead.html
-->




### 源码解析

Buffer Pool 的初始化入口在 ```innobase_start_or_create_for_mysql()``` 函数中，调用流程如下。

{% highlight text %}
buf_pool_init()                     # 初始化buffer pool主函数入口
 |-buf_pool_init_instance()         # 1.1 每个buf-pool大小为srv_buf_pool_size/instance，通过该函数初始化各个实例
 | |-UT_LIST_INIT(buf_pool->free)   # 2.1 初始化buf pool的free list
 | |
 | |-buf_chunk_init()               # 初始化buffer pool中的chunk
 | | |-ut_2pow_round()              # 2.2 计算每个buf pool实际所需空间，空间必须按照page_size对齐
 | | |-ut_2pow_round()              #     必须为每个page分配一个内存block结构，用于管理内存page
 | | |-os_mem_alloc_large()         # 2.3 为buf pool分配大块内存，对于不同系统配置，调用不同函数
 | | | |-shmget() shmat() shmct()   #     若用了大页(Huge Page)，则使用这些方法分配空间
 | | | |-VirtualAlloc()             #     Win平台使用该函数分配空间
 | | | |-ut_malloc_low()            #     若未使用MMAP，则调用该函数分配空间
 | | | |-mmap()                     #     若使用MMAP，则调用mmap函数分配空间
 | | |-ut_align()                   # 2.4 将分配出来的mem空间，按page size对齐，作为page的起点
 | | |-... ...                      # 2.5 将mem划分为两部分：前部分作为block结构空间；后部分作为page空间
 | | |                              #     page空间每一个page的起始位置，必须按照page size对齐
 | | |                              #     block结构起始位置为mem空间的0号位置
 | | |                              #     page空间的起始位置，为预留足够的block结构之后的第一个frame位置
 | | |-buf_block_init()             # 2.6 为每个page指定一个block头结构，并初始化各种mutex与lock
 | | |-UT_LIST_ADD_LAST()           # 2.7 将page加入buf pool的free list链表，等待分配
 | |-hash_create()                  # 2.8 创建相应的hash表，若page对应于文件中的一个页，则在page hash表中存在
 |                                  #     便于page在内存中的快速定位
 |
 |-buf_pool_set_sizes()             # 1.2 设置大小参数
 |-buf_LRU_old_ratio_update()       # 1.3 设置buf_pool.LRU中oldList部分所占的比率，默认为3/8
 |-btr_search_sys_create()
{% endhighlight %}

其中比较重要的参数是 innodb_old_blocks_pct，该参数运行时可以调整，初始值为 3/8 。

若设置不同的 old ratio，则涉及到调整 buf_pool-&gt;LRU_old 指向的位置，LRU_old 指向的是 LRU list 中位于 old ratio 处的 block 位置，也就是说 old ration 调整，LRU_old 需要相应的做出调整。

### 页面管理

在 MySQL 5.5 之后，InnoDB 支持多 Buffer Pool Instance，内存中有多个 buffer pool 管理，如果此时指定一个 page，需要确定属于哪个 buffer pool 的。

其入参是给定一个 page 的 tablespace id 与 pageno，然后可以定位到对应的管理 buffer pool 。

{% highlight c %}
class page_id_t {
private:
    ib_uint32_t m_space;     // Tablespace id
    ib_uint32_t m_page_no;   // Page number
    mutable ulint   m_fold;  // 通过m_space+m_page_no计算的hash值
};

buf_pool_t* buf_pool_get(const page_id_t& page_id)
{
    // 每次读取64Pages，BUF_READ_AHEAD_AREA，这64页统一在一个BP实例中管理
    ulint       ignored_page_no = page_id.page_no() >> 6;
    page_id_t   id(page_id.space(), ignored_page_no);   // 创建一个实例，无它
    ulint       i = id.fold() % srv_buf_pool_instances; // fold是计算一个hash值
    return(&buf_pool_ptr[i]);
}
{% endhighlight %}

因为目前 InnoDB 的一个 read ahead 是 64 个 page，因此右移 6 位能够保证一个 read ahead area 中的页，都属于同一个 buffer pool 管理。


### Buffer Pool LRU List

如上所述，InnoDB Buffer Pool 会通过 LRU 算法作为管理页面的替换策略，将 LRU List 划分为两部分：LRU_young 与 LRU_old，参数的占比可以通过 ```innodb_old_blocks_pct``` 参数设置，默认是 37，也就是 LRU_old 为链表长度的 3/8 。

#### old-page 移动到 new-page

在页面读取时 (get/read ahead)，会先将页链入到 LRU_old 的头部；当页面第一次访问时 (read/write)，从 LRU_old 链表移动到 LRU_young 的头部，也就是整个 LRU 链表头。

<!--
全表扫描的所有页面，也遵循先读入 LRU_old，后移动到 LRU_young 的原则，但是会导致 BP 中的其他页面被替换出内存。为防止全表扫描的负面影响，InnoDB 提供了系统参数， innodb_old_blocks_time:只有当页面的后续访问与第一次访问时间间隔大于此值,才会移动
到 LRU 链表头。innodb_old_blocks_time 在 5.1.41 版本中引入。默认为 0,也就是说全表扫
描的页面会进入 LRU_young(链表头),一个大表的全表扫描会导致大量 page 被替换出内存。
-->

会通过如下函数进行判断是否要写入到 LRU_head 中。

{% highlight text %}
buf_page_make_young_if_needed()
 |-buf_page_peek_if_too_old()
 |-buf_page_make_young()
   |-buf_LRU_make_block_young()
     |-buf_LRU_remove_block()
     |-buf_LRU_add_block_low()
{% endhighlight %}

其中判断是否需要写入 LRU_head 的处理如下。

{% highlight c %}
ibool buf_page_peek_if_too_old(const buf_page_t* bpage)
{
    buf_pool_t*     buf_pool = buf_pool_from_bpage(bpage);

    if (buf_pool->freed_page_clock == 0) {
        /* 当前BP还没有淘汰任何页，说明在内存中还有足够的页，那么就不需要更新统计值，
           也不需要在LRU链表中移动；通常在warm-up阶段。 */
        return(FALSE);
    } else if (buf_LRU_old_threshold_ms && bpage->old) {
        unsigned access_time = buf_page_is_accessed(bpage); // 直接返回bpage->access_time
        /* 若当前页已经被访问过，那么会计算距上次访问的时间间隔是否超过了用户设置的
           innodb_old_blocks_time参数，如果未超过，则不会将页移动到LRU_head。
           PS. 这里有个整数溢出的情况，通常是50天，也就是如果页面据上次访问超过了50天，
               紧接着在此访问时，可能会判断没有超过阈值，从而没有移动到LRU_head。 */
        if (access_time > 0
            && ((ib_uint32_t) (ut_time_ms() - access_time))
            >= buf_LRU_old_threshold_ms) {
            return(TRUE);
        }
        buf_pool->stat.n_pages_not_made_young++;
        return(FALSE);
    } else {
        /* 此时，已经有页面淘汰过，也就是内存中的页已经不够用；但没有设置old_blocks_time参数，
           或者该页第一次被访问，那么就通过如下算法判断是否需要移动到LRU_head。*/
        return(!buf_page_peek_if_young(bpage));
    }
}

ibool buf_page_peek_if_young(const buf_page_t* bpage)
{
    buf_pool_t* buf_pool = buf_pool_from_bpage(bpage);

    /* 判断当前页是否足够新(Most Recently Update, MRU)，而不需要移动
       buf_pool->freed_page_clock:  该BP实例一共淘汰了多少页
       bpage->freed_page_clock   :  该页最近一次移动到LRU_head时，对应BP中该参数的取值
       buf_pool->curr_size       :  该BP当前使用的页面数量
       BUF_LRU_OLD_RATIO_DIV     :  宏定义为1024
       buf_pool->LRU_old_ratio   :  old LRU的占比，默认3/8时，该值为378
    公式意义解释:
       从该页最近一次移动到BP的LRU_head以来，BP在此期间淘汰的页数量，超过了LRU_yound列表长度
       的1/4，那么说明该页已经不够年轻，该页需要移动到LRU_head；否则该页属于MRU，不需要移动。
    */
    return((buf_pool->freed_page_clock & ((1UL << 31) - 1))
           < ((ulint) bpage->freed_page_clock
             + (buf_pool->curr_size
             * (BUF_LRU_OLD_RATIO_DIV - buf_pool->LRU_old_ratio)
             / (BUF_LRU_OLD_RATIO_DIV * 4))));
}
{% endhighlight %}

#### new-page 移动到 old-page

正常来说，每次访问页面时，都有可能会将该页移动到 LRU_head 中 (make young)，随着越来越多的页加入 LRU_head，那么原来在 LRU list 中的页也就慢慢退化到 LRU list 的 LRU_old list 部分。

实际上，在 ```class buf_page_t``` 类中有个成员变量 old 用来标示是否在 LRU 的 old list 中；该变量的设置基本是通过 ```buf_page_set_old()``` 函数进行设置。

##### 第一次读取

在第一读入到 BP 时，会加入到 LRU 链表的 LRU_old 头部，调用堆栈如下。

{% highlight text %}
buf_page_init_for_read()
 |-buf_LRU_add_block(bpage, TRUE)    参数TRUE表示添加到old-list
   |-buf_LRU_add_block_low()
{% endhighlight %}

当入参为 TRUE 时，会将页放到 LRU 的 old list 中，否则放在链表头部；当然，如果 LRU 链表比较小，那么会直接忽略该参数，将页放在头部。

{% highlight text %}
void buf_LRU_add_block_low(buf_page_t* bpage, ibool old)
{
    buf_pool_t* buf_pool = buf_pool_from_bpage(bpage);

    if (!old || (UT_LIST_GET_LEN(buf_pool->LRU) < BUF_LRU_OLD_MIN_LEN)) {

        UT_LIST_ADD_FIRST(buf_pool->LRU, bpage);

        bpage->freed_page_clock = buf_pool->freed_page_clock;
    } else {
#ifdef UNIV_LRU_DEBUG
        /* buf_pool->LRU_old must be the first item in the LRU list
        whose "old" flag is set. */
        ut_a(buf_pool->LRU_old->old);
        ut_a(!UT_LIST_GET_PREV(LRU, buf_pool->LRU_old)
             || !UT_LIST_GET_PREV(LRU, buf_pool->LRU_old)->old);
        ut_a(!UT_LIST_GET_NEXT(LRU, buf_pool->LRU_old)
             || UT_LIST_GET_NEXT(LRU, buf_pool->LRU_old)->old);
#endif /* UNIV_LRU_DEBUG */
        UT_LIST_INSERT_AFTER(buf_pool->LRU, buf_pool->LRU_old,
            bpage);

        buf_pool->LRU_old_len++;
    }

    ut_d(bpage->in_LRU_list = TRUE);

    incr_LRU_size_in_bytes(bpage, buf_pool);

    if (UT_LIST_GET_LEN(buf_pool->LRU) > BUF_LRU_OLD_MIN_LEN) {

        ut_ad(buf_pool->LRU_old);

        /* Adjust the length of the old block list if necessary */

        buf_page_set_old(bpage, old);
        buf_LRU_old_adjust_len(buf_pool);

    } else if (UT_LIST_GET_LEN(buf_pool->LRU) == BUF_LRU_OLD_MIN_LEN) {

        /* The LRU list is now long enough for LRU_old to become
        defined: init it */

        buf_LRU_old_init(buf_pool);
    } else {
        buf_page_set_old(bpage, buf_pool->LRU_old != NULL);
    }

    /* If this is a zipped block with decompressed frame as well
    then put it on the unzip_LRU list */
    if (buf_page_belongs_to_unzip_LRU(bpage)) {
        buf_unzip_LRU_add_block((buf_block_t*) bpage, old);
    }
}
{% endhighlight %}


### 其它

#### innodb_old_blocks_pct

该变量会通过 buf_LRU_old_ratio_update_instance() 函数进行更新，默认 37，范围是 5~95 。

{% highlight c %}
#define BUF_LRU_OLD_RATIO_DIV   1024
#define BUF_LRU_OLD_RATIO_MAX   BUF_LRU_OLD_RATIO_DIV
#define BUF_LRU_OLD_RATIO_MIN   51

uint buf_LRU_old_ratio_update_instance(buf_pool_t* buf_pool, uint old_pct, ibool adjust)
{
    uint    ratio;

    ratio = old_pct * BUF_LRU_OLD_RATIO_DIV / 100;
    if (ratio < BUF_LRU_OLD_RATIO_MIN) {
        ratio = BUF_LRU_OLD_RATIO_MIN;
    } else if (ratio > BUF_LRU_OLD_RATIO_MAX) {
        ratio = BUF_LRU_OLD_RATIO_MAX;
    }

    if (adjust) {
        buf_pool_mutex_enter(buf_pool);

        if (ratio != buf_pool->LRU_old_ratio) {
            buf_pool->LRU_old_ratio = ratio;

            if (UT_LIST_GET_LEN(buf_pool->LRU)
                >= BUF_LRU_OLD_MIN_LEN) {

                buf_LRU_old_adjust_len(buf_pool);
            }
        }

        buf_pool_mutex_exit(buf_pool);
    } else {
        buf_pool->LRU_old_ratio = ratio;
    }
    /* the reverse of
    ratio = old_pct * BUF_LRU_OLD_RATIO_DIV / 100 */
    return((uint) (ratio * 100 / (double) BUF_LRU_OLD_RATIO_DIV + 0.5));
}
{% endhighlight %}

如上，在计算时，实际上会乘以 BUF_LRU_OLD_RATIO_DIV ，然后再除以 100，也就意味着在内部使用时会大值乘于 10 。


### 读取

接下来看看如何读取一个页，其中实际分配缓存是在 buf_LRU_get_free_block() 函数中。这一函数是在用户线程中调用，而且只会从 free_list 中获取空闲页，即使是从 LRU_list 上获取的，也需要先将 LRU_list 中的页保存在 free_list 中。

尝试从 BP 中获取一个 block，大多数情况下 free_list 上是有足够的空闲页，因此可以直接分配，分配完之后，直接从 free_list 上删除。

如果 free_list 上没有空闲页了，那么尝试从 LRU_list 上分配，一般会经历如下循环。

{% highlight text %}
* iteration 0:
  * get a block from free list, success:done
  * if buf_pool->try_LRU_scan is set
    * scan LRU up to srv_LRU_scan_depth to find a clean block
    * the above will put the block on free list
    * success:retry the free list
  * flush one dirty page from tail of LRU to disk
    * the above will put the block on free list
    * success: retry the free list
* iteration 1:
  * same as iteration 0 except:
    * scan whole LRU list
    * scan LRU list even if buf_pool->try_LRU_scan is not set
* iteration > 1:
  * same as iteration 1 but sleep 10ms
@return the free control block, in state BUF_BLOCK_READY_FOR_USE */
{% endhighlight %}



{% highlight text %}
buf_page_get_gen()                      获取数据库中的页
 |-buf_pool_get()                       所在buffer pool实例
 |-buf_page_hash_lock_get()             获取锁
 |-rw_lock_s_lock()                     加锁
 |-buf_page_hash_get_low()              尝试从bp中获取页
 |-buf_pool_watch_is_sentinel()         干吗的???
 |-buf_read_page()
 | |-buf_read_page_low()
 |   |-buf_page_init_for_read()         初始化bp
 |     |-buf_LRU_get_free_block()       如果没有压缩，则直接获取空闲页
 |       |-buf_LRU_get_free_only()      尝试从free_list获取，有则直接返回即可
 |       |-buf_LRU_scan_and_free_block()
 |         |-buf_LRU_free_from_unzip_LRU_list()
 |         |-buf_LRU_free_from_common_LRU_list()
 |     |-buf_LRU_add_block()
 |     |
 |     |-buf_buddy_alloc()              压缩页，使用buddy系统
 |
 |-buf_read_ahead_random()              如果需要做随机预读


  |-fil_io()
 |-buf_block_get_state()         根据页的类型，判断是否需要进一步处理，如ZIP

buf_read_ahead_linear()
{% endhighlight %}

## 参考

关于 LRU list 被划分为 new 与 old 两部分的原因及意义，可参考 [InnoDB Buffer Pool](http://dev.mysql.com/doc/refman/en/innodb-buffer-pool.html) 。



<!--
Buffer Pool 空间如何初始化？
Buffer Pool 是通过什么数据结构管理的？
如何从 Buffer Pool 中分配 page ？
Buffer Pool 已满情况下，如何替换？
Buffer Pool 的 LRU list ， Flush list ， Free list 上分别有哪些操作？
LRU list flush 与 Flush list flush 有和不同？



[MySQL学习] 一个压缩Page从磁盘读入buffer pool的过程
http://mysqllover.com/?p=303

原创mysql内核源代码深度解析缓冲池bufferpool整体概述
http://www.2cto.com/database/201604/497862.html

MySQL · 性能优化· InnoDB buffer pool flush策略漫谈
http://www.linuxidc.com/Linux/2016-03/128829.htm

Introducing page_cleaner thread in InnoDB
https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in

MySQL 5.7: Page Cleaner的刷脏问题
http://mysqllover.com/?p=1113

【MySQL】mysql buffer pool结构分析
http://www.cnblogs.com/jiangxu67/p/3765708.html

原创mysql内核源代码深度解析缓冲池bufferpool整体概述
http://www.2cto.com/database/201604/497862.html

innodb buffer pool相关特性
http://www.cnblogs.com/justfortaste/p/5507584.html
-->


{% highlight text %}
{% endhighlight %}
