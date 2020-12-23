---
title: Redis 运维简介
layout: post
comments: true
language: chinese
category: [misc]
keywords:
description:
---



<!-- more -->




https://redis.io/topics/cluster-spec
https://redis.io/topics/cluster-tutorial
http://redisdoc.com/topic/cluster-tutorial.html

Redis 当前支持的数据类型可以参考 [An introduction to Redis data types and abstractions](https://redis.io/topics/data-types-intro) 中的介绍，很简单。

Redis 的高可用方案包括了：A) 主备模式；B) 哨兵模式；C) 集群模式。

## 集群模式

该模式在 Redis3.x 之后引入，主要是为了解决前两种模式中不能水平扩容的问题，这也是官方推荐的集群方案。

其设计的目标有：

1. 高性能，不能因为引入集群方案而对性能造成太大的影响，没有代理、异步复制、查询合并操作；
2. 水平扩展，这也是最被诟病的；
3. 高可用，之前的方案是通过哨兵来保证，而集群仍然可以采用这种方式。


### 写安全

Redis 集群采用异步的主从同步方式实现，只能保证最终一致性，在高并发的时候就可能会出现数据丢失。

例如，Client 向 Master 写请求，当 Master 写入成功并返回 Client 成功之后，在未同步到 Slave 之前 Master 宕机了，那么就可能会发生数据丢失。对于这一问题来说，很难进行恢复。

另外一种场景是 Master 宕机后 Slave 接管，一段时间之后 Master 恢复，如果客户端一直有缓存连接，那么就有可能重新连接到老的 Master 节点上写入数据，就可能导致异常。


https://juejin.im/post/5c1bb40a6fb9a049f36211b0


做水平扩展时，比较关键的是如何对数据做分区或者分片，常用的方法有：A) 普通哈希分区；B) 一致性哈希分区；C) 虚拟槽分区。

其中 Redis 选择的就是虚拟槽分区，将所有的数据映射到一个固定范围内的整数集合，其中将整数定义为槽 (slot) ，并将槽作为数据管理和迁移的最基本单位，其中 Redis 槽的范围为 `0~16383` 。

## 安装

直接安装对应的 RPM 即可，然后通过 `systemctl start redis` 即可，实际上直接通过 `redis-server /etc/redis.conf` 启动即可。

### 配置

#----- 是否要后台运行，例如systemd允许前台运行
daemonize no
#----- 如果进程后台运行，可以将PID写入到PID文件中
pidfile /var/run/redis.pid
#----- 监听端口，默认是6379
port 6379
#----- 监听地址
bind 127.0.0.1
#----- 客户端超时时间，当超过N秒后关闭连接
timeout 0
#----- 使用TCP的keepalive机制
tcp-keepalive 300
#----- 日志设置
loglevel notice
logfile /var/log/redis/redis.log
#----- 逻辑数据库的个数，每个数据库中的命名空间隔离
databases 16
#----- 持久化次数，一般是在多长时间内，有多少次更新操作，就将数据同步到数据文件
save 900 1     # 15分钟内有1个更改
save 300 10    # 5分钟内有10个更改
save 60 10000  # 60秒内有10000个更改


#>>>>>>>> 集群配置
#----- 开启集群模式
cluster-enabled yes

https://juejin.im/entry/5c246b17f265da615e058418
https://www.jianshu.com/p/41f393f594e8
https://www.twle.cn/l/yufei/redis/redis-basic-conf-options.html

----- 停止服务
redis-cli -h 127.0.0.1 -p 7001 shutdown

----- 查看所有KEY，可以正则匹配
redis> keys *


要让集群正常运作至少需要三个主节点，建议使用六个节点，其中三主三备。


通过 `cluster-config-file` 配置一个集群配置文件，该文件不能进行人工编辑，是由集群的节点自动维护，用来记录集群中有哪些节点、它们的状态以及一些持久化参数。

一个 Redis 集群由多个节点 Node 组成，初始启动的时候，各个节点之间互相独立，在搭建集群的过程中，需要将各个节点组合起来。

### 节点握手

当在节点 A 上执行 `cluster meet A:IP A:PORT` 命令时，A 实例会向 B 实例发送 meet 请求，当 B 实例接收到 meet 请求之后会回复 pong 消息，然后两个节点之间会定期进行通讯。

$ redis-cli -h 127.0.0.1 -p 7001
----- 查看集群的节点信息
redis> cluster nodes
----- 查看集群信息，关键的是cluster_slots_assigned字段
redis> cluster info

然后可以直接通过 `redis-cli --cluster create 127.0.0.1:7001 <ETC.> --cluster-replicas 1` ，其中的 `<ETC.>` 需要将所有的节点添加上。

redis-cli --cluster create 127.0.0.1:7001 127.0.0.1:7002 127.0.0.1:7003 127.0.0.1:7004 127.0.0.1:7005 127.0.0.1:7006 --cluster-replicas 1

也可以一步一步进行配置。

----- 和其它节点之间建立联系，将所有的6个集群添加到一起，如果要取消可以使用forget命令
redis> cluster meet 127.0.0.1 7002

----- 移动槽位
redis-cli --cluster reshard 127.0.0.1:7002
----- 检查当前健康信息
redis-cli --cluster check 127.0.0.1:7002
----- 添加master节点
redis-cli --cluster add-node 127.0.0.1:7007 127.0.0.1:7002
----- 添加slave节点
redis-cli --cluster add-node 127.0.0.1:7008 127.0.0.1:7002 --cluster-slave --cluster-master-id <ID>

添加 master 和 slave 节点时的命令基本相同，其中 slave 会通过 `--cluster-slave` 表示创建 slave 节点，并通过 `--cluster-master-id` 指定 master 节点。

### 设置主备

然后登陆作为 Slave 的节点，并执行 `cluster replicate <SVR-ID>` 命令，需要根据上面集群的配置拼接命令。

$ redis-cli -h 127.0.0.1 -p 7004
cluster replicate c312332c6722e934520a607660e6f199955dd4a6
$ redis-cli -h 127.0.0.1 -p 7005
cluster replicate 6df2e67e8fb6d298481fc16401134e851a3bf106
$ redis-cli -h 127.0.0.1 -p 7006
cluster replicate 35a850033f8542b6f290f182b8f94ce67bce01db

暂时不太确定 Redis 中的 slot 是如何持久化保存的。

redis-cli -h 127.0.0.1 -p 7001 cluster delslots {0..5461}
redis-cli -h 127.0.0.1 -p 7002 cluster delslots {5462..10922}
redis-cli -h 127.0.0.1 -p 7003 cluster delslots {10923..16383}

redis-cli -h 127.0.0.1 -p 7001 cluster addslots {0..5461}
redis-cli -h 127.0.0.1 -p 7002 cluster addslots {5462..10922}
redis-cli -h 127.0.0.1 -p 7003 cluster addslots {10923..16383}



### 槽分配情况

可以通过 `cluster slots` 查看当前槽的分配情况。

redis> cluste slots
1) 1) (integer) 10923
   2) (integer) 16383
   3) 1) "127.0.0.1"
      2) (integer) 7002
      3) "3110ed887330d428e1b544932dc22b1fcbd3fa45"
2) 1) (integer) 5461
   2) (integer) 10922
   3) 1) "127.0.0.1"
      2) (integer) 7001
      3) "2a1035a6642f8f1020c279c8a69ce66ff7eb564b"
3) 1) (integer) 0        //起始槽编号
   2) (integer) 5460     //结束槽编号
   3) 1) "127.0.0.1"     //IP
      2) (integer) 7000  //PORT
      3) "f745788c4c24711a11c8ad516ead9a485e5b1dbb" //NODE编号


## 常用命令

与集群操作相关的可以直接通过 `redis-cli --cluster help` 命令查看，常见操作如下。

----- 创建，其中参数为所有实例的列表，最后参数表示每个主节点需要一个从节点
redis-cli –-cluster create 127.0.0.1:7001 127.0.0.1:7002 127.0.0.1:7003 --cluster-replicas 1

----- 检查集群
redis-cli --cluster check 127.0.0.1:7001

----- 查看集群信息
redis-cli --cluster info 192.168.163.132:6384

----- 启动时需要使用-c标示集群模式
redis-cli -c -h 127.0.0.1 -p 7001


127.0.0.1:7001> set name foobar
-> Redirected to slot [5798] located at 127.0.0.1:7002
OK

----- 需要重新登录下
127.0.0.1:7001> get name
-> Redirected to slot [5798] located at 127.0.0.1:7002
"foobar"

注意，如果使用的是集群模式，在使用命令行的时候需要添加 `-c` 参数，否则会报类似 `(error) MOVED 5798 127.0.0.1:7002` 的错误。

https://my.oschina.net/guol/blog/506193

#!/bin/bash

WORKDIR=/opt/redis
BIND="127.0.0.1"
INSTANCES=(7001 7002 7003 7004 7005 7006)

GenerateConfigFile() {
        if [[ $# != 2 ]]; then
                echo "Expect 2 arguments when generate file."
                exit 1
        fi
        confile="${1}"
        instance="${2}"
        cat > "${confile}" << EOF
port ${instance}
dir /opt/redis/${instance}
bind ${BIND}
pidfile redis.pid
logfile redis.log
daemonize yes
cluster-enabled yes
cluster-config-file node.conf
cluster-node-timeout 15000
appendonly yes
EOF
}

if [[ "$1" == "start" ]]; then
        echo "STEP0: Create instance."
        for instance in ${INSTANCES[@]}; do
                instdir="${WORKDIR}/${instance}"
                confile="${instdir}/redis.conf"

                if [[ ! -d "${instdir}" ]]; then
                        mkdir -p "${instdir}"
                fi
                echo "   Generate config file '${confile}'."
                GenerateConfigFile "${confile}" "${instance}"

                echo "   Start redis server."
                /bin/redis-server "${confile}"
        done

        LIST=""
        for instance in ${INSTANCES[@]}; do
                LIST="${LIST} ${BIND}:${instance}"
        done
        /usr/bin/redis-cli --cluster create ${LIST} --cluster-replicas 1 --cluster-yes

        exit 0

        echo "STEP1: Prepare cluster."
        for i in "${!INSTANCES[@]}"; do
                if [[ x"$i" == x"0" ]]; then
                        continue
                fi
                echo "   Cluster meet ${INSTANCES[0]}."
                /usr/bin/redis-cli -h "${BIND}" -p "${INSTANCES[0]}" cluster meet 127.0.0.1 "${INSTANCES[$i]}" > /dev/null
        done
elif [[ "$1" == "stop" ]]; then
        for instance in ${INSTANCES[@]}; do
                echo "====> Stop instance '${BIND}:${instance}'."
                /usr/bin/redis-cli -h "${BIND}" -p "${instance}" shutdown
        done
elif [[ "$1" == "cleanup" ]]; then
        for instance in ${INSTANCES[@]}; do
                echo "====> Cleanup instance '${BIND}:${instance}'."
                /bin/rm -rf "${WORKDIR}/${instance}"
        done
fi

{% highlight text %}
{% endhighlight %}
