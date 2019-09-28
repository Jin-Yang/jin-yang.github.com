---
title: MonitorAgent 实现
layout: project
comments: true
language: chinese
category: [misc]
keywords:
description:
---

负责周期性的采集数据，并将数据 PUSH 到服务器。

<!--
为了简单起见，队列没有采用超时机制，简单来说，也就是读写线程采用协作型。
对于读线程，不会设置超时时间，完全由采集线程控制，不过如果采集超时，会对读线程进行惩罚，简单来说就是增加采集的时间间隔。
对于写线程，每次发送时都会直接从队列中摘除，如果有多个写插件，那么只要有一个插件写成功那么就认为发送成功，失败时则会直接丢弃，不会维护队列进行重试。
  0. 每次接收到一个采集指标时，都会触发写操作，而且会通过单个线程写所有写插件。
  1. 每个写插件需要维护自己的队列进行重试，而且可以在线程中阻塞。
  2. 当所有的线程都阻塞时，那么就没有线程处理写请求了，此时就会阻塞到缓冲队列中。
  3. 要其每个写插件是可重入的。
-->

## 命令

### 常驻进程

也就是 `monitoragent` ，支持的详细参数可以直接通过 `monitoragent -h` 命令查看，常见操作如下。

#### 测试参数

一般用于启动之前检查参数或者插件是否正常。

##### 检查配置

通过 `-t` 参数检查配置文件是否合法，一般使用命令 `monitoragent -t -L test.log` ，也就是同时指定日志文件地址，防止日志被混淆。

如果成功则会显示 `Success` 否则 `Failed!!!` ，同时程序返回值分别为 `0` 和 `1` 。

假设要测试某个配置文件是否合法，可以通过如下命令测试。

{% highlight text %}
$ monitoragent -f -C monitoragent.server.conf -L server.log
{% endhighlight %}

##### 测试插件

也就是通过 `-T` 参数，同样一般使用命令 `monitoragent -T -L test.log` ，会将插件的一次采集打印出来，格式以及其中的示例如下。

{% highlight text %}
采集时间/周期 插件名(采集指标数/types.db指标数) 采集值1/类型1 采集值2/类型2 ... 采集值N/类型N

1511965574.110/10.000 load(3/3) 2.71/G 2.53/G 2.43/G
{% endhighlight %}

### 控制命令

也就是 `monitorctl` ，用于查看当前进程的运行状态，如果要查看与服务端的详细通讯过程，可以在执行命令之前设置环境变量，也就是 `MONITOR_TRACE=1;` ，常见示例如下。

{% highlight text %}
----- 查看当前加载了那些插件
$ MONITOR_TRACE=1 ./daemon/monitorctl plugin list

----- 删除某个插件
$ MONITOR_TRACE=1 ./daemon/monitorctl plugin remove load

----- 停止某个插件的数据采集
$ MONITOR_TRACE=1 ./daemon/monitorctl plugin stop load

----- 启动某个插件的数据采集
$ MONITOR_TRACE=1 ./daemon/monitorctl plugin start load
{% endhighlight %}

## 开发文档

### 回调函数

动态库加载完之后，会调用指定的 `module_register()` 函数，一般来说在该函数中会注册各种的回调函数，也就是如下介绍的内容。

注意，不建议在 `module_register()` 中执行太复杂的逻辑，如果返回的是非零，那么会打印报错信息加载失败。

初始化的顺序为。

{% highlight text %}
+--------+    +------+    +------+
| config |--->| init |--->| read |
+--------+    +------+    +------+
{% endhighlight %}

#### config

config 失败只会打印告警信息，但是不会进行其它处理，包括了简单和复杂两种回调函数，如果同时存在两个，会优先调用复杂的回调函数。

#### init

init 在插件加载完之后会调用该函数，失败会将插件状态转换为 FAILED 或者直接卸载(宏定义)。

#### read

用于周期性的采集数据，包括了两种。

#### write

主要用于上报插件的实现。

#### shutdown

在插件退出时进行相关的清理操作，如果返回非零值，则只打印报错信息。

#### 其它

注意，在 `init()` 和 `config()` 回调函数中只能设置参数，但是不能阻塞，例如不能调用 `sleep()`、`connect()` 之类的操作。

### 配置文件

除了一些常见的配置项之外，还支持类似 `<LoadPlugin>` `<Plugin>` 之类的配置块，其中前者包含了一些通用的配置项，例如采集间隔、加载 flag 、是否自动启动等等；而后者为插件的配置项。

### Pending 机制

为了支持一些异步的操作，例如重新加载、卸载、配置等等，提供了一种 Pending 请求的机制，包括了全局、插件两种纬度。

对应的特性包括了：

1. 允许多个线程处理，需要进行并发控制。包括了主进程(通过ctl命令行、重试)、Read线程(因为PROCESS、RUNNING状态)、Write线程(主要是与上层交互的接口，例如目前的CloudAgent)
2. 尽量减少用户感知。允许有多次重复的操作，例如多次 reload 操作。

<!--
注意，只有在将 struct plugin 从全局结构体中添加、删除时才会对全局进行加锁；而插件的状态转移只对插件结构进行加锁。在添加 Pending 请求时需要先获取 G 再获取 P 。

新增请求步骤：A) 创建请求结构体；B) 获取全局锁；C) 获取局部锁；D) 添加到Pending列表。
处理请求部署：A) 处理步骤操作；B) 获取局部锁修改状态；C) 判断是否需要删除；D) 需要删除，则释放局部锁，获取全局锁重新检查是否释放。

只有当插件中没有 Pending 任务时才可以将插件从全局的树中删除。

相关的上下文信息需要确保在进程调用过程中是安全的，例如上下文中包含了对插件指针的引用，需要确保在调度过程中不会发生引用指针异常的情况。

注意，当前含有 write、notify、flush 接口的插件是不支持卸载的。
-->

#### 测试用例

{% highlight text %}
===> COMMON
  * 启动时已经存在 PENDING 任务，直接返回 API_RC_PENDING。
  * 操作成功，并重复执行，返回 API_RC_DONE。

===> START
  *

===> STOP
  * 停止已经停止任务，直接返回 API_RC_DONE。

===> UNLOAD
  * 不支持有write钩子的插件，直接返回 API_RC_UNSUPPORT。

===> RELOAD
  * 不支持有write钩子的插件，直接返回 API_RC_UNSUPPORT。

{% endhighlight %}

## 插件

### 系统类

#### load

采集系统的负载，有两种方式：A) 调用 `getloadavg()` 函数；B) 读取 `/proc/loadavg` 文件。默认上报的是系统总的负载再处以 CPU 核数，也就是所谓的平均负载，也可以通过 `ReportAbsolute true` 配置项上报绝对值。

{% highlight text %}
LoadPlugin load
<Plugin load>
        ReportRelative    false
</Plugin>
{% endhighlight %}

#### uptime

系统的启动时间，单位是秒，有两种采集方式：A) 调用 `sysinfo()` 函数；B) 读取 `/proc/uptime` 文件。

### 测试用例

#### foobar

提供一个简单的测试用例 foobar ，可以通过 `MONITOR_FOOBAR_SLEEP` 环境变量设置在 read 函数中的休眠时间，可以通过如下方式测试。

{% highlight text %}
MONITOR_FOOBAR_SLEEP=60 ./wrapper.sh ./daemon/monitoragent -f
{% endhighlight %}

## FAQ

### 查看读函数

查看读插件的调用情况，会显示读线程以及各个读函数的调用过程，需要打开 DEBUG 日志。

{% highlight text %}
$ tail -f nodus.log | grep -E '([Rr]ead-function|[Rr]ead thread)'
{% endhighlight %}



<!--
https://www.circonus.com/

enum Type {
	COUNTER;
	GAUGE;
}
message Tag {
	required string Key;   // 长度限制为64
	required string Value; // 长度限制为1024
}
message Value {
	required Type type;
	required double value;
}
message Metric {
	optional string hostname;
	required string category;
	optional string indicator;
	optional string instance;

	required int timestamp;
	required int step;            // 包括了高精度ms和低精度s
	required repeated Value values;
	optional repeated Tag mtags;
}
message Data {
	optional string uuid;         // 可以优化为16Bytes的数值
	optional repeated Tag gtags;  // 全局的Tags列表
	repeated Metric metrics;
}

disk.usage  /
mysql.innodb.cache

保留tag关键字

host 主机名，需要按照通用的主机规范进行命名
service 服务名称，可以按照层级进行命名
component 组件或者微服务名称
device 设备类型，例如CPU、GPU、ARM、x86等
from/source 数据来源信息

所有的聚合、告警、展示都有与 tag 相关的内容。

以 DISK 为例，一个指标包含了如下内容：


namespace 命名空间，例如system

metric 指标名称包括三部分组成，通过 `.` 分隔，分别为：
* category 对应了一个插件，最大，例如disk、mysql；
* indicator 指标名称，可以是一个插件的内部子类，例如usage、innodb；
* instance 一般是一个示例

实际上大部分数据只会有一个 tags ，所以在结构体中会默认有一个 tag ，其它的则通过链表进行添加。
-->


1. 插件管理
	1.1 生命周期管理，包括了
2. 任务管理


针对多个插件的操作是可以并发执行的。

正在运行中的插件不会处理，而是添加到插件的 Pending 列表中。

start 启动采集任务
stop 停止采集任务，并执行清理操作但是不卸载

load 加载并根据配置文件判断是否自动运行
unload 卸载
reload 重新加载


INVALID 新建了对象，初始化并添加到了全局内存中。
LOADED 动态库已经加载成功

LAUNCH 已经加载并添加到了 heap 中，但是还没有


可能会在两个线程中调用


在加载时的流程：

1. 在指定的目录下遍历所有的配置文件，并解析，如果有 LoadPlugin 指令则会去尝试加载。
1. 新建 plugin 内存对象，并添加到全局对象中，此时状态为 INVALID 。
2. 调用 plugin_do_load() 函数执行真正的加载，这里会调用 `module_register()`，成功之后为 LOADED 。
3. 插件加载完成之后会调用 config 函数，注意返回指非 0 只会打印告警信息，而不会进行其它处理。
3. 所有的插件加载完成之后，如果存在 init 函数则统一再调用 init 函数。
4. 如果插件配置了自动启动，那么在加载完成之后会开始周期调用 read 函数。

1. 插件管理。任务、插件先后顺序。
2. 自监控能力。脚本失败。
3. 错误信息统一。
4. 降龙。
5. 消息上报。提供多通道上报能力。
6. 调度状态统计，是否有延迟。
7. 消息通道做成通用的。

int config_read(const char *plugin) 目前是cf_read()函数

读取指定目录下的配置文件，并尝试加载

plugin_do_load(struct plugin *plg, int flags) do_plugin_load()

真正加载动态库，执行步骤为：

1. 扫描指定目录 (目录无权限访问) 找到对应的动态库，如果不存在则报错 ENOENT 。
2. 加载动态库 (非法动态库)，查找指定的 `module_register()` 函数 (无函数非法) 。
3. 执行 `module_register()` 函数，并检查其返回结果，如果失败则会直接关闭打开的动态库。

TODO:

在调用 `module_register()` 函数时每次都会在全局变量中搜索，是否可以通过 ctx 来减少查找的次数？
明确 context 的真正用途，能否直接干掉。

struct request {
	struct request *next;
	int flags; // 标示是否正在处理
};

struct plugin {
	char name[64];
	void *handle; // 动态库的地址
	struct json *cfg; // 配置项
	struct context *ctx; // 上下文信息
	struct request *req; // 当前请求

	init
	config
	read
	write
	shutdown
};

如果需要修改采集周期，那么每次都需要触发


start 启动采集任务
stop 停止采集任务，并执行清理操作但是不卸载

load 加载并根据配置文件判断是否自动运行
unload 卸载
reload 重新加载
config 修改配置
        在LAUNCH、PROCESS、RUNNING状态



Interface Description Language, IDL 接口描述语言

Python V2 VS. V3 版本对比
https://sebastianraschka.com/Articles/2014_python_2_3_key_diff.html


初始化过程：

1. 新建 plugin tree、初始化 context、新建 exec 任务树等内存结构，设置 PR_SET_CHILD_SUBREAPER；
2. 启动SQLite，主要是创建表、设置busy hook、设置同步模式；
3. 设置常见的信号处理函数，包括了 SIGCHLD、SIGPIPE(设置为ignore)；
4. 通过读取配置文件加载所有的动态库cf_read() (TODO: 文件解析统一)，主要是加载、然后在 module_register() 中注册回调函数；
5. 然后通过db启动所有的任务。

在 Linux 内核中保存了一个通用的链表头文件 [linux/list.h](https://github.com/torvalds/linux/blob/master/include/linux/list.h) ，

注意access()会对文件路径上的权限进行检查，但是不会判断是否有 root 中的 CAP_FOWNER 权限。
https://blog.csdn.net/gmq_syy/article/details/73793721

1. 将解析配置文件和发送配置信息分开。
2. 启动时可以不需要对操作进行加锁。
3. 对于API的请求，真正处理插件请求的实际上只有一个线程，通过pending机制进行类似的加锁。



COMMON:
1. 每个插件只允许最大有20个Pending的任务。
2. 转交给global处理时的方式。

1. 配置文件不存在。则直接使用默认的配置(不自动启动，间隔为 10 秒)。DONE
2. 配置文件存在。DONE
3. 配置文件存在但是非法。不支持配置项只打印警告信息。DONE
4. 动态库不存在。添加到自动重试列表里面。DONE
4. 超过了Pending列表数之后应该直接返回失败。
5. 连续多次测试

LOAD
1. 已经加载。直接返回成功。DONE


结束的事件一般有两种，可以进行重试的，以及致命错误：

1. 致命错误。
2. 可以重试。

在非主线程中，一次仅处理一个操作，如果有多于一个的则交由主进程进行处理。

启动

1. 检查是否存在 read_func 指针。

信号转换为同步的是否就是信号安全的了？

注意，模板和参数应该是分开的，这样可以检查例如系统监控是否全网绑定，但是允许阈值设置不同。在模板修改之后，同时可以将模板修改的内容向相关的责任人推送消息。
在修改、添加、绑定告警的时候，应该向相关的责任人发送消息通知，同时为了防止消息泛滥，允许责任人进行过滤。

插件名称支持分组，也就是说允许一个动态库中注册多个函数，此时的名称为 `mysql.innodb` ，

enum Type {
	COUNTER;
	GAUGE;
}
message Tag {
	required string Key;   // 长度限制为64
	required string Value; // 长度限制为1024
}
message Value {
	required Type type;
	required double value;
}
message Metric {
	optional string hostname;
	required string category;
	optional string indicator;
	optional string instance;

	required int timestamp;
	required int step;            // 包括了高精度ms和低精度s
	required repeated Value values;
	optional repeated Tag mtags;
}
message Data {
	optional string uuid;         // 可以优化为16Bytes的数值
	optional repeated Tag gtags;  // 全局的Tags列表
	repeated Metric metrics;
}

disk.usage  /
mysql.innodb.cache

保留tag关键字

host 主机名，需要按照通用的主机规范进行命名
service 服务名称，可以按照层级进行命名
component 组件或者微服务名称
device 设备类型，例如CPU、GPU、ARM、x86等
from/source 数据来源信息

所有的聚合、告警、展示都有与 tag 相关的内容。

以 DISK 为例，一个指标包含了如下内容：

namespace 命名空间，例如system

metric 指标名称包括三部分组成，通过 `.` 分隔，分别为：
* category 对应了一个插件，最大，例如disk、mysql；
* indicator 指标名称，可以是一个插件的内部子类，例如usage、innodb；
* instance 一般是一个示例

实际上大部分数据只会有一个 tags ，所以在结构体中会默认有一个 tag ，其它的则通过链表进行添加。

1. 插件管理
	1.1 生命周期管理，包括了
2. 任务管理


### 回调函数类型

插件加载完成之后会调用固定的函数 `module_register()` ，在该函数中会注册各种的回调函数，也就是如下介绍的内容。

注意，不建议在 `module_register()` 中执行太复杂的逻辑，如果返回的是非 0 ，那么会打印报错信息直接退出。TODO: 目前直接忽略，卸载插件。

init 和 config 回调函数中只能设置参数，但是不能阻塞，例如不能调用 sleep()、connect() 等操作。




init 在插件加载完之后会调用该函数，失败会将插件状态转换为 FAILED 或者直接卸载(宏定义)。 (TODO: init失败后卸载插件)
config 失败只会打印告警信息，但是不会进行其它处理，包括了简单和复杂两种回调。
read 用于周期性的采集数据。
write 主要用于上报插件的实现。
shutdown 在插件退出时进行相关的清理操作。

针对多个插件的操作是可以并发执行的。

正在运行中的插件不会处理，而是添加到插件的 Pending 列表中。

start 启动采集任务
stop 停止采集任务，并执行清理操作但是不卸载

load 加载并根据配置文件判断是否自动运行
unload 卸载
reload 重新加载

INVALID 新建了对象，初始化并添加到了全局内存中。
LOADED 动态库已经加载成功

LAUNCH 已经加载并添加到了 heap 中，但是还没有

可能会在两个线程中调用
plugin_reload()

API_DONE 已经处理完成
API_PENDING 已经有任务了，将添加到pending表中
API_CONT 交给另外的线程进行处理

## reload
1. 插件不存在。

./daemon/cloudagent_monitorctl plugin reload foobar

在加载时的流程：

1. 在指定的目录下遍历所有的配置文件，并解析，如果有 LoadPlugin 指令则会去尝试加载。
1. 新建 plugin 内存对象，并添加到全局对象中，此时状态为 INVALID 。
2. 调用 plugin_do_load() 函数执行真正的加载，这里会调用 `module_register()`，成功之后为 LOADED 。
3. 插件加载完成之后会调用 config 函数，注意返回指非 0 只会打印告警信息，而不会进行其它处理。
3. 所有的插件加载完成之后，如果存在 init 函数则统一再调用 init 函数。
4. 如果插件配置了自动启动，那么在加载完成之后会开始周期调用 read 函数。


实际上我们分了两类的接口：A) 对外提供的 API，一般是在启动完成之后使用，此时实际上会通过 pending 机制变相加锁；B) 在启动时明确不会出现冲突，对应了 do_XXX 之类的函数，此时只做具体的工作，不需要对全局进行加锁。

为了简化，开始只提供第一种的采集方式。

int config_read(const char *plugin, int flag) 目前是cf_read()函数

读取指定目录下的配置文件，并尝试加载

plugin_do_load(struct plugin *plg, int flags) do_plugin_load()

真正加载动态库，执行步骤为：

1. 扫描指定目录 (目录无权限访问) 找到对应的动态库，如果不存在则报错 ENOENT 。
2. 加载动态库 (非法动态库)，查找指定的 `module_register()` 函数 (无函数非法) 。
3. 执行 `module_register()` 函数，并检查其返回结果，如果失败则会直接关闭打开的动态库。

TODO:

在调用 `module_register()` 函数时每次都会在全局变量中搜索，是否可以通过 ctx 来减少查找的次数？
明确 context 的真正用途，能否直接干掉。

struct request {
	struct request *next;
	int flags; // 标示是否正在处理
};

struct plugin {
	char name[64];
	void *handle; // 动态库的地址
	struct json *cfg; // 配置项
	struct context *ctx; // 上下文信息
	struct request *req; // 当前请求

	init
	config
	read
	write
	shutdown
};

如果需要修改采集周期，那么每次都需要触发

+---------+           +--------+              +------+             +-------+           +--------+           +---------+           +---------+
| INVALID |--[load]-->| LOADED |--[init OK]-->| IDLE |--[config]-->| READY |--[heap]-->| LAUNCH |--[wait]-->| PROCESS |--[read]-->| RUNNING |
+---------+           +--------+              +------+             +-------+           +--------+           +---------+           +---------+
     |                                                                                      |                    |                      |
 [init FA]                                                                               <stop>               <stop>                 <stop>
     |                                                                                      |                    |                      |
     |                            +--------+                     +--------+                 |                    |                      |
     `--------------------------->| FAILED |                     | STOPED |<------------------------------------------------------------'
                                  +--------+                     +--------+

全局的插件结构体中保留了相关的信息。

保留的列表包括了：

write
flush
notify
pending

为了防止由于不同的状态与命令发生冲突，例如多次修改配置(某个可能丢失)、多次重启请求，真正在判断处理的时候是在：

1. 转换为 READY 时判断。
2. 在 LAUNCH PROCESS RUNNING 中等待。
3. 在 FAILED STOPED 中直接处理。

原则：

1. 插件的数据发送之后已经包含了所有信息，不再依赖原有插件的函数、数据等，也就意味着原有的插件可以随时被卸载。
2. 状态迁移过程中加锁的时间很短，而且不能失败。对于创建内存等操作一般都是提前申请并初始化完成。

一个线程加锁释放后如何通知另外被阻塞的线程执行？
一个线程加锁释放后何时开始调度执行另外被阻塞的线程？

那个线程在添加 request 的时候的链表为空，那个线程进行处理。

转换的时候用科学计数法

每次新增任务

start 启动采集任务
	1. 获取全局的plugins_lock；
	2. 检查插件是否存在，不存在则添加全局pending；
	3. 是否有pending的请求，可能会是正在加载，有则添加到pending列表中；
	4. 是否支持read_func，否则直接退出；
	5. 获取read_lock锁，直接处理(处理速度很快);
	6. 释放全局的plugins_lock；
stop 停止采集任务，并执行清理操作但是不卸载
	1. 获取全局的plugins_lock；
	2. 检查插件是否存在，不存在则直接返回成功；
	3. 检查是否有pending任务，如果有则添加；
	4. 是否支持read_func，否则直接退出；
	5. 获取read_lock锁，直接处理(处理速度很快)；
	6. 释放全局的plugins_lock；

load 加载并根据配置文件判断是否自动运行
	1. 获取全局的plugins_lock；
	2. 检查插件是否存在，不存在则新建并添加到plugins_loaded树中；
	3. 是否有pending的请求，有则添加到pending列表中；
	4. 释放全局的plugins_lock；
	5. 开始解析配置文件并加载。参考注 <1>

unload 卸载
	1. 获取全局的plugins_lock；
	2. 检查插件是否存在，不存在则直接返回成功；
	3. 是否有pending的请求，有则添加到pending列表中；
	4. 释放全局的plugins_lock；
	5. <<<< 开始执行卸载
	6.

reload 重新加载
	1. 获取全局的plugins_lock；
	3. 是否有pending的请求，有则添加到pending列表中；
	4. 释放全局的plugins_lock；
	5. 开始执行卸载操作。

config 修改配置
	在LAUNCH、PROCESS、RUNNING状态

<1> 注意，因为已经添加了一个pending请求，也就是意味着该线程会一直持有该插件的执行权限。

目前只使用了一个全局锁，并使用pending做单线程调用的限制，

测试场景：

### reload

1. 插件正在运行。
   A) INVALID LOADED IDLE 状态，那么必然存在着一个 Pending 请求，此时直接添加到 Pending 的末尾。
   B) READY LAUNCH PROCESS RUNNING STOPED FAILED 此时可能会存在 Pending 请求，有则直接添加到 Pending 的末尾。

### stop

### 其它

1. 设置了一个很大的采集时间间隔(24小时)，然后新增多个插件，或者修改采集间隔。

libev 优化


{% highlight text %}
+---------+           +--------+              +------+             +-------+           +--------+           +---------+           +---------+
| INVALID |--[load]-->| LOADED |--[init OK]-->| IDLE |--[config]-->| READY |--[heap]-->| LAUNCH |--[wait]-->| PROCESS |--[read]-->| RUNNING |
+---------+           +--------+              +------+             +-------+           +--------+           +---------+           +---------+
     |                                                                                      |                    |                      |
 [init FA]                                                                               <stop>               <stop>                 <stop>
     |                                                                                      |                    |                      |
     |                                                                                      |                    `----------------------'
     |                                                                                      |                                |
     |                                                                                      |                                V
     |                                                                                      |                            +--------+
     |                                                                                      |                            | REMOVE |
     |                                                                                      |                            +--------+
     |                                                                                      |                                           |
     |                                                                                      |                    |                      |
     |                                                                                      |                    |                      |
     |                                                                                      |                    |                      |
     |                                                                                      |                    |                      |
     |                                                                                      |                    |                      |
     |                            +--------+                     +--------+                 |                    |                      |
     `--------------------------->| FAILED |                     | STOPED |<------------------------------------------------------------'
                                  +--------+                     +--------+
{% endhighlight %}





如果在多线程中调用 `dlopen()` 打开动态库，在通过 valgrind 测试时会有报错，目前来看应该是误报，暂时忽略。

<!-- gcc -Wl,--no-as-needed -g -o stuff  main.c -ldl -lpthread -->

<!--
==23267== 21 bytes in 1 blocks are still reachable in loss record 1 of 5
==23267==    at 0x4C29C23: malloc (vg_replace_malloc.c:299)
==23267==    by 0x4019F39: strdup (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4017183: _dl_load_cache_lookup (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4008A69: _dl_map_object (in /usr/lib64/ld-2.17.so)
==23267==    by 0x40143E3: dl_open_worker (in /usr/lib64/ld-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4013CCA: _dl_open (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695081: do_dlopen (in /usr/lib64/libc-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695141: __libc_dlopen_mode (in /usr/lib64/libc-2.17.so)
==23267==    by 0x5049E52: pthread_cancel_init (in /usr/lib64/libpthread-2.17.so)
==23267==    by 0x504A01B: _Unwind_ForcedUnwind (in /usr/lib64/libpthread-2.17.so)
==23267==
{
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   fun:strdup
   fun:_dl_load_cache_lookup
   fun:_dl_map_object
   fun:dl_open_worker
   fun:_dl_catch_error
   fun:_dl_open
   fun:do_dlopen
   fun:_dl_catch_error
   fun:__libc_dlopen_mode
   fun:pthread_cancel_init
   fun:_Unwind_ForcedUnwind
}
==23267== 21 bytes in 1 blocks are still reachable in loss record 2 of 5
==23267==    at 0x4C29C23: malloc (vg_replace_malloc.c:299)
==23267==    by 0x400B603: _dl_new_object (in /usr/lib64/ld-2.17.so)
==23267==    by 0x40062F3: _dl_map_object_from_fd (in /usr/lib64/ld-2.17.so)
==23267==    by 0x40087B7: _dl_map_object (in /usr/lib64/ld-2.17.so)
==23267==    by 0x40143E3: dl_open_worker (in /usr/lib64/ld-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4013CCA: _dl_open (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695081: do_dlopen (in /usr/lib64/libc-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695141: __libc_dlopen_mode (in /usr/lib64/libc-2.17.so)
==23267==    by 0x5049E52: pthread_cancel_init (in /usr/lib64/libpthread-2.17.so)
==23267==    by 0x504A01B: _Unwind_ForcedUnwind (in /usr/lib64/libpthread-2.17.so)
==23267==
{
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   fun:_dl_new_object
   fun:_dl_map_object_from_fd
   fun:_dl_map_object
   fun:dl_open_worker
   fun:_dl_catch_error
   fun:_dl_open
   fun:do_dlopen
   fun:_dl_catch_error
   fun:__libc_dlopen_mode
   fun:pthread_cancel_init
   fun:_Unwind_ForcedUnwind
}
==23267== 360 bytes in 1 blocks are still reachable in loss record 4 of 5
==23267==    at 0x4C2B9B5: calloc (vg_replace_malloc.c:711)
==23267==    by 0x40112AD: _dl_check_map_versions (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4014720: dl_open_worker (in /usr/lib64/ld-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x4013CCA: _dl_open (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695081: do_dlopen (in /usr/lib64/libc-2.17.so)
==23267==    by 0x400F913: _dl_catch_error (in /usr/lib64/ld-2.17.so)
==23267==    by 0x5695141: __libc_dlopen_mode (in /usr/lib64/libc-2.17.so)
==23267==    by 0x5049E52: pthread_cancel_init (in /usr/lib64/libpthread-2.17.so)
==23267==    by 0x504A01B: _Unwind_ForcedUnwind (in /usr/lib64/libpthread-2.17.so)
==23267==    by 0x5048331: __pthread_unwind (in /usr/lib64/libpthread-2.17.so)
==23267==    by 0x5042E76: pthread_exit (in /usr/lib64/libpthread-2.17.so)
==23267==
{
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:calloc
   fun:_dl_check_map_versions
   fun:dl_open_worker
   fun:_dl_catch_error
   fun:_dl_open
   fun:do_dlopen
   fun:_dl_catch_error
   fun:__libc_dlopen_mode
   fun:pthread_cancel_init
   fun:_Unwind_ForcedUnwind
   fun:__pthread_unwind
   fun:pthread_exit
}

###################################
## Histogram
###################################

最简单的是指定上下限，然后进行统计。

Brubeck 实际上没有使用直方图，而是将所有的数据保存下来，当需要进行采样时会进行快排，在采样时才会真正的执行计算。

Statsite 中同样固定长度，但是对于百分位数的计算，可以在丢失一部分精度的前提下使用 Cormode-Muthukrishnan Algorithm 算法。

目前实现的方式是，固定整体的 `bins` 数量 (1000) ，如果超过范围大小，那么直接调整 `bin_width` 的宽度以适应新的范围，因为需要将原有的统计信息重新添加到分组中，有一定的计算量，应该尽量避免。

注意，使用前最好通过 `bin_width` 明确好当前直方图支持的范围，如果当超过支持范围后，当执行 `bin_width` 的指数扩展时，可能会丢失原有的精度。

High Dynamic Range (HDR) Histogram
http://hdrhistogram.github.io/HdrHistogram/
https://github.com/sorenmacbeth/streaming-papers
https://github.com/powturbo/TurboHist

-->


{% highlight text %}
{% endhighlight %}
