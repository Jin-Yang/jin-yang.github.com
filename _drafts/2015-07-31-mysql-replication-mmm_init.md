---
title: MySQL 高可用 MMM
layout: post
comments: true
language: chinese
category: [mysql,database]
keywords: mysql,高可用,mmm
description: MySQL-MMM (Master-Master Replication Manager for MySQL) 是 Google 的一个开源项目，用来监控 MySQL 双主+多读架构，并在失败时完成自动切换，不过该方案不适用对数据一致性要求很高的业务。其原理是将真实数据库节点的 IP 映射为虚拟 IP 地址，包括了一个专用于写的 IP，多个用于读的 IP 。接下来，看看详细的实现原理。
---

MySQL-MMM (Master-Master Replication Manager for MySQL) 是 Google 的一个开源项目，用来监控 MySQL 双主+多读架构，并在失败时完成自动切换，不过该方案不适用对数据一致性要求很高的业务。

原理是将真实节点的 IP 映射为虚拟 IP 地址，包括了一个专用于写的 IP，多个用于读的 IP 。

接下来，看看详细的实现原理。

<!-- more -->

## 简介

MMM 适用于如下的两种场景，一种是双主，另外一种是双主+多个只读备库。
 
![mmm]({{ site.url }}/images/databases/mysql/mmm-sample-setup-1.png "mmm"){: .pull-center }

![mmm]({{ site.url }}/images/databases/mysql/mmm-sample-setup-2.png "mmm"){: .pull-center }

简单来说，MMM 主要是为了保证系统的高可用，而非数据一致性，如下是其优缺点：

##### 优点：

1. 自动完成切换，包括了主主Failover切换；
2. 多个Slave读的负载均衡；
3. 自动的VIP切换，通过ARP广播发送，这也就意味着需要在同一个网段中，否则需要用到虚拟路由技术；
4. 支持抖动检测，防止服务器状态频繁切换；

##### 缺点：

1. 无法完全保证数据的一致性，备库落后时执行切换会导致数据丢失；
2. 无论何时可以保证只有一个可写(通过readonly设置)，但切换时原链接没有直接断开，可能会导致不一致性；
3. monitor单点问题；

MySQL-MMM中 有三种比较核心的概念：节点状态(States)、角色(Roles)、模式(Modes)，如下简单介绍下：

### 相关脚本

MySQL-MMM的主要功能通过以下三个脚本来实现：
mmm_mond：监控进程，监控所有服务器，决定节点角色。
mmm_agentd：代理进程，运行在每台MySQL服务器上，为监控节点提供远程执行；TODODO: 是否支持双向通讯。
mmm_control：为mmm_mond提供管理命令，常见的有状态检查，控制节点上下线；


### 节点状态

也就是MySQL服务相关的状态，实际上可以通过如下状态判断是否需要发送报警，总共有六种，如下：
1. ONLINE 正常运行状态，只有该状态才会有角色信息，切换之后角色将移除；
2. ADMIN_OFFLINE 人工设置为离线状态，一般为手动运维操作，如更换内存；
3. HARD_OFFLINE 离线状态，一般为ping主机失败或者mysql连接失败；
4. AWAITING_RECOVERY 等待恢复，需要手动处理，一般是由于抖动(Flapping)或者没有设置自动恢复(TODODO:配置项是那个？？？)
5. REPLICATION_DELAY 复制延时过大，也就是rep_backlog检测失败，实际通过SHOW SLAVE STATUS检测Seconds_Behind_Master状态值；
6. REPLICATION_FAIL 复制线程没有运行，也就是rep_threads检测失败，实际通过SHOW SLAVE STATUS检测Slave_IO_Running或者Slave_SQL_Running是否为No状态。

其中状态转换在Monitor的主处理逻辑的_check_host_states()函数中检测。

FIXME: 状态切换比较复杂，远超文档中的介绍，后面再具体梳理吧
当主机处在REPLICATION_DELAY或REPLICATION_FAIL状态，一旦恢复，将切换到ONLINE状态。除非抖动不稳定。
主机在HARD_OFFLINE状态，如果所有的问题都解决了，那么将会切换到AWAITING_RECOVERY状态。如果它故障时间小于60s，并且它没有重启或者auto_set_online>0,
那么将会被自动切换到ONLINE状态，除非抖动不稳定。
活动的主服务器复制延时或复制失败将不被视为一个问题。因此活动的主服务器状态不会被置于REPLICATION_DELAY或REPLICATION_FAIL。
在节点被切换到ONLINE状态的60s内，如果复制延时或复制失败将会被忽略(默认时间为master-connect-retry值)。
如果rep_backlog和rep_threads都检测失败，将会切换到REPLICATION_FAIL状态。
A host that was in state REPLICATION_DELAY or REPLICATION_FAIL will be switched back to ONLINE if everything is OK again, unless it is flapping (see Flapping).
A host that was in state HARD_OFFLINE will be switched to AWAITING_RECOVERY if everything is OK again. If its downtime was shorter than 60 seconds and it wasn't rebooted or auto_set_online is > 0 it will be switched back to ONLINE automatically, unless it is flapping (see Flapping again).
Replication backlog or failure on the active master isn't considered to be a problem, so the active master will never be in state REPLICATION_DELAY or REPLICATION_FAIL.
Replication backlog or failure will be ignored on hosts whos peers got ONLINE less than 60 seconds ago (That's the default value of master-connect-retry).
If both checks rep_backlog and rep_threads fail, the state will change to REPLICATION_FAIL.

### 检测方式

mmm_mond对每个主机检测4项决定是否OK
1. ping 主机是否存活
2. mysql mysqld 进程是否存活
3. rep_threads 复制线程是否运行，通过SHOW SLAVE STATUS检测Slave_IO_Running或者Slave_SQL_Running是否为No状态。
4. rep_backlog 延时少、复制积压的日志很少，通过SHOW SLAVE STATUS检测Seconds_Behind_Master状态值。


3、角色

exclusive角色：互斥角色只有一个ip，并且同一时间只能分配给一个主机。可以指定一个首选主机（preferred 存疑？？），如果这个主机是ONLINE状态

，那么角色将被赋予到这个主机。注意：不能移动被分配到首选主机的角色，因为他们将立刻再次被移动回到它。

balanced角色：负载均衡角色可以有多个ip。没有一个主机可以比其他主机多出两个角色。
TODODO: 难点，既保证可以快速回复，有需要尽量避免Flapping。

## 抖动检测

mmm_mond支持抖动检测，通常是在网络不稳定时，例如光纤老化；抖动检测主要是针对主机频繁在ONLINE和(HARD_OFFLINE|REPLICATION_FAIL|REPLICATION_DELAY)状态之间切换，


每次切换到ONLINE状态(auto_set_online或者

down的时间小于60s)，将会导致角色的切换非常频繁。


为了避免这种情况mmm_mond内建了flap检测，可以通过配置文件配置。


如果一个主机在flap_duration时间内宕掉了flap_count次，则认为该主机处于flap状态，就不会自动被设置为ONLINE状态，此时主机将一直处于AWAITING_RECOVERY状态，直到手动设置为online (mmm_control set online host)。



如果auto_set_online>0,处于flapping的主机在flap_duration时间后将自动设置为ONLINE状态

TODODO: 业务高峰期，可能会导致频繁的复制延迟???加权限

???5、模式

active mode：Monitor将会自动的把角色从失败的主机上移除，并切换到其他主机上。

manual mode：Moniter会自动把负载均衡的角色分配给对应主机，但是不会自动的把角色从失败的主机上移除。可以通过move_role来手工移除。

wait mode：类似manual模式，但是当两个master都是online状态或者超过了wait_for_other_master的时间，将被切换为ACTIVE模式。

passive mode：在此模式下，monitor不会改变角色，不更新状态文件和发送任何信息给mmm agents。在此模式下可以使用set_ip来改变roles，但是这些改变在monitor切换到

ACTIVE或者MANUAL模式(set_active or set_manual)前都不会生效。在启动时检测到角色发生冲突将会进入被动模式。

## 参考

关于 MMM 的文档参考 [mysql-mmm.org](http://mysql-mmm.org/mysql-mmm.html) 中的介绍。

http://www.cnblogs.com/chenmh/p/5563778.html

What's wrong with MMM

https://www.xaprb.com/blog/2011/05/04/whats-wrong-with-mmm/

mmm_tools配置手册

http://linuxguest.blog.51cto.com/195664/608338/

MMM部署常见问题

http://www.codexiu.cn/linux/blog/18484/

ifconfig lo:0 127.0.1.1 netmask 255.255.255.0 up
ifconfig lo:0 down

ifconfig lo:1  127.0.1.1  netmask 255.255.255.0 up
ifconfig lo:1  down


用户权限设置，包括了mmm_agent(本地客户端操作，可以设置为本地登陆)、mmm_monitor(远程监控操作，可以只指定监控IP):

GRANT REPLICATION CLIENT ON *.* TO 'mmm_monitor'@'127.0.1.%' IDENTIFIED BY 'monitor';
GRANT SUPER, REPLICATION CLIENT, PROCESS ON *.* TO 'mmm_agent'@'127.0.1.%' IDENTIFIED BY 'agent';
GRANT REPLICATION SLAVE ON *.* to 'mysync'@'127.0.1.%' IDENTIFIED BY 'kidding';

MMM::Monitor::Monitor::init()  初始化
  |-main() 主要的循环处理过程
    |-MMM::Monitor::NetworkChecker::main()  启动一个线程监控ping_ips指定的IP列表
    | ###WHILE###BEGIN
 |-_process_check_results()
    |-_check_host_states()
    |-_process_commands()
    |-_distribute_roles()
    |-send_status_to_agents()
    | ###WHILE###END


mmm_agentd启动过程
MMM::Agent::Agent::main()
 |-create_listener()
 | ###WHILE1###BEGIN
 |-accept()
 | ###WHILE2###BEGIN
 |-handler_command()
 | |-cmd_ping() PING命令，直接返回OK: Pinged!
 | |-cmd_set_status() SET_STATUS命令，设置状态
 | | |-MMM::Agent::Helper::set_active_master() 如果是备库，则设置主库，该函数中会执行CHANGE MASTER TO
 | |-cmd_get_agent_status()
 | |-cmd_get_system_status()
 | |-cmd_clear_bad_roles()
 |
 | ###WHILE2###END
 | ###WHILE1###END
bin/agent/configure_ip 检查是否设置IP，如果没有设置，则设置并通过ARP通知其它服务器
configure_ip
MMM::Agent::Helpers::Network::send_arp()


HA工具需求
错误配置时(从其他机器复制配置文件过来)不会对现有环境造成影响；



#### 配置文件

DB服务器配置文件mmm_agent.conf、mmm_common.conf；监控服务器的配置文件mmm_mon.conf、mmm_common.conf；其中所有节点的mmm_common.conf文件都是相同的。

#
active_master_role      writer     ###积极的master角色的标示，所有的db服务器都需要开启read_only参数，对于writer服务器监控代理会自动将read_only属性关闭。

<host default>
    cluster_interface       eth0      #####群集的网络接口
    pid_path                /var/run/mysql-mmm/mmm_agentd.pid    ####pid路径
    bin_path                /usr/libexec/mysql-mmm/              #####可执行文件路径
    replication_user        repl           #######复制用户
    replication_password    repl           #######复制用户密码
    agent_user              mmm_agent      #######代理用户，用于更改只读操作
    agent_password          mmm_agent      #######代理用户密码
</host>

<host master>            ##########master1的host名
    ip      192.168.137.10   #####master1的ip
    mode    master       ########角色属性，master代表是主
    peer    backup       ########与master1对等的服务器的host名，也就是master2的服务器host名
</host>

<host backup>     ####和master的概念一样
    ip      192.168.137.20
    mode    master
    peer    master
</host>

<host slave>      #####从库的host名,如果存在多个从库可以重复一样的配置
    ip      192.168.137.30   ####从的ip
    mode    slave    #####slave的角色属性代表当前host是从
</host>

<role writer>   ####writer角色配置
    hosts   master,backup   ####能进行写操作的服务器的host名，如果不想切换写操作这里可以只配置master,这样也可以避免因为网络延时而进行write的切换，但是一旦master出现故障那么当前的MMM就没有writer了只有对外的read操作。
    ips     192.168.137.100  #####对外提供的写操作的虚拟IP
    mode    exclusive    #####exclusive代表只允许存在一个主，也就是只能提供一个写的IP
</role>

<role reader>   #####read角色配置
    hosts   backup,slave  ######对外提供读操作的服务器的host名,当然这里也可以把master加进来
    ips     192.168.137.120,192.168.137.130,192.168.137.140  ###对外提供读操作的虚拟ip，这两个ip和host不是一一对应的,并且ips也hosts的数目也可以不相同，如果这样配置的话其中一个hosts会分配两个ip
    mode    balanced   ###balanced代表负载均衡
</role>

include mmm_common.conf
<monitor>
    ping_ips            192.168.137.10,192.168.137.20,192.168.137.30 # 被监控的db服务器的ip地址
 ping_interval       1 # 默认是1秒，也就是ping主机的监控时间间隔，详见MMM::Monitor::NetworkChecker
    auto_set_online     0 # 设置自动online的时间，默认是超过60s就将它设置为online，默认是60s，这里将其设为0就是立即online
    ip                  127.0.0.1
    pid_path            /var/run/mysql-mmm/mmm_mond.pid
    bin_path            /usr/libexec/mysql-mmm
    status_path         /var/lib/mysql-mmm/mmm_mond.status  #####群集的状态文件，也就是执行mmm_control show操作的显示来源。

    # The kill_host_bin does not exist by default, though the monitor will
    # throw a warning about it missing.  See the section 5.10 "Kill Host
    # Functionality" in the PDF documentation.
    #
    # kill_host_bin     /usr/libexec/mysql-mmm/monitor/kill_host
    #
</monitor>

### binlog_mode和隔离级别

1. 查看当前数据库状态，包括了版本、当前数据库等。
mysql> status

2. 创建测试表
mysql>CREATE TABLE foobar (
  id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
  book CHAR(10) DEFAULT NULL
  ) ENGINE=InnoDB;
Query OK, 0 rows affected (0.03 sec)

3. 测试RC隔离级别，使用STATEMENT
----- 改变事务级别为read committed
mysql>set session transaction isolation level read committed;
Query OK, 0 rows affected (0.00 sec)
----- 改变二进制日志格式
mysql>set binlog_format=STATEMENT;
Query OK, 0 rows affected (0.00 sec)
----- 插入数据测试
mysql>insert into foobar(book) values('wuli');
ERROR 1598 (HY000): Binary logging not possible. Message: Transaction level 'READ-COMMITTED' in InnoDB is not safe for binlog mode 'STATEMENT'

4. 测试隔离级别RR，使用STATEMENT
mysql>set session transaction isolation level REPEATABLE READ;
Query OK, 0 rows affected (0.00 sec)
mysql>insert into foobar(book) values('wuli');
Query OK, 1 row affected (0.00 sec)

5. 测试隔离级别RC，使用非STATEMENT格式
mysql>set session transaction isolation level read committed;
Query OK, 0 rows affected (0.00 sec)
mysql>set session binlog_format=row;   改为行格式
Query OK, 0 rows affected (0.00 sec)
mysql>insert into foobar(book) values('wuli');
Query OK, 1 row affected (0.00 sec)
mysql>set session binlog_format=mixed; 改为混合格式
Query OK, 0 rows affected (0.00 sec)
mysql>insert into foobar(book) values('wuli');
Query OK, 1 row affected (0.00 sec)

那是因为read committed可能会导致不可重复读，也就是说可以读取到其它事务已经提交的数据，如果基于STATEMENT格式的话，会导致主从数据不一样，因为STATEMENT是基于SQL语句的复制模式。



### 网页PPT
https://mmonit.com/monit/
https://vxhly.github.io/2016/09/03/reveal-js-cn-document/

### master_pos_wait

如果将MySQL设置为双主 M1(rw)+M2(r)，正常switchover时的切换流程大致如下：
1. 停止应用写M1，将M1设置为只读；
2. 检查M2的slave status直到赶上M1；
   2.1 在M1上show master status;得到binlog位置P，因为已经设为只读，不会变化；
   2.2 循环检测M2上的执行位置，若未到P，则过几秒再查，循环直到从库追上；
3. 将M1设置为可写。

实际上，对于第2步，可以通过select master_pos_wait(file, pos[, timeout])函数完成，这里的file和pos对应于主库show master status得到的值，该函数会等待当前从库达到这个位置后返回，返回参数为这期间执行的事务数；超时时间可选，默认是无限等待(参数小于0)。

返回值：-1)等待超时；NULL) 当前slave未启动或在等待期间被终止；0) 指定的值已经在之前达到。

master_pos_wait的实现逻辑

    用户调用该函数后，根据传入参数调用pthread_cond_timedwait或pthread_cond_wait。 SQL_THREAD线程每次apply完一个事件后会触发更新relay info, 并通知上面等待的线程。因为可能有多个用户等待，因此用广播方式。

    关于事件个数的计算比较复杂，有兴趣的同学可以看这篇, 不过在本文讨论的这个问题上，正数返回值并不重要。

用master_pos_wait来实现上面的步骤b，则可以简化为：
b’) 在M上执行select master_pos_wait(file, pos)，返回后判断一下返回值>=0 则认为主从同步完成。

 好处是
1) 简化逻辑，不用在应用脚本判断
2) 在追上的第一时间就能感知，否则可能多等若干秒

#### syslog
/etc/sysconfig/syslog
/etc/syslog-ng/syslog-ng.conf

syslog系统默认会存在两个守护进程klogd+syslog-ng(syslog-Next generation)，前者用于记录内核生成的日志，后者是用于非内核信息。


### syslog-ng
http://cyr520.blog.51cto.com/714067/1245650
http://ckl893.blog.51cto.com/8827818/1789967
提供了Client、Relay、Server三种模式，对于Client来说，配置文件中包括了全局配置、消息源、过滤器、消息目的地和日志路径。

# Global options.
options { long_hostnames(off); sync(0); perm(0640); stats(3600); };

# 'src' is our main source definition. you can add more sources driver
# definitions to it, or define your own sources, i.e.:
#source my_src { .... };
source src {
    # include internal syslog-ng messages
    # note: the internal() soure is required!
    internal();
    # the default log socket for local logging:
    unix-dgram("/dev/log");
    # uncomment to process log messages from network:
    #udp(ip("0.0.0.0") port(514));
};

source my_src {
    udp(ip("127.0.0.1") port(514));
};
filter f_local2 { facility(local2); };
destination d_haproxy { file("/var/log/haproxy.log", owner(elb), group(wheel), perm(0600)); };
log { source(my_src); filter(f_local2); destination(d_haproxy); };


#### 密码存储
http://www.yunweipai.com/archives/4518.html
http://www.jianshu.com/p/d13ff016ff88
https://www.zhihu.com/question/20479856
http://www.williamlong.info/archives/3224.html
http://www.freebuf.com/articles/web/28527.html
http://blog.csdn.net/shanliangliuxing/article/details/7365920


### Perl DBD::mysql 安装

安装perl-DBD-MySQL时，会出现错误提示依赖错误(error: Failed dependencies) libmysqlclient.so.18()(64bit) is needed by perl-DBD-MySQL-4.023-5.el7.x86_64 ，但是我们 mysql-community-libs 提供的是 libmysqlclient.so.20 动态库。当然，我们可以直接下载源码然后安装，不过也可以通过 mysql-community-libs-compat 安装，libmysqlclient.so.18

Percona-toolkits环境安装
1. 安装DBD::mysql模块
源码从https://dev.mysql.com/downloads/dbi.html下载，

perl Makefile.PL --mysql_config=/bin/mysql_config
make
make test
make install

#!/usr/bin/perl
use DBI;
$user="test";
$passwd="test";
$dbh="";
$dbh = DBI->connect("dbi:mysql:database=test;host=192.1.1.168;port=3306",$user,$passwd) or die "can't connect to
database ". DBI-errstr;
$sth=$dbh->prepare("select * from infobright_loaddata_status");
$sth->execute;
while (@recs=$sth->fetchrow_array) {
print $recs[0].":".$recs[1].":".$recs[2]."\n";
}
$dbh->disconnect;


### Linux C 动态库依赖

http://littlewhite.us/archives/301
http://bbs.chinaunix.net/thread-4130723-1-1.html


https://github.com/KredytyChwilowki/MySQLReplicaIntegrityCheck
https://www.percona.com/blog/2016/03/17/mysql-replication-primer-with-pt-table-checksum-pt-table-sync-part-2/
https://segmentfault.com/a/1190000004309169
http://www.cnblogs.com/zhoujinyi/archive/2013/05/09/3067045.html

保证数据一致性，常用的如数据校验、数据一致性备份。

建议通过pt-table-checksum完成数据校验，然后通过pt-table-sync进行数据修复，原因是前者会提供很好的限流措施，可以尽量减小对现网的影响。

### 使用参数

直接从源码中截取。

```
if DSN has a t part, sync only that table:
   if 1 DSN:
      if --sync-to-master:
         The DSN is a slave.  Connect to its master and sync.
   if more than 1 DSN:
      The first DSN is the source.  Sync each DSN in turn.
else if --replicate:
   if --sync-to-master:
      The DSN is a slave.  Connect to its master, find records
      of differences, and fix.
   else:
      The DSN is the master.  Find slaves and connect to each,
      find records of differences, and fix.
else:
   if only 1 DSN and --sync-to-master:
      The DSN is a slave.  Connect to its master, find tables and
      filter with --databases etc, and sync each table to the master.
   else:
      find tables, filtering with --databases etc, and sync each
      DSN to the first.
```

简单来说，如果使用了--replicate或者--sync-to-master可以只使用一个DSN配置，否则就需要配置两个DSN值。

#### 常用参数介绍如下

* --sync-to-master   
将当前库视为备库，需要去同步主库数据，此时会修改--wait=60 --lock=1 --notransaction ；
* --replicate   
与pt-table-checksum工具的对应参数相同，该工具会通过WHERE过滤检查数据，并与主库同步数据，如果没有通过--sync-to-master参数指定备库，那么就会尝试通过SHOW PROCESSLIST、SHOW SLAVE HOSTS查找备库；
* --wait   
主库等待备库多长时间追上主库，如果超时则输出MASTER_POS_WAIT returned -1，此时可以尝试增加时间，默认会同时设置--lock=1 --notransaction；
* --timeout-ok   
默认主库等待备库超时后会直接退出，通过该选项可以使程序继续执行，如果要保证一致性的比较最好退出!!!
* --[no]check-triggers  
检测目标表上是否定义了trigger，如果定义默认会直接退出；
* --execute   
如果有数据不一致可以直接执行，如果担心软件有问题，可以通过--print打印SQL而非执行；
* --algorithms   
数据校验时采用的算法，目前支持chunk、nibble、groupby、stream四种算法；

调试参数：
* --print   
主要用于防止工具执行错误，此时不会真正执行表同步操作，只打印需要执行的SQL，用于手动执行；
* --dry-run   
用于调试，会同时设置--verbose选项，此时工具并不会访问表，所以也无从知道哪些表出现了不一致，只用于显示会执行哪些操作；
* --explain-hosts   
只打印链接各个数据库的参数信息，然后退出；


### 数据校验算法

数据校验算法目前包括了Chunk、Nibble、GroupBy、Stream四种算法，当然上述算法各有优缺点，在源码中通过插件实现，也可以新增新插件实现；在此，主要介绍前两种方式：
* Chunk开始会将表分给为chunks，适用于区分度高的表；
* Nibble与Chunk类似，不过分区是通过limit实现；

### 执行流程


在此，主要介绍指定--replicat参数时，该工具的详细执行流程：

1. 从校验表中找到不一致的chunk，包括其边界，详见find_replication_differences()函数，然后一般是每行开始循环校验；
2. 对主库使用FOR UPDATE对chunk加锁，从库使用LOCK IN SHARE MODE，正常来说在主库添加了悲观锁之后从库的数据就不会被修改；
3. 上步同时会通过SELECT ... CRC32(CONCAT_WS('#', ...))生成校验码，这里实际上只使用主键，如果数据不一致后续统一获取各行的值；
4. 如果上述有异常，则将数据保存到队列中，后续统一处理；
5. 根据不同的类型，会生成REPLACE、INSERT、DELETE等语句；
6. 通过lock_and_wait()等待主从同步完成，然后开始修复数据，每次会处理一行记录，并最后等待同步完成。


### pt-table-sync

perldoc /bin/pt-table-sync

pt-table-sync --print --algorithms=chunk --charset=utf8 --replicate=zabbix.checksums h=localhost,u=checker,p=Opsmonitordb@2015 h=192.30.19.82,u=checker,p=Opsmonitordb@2015

PTDEBUG=1

PTDEBUG=1 pt-table-sync --print --execute --algorithms=chunk --charset=utf8 --replicate=zabbix.checksums h=localhost,u=checker,p=Opsmonitordb@2015 h=192.30.19.82,u=checker,p=Opsmonitordb@2015 >/tmp/1 2>&1

### 登陆选项：
可以通过DSN指定，也可以使用参数。
--ask-pass
--password
--port
--host
--user
--socket
--slave-user
--slave-password

双向修复相关：
--bidirectional
--conflict-column
--conflict-comparison
--conflict-error
--conflict-threshold
--conflict-value

* --[no]bin-log   
是否记录到binlog中，如果采用GTID，那么在备库修复会存在问题；


### 过滤选项

可以指定只检查哪些数据库、表、列、存储引擎等，可以使用Perl正则表达式，也可以指定忽略哪些。

--ignore-columns
--ignore-databases
--ignore-engines
--ignore-tables
--ignore-tables-regex
=item --columns
=item --databases
=item --tables
=item --engines
#### 安全检查

用于检查是否可以进行同步，详细可以查看ok_to_sync()。

--[no]check-child-tables
--[no]check-master
--[no]check-slave
--[no]check-triggers
--[no]foreign-key-checks


=item --buffer-in-mysql
=item --[no]buffer-to-client
=item --charset
=item --chunk-column
=item --chunk-index
=item --chunk-size
=item --config
=item --defaults-file
=item --float-precision
=item --function
=item --[no]hex-blob
=item --[no]index-hint
=item --lock
=item --lock-and-rename
=item --pid
=item --recursion-method
=item --replace
=item --set-vars
=item --timeout-ok
=item --[no]transaction
=item --trim
=item --[no]unique-checks
=item --verbose
=item --version
=item --[no]version-check
=item --wait
=item --where
=item --[no]zero-chunk

GRANT ALL ON *.* TO 'checker'@'localhost';
GRANT ALL ON *.* TO 'checker'@'192.30.19.83';

main()
 |-lock_and_rename() 如果使用了--lock-and-rename参数
 |-sync_one_table() 只同步一个表，可通过--tables指定需要同步那些表
 |-sync_via_replication() 指定了--replicate参数，重点查看
 | | ###BEGIN###使用sync-to-master参数，也就是在备库执行
 | |-find_replication_differences() 校验主库是否由不一致，如果有那么直接忽略对应表
 | |-find_replication_differences() 查看备库中不一致的表
 | |-filter_diffs() 过滤不需要同步的表
 | |-lock_server() 只有在lock=3时才有效，会执行FLUSH TABLES WITH READ LOCK命令
 | |-sync_a_table() 一个表一个表同步数据
 | | |-get_server_time() 获取开始时间
 | | |-ok_to_sync() 检测是否可以同步表:1)获取表结构；2)确认表在目的库存在；3)确认目的库没有定义trigger；4)如果需要执行，确认目的库没有外键子表
 | | |-diff_where() 获取WHERE子句
 | | |-get_change_dbh() 获取需要执行修改操作的链接信息
 | | |-make_action_subs() 根据execute/print参数将设置回调actions函数
 | | |-sync_table() 真正执行数据修复的函数
 | | | |-get_best_plugin() 获取最适用的插件
 | | | | |-can_sync() 遍历各个插件，获取第一个可以使用的插件
 | | | |   |-find_chunk_columns() Chunk算法
 | | | |-prepare_to_sync()
 | | | |-lock_and_wait() 处理表锁：1) COMMIT/UNLOCK TABLES；2) 开启事务或者锁表(lock=3)；3) 通过MASTER_POS_WAIT()等待备库完成
 | | | | |-wait_for_master() 等待主库
 | | | |-###BEGIN###while 等待$plugin->done()完成
 | | | |-lock_and_wait() 只有在一个chunk的开始和结束会执行
 | | | |-execute() 在主库、备库分别执行
 | | | |-compare_sets() 比较结果集如果正常则会打印Left and right have the same key，如果有异常则会保存在队列中，一般值含主键
 | | | |-process_rows() 开始执行修复
 | | | | |-make_$action() 执行修改，一般为make_REPLACE...
 | | | | | |-make_row() 获取SQL
 | | | | |-_take_action() 开始执行
 | | | |-###END###while
 | | |-get_server_time() 获取结束时间
 | |-unlock_server()
 | | ###END###sync-to-master
 |-sync_all()

{% highlight text %}
{% endhighlight %}
