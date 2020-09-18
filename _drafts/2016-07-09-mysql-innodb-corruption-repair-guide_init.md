---
title: InnoDB 故障修复
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,innodb,crash recovery,崩溃恢复
description: 如果 InnoDB 没有正常关闭，会在服务器启动的时候执行崩溃恢复 (Crash Recovery)，这一流程比较复杂，涉及到了 redo log、undo log 甚至包括了 binlog 。在此简单介绍下 InnoDB 崩溃恢复的流程。
---


<!-- more -->

## 崩溃恢复

第一步操作
停止、备份、重启

1. 停止
停止MySQL服务，如果已经下线或者崩溃可以忽略，目的是要冻结数据和表文件的当前状态保证没有新数据写入，可以直接进行文件操作，不需要关心是否会导致数据不一致或者数据丢失等。
/etc/init.d/mysqld stop

2. 备份
备份整个数据目录，也可以只备份数据和日志文件。
mkdir /data/innodb.bak
cd /var/lib/mysql
dd if=ibdata1 of=ibdata1.bak conv=noerror
cp -p ./ibdata* /data/innodb.bak/
cp -p ./ib_log* /data/innodb.bak/
里面需要注意的是，对于ibdata1分别使用了dd和cp进行复制，这是由于两者复制方式不同，dd是直接复制原始文件，而cp则复制文件内容到一个新的文件；可能这个不是必须的，但至少不是一个坏习惯。

如果没有数据备份，也可以直接备份整个数据目录；当然，这一步骤会比较耗时，对于紧急情况不太实际，如果这不可行，至少数据文件和InnoDB数据库目录应该提供一些能回退的数据：
cp -Rp /var/lib/mysql{,.orig}

3. 备份InnoDB数据库文件
如上所述，如果没有备份完整的MySQL数据目录，最好还是备份下InnoDB对应的目录，如果不确认哪些目录下含有InnoDB表，可以直接使用如下命令。
DATADIR=/var/lib/mysql; find $DATADIR -type f -name *.ibd | awk -F/ '{print $(NF-1)}' | sort | uniq | xargs -I {} cp -Rp $DATADIR/{} /root/innodb.bak

4. 重启MySQL服务
如果启动不会导致崩溃的话，可以启动然后通过mysqldump备份数据。
/etc/init.d/mysqld start
mysqldump --single-transaction -AER > /root/dump_wtrans.sql
mysqldump -AER > /root/dump.sql
备份之后，一定要检查是否可用，有可能会由于部分情况导致数据没有备份下来。

5. 无法启动
如果无法启动或者启动后无法备份，除了希望尽快上线之外，启动服务的目的之一是尽可能的恢复数据，减小数据丢失。实际上，InnoDB为了满足ACID属性，保持数据一致性，如果遇到数据的任何问题，它几乎总是使MySQL崩溃以防止进一步损坏数据。

可以通过如下方式修改innodb_force_recovery参数：
mode=1; sed -i "/^\[mysqld\]/{N;s/$/\ninnodb_force_recovery=$mode/}" /etc/my.cnf
然后，一旦你准备把你的服务器返回到默认模式，你可以通过以下命令删除innodb_force_recovery行：
sed -i '/innodb_force_recovery/d' /etc/my.cnf

Mode 1当遇到损坏页时，不使 MySQL 崩溃
Mode 2不运行后台操作
Mode 3不会尝试回滚事务
Mode 4不计算统计数据或应用存储/缓冲的变化
Mode 5在启动过程中不查看撤消日志
Mode 6在启动时不从重做日志（ib_logfiles）前滚
  
第二步 判断问题所在

1. 检查日志
如果怀疑InnoDB表或数据库损坏，可能是因为数据页损坏、数据不存在、MySQL服务无法启动等，对于任何一种情况，你要首先查看的是MySQL错误日志。
tail -200 /var/lib/mysql/`hostname`.err
通常可以大致确认服务崩溃的原因，在此仅讨论三种比较常见的情况：A)页损坏；B)LSN；C)数据字典。

1.A 页损坏
InnoDB: Database page corruption on disk or a failed
InnoDB: file read of page 515891.
通常会包含了相关的一些信息，例如通过调用堆栈确认是在什么场景下触发的异常、崩溃时的环境参数，重点查看下是那个页损坏了，当然也有可能只是由于某些原因无法读取而已。

当然，崩溃后并不一定意味着真正有页损坏，事实上，在某些情况下，这可能只是操作系统的文件缓存被损坏；所以，建议在创建备份后，在无任何进一步操作之前，可以通过如下的工具检查是否有页损坏，或者直接重启你的计算机看下是否有异常。
#!/bin/bash
for i in $( ls /var/lib/mysql/*/*.ibd); do
    innochecksum $i
done
innochecksum工具会查看表空间文件中的页，计算每页的校验值；然后，将计算值与存储的校验相比，如果两个值不同，通常意味着页被损坏，则报错。
如果MySQL是在线且可访问的，你可以使用CHECK TABLE命令。

2.B LSN/时间异常
120901  9:43:55  InnoDB: Error: page 70944 log sequence number 8 1483471899
InnoDB: is in the future! Current system log sequence number 5 612394935.
InnoDB: Your database may be corrupt or you may have copied the InnoDB
InnoDB: tablespace but not the InnoDB log files.
在InnoDB上每次修改操作，都会生成一个redo记录，然后保存在ib_logfileN文件中，这些记录会按顺序写入文件，如果写满则重新开始；所有这些记录都有一个相关的LSN，通常也就是日志文件的偏移量。

？？？？？？
此外，当一个数据库被修改，在该数据库中的特定页面也得到一个相关LSN。两者之间，这些LSN被一起检查，确保操作以正确的顺序执行。LSN本身基本上是一个到日志文件的偏移，且存储在数据库页头中的LSN告诉InnoDB有多少日志需要被刷。
在过程中，无论是意外重启，内存问题，文件系统损坏，复制问题，手动更改为InnoDB的文件或其他，这些LSN不再“同步”。无论是否使你的服务器崩溃，这应该被当作合理损坏，通常你需要解决它。

2.C 数据字典错误
[ERROR] Table ./database/table has no primary key in InnoDB data dictionary, but has one in MySQL!
InnoDB: Error: table ‘database/table’
InnoDB: in InnoDB data dictionary has tablespace id 423,
InnoDB: but tablespace with that id or name does not exi st. Have
InnoDB: you deleted or moved .ibd files
?[ERROR] Cannot find or open table database/table from
the internal data dictionary of InnoDB though the .fr m file for the
table exists. Maybe you have deleted and recreated InnoDB data
files but have forgotten to delete the corresponding .frm file s
of InnoDB tables, or you have moved .frm files to another datab ase
?or, the table contains indexes that this version of the engine
doesn’t support.
InnoDB的数据字典存在于系统表空间(Space0)，保存在ibdata1文件中，储存了包括表、列、索引的元数据信息；与每个InnoDB表的*.frm文件有很多相同信息。如果由于某些原因导致ibdata1文件被损坏、修改、移动或或替换，就可能导致无法通过数据字典查看数据。

？？？？
如果你看过之前的错误描述，你应该知道在ibdata1中（或以其他方式命名）文件中的数据与在单个表空间/.ibd / .frm文件的数据之间有明显的关联。当该关联丢失或损坏，可能会发生不好的情况。所以这像这样的数据字典的错误出现，最常见的原因是有些文件被手动移动或修改。它通常归结为：“数据字典预计这一文件或表空间在这里，但它不在！”，或“.ibd / .frm文件预计此项目在数据字典中，但它不在！ “。再次记住，数据字典存储在ibdata文件中，在大多数环境中，就是MySQL数据目录中的ibdata1。
 

## 参考  

[MySQL InnoDB Corruption Repair Guide](https://forums.cpanel.net/threads/innodb-corruption-repair-guide.418722/) 或者 [本地文档](/reference/databases/mysql/InnoDB Corruption Repair Guide.mht) 。

[xxxx](http://www.askmaclean.com/archives/mysql-recover-innodb.html)









{% highlight text %}
{% endhighlight %}
