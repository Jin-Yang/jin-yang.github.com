---
title: BootAgent 实现
layout: project
language: chinese
---

当管理的机器达到上万的数量级时，即使 Agent 的部署成功率能达到 99% 那也会导致有几百台的机器离线，如果 Agent 频繁变更可能会导致离线机器更多。

BootAgent 就是为了管理各个 Agent ，同时为了保证机制简单，可以使功能快速稳定。

## 简介

用来管理最基本的 Agent，提供基本的管理、监控功能。通过最简单的方式与服务端进行通讯，也就是利用 `HTTP/1.1` 短链接与服务端通讯，只提供简单的 PUSH 机制，由 BootAgent 主动发起。

简单来说，尽量保证 BootAgent 功能的简单稳定，通过尽量少的资源实现其功能，这也同时意味着，很多的功能无法保证其时效性。

通过单个二进制文件运行，配置使用命令行参数，也可以使用配置文件。当然安装包会包含了很多辅助程序，可以用来调测等。

### 限制

如上所述，在实现时尽量做到简化，所以必然会带来很多的限制，简单列举如下：

1. 与服务端通讯的发送、接收缓存默认是 64K 大小，通过 `CLI_SNDBUF_SIZE` `CLI_RCVBUF_SIZE` 设置。
2. 客户端会周期将状态上报到服务端，默认 10min (可配 3min~60min )，通过 `PROJECT_CLIENT_INTERVAL` 设置。
3. 如果在一个周期内数据上报失败，那么会减少到 3min (可配 1min~60min) 重试上报，可通过 `PROJECT_RETRY_DELAY` 设置。

注意，客户端上报周期并非严格设置，可能会在连接服务端失败时导致延迟。

### 实现功能

其中主要功能点包括了。

{% highlight text %}
client.c    作为客户端与服务端进行通讯
process.c   提供异步进程的实现
{% endhighlight %}

{% highlight text %}
1. 子进程管理。
   1.1 任务接口，包括了子Agent的安装、卸载、启动、停止。
   1.2 状态管理。
       自动拉起，可以配置是否在启动时拉起进程。
       状态检查、资源限制。

2. 任务管理。
   2.1 修改配置。

3. 事件上报机制。
   3.1 子进程异常。

4. 状态信息上报。与BootAgent相关的状态信息。
   4.1 任务信息。接收到的任务数、执行成功数、执行失败数、忽略执行数(任务已经存在)。

5. 其它
   5.1 元数据信息MetaFile，保存了一些元信息，例如AgentSN、Tags、ServerList等。
{% endhighlight %}

上述的子进程管理，支持基于 tag 的批量升级，同时允许按照比例升级部分 (用于灰度验证) 。

如果任务执行失败，只能等待下次 BootAgent 上报数据时处理。

## 1. 子进程管理

默认会在配置目录下保存相关的配置，配置文件的后缀需要确保是 `*.json`，默认配置目录可以通过 `BootAgent -h` 查看。在管理进程时，会将配置的名称作为唯一标示，如果有重复则会忽略后面的配置。

另外，配置文件的名称一般要求与文件中 `name` 字段的名称保持一致(非强制)，其中名字允许 `[A-Za-z0-9_-]` ，不能出现 `.` ，长度不能超过 `50` 字节(`AGT_NAME_LEN_MAX`)。

其中的配置文件示例如下，文件名为 `BasicAgent.json` ，文件最大为 16K(`AGT_CONFIG_MAXSIZE`)。

{% highlight text %}
{
        "name": "BasicAgent",                          # 必选，子Agent的名称
        "version": "1.2.3",                            # 必选，子Agent的版本号
        "exec": "/bin/bash /usr/bin/gearman",          # 必选，注意，命令行必须是绝对路径，用来匹配进程，最长为255

        "type": "simple",                              # 可选，以不同的方式启动 (默认是simple)
                                                       #       simple 以fork+exec方式运行，作为子进程
                                                       #       daemon 子进程会fork子进程，也就是常驻进程

        "pidfile": "/var/run/cargo/gearman.pid",       # 对于fork必选，每行一条记录，不过只会检查第一行
        "user": "root",                                # 可选，默认是root
        "group": "root",
        "envs": {                                      # 可选
                "PATH": "/usr/bin:/usr/local/bin",
                "LANG": "en_US.UTF-8"
        },

        "startsecs": 60,                               # 可选，启动多久之后认为正常，其中fork默认为60
        "autostart": false,                            # 可选，是否在安装或者启动BootAgent时自动拉起该进程，默认false
	"single": true,                                # 可选，在启动前是否允许有多个进程存在 (默认是true)
                                                       #       true 每次启动检查进程是否已经启动
                                                       #       false 启动时直接拉起，如果需要单实例则由子进程自己控制

        "stopsecs": 60,                                # 可选，超过多久之后直接向进程发送SIGKILL
        "stopit": false,                               # 可选，进程退出是否杀死进程，默认是false
        "stopasgroup": true,                           # 可选，在kill进程时以组方式
        "stopsignal": "SIGTERM"                        # 可选，在退出时向进程发送的信号，默认为SIGTERM
                                                       #       支持信号TERM HUP INT QUIT KILL USR1 USR2

        "restartsecs": 20,                             # 可选，失败之后启动前sleep时间
        "retries": 3,                                  # 可选，重试次数，包括了启动、异常退出等状态时的重试
        "exitcodes": "0,9",                            # 可选，认为正常的退出码，不会再重启，只支持正值
        "autorestart": "yes",                          # 可选，失败之后的启动方式，默认或者非法是yes
                                                       #       no 不再重启，无论退出的状态是什么
                                                       #       yes 一直尝试重启，同样无论退出的状态是什么
                                                       #       unexpect 只有退出码不在exitcode中时才会重启

        "cgroup": {                                    # 可选，会通过cgroup进行资源限制
                "CPU": 20,                             # CPU资源限制，单位是%
                "MEM": 3000                            # 内存限制，单位是KB
        },

	"limits": {                                    # 可选，BA会周期性的检查，超过资源限制后kill进程
		"interval":60,                         # 检查间隔
		"enable":true,                         # 启动检查，可以判断进程是否存在
		"CPU":30,                              # CPU使用率，单位%
		"MEM":102400,                          # RSS内存，单位KB
		"FDS":1000                             # 文件描述符
	},

	"check": {                                     # 可选，健康检查，由BA向子Agent发送请求，超时时间是70%*interval
		"interval":60,                         # 检查间隔
		"path":"/usr/run/BootAgent.sock",      # 目前只支持Unix Domain Socket
		"match":"regex:success"                # 对返回信息进行检查，可以使用正则(regex)或字符串(string)
	},

	"heartbeat": {                                 # 可选，心跳检查，由子Agent向BA发送请求
		"interval":60,                         # 心跳间隔，默认是1分钟
		"match":"regex:success"                # 对上报报文的"message"字段进行匹配
	},

        "checkall": true,                              # 可选，当所有都失败时才认为异常
        "checks": 3,                                   # 可选，如上的检查超过这里的设置次数后认为异常
        "checksecs": 20,                               # 可选，健康检查默认时间
}
{% endhighlight %}

注意，在匹配时会检查 `/proc/<PID>/exe` 中的二进制文件路径，需要保证与配置文件 `exec` 中的第一个参数相同。

### 处理流程

#### 启动

每次 BootAgent 启动时，会根据 `autostart` 确认是否启动子进程，如果 `mode` 是 `single` 那么会根据 `/proc/<PID>/exec` 检查进程是否已经启动，如果已经启动，则会更新状态。

### 进程类型

进程分为了 `simple` 和 `fork` 两种方式，使用方式如下。

#### simple

作为 BootAgent 的子进程存在，此时在父进程退出时，子进程同样会退出。

这也就意味着，如果子进程退出那么就认为进程异常。

#### fork

BootAgent 在调用子 Agent 之后，子 Agent 会作为 Daemon 进程存在，所以启动流程会比较复杂。

1. 调用子进程后，一般子进程会再次执行 fork ，也就是真正执行业务逻辑的进程，所以会忽略子进程退出；
2. 在 `startsecs` 之后检查并刷新进程信息，主要是 PID；

注意，如果进程的启动时间超过了 `startsecs` 设置，那么会在该进程退出后再次刷新。

### 常见场景

简单列举一些常见的使用场景。

#### 一直尝试拉起

直接忽略退出状态，一直尝试重新拉起，参数设置包括 `"autorestart": "yes"`、`"restartsecs": 20`，其中后者主要是为了防止重复拉起导致异常，例如由于 cgroup OOM 。

#### 注意事项

子进程需要处理好部分状态，其中部分场景列举如下：

##### 退出异常

如果 BootAgent 接收到了停止命令，并向子进程发送了退出信号，而子进程还没有退出；此时，如果 BootAgent 又收到启动命令，那么会立即再启动一个进程。

所以，子进程需要处理好该场景，例如可以只保留一个进程。同时，也就意味着需要注意 `startsecs` 参数的设置。

## 2. 健康检查

用来检查子进程是否正常，总共有三种方式，可以选择其中一种，如果配置了多种检查方式，可以选择是都失败时认为异常，还是只要当单个检查失败就认为异常。

### 资源检查

主要是检查子进程的 CPU、内存、句柄等信息，也可以检查进程是否存在。

{% highlight text %}
"limits": {
	"interval":60,
	"enable":true,
	"CPU":30,
	"MEM":102400,
	"FDS":1000
}
{% endhighlight %}

### 健康度

通过配置的健康检查端口判断，目前只支持 Unix Domain Socket，使用简单的换行进行分割。

{% highlight text %}
"check": {
	"interval":60,
	"path":"/usr/run/BootAgent.sock",
	"match":"regex:success"
}
{% endhighlight %}

会发送如下内容的报文，并以 `\r\n` 结束，同时，返回的结果会读取到 `\r\n` 处为止，目前最大长度为 1023 字节。

{% highlight text %}
{
	"gardian":"BootAgent",
	"method":"health",
}
{% endhighlight %}


### 心跳

由子 Agent 周期性的向 BootAgent 发送固定格式的心跳报文，报文同样需要以 `\r\n` 格式结束。

{% highlight text %}
"heartbeat": {
	"interval":60,
	"match":"regex:success"
}
{% endhighlight %}

上报的示例报文如下，其中 `agent` 标识了被管理 Agent 的名称以及版本信息，版本信息暂未使用。

{% highlight text %}
{
	"agent": {
		"name": "MiniAgent",      // 必选
		"version": "1.2.3"        // 可选
	},
	"message": "success"              // 必选
}
{% endhighlight %}

可以通过 `BOOTSOCK` 环境变量向该 Unix Domain Socket 发送心跳信息。


## 3. 任务管理

关于 BootAgent 的任务主要包含两类：A) BootAgent 自身的配置管理；B) 子 Agent 的相关操作，包括了安装、升级、重启等等。其中前者会立即执行，而后者会依次执行，另外，为了提升对子 Agent 的管理，会有限处理子 Agent 的停止、重启等动作，前提是在此之前没有相关的任务，例如升级。

BootAgent 不会持久化任务信息，因此实现的各种任务需要保证任务的可重入性，也就是可以重复执行多次不会带来逻辑上的问题。

另外，为了防止由于服务端的 BUG 引起任务多次重复执行，在 Agent 的内存中会保存一段时间的任务信息(`1小时 50个`)，当检查到有重复执行的任务时则直接忽略。

每次上报信息时会带上正在执行的任务信息，不过需要注意，如果报文非法(`id` 或者 `action` 不存在)、内存不足 那么不会返回相应的任务状态，此时就需要依赖上层重试，另外 `id` 不能超过 `TASK_ID_LEN_MAX(127)` 大小。

对于相同的 Agent 在底层只能有一个任务在处理，因为如果中间有一个任务执行失败，在底层很难决策如何处理接下来的任务。唯一一个比较特殊的是对子 Agent 进程的管理，如果添加了强制参数，那么会取消之前所有的任务，同时可能引起不一致。

最大的任务并发为 `TASK_NUM_MAX(50)`，注意，这里不包含配置管理任务，配置任务会立即处理。

### 任务状态

任务的基本状态处理流程如下。

{% highlight text %}

+--------+
|  INIT  |
+--------+

INIT   新建任务对象之后的状态。
VALID  任务已经添加到任务列表。
START  已经开始处理任务。
{% endhighlight %}

### 任务类型

包括了异步任务和同步任务，同步任务在下发解析时会立即开始执行(实现时一般为异步)，而异步任务则会串行执行，例如对于下载任务来说，为了防止带宽不可控。

**注意** 任务必须确保 `id` `action` `name` `version` 字段存在，如果 `id` 和 `action` 不存在，则会直接忽略任务；其中 `name` 和 `version` 用来确定操作的对象 (如果是全局配置不需要)。

{% highlight text %}
----- 安装任务，异步，用于第一次安装 注意，如果已经安装则尝试升级
{
	"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",  # 必选，任务ID信息，由服务端指定，客户端上报执行结果时会带上
	"action": "install",                           # 必选，指定任务操作
	"name": "BasicAgent",                          # 必选，需要操作的子Agent名称
	"version": "0.1.0",                            # 必选，操作子Agent的版本号
	"speed": 100,                                  # 可选，下载限速，默认是100，单位是KB/s
	"url": "ftp://server:port/BasicAgent/BasicAgent-0.1.0-0.x86_64.rpm",
	"checksum": "SHA256:4a34b8d7d3009bb9ef9475fbf33e7bbe4a1e8db003aefc578a241c2f51c2c2f2",

	"parameter": {                                 # 进程相关的参数，详见<子进程管理>中的配置参数
		"name": "BasicAgent",                  # 必选，注意需要与上述名称保持相同
		"version": "0.1.0",                    # 必选，类似，与上述安装包保持相同

		"envs": {
			"PATH": "/usr/bin;/usr/local/bin"
		}
	}
}

----- 升级任务，异步
{
	"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",
	"action": "upgrade",
	"name": "BasicAgent",                         # 必选，需要操作的子Agent名称
	"version": "1.2.3",                           # 必选，及其版本号
	"url": "ftp://server:port/BasicAgent/BasicAgent-0.1.0-0.x86_64.rpm",
	"checksum": "SHA256:4a34b8d7d3009bb9ef9475fbf33e7bbe4a1e8db003aefc578a241c2f51c2c2f2",
}

----- 卸载任务，异步
{
	"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",
	"action": "uninstall",
	"name": "BasicAgent",                         # 必选，需要操作的子Agent名称
	"version": "1.2.3",                           # 必选，及其版本号
	"option": "force",                            # 可选，是否尝试强制卸载
}

----- 进程操作，同步
{
	"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",
	"action": "program",
	"name": "BasicAgent",                         # 必选，需要操作的子Agent名称
	"version": "1.2.3",                           # 必选，及其版本号
	"operation": "restart",                       # 必选，对子进程的操作，包括了start、restart、stop
}

----- 配置相关，同步
{
	"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",
	"action": "config",
	"name": "BasicAgent",                         # 可选，如果配置的是子Agent需要与版本一块添加
	"version": "1.2.3",                           # 可选，及其版本号
	"svrlist": "192.168.9.1:1234,",               # 全局配置，服务端列表，不会修改默认的列表
	"step": 1200                                  # 全局配置，状态上报时间间隔，单位是秒，其范围为[60, 3600]
}
{% endhighlight %}

### 3. 事件上报




简单来说就是将 Agent 中发生的关键事件上报，其中某个事件通过 `AgentSN` `TimeStamp` `LocalID` 来标识，表示在那台主机上何时发生了什么事件，其中 `LocalID` 在进程重启后会重新开始计数。

{% highlight text %}
{
	"timestamp": 123456789,                         # 必选，事件发生的时间
	"category": "agent"                             # 必选，分类信息
	"subject": "MiniAgent"                          # 可选，事件发生的主体，标示那个进程异常
	"extra": "1.2.1-2.x86_64"                       # 可选，标识主体的附加信息，例如进程的版本号等
	"message": "resource overflow, CPU(10%)"        # 必选，详细日志信息，例如资源超过限制
}
{% endhighlight %}

包括了，BootAgent 检测到异常后发送的数据，以及服务端通过 BootAgent 上报的数据判断异常的事件：

1. 服务端健康检查。BootAgent 离线、进程 CPU MEM 异常。
2. 子进程信息。例如升级、重启、异常、健康检查失败等。


### 4. 状态信息上报

与 BootAgent 相关的状态信息。

{% highlight text %}
A. 任务状态信息统计
    tasknum     Agent在内存中缓存的任务数
    taskrun     已经执行的任务数，单调递增
    taskfail    执行任务失败数，单调递增
    taskpass    任务已经存在忽略的任务数，单调递增
{% endhighlight %}

<!--
### 其它

BootAgent 在启动时通过判断是否存在 `MetaFile` 来决定是否为第一次启动。
-->

## cgroup 管理

这里只针对 CPU、内存进行限制，简单来说，会新建一个管理所有 DEVOPS Agent 相关的分组，默认使用的是 `devops` ，也可以在启动的时候通过 `-C` 参数指定。

对于子 Agent 来说，如果使用了 cgroup 机制，则对应的 Agent 会存放到所对应分组目录下，分组名为 Agent 的名称，如果没有配置则会添加到 cgroup 的根目录下，也就是资源不做限制。

注意，此时各个子 Agent 指定的 CPU 使用率实际上是相对于总体而言，也就是说设置的是 `cpu.shares` 参数对应的值。

### 分组

默认 BootAgent 会使用 systemd 配置的 cgroup 组，而其它的 DevOps 工具则使用一个统一的 cgroup 组，这样可以将两者分开。

可以在启动的时候通过 `-G` 参数指定，也就是将 BootAgent 同样添加到 DevOps 组中。

### 实现

在实现时使用的是 libcgroup 库，启动顺序如下：

1. 在 cgroup 的根目录下创建对应的分组，并将 BootAgent 添加到分组中，默认不会限制资源使用。

## 文件下载

支持 FTP 以及 HTTP 下载，目前只支持单连接下载，而且只支持单进程。另外，也可以通过 yum 仓库实现。

### FTP 协议

支持 FTP 协议下载，关于详细的服务器配置方式可以查看 [FTP 服务简介]({{ site.production_url }}/post/network-service-ftp.html) 中的相关介绍，这里简单介绍其实现流程。

{% highlight text %}
1. 建立TCP链接，也就是命令管道。
2. USER + PASS 登陆。
3. TYPE I 使用二进制模式。
4. CWD dir 切换到目标目录下。

FTP 第一个链接
1. REST 1 测试是否支持分段下载。
2. SIZE filename.txt 获取文件的大小。

FTP 分组链接，可以并发下载
1. 同建立链接过程。
2. PASV 建立被动链接。
3. REST offset 发送本链接的文件偏移。
4. RETR filename.txt 开始下载。
5. 命令链接等待处理完成。
{% endhighlight %}

## 开发调试

### 编译工程

{% highlight text %}
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DBOOT_SERVER_ADDR="booter.cargo.com:8180,192.168.9.1:9090"
{% endhighlight %}

### 打包

在源码目录下可以直接通过如下命令进行打包，其中入参是版本号。

{% highlight text %}
$ ./contrib/package.sh BootAgent 1.2.1-1
{% endhighlight %}

### 安装路径

{% highlight text %}
----- 配置文件
/etc/cargo/BootAgent

----- 二进制程序
/usr/sbin/bootagent
/usr/sbin/bootctl

----- 基目录以及临时目录
/usr/local/cargo/BootAgent
/usr/local/cargo/BootAgent/tmp

----- 日志以及PID文件
/var/log/cargo/BootAgent.log
/var/run/cargo/BootAgent.pid
/var/run/cargo/BootAgent.sock
{% endhighlight %}

<!--
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
-->


### 命令行参数

#### bootagent

这个是主要的进程。

{% highlight text %}
-f
    前台运行，默认是后台运行。
-l <level>
    日志级别，可以
-L <path>
    日志保存路径。
-P <path>
    PIDFile 保存路径。
-S <list>
    服务器地址。
-C <name>
    cgroup 组名称。
-T
    用于测试当前的配置，而非真正运行，为了防止覆盖老日志，可以同时使用 -L 参数。
{% endhighlight %}

#### bootctl

一般用来调试、检查状态等使用，可以通过 `BOOTAGENT_TRACE` 环境变量来查看 `bootctl` 与 `bootagent` 的命令交互过程。

##### 检查当前配置

##### 设置日志级别

{% highlight text %}
export BOOTAGENT_TARCE=1; ./daemon/bootctl config setlog trace
{% endhighlight %}

##### 检查文件 SHA256

{% highlight text %}
./daemon/bootctl sha256 <YourFile>
{% endhighlight %}

### Systemd

在 CentOS 中也就是通过 systemd 管理进程，其中相关的 service 文件可以参考 `contrib/BootAgent.service` 中的配置内容。

{% highlight text %}
----- 更新配置
# cp BootAgent.service /usr/lib/systemd/system/BootAgent.service
# systemctl daemon-reload

----- 查看状态以及重启
# systemctl status BootAgent
# systemctl restart BootAgent

----- 配置后的cgroup会添加到如下目录
$ ls /sys/fs/cgroup/cpu/system.slice/BootAgent.service
$ ls /sys/fs/cgroup/memory/system.slice/BootAgent.service
{% endhighlight %}

### 测试

通过 Flask 实现了一个 `contrib/testmocksvr.py` 用来模拟服务端，可以发送任务。

## REST-API 接口

状态返回接口。

{% highlight text %}
{
	"status": 2,                            # 必选，0表示正常，非0则表示异常
	"message": "common error type",         # 可选，通常是一类通用的错误类型
	"cause": "unmarshal body failed"        # 可选，具体的错误信息
}
{% endhighlight %}

### 启动注册接口

{% highlight text %}
----- POST /api/v1/agent/register
{
	"hostname": "127.0.0.1",                            # 可以通过hostname命令查看
	"ipaddr": "127.0.0.1",                              # 在发送注册信息时与服务端建立连接的IP
	"agentsn": "bfdcc18c-b6b9-4725-9c47-37fd93dba5b6"   # 本地生成的UUID用来唯一标识一台主机
	"version": "1.2.4-x86_64",                          # BootAgent的版本号
	"feature": 7,                                       # 支持特性 0: cgroup
}

----- 返回信息
{
	"status": 0,
	"agentsn": "dcb886e9-04ed-41bb-9c12-4d2de12cd59b",  # 如果上层判断有冲突，则返回合法的AgentSN
	"tags": "svc=ecs,cmpt=DB",
}
{% endhighlight %}

当第一次启动时 (MetaFile不存在) 会向服务端注册，此时服务端可以根据配置返回一些配置信息，Agent 会尝试持久化到 MetaFile 文件中。

如果 Agent 因为某些原因失败，会尝试下次重试，此时上报的 AgentSN 可能会被修改，因此，只有当上报收到状态信息之后，才会被认为是合法的。

### 状态上报接口

{% highlight text %}
----- POST /api/v1/agent/status 上报当前Agent状态
{
	"uptime": 1234,                                         # 进程已经启动时间，单位秒
	"uptime": 1234,                                         # 进程已经启动时间，单位秒
	"timestamp": 1232,                                      # 上报时的时间戳
	"step": 600,                                            # 上报的时间间隔，可以做修改，默认10min
	"stats": {                                              # 当前BootAgent的状态统计信息
		"task": [1, 2, 3, 4],                           # 任务执行状态信息，分别为tasknum taskrun taskfail taskpass
	},
	"agents": [{                                            # 当前主机Agent信息
		"name": "BasicAgent",                           # Agent名称，需要保证唯一
		"version": "1.2.3",                             # Agent当前版本号
		"status": "stoped",                             # Agent的状态
		"uptime": 123,                                  # 已经运行时间
		"resource": {
			"cpu": 10,                              # % CPU使用率
			"rss": 100,                             # B 内存使用
			"fds": 10,                              # 文件句柄数
		},
		"cgroup": {                                     # 如果配置的cgroup组
			"cpu": 10,                              # % cgroup的CPU使用率
			"rss": 100,                             # B内存使用
		},
	}],
	"tasks": [{                                             # 任务执行状态
		"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",   # 任务ID
		"errcode": 0,                                   # 统一错误码信息
		"message": "success"                            # 返回信息
	}],
	"events": [{                                            # 本地发生的事件信息
		"timestamp": 123456789,                         # 发生的事件点
		"category": "agent"                             # 分类信息
		"message": "resource overflow, CPU(10%)"        # 详细日志信息，例如资源超过限制
	}],
}

----- 返回信息，可以包含请求
{
	"status": 2,                                           # 返回状态，0表示正常，目前直接忽略返回值
	"tasks": [{
		"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",  # 任务ID信息，由服务端指定，客户端上报执行结果时会带上
		"action": "install",                           # 指定任务操作，也开始是"upgrade" "uninstall" "restart" "stop"等
		"name": "BasicAgent",                          # 需要操作的子Agent名称
		"url": "ftp://server:port/BasicAgent/BasicAgent-0.1.0-0.x86_64.rpm",
		"checksum": "SHA256:4a34b8d7d3009bb9ef9475fbf33e7bbe4a1e8db003aefc578a241c2f51c2c2f2",
		"parameters": {                                      # 运行时的环境变量，一般在初次安装时配置，可以每次更新
			"PATH": "/usr/bin;/usr/local/bin",
			"limits": {                                    # 资源使用限制
				"CPU": "10%",
				"MEM": "20M"
			}
		},
	}, {
		"id": "ddc8a9b9-55bd-4ddd-b53d-47095ee19466",
		"action": "config",                            # 修改BootAgent的相关配置
		"serverList": "192.168.9.1:1234,",             # 服务端列表，不会修改默认的列表
		"interval": 1200,                              # 状态上报时间间隔，其范围为[60, 3600]
	}]
}

备注:
    Agent状态
      * stoped 主动停止
      * running 正在运行
      * fatal 多次重试后失败
      * killed 资源超限等原因被强制杀死
{% endhighlight %}

#### 其它

##### 下载地址

目前支持两种方式 `FTP`、`FILE`，其中后者仅用来测试。


<!--
BUG:
   1. 只使用测试时，会导致libev部分内存未释放。


1. 服务器地址配置
   1. 域名解析错误。

1. 进程管理
   1.1 DONE 配置文件中有多个 Name 相同的配置文件(最大为8K)。后续的配置文件解析时会报错，注意，读取时不保证顺序。
   1.2 执行用户相关(主进程需要以ROOT运行)。
       1.2.0 用户存在。以指定用户执行。
       1.2.1 用户不存在。直接报错退出。
       1.2.2 用户没有指定。默认通过root执行。
       1.2.3 属组非默认。指定属组执行。
   1.3 进程资源使用检查。
`
2. 通讯管道异常
   2.1 服务端未开启，链接失败。
       2.1.1 本地子网段(127.0.0.1~20)，路由正常，访问IP无监听服务，此时TCP交互时会返回RST，也就是立即返回 Connection refused.
       2.1.2 非本地子网(192.168.9.1~20)，此时会由于路由不通导致异常，发送SYN报文之后无响应，对于Linux来说2min报错 Connection timed out.
       2.1.3 网络不可达(227.0.0.1~20)，一般是由于路由不可达，此时报错 Network is unreachable.
   2.2 建立链接成功，数据发送，但是服务端无响应，客户端超时后重联。CLI_READ_TIMEOUT
   2.3 从服务端读取的数据格式异常，关闭重试。

   2.2 服务端返回的报文格式异常。

3. cgroup 资源隔离
   3.1 DONE 如果有 CPU MEM 不存在会修复两者所在目录。
   3.2 DONE 首先会解析任务的配置文件，启动时如果任务不存在则会删除。
   3.2 CPU MEM 资源限制。

4. 任务管理
   4.1 配置任务。

域名解析失败
-->

## FAQ

#### Agent 重启策略

每隔 60s 检查一次，如果进程异常退出在 3min 内不会重启，如果在 15min 内异常则标记一次异常事件，当连续异常超过 3 次后则认为程序异常，不再尝试重启。

#### 临时目录

主要有两个用途：A) 文件原子操作；B) 临时目录，例如下载文件等。

为了保证配置文件的完整性，会先将文件写入到临时目录下的临时文件，然后通过 rename 操作保证原子性。注意，在编译时，需要 **保证配置文件的目录与临时目录在同一个挂载点**，否则会报 `Invalid cross-device link` 的错误。

另外，为了防止临时目录中出现脏数据，会每隔 15min 扫描一次目录 (注意，不扫描子目录以及隐藏文件)，对于修改时间超过 24 小时的文件会自动删除。

### TODO

{% highlight text %}
1. 安全通讯。
2. systemd notify 启动，自动健康检查。
3. completion 命令自动补齐。
4. Async DNS 目前的 DNS 使用同步方式。
5. 数据上报压缩。
6. 磁盘空间不足，只有内存时也可以使用。
7. 包括了 normal、bypass 等几种模式。
{% endhighlight %}

<!--
ps -eo ppid,pid,pgid,user,group,euser,egroup,cmd | grep gearman
usermod -a -G root monitor 将monitor用户添加到root组中

https://github.com/Jin-Yang/cgfy
https://github.com/chr15p/cgshares

Reap zombie processes using a SIGCHLD handler
http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html


https://github.com/mludvig/mini-printf
https://github.com/mpaland/printf
https://github.com/cjlano/tinyprintf
https://github.com/MarioViara/xprintfc
https://github.com/wkoszek/mini_printf
https://github.com/idning/safe_snprintf  这个不错


## 直方图

High Dynamic Range Histogram
http://hdrhistogram.org/
https://github.com/HdrHistogram/HdrHistogram_c

https://github.com/powturbo/TurboHist
https://github.com/astrofrog/fast-histogram

Vector Field Histogram 机器人中的算法
https://github.com/agarie/vector-field-histogram

https://github.com/retrage/sqlite-bench
https://github.com/Scottz0r/sqlite_performance_demo


down_new() 申请所需要的内存，并解析下载URL
  |-conn_parse() 解析下载地址信息，包括路径以及用户名密码
  |-conn_init() 根据不同的下载协议进行初始化
    |-ftp_connect() FTP建立链接登陆，并切换到Binary模式
    | |-tcp_connect() 通过TCP链接，注意，如果设置了io_timeout会采用异步链接 尝试建立链接
    |-ftp_cwd() 切换目录
    |
    |-http_connect() 建立TCP链接  HTTP connected with fd
  |-conn_info() 获取文件大小信息
    |-ftp_size() 获取文件大小
    |
    |-conn_setup() 对于HTTP下载模式，会包括了多次的重定向
    | |-conn_init() 如果还没有建立链接则初始化
    | |-http_get() 拼接请求头信息
    |-conn_exec() 发送HTTP请求获取返回头
      |-http_exec()
    |-conn_disconnect() 关闭请求

down_open()  打开状态文件，以及保存文件，如果不支持并发下载，则回归到单线程
  |-open() 打开*.st文件，查看是否有之前的保存信息
  |-downloader_divide() 拆分单个线程下载文件的范围
  |-open() 打开dl->filename文件下载

booter_start() 开始下载文件，会创建多个线程，入口函数为setup_thread()
  |-conn_set() 针对不同的线程重新解析URL
  |-setup_thread()
    |-conn_setup() 不同的线程建立不同的链接，多线程用于建立链接
      |-conn_init() 初始化链接
    |-conn_exec()

booter_do() 开始真正下载

简单的SO加密方法
https://bbs.pediy.com/thread-191649.htm
https://paper.seebug.org/89/







一个文件所属的分区可以通过 fstat() 函数查看，或者 `df <DIR>` 。


EAGAIN的处理方式
https://blog.csdn.net/tianmohust/article/details/8691644
-->


{% highlight text %}
{% endhighlight %}
