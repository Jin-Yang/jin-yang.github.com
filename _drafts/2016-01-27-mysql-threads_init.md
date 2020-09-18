---
title: MySQL 线程简介
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,杂项
description: 简单记录下 MySQL 常见的一些操作。
---


<!-- more -->

## 简介

我们知道，MySQL 采用的是线程模型，在此主要列举出 MySQL 可能存在的线程，一些常见的操作命令如下。

{% highlight text %}
$ pstack `pgrep mysqld`       打印所有线程的堆栈
{% endhighlight %}

在源码中，主要通过 ```mysql_thread_create()``` 函数创建线程。



## 同步机制

InnoDB 中有多种类型的处理线程，通常是采用一个协调线程 + 多个工作线程，例如 page cleaner 线程，可以通过 innodb_page_cleaners 变量设置。

当将 innodb_page_cleaners 变量设置为 7 时，会包含一个协调线程以及 6 个工作线程，

{% highlight text %}
pc_request()                              协调线程，设置slots
 |-os_event_set()                         通知等待的工作线程
   |-os_event_t::set()
     |-os_event_t::broadcast()
       |-mysql_cond_broadcast()
         |-inline_mysql_cond_broadcast()
           |-native_cond_broadcast()
{% endhighlight %}



## 主库线程

主要在链接主库时，会调用如下函数，执行主库的 dump 操作。

{% highlight text %}
mysql_binlog_send()
{% endhighlight %}



## 备库线程

{% highlight text %}
start_slave_threads()
 |-handle_slave_io()
 |-handle_slave_sql()
{% endhighlight %}



<!--
IO 线程，innodb<br>
主要是 innodb 的线程。

Monitor 监控线程，innodb<br>
innodb 的监控线程，监控其状态。
<pre style="font-size:0.8em; face:arial;">
Thread 13 (Thread 0x7f166cffd700 (LWP 3859)):
#0  pthread_cond_timedwait@@GLIBC_2.3.2 () at pthread_cond_timedwait.S:238
#1  0x0000000000bf4593 in os_cond_wait_timed (...) at storage/innobase/os/os0sync.cc:177
#2  0x0000000000bf4d72 in os_event_wait_time_low (...) at storage/innobase/os/os0sync.cc:705
#3  0x0000000000cb1aa9 in <font color="red">srv_monitor_thread (arg=0x0)</font> at storage/innobase/srv/srv0srv.cc:1914
#4  0x00007f1688089dc5 in start_thread (arg=0x7f166cffd700) at pthread_create.c:308
#5  0x00007f1686f5421d in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:113
</pre>
-->



## InnoDB

在 InnoDB 中，线程的创建都是通过 ```DECLARE_THREAD()``` 声明函数。


### 并发控制

在 InnoDB 中，与并发控制相关的有 innodb_thread_concurrency、innodb_thread_sleep_delay、innodb_concurrency_tickets 三个参数，下面解释一下他们是如何工作的。

MySQL 的各个模块包括了解析、优化和存储引擎，而存储采用插件式存储引擎，对应 class handler 基类，常见的调用接口包括了 read_row()、index_read()等。<br><br>

<!--
这样的方法有好几个，例如：read by index(有序读）sequential read（连续读）random read（随机读）
write_row;
update_row;
delete_row; 具体函数参见源码。
-->

在调用上述的这些 API 时，InnoDB 需要先检查内部的线程计数，如果内部线程计数超过了 innodb_thread_concurrency 就等待 innodb_thread_sleep_delay 变量设置的微妙后再尝试进入；如果第二次仍然不成功，则进入线程队列 sleep (FIFO)。之所以尝试两次，主要是为了减少线程等待计数和降低上下文切换。<br><br>

一但线程进入，就会得到 innodb_concurrency_tickets 票数，以便以后的 innodb_concurrency_tickets 次线程都不会再次检测，可以免 check 进入。<br><br>

当读写记录结束后退出 InnoDB 层时会将当前并发线程数减 1，并检查其是否低于 innodb_thread_concurrency，如果是的话，则从 FIFO 中唤醒一个等待的线程，保证并发线程不会超过 innodb_thread_concurrency 参数。<br><br>

<!--
2、当线程进入InnoDB层后，但在获取数据时由于锁请求无法得到满足而需要挂起时，线程将强制退出InnoDB层，当锁请求满足后，线程继续运行并强制进入到InnoDB层，这会导致实际并发线程数不是严格控制在innodb_thread_concurrency之内
-->

从 5.6 开始，如果使用了 GCC Build-in 的原子操作，在进入 Innodb 层的线程并发控制走与之前不同的逻辑，5.5也可以调用通过原子操作进行并发控制的逻辑，但需要打开只读选项innodb_thread_concurrency_timer_based来控制.
<pre style="font-size:0.8em; face:arial;">
ha_innobase::index_read()
  |-innobase_srv_conc_enter_innodb()
  |   |-srv_conc_enter_innodb()
  |       |-srv_conc_enter_innodb_with_atomics()      如果使用了GCC Build-in的原子操作
  |
  |-row_search_for_mysql()
  |-innobase_srv_conc_exit_innodb()
</pre>
在 5.6 引入了 adaptive sleep 方法，可以根据系统的负载做一些自适应调整，且新增了 innodb_adaptive_max_sleep_delay 参数。<br><br>

当 innodb_adaptive_max_sleep_delay>0 时，innodb_thread_sleep_delay 则会动态调整，以前者为上限。
<!--
  如果sleep了当前值以后还是不能进入，就把sleep时间+1；
  如果还有线程在sleep时，已经有了空闲线程，就把当前值的sleep 时间减半。
innodb_thread_sleep_delay降低比增加的更快，这样在并发线程数很高时，当限制并发数早就达到，其他线程的每次sleep时间会缓慢拉长。而当Innodb层很空闲时，sleep时间又会快速降到非常低
调整sleep到一个优化值的目的是，过小的sleep值可能会产生太多的线程切换，但过长的sleep时间，在并发比较空闲的时候又会影响性能。新的并发控制策略有利于随着负载的变化而做自适应调整。


Innodb_thread_concurrency不足

1线程因为锁等待而退出innodb层，当获取锁时可以直接重入innodb(跳过此参数检查)，因此系统并发执行的线程数可能大于此参数值。

2代码路径靠后，此时线程已经开始执行命令，进入到ha_innobase，应该在mysql_start_query前限制即thread_running。

为此好多技术高手开发了相应patch，从server层控制并发执行的线程数，下文将做描述。



当线程能够进入Innodb层时：
a.如果当前线程之前sleep过一次，并且当前innodb_thread_sleep_delay>20，将innodb_thread_sleep_delay减1
b.如果当前没有等待的线程，将innodb_thread_sleep_delay除以2
如果线程目前因并发控制无法进入Innodb层：
a.如果当前innodb_thread_sleep_delay>innodb_adaptive_max_sleep_delay
，将innodb_thread_sleep_delay的值设置为innodb_adaptive_max_sleep_delay
b.sleep  innodb_thread_sleep_delay毫秒
c.如果该线程已经sleep了超过1次，将innodb_thread_sleep_delay++
可以看到innodb_thread_sleep_delay降低比增加的更快。这样在并发线程数很高时，当限制并发数早就达到，其他线程的每次sleep时间会缓慢拉长。而当Innodb层很空闲时，sleep时间又会快速降到非常低

调整sleep到一个优化值的目的是，过小的sleep值可能会产生太多的线程切换，但过长的sleep时间，在并发比较空闲的时候又会影响性能。新的并发控制策略有利于随着负载的变化而做自适应调整。

另外注意，在使用原子操作进行并发控制后，就再也没有使用信号量让线程进行等待了。

这种自适应调整策略的效率依然有待于评估，也不确定其对某些工作负载是否存在不利影响。合适的参数配置应当由性能测试来给出。



那么什么是innodb_thread_concurrency最佳值？
这个值依赖许多因素，包括你的负载量，跑在什么样的硬件的软件上。如果你有1-2个CPU，你经常能做的很好，可以禁用它（innodb_thread_concurrency=0，从5.0.19开始还有更多的默认值）。多个CPU（4+CPU）的情况又有所不同。

MYSQL5.1以后对多核CPU支持改进了很多，将该值设置为0，或者2*CPU是一个比较好的注意。

waiting in InnoDB queue状态频繁出现，说明数据库并发太大，需要分析引起并发太大的原因或者增加数据库服务器（假设你根据硬件精确计算了该值）。

理论上你可以使用2*（NUMCPU+NUMDISKS)计算值以便能够激活2倍于CPU加磁盘的资源。然后在实际环境中，实践被证明，这个值并不是最优的。

如果正在使用innodb，这个值设置的小于CPU数量，mysql将不会利用到所有的CPU资源。

关于innodb_commit_concurrency

如你所见innodb_commit_concurrency仅用来保护行访问，使用内部结构和锁定分阶段提交，他仍是为保护变量。他限制innodb内核激活线程的commit。一个最佳的值也依赖很多其他因素。但是从5.1.36开始mysql不允许将该值设置为非0，该值只能为默认值0，mysql不限制并发提交。

如果没有电池保护，设置为innodb_flush_logs_at_trx_commit=1，但你可能想使用2或0。如果二进制日志被开启，他通常不重要，innodb会进行同步操作。

在unix平台测试的结果：
日志刷新方法设置为O_DIRECT
innodb_flush_logs_at_trx_commit 改参数的调整对性能和崩溃恢复数据丢失至关重要
1                       2                              0
                    提升130%                       提升10%
一般设置为2是一种平衡的方法，崩溃丢失1-2秒的数据，1是默认的，但是十分影响性能，如果是在要求十分安全的应用中，建议为1，并且设置日志同步sync_log。

http://www.gpfeng.com/?p=451      Pthread mutex与GCC atomic性能测试
-->
</p>







https://my.oschina.net/realfighter/blog/363853
http://blog.itpub.net/15480802/viewspace-1067518/



### 主线程

{% highlight text %}
srv_master_thread()
  作用：
{% endhighlight %}

主线程的当前状态可以通过 ```SHOW ENGINE INNODB STATUS\G``` 命令查看。

{% highlight text %}
mysql> SHOW ENGINE INNODB STATUS\G
... ...
-----------------
BACKGROUND THREAD
-----------------
srv_master_thread loops: 1 srv_active, 0 srv_shutdown, 78844 srv_idle
srv_master_thread log flush and writes: 78845
... ...
--------------
ROW OPERATIONS
--------------
... ...
Main thread process no. 6432, id 140497645459200, state: sleeping
... ...
{% endhighlight %}



如下是该线程的主要处理流程。

{% highlight text %}
srv_master_thread()
 |-srv_get_activity_count()         ← 初始化时获取活跃事件计数
 |
 |  ###BEGIN while
 |-srv_master_sleep()               ← 休眠1秒
 |-srv_check_activity()             ← 查看是否有活跃的事件，也就是比较当前值与历史值
 |
 |-srv_get_activity_count()         ← 有活跃事件，则更新历史值old_activity_count
 |-srv_master_do_active_tasks()     ← 处理活跃的事件
 | |-row_drop_tables_for_mysql_in_background()  ← 在没有查询时延迟删除表
 | |-log_free_check()                      ← 确保仍有足够的redo log空间
 | |-ibuf_merge_in_background()            ← 做insert buffer的合并操作
 | |-srv_sync_log_buffer_in_background()   ← 刷redo日志
 | |-srv_master_evict_from_table_cache()   ← 检查dict cache
 | |-log_checkpoint()
 |
 |-srv_master_do_idle_tasks()       ← 没有活跃事件
 | |-row_drop_tables_for_mysql_in_background()  ← 在没有查询时延迟删除表
 | |-log_free_check()                      ← 确保仍有足够的redo log空间
 | |-ibuf_merge_in_background()            ← 做insert buffer的合并操作
 | |-srv_master_evict_from_table_cache()   ← 检查dict cache
 | |-srv_sync_log_buffer_in_background()   ← 刷redo日志
 |   |-log_buffer_sync_in_background()     ← 根据innodb_flush_log_at_timeout参数判断刷新时间
 |     |-log_write_up_to()
 | |-log_checkpoint()
 |  ###END while
{% endhighlight %}

http://mysqllover.com/?p=636
http://www.penglixun.com/tech/database/innodb_master_thread.html

在 srv_check_activity() 函数中，实际会比较 ```srv_sys->activity_count``` 与历史记录的值，srv_inc_activity_count

从如上代码逻辑基本可以看出，实际上 idle 和 active 的时候，所做的事情几乎是一样的，不同的是，在 active 状态下，每 47 秒才检查 dict cache，每 7 秒才做一次 check point ；因此，在 idle 状态下，master 线程可能会更加繁忙。

{% highlight cpp %}
extern "C" os_thread_ret_t
DECLARE_THREAD(srv_master_thread)(
    void*   arg MY_ATTRIBUTE((unused)))
{
    ... ...
    while (srv_shutdown_state == SRV_SHUTDOWN_NONE) {

        srv_master_sleep();

        MONITOR_INC(MONITOR_MASTER_THREAD_SLEEP);

        if (srv_check_activity(old_activity_count)) {
            old_activity_count = srv_get_activity_count();
            srv_master_do_active_tasks();
        } else {
            srv_master_do_idle_tasks();
        }
    }

    while (srv_shutdown_state != SRV_SHUTDOWN_EXIT_THREADS
           && srv_master_do_shutdown_tasks(&last_print_time)) {

        /* Shouldn't loop here in case of very fast shutdown */
        ut_ad(srv_fast_shutdown < 2);
    }
    ... ...
    my_thread_end();
    os_thread_exit();
    DBUG_RETURN(0);
}
{% endhighlight %}





{% highlight text %}
{% endhighlight %}
