---
Date: October 19, 2013
title: MySQL 代码导读
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,database,数据库,源码,解析
description: 在 MySQL 的官网上，MySQL 号称是 The World's Most Popular Open Source Database ，既然是开源的，据说又这么牛掰，那不看看源码真有点对不起 MySQL 了。 不禁想起了 PostgreSQL 号称是 The World's Most Advanced Open Source Database ^_^'' 废话少说，本文简单介绍一下 MySQL 的执行流程。
---

在 MySQL 的官网上，MySQL 号称是 The World's Most Popular Open Source Database ，既然是开源的，据说又这么牛掰，那不看看源码真有点对不起 MySQL 了。

不禁想起了 PostgreSQL 号称是 The World's Most Advanced Open Source Database ^_^''

废话少说，本文简单介绍一下 MySQL 的执行流程。

<!-- more -->

![mysql skeleton]({{ site.url }}/images/databases/mysql/skeleton-logo.jpg "mysql skeleton"){: .pull-center width="50%" }

## 简介

MySQL 是基于线程的，在进程启动之后可以通过如下方式查看 MySQL 启动的进程信息。

{% highlight text %}
$ cat /proc/`pidof mysqld`/status | grep ^Threads           ← 查看线程数
$ cat /proc/`pidof mysqld`/sched                            ← 第一行即为线程数
$ ls /proc/`pidof mysqld`/task                              ← 查看对应线程信息

$ pstree                                                    ← 查看启动后进程之间的关系
$ pstree -p `pidof mysqld`                                  ← 查看进程对应的线程

$ ps -Lf `pidof mysqld`                                     ← 同样查看线程
$ ps -eo ruser,pid,ppid,lwp,psr,args -L | grep mysql        ← psr为线程运行的cpu-id

$ pstack `pidof mysqld` | less                              ← 打印对应进程的调用堆栈
{% endhighlight %}





## 源码导读

简单介绍下 MySQL 源码实现。

### 简介

在 MySQL 源码中，有很多类似 HAVE_XXX 的宏定义，如果是 RPM 安装包，也可以查看 mysql-xxx.devel 包中包含的 my_config.h 文件定义，也就是编译相关二进制时的宏定义。

实际上，在通过 CMake 编译源码时，会以 config.h.cmake 为模板扫描系统的一些配置，并生成 config.h 文件，然后 CMake 脚本会把 config.h 拷贝一份保存为 my_config.h 文件。

下面以 HAVE_SYS_EPOLL_H 宏定义为例，看下是如何实现的；在 configure.cmake 文件中，有如下的定义。

{% highlight text %}
CHECK_INCLUDE_FILES(sys/epoll.h HAVE_SYS_EPOLL_H)
{% endhighlight %}

上述文件中包含了 ```INCLUDE(CheckSymbolExists)```，而 CheckSymbolExists 是 CMake 的公共模块，一般在 cmake 的安装目录下，通常位于 /usr/share/cmake-X.X/Modules 目录下。

继续研究上述的文件，在源代码中有 ```MACRO(CHECK_SYMBOL_EXISTS SYMBOL FILES VARIABLE)``` 定义；这个宏的作用就是，查找相关文件 (FILES) 里面是否包含相关符号 (SYMBOL)；如果存在则设置变量为 1，MESSAGE 宏会在屏幕上做相关打印。

部分宏也可以在编译时，通过类似 ```-DEMBEDDED_LIBRARY``` 定义。


### ut_ad()宏定义

在代码中，会有 ```ut_ad(dict_index_is_clust(index));``` 类似的代码，下面看看这段代码的作用。

{% highlight cpp %}
#include "os0thread.h"
#define ut_a(EXPR) do {                     \
    if (UNIV_UNLIKELY(!(ulint) (EXPR))) {           \
        ut_dbg_assertion_failed(#EXPR,          \
                __FILE__, (ulint) __LINE__);    \
    }                           \
} while (0)

#define ut_error                        \
    ut_dbg_assertion_failed(0, __FILE__, (ulint) __LINE__)

#ifdef UNIV_DEBUG
#define ut_ad(EXPR) ut_a(EXPR)
#define ut_d(EXPR)  EXPR
#else
#define ut_ad(EXPR)
#define ut_d(EXPR)
#endif
{% endhighlight %}

也就是只有在 debug 模式下，会执行上述的代码。

<br>

我们从 MySQL 启动开始，看一下 MySQL 业务流程，详细的执行流程如下。

### 系统启动

首先是入口函数，也就是 C/C++ 的通用入口 main()，该函数在 sql/main.cc 文件中，而实际上其最终调用的是 mysqld_main()@sql/mysqld.cc，也就是 MySQL 的真正入口函数。其详细内容如下：

<!--
 | |-init_glob_errs()                   ← 设置初始化错误提示信息，现在是静态设置，因此为空
 | |-my_mutex_init()                    ← 互斥量的初始化，用来初始化互斥量的属性，修改线程互斥量的默认属性
 -->

{% highlight text %}
mysqld_main()
 |-my_init()                            ← 做一些基本的初始化工作
 | |-getenv()                           ← 设置umask，获取HOME等
 | |-my_thread_global_ init()           ← 初始化全局线程环境，包括私有数据、互斥量的初始化等
 | |-my_thread_init()                   ← 分配线程内存，主要用于mysys以及dbug
 |
 |-load_defaults()                      ← 加载默认的配置项
 |-handle_early_options()               ← 做些初始参数解析，例如PS的初始化
 | |-handle_options()                   ← 通用的解析命令行函数
 |-init_sql_statement_names()           ← 通过com_status_vars[]初始化，例如analyze等
 |-sys_var_init()                       ← 系统变量初始化
 | |-my_hash_init()                     ← 通过hash保存系统变量
 |-adjust_related_options()             ← 调整参数，如open_file_limit等
 |-initialize_performance_schema()      ← 如果需要则初始化PS
 |-init_server_psi_keys()               ← 如果需要则初始化PSI
 |
 |-init_error_log()
 |-mysql_audit_initialize()             ← 初始化audit全局接口，具体初始化稍后完成
 |
 |-init_common_variables()              ← 变量的初始化
 |-my_init_signals()
 |-init_server_components()             ← MySQL Server常用模块的初始化
 | |-mdl_init()
 | |-partitioning_init()
 | |-my_timer_initialize()
 | |-init_server_query_cache()
 | |-randominit()
 | |-setup_fpu()
 | |-init_slave_list()
 | |-open_error_log()
 | |-transaction_cache_init()
 | |-delegates_init()
 | |-process_key_caches()
 | |-ha_init_errors()
 | |-gtid_server_init()
 | |-plugin_init()
 | | |-plugin_load_list()
 | |   |-plugin_dl_add()                ← 包含了线程池类似插件的处理
 | |
 | |-ha_init()
 | |-initialize_storage_engine()
 | |-init_optimizer_cost_module()
 |
 |-init_ssl()
 |-network_init()                       ← 初始化网络模块，包括初始化调度器，创建socket监听端口
 |
 |-init_status_vars()
 |
 |-connection_event_loop()              ← 管理、创建新连接，会是一个死循环
 | |-listen_for_connection_event()
 | | |-poll()
 | |-process_new_connection()
 |   |-add_connection()
 |     |-mysql_thread_create()          ← 根据thread_handling参数选择具体方法
 |
 |-my_thread_join()
 |-clean_up()
 |-mysqld_exit()                        ← 程序退出
{% endhighlight %}

### 初始化网络配置

网络配置其实比较简单，就是设置端口，创建套接字，绑定端口，监听端口，实现全部集中在 network_init() 函数中，下面直接给出相应的伪代码：

{% highlight text %}
network_init()
 |-set_ports()                          ← 设置端口号，#define MYSQL_PORT 3306
 |-Mysqld_socket_listener()             ← 根据参数等，启动实例
 |-init_connection_acceptor()
   |-setup_listener()                   ← 不同类型listerner调用接口不同，如socket、pipe、share_memory
     |-tcp_socket()                     ← 创建tcp_socket实例
     |-get_listener_socket()            ← 创建监听socket，并准备接收连接
       |-create_lockfile()
       |-mysql_socket_socket()
       | |-inline_mysql_socket_socket()
       |   |-socket()                   ← 创建套接字
       |
       |-mysql_socket_bind()
       | |-inline_mysql_socket_bind()
       |   |-bind()                     ← 绑定端口号
       |
       |-mysql_socket_listen()
         |-inline_mysql_socket_listen()
           |-listen()                   ← 监听端口号
{% endhighlight %}

客户端与服务端通信的方式不止是 SOCKET 一种，MySQL 还支持三种连接方式：namepipe、unix socket 和 shared memory，即命名管道、unix 套接字和共享内存的方式，这三种方式是可以共存的，只是有些只支持本地，socket 是最通用的方式。


### 管理/创建新连接  !!!!

通过 connection_event_loop() 实现，而且 socket 管理其实比较简单，下面是其简单的处理代码：

{% highlight text %}
connection_event_loop()                 ← 对应死循环，不断判断abort_loop参数
 |-get_instance()                       ← 获取连接处理的实例
 |-listen_for_connection_event()
 | |-poll()/select()                    ← 监视socket文件描述符
 | |-mysql_socket_accept()              ← 处理到来的客户端连接
 | |-Channel_info_tcpip_socket()        ← 创建一个实例
 |
 |-process_new_connection()
   |-add_connection()                   ← 创建一个新的线程，不同方式会有不同处理方式
     |-mysql_thread_create()
       |-pthread_create()               ← 对应的处理函数是handle_connection()
{% endhighlight %}

主要处理函数，一系列异常保护之后会停止在 select()/poll() 函数处，等待接受到新的连接，如果监控到有连接，则通过 accept() 函数接受客户端的连接。


，然后新建一个 THD 类，将连接参数全部设置到 THD 类的参数上，最后调用 create_new_thread() 函数，这个函数便是重点。

mysql 为每个连接设置一个线程，而 oracle 同时也可以将请求放入一个队列当中。


接着是创建线程来处理客户端发送来的请求，通过 create_new_thread()@sql/mysqld.cc 实现，该函数执行的主要流程如下：

{% highlight c %}
static void create_new_thread(THD *thd) {
    ++*thd->scheduler->connection_count;       // 全局连接数自增
    thread_count++;                            // 全局线程数自增

    // 真正创建线程，实际调用的是 thd->scheduler.add_connection(thd);
    MYSQL_CALLBACK(thd->scheduler, add_connection, (thd));
}
{% endhighlight %}

在创建链接时，会对当前连接数检测 connection_count，先对互斥量 LOCK_connection_count 加锁，如果大于 max_connections+1，则报错，没有问题，才新建线程，一个典型的互斥线程。此时，全局连接数+1，全局线程数+1，然后调用 add_connection() 函数，现在线程创建成功了。

在 create_new_thread(thd) 的末尾，有一行代码，也就是如下的宏定义：

{% highlight c %}
MYSQL_CALLBACK(thd->scheduler, add_connection, (thd));   sql/sql_callback.h

#define MYSQL_CALLBACK(OBJ, FUNC, PARAMS)   \
    do {                                    \
        if ((OBJ) && ((OBJ)->FUNC))         \
            (OBJ)->FUNC PARAMS;             \
    } while (0)
{% endhighlight %}

这样，这个代码就是调用 thd->scheduler 的 add_connection 函数，参数是 (thd) 。这个函数就是我们在上面第一步设置连接的线程数中，one_thread_scheduler 和 one_thread_per_connection_scheduler 中设置的一个参数。这两者的区别便是是否创建了一个新的线程来处理到来的连接。

thd->scheduler 在 THD::THD() 构建函数中初始化，该值将继承全局的 thread_scheduler 。


## 链接处理

在此，根据不同的链接方式会调用不同的接口，现在 MariaDB 支持三种处理方式。one_thread_scheduler 是单线程方式，也就是不会去新建线程，而线程池实现方式有些复杂，以后再详细了解。

所以，在此，重点研究 one_thread_per_connection_scheduler 链接方式，也就是说设置的 add_connection 函数实际最终调用的是 create_thread_to_handle_connection()。

void create_thread_to_handle_connection(THD *thd)@sql/mysqld.cc，在该函数中，如果设置了线程缓存，且缓存中有空闲的线程，则直接从栈中取出一个线程即可。

{% highlight c %}
create_thread_to_handle_connection(THD *thd)
{
    if (cached_thread_count > wake_thread)
        thread_cache.push_back(thd);
    else
        thread_created++;
        threads.append(thd);               // 创建线程数自增，并加入到threads链表上
        mysql_thread_create(key_thread_one_connection,
                &thd->real_id,&connection_attrib,
                handle_one_connection,
                (void*)thd) ;              // 这就是真正创建线程的地方了
}
{% endhighlight %}

可见，最后调用了 mysql_thread_create() 函数，这是一个封装之后的函数，用于跨平台调用，对于 Linux，最后实际是通过 pthread_create() 创建了一个新的线程，而新线程的 处理函数为 handle_one_connection()。


### 新线程处理流程

新线程处理函数为 void *handle_connection(void *arg)，到此为止，一个新的 connection 被一个新创建的线程所单独处理，我们看下其中是如何进行处理的。

{% highlight c %}
// 连接处理函数，入参是连接对象Channel_info
void *handle_connection(void *arg)
{
    my_thread_init()                               // 初始化线程
    for (;;) {
        THD *thd= init_new_thd(channel_info);      // 新建一个线程对象
        thd_manager->add_thd(thd);                 // 添加到线程管理

        if (thd_prepare_connection(thd))           // 包括用户认证
          handler_manager->inc_aborted_connects();
        else
        {
          while (thd_connection_alive(thd))
          {
            if (do_command(thd))                   // 处理命令
              break;
          }
          end_connection(thd);
        }
        close_connection(thd, 0, false, false);

        thd->get_stmt_da()->reset_diagnostics_area();
        thd->release_resources();
    }
}
{% endhighlight %}

在新建完线程之后，会先调用 my_thread_init() 做线程的初始化，到目前为止，才算创建了一个新的线程，接着会有一些初始化的工作。

**注意，在此新建完线程后，后续的很多操作都会携带上该线程对象指针。**

接着会通过 thd_prepare_connection() 函数进行一些登陆认证等操作，通过 login_connection() 函数实现，还有一些其它的初始化工作。

接下来主要执行工作是在 do_command() 函数，也就是主要的命令处理函数。

### 命令分发

接下来是主要的命令处理函数 ```bool do_command(THD *thd)@sql/sql_parse.cc```，该函数主要用来接收、解析、执行命令报文；在线程中，该函数会不断循环执行。

{% highlight c %}
bool do_command(THD *thd)
{
  thd->m_server_idle= true;
  // 如下的命令会阻塞在网络读取，直到读取了最新的报文
  rc= thd->get_protocol()->get_command(&com_data, &command);
  thd->m_server_idle= false;

  // 接下来准备分发命令
  return_value= dispatch_command(thd, &com_data, command);
}
{% endhighlight %}

当客户端通过 TCP 连接上 MySQL 的服务器后，在发送请求之前，服务端的线程实际上是阻塞在 do_command() 函数中，也就是 socket 里的 read()。当接收到报文后，该函数同时还会作一些处理，如去除头部等。

<!--
当客户端键入 sql 语句，如 select * from test，发送到服务端之后，my_net_read() 返回，并从 tcpbuffer 中读取数据，将 packet 指针指向读取的字符串位置。现在开始基本上每个函数执行的时候都会带有一个 thd 参数，也就是线程描述符。

packet 的第一个字节是个命令的标志位，决定数据包是查询还是命令、成功、或者出错，然后做一些简单的处理（也就是预处理），接下来就进入 dispatch_command()@sql/sql_parse.cc 函数，也就是主要的命令处理函数，此时数据类型不再需要。
-->

**需要注意的是**，有的命令只需要在 dispatch_command() 执行，例如 COM_REGISTER_SLAVE；而部分则会在 mysql_execute_command() 中执行，例如 SQLCOM_CHANGE_MASTER 。

在 dispatch_command() 函数中，其主要的处理流程如下。

{% highlight c %}
bool dispatch_command(enum enum_server_command command, THD *thd, char* packet, uint packet_length)
{
  switch (command) {
    case COM_INIT_DB: ... ...;
    case COM_QUERY: {
      if (alloc_query(thd, com_data->com_query.query,
                      com_data->com_query.length))
        break;                    // fatal error is set

      Parser_state parser_state;
      if (parser_state.init(thd, thd->query().str, thd->query().length))
        break;
      // 开始进行SQL解析
      mysql_parse(thd, &parser_state);

      // 如果SQL中有通过分号分割的多条语句，同时会在下面处理，在此不赘述
    }
  }
}
{% endhighlight %}
<!---
首先通过 statistic_increment() 函数增加统计量，可以通过 show global status questions 查看。
-->

在该函数中，其主要作用的是一个巨大的 switch 语句，涵盖了 MySQL 支持的所有语句，包括了查询、PING、QUIT等指令，这些命令会在 include/my_command.h 中定义：

{% highlight c %}
enum enum_server_command
{
    COM_SLEEP, COM_QUIT, COM_INIT_DB, COM_QUERY, COM_FIELD_LIST,
    ... ...
    COM_END
};
{% endhighlight %}

接下来命令的处理，就是根据不同的请求通过 switch 进入不同的函数入口，对于查询命令最后进入的是 COM_QUERY，先做一些初始化、写日志等后进入 ```mysql_parse()@sql/sql_parse.cc```，该函数是 SQL 语句解析的总入口。

### 命令解析

SQL 的解析包括了：词法分析，语法分析，语义分析，构造执行树，生成执行计划，计划的执行。SQL92 是最新的标准，里面的定义都是一些巴科斯范式(BNF)，就是一种语法定义的标准。

MySQL 通过 YACC(Yet Another Compiler Compiler) 进行语法解析，不过没有采用 LEX 进行词法分析，YACC 接收来自词法分析阶段分解出来的 token 然后去匹配那些 BNF 。

另外，比较不错的嵌入式数据库 SQLite，词法分析器是手工写的，语法分析器由 Lemon 生成，如果感兴趣可以看下代码，在此就不详述了。

在 sql/sql_yacc.cc 源码中，有如下的定义；其中词法解析相关的主要处理函数在 sql/sql_lex.cc 文件中，其入口即 MYSQLlex() ，而主要的分词处理函数为 lex_one_token() 。

{% highlight c %}
#define yyparse         MYSQLparse
#define yylex           MYSQLlex
{% endhighlight %}

#### 词法解析

可以直接通过 state_map[] 获得对应的状态，该数组在 init_state_maps() 中初始化，首先会将字符设置为 MY_LEX_IDENT 、数字设置为 MY_LEX_NUMBER_IDENT、空白字符设置为 MY_LEX_SKIP、其它的设置为 MY_LEX_CHAR ，然后会将一些特殊字符初始化。

而关于字符的判断如下，其中 s 为对应的字符集，c 对应的序号，也就是通过 _MY_X 进行判断。

{% highlight c %}
#define my_isalpha(s, c)  (((s)->ctype+1)[(uchar) (c)] & (_MY_U | _MY_L))
{% endhighlight %}

每个字符集都会对应一个 ctype ，会通过该数组判断其类型。在 sql/lex.h 中定义了关键字，用两个数组存储 static SYMBOL symbols[] 和 static SYMBOL sql_functions[]。

#### SQL解析

仍回到如上的函数入口。

SQL 命令解析的入口是 mysql_parse(); sql/sql_parse.cc，如上所述 SQL 的语法/语义解析是通过 yacc 实现，规则文件是 sql/sql_yacc.yy 。

{% highlight c %}
void mysql_parse(THD *thd, char *rawbuf, uint length, Parser_state *parser_state)
{
    mysql_reset_thd_for_next_command(thd);              // 重置结构体
    lex_start(thd);                                     // 初始化词法分析结构体

    if (query_cache_send_result_to_client(...) <= 0) {  // 在cache中查询
        err= parse_sql(thd, parser_state, NULL);        // 不在cache中，直接查询
        error= mysql_execute_command(thd);              // 解析完后开始执行SQL
    } else {                                            // 命中cache，直接返回
        hd->lex->sql_command= SQLCOM_SELECT;            // 设置结果，更新统计
        ... ...
    }
}
{% endhighlight %}

在 mysql_parse() 中有段注释，大概的意思是：本来应该先调用 query_cache_send_result_to_client()，也即在 query_cache 中查询该语句，加快查询速度。失败才调用 lex_start() 和 mysql_reset_thd_for_next_command() 来初始化 thd 解析 sql。但是查询 cache 也需要干净的 thd，只能先调用 lex_start() 和 mysql_reset_thd_for_next_command() 来初始化 thd 了，这样导致代码和逻辑有悖。

首先是初始化以及重置操作，接着会在 cache 中查询，如果有相同的语句，则立即从 cache 返回结果，于是整个 sql 就结束了。

如果 cache 里不存在该 sql，则继续前进来到 parse_sql()@sql/sql_parse.cc，这个函数主要就是调用了 MYSQLparse()，而 MYSQLparse() 其实就是 bison/yacc 里的 yyparse。

下面就开始解析 sql 了，主要是关于词法分析和语法匹配，对于一条像 select * from test 的语句首先进入词法分析，此时会找到 2 个 token(select, from)，然后根据 token 进行语法匹配，规则在 sql/sql_yacc.yy 里。

最后的解析结果中，lex->sql_command 保存了相应的命令。

sql 解析完了，然后是一些优化操作等，接着进入 mysql_execute_command()@sql/sql_ parse.cc 函数，这个函数是所有 sql 命令执行的总入口。


### 命令执行

{% highlight c %}
int mysql_execute_command(THD *thd)
{
    switch (lex->sql_command) {
        case SQLCOM_SHOW_EVENTS: ...;
        case SQLCOM_SELECT: {
            check_table_access(...);
            res= execute_sqlcom_select(thd, all_tables);    // 执行查询
        }
    }
}
{% endhighlight %}

在 mysql_execute_command() 中，先确定 command 要对哪张表操作 lex->first_lists_tables_same(); 根据该表的状态，会做一些预处理，尽量减少之后的操作对表的影响（因为目前还不知道这条指令执行之后，会对数据库产生什么样的影响）做好保护是必须的。

然后有个 switch 语句，他决定了 command 属于哪种类型，这些类型定义在 sql/sql_ lex.h 中：

{% highlight c %}
enum enum_sql_command {
    SQLCOM_SELECT, SQLCOM_CREATE_TABLE, ...... SQLCOM_END
};
{% endhighlight %}

仍然以查询命令为例，最后会进入 SQLCOM_SELECT 这个 case 分支。之后就是命令的解析，处理，以及然后查询，规整结果集。

最后 select 的执行，通过 execute_sqlcom_select()@sql/sql_parse.cc 实现，在 execute_sqlcom_select() 函数中，调用 handle_select() (优化入口)，然后调用 mysql_select()。

mysql_select() 就是执行模块，这个模块代码比较复杂，可以清楚看到创建优化器 (JOIN::prepare)、优化 (JOIN::optimize)、执行 (JOIN::exec) 的3个步骤，在 MySQL 中，会将任何 select 都转换为 JOIN 来处理的。

MySQL 在设计时，采用了这样的思路：针对主要应用场景选择一个或几个性能优异的核心算法作为引擎，然后努力将一些非主要应用场景作为该算法的特例或变种植入到引擎当中。具体而言，MySQL 的 select 查询中，核心功能就是 JOIN 查询，因此在设计时，核心实现 JOIN 功能，对于其它功能，都通过转换为 JOIN 来实现。

即使对于最简单的 select name from student 也会转换为 JOIN 来操作。

{% highlight c %}
if (!(join= new JOIN(thd, fields, select_options, result)))
    ...
if ((err= join->optimize()))
    ...
join->exec();
{% endhighlight %}

结束了优化，我们要具体执行 join->exec()，该函数实际进入的是 JOIN::exec()@sql_select.cc。

exec()首先向客户端发送字段title的函数send_result_set_metadata()，没数据但字段也是要的。然后再进入 do_select() ，根据表的存储引擎跳入到引擎具体的实现。如果是 myisam，则通过 myisam 引擎扫描文件，其中 info->filename 实际保存的是文件的地址。

最后通过 join->result->send_data() 将数据发送给用户。并从 dispatch_command() 返回，最后在 net_end_statement 结束整个 sql 。


## 总结

处理 MySQL 客户端命令，在此以 one_thread_per_connection_scheduler 方式为例，也就是创建 handle_one_connection() 独立线程处理请求。

{% highlight text %}
handle_connections_sockets()
  |-poll()                           通过gdb查看，可以看到在此等待连接
  |-thd = new THD; my_net_init()
  |-create_new_thread()              根据不同的thread handler调用不同的函数
     |-create_thread_to_handle_connection()       one_thread_per_connection_scheduler方式
          |-handle_one_connection()                创建的新线程来处理
              |-do_handle_one_connection()
                  |-do_command()                   在死循环中处理
                      |-my_net_read_packet()
                      |-dispatch_command()         一堆的switch，根据客户端报文类型解析，include/mysql_com.h
++=== SQL Interface ==|+++|
                      |   |-mysql_change_db()      执行use db命令，COM_INIT_DB
                      |   |-sql_kill()             执行kill命令，COM_PROCESS_KILL
                      |   |- ... ...
                      |   |-mysql_parse()          执行SQL语句，COM_QUERY
                      |     |-lex_start()
                      |     |-mysql_reset_thd_for_next_command()
                      |     |-query_cache_send_result_to_client()
                      |     |-parse_sql()
                      |     |   |-MYSQLparse()   通过yacc解析SQL，规则文件保存在sql/sql_yacc.yy
                      |     |
                      |     | <font color="red">各种类型的SQL，一个大switch语句</font>
                      |     |-mysql_execute_command()         根据不同的SQL语句执行，sql/sql_cmd.h，对item调试
                      |         |-execute_show_status()       执行show status，SQLCOM_SHOW_STATUS
                      |         |- ... ...
                      |         |-check_table_access()        执行select，<font color='red'>SQLCOM_SELECT</font>
                      |         |-execute_sqlcom_select()
                      |         |   |-open_and_lock_tables()
                      |         |   |   |-open_tables()
                      |         |   |   |   |-open_and_process_table()
                      |         |   |   |       |-open_table()
                      |         |   |   |           |-Table_cache::get_table()
                      |         |   |   |           |-get_table_share_with_discover()
                      |         |   |   |           |   |-get_table_share()
                      |         |   |   |           |       |-open_table_def()
                      |         |   |   |           |           |-my_open()
                      |         |   |   |           |           |-open_binary_frm()
                      |         |   |   |           |               |-get_new_handler()  获取表的handler
                      |         |   |   |           |-my_malloc // 申请表数据结构
                      |         |   |   |           |-open_table_from_share
                      |         |   |   |               |-handler::ha_open
                      |         |   |   |                   |-ha_innobase::open
                      |         |   |   |                       |-dict_table_open_on_name
                      |         |   |   |                           |-dict_load_table
                      |         |   |   |                               |-btr_pcur_is_on_user_rec
                      |         |   |   |                               |-dict_load_table_low
                      |         |   |   |                               |   |-dict_mem_table_create
                      |         |   |   |                               |-fil_space_for_table_exists_in_mem
                      |         |   |   |                               |-fil_open_single_table_tablespace // 打开表空间文件
                      |         |   |   |-lock_tables()
                      |         |   |   |-mysql_handle_derived()
                      |         |   |-query_cache_store_query()       先查看缓存
                      |         |   |
                      |         |   |-handle_select()                 SQL处理的真正入口，会判断是否为union
                      |         |       |-mysql_union()               如果含有union，则调用该函数
                      |         |       |-mysql_select()              否则调用该函数
++=== Query Parser ===|++       |           |</font>
                      |         |           |-mysql_prepare_select()
                      |         |           | |-JOIN::prepare()@sql/sql_select.cc
                      |         |           | | |-setup_tables_and_check_access()
                      |         |           | | |-setup_wild()
                      |         |           | | |-setup_fields()
                      |         |           | | |-setup_without_group()
                      |         |           | | |-setup_order()                      order by语句相关
                      |         |           |     |-find_order_in_list()
                      |         |           |       |-find_item_in_list()
                      |         |           |
                      |         |           |-lock_tables()
                      |         |           |-query_cache_store_query()
                      |         |           |-mysql_execute_select()
                      |         |               |
++=== Query Prepare ==|+++      |               |<font color="blue">
                      |         |               |-JOIN::optimize()                     @sql/sql_optimizer.cc
                      |         |               |
                      |         |               |
                      |         |               |-JOIN::explain()                      @sql/sql_explain.cc
                      |         |               |   |                                  如果使用的是explain语句，返回而不执行
                      |         |               |   |-prepare_result()
                      |         |               |   |-explain_query_specification()
                      |         |               |
                      |                            Explain_query::send_explain()
++=== Query Optimizer |==+++    |               |</font>
                      |         |               |-JOIN::exec()            根据执行计划进行相应处理
                      |         |                   |-exec_inner()
                      |         |                       |-select_result::prepare()
                      |         |                       |-select_result::prepare2()
                      |         |                       |-select_send::send_result_set_metadata()
                      |         |                       |   |-Protocol::send_result_set_metadata()
                      |         |                       |
                      |         |                       |-do_select()                           查询入口函数
                      |         |                         |-join->first_select()            1. 实际调用sub_select()，也即循环调用
                                                          | |                                  rnd_next()+evaluate_join_record()
                      |         |                         | |
                      |         |                         | |                               while循环读取数据
                      |         |                         | |-join_tab->read_first_record() 首次调用，实际为init_read_record()
                      |         |                         | |   |-ha_rnd_init()
                      |         |                         | |   |   |-change_active_index()
                      |         |                         | |   |       |-innobase_get_index()
                      |         |                         | |   |-innobase_trx_init()
                      |         |                         | |-info->read_record()           再次调用，该函数在init中初始化
                      |         |                         | |
                      |         |                         | |-evaluate_join_record()        处理一条查询记录
                      |         |                         |     |-end_send()
                      |         |                         |         |-select_send::send_data()
                      |         |                         |             |-Protocol::write()
                      |         |                         |
                      |         |                         |-join->result->send_eof()
++=== Query Execution |==+++    |                         |
   st_select_lex::cleanup       |
                      |         |
                      |         |
                      |         |-update_precheck()
                      |         |-mysql_update()
                      |         |   |-open_normal_and_derived_tables()
                      |         |   |-mysql_prepare_update()
                      |         |   |-innobase_register_trx()
                      |         |   |-innobase_register_trx()
                      |         |
                      |         |
                      |         |
                      |         |
                      |         |
                      |         |
                      |         |
                      |         |
                      |
                      |-thd->protocol->end_statement()        将获得的查询结果发送到客户端
{% endhighlight %}

在查询记录时，会循环调用 ha_innobase::rnd_next() 和 evaluate_join_record() 获取并处理该部分的每条记录。


### 结论

整个 connection manager 的流程十分清晰，单线程的连接一般很少使用，大多使用多线程方式。多线程连接中其实还涉及到线程缓冲池的概念，即如果一个连接断开后，其所创建的线程不会被销毁掉，而是放到缓冲池中，等待下一个新的 connection 到来时，首先去线程缓冲池查找是否有空闲的线程，有的话直接使用，木有的话才去创建新的线程来管理这个 connection。


{% highlight text %}
{% endhighlight %}
