---
title: 通用规范
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

## 规范

### 变量名

这里的命名规范包括了 `Tags` 。

可用字符为 ASCII ，包括了 `A-Z` `a-z` `0-9` `_` `@` `#` 。

### 统一错误码

错误码采用 `32bit` 的无符号整型，也就是最大为 `4294967295` ，如果采用十六进制表示，对于用户来说不友好，所以采用十进制标示，总共 10 位，区分如下。

{% highlight text %}
00000 00 000
   高三位 分类，BootAgent 003 BasicAgent 006 MonitorAgent 009
             Aspire 013
   中三位 由不同的分类进行重新划分
   低四位 具体的错误码信息，可以用有限的16bit标识
{% endhighlight %}

对于 REST API 来说，返回的错误信息为。

{% highlight text %}
{
	"status": 0,                       统一错误码
	"message": "invalid parameters",   统一错误信息，是错误码的解释
	"cause": "failed to login",        具体的错误信息
}
{% endhighlight %}

#### BootAgent

{% highlight text %}
000 000 内部错误信息
    001 任务下发失败
{% endhighlight %}

#### Server(Aspire)

也就是 BootAgent 的服务端，高位是 `0x10` 。

{% highlight text %}
0x1001   AgentSN冲突无法更新
{% endhighlight %}

### 测试用例

包括了如下几种类型：

* 单元测试，一般是白盒测试，针对代码块的测试，可以直接在编译时运行。
* 功能测试，针对具体功能点的测试，例如通讯模块。
* 集成测试，在整个系统搭建完成后的测试，包括了异常的场景，一般需要整套的系统运行。

因为对于 Agent 来说，需要支持不同的平台(Linux、Windows、IOS等)，不同的内核版本，为了方便在不同的平台之间进行迁移，那么需要根据具体的特性生成一些平台相关的测试用例，以方便对不同的平台进行评估。

测试方式包括了简单的测试用例，通过宏实现的条件开关等等，这里简单称之为平台依赖测试。

<!--
## C 语言规范

switch 未使用 break 或者 return 时，应该通过 /* fall-through */ 添加注释。

模块内部函数参数的合法性检查，由调用者负责。
-->


## 日志管理

日志用来记录用户操作、系统运行状态等信息，对于问题排查、审计、性能风险评估等都非常重要，对于目前的监控体系来说，日志监控是很重要的一部分。

一般分为了如下几类：

1. 诊断日志。A) 一般在操作异常时打印相关信息，例如打开文件失败、数据库连接建立失败、连接服务端失败、三方接口调用失败等；B) 耗时操作打印，此时需要统计操作时间，对于时间超过预期的打印相关信息。
2. 审计日志。一般是一些比较关键、风险高的操作打印。
3. 统计日志。通常是为了提供监控能力，格式相对来说比较严格，便于统计，例如：UV、PV、请求耗时等。

### 审计日志

对于审计日志而言，该日志是在任何情况下都不能关闭掉的，而且格式统一，方便过滤、清洗，一般采用的是 jsonfmt 或者 logfmt 。

一般来说，会基于正常的日志系统进行格式的封装，为了可以快速解析，可以对字段进行约束，例如不能有空格、引号，除非是最后一个字段。例如，可以提供如下的接口 `log_audit(reqid, type, body);` 。

### RequestID

也就是请求 ID ，一般用来标识从那台主机、什么时间开始 (如果需要也可以分成几部分，例如 ID + 序号)，对于分布式系统排查问题来说尤其重要。

其中使用比较多的是 UUID ，除了 UUID 之外，还有一个 [ULID Spec](https://github.com/ulid/spec) 也提供了一个类似的唯一标识，可以参考其实现方式，然后定义一个自己的 RequestID 。

<!--
在上述的文档中也包含了一些参考文档，介绍的还不错。
https://github.com/aperezdc/ulid-c
http://wjp2013.github.io/other/uuids-and-ulids/
-->

### 详解

简单介绍在使用时的详细内容。

#### 通用规范

1. 提供毫秒级的时间戳，大部分平台当调用 `time(NULL)` 函数时，实际上等价于 `gettimeofday()` ，而该函数会精确到微秒；
2. 打印线程ID，注意，非pthread中的线程ID，而是Linux操作系统中的线程ID，以方便与 OS 相关信息对应；
3. 打印代码行信息以方便问题定位，尤其是对于一些通用的错误，例如内存申请失败，可以方便定位，同时版本信息里应该带有仓库信息，例如tag；

#### 启动日志

启动时打印相关的软件版本信息 (Tag、CommitID等)，加载后的配置信息 (部分运行时的参数依赖)。

#### 内存申请失败

内存申请失败一般有几种情况，如果时关键位置没有内存，那么就直接退出，否则可以只打印日志，而保证基本功能的运行。

在打印日志时，同时打印申请内存的大小信息，例如：`log_error("out of memory, size %d.", size);` 。

#### 参数校验

函数入参校验失败时同时打印日志，并打印参数地址，例如：检测非空地址失败时打印 `log_error("invalid arguments (info, handler) = (%p, %p).", info, handler);` 。

#### API 三方

包含了标准库的调用、系统 API 、调用三方接口等等，应该将入参、错误码、错误信息打印，例如：

* 打开文件失败，`log_error("open file '%s' failed, %d:%s.", file, errno, strerror(errno));` 。
* 建立连接失败，`log_error("connect to host '%s:%d' failed, %d:%s.", host, port, errno, strerror(errno));` 。
* DB 操作失败，`log_error("execute SQL in host '%s:%d' with db '%s' failed, sql='%s', err='%s'.", host, port, db, sql, errmsg);` 。

另外，在配置文件解析时，如果发生了错误，需要将文件名、异常行号、原因打印，例如：`log_error("parse file '%s' failed, line:%d, %s.", file, line, errmsg);` 。

## 发布相关

### 版本管理

这里采用的是语义化的版本控制，详细可以参考 [https://semver.org](https://semver.org/lang/zh-CN/) 中的介绍。

{% highlight text %}
BootAgent-1.2.3-4.x86_64

主版本号
    做了不兼容的API修改，一般是增加了一些重大的特性修改。
次版本号
    向下兼容的功能性新增，也可以是针对一些特性的优化，但是不影响原有的接口。
修订号
    向下兼容的问题修正，一般是bugfix。
打包号
    在集成验证时打包的版本次数。
{% endhighlight %}



<!--
语义化版本控制
0.1.0-beta.78961a3
0.1.0-alpha

https://semver.org/
https://semver.org/lang/zh-CN/
-->

### 发布归档

{% highlight text %}
/BootAgent/BootAgent-1.2.3-1.tar.bz2                 源码包
/BootAgent/BootAgent-1.2.3-1.x86_64.rpm              RPM安装包
/BootAgent/BootAgent-debuginfo-1.2.3-1.x86_64.rpm    调试信息
{% endhighlight %}

### 域名管理


### 安装路径

对于 Agent 来说，其安装路径如下。

{% highlight text %}
----- 配置文件
/etc/cargo/gearman.conf

----- 二进制程序
/usr/sbin/gearman

----- 动态库
/usr/lib/cargo/nodus/cpu.so
/usr/lib/cargo/gearman/userinfo.so

----- 文档以及特定的配置文件等
/usr/share/cargo
/usr/share/cargo/nodus/types.db
/usr/share/doc/cargo/AUTHORS
/usr/share/doc/cargo/COPYING
/usr/share/doc/cargo/ChangeLog
/usr/share/doc/cargo/README
/usr/share/doc/cargo/TODO
/usr/share/man/man1/cargo.1.xz
/usr/share/man/man5/cargo-exec.5.xz

----- 日志以及PID文件
/var/log/cargo/cargo.log
/var/run/cargo/cargo.pid
{% endhighlight %}

## FAQ

### 主机标示

简单来说，就是如何唯一确认一台主机，包括了其从上架开始，到彻底报废整个的生命流程。这里会有一堆的问题需要确认，我们尽量以一种通用的方式。

#### 什么是一台主机？

通常意义上包括了物理机和虚拟机，这两种的生命周期相比来说要更长久一些，而像容器这种，其生命周期过短，在某些场景下 (例如监控) 以主机名方式标示更加合适，这样可以从逻辑上具有连续性。

除此之外，在广义上，可能还包括了一些嵌入式设备，例如物联网设备、主机上的带外设备等。

另外，需要说明的是，对于扩容、缩容来说，同样被认为是同一台主机。

#### 使用方案

目前仅考虑物理机和虚拟机的管理，以 UUID 作为唯一标示，并将配置信息保存在 `/etc/AgentMeta.con` 文件中。当然，该文件中除了 UUID 信息外还有其它的信息，对于 Linux 而言，UUID 的生成规则如下。

1. 首先读取 DMI 中保存的 UUID 信息，并更新上述文件。
2. 失败则随机生成 UUID 并更新上述文件。

#### UUID 更新

目前的策略简单粗暴，如果 UUID 被修改，无论以什么方式，都被认为是不同的机器。

也就是说，如果 UUID 被修改，主机监控数据可能出现断层，同一主机事件可能会出现丢失等等，所以 **不要随意修改UUID** 。

### 任务校验

任务校验可以有多种方法，例如对于 BootAgent ，其上报信息包含上层进行判断的大部分数据，完全不需要再增加额外的机制。

而对于其它类型的 Agent，例如 BasicAgent ，因为会在本地保存一些任务，如果由 Agent 触发，那么可能会出现与上层的不一致(例如上报时有任务在下发，实际上一致，可能会误判为不一致)，同时，如果批量上报可能会对上层造成过大的压力。

为此，建议由服务端发起任务校验，那么一些需要持久化的任务要保证其顺序性，也就是说不同的任务需要标识其并发性。例如，同步/异步命令可以并发；CRON配置则不可以。

## 其它

### 代码量统计

{% highlight text %}
----- 当前目录下 *.h *.c 文件数量
$ find -name '*.c' -or -name '*.c' | wc -l

----- 所有 *.h *.c 文件行数
$ find -name '*.c' -or -name '*.c' | xargs cat | wc -l

----- 过滤空行
$ find -name '*.c' -or -name '*.c' | xargs cat | grep -v ^$ | wc -l
{% endhighlight %}

{% highlight text %}
{% endhighlight %}
