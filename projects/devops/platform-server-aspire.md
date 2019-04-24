---
title: Aspire 实现
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

用来管理 BootAgent 。

## 实现功能

通过 gRPC 实现通讯，同时提供 REST-API 接口。

{% highlight text %}
1. 子Agent管理。
   1.1 安装、卸载，启动、停止、重启。
   1.2 健康检查、资源限制。
2. 命令通道。
   2.1 修改BootAgent配置。上报时间间隔、Tags列表。
   2.2 Agent管理命令，也就是1.1的内容。
3. 前端展示。分为三个TAB页进行管理。
   3.1 DashBoard。主机总数、在线数；Agent的部署数量、在线数。采用离线统计。
   3.2 主机展示。显示基本信息，包括AgentSN、Region分组、主机名、管理IP、OS类型、架构、版本类型、状态(在线、离线、删除)、最近更新时间(状态变化时才会修改)。下拉展示TAG信息。
   3.3 Agent版本管理。包括了BootAgent及其子Agent的版本信息，包括Agent名称、版本号、OS类型、架构、文件路径、安装命令、SHA256。
   3.4 TAG管理。包括TAG的Key/Value、绑定主机数、更新时间。
{% endhighlight %}

注意，所谓的命令通道是有限的功能，简单来说，保证成功率，但是不保证时效性。实现时有两种策略：

最简单的是单机或者通过TAG更新的任务，此时会为每台主机生成一个任务进行跟踪。

另外一类是全局任务，这一类任务在主机数过大时会导致生成任务列表耗时过大，因此会置位一个全局标记，在 BootAgent 上报状态信息时匹配对应的字段，如果不符合则执行对应的操作。(暂时不实现)

## Agent API

{% highlight text %}
pyresttest http://booter.cargo.com:8180 contrib/tests/register.yaml --print-bodies=true --log=debug
{% endhighlight %}

## REST-API

提供了针对 Agent 的 API 接口，其对应的地址为 `booter.cargo.com:8180` 。

### 响应格式

主要是分为了成功和失败的报文格式。

{% highlight text %}
{
	"status": 130010,                        统一的错误码
	"message": "not found",                  报错信息，与错误码对应
	"cause": "no agent package found"        具体的报错信息
}

{
	"status": 0,                             表示正常
	"message": "success"
	"cause":"already deleted"                可选
}
{% endhighlight %}

### 安装包管理

会根据入参自动生成一个相对路径，一般为 `$PACKAGE_PATH/{OS}/{ARCH}/{FILENAME}` 。

{% highlight text %}
----- POST   /api/v1/server/package 上传包
{
	"name":"FoobarAgent",
	"version":"1.2.3-rc1",
	"arch":"x86_64",
	"os":"linux",
	"file":"FoobarAgent-1.2.3-rc1.x86_64.rpm"
}

----- GET    /api/v1/server/package?name=FoobarAgent&version=1.2.4.-rc1&offset=0&limit=10&order=asc&sortby=id
{
	"count": 2,
	"offset": 0,
	"limit": 10,
	"packages": [{
		"id": 7,
		"name": "FoobarAgent",
		"os": "linux",
		"arch": "x86_64",
		"version": "1.2.3-rc1"
		"file": "FoobarAgent-1.2.3-rc1.x86_64.rpm",
		"path": "packages/linux/x86_64/FoobarAgent-1.2.3-rc1.x86_64.rpm",
		"gmt_create": "2018-11-09T00:14:27+08:00",
		"gmt_modify": "2018-11-09T00:14:27+08:00",
	}]
}

----- DELETE /api/v1/server/package?name=FoobarAgent&version=1.2.4.-rc1&offset=0&limit=10&order=asc&sortby=id
{% endhighlight %}

### Agent管理

{% highlight text %}
----- POST /api/v1/server/task 下发任务
{
	"command": "install",                 # 必选，任务类型
	"agents": [                           # 服务器列表
		"9b62aabb-eda4-4d62-b036-b605d887bde3",
		"845d6374-7d4e-402f-87cc-2a780e794dd6"
	],
	"parameters": {
		"url": "http://xxxx",
		"url": "command://yum install MiniAgent -y",   # 通过命令安装
	}
}

{
	"id": 135,                               # 自动生成的任务ID
	"status": 0,                             # 状态码，0 表示成功
	"message": "success",                    # 返回的信息，可能是报错
	"results": [{
		"AgentSN": "9b62aabb-eda4-4d62-b036-b605d887bde3",
	}, {
		"AgentSN": "845d6374-7d4e-402f-87cc-2a780e794dd6",
	}]
}


----- GET  /api/v1/server/agent?agentsn="845d6374-7d4e-402f-87cc-2a780e794dd6"
{% endhighlight %}


### 任务管理

目前基于两种模式：

1. 根据 AgentSN 。直接指定 AgentSN 列表，此时会拆解成子任务运行，当子任务运行完成则返回。
2. 基于 Tags 。会在一个时间窗内一直有效，如果与 Agent 上报的 tag 与之匹配，那么就会下发相关对任务信息。

也就是说，前者适用于少量的 Agent 更新，可以控制数量；而后者适用于大量 Agent 的升级，只能控制时间范围，不能控制具体的更新数量。

{% highlight text %}
----- POST /api/v1/server/task 下发任务
{
	"command": "install",                 # 必选，任务类型
	"agents": [                           # 服务器列表
		"9b62aabb-eda4-4d62-b036-b605d887bde3",
		"845d6374-7d4e-402f-87cc-2a780e794dd6"
	],
	"parameters": {
		"url": "http://xxxx",
		"url": "command://yum install MiniAgent -y",   # 通过命令安装
	}
}

{
	"id": 135,                               # 自动生成的任务ID
	"status": 0,                             # 状态码，0 表示成功
	"message": "success",                    # 返回的信息，可能是报错
	"results": [{
		"AgentSN": "9b62aabb-eda4-4d62-b036-b605d887bde3",
	}, {
		"AgentSN": "845d6374-7d4e-402f-87cc-2a780e794dd6",
	}]
}

----- GET  /api/v1/server/task?id=1 查询任务
{% endhighlight %}


## 表结构

记录了 BootAgent 以及 Aspire 服务端所生成的事件信息，通过 `AgentSN`、`局部ID`、`发生时间戳` 唯一确定一个事件。

{% highlight sql %}
CREATE DATABASE IF NOT EXISTS `Aspire`;
USE `Aspire`;

DROP TABLE IF EXISTS `subtasks`;
CREATE TABLE IF NOT EXISTS `subtasks` (
	`id` CHAR(40) NOT NULL PRIMARY KEY,
	`agentsn` CHAR(64) NOT NULL COMMENT "该任务对应的Agent",
	`status` INT NOT NULL DEFAULT 0 COMMENT "状态 0: INIT",
	`taskid` INT NOT NULL COMMENT "拆解前的任务ID",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	INDEX idx_task(`taskid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "针对任务拆解后的任务信息";

DROP TABLE IF EXISTS `tasks`;
CREATE TABLE IF NOT EXISTS `tasks` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`status` INT NOT NULL DEFAULT 0 COMMENT "状态 0: INIT",
	`user` CHAR(64) COMMENT "下发任务的用户名",
	`count` INT NOT NULL COMMENT "拆解后的子任务数",
	`complete` INT NOT NULL DEFAULT 0 COMMENT "已经完成的子任务数",
	`body` VARCHAR(4096) NOT NULL COMMENT "任务消息体",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "服务端下发的任务";

DROP TABLE IF EXISTS `pendings`;
CREATE TABLE IF NOT EXISTS `pendings` (
	`subtaskid` CHAR(40) NOT NULL PRIMARY KEY,
	`taskid` INT NOT NULL,
	`status` INT NOT NULL DEFAULT 0 COMMENT "状态 0: INIT",
	`gmt_schedule` TIMESTAMP NOT NULL DEFAULT 0,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	INDEX idx_sched(`gmt_schedule`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "正在处理的任务";

DROP TABLE IF EXISTS `events`;
CREATE TABLE IF NOT EXISTS `events` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`agentsn` CHAR(64) NOT NULL COMMENT "那个机器发生的事件",
	`category` INT NOT NULL DEFAULT 0 COMMENT "事件的分类 0:Server",
	`localid` INT NOT NULL COMMENT "机器的本地ID标示",
	`occured` TIMESTAMP NOT NULL COMMENT "事件发生的时间戳",
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_event` (`agentsn`, `localid`, `occured`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "Agent发生的事件记录";

DROP TABLE IF EXISTS `agents`;
CREATE TABLE IF NOT EXISTS `agents` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "名称",
	`os` CHAR(64) NOT NULL COMMENT "操作系统类型，例如Linux、Windows10等",
	`arch` CHAR(16) NOT NULL COMMENT "机器架构，例如x86_64、ARM等",
	`version` CHAR(64) NOT NULL COMMENT "版本号",
	`file` VARCHAR(1024) NOT NULL COMMENT "文件名称",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_name_version` (`name`, `os`, `arch`, `version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "Agent发布的版本信息";
INSERT INTO agents(name, version, arch, os, file, gmt_modify) VALUES("BasicAgent", "0.1.1-1", "x86_64", "linux",
                "BasicAgent-0.1.1-1.x86_64.rpm", now());
INSERT INTO agents(name, version, arch, os, file, gmt_modify) VALUES("MonitorAgent", "0.1.2-1", "x86_64", "linux",
                "MonitorAgent-0.1.2-1.x86_64.rpm", now());

DROP TABLE IF EXISTS `hosts`;
CREATE TABLE IF NOT EXISTS `hosts` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`agentsn` CHAR(64) NOT NULL COMMENT "AgentSN用来唯一标示一台主机",
	`hostname` CHAR(64) COMMENT "主机名",
	`ipaddr` CHAR(64) COMMENT "注册时连接到Server的客户端IP，管理面IP",
	`region` INT NOT NULL DEFAULT 0 COMMENT "所属region信息",
	`status` INT NOT NULL DEFAULT 0 COMMENT "主机状态 0:unknown 1:register 2:online 3:offline",
	`feature` INT NOT NULL DEFAULT 0 COMMENT "Agent所具有的特性 0: cgroup",
	`step` INT NOT NULL DEFAULT 300 COMMENT "Agent的采集时间间隔，单位是秒",
	`prevers` CHAR(64) COMMENT "上个版本，用来回滚",
	`curvers` CHAR(64) COMMENT "当前版本号",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE KEY `uk_agentsn` (`agentsn`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "保存主机基本信息";

DROP TABLE IF EXISTS `tags`;
CREATE TABLE IF NOT EXISTS `tags` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "TAG名称",
	`value` CHAR(64) NOT NULL COMMENT "TAG值",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "TAG信息表";
INSERT INTO tags(name, value) VALUES("svc", "ecs");

DROP TABLE IF EXISTS `regions`;
CREATE TABLE IF NOT EXISTS `regions` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(64) NOT NULL COMMENT "Region的名称",
	`gmt_modify` TIMESTAMP NOT NULL ON UPDATE CURRENT_TIMESTAMP,
	`gmt_create` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "保存的Region信息";
INSERT INTO regions(id, name) VALUES(0, "global");

DROP TABLE IF EXISTS `hosts_tags`;
CREATE TABLE IF NOT EXISTS `hosts_tags` (
	`hostid` INT NOT NULL COMMENT "主机ID信息",
	`tagid` INT NOT NULL COMMENT "TAG ID信息",
	PRIMARY KEY (hostid, tagid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "主机与TAG的关联表";

DROP TABLE IF EXISTS `hosts_agents`;
CREATE TABLE IF NOT EXISTS `hosts_agents` (
	`hostid` INT NOT NULL COMMENT "主机ID信息",
	`agentid` INT NOT NULL COMMENT "Agent版本信息",
	PRIMARY KEY (hostid, agentid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "主机与Agent的关联表";
{% endhighlight %}



## API




<!--
https://my.oschina.net/henrylee2cn/blog/741315
-->

<!--
#### 3. 常驻进程管理

主要用于管理一些子进程，包括了监控、日志、安全相关的进程管理。

##### 3.1 表结构设计

常驻进程相关的数据同样保存在 `gearman.db` 文件中。

{% highlight sql %}
DROP TABLE IF EXISTS `daemon`;

CREATE TABLE IF NOT EXISTS `daemon` (
    `name` CHAR(64) PRIMARY KEY NOT NULL,
    `version` CHAR(64) NOT NULL,
    `url` CHAR(512) NOT NULL,
    `precheck` CHAR(512),
    `precheck_retries` INT,
    `start` CHAR(512) NOT NULL,
    `start_delay` INT,
    `stop` CHAR(512) NOT NULL,
    `stop_timeout` INT,
    `check` CHAR(64),
    `check_interval` INT,
    `user` CHAR(64),
    `group` CHAR(64),
    `pidfile` CHAR(512),
    `socket` CHAR(512),
    `env` TEXT,
    `limits` TEXT,
    `gmt_modify` NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `gmt_create` NOT NULL DEFAULT CURRENT_TIMESTAMP
);
{% endhighlight %}


##### 3.2 下发任务

这里只会更新数据库。

{% highlight text %}
{
    "id": "xxxxx",
    "method": "daemon.init",
    "name": "plugin_name",                   // 必选，任务名称 64
    "version": "1.0.1",                      // 必选，版本号 64
    "url": "http://your/monitor.rpm",        // 必选，下载安装包路径 512
    "precheck": "/your/program/path/check",  // 启动前检查命令 512
    "precheck_retries": 3,                   // 检查重试次数
    "start": "/your/program/path/start",     // 必选，启动命令 512
    "start_delay": 10,                       // 启动任务后sleep多久之后检查是否启动成功
    "stop": "/your/program/path/stop",       // 必选，停止任务 512
    "stop_timeout": 10,                      // 停止任务超时时间，超时直接kill -9
    "check": "process",                      // 存活检查方式
    "check_interval": 10,                    // 存活检查时间间隔
    "user": "user name",
    "group": "group name",
    "pidfile": "/var/run/program.pid",
    "socket": "/var/run/program.sock",
    "env": {
         "PATH":"/usr/bin:/usr/sbin"
    },
    "limits": {
        "hits": 5,
        "cpu": "5%",
        "memory": "100M"
    }
}
{% endhighlight %}

其中 stop 中可以使用 `<process|kill>:argument` 这种方式，前者会执行一个命令，后者则会发送信号给对应的进程。


#### 4. 服务端表结构设计

{% highlight sql %}
DROP TABLE IF EXISTS `hosts`;
CREATE TABLE IF NOT EXISTS `hosts` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`hostname` CHAR(256) NOT NULL COMMENT "可以是主机名、AgentSN等",
	`region` CHAR(64) NOT NULL DEFAULT "unkown" COMMENT "所属region信息",
	`status` ENUM('unknown', 'online', 'offline') DEFAULT 'unknown' COMMENT "主机状态",

	`gmt_modify` NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`gmt_create` NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "保存主机基本信息";

DROP TABLE IF EXISTS `plugins`;
CREATE TABLE IF NOT EXISTS `plugins` (
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`name` CHAR(128) NOT NULL COMMENT "插件名称",
	`version` CHAR(64) NOT NULL COMMENT "插件版本号信息",

	`gmt_modify` NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`gmt_create` NOT NULL DEFAULT CURRENT_TIMESTAMP,
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT "插件信息";
{% endhighlight %}



https://github.com/sevagas/swap_digger

## 直方图

High Dynamic Range Histogram
http://hdrhistogram.org/
https://github.com/HdrHistogram/HdrHistogram_c

https://github.com/powturbo/TurboHist
https://github.com/astrofrog/fast-histogram

Vector Field Histogram 机器人中的算法
https://github.com/agarie/vector-field-histogram
SSH加固
https://www.freebuf.com/articles/system/185846.html

https://www.freebuf.com/articles/web/186298.html
DNS后门
https://www.freebuf.com/articles/network/185324.html
https://www.freebuf.com/articles/system/185942.html
https://www.freebuf.com/sectool/185276.html

test/common.c
中对应的value_to_rate


/home/andy/Workspace/4-Aspire/gopath:/home/andy/Workspace/go
unexpected directory layout:
        import path: _/home/andy/Workspace/4-Aspire/gopath/src/github.com/aspire/taskinfo
        root: /home/andy/Workspace/4-Aspire/gopath/src
        dir: /home/andy/Workspace/4-Aspire/gopath/src/github.com/aspire/taskinfo
        expand root: /home/andy/Workspace/4-Aspire/vendor
        expand dir: /home/andy/Workspace/4-Aspire/taskinfo
        separator: /
-->

{% highlight text %}
{% endhighlight %}
