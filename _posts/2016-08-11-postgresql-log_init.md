---
Date: October 19, 2013
title: PG 日志
layout: post
comments: true
language: chinese
category: [sql]
---




<!-- more -->

{% highlight text %}
#----- 日志记录在那里
log_destination = 'stderr'                     # 日志记录到什么地方，stderr、csvlog、syslog等
logging_collector = on                         # 是否开启日志收集，此时会开启logger进程
log_directory = 'pg_log'                       # 指定日志保存的目录，相对路径为$PGPATH
log_filename = 'postgre-%Y%m%d_%H%M%S.log'     # 日志文件名格式，参数采用strftime()格式
log_file_mode = 0600                           # 日志文件的权限设置
log_truncate_on_rotation = off                 # 如果有相同的日志文件名，则会truncate而非append
log_rotation_age = 1d                          # 多长时间切换一次日志
log_rotation_size = 256MB                      # 输出多少日志后会切换日志文件，0表示不切换

#----- 什么时候记录日志
client_min_messages = notice                   # 设置返回给客户端的日志，建议保持默认即可
log_min_messages = warning                     # 保存到服务器日志的信息，只有superuser可以设置
log_min_error_statement = error                # 那些错误的SQL记录到日志
log_min_duration_statement = 1s                # SQL耗时多久会记录到日志

#----- 记录什么
debug_print_parse = off
debug_print_rewritten = off
debug_print_plan = off
debug_pretty_print = on                        # 如上配置解析SQL时的数据结构
log_checkpoints = on                           # 记录checkpoints日志，某些情况下会影响到性能
log_connections = on
log_disconnections = on                        # 每次客户端连接/断开都会打印日志
log_line_prefix = '[%m %p %u %d %h]'           # 日志的格式，详细的可以查看配置文件
log_lock_waits = on                            # 记录超过死锁时间的SQL，lock waits >= deadlock_timeout
log_statement = 'none'                         # 记录那些类型的SQL语句
log_timezone = 'PRC'
log_duration = off                             # 关闭每次打印耗时，只有超过上述min才打印

log_hostname = off                             # ????


#log_error_verbosity = default          # terse, default, or verbose messages
#log_replication_commands = off ????
#log_temp_files = -1                    # log temporary files equal or larger
                                        # than the specified size in kilobytes;
                                        # -1 disables, 0 logs all temp files


{% endhighlight %}


# 常用技巧


## 慢SQL查询

通过 log_min_duration_statement 参数进行设置，一般当超过 1s 后记录 SQL 。另外，log_duration 如果打开，则会记录所有 SQL 的耗时。

可以通过如下方式测试。

{% highlight text %}
postgres=# show log_min_duration_statement;
 log_min_duration_statement
----------------------------
 1s
(1 row)

postgres=# \timing
Timing is on.
postgres=# select now(),pg_sleep(1);
              now              | pg_sleep
-------------------------------+----------
 2016-08-10 15:10:04.713244+08 |
(1 row)

Time: 1006.196 ms
postgres=# select now(),pg_sleep(3);
              now              | pg_sleep
-------------------------------+----------
 2016-08-10 15:10:31.080744+08 |
(1 row)

Time: 3002.663 ms
postgres=# \timing
Timing is off.
{% endhighlight %}

日志信息类似如下。

{% highlight text %}
duration: 3002.663 ms statement: select now(),pg_sleep(3);
{% endhighlight %}


## 监控checkpoint

当数据库进行大量数据更新时，如果参数设置有误，则会导致频繁做 checkpoint 导致系统变慢。

{% highlight text %}
postgres=# show log_checkpoints;
 log_checkpoints
-----------------
 on
(1 row)

postgres=# checkpoint;
CHECKPOINT
{% endhighlight %}

此时会打印如下信息。

{% highlight text %}
checkpoint starting: immediate force wait
checkpoint complete: wrote 0 buffers (0.0%); 0 transaction log file(s) added ...
{% endhighlight %}

## 监控数据库死锁

当前使用的锁信息保存在 pg_locks 系统表中，如果要查看一天内有多少锁超时，可以开启日志。

{% highlight text %}
postgres=# show log_lock_waits;
 log_lock_waits
----------------
 on
(1 row)

postgres=# show deadlock_timeout;
 deadlock_timeout
------------------
 1s
(1 row)

postgres=# CREATE TABLE foobar (id INT);
CREATE TABLE
postgres=# INSERT INTO FOOBAR VALUES(1);
INSERT 0 1
postgres=# begin;
BEGIN
postgres=# delete from foobar;
DELETE 1

----- 开启另外一个会话
postgres=# begin;
BEGIN
postgres=# delete from foobar;
DELETE 1
{% endhighlight %}

打印信息如下。

{% highlight text %}
LOG: process 9431 still waiting for ShareLock on transaction 1910 after 1000.523 ms
DETAIL:  Process holding the lock: 3184. Wait queue: 9431.
CONTEXT:  while deleting tuple (0,1) in relation "foobar"
STATEMENT:  delete from foobar;
{% endhighlight %}

# 最佳实践

{% highlight text %}
----- 按照创建时间倒序排序
$ ls -hlrc
{% endhighlight %}

# 参考

[Error Reporting and Logging](https://www.postgresql.org/docs/current/static/runtime-config-logging.html)


{% highlight text %}
{% endhighlight %}
