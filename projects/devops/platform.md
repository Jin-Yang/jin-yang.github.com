---
title: 搭建基础平台
layout: project
comments: true
language: chinese
category: [misc]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---

<!-- more -->

## Polaris


### REST-API

{% highlight text %}
-----> 发送请求
{
	"id": "2719cb14-2da6-44a0-8349-1ea33b3ab2d9",
	"hosts": [ "eccb24a7-fe55-4d84-9c40-6a9192419ae9", "b19ffa7e-712e-46d4-b328-2d2892c96076" ],
	"method": "sync.bash",
	"cmd": "ls"
}

<----- 响应报文
{
	"id": "2719cb14-2da6-44a0-8349-1ea33b3ab2d9",
	"response": [ {
		"host": "eccb24a7-fe55-4d84-9c40-6a9192419ae9",
		"resp": "failed",
		"retcode": 203,
		"message": "No such host"
	}, {
		"host": "b19ffa7e-712e-46d4-b328-2d2892c96076",
		"resp": "success",
		"retcode": 2,
		"message": "Normal termination",
		"data": "xxxxxx"
	} ],
}


{% endhighlight %}




## Agents

### 日志 Agent

### 安全 Agent

[Advanced Intrusion Detection Environment, AIDE](http://aide.sourceforge.net/) 是一个目录以及文件的完整性检查工具。

<!-- https://linux.cn/article-4242-1.html -->


## 其它



### 参考











<!--
前端介绍
https://github.com/brickspert/blog

前端使用的FontAwesom字体
http://corporate.joostrap.com/features/fontawesome-icons

http://sharadchhetri.com/2013/12/13/how-to-lock-and-unlock-user-account-in-linux/
https://github.com/Distrotech/shadow-utils/blob/distrotech-shadow-utils/src/chage.c

一些常用库
https://github.com/fmela/libdict
https://github.com/attractivechaos/klib
https://github.com/nanomsg/nanomsg

GoLang ZeroMQ
http://blog.haohtml.com/archives/14496

using zeromq with libev
https://github.com/pijyoi/zmq_libev
buffered socket library for libev
https://github.com/mreiferson/libevbuffsock

Introspected tunnels to localhost
https://github.com/koolshare/ngrok-libev
https://github.com/inconshreveable/ngrok

Proxy
https://github.com/isayme/socks5
https://github.com/z3APA3A/3proxy


Message Queue
https://github.com/je-so/iqueue *****
https://github.com/circonus-labs/fq
https://github.com/liexusong/mx-queued
https://github.com/chaoran/fast-wait-free-queue
https://github.com/haipome/lock_free_queue
https://github.com/supermartian/lockfree-queue
https://github.com/slact/nchan
https://github.com/darkautism/lfqueue
https://github.com/tylertreat/gatling
https://github.com/bangadennis/networking
https://github.com/fcten/webit

HTTP Server
https://github.com/bachan/ugh
https://github.com/dexgeh/webserver-libev-httpparser
https://github.com/Lupus/libevfibers
https://github.com/h2o/h2o
https://github.com/monkey/monkey
https://github.com/lpereira/lwan


http://www.tildeslash.com/libzdb/#





DROP TABLE IF EXISTS `hosts`;
CREATE TABLE IF NOT EXISTS `hosts` (
	`id` BIGINT NOT NULL AUTO_INCREMENT,
	`name` VARCHAR(128) DEFAULT NULL COMMENT 'TAG、服务或主机标示(如AgentSN、IP、HOSTNAME)',


	`gmt_create` datetime DEFAULT CURRENT_TIMESTAMP,
	`gmt_modify` datetime ON UPDATE CURRENT_TIMESTAMP,
	UNIQUE KEY uk_name (name),
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `jobs`;
CREATE TABLE IF NOT EXISTS `jobs` (
	`id` BIGINT NOT NULL AUTO_INCREMENT,


	`gmt_create` datetime DEFAULT CURRENT_TIMESTAMP,
	`gmt_modify` datetime ON UPDATE CURRENT_TIMESTAMP,
	UNIQUE KEY uk_name (name),
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


DROP TABLE IF EXISTS `host_job`;
CREATE TABLE IF NOT EXISTS `host_job` (
	`hostid` BIGINT NOT NULL,
	`jobid` BIGINT NOT NULL,
	UNIQUE KYE (`jobid`, `hostid`),
	PRIMARY KEY (`hostid`, `jobid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


假设产品树采用的是三层结构，
产品，对外售卖的服务(如ECS、RDS等)、内部使用平台(数据库管理平台、运维系统等)
服务，一般也就是一个团队独立开发维护的。
组件/微服务，可以独立安装部署的最小单元，例如Console、DB、Server等。

那么保存的任务就会涉及了继承的层级关系，一般来说层级越低的优先级也越高 ECS < OpenStack < Console < Host ，而这里的优先级处理则是直接通过服务端进行处理。



获取主机对应的任务



内存不足测试
https://oomake.com/question/12305
https://code-examples.net/zh-CN/q/1a9c8
kill信号的排查
https://hongjiang.info/shell-script-background-process-ignore-sigint/


1. 超过最大buffer则丢弃后续的数据，目前设置为 64K。
2. 内存不足、read返回失败(认为内部错误，会打印错误信息) 时会强制 KILL 子进程。
   此时进程会返回状态9(直接在回调函数中kill进程进行测试)
3. fork进程后没有通过exevp()执行，而是直接退出。
   此时通过valgrind会看到有reachable的报错，主要是子进程继承的资源未被释放，如果子进程退出时释放所有资源，那么就不会报错。
   不过可以忽略，操作系统会在进程退出时对这部分的内存进行回收。
4. 执行超时3次以后则会强制退出。测试脚本需要忽略 SIGINT(2) 信号，也就是 trap '' 2
5. libev在新创建的进程里面，实际不需要手动关闭正常运行流程，但是以防后面添加了其它的处理逻辑，还是关闭掉。













             revoke
 [PENDING]------------ >[REVOKED]
     |
     |
     |
     |  put with delay               release with delay
     |`--------------- > [DELAYED] < ---------------------.
     |                       |                             |
     |                kick   | (time passes)               |
     |                       |                             |
     |  put                  v     reserve<1>              |       delete
      `---------------- > [READY] ------------------ > [RESERVED] -------- > [SUCCESS] *poof*
                            ^  ^                        |  |  |
                            |   \    release(retry)     |  |  |
                            |    `---------------------'   |  |
                            |                              |  |
                            | kick                         |  | delete
                            |                              |  |
                            |       bury(retry many times) |  |
                         [BURIED] <-----------------------'   |
                            |                                 |
                            |          delete                 |
                             `--------------------------------`---- > [FAILED] *poof*


## 任务状态

任务最终状态。

* Pending 任务已经下发但是还没有拆解添加到队列中。
* Success 执行成功，包括重试N次之后成功。
* Failed 执行失败，包括重试N次之后失败。
* Revoked 任务被撤销，注意除了Reserved状态的任务外，其它状态都可以被撤销。

任务队列中保存的状态。

* Ready 任务已经满足条件(一般是时间到了)准备被调度执行。
* Reserved 任务被Worker取出准备执行，也就是预定。
* Delayed 任务延迟一段时间之后调整为 Ready 状态。
* Buried 一般是任务处理失败，等待后续的调用，通常超过一段较长时间后自动会删除。


1. 如果Worker拿到任务之后由于某些原因(宕机、进程崩溃等)失败了。
   这一场景会在当 ttr 超时后重新添加到 Ready 队列中，如果 ttr 过长那么会导致等待时间过长，为此最好添加主动探测机制。

int put(unsigned int priority, int delay, int ttl, int ttr, int nbytes, char *buffer);
   新增一个任务。
   Args:
     @priority 任务的优先级，值越小优先级越高，默认是 1024，取值范围为 (0~2^32) 。
     @delay    延迟调度，单位是秒，此时的任务处于delayed状态。
     @ttl      Time To Live 调度有效期，从放置到Ready队列中，到开始执行的时间，如果超过则直接丢弃。
     @ttr      Time To Run 允许Worker的最大执行秒数，对处于Reserved状态的任务，如果在这段时间内。
     @nbytes   任务消息体的大小。
   Return:
     > 0 任务添加成功，返回的是任务ID

int reserve(int ttr)
   取出任务，并将任务保存为Reserved状态，对于取出的任务要求在任务的ttr时间内执行完成，超时则重装为Ready。
   Args:
     @ttr      Time To Run 用户端同时可以设置超时时间，一般 <= 0。

int delete(int taskid)
   删除任务。

int release(int taskid)
   将任务重新放回到 Ready 队列中。

int bury(int taskid)
   将任务迁移到Buried状态，一般是在执行失败之后，此时一般需要用户介入，判断是否要重新执行。

int touch(int taskid)
   为了防止任务超时，更新任务的执行时间，一般用于复杂场景。

GDB使用
http://sunyongfeng.com/201506/programmer/tools/gdb.html
https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf

任务调度策略(参考Beanstalk)
https://github.com/kr/beanstalkd/blob/master/doc/protocol.zh-CN.md

## 统一错误码

通过uint16_t表示，分为 ErrorType(高八位)、ErrorCode(低八位) 

01 执行结果返回错误，也就是任务已经调度执行，只是任务本身执行时返回错误。
    00 执行超时。
	01 任务执行报错。

命令管道：
   A) 同步命令，直接通过TCP链接执行，会有超时时间设置；不会保存到任务队列中，由用户负责任务失败后的重试。
   B) 异步任务，一般耗时比较大，如安装部署；通过任务队列调度执行，可以指定调度策略(该策略由上层负责调度，如分批、延迟、定时等)。
   D) 持久任务，由上层下发到底层的Agent，并由Agent负责任务的调度执行，通过任务管理系统保证上下的参数一致。
   C) 查询接口，提供通用的实时查询接口；例如用户数、IP地址、内存、CPU等信息。

高可用、高并发
安全：A) 安全通道：签名、密文；B) 使用者权限控制：命令白名单、执行范围、IP白名单。

场景：
1. 贯穿服务器整个生命周期。
   1.1 资产核对。
   1.2 装机、硬件故障报修、维护、下线
2. 应用运维。发布、回滚、重启
3. 日志服务。推、拉
4. 监控。OS、JVM、DB、安全
5. 配置管理


任务依赖。
Directed Acyclic Graph, DAG

1. 任务下发执行、状态/结果查询，自动重试。PUSH
2. 全量持久化任务校验、同步。低频 PULL

保证单个任务下发成功。

基于产品树的任务继承，类似于模板机制

SVR 负责任务分发，监控 Agent 的状态，同步 Agent 任务。

Tips:
1. 所有时间除了前台展示都是 UTC/GMT 时间。

### 1. 任务管理

支持调度策略(批量、延迟下发)、重试策略(失败直接退出、失败等N秒后重试M次)、超时机制(任务时效性，超过一段时间后不需要再执行)、任务状态查询、任务取消、任务停止。

#### 1.1 任务下发

主要是接收用户发送的请求，校验参数是否合法，并持久化到数据库中。

A). 用户拼接请求，通过 REST API 调用发送到服务端。
B). 服务端接收到任务后校验入参是否合法。
   B.1 参数格式不符合规范。
   B.2 操作资源不存在，例如主机、产品树。
   B.3 服务端不可用，例如任务入库失败。

在这一步执行完成之后，将未调度执行的任务保存到数据库的重试列表中，由任务调度系统完成后续的重试逻辑，常见场景有：

1. SVR 升级、异常导致不可用。
2. Agent 升级、异常导致不可用；对于该场景，Agent 需要提供查询的 API 接口，如果失败则直接发起重试。

#### 1.2 任务调度执行

任务调度是通过 SVR 执行的，对于单台主机任务 ID 是单调递增的，如果发现 ID 不匹配，那么就需要获取相关任务的 ID 信息。

A). 按照用户配置的策略，将任务拆分为主机粒度，如果已经满足了运行条件，则添加到Kafka队列中。
B). SVR会订阅指定的Topic消费任务信息，并执行属于该主机的任务。<1>


1. 这里会存在几个问题；
   1.1 如果在消费时主机不在线，会导致任务丢失，此时的任务ID会出现空白，需要通过 RPC 远程请求任务。

维护两个任务 ID，包括主任务以及拆分后的任务。


#### 讨论

对于任务调度以及状态查询，实际上最好的方式是使用类似于 Beanstalk 专有的任务调度关系系统，不过类似于 Memcached ，这是一个单点，而且各个节点之间无法相互通讯。

因为缺少任务队列，只能保存、查询部分的任务状态，同时需要增加 Redis 。

https://github.com/crazy-canux/awesome-monitoring
-->



{% highlight text %}
{% endhighlight %}
