---
title: MySQL 配置文件
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,database,配置文件,配置
description: 在 MySQL 中，配置项可以通过配置文件文件或者启动时通过命令行指定，而且对于各个配置项的含义比较复杂，接下来我们看看 MySQL 中配置项的内容。
---

在 MySQL 中，配置项可以通过配置文件文件或者启动时通过命令行指定，而且对于各个配置项的含义比较复杂，接下来我们看看 MySQL 中配置项的内容。

<!-- more -->

![mysql logo]({{ site.url }}/images/databases/mysql/config-logo.png "mysql logo"){: .pull-center width="300px"}

## 简介

读取配置文件的顺序通常为 /etc/my.cnf 、 basedir/my.cnf 、 datadir/my.cnf 、\-\-defaults-extra-file 、 ~/.my.cnf 、 \-\-defaults-file ，也就是用户目录下的配置文件最后读取。

在实际环境中，如果需要准确找到 MySQL 的配置文件位置，我们可以尝试如下的方法。查看执行 MySQL 进程的全部命令参数，找到 mysqld 的位置。

{% highlight text %}
$ cat /proc/$(pidof mysqld)/cmdline | tr '\0' '\n'
{% endhighlight %}

根据以上获取的 mysqld 的位置后，可以通过如下命令查询 mysqld 查找配置文件顺序，其中如下命令中的 mysqld 可以实用如上获得的绝对地址。

{% highlight text %}
$ mysqld --help --verbose 2>/dev/null | grep -A1 "Default options are read"
{% endhighlight %}

最后可知道其读取的顺序为 /etc/mysql/my.cnf /etc/my.cnf ~/.my.cnf 。

其中，有一个参数 \-\-defaults-extra-file 指定了配置文件，该文件通常会出现在全局配置文件之后，在用户配置文件之前。

也就为意味着，如果启动时使用了上述配置项，而且其它配置文件都存在，当有一个 "变量" 在其它配置文件中都出现了，那么 **后面的配置文件中的参数变量值会覆盖前面配置文件中的参数变量值**，就是说会使用 ~/.my.cnf 中设置的值。

<!--
注意，如果使用 mysqld_safe 守护进程启动 mysql 数据库时，使用了 \-\-defaults-file=&lt;配置文件的绝对路径&gt; 参数，这时只会使用这个参数指定的配置文件。
-->

另外，一般在安装路径的 share 路径下存放了推荐的配置文件：my-small.cnf 、 my-medium.cnf 、 my-large.cnf 、 my-huge.cnf；可以通过如下命令查找。

{% highlight text %}
# find / -name "my-medium\.cnf"
{% endhighlight %}

如果想查看 MySQL 的一些全局变量设置，在非登录并有权限情况下可以这样查看。

{% highlight text %}
$ mysqladmin variables -u root -p
{% endhighlight %}

这个操作也就相当于登录时使用命令 show global variables; 。

### 配置文件格式

在每个配置文件中，根据不同的配置项分为了不同的模块，例如客户端配置项 ([client])、服务端配置项 ([mysqld]) 等。

而注释通过 # 标示。

## 典型配置

{% highlight text %}
[client]                                                    #=== 客户端配置
user                                   = root               # 用户名、密码等设置
password                               = passwd
host                                   = 127.1              # 默认使用本地IP
socket                                 = /tmp/mysql.sock    # 指定UNIX Socket文件位置

[mysqld]                                                    #=== 服务端配置
########basic settings########
server-id                              = 11
port                                   = 3306
user                                   = mysql
bind_address                           = 10.166.224.32

plugin_dir                             = /opt/mysql/lib/plugin  # 指定plugins的路径


autocommit                             = 0
character_set_server                   = utf8mb4
skip_name_resolve                      = 1
max_connections                        = 800
max_connect_errors                     = 1000
datadir                                = /data/mysql_data
transaction_isolation                  = READ-COMMITTED
explicit_defaults_for_timestamp        = 1
join_buffer_size                       = 134217728
tmp_table_size                         = 67108864
tmpdir                                 = /tmp
max_allowed_packet                     = 16777216
sql_mode                               = "STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER"
interactive_timeout                    = 1800
wait_timeout                           = 1800
read_buffer_size                       = 16777216
read_rnd_buffer_size                   = 33554432
sort_buffer_size                       = 33554432
########log settings########
log_error                              = error.log          # 使用datadir相对路径，如果没有err后缀会自动添加
log_warnings                           = 1                  # 是否将警告信息记录进错误日志，0(禁用)，1(启用)
                                                            # >1(建新连接产生的"失败的连接"和"拒绝访问"的错误信息)
log_output                             = FILE               # 日志输出到文件，可作用于查询日志和慢查询日志
long_query_time                        = 2                  # 记录超过2秒的查询输出，压力较大时会输出较多日志
slow_query_log                         = ON                 # 是否记录超过long_query_time时间的慢查询
slow_query_log_file                    = slow.log           # 设定文件格式的慢查询日志的存储路径
log_queries_not_using_indexes          = ON                 # 记录没有使用索引的SQL
log_slow_admin_statements              = ON                 # 记录表管理语句
log_slow_slave_statements              = ON                 # 记录主备复制时，超时的SQL
expire_logs_days                       = 90                 # 超出此天数的二进制日志文件将被自动删除
log_throttle_queries_not_using_indexes = 10                 # 每分钟输出的未使用索引的SQL数量
min_examined_row_limit                 = 100                # 查询检查返回少于该参数指定行的SQL不被记录到慢查询日志

########replication settings########
log_bin                                = mysql-bin          # 开启binlog
log_bin_index                          = mysql-bin.index    # 设置二进制文件索引
binlog-row-image                       = minimal            # 传输时只记录后镜像，减小磁盘、网络、内存的使用
binlog_format                          = row                # 指定binlog的格式，包括了statement,row,mixed
sync_binlog                            = 1                  # 日志缓存刷新时机，有组提交之后性能损耗不会太大
max_binlog_size                        = 1073741824         # binlog文件的最大值，默认1GB
binlog_cache_size                      = 32768              # 为每个会话分配的binlog内存大小

relay_log                              = relay-log          # (备库)对于设置relay-log
relay_log_index                        = relay-log.index    # (备库)同时设置文件索引
slave_parallel_workers                 = 4                  # (备库)SQL线程的并发数，最大为1024个线程
report_host                            = 127.1              # (备库)汇报host，在主使用SHOW SLAVE HOSTS查看
report_port                            = 3308               # (备库)同上，会自动汇报
master_info_repository                 = TABLE              # (备库)保存主机相关信息master.info(FILE)
                                                            #       slave_master_info(TABLE)
relay_log_info_repository              = TABLE              # (备库)记录relaylog相关信息relay-log.info(FILE)
                                                            #       slave_relay_log_info(TABLE)
relay_log_recovery                     = ON                 # (备库)relaylog自动修复
relay_log_purge                        = ON                 # (备库)不保存relaylog，执行完后直接删除
sync-relay-log                         = 1                  # (备库)SQL线程执行事务时，同时保存位点
sync-master-info                       = 1000               # (备库)
log_slave_updates                      = OFF                # (备库)是否将接收到的记录到本地binlog，用于级联复制
slave-parallel-workers                 = 8                  # (备库)设置SQL线程的并发
slave-parallel-type                    = LOGICAL_CLOCK      # (备库)利用组提交的逻辑值做并发

gtid_mode                              = ON                 # 开启GTID模式
enforce_gtid_consistency               = 1                  # 设置GTID一致，需要与上一个参数同时开启
binlog_gtid_simple_recovery            = 1
slave_skip_errors                      = ddl_exist_errors   # 忽略可能导致的DDL异常
binlog-rows-query-log-events           = ON                 # ROW模式binlog添加SQL信息，方便排错
log-bin-trust-function-creators        = ON                 # 同时复制主库创建的函数



########innodb settings########
innodb_page_size                       = 8192
innodb_buffer_pool_size                = 6G
innodb_buffer_pool_instances           = 8
innodb_buffer_pool_load_at_startup     = 1
innodb_buffer_pool_dump_at_shutdown    = 1
innodb_lru_scan_depth                  = 2000
innodb_lock_wait_timeout               = 5
innodb_io_capacity                     = 4000
innodb_io_capacity_max                 = 8000
innodb_flush_method                    = O_DIRECT
innodb_file_format                     = Barracuda
innodb_file_format_max                 = Barracuda
innodb_log_group_home_dir              = /redolog/
innodb_undo_directory                  = /undolog/
innodb_undo_logs                       = 128
innodb_undo_tablespaces                = 3
innodb_flush_neighbors                 = 1
innodb_log_file_size                   = 4G
innodb_log_buffer_size                 = 16777216
innodb_purge_threads                   = 4
innodb_large_prefix                    = 1
innodb_thread_concurrency              = 64
innodb_print_all_deadlocks             = 1
innodb_strict_mode                     = 1
innodb_sort_buffer_size                = 67108864

########semi sync replication settings#
plugin_load = "rpl_semi_sync_master = semisync_master.so;rpl_semi_sync_slave = semisync_slave.so"
loose_rpl_semi_sync_master_enabled     = ON                 # 开启semisync master
loose_rpl_semi_sync_slave_enabled      = ON                 # 开启semisync slave
loose_rpl_semi_sync_master_timeout     = 5000               # 设置超时时间5s

[mysqld-5.7]
innodb_buffer_pool_dump_pct            = 40
innodb_page_cleaners                   = 4
innodb_undo_log_truncate               = 1
innodb_max_undo_log_size               = 2G
innodb_purge_rseg_truncate_frequency   = 128
binlog_gtid_simple_recovery            = 1
log_timestamps                         = system
transaction_write_set_extraction       = MURMUR32
show_compatibility_56                  = on


{% endhighlight %}


<!--
# 从服务器开启二进制日志;
log-bin = mysql-bin.log
# Slave更新操作是否记入二进制日志，如果开启则必须要开启log-bin，开启二进制日志可以做级联复制;
log-slave-updates = true
# 锁定从服务器为只读，对于super用户不生效（可用可不用）;
read-only = 1
# 告诉从服务器当服务器启动时不启动从服务器线程，使用START SLAVE语句在以后启动线程;
skip-slave-start = true
# 同步中继日志文件加强数据安全，削弱性能，默认会进行系统缓冲;
sync_relay_log = 1
# 同步master_info文件加强数据安全，削弱性能，默认会进行系统缓冲;
sync_master_info = 1
# 同步relay_log_info文件加强数据安全，削弱性能，默认会进行系统缓冲;
sync_relay_log_info = 1
-->


## 配置选项详解

为了方便登陆，可以在 [lient] 中根据需要使用如下配置文件指定登陆的默认值；当然，**线上生产环境一定不要设置** 。

### 常用配置

{% highlight text %}
bind-address=0.0.0.0  # 指定监听IP地址
{% endhighlight %}

### log-bin

启动 binlog 记录，log-bin=/data/mysql/mysql-bin，可以指定绝对路径，默认在 datadir 目录下。

#### report-*

report 相关的参数是设置在从库上的，包含四个参数 ```report-[host|port|user|password]```，可以在 my.cnf 中设置了 report-host 时，在从库执行 ```START SLAVE``` 的时候，会将 report-host 和 report-port(默认3306) 发给主库，主库记录在全局哈希结构变量 slave_list 中。

如果想要连 report-user 和 report-password 也显示出来，则需要主库配置 show-slave-auth-info 。


<!--
        配置最大连接数<br>
        1) 可以在配置文件中修改max_connections=100，需要重起服务器；2) 登陆后通过SQL语句设置。<pre>
show variables like '%max_connections%';
set global max_connections=1000;
show [full] processlist;     # 查看连接数
<pre>
    3) 启动时修改mysqld_safe中的-O max_connections=1500。
    或者修改源码。
-->


profiling=[ON|OFF]<br>
在 5.1 中添加，可以用来查看一调 query 消耗的 CPU、IO、IPC、SWAP、PAGE FAULTS、CONTEXT SWITCH 等，同时还能得到该语句执行过程中 MySQL 所调用的各个函数在源文件中的位置。
<pre style="font-size:0.8em; face:arial;">
mysql> set profiling=ON;
mysql> show profiles;                                         # 获取保存的所有查询的信息
mysql> select emp_no from salaries where salary = 90930;
mysql> show profile;                                          # 查看最近一条SQL的执行信息
mysql> show profile cpu, block io for query N;                # 查询单个SQL的性能指标
</pre>
详细语法信息可以查看 <a href="http://dev.mysql.com/doc/refman/en/show-profile.html">SHOW PROFILE Syntax</a> 。
</li></ul>
</p>

# 参考

关于 MySQL 配置选项的处理顺序可以参考 [Command-Line Options that Affect Option-File Handling](http://dev.mysql.com/doc/refman/5.7/en/option-file-options.html) 中的内容。



<!--
binlog_checksum = CRC32
slave_allow_batching = 1
master_verify_checksum = 1
slave_sql_verify_checksum = 1
binlog_rows_query_log_events = 1
这四个参数是启用binlog/relaylog的校验，防止日志出错
-->


{% highlight text %}
{% endhighlight %}
