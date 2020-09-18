---
Date: October 19, 2013
title: PG 备份
layout: post
comments: true
language: chinese
category: [sql]
---

<!-- more -->

# 简介

pg_rman 支持全量、增量、归档三种备份模式，支持压缩以及备份集的管理。源码可以参考 [github rman](https://github.com/ossc-db/pg_rman)，需要选择相应的版本。



# Step By Step

接下来，我们看看如何一步步进行测试。

## 0. 准备环境

{% highlight text %}
$ pg_ctl -D /var/lib/pgsql/data init
$ mkdir /tmp/archive
$ cat data/postgresql.conf
wal_level = archive
archive_mode = on
archive_command = 'cp %p /tmp/archive/%f'
$ pg_ctl -D /var/lib/pgsql/data -l logfile start

SELECT name, setting FROM pg_settings WHERE name IN ('wal_level', 'archive_mode', 'archive_command');
CREATE TABLE foobar(id INT);
INSERT INTO foobar VALUES(generate_series(1, 1000000));

$ mkdir /tmp/backup
{% endhighlight %}

## 1. 初始化

实际上就是需要一个目录，用于存放备份的文件以及一些元数据，如备份的配置文件、数据库的 systemid、时间线文件历史等等。该命令只需要两个参数：备份目录、数据库的 $PGDATA 。

{% highlight text %}
$ pg_rman -B /tmp/backup -D /var/lib/pgsql/data init
$ tree /tmp/backup/
|-- backup/
|   |-- pg_xlog/
|   `-- srvlog/
|-- pg_rman.ini            # 生成的配置文件
|-- system_identifier      # 数据库system-id
`-- timeline_history/

$ cat pg_rman.ini
ARCLOG_PATH='/tmp/archive'
SRVLOG_PATH='/var/lib/pgsql/data/pg_log'

$ cat system_identifier
SYSTEM_IDENTIFIER='6340953224459249572'
{% endhighlight %}

其中 system_identifier 用于区分备份的数据库是不是一个数据库，防止被冲。

## 2. 全量备份

{% highlight text %}
$ pg_rman backup --backup-path=/tmp/backup --pgdata=/var/lib/pgsql/data \
    --arclog-path=/tmp/archive --srvlog-path=/var/lib/pgsql/data/pg_log \
    --backup-mode=full --with-serverlog --compress-data --smooth-checkpoint \
    --keep-data-days=10 \
    --keep-arclog-files=15 --keep-arclog-days=10 \
    --keep-srvlog-files=10 --keep-srvlog-days=15 \
    --verbose --progress \
    -h 127.0.0.1 -U postgres -d postgres
{% endhighlight %}

pg_rman 在备份时会记录每个备份文件的 crc 校验码，保存在 file_database.txt 文件中，内容为：路径、文件类型、大小、CRC校验值、权限、时间。

此时，可以通过如下命令查看当前备份的状态，正常来说应该是 DONE，需要再执行一次 validate 。

{% highlight text %}
$ pg_rman show -B /tmp/backup
{% endhighlight %}

每个备份集都包含了一个备份状态文件 backup.ini，这个文件中包含了很重要的信息，如 LSN，后面 LSN 将用于比对增量备份时对比数据块的 LSN 是否发生了变化，是否需要备份。


## 3. 数据校验

每次备份完，必须要做一次校验，否则备份集不可用来恢复，也不会用来做增量的比较。

{% highlight text %}
$ pg_rman validate -B /tmp/backup
{% endhighlight %}

备份时，会记录每个备份文件的 CRC 校验，用来 validate 做校验，每个目录会对应着一个校验文件。

{% highlight text %}
$ ls /tmp/backup/20160826/105229/file_*.txt
file_arclog.txt  file_database.txt  file_srvlog.txt

$ cat /tmp/backup/20160826/105229/file_database.txt
... ...
global/2397 F 18446744073709551615 0 0600 2016-08-26 10:45:19
global/2676 F 18446744073709551615 0 0600 2016-08-26 10:45:18
global/2677 F 18446744073709551615 0 0600 2016-08-26 10:45:18
global/2694 F 18446744073709551615 0 0600 2016-08-26 10:45:18
global/2695 F 18446744073709551615 0 0600 2016-08-26 10:45:18
global/2697 F 18446744073709551615 0 0600 2016-08-26 10:45:18
global/2698 F 18446744073709551615 0 0600 2016-08-26 10:45:18
... ...
{% endhighlight %}

每行分别对应了路径、文件类型、大小、CRC 校验值、权限、时间等值。另外，每个备份目录下还包括了一个 backup.ini 文件，保存了这个备份的状态，含有一些重要的信息，如 LSN 等。


## 4. 增量备份

实际上与上述的全量备份相同，只需要修改 backup-mode 即可。

{% highlight text %}
----- 插入一部分数据
INSERT INTO foobar VALUES(generate_series(1, 1000000));

$ pg_rman backup --backup-path=/tmp/backup --pgdata=/var/lib/pgsql/data \
    --arclog-path=/tmp/archive --srvlog-path=/var/lib/pgsql/data/pg_log \
    --backup-mode=incremental --with-serverlog --compress-data --smooth-checkpoint \
    --keep-data-days=10 \
    --keep-arclog-files=15 --keep-arclog-days=10 \
    --keep-srvlog-files=10 --keep-srvlog-days=15 \
    --verbose --progress \
    -h 127.0.0.1 -U postgres -d postgres

$ pg_rman validate -B /tmp/backup

----- 查看当前的备份集
$ pg_rman show -B /tmp/backup
{% endhighlight %}


## 5. 按指定时间从catalog删除备份集

例如只需要我的备份集能恢复到 2016-08-26 19:59:00，在这个时间点以前，不需要用来恢复到这个时间点的备份全删掉。

{% highlight text %}
$ pg_rman delete "2016-09-22 23:12:14" -B /tmp/backup

----- 物理删除已从catalog删除的备份集
$ pg_rman purge -B /tmp/backup
{% endhighlight %}

## 6. 恢复

数据恢复时有两个必要的要素：1) 新的 $PGDATA；2) 备份目录。备份恢复的时候有几个参数，实际上与 PG 的恢复配置文件 recovery.conf 中的意思对齐。

在恢复时，可以选择原地恢复，或者使用新的 $PGDATA 目录，不过不管是哪种恢复方式，如果在本机恢复，可能会覆盖原有的数据文件，所以最好先将原数据目录重命名。




<!--

如果不指定时间线，则使用$PGDATA/global/pg_control，如果没有$PGDATA/global/pg_control，则使用最新的全量备份集的时间线。

--recovery-target-timeline TIMELINE
Specifies recovering into a particular timeline. If not specified, the current timeline from ($PGDATA/global/pg_control) is used.

如果不指定，则恢复到最新时间

--recovery-target-time TIMESTAMP
This parameter specifies the time stamp up to which recovery will proceed. If not specified, continue recovery to the latest time.

如果不指定，则恢复到最新xid

--recovery-target-xid XID
This parameter specifies the transaction ID up to which recovery will proceed. If not specified, continue recovery to the latest xid.

如果不指定，则默认使用true，即恢复到包含恢复目标XID的commit record为止，或者第一笔commit record ts>指定ts的 commit redo record为止；
如果是false则不apply恢复目标XID的commit record，或者不apply第一笔commit record ts>=指定ts的 commit redo record。

--recovery-target-inclusive
Specifies whether we stop just after the specified recovery target (true), or just before the recovery target (false). Default is true.

是否使用硬链接复制archive log，而不需要拷贝文件

The following parameter determines the behavior of restore.
--hard-copy
The archive WAL are copied to archive WAL storage area. If not specified, pg_rman makes symbolic link to archive WAL where are in the backup catalog directory.
-->


### 5.1 在本机恢复的例子

{% highlight text %}
----- 1. 停库
$ pg_ctl -D /var/lib/pgsql/data -l logfile stop

----- 2. 重命名原数据相关目录，PGDATA、PG_XLOG、表空间、归档目录
$ mv /var/lib/pgsql/{data,data.bak}
$ mv /tmp/{archive,archive.bak}

----- 3. 恢复数据，并修改权限
$ pg_rman restore -B /tmp/backup -D /var/lib/pgsql/data
$ chmod 700 /var/lib/pgsql/data

----- 4. 启动恢复目标数据库
$ pg_ctl start -D /var/lib/pgsql/data

----- 5. 查看恢复的状态
postgres=# select pg_is_in_recovery();
 pg_is_in_recovery
-------------------
 f
(1 row)
{% endhighlight %}

恢复时可以指定 $PGDATA，从而恢复到新目录，但是 arch_log、表空间、pg_xlog 目录无法指定新的位置，所以原地还原时，必须注意这些目录可能被覆盖，最好是重命名。




# pg_rman 源码浅析

可以直接从官方下载相应的源码包，然后可以简单通过如下的命令进行安装。

{% highlight text %}
# yum install pam-devel
$ make
{% endhighlight %}

实际上，pg_rman 的源码非常简单，开始会做基本的参数解析，对于不同的子命令会调用应用的 do_XXX() 函数，如 do_init()、do_show() 等等。

pg_rman 会将备份数据保存到 DATE/TIME 目录下，其中备份结果会保存到 backup.ini 文件中，实际上在内存中通过 struct pgBackup 结构体表示。


{% highlight c %}
typedef struct pgBackup
{
    /* Backup Level */
    BackupMode  backup_mode;
    bool        with_serverlog;
    bool        compress_data;
    bool        full_backup_on_error;

    /* Status - one of BACKUP_STATUS_xxx */
    BackupStatus    status;

    /* Timestamp, etc. */
    TimeLineID  tli;

    XLogRecPtr  start_lsn;      // pg_start_backup()调用返回值
    XLogRecPtr  stop_lsn;       // pg_stop_backup()调用返回值

    time_t      start_time;
    time_t      end_time;
    time_t      recovery_time;
    uint32      recovery_xid;

    /* Size (-1 means not-backup'ed) */
    int64       total_data_bytes;
    int64       read_data_bytes;
    int64       read_arclog_bytes;
    int64       read_srvlog_bytes;
    int64       write_bytes;

    /* data/wal block size for compatibility check */
    uint32      block_size;
    uint32      wal_block_size;

    /* if backup from standby or not */
    bool        is_from_standby;

} pgBackup;
{% endhighlight %}





## 增量备份

上次备份后，数据块的 LSN 是否发生变化，若从上次备份的 start_lsn 以来没有发生变化，则不备份。

{% highlight text %}
do_backup()
  |-fgets()                    # 获取备份目录下system_identifier的值
  |-read_control_file()        # 读取当前数据库的system_identifier值
  |=========================== backup start
  |
  |-do_backup_database()       # 备份数据库数据
  | |                          ===== INFO: copying database files等待时间有点长
  | |-pg_start_backup()
  | |
  | |-parray_new()             # 新建需要复制文件的数组
  | |-dir_list_file()          # 获取所有需要复制的目录列表
  | |-dir_print_mkdirs_sh()    # 非check模式，则创建mkdirs.sh脚本
  | |-parray_free()            # 清空目录列表
  | |
  | |-backup_files()           # 执行文件备份过程
  | | |-backup_data_file()     # 会校验lsn是否需要复制
  | |
  | |-pg_stop_backup()
  |   |-wait_for_archive()
  |                            ===== database backup completed
  |
  |-do_backup_arclog()         # 备份WAL日志
  |-do_backup_srvlog()         # 备份系统日志
  |
  |-pgBackupWriteIni()         # 备份完成将相应信息保存到backup.ini文件中
  |
  |-pgBackupDelete()           # 清除老的备份数据
{% endhighlight %}

在启动时，会读取备份目录下的 system_identifier 文件的内容，与数据库中 global/pg_control 文件保存的相应的标示做对比，该文件保存格式为 ControlFileData 。




StartupXLOG()
readRecoveryCommandFile()    # 读取恢复文件中的配置项，可以判断保存的变量





# 注意事项

* PG 会从配置文件中读取 log_directory、archive_command 的值，所以如果这两个配置项在其它文件中或者在 postgresql.auto.conf 中，则这两个值将不准确。

* 在编译 pg_rman 时，需要保证 BLCKSZ、XLOG_BLCKSZ 与服务器相同，因为需要做块的校验、读取 LSN 等操作，都与块大小有关，最好使用 pg_config 获取相同的配置，确保块大小一致。

* 通过 pg_start_backup() 启动时，采用的是 exclusive 模式，所以同一时间，只能跑一次上述的命令。


{% highlight text %}
{% endhighlight %}
