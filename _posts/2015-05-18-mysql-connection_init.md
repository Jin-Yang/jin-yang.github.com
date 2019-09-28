---
title: MySQL 链接方式
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,连接,connection
description: 与 Oracle 或者 Postgre 不同，MySQL 采用的是线程模型，在这里介绍通过 socket 链接到服务器之后，线程与链接直接是怎么处理的。
---

与 Oracle 或者 Postgre 不同，MySQL 采用的是线程模型，在这里介绍通过 socket 链接到服务器之后，线程与链接直接是怎么处理的。

<!-- more -->

## 简介

现在 MySQL 支持三种处理链接的方式：no-threads、one-thread-per-connection 和 pool-of-threads，默认使用 one-thread-per-connection。而线程池的方式，只有企业版才支持；单线程则通常用户调试或者嵌入式的模式，因此，在此主要介绍每个连接单线程的方式。

在启动时可以通过 \-\-thread-handling=XXX 参数指定，也可以在配置文件中指定，而当前使用的链接方式可以通过 ```show variables like 'thread_handling'``` 查看。注意，该选项是只读的，也就是链接方式只能在启动时进行设置。

MariaDB 在 5.5 引入了一个动态的线程池方案，可以根据当前请求的并发情况自动增加或减少线程数，在此的线程池就是 MariaDB 的解决方案。

1. 单线程<!--，one_thread_scheduler()--><br>在同一时刻，最多只能有一个链接连接到 MySQL ，其他的连接会被挂起，一般用于实验性质或者嵌入式应用。

2. 多线程<!--，one_thread_per_connection_scheduler()--><br>
    同一时刻可以支持多个链接同时连接到服务器，针对每个链接分配一个线程来处理这个链接的所有请求，直到连接断开，线程才会结束。<br>
    这种方式存在的问题就是需要为每个连接创建一个新的 thread，当并发连接数达到一定程度，性能会有明显下降，因为过多的线程会导致频繁的上下文切换，CPU cache 命中率降低和锁的竞争会更加激烈。

3. 线程池<!--，pool_of_threads_scheduler()--><br>解决多线程的方法就是降低线程数，这样就需要多个连接共用线程，这便引入了线程池的概念。线程池中的线程是针对请求的，而不是针对连接的，也就是说几个连接可能使用相同的线程处理各自的请求。

注意，后面主要介绍 MariaDB 的实现方式。

### 设置

如上所述，可以通过设置服务器的启动参数来设定连接的方式，通过 mysqld \-\-verbose \-\-help 命令可以查看所支持的选项，启动后可以通过 show status 查看与行状态，通过 show variables 查看启动时的变量。

{% highlight text %}
$ mysqld --thread-handling=no-threads/one-thread-per-connection/pool-of-threads

$ cat /etc/my.cnf
[mysqld]
thread_handling=pool-of-threads

mysql> SHOW VARIABLES LIKE 'thread_handling';       ← 查看连接配置
+-----------------+---------------------------+
| Variable_name   | Value                     |
+-----------------+---------------------------+
| thread_handling | one-thread-per-connection |
+-----------------+---------------------------+
1 row in set (0.01 sec)
{% endhighlight %}

除了上述的链接方式之外，为了防止链接过多，导致在管理时无法登陆，MariaDB 提供了额外的链接方式，可以通过设置如下的参数实现 \-\-extra-port=3308 \-\-extra-max-connections=1 。

注意，如果 extra-max-connections 设置为 2 则实际上可以创建三个链接，而且只支持 one-thread-per-connection 类似的方式。

### 监控状态

MySQL 启动后，会监听端口，当有新的客户端发起连接请求时，MySQL 将为其分配一个新的 thread，去处理此请求。从建立连接开始，CPU 要给它划分一定的 thread stack，然后进行用户身份认证，建立上下文信息，最后请求完成，关闭连接，释放资源。

高并发情况下，将给系统带来巨大的压力，不能保证性能。MySQL 通过线程缓存来是实现线程重用，减小这部分的消耗；一个连接断开，并不销毁承载其的线程，而是将此线程放入线程缓冲区，并处于挂起状态，当下一个新的连接到来时，首先去线程缓冲区去查找是否有空闲的线程，如果有，则使用之，如果没有则新建线程。

{% highlight text %}
mysql> SHOW VARIABLES LIKE 'thread_cache_size'; ← 可以重用线程的个数
+-------------------+-------+
| Variable_name     | Value |
+-------------------+-------+
| thread_cache_size | 9     |
+-------------------+-------+
1 row in set (0.03 sec)

mysql> SHOW STATUS LIKE 'threads%';             ← 查看状态
+-------------------+-------+
| Variable_name     | Value |
+-------------------+-------+
| Threads_cached    | 0     |       ← 已被线程缓存池缓存的线程个数
| Threads_connected | 2     |       ← 当前MySQL的连接数
| Threads_created   | 1065  |       ← 已创建线程个数，可用来判断thread_cache_size大小
| Threads_running   | 1     |       ← 正在运行的线程数
+-------------------+-------+
4 rows in set (0.13 sec)
{% endhighlight %}




## 源码导读

首先大致介绍线程管理。

### 数据结构

MariaDB 支持的链接类型可以通过 enum scheduler_types 查看，目前只支持上述的三种类型。根据启动时的配置项，会将所使用的链接方式会保存在 scheduler_functions thread_scheduler 变量中。

{% highlight c %}
struct scheduler_functions
{
  uint max_threads, *connection_count;
  ulong *max_connections;
  bool (*init)(void);
  bool (*init_new_connection_thread)(void);
  void (*add_connection)(THD *thd);
  void (*thd_wait_begin)(THD *thd, int wait_type);
  void (*thd_wait_end)(THD *thd);
  void (*post_kill_notification)(THD *thd);
  bool (*end_thread)(THD *thd, bool cache_thread);
  void (*end)(void);
};
{% endhighlight %}

在初始化时，会通过如下函数设置全局变量 thread_scheduler 的值。

{% highlight text %}
mysqld_main()
 |-init_common_variables()
   |-get_options()
{% endhighlight %}

该选项会在 mysqld_get_one_option()@sql/mysqld.cc 中处理，这个是在解析参数时调用的，相关的部分如下：

{% highlight c %}
case OPT_ONE_THREAD:
    thread_handling = SCHEDULER_NO_THREADS;
    break;
{% endhighlight %}

在主函数中，调用 logger.init_base() 之后，调用了一个 init_common_variables() 函数，里面初始化了这个变量的值。在 init_common_variables() 中调用了一个 get_options(); sql/mysqld.cc 函数，在该函数中对全局变量 thread_handling 进行初始化，然后设置相应的模式：

{% highlight c %}
if (thread_handling <= SCHEDULER_ONE_THREAD_PER_CONNECTION)
    one_thread_per_connection_scheduler(thread_scheduler, &max_connections,
                                             &connection_count);
else if (thread_handling == SCHEDULER_NO_THREADS)
    one_thread_scheduler(thread_scheduler);
else
    pool_of_threads_scheduler(thread_scheduler,  &max_connections,
                                         &connection_count);
{% endhighlight %}

在这三个函数中，其时就是设置了一个类型为 struct scheduler_functions 的 thread_scheduler 变量，只不过这些参数是一些函数指针罢了，也就是说在具体的调用中，只需要调用 add_connection 或 end_thread 即可，不需要知道到底是调用了哪个函数。




## 线程池

对于为每个连接创建一个线程的方式，当并发连接数与可以提供服务的 CPU 数的比例达到一定程度后，性能会有明显下降。这是因为过多的线程会导致较多的内存消耗、频繁的上下文切换以及 CPU cache 命中率下降；同时在访问临界资源（通常是用互斥量包裹的资源，用于防止不同的CPU同时修改导致错误）时会大量增加锁的竞争，这种情况对写入影响会更大。

为了解决这一问题，最好是服务的线程数要小于客户端的链接数，同时希望能发挥 CPU 的最大性能，因此，通常是每个 CPU 会有一个工作线程。线程池适合短查询以及 CPU 密集型（如OLTP）。

### 简介

线程池的特点。

1. 整个连接池内部被分成 N 个小的 group，默认为 CPU 的个数，可以通过参数 thread_pool_size 设置，group 之间通过 round robin 的方式分配连接，group 内通过竞争方式处理连接，一个 worker 线程只属于一个 group 。

2. 每个group有一个动态的listener，worker线程在循环取event时，发现队列为空时会充当listener通过epoll的方式监听数据，并将监听到的event放到group中的队列

3. 延时创建线程，group中的活动线程数为0或者group被阻塞时，worker线程会被创建，worker线程被创建的时间间隔会随着group内已有的线程数目的增加而变大

4. worker线程，数目动态变化，这也是相对于5.1版本的一个改进，并发较大时会创建更多的worker线程，当从队列中取不到event时work线程将休眠，超过thread_pool_idel_timeout后结束生命

5. timer线程，它会每隔一段时间做两件事情：1）检查每个group是否被阻塞，判定条件是：group中的队列中有event，但是自上次timer检查到现在还没有worker线程从中取出event并处理；2）kill超时连接并做一些清理工作

### 配置参数

线程池相关的参数都保存在 sql/sys_vars.cc 文件中，可以通过 show variables like 'thread_pool%'; 命令查看，相关的参数有：

* thread_pool_size，线程池中group的数目

    MariaDB 的线程池分成了不同的 group ，而且是按照到来 connection 的顺序进行分组的，如第一个 connection 分配到 group[0] ，那么第二个 connection 就分配到 group[1] ，是一种 Round Robin 的轮询分配方式，默认值是 CPU core 个数。

* thread_pool_idle_timeout，线程最大空闲时间

    如果某个线程空闲的时间大于这个参数，则线程退出。

* thread_pool_stall_limit，监控线程的间隔时间

    thread pool 有个监控线程，每隔一段时间，会检查每个 group 的线程可用数等状态，然后进行相应的处理，如 wake up 或者 create thread 。

* thread_pool_oversubscribe，允许的每个 group 上的活跃的线程数

    注意这并不是每个 group上 的最大线程数，而只是可以处理请求的线程数。

* thread_pool_max_threads，最大线程数



### 源码解析

如上的源码中，对于线程池，会在 pool_of_threads_scheduler() 中会将全局变量 thread_scheduler 初始化为 tp_scheduler_functions 。

{% highlight c %}
static scheduler_functions tp_scheduler_functions = {
  0,                                  // max_threads
  NULL,
  NULL,
  tp_init,                            // init
  NULL,                               // init_new_connection_thread
  tp_add_connection,                  // add_connection
  tp_wait_begin,                      // thd_wait_begin
  tp_wait_end,                        // thd_wait_end
  post_kill_notification,             // post_kill_notification
  NULL,                               // end_thread
  tp_end                              // end
};

void pool_of_threads_scheduler(struct scheduler_functions *func,
    ulong *arg_max_connections, uint *arg_connection_count)
{
  *func = tp_scheduler_functions;
  func->max_threads= threadpool_max_threads;
  func->max_connections= arg_max_connections;
  func->connection_count= arg_connection_count;
  scheduler_init();
}
{% endhighlight %}

也就是说，根据 tp_scheduler_functions 变量中的定义，对于 thread pool 方式，这种方式对应的初始函数为 tp_init()，创建新连接的函数为 tp_add_connection()，等待开始函数为 tp_wait_begin()，等待结束函数为 tp_wait_end() 。

其中 thread pool 涉及的源码在 sql/threadpool_{common,unix}.cc，其中初始化函数如下。


### 初始化

初始化的函数调用逻辑如下：

{% highlight text %}
tp_init()
 |-scheduler_init()
 |-thread_group_init()              # 对组进行初始化
 |-start_timer()                    # 开启监控线程timer_thread()
{% endhighlight %}

至此为止，thread pool 里面只有一个监控线程启动，而没有任何工作线程，直到有新的连接到来。

### 处理链接

当有新连接到来时，会调用 create_new_thread() 函数，而该函数实际会调用 tp_add_connection() 函数，其中比较重要的数据结构如下：

{% highlight c %}
struct connection_t {
  THD *thd;
  thread_group_t *thread_group;
  connection_t *next_in_queue;
  connection_t **prev_in_queue;
  ulonglong abs_wait_timeout;      // 等待超时时间
  bool logged_in;                  // 是否进行了登录验证
  bool bound_to_poll_descriptor;   // 是否添加到了epoll进行监听
  bool waiting;                    // 是否在等待状态，如I/O、sleep
};

struct thread_group_t {
  mysql_mutex_t      mutex;
  connection_queue_t queue;           // connection请求链表
  worker_list_t      waiting_threads;   // group中正在等待被唤醒的thread
  worker_thread_t    *listener;       // 当前group中用于监听的线程
  pthread_attr_t     *pthread_attr;
  int                pollfd;                     // epoll 文件描述符，用于绑定group中的所有连接
  int  thread_count;               // 线程数
  int  active_thread_count;//活跃线程数
  int  connection_count; //连接数
  /* Stats for the deadlock detection timer routine.*/
  int io_event_count;  //epoll产生的事件数
  int queue_event_count; //工作线程消化的事件数
  ulonglong last_thread_creation_time;
  int  shutdown_pipe[2];
  bool shutdown;
  bool stalled; // 工作线程是否处于停滞状态
} MY_ALIGNED(512);
{% endhighlight %}

调用逻辑。

{% highlight text %}
tp_add_connection()
 |-threads.append()                 # 首先添加到threads列表中
 |-alloc_connection()               # 申请连接
 |-queue_put()                      # 放到队列中
   |-wake_or_create_thread()        # 如果没有活跃的线程，那么就无法处理这个新到的请求
     |                                这时调用该函数
     |-wake_thread()                # 首先尝试唤醒group
{% endhighlight %}

先根据 thread_id 对 group_count 取模，找到所属的 group，然后调用 queue_put() 将此 connection 放到 group 中的 queue 中。

等待线程链表 waiting_threads 中的线程，如果没有等待中的线程，则需要创建一个线程。至此，新到的 connection 被挂到了 group 的 queue 上，这样一个连接算是 add 进队列了，那么如何处理这个连接呢？

由于是第一个连接到来，那么肯定没有 waiting_threads，此时会调用create_worker() 创建一个工作线程，也就是 worker_main() 。

{% highlight text %}
worker_main()
 |-get_event()                      # 获取要处理的连接，其中等待事件可能是登陆请求或socket中未读的字节
 | |-queue_get()                    # 从队列中获取相应的连接
 | |-listener()                     # 如果队列中没有监听的进程，则选择其中一个用来监听
 |
 |-handle_event()                   # 处理相应的连接
 | |-threadpool_add_connection()    # 如果没有登陆过，则调用该函数
 | |-threadpool_process_request()   # 否则已经登陆，则直接处理请求
 | |-set_wait_timeout()
 | |-start_io()
 |   |-mysql_socket_getfd()
 |   |-io_poll_associate_fd()       # 将新到连接的socket绑定到group的epoll上进行监听
 |
 |-my_thread_end()
{% endhighlight %}

其中 one thread per connection 中每个线程也是一个循环体，这两者的区别是，thread pool 的循环等待的是一个可用的 event，并不局限于某个固定的 connection 的 event；而前者的循环等待是等待固定连接上的 event，这就是两者最大的区别。

第一个连接到来，queue 中有了一个 connection，这时 get_event 便会从 queue 中获取到一个 connection，返回给 worker_main 线程。worker_main 接着调用 handle_event 进行事件处理。

每个新的连接到服务器后，其 socket 会绑定到 group 的 epoll 中，所以，如果 queue 中没有连接，需要从 epoll 中获取，每个 group 的所有连接的 socket 都绑定在 group 的 epoll 中，所以任何一个时刻，最多只有一个线程能够监听 epoll，如果epoll 监听到有 event的话，也会返回相应的connection，然后再调用handle_event进行处理。


当 group 中的线程没有任务执行时，所有线程都会在 get_event() 处等待，但是有两种等待方式，一种是在 epoll 上等待事件，每个 group 中只有一个线程会做这个事情，且这个会一直等待，直到有新的事件到来。

另一种是等待参数 thread_pool_idle_time 指定的时间，若超过了这个时间，那么当前的线程的 get_event 就会返回空，然后 worker_main 线程就会退出。如果在线程等待的过程被唤醒的话，那么就会继续在 get_event 中进行循环，等待新的事件。


<!--
http://www.cnblogs.com/nocode/archive/2013/05/25/3098317.html
http://blog.jobbole.com/87944/
-->

## 其它

介绍一些与线程相关的内容。

### 链接池和线程池

链接池是部署在应用端，为了防止客户端会频繁建立链接然后中断链接，当用户不需要该链接时，会在客户端缓存这些链接，这样如果下次用户再需要建立链接时可以直接复用该链接。

链接池可以有效减小服务器和客户端的执行时间，但是不会影响查询性能。


