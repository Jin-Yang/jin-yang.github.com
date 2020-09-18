---
Date: October 19, 2013
title: PG WAL (Write Ahead Log)
layout: post
comments: true
language: chinese
category: [sql]
---



<!-- more -->

# 文件命名

在 $PGDATA/pg_xlog 目录下，保存的是 WAL 文件，默认是 16M，如果要修改，需要在编译时添加如下配置项，详细的其它参数可以直接参考源码的 INSTALL 文件。

{% highlight text %}
----- 编译时配置XLOG文件的大小
$ ./configure --with-wal-segsize=64

----- 查看配置后的定义宏大小
$ grep -rne 'define.*XLOG_SEG_SIZE' src/include/pg_config.h
#define XLOG_SEG_SIZE (16 * 1024 * 1024)
{% endhighlight %}

注意，上述定义的 XLOG_SEG_SIZE 是 XLOG 日志文件的段大小，就是一个物理的日志文件的大小；而一个逻辑的 XLOG 日志文件大小是 4GB。


每个文件的命名格式类似 000000010000000000000001，这个是起始的值。每个 XLOG 日志的文件名，实际上分为了两大部分，而第二部分又被分成两部分：

* Time Line ID，用来标识数据库状态，每次恢复都会增加，主要为了将恢复后与之前的状态区分开。

* XLOG Sequence Number，表示物理日志文件的序列号。这部分又分为两部分，高 32bits 表示 xlog 的序号，低 32bits 表示此 xlog 包含的 segment 号。

在内存中，是通过 XLogRecPtr 类型表示内存偏移的，而保存到文件时，每个文件名按如上的方式定义，源码中各个类型的定义如下：

{% highlight text %}
0000 0001 : 0000 0000 : 0000 000E

----- 包括了32bits的TLI
$ grep -rne 'typedef.*TimeLineID;' include/access/xlogdefs.h
typedef uint32 TimeLineID;
----- 以及64bits的物理日志文件序列号
$ grep -rne 'typedef.*XLogSegNo;' include/access/xlogdefs.h
typedef uint64 XLogSegNo;

$ grep -rne 'define XLogSeg.*XLOG_SEG_SIZE' include/access/xlog_internal.h
#define XLogSegSize           ((uint32) XLOG_SEG_SIZE)
#define XLogSegmentsPerXLogId (UINT64CONST(0x100000000) / XLOG_SEG_SIZE)

----- 文件名的命名格式
$ grep -A 3 -rne 'define XLogFileName(' include/access/xlog_internal.h
#define XLogFileName(fname, tli, logSegNo)   \
     snprintf(fname, MAXFNAMELEN, "%08X%08X%08X", tli,               \
                      (uint32) ((logSegNo) / XLogSegmentsPerXLogId), \
                      (uint32) ((logSegNo) % XLogSegmentsPerXLogId))
{% endhighlight %}

如上，其中文件的范围为 000000010000000000000001 ~ 00000001FFFFFFFF000000FF，可以做如下测试，当达到最大值之后会回到 00 ，而该值是一个非法值，对于 PG 来说，实际上会直接崩溃掉。

{% highlight text %}
$ pg_ctl -D /var/lib/pgsql/9.5/data stop
$ pg_resetxlog -f -l 00000001FFFFFFFF000000FE /var/lib/pgsql/9.5/data
$ pg_ctl -D /var/lib/pgsql/9.5/data -l logfile start
$ ls -lrt $PGDATA/pg_xlog/
postgres=# checkpoint;
postgres=# select pg_switch_xlog();
{% endhighlight %}

有人计算过，如果每天产生 10T 的日志，可以使用 4K 多年，所以目前来说是没有问题的。



# XLOG 格式

PG XLOG 文件的存储格式如下：

{% highlight text %}
<PageHeaderData Fixed-size>
<XLogRecord>
<rmgr-specific data>

<BkpBlock>
<XLogRecData>里面包括<CheckPoint>等

<BkpBlock>
<XLogRecData>

<BkpBlock>
<XLogRecData>
{% endhighlight %}


## PageHeaderData

PageHeaderData 在代码中可能是 XLogLongPageHeaderData 或者 XLogPageHeaderData 两个结构体中的一个，通过 xlp_info 中是否未 XLP_LONG_HEADER 标示符确定。

{% highlight c %}
typedef struct XLogPageHeaderData
{
    uint16      xlp_magic;      /* magic value for correctness checks */
    uint16      xlp_info;       // 记录flags，详细信息查看下面内容
    TimeLineID  xlp_tli;        // 本页中第一条记录的TimeLineID
    XLogRecPtr  xlp_pageaddr;   /* XLOG address of this page */
    uint32      xlp_rem_len;    /* total len of remaining data for record */
} XLogPageHeaderData;
typedef struct XLogLongPageHeaderData
{
    XLogPageHeaderData std;     /* standard header fields */
    uint64      xlp_sysid;      /* system identifier from pg_control */
    uint32      xlp_seg_size;       // XLOG文件大小，在编译时确定，如上所述
    uint32      xlp_xlog_blcksz;    // XLOG文件中页的大小，默认为8K
} XLogLongPageHeaderData;
{% endhighlight %}

其中 Long 模式的头只在一个 XLOG 文件的第一页保存，多余的字段用来校验有效性。另外，xlp_info 保存了一些标示，例如是否跨页、是否为 Long 模式，其值定义如下：

{% highlight c %}
#define XLP_FIRST_IS_CONTRECORD    0x0001     /* 表示页头后面跟的是一个跨页结构的一部分 */
#define XLP_LONG_HEADER            0x0002     /* 标示是Long模式，只在文件的第一页       */
#define XLP_ALL_FLAGS              0x0003     /* 定义所有标志位，用于头的有效性检查     */
{% endhighlight %}

<!--
XLOG文件里的页面头结构XLogPageHeaderData的成员xlp_magic表示WAL版本号。成员xlp_tli表示本页第一个记录的时间线，成员xlp_pageaddr表示本页的XLOG位置。
-->



## XLogRecord

日志在PG 定义如下。

{% highlight text %}
typedef struct XLogRecord
{
    uint32      xl_tot_len;     // 整个记录的长度
    TransactionId xl_xid;       // 该记录事务ID
    XLogRecPtr  xl_prev;        // 前一个XLogRecord的指针
    uint8       xl_info;        // 标准位
    RmgrId      xl_rmid;        // 本记录的资源管理器ID，例如RM_XLOG_ID、RM_CLOG_ID等
    pg_crc32c   xl_crc;         // 该记录的CRC校验
} XLogRecord;
{% endhighlight %}


当页头记录是跨页记录的一部分时，在XLogLongPageHeaderData结构的后面跟着XLogContRecord结构。原则是XLogRecord记录头从不被分到多个页，如果页尾空间小于SizeOfXLogRecord，就弃之不用，直接使用下一个页面。如果在一个记录在一个页面没写完，在下一个页头的XLogLongPageHeaderData结构后面用XLogContRecord结构，此时XLogLongPageHeaderData结构的成员xlp_info的值是XLP_FIRST_IS_CONTRECORD。


XLOG资源管理器时，其成员xl_info值可以是：

#define XLOG_CHECKPOINT_SHUTDOWN       0x00
#define XLOG_CHECKPOINT_ONLINE         0x10
#define XLOG_NOOP                  0x20
#define XLOG_NEXTOID               0x30
#define XLOG_SWITCH                    0x40
#define XLOG_BACKUP_END                0x50
#define XLOG_PARAMETER_CHANGE          0x60
#define XLOG_RESTORE_POINT             0x70



XACT资源管理器时，其成员xl_info高4位储存如下信息：

#define XLOG_XACT_COMMIT        0x00
#define XLOG_XACT_PREPARE          0x10
#define XLOG_XACT_ABORT            0x20
#defineXLOG_XACT_COMMIT_PREPARED  0x30
#defineXLOG_XACT_ABORT_PREPARED   0x40
#defineXLOG_XACT_ASSIGNMENT       0x50


XLOG仅使用xl_info的低4位，高4为由资源管理器rmgr使用

#define XLR_INFO_MASK           0x0F

如果用XLOG记录备份任一磁盘块（数据文件块吧），用其成员xl_info标志位记录，支持每个XLOG记录3个磁盘块的备份，使用xl_info标志的低1、2、3位

#define XLR_BKP_BLOCK_1         XLR_SET_BKP_BLOCK(0) /* 0x08 */

#define XLR_BKP_BLOCK_2         XLR_SET_BKP_BLOCK(1) /* 0x04 */

#define XLR_BKP_BLOCK_3         XLR_SET_BKP_BLOCK(2) /* 0x02 */



如果已备份块能从XLOG的压缩版本中被安全删除，设置Xl_info的0位（这就是，备份它们仅是为了防止写部分页（partial-page-write）问题，并且不保证PITR恢复的一致性）。压缩算法将需要从这些块中解析出数据以创建一个相同的非全页XLOG记录。

#define XLR_BKP_REMOVABLE       0x01




3

<rmgr-specificdata>可能是事务状态定义等，例如结构xl_xact_commit、xl_xact_abort等



typedefstruct xl_xact_commit

{

    TimestampTzxact_time;      /* 提交时间 */

    uint32     xinfo;        /* 信息位 */

    int        nrels;        /* 关系文件数 */

    int        nsubxacts;    /* 子事务XID数 */

    int        nmsgs;        /* 共享失效信息数 */

    Oid        dbId;         /* 数据库ID */

    Oid        tsId;         /* 数据库表空间ID */

    /* 提交时要drop的关系文件节点数组*/

    RelFileNodexnodes[1];      /* 变长数组 */

    /* 后面跟已提交子事务ID数组 */

    /* 后面跟共享失效消息数组*/

} xl_xact_commit;



typedefstruct xl_xact_abort

{

    TimestampTzxact_time;      /* 退出时间 */

    int        nrels;        /* 关系文件节点数 */

    int        nsubxacts;    /* 子事务XID数 */

    /* 退出时要drop的关系文件节点数组 */

    RelFileNodexnodes[1];      /*变长数组*/

    /* 后面跟以退出的子事务XID数组 */

} xl_xact_abort;



4

<BkpBlock>表示BkpBlock结构，是跟在XLOG记录XLogRecord后面的备份块头信息。XLOG代码知道PG数据页面中间常常包含一个无用的“洞”（“hole”——未使用的空间），其只包含值是0的字节。如果这个洞的长度大于0（hole_length >0），实际的跟在结构BkpBlock后面的块数据量是BLCKSZ -hole_length个字节

typedefstruct BkpBlock

{

    RelFileNodenode;        /* 包含该块的关系文件 */

    ForkNumberfork;         /* 关系分支 */

    BlockNumberblock;          /* 块数 */

    uint16     hole_offset;  /* "hole"前字节数 */

    uint16     hole_length;  /* "hole"的字节数 */



    /* 实际的块数据跟在结构后面 */

} BkpBlock;



5

<XLogRecData>表示XLogRecData结构。要写入XLOG日志文件的资源管理器数据，由一或多个XLogRecData结构定义。

typedefstruct XLogRecData

{

    char      *data;         /* 资源管理器包含数据的开始 */

    uint32     len;          /* 资源管理器包含数据的长度 */

    Buffer     buffer;       /* 有相应数据的buffer，如果有的话 */

    bool       buffer_std;   /* buffer是否有标准pd_lower/pd_upper头 */

    struct XLogRecData *next;   /* 链里的下一个结构 */

} XLogRecData;

pd_lower页面开始位置与未分配空间开头的字节偏移，pd_upper与未分配空间结尾的字节偏移，LSN:最后修改这个页面的 xlog 记录最后一个字节后面第一个字节。



XLogRecData结构里记录各种数据库操作数据，其中典型的，数据是一个检查点（CheckPoint struct）。根据XLogRecord的成员xl_rmid是否等于RM_XLOG_ID以及的成员xl_info是否等于XLOG_CHECKPOINT_SHUTDOWN或XLOG_CHECKPOINT_ONLINE来判定数据是检查点。检查点的定义见下面。



typedefstruct CheckPoint

{

    XLogRecPtrredo;         /*开始创建一个检查点时下一个XLOG记录的位置*/

    TimeLineIDThisTimeLineID; /*当前时间线*/

    uint32     nextXidEpoch; /*下一个事务ID的高排序位 */

    TransactionIdnextXid;      /* 下一个空闲事务ID */

    Oid        nextOid;      /* 下一个空闲OID */

    MultiXactIdnextMulti;      /* 下一个空闲多事务ID */

    MultiXactOffsetnextMultiOffset;   /* next free MultiXact offset */

    TransactionIdoldestXid; /* cluster-wide minimum datfrozenxid */

    Oid        oldestXidDB;  /* database with minimum datfrozenxid*/

    pg_time_t  time;         /* 检查点时间戳 */



    /*

仍在运行的最早的事务ID（XID）。只有在从一个在线检查点初始化热备模式时才需要，以使在GUC参数wal_level是hot_standby时我们不用为在线检查点计算运行最早的XID。否则设置为常量InvalidTransactionId。

     */

    TransactionIdoldestActiveXid;

} CheckPoint;


------------


# 时间线

http://mysql.taobao.org/monthly/2015/07/03/




pg_xlogdump -b 00000001000000000000000E


{% highlight text %}
{% endhighlight %}
