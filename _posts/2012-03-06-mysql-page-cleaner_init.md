---
title: Hello World !!!
layout: post
comments: true
language: chinese
category: [mysql, database]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---


MySQL 5.6.2 引入了一个新的后台线程 page_cleaner，将之前主线程中的 adaptive flushing、前端线程触发的 async flushing、空闲时的刷新、关闭刷新都是在这个线程完成；目前只有同步刷新放在了前台的用户查询线程中。

同时，引入了多个计数器。

{% highlight text %}
mysql> SELECT name, comment FROM information_schema.innodb_metrics WHERE name LIKE 'buffer_flush_%';
+---------------------------------------+-----------------------------------------------------------------+
| name                                  | comment                                                         |
+---------------------------------------+-----------------------------------------------------------------+
| buffer_flush_batch_scanned            | Total pages scanned as part of flush batch                      |
| buffer_flush_batch_num_scan           | Number of times buffer flush list flush is called               |
| buffer_flush_batch_scanned_per_call   | Pages scanned per flush batch scan                              |
| buffer_flush_batch_total_pages        | Total pages flushed as part of flush batch                      |
| buffer_flush_batches                  | Number of flush batches                                         |
| buffer_flush_batch_pages              | Pages queued as a flush batch                                   |
| buffer_flush_neighbor_total_pages     | Total neighbors flushed as part of neighbor flush               |
| buffer_flush_neighbor                 | Number of times neighbors flushing is invoked                   |
| buffer_flush_neighbor_pages           | Pages queued as a neighbor batch                                |
| buffer_flush_n_to_flush_requested     | Number of pages requested for flushing.                         |
| buffer_flush_n_to_flush_by_age        | Number of pages target by LSN Age for flushing.                 |
| buffer_flush_adaptive_avg_time_slot   | Avg time (ms) spent for adaptive flushing recently per slot.    |
| buffer_flush_adaptive_avg_time_thread | Avg time (ms) spent for adaptive flushing recently per thread.  |
| buffer_flush_adaptive_avg_time_est    | Estimated time (ms) spent for adaptive flushing recently.       |
| buffer_flush_avg_time                 | Avg time (ms) spent for flushing recently.                      |
| buffer_flush_adaptive_avg_pass        | Numner of adaptive flushes passed during the recent Avg period. |
| buffer_flush_avg_pass                 | Number of flushes passed during the recent Avg period.          |
| buffer_flush_avg_page_rate            | Average number of pages at which flushing is happening          |
| buffer_flush_lsn_avg_rate             | Average redo generation rate                                    |
| buffer_flush_pct_for_dirty            | Percent of IO capacity used to avoid max dirty page limit       |
| buffer_flush_pct_for_lsn              | Percent of IO capacity used to avoid reusable redo space limit  |
| buffer_flush_sync_waits               | Number of times a wait happens due to sync flushing             |
| buffer_flush_adaptive_total_pages     | Total pages flushed as part of adaptive flushing                |
| buffer_flush_adaptive                 | Number of adaptive batches                                      |
| buffer_flush_adaptive_pages           | Pages queued as an adaptive batch                               |
| buffer_flush_sync_total_pages         | Total pages flushed as part of sync batches                     |
| buffer_flush_sync                     | Number of sync batches                                          |
| buffer_flush_sync_pages               | Pages queued as a sync batch                                    |
| buffer_flush_background_total_pages   | Total pages flushed as part of background batches               |
| buffer_flush_background               | Number of background batches                                    |
| buffer_flush_background_pages         | Pages queued as a background batch                              |
+---------------------------------------+-----------------------------------------------------------------+
31 rows in set (0.01 sec)
{% endhighlight %}





page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 没有空闲缓存页，需要按照LRU规则将最近最少使用的页(the least recently used pages)从LRU_list上移除，因此也被称为LRU_list刷新；
2. 需要重用redo log的空间，现在多数(包括InnoDB)数据库都是循环使用redo空间，如果要重用，只有保证redo对应的脏页已经刷新到磁盘才可以，也就是将the oldest modified non-flushed pages从flush_list上移除，被称之为flush_list；

在进行刷脏时，会导致IO出现尖刺，进而影响到redo log的刷盘，从而影响到系统的性能；为了解决这一问题，引入了 adaptive flushing 策略，这一策略主要作用于 flush_list 的刷脏，当然对 LRU_list 的刷脏也有一些影响。


## Page Cleaner

为了提升刷脏效率，在 MySQL 5.7.4 版本里引入了多个 page cleaner 线程，从而可以达到并行刷脏的效果。采用协调线程+工作线程模式，协调线程本身也是工作线程，可通过 ```innodb_page_cleaners``` 变量设置，例如设置为 8 时就是一个协调线程，加 7 个工作线程。

{% highlight text %}
buf_flush_page_cleaner_coordinator()         该函数基本上由page_cleaner每隔1s调用一次
 |-buf_flush_page_cleaner_set_priority()     设置线程的优先级，默认为-20
 |-page_cleaner_flush_pages_recommendation()
 | |-af_get_pct_for_dirty()                  需要刷新多个页
 | | |-buf_get_modified_ratio_pct()
 | |   |-buf_get_total_list_len()
 | |
 | |-af_get_pct_for_lsn()                    计算是否需要进行异步刷redo log
 |   |-log_get_max_modified_age_async()
 |
 |-pc_request()
 |-pc_flush_slot()                           协调线程同时也处理请求
 |-pc_wait_finished()                        等待刷新完成

buf_flush_page_cleaner_worker
 |-my_thread_init()
 |-buf_flush_page_cleaner_set_priority()     设置线程的优先级，默认为-20
 | ###BEGIN###while
 |-os_event_wait()                           等待page_cleaner->is_requested事件
 |-pc_flush_slot()                           刷新
 | ###BEGIN###end
{% endhighlight %}

在启动时会初始化一个slot数组，大小为buffer pool instance的个数(buf_flush_page_cleaner_init)。

协调线程在决定了需要flush的page数和lsn_limit后，会设置slot数组，将其中每个slot的状态设置为PAGE_CLEANER_STATE_REQUESTED, 并设置目标page数及lsn_limit，然后唤醒worker线程 (pc_request)

worker线程被唤醒后，从slot数组中取一个未被占用的slot，修改其状态，表示已被调度，然后对该slot所对应的buffer pool instance进行操作。

为了支持对单个bp instance进行LRU/FLUSH_LIST的刷新，对原有代码做了大量的改动，worker线程可以直接调用buf_flush_LRU_list 及buf_flush_do_batch 指定buffer pool进行flush操作。 互相之间不干扰，因此可以并行刷脏。 改动整体而言比较简单。


page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 最近最少(the least recently used pages )使用的页将会从LRU_list上移除；
2. the oldest modified non-flushed pages从flush_list上移除；

https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***

https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded

https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive

https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in






buf_flush_wait_batch_end()


#define PCT_IO(p) ((ulong) (srv_io_capacity * ((double) (p) / 100.0)))



af_get_pct_for_lsn()计算方法涉及变量
srv_adaptive_flushing_lwm

srv_flushing_avg_loops

storage/innobase/log/log0log.cc

max_modified_age_sync

  |log_write_up_to()
    |-log_write_flush_to_disk_low()
      |-fil_flush()


#####FLUSH_LRU_LIST Checkpoint
srv_LRU_scan_depth


#####Async/Sync Flush Checkpoint
log_free_check() 用户线程调用
 |-log_check_margins()
   |-log_flush_margin()
   | |-log_write_up_to()
   |-log_checkpoint_margin() 执行sync操作，尝试空出足够的redo空间，避免checkpoint操作，可能会执行刷脏操作
     |-log_buf_pool_get_oldest_modification() 获取BP中最老的lsn，也就是LSN4
     | |-buf_pool_get_oldest_modification() 遍历各个BP实例，找出最大lsn，如果刚初始化完成则返回sys->lsn
     | 计算log->lsn-oldest_lsn，如果超过了max_modified_age_sync值，则执行sync操作

log_checkpoint_margin 核心函数，用于判断当前age情况，是否需要执行异步甚至是同步刷新。

buff async/sync是在前面，因为redo的刷新成本更低

buf_pool_resize() BP调整大小时的操作
 |-buf_pool_withdraw_blocks()


innodb_adaptive_flushing
innodb_adaptive_flushing_lwm 百分比，配置自适应flush机制的低水位(low water mark)，超过该限制之后，即使没有通过上述参数开启AF，仍然执行AF
innodb_io_capacity
innodb_io_capacity_max redo 刷盘的最大值，如果刷盘落后很多，那么IO可能会超过innodb_io_capacity而小于max
innodb_max_dirty_pages_pct 刷脏时，需要保证没有超过该值；注意，该值是一个目标，并不会影响刷脏的速率。
innodb_max_dirty_pages_pct_lwm 脏页的低水位，用于决定什么时候开启pre-flush操作，从而保证不会超过上面配置的百分比
innodb_flushing_avg_loops 决定了利用上述的值循环多少次之后重新计算dirty page和LSN，次数越少对外部的动态变化就越敏感


要刷新多少page和lsn主要代码在af_get_pct_for_dirty()和af_get_pct_for_lsn()中，其中主要控制adaptive flush的代码位于后者函数中。

http://www.cnblogs.com/Amaranthus/p/4450840.html

1.先判断redo log的容量是否到了innodb_adaptive_flushing_lwm低水位阀值。
2.是否配置了adaptive flush或者age超过了异步刷新的阀值。
3.lsn_age_factor=age占异步刷新阀值的比例。
4.要被刷新的比率=innodb_io_capacity_max/innodb_io_capacity*lsn_age_factor* sqrt(innodb_io_capacity)/7.5










https://www.percona.com/blog/2013/10/30/innodb-adaptive-flushing-in-mysql-5-6-checkpoint-age-and-io-capacity/  ********

https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***

https://yq.aliyun.com/articles/64677

http://mysql.taobao.org/monthly/2015/06/01/

https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***

https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded

https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive











MySQL buffer pool里的三种链表和三种page


buffer pool是通过三种list来管理的

1) free list
2) lru list
3) flush list

 

Buffer Pool 中的最小单位是 page，总共分为三种类型，包括了：

* free page<br>预分配，此类 page 未被使用，位于 free 链表中；
* clean page<br>该类型的 page 对应数据文件中的一个页面，但是页面没有被修改，位于 lru 链表中；
* dirty page<br>对应数据文件中的一个页面，但是页面已经被修改过，此种类型 page 位于 lru 链表和 flush 链表中。

在 flush list 中存在的页只能是脏页，而且最近修改的页保存在链表头部，当页面修改时，都会被封装为一个 mtr ，在提交的时候，则 mtr 涉及到的页面就会添加到 flush 链表的头部。

<!--
这样当flush链表做flush动作时，从flush链表的尾部开始scan，写出一定数量的dirty page到磁盘，推荐checkpoint点，使恢复的时间尽可能的短。除了flush链表本身的flush操作可以把dirty page从flush链表删除外，lru链表的flush操作也会让dirty page从flush链表删除。
-->


buffer pool lru list的工作原理

总的来说每当一个新页面被读取buffer pool之后，MySQL数据库InnoDB存储引擎都会判断当前buffer pool的free page是否足够，若不足，则尝试flush LRU链表。

在MySQL 5.6.2之前，用户线程在读入一个page (buf_read_page)、新建一个page(buf_page_create)、预读page(buf_read_ahead_linear) 等等操作时，都会在操作成功之后，调用buf_flush_free_margin函数，判断当前buffer pool是否有足够的free pages，若free pages不足，则进行LRU list flush，释放出足够的free pages，保证系统的可用性。


通过判断当前buf pool中需要flush多少dirty pages，才能够预留出足够的可被替换的页面(free pages or clean pages in LRU list tail)。

 

说明：
可用pages由以下两部分组成：
1. buf pool free list中的所有page，都是可以立即使用的。
2. buf pool LRU list尾部(5+2*BUF_READ_AHEAD_AREA)所有的clean pages。
其中：BUF_READ_AHEAD_AREA为64，是一个linear read ahead读取的大小，1 extent

 

由于buf_flush_free_margin函数是在用户线程中调用执行的，若需要flush LRU list，那么对于用户的响应时间有较大的影响。因此，在MySQL 5.6.2之后，InnoDB专门开辟了一个page cleaner线程，处理dirty page的flush动作(包括LRU list flush与flush list flush)，降低page flush对于用户的影响。

在MySQL 5.6.2前后的版本中，LRU list flush的不同之处在于是由用户线程发起，还是有后台page cleaner线程发起。但是，无论是用户线程，还是后台page cleaner线程，再决定需要进行LRU list flush之后，都会调用buf_flush_LRU函数进行真正的flush操作。

 
不同之处在于，MySQL 5.6.2之前，用户线程调用的buf_flush_free_margin函数，在判断是否真正需要进行LRU list flush时，将LRU list tail部分的clean pages也归为可以被replace的pages，不需要flush。而在page cleaner线程中，每隔1s，无论如何都会进行一次LRU list flush调用，无论LRU list tail中的page是否clean。这也可以理解，用户线程，需要尽量降低flush的概率，提高用户响应；而后台线程，尽量进行flush尝试，释放足够的free pages，保证用户线程不会堵塞。

 

Buffer Pool LRU/Flush List flush对比

1).LRU list flush，由用户线程触发(MySQL 5.6.2之前)；而Flush list flush由MySQL数据库InnoDB存储引擎后台srv_master线程处理。(在MySQL 5.6.2之后，都被迁移到page cleaner线程中)
 
2).LRU list flush，其目的是为了写出LRU 链表尾部的dirty page，释放足够的free pages，当buf pool满的时候，用户可以立即获得空闲页面，而不需要长时间等待；Flush list flush，其目的是推进Checkpoint LSN，使得InnoDB系统崩溃之后能够快速的恢复。
 
3).LRU list flush，其写出的dirty page，需要移动到LRU链表的尾部(MySQL 5.6.2之前版本)；或者是直接从LRU链表中删除，移动到free list(MySQL 5.6.2之后版本)。Flush list flush，不需要移动page在LRU链表中的位置。
 
4).LRU list flush，由于可能是用户线程发起，已经持有其他的page latch，因此在LRU list flush中，不允许等待持有新的page latch，导致latch死锁；而Flush list flush由后台线程发起，未持有任何其他page latch，因此可以在flush时等待page latch。
 
5).LRU list flush，每次flush的dirty pages数量较少，基本固定，只要释放一定的free pages即可；Flush list flush，根据当前系统的更新繁忙程度，动态调整一次flush的dirty pages数量，量很大。

 

buffer pool free list工作原理

free链表里存放的是空闲页面，初始化的时候申请一定数量的page，在使用的过程中，每次成功load页面到内存后，都会判断free page是否够用，如果不够用的话，就flush lru链表和flush链表来释放free page，这就可以满足其他进程在申请页面，使系统可用。





{% highlight text %}
{% endhighlight %}
