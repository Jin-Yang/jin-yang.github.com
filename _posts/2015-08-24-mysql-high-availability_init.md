---
Date: October 19, 2013
title: MySQL 高可用
layout: post
comments: true
language: chinese
category: [mysql,database]
---

在此会介绍 MySQL 的高可用解决方案。


<!-- more -->

实际上高可用需要从软件和硬件进行保证，目前对于 MySQL 来说，常见的高可用解决方案可以参考如下内容：

* 硬件复用。基本都是两套系统，如多路电源、双网卡、RAID 卡等，从而避免单点故障 (Single Point Of Failure, SPOF)。

* 共享存储。RAID 可以避免单磁盘故障，但是如过整机故障仍有问题，此时可以使用一个备机，同时使用 SAN/NAS 搭建共享存储。这样单机故障之后，直接利用共享存储上的数据，在备机上启动进程即可。

* Distributed Replicated Block Device, DRBD。于共享存储相似，也是为了解决存储的故障，不过这是内核提供的一个文件复制模块。

* 主备复制。也就是本文讨论的内容。

* Network DataBase, NDB。可以支持多个示例写入，从而当一个服务器宕机之后，另外的机器仍可以提供服务。不过，这种方式只适合写多读少的场景。

* Galera。MySQL 的多主在发生网络分区后，可能会导致不一致产生，而 Galera 可以保证在副本中的多数派写入成功之后才提交，从而保证数据一致性。

* Percona XtraDB Cluster, PXC。这个是对 Galera 的优化，原理基本相似，还没有仔细研究。

接下来我们看看是如何通过复制来获取高可用的。

{% highlight text %}
=== Master Slave ===
+-----+      +-----+
|-----|      |     |
|--M--| =>=> |  S  |
|-----|      |     |
+-----+      +-----+

=== Master Multi-Slave ===
             +-----+
             |     |
          +=>|  S  |
          |  |     |
          |  +-----+
          |
+-----+   |  +-----+
|-----|   |  |     |
|--M--|=>=+=>|  S  |
|-----|   |  |     |
+-----+   |  +-----+
          |
          |  +-----+
          |  |     |
          +=>|  S  |
             |     |
             +-----+

=== Master Master ===
+-----+      +-----+
|-----|      |-----|
|--M--| <==> |--M--|
|-----|      |-----|
+-----+      +-----+

=== Master Slave Slave ===
                         +-----+
                         |     |
                      +=>|  S  |
                      |  |     |
                      |  +-----+
                      |
+-----+      +-----+  |  +-----+
|-----|      |     |  |  |     |
|--M--| =>=> |  S  | =+=>|  S  |
|-----|      |     |  |  |     |
+-----+      +-----+  |  +-----+

                      |  +-----+
                      |  |     |
                      +=>|  S  |
                         |     |
                         +-----+
{% endhighlight %}

# 参考

当前 MySQL 高可用解决方案的很不错介绍，可以参考 [MySQL high availability -- Top 7 solutions to improve MySQL uptime](https://bobcares.com/blog/mysql-high-availability-top-7-solutions/)，或者可以参考 [本地保存的版本](/reference/mysql/Top 7 solutions to improve MySQL uptime.mht) 。




MySQL的各种高可用方案，大多是基于以下几种基础来部署的：

基于主从复制;

基于Galera协议;

基于NDB引擎;

基于中间件/proxy;

基于共享存储;

基于主机高可用;

在这些可选项中，最常见的就是基于主从复制的方案，其次是基于Galera的方案，我们重点说说这两种方案。其余几种方案在生产上用的并不多，我们只简单说下。




# 基于主从复制

双节点主从 + keepalived/heartbeat

一般来说，中小型规模的时候，采用这种架构是最省事的。

两个节点可以采用简单的一主一从模式，或者双主模式，并且放置于同一个VLAN中，在master节点发生故障后，利用keepalived/heartbeat的高可用机制实现快速切换到slave节点。

在这个方案里，有几个需要注意的地方：

采用keepalived作为高可用方案时，两个节点最好都设置成BACKUP模式，避免因为意外情况下(比如脑裂)相互抢占导致往两个节点写入相同数据而引发冲突;

把两个节点的auto_increment_increment(自增起始值)和auto_increment_offset(自增步长)设成不同值。其目的是为了避免master节点意外宕机时，可能会有部分binlog未能及时复制到slave上被应用，从而会导致slave新写入数据的自增值和原先master上冲突了，因此一开始就使其错开;当然了，如果有合适的容错机制能解决主从自增ID冲突的话，也可以不这么做;

slave节点服务器配置不要太差，否则更容易导致复制延迟。作为热备节点的slave服务器，硬件配置不能低于master节点;

如果对延迟问题很敏感的话，可考虑使用MariaDB分支版本，或者直接上线MySQL 5.7最新版本，利用多线程复制的方式可以很大程度降低复制延迟;

对复制延迟特别敏感的另一个备选方案，是采用semi sync replication(就是所谓的半同步复制)或者后面会提到的PXC方案，基本上无延迟，不过事务并发性能会有不小程度的损失，需要综合评估再决定;

keepalived的检测机制需要适当完善，不能仅仅只是检查mysqld进程是否存活，或者MySQL服务端口是否可通，还应该进一步做数据写入或者运算的探测，判断响应时间，如果超过设定的阈值，就可以启动切换机制;

keepalived最终确定进行切换时，还需要判断slave的延迟程度。需要事先定好规则，以便决定在延迟情况下，采取直接切换或等待何种策略。直接切换可能因为复制延迟有些数据无法查询到而重复写入;

keepalived或heartbeat自身都无法解决脑裂的问题，因此在进行服务异常判断时，可以调整判断脚本，通过对第三方节点补充检测来决定是否进行切换，可降低脑裂问题产生的风险。

双节点主从+keepalived/heartbeat方案架构示意图见下：
MySQL双节点高可用架构

图解：MySQL双节点(单向/双向主从复制)，采用keepalived实现高可用架构。

多节点主从+MHA/MMM

多节点主从，可以采用一主多从，或者双主多从的模式。

这种模式下，可以采用MHA或MMM来管理整个集群，目前MHA应用的最多，优先推荐MHA，最新的MHA也已支持MySQL 5.6的GTID模式了，是个好消息。

MHA的优势很明显：

开源，用Perl开发，代码结构清晰，二次开发容易;

方案成熟，故障切换时，MHA会做到较严格的判断，尽量减少数据丢失，保证数据一致性;

提供一个通用框架，可根据自己的情况做自定义开发，尤其是判断和切换操作步骤;

支持binlog server，可提高binlog传送效率，进一步减少数据丢失风险。

不过MHA也有些限制：

需要在各个节点间打通ssh信任，这对某些公司安全制度来说是个挑战，因为如果某个节点被黑客攻破的话，其他节点也会跟着遭殃;

自带提供的脚本还需要进一步补充完善，当然了，一般的使用还是够用的。

多节点主从+etcd/zookeeper

在大规模节点环境下，采用keepalived或者MHA作为MySQL的高可用管理还是有些复杂或麻烦。

首先，这么多节点如果没有采用配置服务来管理，必然杂乱无章，线上切换时很容易误操作。

在较大规模环境下，建议采用etcd/zookeeper管理集群，可实现快速检测切换，以及便捷的节点管理。



## 搭建环境

下图演示了从一个主服务器（master）把数据同步到从服务器（slave）的过程。

![Simple Master Slave]({{ site.url }}/images/databases/mysql/ha_master_slave.jpg "Simple Master Slave"){: .pull-center}

这是一个简单的主-从复制的例子，而主-主互相复制只是把上面的例子反过来再做一遍，所以我们以这个例子介绍原理。

对于一个mysql服务器，一般有两个线程来负责复制和被复制，当开启复制之后。

1. 作为主服务器Master，会把自己的每一次改动都记录到二进制日志 Binarylog 中。

2. 从服务器会用 master 上的账号登陆到 master ，读取 master 的 Binarylog ，写入到自己的中继日志 Relaylog ，然后自己的 sql 线程会负责读取这个中继日志，并执行一遍。

其中服务器以 [MySQL 简介](/blog/mysql-introduce.html) 中介绍的多实例为例，在配置文件中需要设置 log-bin=mysql-bin ，server-id 两个参数，第一个表示启用二进制日志。

{% highlight text %}
----- 在主建立帐户并授权slave，一般来说用其它帐号也可以
mysql> grant replication slave on *.* to 'mysync'@'localhost' identified by '123456';

----- 查看主服务器的状态，记住现在的File和Position
mysql> show master status;

----- 启动slave
mysql> change master to master_host='localhost', master_user='mysync', master_port=3306,
    -> master_password='123456', master_log_file='mysql-bin.000001', master_log_pos=313;

----- 在导出数据时可以执行如下内容
mysql> flush tables with read lock;                         // 锁定数据库然后将数据导出
$ mysqldump --master-data -uroot -p some_db > some_db.sql   // 将数据导出
mysql> unlock tables;                                       // 解锁数据库
$ mysql -uroot -p some_db < some_db.sql                     // 重新导入
{% endhighlight %}

正常来说，只要开启了 bin-log ，show master status 都是有效的，show slave status 在备库中有效。

{% highlight text %}
{% endhighlight %}
