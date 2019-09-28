---
title: MySQL 文件 IO
layout: post
comments: true
language: chinese
category: [mysql,database]
---


<!-- more -->

## 简介

在 InnoDB 中所有需要持久化的信息都需要文件操作，例如：表文件、重做日志文件、事务日志文件、备份归档文件等。InnoDB 对文件 IO 操作可以是煞费苦心，主要包括两方面：A) 对异步 IO 的实现；B) 对文件操作管理和 IO 调度的实现。

其主要实现代码集中在 os_file.* + fil0fil.* 文件中，其中 os_file.* 是实现基本的文件操作、异步 IO 和模拟异步 IO；fil0fil.* 是对文件 IO 做系统的管理和 space 结构化。

Innodb 的异步 IO 默认使用 libaio。


<!--
在innodb中，文件的操作是比较关键的，innodb封装了基本的文件操作，例如：文件打开与关闭、文件读写以及文件属性访问等。这些是基本的文件操作函数封装。在linux文件的读写方面，默认是采用pread/pwrite函数进行读写操作，如果系统部支持这两个函数，innodb用lseek和read、write函数联合使用来达到效果. 以下是innodb文件操作函数:
os_file_create_simple                        创建或者打开一个文件
os_file_create                                     创建或者打开一个文件，如果操作失败会重试，直到成功
os_file_close                                       关闭打开的文件
os_file_get_size                                   获得文件的大小
os_file_set_size                                   设置文件的大小并以0填充文件内容
os_file_flush                                        将写的内容fsync到磁盘
os_file_read                                        从文件中读取数据
os_file_write                                       将数据写入文件
innodb除了实现以上基本的操作以外，还实现了文件的异步IO模型，在Windows下采用的IOCP模型来进行处理（具
体可以见网上的资料），在linux下是采用aio来实现的，有种情况，一种是通过系统本身的aio机制来实现，还有一种是
通过多线程信号模拟来实现aio.这里我们重点来介绍，为了实现aio,innodb定义了slot和slot array,具体数据结构如下：

typedef struct os_aio_slot_struct
{
     ibool   is_read;                             /*是否是读操作*/
     ulint   pos;                                    /*slot array的索引位置*/
     ibool   reserved;                           /*这个slot是否被占用了*/
     ulint   len;                                     /*读写的块长度*/
     byte*   buf;                                   /*需要操作的数据缓冲区*/
     ulint   type;                                   /*操作类型：OS_FILE_READ OS_FILE_WRITE*/
     ulint   offset;                                 /*当前操作文件偏移位置，低32位*/
     ulint   offset_high;                        /*当前操作文件偏移位置，高32位*/
     os_file_t   file;                               /*文件句柄*/
     char*   name;                               /*文件名*/
     ibool   io_already_done;             /*在模拟aio的模式下使用，TODO*/
     void*   message1;
     void*   message2;
#ifdef POSIX_ASYNC_IO
     struct aiocb   control;                 /*posix 控制块*/
#endif
}os_aio_slot_t;

typedef struct os_aio_array_struct
{
 os_mutex_t  mutex;          /*slots array的互斥锁*/
 os_event_t  not_full;         /*可以插入数据的信号，一般在slot数据被aio操作后array_slot有空闲可利用的slot时发送*/
 os_event_t  is_empty;       /*array 被清空的信号，一般在slot数据被aio操作后array_slot里面没有slot时发送这个信号*/

 ulint   n_slots;                     /*slots总体单元个数*/
 ulint   n_segments;             /*segment个数，一般一个对应n个slot，n = n_slots/n_segments，一个segment作为aio一次的操作范围*/
 ulint   n_reserved;              /*有效的slots个数*/
 os_aio_slot_t* slots;         /*slots数组*/

 os_event_t*     events;         /*slots event array，暂时没弄明白做啥用的*/
}os_aio_array_t;

-->

其中数据刷盘的主要代码在 innodb/buf/buf0flu.c 中。

<pre style="font-size:0.8em; face:arial;">
buf_flush_batch()
 |-buf_do_LRU_batch()                         根据传入的type决定调用函数
 |-buf_do_flush_list_batch()
   |-buf_flush_page_and_try_neighbors()
     |-buf_flush_try_neighbors()
       |-buf_flush_page()                     刷写单个page
          |-buf_flush_write_block_low()       实际刷写单个page

    buf_flush_write_block_low调用buf_flush_post_to_doublewrite_buf （将page放到double write buffer中，并准备刷写）

    buf_flush_post_to_doublewrite_buf 调用 fil_io （ 文件IO的封装）

    fil_io 调用 os_aio （aio相关操作）

    os_aio 调用 os_file_write （实际写文件操作）

</pre>


其中buf_flush_batch 只有两种刷写方式： BUF_FLUSH_LIST 和 BUF_FLUSH_LRU 两种方式的方式和触发时机简介如下：

BUF_FLUSH_LIST: innodb master线程中 1_second / 10 second 循环中都会调用。触发条件较多（下文会分析）

BUF_FLUSH_LRU: 当Buffer Pool无空闲page且old list中没有足够的clean page时，调用。刷写脏页后可以空出一定的free page，供BP使用。

从触发频率可以看到 10 second 循环中对于 buf_flush_batch( BUF_FLUSH_LIST ) 的调用是10秒一次IO高负载的元凶所在。

我们再来看10秒循环中flush的逻辑：

    通过比较过去10秒的IO次数和常量的大小，以及pending的IO次数，来判断IO是否空闲，如果空闲则buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    如果脏页比例超过70，则 buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    否则  buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(10) );

可以看到由于SSD对于随机写的请求响应速度非常快，导致IO几乎没有堆积。也就让innodb误认为IO空闲，并决定全力刷写。

其中PCT_IO(N)  = innodb_io_capacity *N% ，单位是页。因此也就意味着每10秒，innodb都至少刷10000个page或者刷完当前所有脏页。

updated on 2013/10/31: 在5.6中官方的adaptive flush算法有所改变，但是空闲状态下innodb_io_capacity对于刷写page数量的影响仍然不改变。
UNIQUE 索引 IO 与聚簇索引 IO 完全一致，因为二者都必须读取页面，不能进行 Insert Buffer 优化。

{% highlight text %}


buf_page_get_gen()                            获取数据库中的页
 |-buf_pool_get()                             尝试从buffer pool中获取
 |-buf_page_hash_lock_get()                 # 判断所需的页是否在缓存中
 |-buf_read_page()                          # 如果不存在则直接从文件读取的buff_pool中
   |-buf_read_page_low()                    # 实际底层执行函数
     |-fil_io()
        |-os_aio()                          # 实际是一个宏定义，最终调用如下函数
        | |-os_aio_func()                   # 其入参包括了mode，标识同步/异步
        |   |-os_file_read_func()           # 同步读
        |   | |-os_file_pread()
        |   |   |-pread()
        |   |
        |   |-os_file_write_func()          # 同步写
        |   | |-os_file_pwrite()
        |   |   |-pwrite()
        |   |
        |   |-... ...                       # 对于异步操作，不同的mode其写入array会各不相同 #A
        |   |-os_aio_array_reserve_slot()   # 从相应队列中选取一个空闲slot，保存需要读写的信息
        |   | |
        |   | |-local_seg=... ...           # 1. 首先在任务队列中选择一个segment #B
        |   | |
        |   | |-os_mutex_enter()            # 2. 对队列加锁，遍历该segement，选择空闲的slot，如果没有则等待
        |   | |
        |   | |                             # 3. 如果array已经满了，根据是否使用AIO决定具体策略
        |   | |-os_aio_simulated_wake_handler_threads()    # 非native AIO，模拟唤醒
        |   | |-os_wait_event(array->not_full)             # native aio 则等待not_full信号
        |   | |
        |   | |-os_aio_array_get_nth_slot() # 4. 已经确定是有slot了，选择空闲的slot
        |   | |
        |   | |-slot... ...                 # 5. 将文件读写请求信息保存在slot，如目标文件、偏移量、数据等
        |   | |
        |   | |                             # 6. 对于Win AIO、Native AIO采取不同策略
        |   | |-ResetEvent(slot->handle)        # 对于Win调用该接口
        |   | |-io_prep_pread()                 # 而Linux AIO则根据传入的type，决定执行读或写
        |   | |-io_prep_pwrite()
        |   |
        |   |                               # 执行IO操作
        |   |-WriteFile()                       # 对于Win调用该函数
        |   |-os_aio_linux_dispatch()           # 对于LINUX_NATIVE_AIO需要执行该函数，将IO请求分发给内核层
        |   | |-io_submit()                 # 调用AIO接口函数发送
        |   |
        |   |-os_aio_windows_handle()       # Win下如果AIO_SYNC调用则通过该函数等待AIO结束
        |     |-... ...                     # 根据传入的array判断是否为sync_array
        |     |-WaitForSingleObject()           # 是则等待指定的slot aio操作完成
        |     |-WaitForMultipleObjects()        # 否则等待array中所有的aio操作完成
        |     |-GetOverlappedResult()       # 获取AIO的操作结果
        |     |-os_aio_array_free_slot()    # 最后释放当前slot
        |
 |      |-fil_node_complete_io()            # 如果是同步IO，则会等待完成，也就是确保调用os_aio()已经完成了IO操作
 |-buf_read_ahead_random()                  # 同时做预读

fil_aio_wait()
 |-os_aio_linux_handle()

os_aio_linux_handle
{% endhighlight %}

    分析完os_aio_windows_handle函数，接着分析Linux下同样功能的函数：os_aio_linux_handle
        无限循环，遍历array，直到定位到一个完成的I/O操作(slot->io_already_done)为止
        若当前没有完成的I/O，同时有I/O请求，则进入os_aio_linux_collect函数
            os_aio_linux_collect：从kernel中收集更多的I/O请求
                调用io_getevents函数，进入忙等，等待超时设置为OS_AIO_REAP_TIMEOUT

            /** timeout for each io_getevents() call = 500ms. */

            #define OS_AIO_REAP_TIMEOUT    (500000000UL)
                若io_getevents函数返回ret > 0，说明有完成的I/O，进行一些设置，最主要是将slot->io_already_done设置为TRUE

                slot->io_already_done = TRUE;
                若系统I/O处于空闲状态，那么io_thread线程的主要时间，都在io_getevents函数中消耗。


log_buffer_flush_to_disk()
 |-log_write_up_to()
</pre>



<ol type='A'><li>
<!--type = OS_FILE_READ; mode = OS_AIO_SYNC；-->
在这步中会选择不同的 array，包括了 os_aio_sync_array、os_aio_read_array、os_aio_write_array、os_aio_ibuf_array、os_aio_log_array。每个 aio array 在系统启动时调用 os0file.c::os_aio_init() 初始化。
<pre style="font-size:0.8em; face:arial;">
innobase_start_or_create_for_mysql() {
    ... ...
    os_aio_init(io_limit,            // 每个线程可并发处理pending IO的数量
        srv_n_read_io_threads,       // 处理异步read IO线程的数量
        srv_n_write_io_threads,      // 处理异步write IO线程的数量
        SRV_MAX_N_PENDING_SYNC_IOS); // 同步IO array的slots个数，
    ... ...
}

io_limit:
   windows = SRV_N_PENDING_IOS_PER_THREAD = 32
     linux = 8 * SRV_N_PENDING_IOS_PER_THREAD = 8 * 32 = 256

srv_n_read_io_threads:
    通过innobase_read_io_threads/innodb_read_io_threads参数控制
    因此可并发处理的异步read page请求为：io_limit * innodb_read_io_threads

srv_n_write_io_threads:
    通过innobase_write_io_threads/innodb_write_io_threads参数控制
    因此可并发处理的异步write请求为：io_limit * innodb_write_io_threads
    注意，当超过此限制时，必须将已有的异步IO部分写回磁盘，才能处理新的请求

SRV_MAX_N_PENDING_SYNC_IOS:
    同步IO不需要处理线程log thread、ibuf thread个数均为1
</pre>
接下来是创建 array 。
<pre style="font-size:0.8em; face:arial;">
os_aio_init()
 |-os_aio_array_create()
</pre>
异步 IO 主要包括两大类：A) 预读page，需要通过异步 IO 方式进行；B) 主动merge，Innodb 主线程对需要 merge 的 page 发出异步读操作，在read_thread 中进行实际 merge 处理。<!--
注：如何确定将哪些read io请求分配给哪些read thread？

    首先，每个read thread负责os_aio_read_array数组中的一部分。
    例如：thread0处理read_array[0, io_limit-1]；thread1处理read_array[io_limit, 2*io_limit – 1]，以此类推
    os_aio_array_reserve_slot函数中实现了array的分配策略(array未满时)。
    给定一个Aio read page，[space_id, page_no]，首先计算local_seg(local_thd):
    local_seg = (offset >> (UNIV_PAGE_SIZE_SHIFT + 6)) % array->n_segments;
    然后从read_array的local_seg * io_limit处开始向后遍历array，直到找到一个空闲slot。
    一来保证相邻的page，能够尽可能分配给同一个thread处理，提高aio(merge io request)性能；
    二来由于是循环分配，也基本上保证了每个thread处理的io基本一致。
--></li><br><li>



选择 segment 时，是根据偏移量来计算 segment 的，从而可以尽可能的将相邻的读写请求放到一起，从而有利于 IO 层的合并操作。
</li></ol>
<!--
http://blog.csdn.net/wudongxu/article/details/8647501  innodb学习（一）——innodb如何使用aio
http://blog.csdn.net/yuanrxdu/article/details/41418421  MySQL系列：innodb源码分析之文件IO
http://hedengcheng.com/?p=98   InnoDB AIO
http://mysqllover.com/?p=1444  InnoDB IO子系统介绍
-->
</p>






## 参考

XtraDB: The Top 10 enhancements
https://www.percona.com/blog/2009/08/13/xtradb-the-top-10-enhancements/

https://forums.cpanel.net/threads/innodb-corruption-repair-guide.418722/

http://www.itpub.net/thread-2083877-1-1.html

















当事务执行速度大于刷脏速度时，Ckp age和Buf age (innodb_flush_log_at_trx_commit!=1时) 都会逐步增长，当达到async点的时候，强制进行异步刷盘或者写Checkpoint，如果这样做还是赶不上事务执行的速度，则为了避免数据丢失，到达sync点的时候，会阻塞其它所有的事务，专门进行刷盘或者写Checkpoint。

因此从理论上来说,只要事务执行速度大于脏页刷盘速度，最终都会触发日志保护机制，进而将事务阻塞，导致MySQL操作挂起。



由于写Checkpoint本身的操作相比写脏页要简单，耗费时间也要少得多，且Ckp sync点在Buf sync点之后，因此绝大部分的阻塞都是阻塞在了Buf sync点，这也是当事务阻塞的时候，IO很高的原因，因为这个时候在不断的刷脏页数据到磁盘。例如如下截图的日志显示了很多事务阻塞在了Buf sync点：



log_free_check()
 |-log_check_margins()

buf_flush_wait_batch_end()






buf_flush_page_cleaner_coordinator() 该函数基本上由page_cleaner每隔1s调用一次
 |-buf_flush_page_cleaner_coordinator()
   |-page_cleaner_flush_pages_recommendation()
     |-af_get_pct_for_dirty()
     | |-buf_get_modified_ratio_pct()
     |   |-buf_get_total_list_len()
  |
     |-af_get_pct_for_lsn()    计算是否需要进行异步刷redo log
       |-log_get_max_modified_age_async()

af_get_pct_for_lsn()计算方法涉及变量
srv_adaptive_flushing_lwm

srv_flushing_avg_loops


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


定义BP中的页。
class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{


https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***


https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms














logs_empty_and_mark_files_at_shutdown()系统关闭时执行sharp checkpoint


当事务执行速度大于刷脏速度时，Ckp age和Buf age (innodb_flush_log_at_trx_commit!=1时) 都会逐步增长，当达到 async 点的时候，强制进行异步刷盘或者写 Checkpoint，如果这样做还是赶不上事务执行的速度，则为了避免数据丢失，到达 sync 点的时候，会阻塞其它所有的事务，专门进行刷盘或者写Checkpoint。

因此从理论上来说,只要事务执行速度大于脏页刷盘速度，最终都会触发日志保护机制，进而将事务阻塞，导致MySQL操作挂起。

class MVCC {
private:
    view_list_t             m_views;
};


定义BP中的页。

class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{


https://www.percona.com/blog/2013/10/30/innodb-adaptive-flushing-in-mysql-5-6-checkpoint-age-and-io-capacity/  ********
https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***

https://yq.aliyun.com/articles/64677
http://mysql.taobao.org/monthly/2015/06/01/
https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive
https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in



MySQL 5.6.2引入了一个新的后台线程page_cleaner，其中包括adaptive flushing、前端线程触发的 async flushing、空闲时的刷新、关闭刷新都是在这个线程完成；目前只有同步刷新放在了前台的查询线程中。

page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 没有空闲缓存页，需要按照LRU规则将最近最少使用的页(the least recently used pages)从LRU_list上移除，因此也被称为LRU_list刷新；
2. 需要重用redo log的空间，现在多数(包括InnoDB)数据库都是循环使用redo空间，如果要重用，只有保证redo对应的脏页已经刷新到磁盘才可以，也就是将the oldest modified non-flushed pages从flush_list上移除，被称之为flush_list；

在进行刷脏时，会导致IO出现尖刺，进而影响到redo log的刷盘，从而影响到系统的性能；为了解决这一问题，引入了 adaptive flushing 策略，这一策略主要作用于 flush_list 的刷脏，当然对 LRU_list 的刷脏也有一些影响。


https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms
https://www.percona.com/blog/2014/11/18/mysqls-innodb_metrics-table-how-much-is-the-overhead/
https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb

http://www.cnblogs.com/digdeep/p/4947694.html ******
http://hedengcheng.com/?p=220 ***
http://blog.itpub.net/30496894/viewspace-2121517/


Purge 实际上就是一个垃圾回收策略，简单来说，对于类似 "DELETE FROM t WHERE c = 1;" 的 DML，InnoDB 实际上并不会直接就删除，主要原因是为了回滚以及MVCC机制，简述如下：
1. 在记录的控制标志位中，标记该行已经删除；
2. 将修改列的前镜像保存到UNDO log中；
3. 修改聚集索引中的DB_TRX_ID、DB_ROLL_PTR系统列，前者标示最近一次修改的事务信息，后者则指向undo log中的记录，而 undo log 可能会存在同样的两列指向其历史记录。
另外，B+Tree的合并操作比较耗时，通过后台的异步线程可以避免阻塞用户的事务。

当事务已经提交，而且其它事务也不再依赖该记录了，那么就可以删除掉相应的记录，当然，也包括了二级索引对应的记录；这也就是 purge 线程的工作。

接下来，看看 purge 是如何工作的？


trx_purge是purge任务调度的核心函数，包含三个参数：
* n_purge_threads —>使用到的worker线程数
* batch_size  —-> 由innodb_purge_batch_size控制，表示一次Purge的记录数
* truncate —>是否truncate history list

trx_purge()
 |-trx_purge_dml_delay() 计算是否需要对dml延迟
 | ### 持有purge_sys->latch的x锁
 |-clone_oldest_view() 复制当前的view，也就是Class MVCC:m_views链表的中尾部
 |-trx_purge_attach_undo_recs() 获取需要清理的undo记录
 |
 | ### 多线程
 |-que_fork_scheduler_round_robin() 根据是否是单线程
 |-srv_que_task_enqueue_low() 将线程添加到队列中
 |-que_run_threads() 协调线程也会运行执行一个任务
 |-trx_purge_wait_for_workers_to_complete() 等待任务执行完成
 |
 | ### 单线程
 |
 |-trx_purge_truncate() 如果需要删除
 http://mysqllover.com/?p=696

purge 会复制一份系统中最老的 view，通过这一结构体，可以断定哪些回滚段需要回收。


mysql> show variables like 'innodb%purge%';
+-----------------------------------------+-------+
| Variable_name                           | Value |
+-----------------------------------------+-------+
| innodb_max_purge_lag                    | 0     |   如果purge操作比较慢，可以通过该参数设置dml操作的延迟时间
| innodb_max_purge_lag_delay              | 0     |   最大延迟不会超过该参数
| innodb_purge_batch_size                 | 300   |   一次处理多少页
| innodb_purge_rseg_truncate_frequency    | 128   |
| innodb_purge_run_now                    | OFF   |
| innodb_purge_stop_now                   | OFF   |
| innodb_purge_threads                    | 4     |   并发线程数
| innodb_trx_purge_view_update_only_debug | OFF   |
+-----------------------------------------+-------+
8 rows in set (0.00 sec)
Changes in 5.5

In 5.5 there is an option innodb-purge-threads=[0,1] to create a dedicated thread that purges asynchronously if there are UNDO logs that need to be removed. We also introduced another option innodb-purge-batch-size that can be used to fine tune purge operations. The batch size determines how many UNDO log pages purge will parse and process in one pass.


 The default setting is 20, this is the same as the hard coded value that is in previous InnoDB releases. An interesting side effect of this value is that it also determines when purge will free the UNDO log pages after processing them. It is always after 128 passes, this magic value of 128  is the same as the number of UNDO logs in the system tablespace, now that 5.5 has 128 rollback segments. By increasing the innodb-purge-batch-size the freeing of the UNDO log pages behaviour changes, it will increase the number of UNDO log pages that it removes in a batch when the limit of 128 is reached. This change was seen as necessary so that we could reduce the cost of removing the UNDO log pages for the extra 127 rollback segments that were introduced in 5.5. Prior to this change iterating over the 128 rollback segments to find the segment to truncate had become expensive.

Changes in 5.6

In 5.6 we have the same parameters as 5.5 except that innodb-purge-threads can now be between 0 and 32. This introduces true multi threaded purging. If the value is greater than 1 then InnoDB will create that many purge worker threads and a dedicated purge coordinator thread. The responsibility of the purge coordinator thread is to parse the UNDO log records and parcel out the work to the worker threads. The coordinator thread also purges records, instead of just sitting around and waiting for the worker threads to complete. The coordinator thread will divide the innodb-purge-batch-size by innodb-purge-threads and hand that out as the unit of work for each worker thread.

对于单表来说，会阻塞在 dict_index_t::lock 中，除非使用分区；对于多表来说是可以并发的。










####### 崩溃恢复(Crash Recovery)
Crash Recovery的起点，Checkpoint LSN存储位置？
InnoDB如何完成Redo日志的重做？
InnoDB如何定位哪些事务需要Rollback？
Crash Recovery需要等待Rollbach完成吗？
InnoDB各版本，在Crash Recovery流程上做了哪些优化？
mysqld_safe是否存在自动重启功能？

ha_recover

{% highlight text %}
innobase_init()
 |-innobase_start_or_create_for_mysql()
   |
   |-recv_sys_create()   创建崩溃恢复所需要的内存对象
   |-recv_sys_init()
   |
   |-srv_sys_space.open_or_create()                 通过系统表空间，获取flushed_lsn
   | |-read_lsn_and_check_flags()
   |   |-open_or_create()                           打开系统表空间
   |   |-read_first_page()                          读取第一个page
   |   |-buf_dblwr_init_or_load_pages()             加载double write buffer，如果ibdata日志损坏，则通过dblwr恢复
   |   |-validate_first_page()                      校验第一个页是否正常
   |   |-restore_from_doublewrite()                 如果有异常，则从dblwr恢复
   |
   |-srv_undo_tablespaces_init()                    对于undo log表空间恢复
   |
   |-recv_recovery_from_checkpoint_start()          ***从redo-log的checkpoint开始恢复；注意，正常启动也会调用
   | |-buf_flush_init_flush_rbt()                   创建一个红黑树，用于加速插入flush list
   | |-recv_recovery_on=true                        表示崩溃恢复已经开始，很多代码逻辑会通过该变量进行判断
   | |-recv_find_max_checkpoint()                   从日志中，找出最大的偏移量
   | |-
   | |-recv_group_scan_log_recs()
   | | |-recv_scan_log_recs()
   | |-recv_group_scan_log_recs()
   | |-recv_group_scan_log_recs()
   | |-recv_init_crash_recovery_spaces()
   |
   |-trx_sys_init_at_db_start()                     完成undo部分操作：收集未成功提交事务，按类别划分，前期准备
   | |-trx_sysf_get()                               完成undo-log的收集
   | |-trx_lists_init_at_db_start()                 根据undo信息重建未提交事务
   |
   |-recv_apply_hashed_log_recs()                   应用redo日志
   | |-recv_recover_page()                          实际调用recv_recover_page_func()
   |
   |-trx_purge_sys_create()                         构建purge，至此undo信息和undo事务创建结束
   |
   |-recv_recovery_from_checkpoint_finish()         完成崩溃恢复


   |-os_thread_create() 创建srv_master_thread线程
{% endhighlight %}


主线程主要完成 purge、checkpoint、dirty pages flush 等操作。



Database was not shutdown normally!   # InnoDB开始Crash Recovery{recv_init_crash_recovery_spaces()}
Starting crash recovery.



1. 读取Checkpoint LSN
2. 从Checkpoint LSN开始向前遍历Redo Log File
   重做从Checkpoint LSN开始的所有Redo日志
3. 重新构造系统崩溃时的事务
   Commit事务，等待Purge线程回收
   Prepare事务，由MySQL Server控制提交或者回滚(与Binlog 2PC相关)
   Active事务，回滚
4. 新建各种后台线程，Crash Recovery完成返回


正常关闭时，会在flush redo log和脏页后，做一次完全同步的checkpoint，并将checkpoint的LSN写到第一个ibdata文件的第一个page中，详细可以参考fil_write_flushed_lsn()。



http://mysqllover.com/?p=376

http://hedengcheng.com/?p=183

http://mysql.taobao.org/monthly/2015/05/01/

http://mysql.taobao.org/monthly/2016/05/01/

http://tech.uc.cn/?p=716

http://hedengcheng.com/?p=88InnoDB

http://mysqllover.com/?p=620

http://apprize.info/php/effective/6.html

http://www.cnblogs.com/chenpingzhao/p/5107480.html

https://www.xaprb.com/blog/2011/01/29/how-innodb-performs-a-checkpoint/

数据库内核分享

https://www.slideshare.net/frogd/inno-db-15344119

检查保存到磁盘的最大checkpoint LSN与redo-log的LSN是否一致；


MySQL · 引擎特性 · InnoDB 崩溃恢复过程

http://mysql.taobao.org/monthly/2015/06/01/













https://blogs.oracle.com/mysqlinnodb/entry/repeatable_read_isolation_level_in



http://mysql.taobao.org/monthly/2015/12/01/
http://hedengcheng.com/?p=148
read_view
http://www.cnblogs.com/chenpingzhao/p/5065316.html
http://kabike.iteye.com/blog/1820553
http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/
http://mysqllover.com/?p=696

隔离级别
详细可以查看row_search_mvcc()中的实现


row_search_for_mysql()
 |-row_search_no_mvcc()       # 对于MySQL内部使用的表(用户不可见)，不需要MVCC机制
 |-row_search_mvcc()

row_search_no_mvcc()用于MySQL的内部表使用，通常是一些作为一个较大任务的中间结果存储，所以希望其可以尽快处理，因此不需要MVCC机制。


innodb_force_recovery变量对应源码中的srv_force_recovery变量，




当innodb_fast_shutdown设置为0时，会导致purge一直工作近两个小时。？？？？？

从5.5版本开始，purge任务从主线程中独立出来；5.6开始支持多个purge线程，可以通过innodb_purge_threads变量控制。

purge后台线程的最大数量可以有32个，包括了一个coordinator线程，以及多个worker线程。

在innobase_start_or_create_for_mysql()函数中，会创建srv_purge_coordinator_thread以及srv_worker_thread线程。


srv_purge_coordinator_thread()
 |-srv_purge_coordinator_suspend()   如果不需要purge或者上次purge记录数为0，则暂停
 |-srv_purge_should_exit()           判断是否需要退出；fast_shutdown=0则等待所有purge操作完成
 |-srv_do_purge()                    协调线程的主要工作，真正调用执行purge操作的函数
 |
 |-trx_purge()                       防止上次循环结束后又新的记录写入，此处不再使用worker线程
 |
 |-trx_purge()                       最后对history-list做一次清理，确保所有worker退出

srv_worker_thread()


最后一次做trx_purge()时，为了防止执行时间过程，批量操作时不再采用innodb_purge_batch_size(300)指定的值，而是采用20。


InnoDB的数据组织方式采用聚簇索引，也就是索引组织表，而二级索引采用(索引键值,主键键值)组合来唯一确定一条记录。
无论是聚簇索引，还是二级索引，每条记录都包含了一个DELETED-BIT位，用于标识该记录是否是删除记录；除此之外，聚簇索引还有两个系统列：DATA_TRX_ID，DATA_ROLL_PTR，分别表示产生当前记录项的事务ID以及指向当前记录的undo信息。



从聚簇索引行结构，与二级索引行结构可以看出，聚簇索引中包含版本信息(事务号+回滚指针)，二级索引不包含版本信息，二级索引项的可见性如何判断？？？？


InnoDB存储引擎在开始一个RR读之前，会创建一个Read View。Read View用于判断一条记录的可见性。Read View定义在read0read.h文件中，其中最主要的与可见性相关的属性如下：

class ReadView {
private:
  trx_id_t        m_low_limit_id;  //
};


mtr_commit(struct mtr_t*)                 提交一个mini-transaction，调用mtr_t::commit()
 |-mtr_t::Command::execute()              写redo-log，将脏页添加到flush-list，并释放占用资源
   |-mtr_t::Command::prepare_write()      准备写入日志
   | |-fil_names_write_if_was_clean()
   |-mtr_t::Command::finish_write()












妈的文件整理文件

http://www.sysdb.cn/index.php/2016/01/14/innodb-recovery/

http://www.cnblogs.com/liuhao/p/3714012.html


持续集成 https://www.zhihu.com/question/23444990




buf_flush_batch


<!--
注意这里，在函数结束时并没有将batch_running设置为FALSE，因为这里对数据文件做的是异步写，设置标记位的工作留给了IO线程来完成
io_handler_thread-> fil_aio_wait-> buf_page_io_complete->buf_flush_write_complete->buf_dblwr_update()：
-->










FAQ系列 | 如何避免ibdata1文件大小暴涨

0、导读

    遇到InnoDB的共享表空间文件ibdata1文件大小暴增时，应该如何处理？

1、问题背景

用MySQL/InnoDB的童鞋可能也会有过烦恼，不知道为什么原因，ibdata1文件莫名其妙的增大，不知道该如何让它缩回去，就跟30岁之后男人的肚腩一样，汗啊，可喜可贺的是我的肚腩还没长出来，hoho~

正式开始之前，我们要先知道ibdata1文件是干什么用的。

ibdata1文件是InnoDB存储引擎的共享表空间文件，该文件中主要存储着下面这些数据：

        data dictionary
        double write buffer
        insert buffer/change buffer
        rollback segments
        undo space
        Foreign key constraint system tables

另外，当选项 innodb_file_per_table = 0 时，在ibdata1文件中还需要存储 InnoDB 表数据&索引。ibdata1文件从5.6.7版本开始，默认大小是12MB，而在这之前默认大小是10MB，其相关选项是 innodb_data_file_path，比如我一般是这么设置的：

    innodb_data_file_path = ibdata1:1G:autoextend

当然了，无论是否启用了 innodb_file_per_table = 1，ibdata1文件都必须存在，因为它必须存储上述 InnoDB 引擎所依赖&必须的数据，尤其是上面加粗标识的 rollback segments 和 undo space，它俩是引起 ibdata1 文件大小增加的最大原因，我们下面会详细说。
2、原因分析

我们知道，InnoDB是支持MVCC的，它和ORACLE类似，采用 undo log、redo log来实现MVCC特性的。在事务中对一行数据进行修改时，InnoDB 会把这行数据的旧版本数据存储一份在undo log中，如果这时候有另一个事务又要修改这行数据，就又会把该事物最新可见的数据版本存储一份在undo log中，以此类推，如果该数据当前有N个事务要对其进行修改，就需要存储N份历史版本（和ORACLE略有不同的是，InnoDB的undo log不完全是物理block，主要是逻辑日志，这个可以查看 InnoDB 源码或其他相关资料）。这些 undo log 需要等待该事务结束后，并再次根据事务隔离级别所决定的对其他事务而言的可见性进行判断，确认是否可以将这些 undo log 删除掉，这个工作称为 purge（purge 工作不仅仅是删除过期不用的 undo log，还有其他，以后有机会再说）。

那么问题来了，如果当前有个事务中需要读取到大量数据的历史版本，而该事务因为某些原因无法今早提交或回滚，而该事务发起之后又有大量事务需要对这些数据进行修改，这些新事务产生的 undo log 就一直无法被删除掉，形成了堆积，这就是导致 ibdata1 文件大小增大最主要的原因之一。这种情况最经典的场景就是大量数据备份，因此我们建议把备份工作放在专用的 slave server 上，不要放在 master server 上。

另一种情况是，InnoDB的 purge 工作因为本次 file i/o 性能是在太差或其他的原因，一直无法及时把可以删除的 undo log 进行purge 从而形成堆积，这是导致 ibdata1 文件大小增大另一个最主要的原因。这种场景发生在服务器硬件配置比较弱，没有及时跟上业务发展而升级的情况。

比较少见的一种是在早期运行在32位系统的MySQL版本中存在bug，当发现待 purge 的 undo log 总量超过某个值时，purge 线程直接放弃抵抗，再也不进行 purge 了，这个问题在我们早期使用32位MySQL 5.0版本时遇到的比较多，我们曾经遇到这个文件涨到100多G的情况。后来我们费了很大功夫把这些实例都迁移到64位系统下，终于解决了这个问题。

最后一个是，选项 innodb_data_file_path 值一开始就没调整或者设置很小，这就必不可免导致 ibdata1 文件增大了。Percona官方提供的 my.cnf 参考文件中也一直没把这个值加大，让我百思不得其解，难道是为了像那个经常被我吐槽的xx那样，故意留个暗门，好方便后续帮客户进行优化吗？（我心理太阴暗了，不好不好~~）

稍微总结下，导致ibdata1文件大小暴涨的原因有下面几个：

        有大量并发事务，产生大量的undo log；
        有旧事务长时间未提交，产生大量旧undo log；
        file i/o性能差，purge进度慢；
        初始化设置太小不够用；
        32-bit系统下有bug。

稍微题外话补充下，另一个热门数据库 PostgreSQL 的做法是把各个历史版本的数据 和 原数据表空间 存储在一起，所以不存在本案例的问题，也因此 PostgreSQL 的事务回滚会非常快，并且还需要定期做 vaccum 工作（具体可参见PostgreSQL的MVCC实现机制，我可能说的不是完全正确哈）
3、解决方法建议

看到上面的这些问题原因描述，有些同学可能觉得这个好办啊，对 ibdata1 文件大小进行收缩，回收表空间不就结了吗。悲剧的是，截止目前，InnoDB 还没有办法对 ibdata1 文件表空间进行回收/收缩，一旦 ibdata1 文件的肚子被搞大了，只能把数据先备份后恢复再次重新初始化实例才能恢复原先的大小，或者把依次把各个独立表空间文件备份恢复到一个新实例中，除此外，没什么更好的办法了。

当然了，这个问题也并不是不能防范，根据上面提到的原因，相应的建议对策是：

        升级到5.6及以上（64-bit），采用独立undo表空间，5.6版本开始就支持独立的undo表空间了，再也不用担心会把 ibdata1 文件搞大；
        初始化设置时，把 ibdata1 文件至少设置为1GB以上；
        增加purge线程数，比如设置 innodb_purge_threads = 8；
        提高file i/o能力，该上SSD的赶紧上；
        事务及时提交，不要积压；
        默认打开autocommit = 1，避免忘了某个事务长时间未提交；
        检查开发框架，确认是否设置了 autocommit=0，记得在事务结束后都有显式提交或回滚。



关于MySQL的方方面面大家想了解什么，可以直接留言回复，我会从中选择一些热门话题进行分享。 同时希望大家多多转发，多一些阅读量是老叶继续努力分享的绝佳助力，谢谢大家 :)

最后打个广告，运维圈人士专属铁观音茶叶微店上线了，访问：http://yejinrong.com 获得专属优惠




MySQL-5.7.7引入的一个系统库sys-schema，包含了一系列视图、函数和存储过程，主要是一些帮助MySQL用户分析问题和定位问题，可以方便查看哪些语句使用了临时表，哪个用户请求了最多的io，哪个线程占用了最多的内存，哪些索引是无用索引等。

其数据均来自performance schema和information schema中的统计信息。

MySQL 5.7.7 and higher includes the sys schema, a set of objects that helps DBAs and developers interpret data collected by the Performance Schema. sys schema objects can be used for typical tuning and diagnosis use cases.

MySQL Server blog中有一个很好的比喻：

For Linux users I like to compare performance_schema to /proc, and SYS to vmstat.

也就是说，performance schema和information schema中提供了信息源，但是，没有很好的将这些信息组织成有用的信息，从而没有很好的发挥它们的作用。而sys schema使用performance schema和information schema中的信息，通过视图的方式给出解决实际问题的答案。

查看是否安装成功
select * from sys.version;
查看类型
select * from sys.schema_object_overview where db='sys';
当然，也可以通过如下命令查看
show full tables from sys
show function status where db = 'sys';
show procedure status where db = 'sys'

user/host资源占用情况
SHOW TABLES FROM `sys` WHERE
    `Tables_in_sys` LIKE 'user\_%' OR
 `Tables_in_sys` LIKE 'host\_%'
IO资源使用，包括最近IO使用情况latest_file_io
SHOW TABLES LIKE 'io\_%'
schema相关，包括表、索引使用统计
SHOW TABLES LIKE 'schema\_%'
等待事件统计
SHOW TABLES LIKE 'wait%'
语句查看，包括出错、全表扫描、创建临时表、排序、空闲超过95%
SHOW TABLES LIKE 'statement%'
当前正在执行链接，也就是processlist
其它还有一些厂家的帮助函数，PS设置。
https://www.slideshare.net/Leithal/the-mysql-sys-schema
http://mingxinglai.com/cn/2016/03/sys-schema/
http://www.itpub.net/thread-2083877-1-1.html

x$NAME保存的是原始数据，比较适合通过工具调用；而NAME表更适合阅读，比如使用命令行去查看。


select digest,digest_text from performance_schema.events_statements_summary_by_digest\G
CALL ps_trace_statement_digest('891ec6860f98ba46d89dd20b0c03652c', 10, 0.1, TRUE, TRUE);
CALL ps_trace_thread(25, CONCAT('/tmp/stack-', REPLACE(NOW(), ' ', '-'), '.dot'), NULL, NULL, TRUE, TRUE, TRUE);

优化器调优
https://dev.mysql.com/doc/internals/en/optimizer-tracing.html


MySQL performance schema instrumentation interface(PSI)

struct PFS_instr_class {}; 基类


通过class page_id_t区分页，

class page_id_t {
private:
    ib_uint32_t     m_space;     指定tablespace
    ib_uint32_t     m_page_no;   页的编号





buf_page_get_gen()          获取数据库中的页
 |-buf_pool_get()           所在buffer pool实例
 |-buf_page_hash_lock_get()
 |-buf_page_hash_get_low()  尝试从bp中获取页
 |-buf_read_page()
   |-buf_read_page_low()
     |-buf_page_init_for_read()  初始化bp
    |-buf_LRU_get_free_block() 如果没有压缩，则直接获取空闲页
    |-buf_LRU_add_block()
    |
    |-buf_buddy_alloc()        压缩页，使用buddy系统
  |-fil_io()
 |-buf_block_get_state()         根据页的类型，判断是否需要进一步处理，如ZIP
 |-buf_read_ahead_random()

buf_read_ahead_linear()

http://www.myexception.cn/database/511937.html
http://blog.csdn.net/taozhi20084525/article/details/17613785
http://blogread.cn/it/article/5367
http://mysqllover.com/?p=303
http://www.cnblogs.com/chenpingzhao/p/5107480.html ？？？
https://docs.oracle.com/cd/E17952_01/mysql-5.7-en/innodb-recovery-tablespace-discovery.html
http://mysqllover.com/?p=1214



[mysqld]
innodb_data_file_path            = ibdata1:12M;ibdata2:12M:autoextend



<br><br><br><h1>文件 IO 操作</h1><p>
在 InnoDB 中所有需要持久化的信息都需要文件操作，例如：表文件、重做日志文件、事务日志文件、备份归档文件等。InnoDB 对文件 IO 操作可以是煞费苦心，主要包括两方面：A) 对异步 IO 的实现；B) 对文件操作管理和 IO 调度的实现。<br><br>

其主要实现代码集中在 os_file.* + fil0fil.* 文件中，其中 os_file.* 是实现基本的文件操作、异步 IO 和模拟异步 IO；fil0fil.* 是对文件 IO 做系统的管理和 space 结构化。<br><br>

Innodb 的异步 IO 默认使用 libaio。
</p>

<!--
在innodb中，文件的操作是比较关键的，innodb封装了基本的文件操作，例如：文件打开与关闭、文件读写以及文件属性访问等。这些是基本的文件操作函数封装。在linux文件的读写方面，默认是采用pread/pwrite函数进行读写操作，如果系统部支持这两个函数，innodb用lseek和read、write函数联合使用来达到效果. 以下是innodb文件操作函数:
os_file_create_simple                        创建或者打开一个文件
os_file_create                                     创建或者打开一个文件，如果操作失败会重试，直到成功
os_file_close                                       关闭打开的文件
os_file_get_size                                   获得文件的大小
os_file_set_size                                   设置文件的大小并以0填充文件内容
os_file_flush                                        将写的内容fsync到磁盘
os_file_read                                        从文件中读取数据
os_file_write                                       将数据写入文件
innodb除了实现以上基本的操作以外，还实现了文件的异步IO模型，在Windows下采用的IOCP模型来进行处理（具
体可以见网上的资料），在linux下是采用aio来实现的，有种情况，一种是通过系统本身的aio机制来实现，还有一种是
通过多线程信号模拟来实现aio.这里我们重点来介绍，为了实现aio,innodb定义了slot和slot array,具体数据结构如下：

typedef struct os_aio_slot_struct
{
     ibool   is_read;                             /*是否是读操作*/
     ulint   pos;                                    /*slot array的索引位置*/
     ibool   reserved;                           /*这个slot是否被占用了*/
     ulint   len;                                     /*读写的块长度*/
     byte*   buf;                                   /*需要操作的数据缓冲区*/
     ulint   type;                                   /*操作类型：OS_FILE_READ OS_FILE_WRITE*/
     ulint   offset;                                 /*当前操作文件偏移位置，低32位*/
     ulint   offset_high;                        /*当前操作文件偏移位置，高32位*/
     os_file_t   file;                               /*文件句柄*/
     char*   name;                               /*文件名*/
     ibool   io_already_done;             /*在模拟aio的模式下使用，TODO*/
     void*   message1;
     void*   message2;
#ifdef POSIX_ASYNC_IO
     struct aiocb   control;                 /*posix 控制块*/
#endif
}os_aio_slot_t;

typedef struct os_aio_array_struct
{
 os_mutex_t  mutex;          /*slots array的互斥锁*/
 os_event_t  not_full;         /*可以插入数据的信号，一般在slot数据被aio操作后array_slot有空闲可利用的slot时发送*/
 os_event_t  is_empty;       /*array 被清空的信号，一般在slot数据被aio操作后array_slot里面没有slot时发送这个信号*/

 ulint   n_slots;                     /*slots总体单元个数*/
 ulint   n_segments;             /*segment个数，一般一个对应n个slot，n = n_slots/n_segments，一个segment作为aio一次的操作范围*/
 ulint   n_reserved;              /*有效的slots个数*/
 os_aio_slot_t* slots;         /*slots数组*/

 os_event_t*     events;         /*slots event array，暂时没弄明白做啥用的*/
}os_aio_array_t;

-->
其中数据刷盘的主要代码在 innodb/buf/buf0flu.c 中。
<pre style="font-size:0.8em; face:arial;">
buf_flush_batch()
 |-buf_do_LRU_batch()                         根据传入的type决定调用函数
 |-buf_do_flush_list_batch()
   |-buf_flush_page_and_try_neighbors()
     |-buf_flush_try_neighbors()
       |-buf_flush_page()                     刷写单个page
          |-buf_flush_write_block_low()       实际刷写单个page

    buf_flush_write_block_low调用buf_flush_post_to_doublewrite_buf （将page放到double write buffer中，并准备刷写）

    buf_flush_post_to_doublewrite_buf 调用 fil_io （ 文件IO的封装）

    fil_io 调用 os_aio （aio相关操作）

    os_aio 调用 os_file_write （实际写文件操作）

</pre>


其中buf_flush_batch 只有两种刷写方式： BUF_FLUSH_LIST 和 BUF_FLUSH_LRU 两种方式的方式和触发时机简介如下：

BUF_FLUSH_LIST: innodb master线程中 1_second / 10 second 循环中都会调用。触发条件较多（下文会分析）

BUF_FLUSH_LRU: 当Buffer Pool无空闲page且old list中没有足够的clean page时，调用。刷写脏页后可以空出一定的free page，供BP使用。

从触发频率可以看到 10 second 循环中对于 buf_flush_batch( BUF_FLUSH_LIST ) 的调用是10秒一次IO高负载的元凶所在。

我们再来看10秒循环中flush的逻辑：

    通过比较过去10秒的IO次数和常量的大小，以及pending的IO次数，来判断IO是否空闲，如果空闲则buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    如果脏页比例超过70，则 buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(100) );

    否则  buf_flush_batch( BUF_FLUSH_LIST,PCT_IO(10) );

可以看到由于SSD对于随机写的请求响应速度非常快，导致IO几乎没有堆积。也就让innodb误认为IO空闲，并决定全力刷写。

其中PCT_IO(N)  = innodb_io_capacity *N% ，单位是页。因此也就意味着每10秒，innodb都至少刷10000个page或者刷完当前所有脏页。

updated on 2013/10/31: 在5.6中官方的adaptive flush算法有所改变，但是空闲状态下innodb_io_capacity对于刷写page数量的影响仍然不改变。
UNIQUE 索引 IO 与聚簇索引 IO 完全一致，因为二者都必须读取页面，不能进行 Insert Buffer 优化。

<pre style="font-size:0.8em; face:arial;">
buf_page_get_gen()
 |-buf_page_hash_lock_get()                 # 判断所需的页是否在缓存中
 |-buf_read_page()                          # 如果不存在则直接从文件读取的buff_pool中
   |-buf_read_page_low()                    # 实际底层执行函数
     |-fil_io()
        |-os_aio()                          # 实际是一个宏定义，最终调用如下函数
        | |-os_aio_func()                   # 其入参包括了mode，标识同步/异步
        |   |-os_file_read_func()           # 同步读
        |   | |-os_file_pread()
        |   |   |-pread()
        |   |
        |   |-os_file_write_func()          # 同步写
        |   | |-os_file_pwrite()
        |   |   |-pwrite()
        |   |
        |   |-... ...                       # 对于异步操作，不同的mode其写入array会各不相同 #A
        |   |-os_aio_array_reserve_slot()   # 从相应队列中选取一个空闲slot，保存需要读写的信息
        |   | |
        |   | |-local_seg=... ...           # 1. 首先在任务队列中选择一个segment #B
        |   | |
        |   | |-os_mutex_enter()            # 2. 对队列加锁，遍历该segement，选择空闲的slot，如果没有则等待
        |   | |
        |   | |                             # 3. 如果array已经满了，根据是否使用AIO决定具体策略
        |   | |-os_aio_simulated_wake_handler_threads()    # 非native AIO，模拟唤醒
        |   | |-os_wait_event(array->not_full)             # native aio 则等待not_full信号
        |   | |
        |   | |-os_aio_array_get_nth_slot() # 4. 已经确定是有slot了，选择空闲的slot
        |   | |
        |   | |-slot... ...                 # 5. 将文件读写请求信息保存在slot，如目标文件、偏移量、数据等
        |   | |
        |   | |                             # 6. 对于Win AIO、Native AIO采取不同策略
        |   | |-ResetEvent(slot->handle)        # 对于Win调用该接口
        |   | |-io_prep_pread()                 # 而Linux AIO则根据传入的type，决定执行读或写
        |   | |-io_prep_pwrite()
        |   |
        |   |                               # 执行IO操作
        |   |-WriteFile()                       # 对于Win调用该函数
        |   |-os_aio_linux_dispatch()           # 对于LINUX_NATIVE_AIO需要执行该函数，将IO请求分发给内核层
        |   | |-io_submit()                 # 调用AIO接口函数发送
        |   |
        |   |-os_aio_windows_handle()       # Win下如果AIO_SYNC调用则通过该函数等待AIO结束
        |     |-... ...                     # 根据传入的array判断是否为sync_array
        |     |-WaitForSingleObject()           # 是则等待指定的slot aio操作完成
        |     |-WaitForMultipleObjects()        # 否则等待array中所有的aio操作完成
        |     |-GetOverlappedResult()       # 获取AIO的操作结果
        |     |-os_aio_array_free_slot()    # 最后释放当前slot
        |
 |      |-fil_node_complete_io()            # 如果是同步IO，则会等待完成，也就是确保调用os_aio()已经完成了IO操作
 |-buf_read_ahead_random()                  # 同时做预读

fil_aio_wait()
 |-os_aio_linux_handle()

os_aio_linux_handle

    分析完os_aio_windows_handle函数，接着分析Linux下同样功能的函数：os_aio_linux_handle
        无限循环，遍历array，直到定位到一个完成的I/O操作(slot->io_already_done)为止
        若当前没有完成的I/O，同时有I/O请求，则进入os_aio_linux_collect函数
            os_aio_linux_collect：从kernel中收集更多的I/O请求
                调用io_getevents函数，进入忙等，等待超时设置为OS_AIO_REAP_TIMEOUT

            /** timeout for each io_getevents() call = 500ms. */

            #define OS_AIO_REAP_TIMEOUT    (500000000UL)
                若io_getevents函数返回ret > 0，说明有完成的I/O，进行一些设置，最主要是将slot->io_already_done设置为TRUE

                slot->io_already_done = TRUE;
                若系统I/O处于空闲状态，那么io_thread线程的主要时间，都在io_getevents函数中消耗。


log_buffer_flush_to_disk()
 |-log_write_up_to()
</pre>



<ol type='A'><li>
<!--type = OS_FILE_READ; mode = OS_AIO_SYNC；-->
在这步中会选择不同的 array，包括了 os_aio_sync_array、os_aio_read_array、os_aio_write_array、os_aio_ibuf_array、os_aio_log_array。每个 aio array 在系统启动时调用 os0file.c::os_aio_init() 初始化。
<pre style="font-size:0.8em; face:arial;">
innobase_start_or_create_for_mysql() {
    ... ...
    os_aio_init(io_limit,            // 每个线程可并发处理pending IO的数量
        srv_n_read_io_threads,       // 处理异步read IO线程的数量
        srv_n_write_io_threads,      // 处理异步write IO线程的数量
        SRV_MAX_N_PENDING_SYNC_IOS); // 同步IO array的slots个数，
    ... ...
}

io_limit:
   windows = SRV_N_PENDING_IOS_PER_THREAD = 32
     linux = 8 * SRV_N_PENDING_IOS_PER_THREAD = 8 * 32 = 256

srv_n_read_io_threads:
    通过innobase_read_io_threads/innodb_read_io_threads参数控制
    因此可并发处理的异步read page请求为：io_limit * innodb_read_io_threads

srv_n_write_io_threads:
    通过innobase_write_io_threads/innodb_write_io_threads参数控制
    因此可并发处理的异步write请求为：io_limit * innodb_write_io_threads
    注意，当超过此限制时，必须将已有的异步IO部分写回磁盘，才能处理新的请求

SRV_MAX_N_PENDING_SYNC_IOS:
    同步IO不需要处理线程log thread、ibuf thread个数均为1
</pre>
接下来是创建 array 。
<pre style="font-size:0.8em; face:arial;">
os_aio_init()
 |-os_aio_array_create()
</pre>
异步 IO 主要包括两大类：A) 预读page，需要通过异步 IO 方式进行；B) 主动merge，Innodb 主线程对需要 merge 的 page 发出异步读操作，在read_thread 中进行实际 merge 处理。<!--
注：如何确定将哪些read io请求分配给哪些read thread？

    首先，每个read thread负责os_aio_read_array数组中的一部分。
    例如：thread0处理read_array[0, io_limit-1]；thread1处理read_array[io_limit, 2*io_limit – 1]，以此类推
    os_aio_array_reserve_slot函数中实现了array的分配策略(array未满时)。
    给定一个Aio read page，[space_id, page_no]，首先计算local_seg(local_thd):
    local_seg = (offset >> (UNIV_PAGE_SIZE_SHIFT + 6)) % array->n_segments;
    然后从read_array的local_seg * io_limit处开始向后遍历array，直到找到一个空闲slot。
    一来保证相邻的page，能够尽可能分配给同一个thread处理，提高aio(merge io request)性能；
    二来由于是循环分配，也基本上保证了每个thread处理的io基本一致。
--></li><br><li>



选择 segment 时，是根据偏移量来计算 segment 的，从而可以尽可能的将相邻的读写请求放到一起，从而有利于 IO 层的合并操作。
</li></ol>
<!--
http://blog.csdn.net/wudongxu/article/details/8647501  innodb学习（一）——innodb如何使用aio
http://blog.csdn.net/yuanrxdu/article/details/41418421  MySQL系列：innodb源码分析之文件IO
http://hedengcheng.com/?p=98   InnoDB AIO
http://mysqllover.com/?p=1444  InnoDB IO子系统介绍
-->
</p>






## 参考

XtraDB: The Top 10 enhancements
https://www.percona.com/blog/2009/08/13/xtradb-the-top-10-enhancements/

https://forums.cpanel.net/threads/innodb-corruption-repair-guide.418722/

http://www.itpub.net/thread-2083877-1-1.html

















当事务执行速度大于刷脏速度时，Ckp age和Buf age (innodb_flush_log_at_trx_commit!=1时) 都会逐步增长，当达到async点的时候，强制进行异步刷盘或者写Checkpoint，如果这样做还是赶不上事务执行的速度，则为了避免数据丢失，到达sync点的时候，会阻塞其它所有的事务，专门进行刷盘或者写Checkpoint。

因此从理论上来说,只要事务执行速度大于脏页刷盘速度，最终都会触发日志保护机制，进而将事务阻塞，导致MySQL操作挂起。



由于写Checkpoint本身的操作相比写脏页要简单，耗费时间也要少得多，且Ckp sync点在Buf sync点之后，因此绝大部分的阻塞都是阻塞在了Buf sync点，这也是当事务阻塞的时候，IO很高的原因，因为这个时候在不断的刷脏页数据到磁盘。例如如下截图的日志显示了很多事务阻塞在了Buf sync点：



log_free_check()
 |-log_check_margins()

buf_flush_wait_batch_end()






buf_flush_page_cleaner_coordinator() 该函数基本上由page_cleaner每隔1s调用一次
 |-buf_flush_page_cleaner_coordinator()
   |-page_cleaner_flush_pages_recommendation()
     |-af_get_pct_for_dirty()
     | |-buf_get_modified_ratio_pct()
     |   |-buf_get_total_list_len()
  |
     |-af_get_pct_for_lsn()    计算是否需要进行异步刷redo log
       |-log_get_max_modified_age_async()

af_get_pct_for_lsn()计算方法涉及变量
srv_adaptive_flushing_lwm

srv_flushing_avg_loops


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


定义BP中的页。
class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{


https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***

page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 最近最少(the least recently used pages )使用的页将会从LRU_list上移除；
2. the oldest modified non-flushed pages从flush_list上移除；

https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive
https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in

MySQL 5.6.2引入了一个新的后台线程page_cleaner，

https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms














logs_empty_and_mark_files_at_shutdown()系统关闭时执行sharp checkpoint


当事务执行速度大于刷脏速度时，Ckp age和Buf age (innodb_flush_log_at_trx_commit!=1时) 都会逐步增长，当达到 async 点的时候，强制进行异步刷盘或者写 Checkpoint，如果这样做还是赶不上事务执行的速度，则为了避免数据丢失，到达 sync 点的时候，会阻塞其它所有的事务，专门进行刷盘或者写Checkpoint。

因此从理论上来说,只要事务执行速度大于脏页刷盘速度，最终都会触发日志保护机制，进而将事务阻塞，导致MySQL操作挂起。

class MVCC {
private:
    view_list_t             m_views;
};

buf_flush_wait_batch_end()




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


定义BP中的页。
class buf_page_t {
public:
        buf_page_state  state;

        UT_LIST_NODE_T(buf_page_t) list;    根据state的不同值决定了list的类型

        UT_LIST_NODE_T(buf_page_t) LRU;

                                       - BUF_BLOCK_NOT_USED:   free, withdraw
                                        - BUF_BLOCK_FILE_PAGE:  flush_list
                                        - BUF_BLOCK_ZIP_DIRTY:  flush_list
                                        - BUF_BLOCK_ZIP_PAGE:   zip_clean

struct buf_pool_t{


https://www.percona.com/blog/2013/10/30/innodb-adaptive-flushing-in-mysql-5-6-checkpoint-age-and-io-capacity/  ********
https://blogs.oracle.com/mysqlinnodb/entry/redo_logging_in_innodb  ***

https://yq.aliyun.com/articles/64677
http://mysql.taobao.org/monthly/2015/06/01/
https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb ***
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_6_multi_threaded
https://blogs.oracle.com/mysqlinnodb/entry/mysql_5_5_innodb_adaptive
https://blogs.oracle.com/mysqlinnodb/entry/introducing_page_cleaner_thread_in



MySQL 5.6.2引入了一个新的后台线程page_cleaner，其中包括adaptive flushing、前端线程触发的 async flushing、空闲时的刷新、关闭刷新都是在这个线程完成；目前只有同步刷新放在了前台的查询线程中。

page_cleaner线程负责刷脏，基本上是基于如下的两个因素：
1. 没有空闲缓存页，需要按照LRU规则将最近最少使用的页(the least recently used pages)从LRU_list上移除，因此也被称为LRU_list刷新；
2. 需要重用redo log的空间，现在多数(包括InnoDB)数据库都是循环使用redo空间，如果要重用，只有保证redo对应的脏页已经刷新到磁盘才可以，也就是将the oldest modified non-flushed pages从flush_list上移除，被称之为flush_list；

在进行刷脏时，会导致IO出现尖刺，进而影响到redo log的刷盘，从而影响到系统的性能；为了解决这一问题，引入了 adaptive flushing 策略，这一策略主要作用于 flush_list 的刷脏，当然对 LRU_list 的刷脏也有一些影响。


https://dev.mysql.com/doc/refman/5.6/en/innodb-system-tablespace.html
http://mysql.taobao.org/monthly/2015/07/01/

系统表空间包括了 InnoDB data dictionary(InnoDB相关的元数据)、doublewrite buffer、the change buffer、undo logs.

innodb_data_file_path
https://www.slideshare.net/Leithal/mysql-monitoring-mechanisms
https://www.percona.com/blog/2014/11/18/mysqls-innodb_metrics-table-how-much-is-the-overhead/
https://blogs.oracle.com/mysqlinnodb/entry/data_organization_in_innodb

http://www.cnblogs.com/digdeep/p/4947694.html ******
http://hedengcheng.com/?p=220 ***
http://blog.itpub.net/30496894/viewspace-2121517/


Purge 实际上就是一个垃圾回收策略，简单来说，对于类似 "DELETE FROM t WHERE c = 1;" 的 DML，InnoDB 实际上并不会直接就删除，主要原因是为了回滚以及MVCC机制，简述如下：
1. 在记录的控制标志位中，标记该行已经删除；
2. 将修改列的前镜像保存到UNDO log中；
3. 修改聚集索引中的DB_TRX_ID、DB_ROLL_PTR系统列，前者标示最近一次修改的事务信息，后者则指向undo log中的记录，而 undo log 可能会存在同样的两列指向其历史记录。
另外，B+Tree的合并操作比较耗时，通过后台的异步线程可以避免阻塞用户的事务。

当事务已经提交，而且其它事务也不再依赖该记录了，那么就可以删除掉相应的记录，当然，也包括了二级索引对应的记录；这也就是 purge 线程的工作。

接下来，看看 purge 是如何工作的？


trx_purge是purge任务调度的核心函数，包含三个参数：
* n_purge_threads —>使用到的worker线程数
* batch_size  —-> 由innodb_purge_batch_size控制，表示一次Purge的记录数
* truncate —>是否truncate history list

trx_purge()
 |-trx_purge_dml_delay() 计算是否需要对dml延迟
 | ### 持有purge_sys->latch的x锁
 |-clone_oldest_view() 复制当前的view，也就是Class MVCC:m_views链表的中尾部
 |-trx_purge_attach_undo_recs() 获取需要清理的undo记录
 |
 | ### 多线程
 |-que_fork_scheduler_round_robin() 根据是否是单线程
 |-srv_que_task_enqueue_low() 将线程添加到队列中
 |-que_run_threads() 协调线程也会运行执行一个任务
 |-trx_purge_wait_for_workers_to_complete() 等待任务执行完成
 |
 | ### 单线程
 |
 |-trx_purge_truncate() 如果需要删除
 http://mysqllover.com/?p=696

purge 会复制一份系统中最老的 view，通过这一结构体，可以断定哪些回滚段需要回收。


mysql> show variables like 'innodb%purge%';
+-----------------------------------------+-------+
| Variable_name                           | Value |
+-----------------------------------------+-------+
| innodb_max_purge_lag                    | 0     |   如果purge操作比较慢，可以通过该参数设置dml操作的延迟时间
| innodb_max_purge_lag_delay              | 0     |   最大延迟不会超过该参数
| innodb_purge_batch_size                 | 300   |   一次处理多少页
| innodb_purge_rseg_truncate_frequency    | 128   |
| innodb_purge_run_now                    | OFF   |
| innodb_purge_stop_now                   | OFF   |
| innodb_purge_threads                    | 4     |   并发线程数
| innodb_trx_purge_view_update_only_debug | OFF   |
+-----------------------------------------+-------+
8 rows in set (0.00 sec)
Changes in 5.5

In 5.5 there is an option innodb-purge-threads=[0,1] to create a dedicated thread that purges asynchronously if there are UNDO logs that need to be removed. We also introduced another option innodb-purge-batch-size that can be used to fine tune purge operations. The batch size determines how many UNDO log pages purge will parse and process in one pass.


 The default setting is 20, this is the same as the hard coded value that is in previous InnoDB releases. An interesting side effect of this value is that it also determines when purge will free the UNDO log pages after processing them. It is always after 128 passes, this magic value of 128  is the same as the number of UNDO logs in the system tablespace, now that 5.5 has 128 rollback segments. By increasing the innodb-purge-batch-size the freeing of the UNDO log pages behaviour changes, it will increase the number of UNDO log pages that it removes in a batch when the limit of 128 is reached. This change was seen as necessary so that we could reduce the cost of removing the UNDO log pages for the extra 127 rollback segments that were introduced in 5.5. Prior to this change iterating over the 128 rollback segments to find the segment to truncate had become expensive.

Changes in 5.6

In 5.6 we have the same parameters as 5.5 except that innodb-purge-threads can now be between 0 and 32. This introduces true multi threaded purging. If the value is greater than 1 then InnoDB will create that many purge worker threads and a dedicated purge coordinator thread. The responsibility of the purge coordinator thread is to parse the UNDO log records and parcel out the work to the worker threads. The coordinator thread also purges records, instead of just sitting around and waiting for the worker threads to complete. The coordinator thread will divide the innodb-purge-batch-size by innodb-purge-threads and hand that out as the unit of work for each worker thread.

对于单表来说，会阻塞在 dict_index_t::lock 中，除非使用分区；对于多表来说是可以并发的。












{% highlight text %}
{% endhighlight %}
