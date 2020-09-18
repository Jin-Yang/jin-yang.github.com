---
Date: October 19, 2013
title: 分布式缓存 memcached
layout: post
comments: true
language: chinese
category: [linux, network]
---

通常 Web 应用的数据都会持久化到数据库中，随着应用数据量以及访问量的增加，就会导致数据库负担加重，从而导致网站的延迟。

Memcached 是一个高性能的分布式内存缓存服务器，用于缓存数据库的查询结果，从而减少数据库的访问次数，提高动态 Web 应用的速度、提高扩展性。



<!-- more -->

# 简介

分布式缓存在设计时需要考虑几点常见的原则：A) 缓存本身的水平线性扩展；B) 缓存大并发下本身的性能问题，包括内存的管理问题，如内存的分配、管理、回收机制；C) 避免缓存的单点故障问题，以及分布式系统常见的多副本和副本一致性问题。

![memcached logo](/images/linux/memcached-logo.png "memcached logo"){: .pull-center}

memcached 采用基于文本的简单通讯协议，可以直接通过 telnet 进行操作；其事件处理是基于 libevent；虽然，我们说 memcached 是分布式缓存服务器，但实际上各个服务器之间并不通讯，其分布式主要是通过客户端实现。



## 安装、使用

在 CentOS 上可以通过 yum 简单安装，或者直接通过源码安装。

{% highlight text %}
# yum install memcached                                # 直接安装
# systemctl start memcached                            # 启动
# ./configure && make && make test && make install     # 通过源码编译

$ memcached -p 11211 -m 64m -d                         # 后台启动
$ telnet 127.1 11211                                   # 通过telnet链接
add foobar 0 60 5                                      # 添加，由重复则失败
get foobar                                             # 获取foobar的值
set foobar 0 60 5                                      # 存在则更新，否则添加
delete foobar                                          # 删除
stats                                                  # 查看状态，也可以使用memcached-tool
{% endhighlight %}

对于 add 和 set 命令，其中 0 为标志，60 表示数据存放 60s，5 表示放入多大数据。

上述是简单的启动和使用方式，常见的启动选项可以参考如下，详细内容查看 man 1 memcached 。

{% highlight text %}
memcached [options]
   -p [num]
       指定 TCP 的监听端口，默认使用 11211。
   -c [num]
       可以同时处理的最大链接数，默认是 1024 。
   -b [num]
       设置 backlog 队列的大小，默认是 1024 。
   -m [num]
       用户可以使用多少MB的内存，默认是 64M 。
   -d
       后台运行。
   -P [filename]
       后台启动时指定 pidfile 的路径。
{% endhighlight %}


# 链接、线程处理

线程通过 LIBEVENT_THREAD *threads 表示，

{% highlight text %}
main()
  |-... ...                               # 设置进程打开文件个数限制等
  |-daemonize()                           # 如果需要后台运行
  | |-fork()
  |-mlockall()                            # 锁定内存，保证内存不会换出
  |
  |-event_init()                          # 1. 初始化主线程的event_base，也就是libevent实例
  |-stats_init()                          # 统计状态的初始化
  |-assoc_init()                          # hash表的初始化
  |-conn_init()                           # 连接初始化
  |-slabs_init()                          # slabs结构的初始化
  |-sigignore()                           # 忽略SIGPIPE信号
  |-memcached_thread_init()               # 2. 创建工作线程，默认是4个，可以通过pstree -p PID查看
  | |
  | |-pipe()                              # 3. 创建pipe用于主线程和工作线程通讯
  | |
  | |-setup_thread()                      # 创建每个线程自己的libevent的event_base
  | | |-event_init()                      # 每个独立的线程都包含独立的event_base
  | | |-event_set()                       # 创建pipe的读事件监听，回调thread_libevent_process方法
  | | |-event_base_set()
  | | |-event_add()                       # 添加事件操作
  | | |-cq_init()                         # 4. 初始化每个工作线程的队列
  | | |-pthread_mutex_init()              # 初始化线程池
  | | |-cache_create()
  | |
  | |-create_worker()                     # 循环创建线程，真正的创建工作线程，实际是worker_libevent()
  |   |-pthread_attr_init()               # 属性初始化
  |   |-pthread_create()                  # 5. 创建线程worker_libevent()，直接启动
  |
  |-start_assoc_maintenance_thread()      # 启动
  |-start_item_crawler_thread()
  |-start_lru_maintainer_thread()
  |-start_slab_maintenance_thread()
  |
  |-clock_handler()                       # 初始化时间设置
  |
  |-server_sockets()                      # 6. 创建socket，绑定地址，设置非阻塞模式，包括TCP/UDP
  | |-server_socket()                     # 实际的绑定函数
  |   |-conn_new()                        # 7. 将监听socket添加到main_base的libevent的事件队列中
  |
  |-save_pid()                            # 保存PID到一个文件中
  |-event_base_loop()                     # 8. 启动主线程的libevetn循环

worker_libevent()                         # 新建的子线程，会初始化子线程的libevent
  |-register_thread_initialized()
  |-event_base_loop()                     # 开启事件循环，每个线程有自己的事件处理机制，启动libevent

event_handler()                           # Master/Worker线程libevent回调函数，此时有网络事件到达
  |-drive_machine()                       # 进入业务处理状态机

thread_libevent_process()                 # pipe接收回调函数
  |-read()                                # 读取pipe中的信息
{% endhighlight %}

主线程中如果有新的连接，会向其中一个线程的 pipe 中写入 1，子线程读取 pipe 中的数据，如果为 1，则说明从 pipe 中获取的数据是正确的。

在初始化完成之后，会忽略 SIGPIPE 信号，其中 PIPE 信号是当网络连接一端已经断开，这时发送数据，会发送 RST 包，当再次发送数据，会触发 PIPE 信号，而 PIPE 信号的默认动作是退出进程，因此忽略。


## 数据结构

CQ_ITEM 是主线程 accept() 后返回的已建立连接的 fd 的封装，而 CQ 是一个管理 CQ_ITEM 的单向链表。

{% highlight c %}
/* An item in the connection queue. */
typedef struct conn_queue_item CQ_ITEM;
struct conn_queue_item {
    int               sfd;
    enum conn_states  init_state;
    int               event_flags;
    int               read_buffer_size;
    enum network_transport     transport;
    CQ_ITEM          *next;
};

/* A connection queue. */
typedef struct conn_queue CQ;
struct conn_queue {
    CQ_ITEM *head;
    CQ_ITEM *tail;
    pthread_mutex_t lock;
};

typedef struct {
    pthread_t thread_id;               /* unique ID of this thread */
    struct event_base *base;           /* libevent handle this thread uses */
    struct event notify_event;         /* listen event for notify pipe */
    int notify_receive_fd;             /* receiving end of notify pipe */
    int notify_send_fd;                /* sending end of notify pipe */
    struct thread_stats stats;         /* Stats generated by this thread */
    struct conn_queue *new_conn_queue; /* queue of new connections to handle */
    cache_t *suffix_cache;             /* suffix cache */
} LIBEVENT_THREAD;

typedef struct conn conn;
struct conn {
    int    sfd;
    sasl_conn_t *sasl_conn;
    bool authenticated;
    enum conn_states  state;
    enum bin_substates substate;
    rel_time_t last_cmd_time;
    struct event event;
    short  ev_flags;
    ... ...
};
{% endhighlight %}

LIBEVENT_THREAD 是 memcached 里的线程结构封装，如上所示，每个线程都包含一个 CQ 队列，一条通知管道 pipe，一个 libevent 的实例 event_base 以及线程的状态。

另外一个重要的结构是对每个网络连接的封装 conn，这个结构体的成员变量比较多，暂时只列举了部分。


## 线程交互

memcached 采用典型的 Master-Worker 线程模型，其模型很简单，Master 监听网络链接，接受链接请求之后通过线程间通讯来唤醒 Worker 线程，后续的读写操作都是通过 Worker 完成。

主线程和各个线程的处理都是通过 libevent 处理，通过实例化多个 libevent 实例实现，分别对应了一个主线程和 N 个 workers 线程。Master 线程和 Workers 线程全部都是通过 libevent 管理网络事件，也就是说实际上每个线程都是一个单独的 libevent 实例。

主线程的 libevent 实例在主线程初始化时设置，工作线程则在 setup_thread() 中建立 libevent 实例。

Master 线程负责监听客户端的建立连接请求，并 accept 连接，Workers 线程负责处理已经建立好的连接的读写等事件。

![memcached threads](/images/linux/memcached-threads.png "memcached threads"){: .pull-center}

### Master Thread

首先看看主线程是如何通知 workers 线程处理新连接的，在初始化时会添加监听事件。

{% highlight c %}
static struct event_base *main_base;           // 主libevent

static int server_socket(...) {
    ... ...
    for (next= ai; next; next= next->ai_next) {
        ... ...
        if (!(listen_conn_add = conn_new(sfd, conn_listening,
                                         EV_READ | EV_PERSIST, 1,
                                         transport, main_base))) {
            fprintf(stderr, "failed to create listening connection\n");
            exit(EXIT_FAILURE);
        }
        ... ...
    }
}

{% endhighlight %}

主线程的 libevent 注册的是监听 socket 描述字的可读事件，就是说当有建立连接请求时，主线程会处理，新建监听端口是通过 conn_new() 初始化，其回调的函数是 event_handler() 。

对于主线程来说，会进入到 conn_listening 分支，也就是调用 dispatch_conn_new() 函数；在该函数中，会创建一个新的 CQ_ITEM，然后通过 round robin 策略选择了一个 thread，并通过 cq_push 将这个 CQ_ITEM 放入了该线程的 CQ 队列里。

最后通过 write() 向该线程管道写了 1 字节数据，则该线程的 libevent 立即回调 thread_libevent_process() 函数进行处理。

### Worker Thread

当 memcached 刚启动时，也就是当刚初始化完成之后，每个 workers 线程只有在自己线程管道的读端有数据时触发调用 thread_libevent_process() 方法，而主线程在有链接时会写入数据，该函数处理的就是主线程新建链接的请求。

{% highlight c %}
static void thread_libevent_process(int fd, short which, void *arg) {
    ... ...
    if (read(fd, buf, 1) != 1)
        if (settings.verbose > 0)
            fprintf(stderr, "Can't read from libevent pipe\n");

    switch (buf[0]) {
    case 'c':
    item = cq_pop(me->new_conn_queue);

    if (NULL != item) {
        conn *c = conn_new(item->sfd, item->init_state, item->event_flags,
                           item->read_buffer_size, item->transport, me->base);
        ... ...
    }
    break;

    /* we were told to pause and report in */
    case 'p':
    register_thread_initialized();
    break;
    }
}
{% endhighlight %}


入参的 fd 是这个线程的管道读端的描述符，在上述的函数中，首先将管道的 1 个字节通知信号读出。需要注意的是，在水平触发模式下如果不处理该事件，则会被循环通知，直到该事件被处理。

cq_pop() 会从该线程的 CQ 队列中取队列头的一个 CQ_ITEM，这个 CQ_ITEM 是被主线程丢到这个队列里的，item->sfd 是已建立的连接的描述符，然后会调用 conn_new() 函数。

{% highlight c %}
conn *conn_new(...) {
    ... ...
    event_set(&c->event, sfd, event_flags, event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->ev_flags = event_flags;

    if (event_add(&c->event, 0) == -1) {
        perror("event_add");
        return NULL;
    }
}
{% endhighlight %}

在 conn_new() 中，为 sfd 描述符注册 libevent 的读事件，接着会调用 event_add() 函数，也就是对该描述符的事件处理交给当前这个 workers 线程处理。

可以看到新的连接被注册了一个事件，实际上是 EV_READ\|EV_PERSIST 事件，当该连接有可读数据时会调用函数 event_handler()，实际上 event_handler() 里主要是调用 memcached 的核心方法 drive_machine() 。

而 Worker 线程最终会走到 conn_read 分支，可以参考如下从网上找的工作流程。

![memcached master workers](/images/linux/memcached-master-workers.jpg "memcached master workers"){: .pull-center}

也就是说，实际上，Master/Worker 线程的 libevent 实例都会调用 event_handler() 函数，而在该函数中的核心处理为其状态机的处理。



## 状态机

Worker 线程在处理 libevent 的事件时，会进入状态机处理不同的逻辑，详细的调用逻辑如下：

{% highlight text %}
event_handler()                           # libevent回调函数，此时有网络事件到达
  |-drive_machine()                       # 进入业务处理状态机
{% endhighlight %}

drive_machine() 就是通过当前连接的 state 来判断该进行何种处理，Master 和 Workers 的 libevent 事件处理都会调用该函数，其链接的状态是通过一个 enum 声明。

{% highlight c %}
enum conn_states {
    conn_listening,  // 主线程listen的主要状态，其工作就是把接到连接分发到worker子线程
    conn_new_cmd,    // 将链接设置为初始态，准备接收下个命令，会清空读写buffer
    conn_waiting,    // 等待读取socket，实际上就是挂起该链接，等待新的链接到来
    conn_read,       // 从客户端读取信息，也就是命令信息
    conn_parse_cmd,  // 尝试从读取的buffer中解析命令
    conn_write,      // 主要是out_string()函数，一般都是提示信息和返回的状态信息
    conn_nread,      // 读取固定大小的数据，会更新到item、hash、lru中，并调转到conn_write
    conn_swallow,    // 忽略不需要的数据
    conn_closing,    // 服务端主动调用close()关闭链接，并把conn结构体放到空闲队列
    conn_mwrite,     // 将这个链接中的msglist返回给客户端，可能会返回多个
    conn_closed,     // 链接已经被关闭
    conn_max_state   // 由于assert判断
};
{% endhighlight %}

首先着重说一下 conn_swallow 状态，对于像 update 操作，如果分配 item 失败，显然后面读取的数据是无效的，不过客户端是不知道的，客户端会继续发送特定的数量的数据，就需要把读到的这些数据忽略掉。

![memcached states](/images/linux/memcached-states.jpg "memcached sates"){: .pull-center width="600"}

接下来详细查看 drive_machine() 函数的处理流程，首先查看下主线程 conn_listening 状态的处理。

{% highlight c %}
static void drive_machine(conn *c) {
    ... ...
    while (!stop) {
        switch(c->state) {
        case conn_listening:
            addrlen = sizeof(addr);
            sfd = accept(c->sfd, (struct sockaddr *)&addr, &addrlen);
            if (sfd == -1) {
                ... ...         // 多种的错误处理
                break;
            }
            if (!use_accept4) { // 设置为非阻塞
                if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK) < 0) {
                    perror("setting O_NONBLOCK");
                    close(sfd);
                    break;
                }
            }

            if (settings.maxconns_fast &&
                stats.curr_conns + stats.reserved_fds >= settings.maxconns - 1) {
                ... .. // 超过了设置的最大链接数
            } else {   // OK，分发给Worker线程，初始化状态为conn_new_cmd
                dispatch_conn_new(sfd, conn_new_cmd, EV_READ | EV_PERSIST,
                                     DATA_BUFFER_SIZE, tcp_transport);
            }

            stop = true;
            break;
        }
    }
}
{% endhighlight %}


### conn_new_cmd

子线程最初进入的状态就是 conn_new_cmd 状态，这个状态主要是做一些清理。

{% highlight c %}
static void drive_machine(conn *c) {
    ... ...
    while (!stop) {
        switch(c->state) {
        ... ...
        case conn_new_cmd:
            --nreqs;     // 记录每个工作线程处理，记录每个libevent实例处理的事件，通过初始启动参数配置
            if (nreqs >= 0) {                      // 还可以处理请求
                reset_cmd_handler(c);              // 清空缓冲区
            } else {     // 拒绝请求
                pthread_mutex_lock(&c->thread->stats.mutex);
                c->thread->stats.conn_yields++;    // 更新统计数据
                pthread_mutex_unlock(&c->thread->stats.mutex);
                if (c->rbytes > 0) {
                    // 已经将数据读取到了buffer中，因此需要发送一个event
                    if (!update_event(c, EV_WRITE | EV_PERSIST)) {
                        if (settings.verbose > 0)
                            fprintf(stderr, "Couldn't update event\n");
                        conn_set_state(c, conn_closing);
                        break;
                    }
                }
                stop = true;
            }
            break;
        }
        ... ...
    }
}

static void reset_cmd_handler(conn *c) {
    c->cmd = -1;
    c->substate = bin_no_state;
    if(c->item != NULL) {
        item_remove(c->item);  // 如果有item，则直接删除
        c->item = NULL;
    }
    conn_shrink(c);            // 整理缓冲区，暂不分析
    if (c->rbytes > 0) {       // 如果缓冲区还有数据，则直接到conn_parse_cmd中继续处理
        conn_set_state(c, conn_parse_cmd);
    } else {   // 否则进入等待状态，状态机没有数据要处理，就进入这个状态
        conn_set_state(c, conn_waiting);
    }
}
{% endhighlight %}

根据是否有数据，分别会进入不同的状态，如果没有数据则会进入 conn_waiting 等待接收数据。


### conn_waiting、conn_read

接下来主要是处理数据报文的接收，包括了 conn_waiting、conn_read 两个状态。

{% highlight c %}
static void drive_machine(conn *c) {
    ... ...
    while (!stop) {
        switch(c->state) {
        ... ...
        case conn_waiting:
            // 更新libevent状态，也就是删除libevent事件后，重新注册libevent事件
            if (!update_event(c, EV_READ | EV_PERSIST)) {
                if (settings.verbose > 0)
                    fprintf(stderr, "Couldn't update event\n");
                conn_set_state(c, conn_closing);
                break;
            }

            conn_set_state(c, conn_read); // 进入读数据状态
            stop = true;
            break;

        case conn_read:
            // 判断采用UDP协议还是TCP协议
            res = IS_UDP(c->transport) ? try_read_udp(c) : try_read_network(c);

            switch (res) {
            case READ_NO_DATA_RECEIVED: // 未读取到数据
                conn_set_state(c, conn_waiting);    // 则继续等待
                break;
            case READ_DATA_RECEIVED:    // 已经读取到数据
                conn_set_state(c, conn_parse_cmd);  // 开始解析命令
                break;
            case READ_ERROR:            // 读取发生错误
                conn_set_state(c, conn_closing);    // 直接关闭链接
                break;
            case READ_MEMORY_ERROR:     // 申请内存发生错误，则继续尝试
                break;
            }
            break;
        ... ...
        }
    }
}
{% endhighlight %}

如果采用 TCP 协议则会调用 try_read_network() 从网络读取数据；对于 UDP 则比较简单，因为是数据报，读取到一个，就是一个完整的数据报，所以其处理过程简单。

### conn_parse_cmd

从网络读取了数据之后，则会进入该状态，按协议来解析读取到的网络数据。

{% highlight c %}
static void drive_machine(conn *c) {
    ... ...
    while (!stop) {
        switch(c->state) {
        ... ...
        case conn_parse_cmd :
            if (try_read_command(c) == 0) {     // 尝试解析命令
                // 如果读取到的数据不够，则继续等待
                conn_set_state(c, conn_waiting);
            }

            break;
        ... ...
        }
    }
}

static int try_read_command(conn *c) {
    ... ...
    if (c->protocol == binary_prot) {
        ... ...
        dispatch_bin_command(c);
    } else {
        ... ...
        process_command(c, c->rcurr);
    }
}
{% endhighlight %}

命令解析包括了文本协议和二进制协议，其命令的处理分别调用不同的命令处理函数。





# 内存管理

对于 memcached 内存高效管理是其最重要的任务之一，为了减小内存碎片的产生，采用 slab 管理其内存。简单来说，就是分配一块大内存，然后按照不同的块切分这些内存，存储业务数据时，按需选择合适的内存空间存储数据。

默认分配 64M 内存，之后所有的数据都是在这 64M 空间进行存储，在启动之后，不会释放这些内存，直到进程退出。

![memcached memory structure](/images/linux/memcached-02-01.png "memcached memory structure"){: .pull-center}

在 memcached 中，与内存相关的配置参数包括了设置内存大小、内存的增长因子，会在 slabs_init() 初始化的时候作为入参传入函数。

{% highlight c %}
static slabclass_t slabclass[MAX_NUMBER_OF_SLAB_CLASSES];   // 定义slab class最大值，64个
static size_t mem_limit     = 0;      // 总的内存大小
static size_t mem_malloced  = 0;      // 初始化内存的大小
static void  *mem_base      = NULL;   // 指向总的内存的首地址
static void  *mem_current   = NULL;   // 当前分配到的内存地址
static size_t mem_avail     = 0;      // 当前可用的内存大小

void slabs_init(const size_t limit, const double factor, const bool prealloc) {
    int i = POWER_SMALLEST - 1;
    //size表示申请空间的大小，其值由配置的chunk_size和单个item的大小来指定
    unsigned int size = sizeof(item) + settings.chunk_size;

    mem_limit = limit;   // 设置入参设置的内存上限

    if (prealloc) {      // 是否要预先分配设置的内存
        mem_base = malloc(mem_limit);  // 通过malloc()申请内存，并将内存基址指向mem_base
        if (mem_base != NULL) {
            mem_current = mem_base;    // mem_current指向当前地址
            mem_avail = mem_limit;     // 可用内存大小为mem_limit
        } else {         // 预分配失败
            fprintf(stderr, "Warning: Failed to allocate requested memory in"
                    " one large chunk.\nWill allocate in smaller chunks\n");
        }
    }

    memset(slabclass, 0, sizeof(slabclass)); // 置空slab class数组

    // 开始分配，i<200 && 单个chunk的size<单个item最大大小/内存增长因子
    while (++i < POWER_LARGEST && size <= settings.item_size_max / factor) {
        if (size % CHUNK_ALIGN_BYTES)   // 将size按照8byte对齐
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);

        slabclass[i].size = size;       // slab对应chunk的大小
        slabclass[i].perslab = settings.item_size_max / slabclass[i].size; // slab对应的chunk的个数
        size *= factor;                 // size下一个值为按增长因子的倍数增长
        if (settings.verbose > 1) {     // 如果有打开调试信息，则输出调试信息
            fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                    i, slabclass[i].size, slabclass[i].perslab);
        }
    }

    power_largest = i;   // size已经增长到1M，再增加一个slab
    slabclass[power_largest].size = settings.item_size_max;  // slab的size为item_size_max
    slabclass[power_largest].perslab = 1;                    // chunk个数为1
    if (settings.verbose > 1) {
        fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                i, slabclass[i].size, slabclass[i].perslab);
    }
    ... ...
    if (prealloc) {
        // 在每个slab class中都先预分配一个1M的内存
        slabs_preallocate(power_largest);
    }
}
{% endhighlight %}


## Hash表

在 memcached 中，其采用链接法来处理 Hash 冲突，如果冲突太高，则会导致链表过长，从而会导致查找时的耗时变长。

为次当表中元素的个数超过 Hash 容量的 1.5 倍时就进行扩容，扩容过程由独立的线程来完成，扩容过程中会采用 2 个 Hash 表，将老表中的数据通过 Hash 算法映射到新表中，每次移动的桶的数目可以配置，默认是每次移动老表中的 1 个桶。

在启动时，会通过 start_assoc_maintenance_thread() 函数启动一个线程，正常如果不需要扩容，则实际会调用 pthread 的 cond_wait() 函数一直等待。

首先看下如何在 hash 表中添加元素。


{% highlight c %}
// 在hash表中增加元素，不同于assoc_update，需要先确保没有相应的数据
int assoc_insert(item *it, const uint32_t hv) {
    unsigned int oldbucket;
    // 如果正在扩容且目前进行扩容还没到需要插入元素的桶，则将元素添加到旧桶中
    if (expanding && (oldbucket = (hv & hashmask(hashpower - 1))) >= expand_bucket)
    {
        it->h_next = old_hashtable[oldbucket];  // 添加元素
        old_hashtable[oldbucket] = it;
    } else {  // 如果没扩容，或者扩容已经到了新的桶中，则添加元素到新表中
        it->h_next = primary_hashtable[hv & hashmask(hashpower)];
        primary_hashtable[hv & hashmask(hashpower)] = it;
    }

    pthread_mutex_lock(&hash_items_counter_lock);
    hash_items++;
    // 还没开始扩容，但是表中元素个数已经超过Hash表容量的1.5倍，触发开始扩容
    if (! expanding && hash_items > (hashsize(hashpower) * 3) / 2) {
        assoc_start_expand();  // 唤醒扩容线程
    }
    pthread_mutex_unlock(&hash_items_counter_lock);

    MEMCACHED_ASSOC_INSERT(ITEM_key(it), it->nkey, hash_items);
    return 1;
}

static void assoc_start_expand(void) {   // 唤醒对应的扩容线程
    if (started_expanding)
        return;

    started_expanding = true;
    pthread_cond_signal(&maintenance_cond);
}
{% endhighlight %}









## CAS

CAS 也即 Compare And Set 或 Compare And Swap，实现的是一中无锁方法，或者说是乐观锁的一种技术，过程中会使用 CPU 提供的原子操作指令，可以提高系统的并发性能，在 Memcached 中用来保证数据的一致性，不是为了实现严格的锁。

当多个应用尝试修改同一个数据时，会出现相互覆盖的情况，此时使用 CAS 版本号验证，可以有效的保证数据的一致性。每次存储数据时，都会将生成的 CAS 值和 item 一起存储，后续的 get 会返回对应的 CAS 值，执行 set 等操作时，需要将 CAS 值传入。

其处理函数为 do_store_item() 。

{% highlight c %}
enum store_item_type do_store_item(item *it, int comm, conn *c, const uint32_t hv) {
    ... ...
    } else if (comm == NREAD_CAS) {
        /* validate cas operation */
        if(old_it == NULL) {
            // LRU expired
            stored = NOT_FOUND;
            pthread_mutex_lock(&c->thread->stats.mutex);
            c->thread->stats.cas_misses++;
            pthread_mutex_unlock(&c->thread->stats.mutex);
        } else if (ITEM_get_cas(it) == ITEM_get_cas(old_it)) {  // CAS值一致
            // cas validates
            // it and old_it may belong to different classes.
            // I'm updating the stats for the one that's getting pushed out
            pthread_mutex_lock(&c->thread->stats.mutex);
            c->thread->stats.slab_stats[ITEM_clsid(old_it)].cas_hits++;
            pthread_mutex_unlock(&c->thread->stats.mutex);

            item_replace(old_it, it, hv);   // 执行存储逻辑
            stored = STORED;
        } else {   // CAS值不一致，不进行实际的存储
            pthread_mutex_lock(&c->thread->stats.mutex);
            c->thread->stats.slab_stats[ITEM_clsid(old_it)].cas_badval++;
            pthread_mutex_unlock(&c->thread->stats.mutex);

            if(settings.verbose > 1) { // 打印错误日志
                fprintf(stderr, "CAS:  failure: expected %llu, got %llu\n",
                        (unsigned long long)ITEM_get_cas(old_it),
                        (unsigned long long)ITEM_get_cas(it));
            }
            stored = EXISTS;
        }
    }
}

uint64_t get_cas_id(void) // 为新的item生成cas值
{
    static uint64_t cas_id = 0;
    return ++cas_id;
}
{% endhighlight %}


# 参考

详细内容以及最新的版本可以查看官方网站 [www.memcached.org](https://memcached.org/) 。

<!--
http://blog.csdn.net/column/details/lc-memcached.html        Memcached源码分析
http://kenby.iteye.com/blog/1423989                          Memcached源码分析之内存管理篇
-->
