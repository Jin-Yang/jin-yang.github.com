---
Date: October 19, 2013
title: PG 配置项
layout: post
comments: true
language: chinese
category: [sql]
---


<!-- more -->







# 配置

在 $PGDATA 目录下有两个与配置相关的文件 postgresql.conf 以及 postgresql.auto.conf，后者为 ALTER SYSTEM 命令生成的文件，这里的参数生效会覆盖前者中的值。

其中部分参数可以 reload 生效，此时 postmaster 会收到 SIGHUP 信号，会触发读取这两个配置文件，并且会把该信号传播到其它运行中的服务进程。

{% highlight text %}
postgres=# SET enable_indexscan = off;  # 设置会话级
postgres=# ALTER ROLE foobar SET enable_indexscan TO 'off';
postgres=# ALTER DATABASE foobar SET enable_indexscan = off;



postgres=# SHOW enable_indexscan;
postgres=# SELECT current_setting('enable_indexscan');



----- 恢复到默认值
postgres=# SET enable_indexscan TO DEFAULT;
postgres=# ALTER ROLE foobar SET enable_indexscan TO DEFAULT;
postgres=# ALTER DATABASE foobar SET enable_indexscan TO DEFAULT;



$ pg_ctl reload
postgres=# SELECT pg_reload_conf()
{% endhighlight %}


PG 中的配置参数分为了四类：

1. internal，参数在编译时确定，如 block_size 。

2. system，修改配置文件，需要重启才能生效，如 max_connections 。

3. global，所有会话都会生效。

4. session，只有当前会话会生效。

其中，global 通过 ALTER SYSTEM 设置后会向主进程发送 sighup 信号，主进程在 reaper() 中将  sighup 再发给各个子进程，各个进程会将 got_SIGHUP 设置为 true ，并在住流程中进行判断。

{% highlight c %}
if (got_SIGHUP)
{
    got_SIGHUP = false;
    ProcessConfigFile(PGC_SIGHUP);
}
{% endhighlight %}


## 修改配置

9.5 之前修改参数需要登陆到服务器，修改配置文件 postgresql.conf，然后 restart 或者 reload 。在 9.5 增加了 ALTER SYSTEM 命令修改参数，在 pg_settings 中增加 pending_restart 字段，用于判断是否需要重启。

{% highlight text %}
ALTER SYSTEM SET archive_mode TO 'on';
ALTER SYSTEM RESET archive_mode;
ALTER SYSTEM RESET ALL;
{% endhighlight %}


## GUC, Grand Unified Configuration

GUC 是 PG 里对数据库变量进行设置，对数据库进行控制的机制，包括了配置文件中变量修改，或者通过 set 命令对参数进行设置。实际上其变量的种类，设置方法要更多样，可直接查看 guc.c 中的实现。

GUC 变量包括变量所属的功能组、变量类型、来源和作用上下文（context）。这里简单说一下GUC变量的作用上下文。



上下文共分internal,postmaster,sighup,backend,suset,userset六种：

internal无法被用户修改，只能被内部进程设置，show命令能够查看此类变量。此类变量通常在编译时设置与改变。

postmaster在postmaster启动时通过读取configure文件或命令行来设置。这类变量的改变在postgreSQL重启时生效。

sighup在postmaster启动或向postmaster或backend进程发sighup信号来读取configure文件时设置。

backend在新backend进程启动时读取configure文件生效。

suset指超级用户修改生效，不需要重新读取configure文件。

user指普通用户修改生效，在当前会话下有效，无需读取configure文件。



几个常用的命令：

{% highlight text %}
select * from pg_settings;
show variableName;
select pg_reload_conf();
pg_ctl -D PGDATA reload
kill -HUG processID
select name,source,settings from pg_settings where source != 'default' and source !='override' order by 2,1;
{% endhighlight %}

## 配置文件

通常 PG 的配置文件保存在 -D /var/lib/pgsql/data 指定目录下。

{% highlight text %}
PGDATA/pg_hba.conf:
    基于主机的访问控制文件，保存对客户端认证方式的设置信息。
PGDATA/pg_ident.conf:
    用户名映射文件，定义了OS和PG用户名之前的对应关系，会被pg_hba.conf使用。
PGDATA/postgresql.conf:
    主配置文件，除了与权限相关的配置外，其它可以设置的参数都保存在该文件中。
PGDATA/postmaster.opts:
    上次启动时的命令行参数。
PGDATA/PG_VERSION:
    版本信息。

PGDATA/pg_clog/:
    事务的元信息，告知PG那些事务已经完成那些还没有。
PGDATA/pg_xlog/:
    WAL日志，每个文件16M，如果归档失败会导致该目录非常大。
PGDATA/pg_log/:
    日志信息。
PGDATA/pg_multixact/:
    多重事务状态数据子目录。
PGDATA/pg_notify/:
    LISTEN/NOTIFY相关的状态数据。
PGDATA/pg_stat_tmp/:
    状态统计信息所保存文件的临时目录。
PGDATA/pg_xlog/:
    WAL日志文件，每个文件占用16M。
{% endhighlight %}

其中常见的配置保存在 postgresql.conf 文件中，当前的变量值可以通过 show VARS 命令查看。

{% highlight text %}
max_connections = 1000                     # 同时连接到PG的客户端数量
log_directory = 'pg_log'                # 数据库日志的位置



track_activities = on                   # 监控每个进程命令执行的情况
track_counts = on                       # 监控表以及索引的访问情况
track_io_timing = off                   # 监控block的读写次数
track_functions = none                  # none, pl, all 跟踪用户定义函数的执行过程

#track_activity_query_size = 1024       # (change requires restart)
#update_process_title = on
stats_temp_directory = 'pg_stat_tmp'    # 一般指向RAM，关闭后保存在pg_stat中


# - Statistics Monitoring -

#log_parser_stats = off
#log_planner_stats = off
#log_executor_stats = off
#log_statement_stats = off











debug_print_parse = off                     # 打印查询解析日志
debug_print_rewritten = off
debug_print_plan = off
debug_pretty_print = on


log_checkpoints = off
log_connections = off
log_disconnections = off
log_duration = off






{% endhighlight %}




{% highlight text %}
{% endhighlight %}
