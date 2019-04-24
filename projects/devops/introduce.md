---
title: 基础平台简介
layout: project
comments: true
language: chinese
category: [misc]
keywords: hello world,示例,sample,markdown
description: 简单记录一下一些与 Markdown 相关的内容，包括了一些使用模版。
---

<!-- more -->

简单来说，就是如何通过如下的一些平台组合，满足运维的需求。目的：提高自动化、标准化，积累经验，减少人工投入，降低故障率。

有时候运维和监控平台的运行是一个矛盾体，如在变更时要求尽量减小对在线服务的影响，那么就要求占用资源尽量少（如网络、CPU、磁盘等）；当出现故障需要处理时，

基本功能：日志审计、权限管理；高级功能：高可用、服务降级 (如OSS不可用) 。

## 基础平台 Basic Platform

基础运维基础平台，用于提供统一运维工具，提供运维、变更、应急标准化流程，其难点在于如何保证数据一致性。

### CMDB +++++

元数据管理，主机管理、业务拓扑、资源池管理、网络设备。【高可用】

{% highlight text %}
1. 元信息。产品、主机、IDC、网络，任务中心(IP扫描、数据同步等任务)  Region->Category->Service->Component
   1.1 主机元信息：
       主机信息。Alias、Tags、SN、采购时间、过保时间、业务分组、备注。
                 业务状态(在线、离线)、物理状态(正常、故障、下架)、保修状态(在保、过保)。
       操作系统。Hostname(主机名 uname -n)、Kernel(内核版本 uname -s + uname -r)、
                 Version(内核版本 uname -v)、Arch(内核平台 uname -m)、OperationSystem(操作系统 uname -o)
       处理器。缓存大小、物理核、逻辑核、主频、型号、厂商。
       内存。物理内存交换区。
       磁盘信息。
       网络。MAC、IP、网卡信息。
   1.2 域名信息
       URL、业务分组、主机地址、TTL、状态、备案情况、CDN类型。
   1.3 公网IP信息
       IP、服务商、业务分组、关联域名。
   1.4 网络设备
       SN、分类、主机名、业务分组。
   1.5 业务信息
       名称、类型、上线时间、资源使用数、服务依赖。

2. 提供REST-API接口供用户访问，例如当前主机的信息。

3. 任务管理。提供常见的基本任务操作，如：
   3.1 网段周期扫描，自动发现未管理主机，通常是在业务已经发展一段时间之后。
   3.2 数据采集。周期更新、校验CMDB数据，可以通过Agent采集，或者从其它服务同步。

4. 容量资源、资产管理。
{% endhighlight %}

### Agent +++++

提供管道服务，执行命令、文件下发、信息采集、任务调度(服务端)。【稳定】 (开源产品有Ansible、SlatStack等)

{% highlight text %}
1. 自管理，初始化(如通过Ansible安装、安装脚本)、自升级(通过简单稳定子程序监控)、状态、版本。
   1.1 插件管理，支持场景包括：周期执行(命令、脚本)；
   1.2 进程管理，主要是子进程的管理(监控、日志、安全、文件等)，包括启停、状态检测等；
   1.3 数据查询，提供标准查询接口，例如：磁盘、内存、内核等；
   1.4 应急处理，主要是自杀退出。

2. 特性，主要是自身的一些特性设置。
   2.1 资源限制，优先使用cgroup做资源隔离，其次周期检测；
   2.2 任务并发控制、优先级；
   2.3 特殊权限的控制，可以通过Linux中的Capability机制；
   2.4 签名机制、通讯管道密文加密；
   2.5 IP白名单，命令白名单；

3. 状态检测，支撑状态视图(含子进程状态)，一般有PING/AGENT/SUB-MODULES，其状态信息保存在缓存中，在升级时会有频繁更新。

4. 任务类型。
   4.1 命令下发。同步任务、异步任务、标准查询(类似SaltStack的Grain)。
   4.2 管理任务。自身配置管理、插件配置管理。

5. 提供统一接口查询系统信息。
   5.1 用户信息。
       A) 所用用户信息getpwent()，密码是否过期，用户是否被锁；
       B) 非系统用户信息(默认)；
       C) 指定UID/GID，用户名、分组名查询getpwuid()/getgrgid()。
   5.2 系统版本。
       A) 内核指标信息 uname()；
       B) 主机名 gethostname()。
   5.3 内存信息。A) RAM信息；B) Swap信息。
   5.4 磁盘信息。

6. 测试用例。
   6.1 资源继承，通过lsof；
   6.2 文件描述符是否泄露；
   6.3 命令执行超时、交互命令、返回内容过大、Unicode支持；
   6.4 大批量操作。

7. 高级特性。
   7.1 自动扩容，可以直接根据默认获取数据。
{% endhighlight %}


### Task ++++

作业管理，包括了任务下发、任务调度、并发管理、任务编排、巡检。【可降级】(Airflow)

{% highlight text %}
1. 任务下发用于同时处理多台主机，建议提供【高可用】，在有其它脚本或者可替换工具时【可降级】。

2. 常见任务场景。
   2.1 发布变更，服务器分组批量配置(分组规则包括版本、业务、机房等)；
   2.2 数据库自动备份；
   2.3 大文件下发、用户管理；
   2.4 巡检(账号扫描、安全扫描)。

3. 特性。超时重试、并发分组控制、密钥管理、
{% endhighlight %}

### Deploy +++

装机服务、软件部署，一般在安装部署时使用。【可降级】

{% highlight text %}
1. 部署标准流程控制，支持蓝绿部署、A/B测试、灰度发布等部署方式。
2. 针对不同的类型配置不同的安装模板，例如HAProxy、MySQL、PostgreSQL、Nginx等。
{% endhighlight %}

### Repository ++

软件仓库，包括了系统OS、CICD部署等。【可降级】

{% highlight text %}
1. OS包基本仓库。
{% endhighlight %}

### Config ++

配置管理，软件配置、主机自发现。【高可用】(Disconf)

## Monitor Center

监控平台，其目标是提高故障发现率；难点: 存储、告警；关键特性：提供可靠实时告警，故障回溯数据支撑，容量评估基础数据。

上报的指标通常用作告警设置以及数据展示；可以分为主机监控、应用监控(APM)、网络监控、公共组件监控。

按照优先级可以按照如下步骤执行：1. 主机单机监控、告警、通知；2. 组件对比、环比、APM；3. NOC(监控大屏)、故障树；4. 智能监控运维。

![category]({{ site.url }}/images/devops/monitor-metrics-tracing-logging.png "category"){: .pull-center width="40%" }

<!-- http://peter.bourgon.org/blog/2017/02/21/metrics-tracing-and-logging.html -->

<!--
Build, Config, Launch and Monitor the Infrastructure.
BCLM

Collecting, processing, aggregating, and displaying real-time quantitative data about a system.

简单来说，其基本功能就是收集、处理、汇总和展示系统的实时状态数据。目的就是为了实时了解系统的状态，以保证其稳定性，简言之就是能够回答： A) What's wrong? B) Why?

监控的核心链路是数据和告警，而难度较大的又属数据存储。监控数据是一个实时连续产生的大数据，几乎不会出现波峰波谷，要求系统有足够大的容量 (capacity)、吞吐量 (Throughput) 以及足够短的时延 (Latency)。

一个监控系统的设计通常需要参考如下的几个方面：

* Data collecting interval and resolution 采集频率和精度
* Data storage, as well as retention period and policy 数据存储及其保存策略
* Alert thresholds and rules 警告筏值和规则
* Data search and services 监控数据的查询服务
* Data aggregation and alert noise reduction 数据聚合以及告警去噪

对于 Google 来说，其监控系统主要聚焦四大核心指标：

* Latency 一个请求的响应时间，包括成功失败
* Traffic 服务的繁忙度，如每秒请求数
* Errors 服务请求失败率
* Saturation 服务的饱和度，如CPU的使用率

而 Google 对监控的建设也遵循以下原则：

* 保证监控系统简单明了、快速可靠。对于海量的实时数据，不建议做太复杂的实时数据分析，此时采集的数据主要回答 What's wrong?
* 完善的事后分析 (post-hoc analysis) 工具，用于做根因分子，用于回答 Why?

Google 关于可靠性的原则之一就是 Reliability 不是由 Google 的系统监控和预警系统决定，而是由用户说了算。另外一个原则是，可靠性要达到业界的黄金标准的 99.99%，只靠完善软件本身的设计和实现是不可能达到的，必须结合优秀的 SRE 运维实践。
-->


### MonitorAgent +++++

监控客户端。【稳定】(Collectd)

{% highlight text %}
1. 系统监控指标上包。
   1.1 只有一个.so即可，内置types.db以及默认配置；可以通过命令行保存成配置文件。
   1.2 支持参数动态修改。
   1.3 支持插件启停。
   1.4 插件的动态加载。

2. 数据采集异常上报。

3. SuperAgent 内网拨测。Agent(Active/Inactive)、SNMP、JMX、SSH、HTTP、UDP、TCP。

4. StatsD 协议。每次只能上报一个指标，对于类似成功率无法计算。
   4.1 上报是嵌套在代码逻辑中的，通过UDP协议，即使StatsD服务不可用，不会影响原有代码逻辑。
{% endhighlight %}

<!--
2.1 插件需要提供两个接口，precheck 用于检查配置是否合法，apply 应用(会保证在未调用回调的时候调用，防止出现竞态条件)；
2. plugin_instance 使用key:value分割；plugin.type定义了类型。
3. 单个插件可以注册多个读函数，例如MySQL可以通过监控多个DBs，不同的DBs通过user_data传递参数。
-->

### LogAgent ++

日志采集客户端。【稳定】(Logstash)

{% highlight text %}
1. 基于Inotify。
{% endhighlight %}

### Alert +++++

告警，告警规则、告警过滤。【高可用、不降级】(Zabbix)

{% highlight text %}
1. 告警规则。主机名支持前缀匹配，考虑性能不支持全文匹配；通过tag设置规则。
   1.1 简单阈值判断。
   1.2 移动平均处理。
2. 告警聚合。预告警信息保存，方便回溯。
   2.1 迟滞处理，用于过滤掉在阈值范围内的波动情况。
   2.2 多次命中，实际就是连续预告警多次后发送告警，用于过滤噪声。
{% endhighlight %}

### Storage +++++

可使用时序数据库 InfluxDB、OpenTSDB 等。

### Notify +++++

通知，邮件、SMN、手机。【高可用、不降级】

{% highlight text %}
1. 通知确保可达，信息不重复，消息中间件可以采用RabbitMQ。
{% endhighlight %}

### View ++++

视图展示，主机视图、服务视图、组件视图。【可降级】

{% highlight text %}
1. 视图。包括了产品树用来选择，标签页中包含了总览、视图、配置、主机视图、服务视图(调用链)、组件视图(数据库)。
   1.1 总览。
   1.1 主机视图，系统类的指标，包括了CPU、MEM、NET、DISK等基础监控。
   1.2 服务视图，业务配置下发的脚本采集数据，也可以包含调用链(太复杂建议单独开个标签页)。
   1.3 组件视图，通常是针对一些公用服务的监控，不同类型通常展示的视角也有所不同，例如HAProxy、Nginx、MySQL等。
2. DashBoard 大盘展示，包括了业务大盘(主机数、在线主机数、插件数目等)、报错大盘(最新告警)、核心指标(主站延迟)。
3. 配置中心。
   1.1 插件的配置管理，主机绑定(可以允许级联)。
4. 告警中心。可以处理告警回溯、抑制等。
{% endhighlight %}

### APM ++

业务调用链，基本都是参考 Google 的 Dapper 论文实现，业界按照侵入性排序为 `Pinpoint->Zipkin(Java)->CAT`，其它的参考 Zipkin 的 `Jaeger(Golang)`、`AppDash(GoLang)`。



<!--
>>>>>> Others >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
3. 其它
AIDE  主动防御安全监测
自动拉起

BootStrap 用于 Agent 的自动升级，只包含了最基本的操作，倾向于小且稳定。
	1. 自动下载升级包完成升级，提供回滚操作。
	2. 升级完成后仍存在一天，检查是否升级成功，检查项包括：
	   2.1 是否频繁重启。
	   2.2 CPU使用率超过 5%；内存超过 100M；内存相比第一次启动增长超过 20M。
	3. 报告升级状态。

### Agent基本需求

不同运行环境、多租户资源隔离、资源限制(流量的削峰填谷)、配置管理、系统监控、容错(网络异常、脚本执行异常)、升级等。

#### 日志采集需求

1. 日志格式解析(通过不同插件实现)，分布式日志汇聚；
2. 不同采集目录，日志轮转；

日志采集主要分为两种模式：A) Polling 也就是周期性的轮询，检查日志文件是否有更新；B) Event 一般是基于操作系统的事件通知机制，例如 Linux 的 Inotify 机制，Windows 的 FindFirstChangeNotification 。

目前的大部分日志采集 Agent ，如 logstash、fluentd、filebeats、nxlog 都采用的是轮询机制，相比来说其实现更简单。

https://yq.aliyun.com/articles/204554
https://yq.aliyun.com/articles/251629

### 存储系统特性 <Reference:Gorilla: A Fast, Scalable, In Memory Time Series Database>

允许单点丢失，更加关注聚合的分析数据，新数据相比老数据来说更加重要。

1. 写多读少。每秒都会有大量的数据写入，读取一般只有部分的图表展示，重要数据读取。
2. 故障发现。故障一般在分钟级内发现是有效的，那么就需要可以缓存处理小窗口内的数据指标。
3. 高可用。当出现了网络分区时，对于不同机房仍然可以保存到本地，不影响本地数据的存取，也就是说对于CAP优先选择AP。
4. 数据冗余。实际上就是需要将完整的数据保存在不同机房中，当单个节点故障时，不会丢失数据。

对于数据存储，不要求 ACID ，但是要保证及时出现机房级的故障时仍可以持久化绝大多数的数据。

存储数据较多的是浮点类型以及整形，对于浮点型可以通过一些压缩算法进行压缩。

在部署时，每个 region 都会部署一个实例，多个实例之间会进行数据同步，当然不是强一致的，这样每个实例都保存了完整的数据，查询可以就近选择对应的实例访问即可。

## 指标命名规则

metric.name:value|type|@sample_rate|#tag1:value,tag2

1. 只支持 ASCII 字符集，不支持 Unicode，可以是 [A-Za-z0-9._]+ ，不建议使用空格。
2. 不能超过127个字符，为了便于前端展示，建议小于100个字符。

注意：处于性能考虑，我们不会校验指标名的合法性，对于其它字符可能会导致不兼容。
# Increment the page.views counter
page.views:1|c

# Record the fuel tank is half-empty
fuel.level:0.5|g

# Sample the song length histogram half of the time
song.length:240|h|@0.5

# Track a unique visitor to the site
users.uniques:1234|s

# Increment the active users counter, tag by country of origin
users.online:1|c|#country:china

# Track active China users and use a sample rate
users.online:1|c|@0.5|#country:china

时序数据库相关
http://www.vldb.org/pvldb/vol8/p1816-teller.pdf
https://github.com/grafana/metrictank
http://www.evanlin.com/gorilla-facebook-tsdb/

指标2.0
http://metrics20.org/spec/

>>>>>> Common Components >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
3. 通用公共组件。
Nginx 路由、负载均衡、动静分离。
HAProxy 负载均衡。
APIService 对外API接口。
ETCD 高可用、服务注册。
SSO{*****} 用户权限管理。
MySQL 关系型数据库，CMDB。
InfluxDB 时序数据库，保存监控采集数据。
Redis/MemCached 数据缓存。
4. 高级功能。
Analyes 数据分析。

5. 开源分析
   1. Ansible 配置管理、任务编排，使用SSHD提供服务，可以作为一些主机的初始化配置，如Agent安装。
   2. SaltStack 强大的高性能工具，原则上来说基本可以用它完成大部分的运维平台搭建。
   3. OSQuery 实际上通过了一个接口屏蔽系统的信息。

6. 部署规范
   1. 文件与目录规范。
   2. 基础软件安装管理。
   3. 用户和权限管理。
   4. 运维脚本规范。
   5. 日志输出规范。

7. 标准化体系
   1. 基础服务标准。
      1. 接入层规范。
	  2. 域名解析使用规范。
	  3. 负载均衡使用规范。LVS、HAProxy、Nginx、VIP。
	  4. 静态页面缓存使用规范。Suqid、Varnish、ATS。
	  5. CDN 使用规范。
	  6. 缓存使用规范。Memcached、Redis。
	  7. 数据库使用规范。MySQL、PostgreSQL。
	  8. 文件存储使用规范。
   2. 稳定性标准规范
      1. 全链路跟踪系统。TraceID、SpanID
	  2. 限流降级系统。
	  3. 容量评估系统。
	  4. 预案系统。
	  5. 业务依赖。
   3. 安全规范
      1. ACL 策略。
	  2. WAF 相关。
	  3. IPTables 相关。

应用场景：
1. 任务编排。
2. 产品发布。
3. 批量执行。
4. 应急处理。
5. 运维上线流程。
#######################
## MonitorAgent
#######################

约束：
    1. 一个采集脚本只运行一次，但是可以设置不同的参数，如采集周期、采集内容等。
	2. 采集指标只能是数值，如果采集失败，应该打印到日志中。也可以放宽到采集失败时返回错误原因，但是该指标不会以数据项的方式保存到数据库中。
	3. 考虑到聚合以及前端查询展示方便，保存到数据库中的持久化数据只能是数值。对于异常的事件(含字符串信息)不能保存到保存数据项对应的数据库，为了回溯可以保存到日志或者数据库其它的事件表中。

常用术语：
    * 采集分类 category，例如默认的系统采集分类是 system，而数据库团队有特殊的定制可以重命名为 db_system 。
	* 采集项 collector，通常是一个脚本、命令、插件，可以有多个实例 (instance)，以 cpu 为例，其实例可以是 total、cpu0 ... cpuN，甚至将系统负载 load 添加进来。
	* 数据项 metric，也就是对应了一个具体的值，例如cpu.total.idle=20。

PluginName(M)-PluginInstance(O)|TypeName(O)-TypeInstance(O)
从现有的系统来说，应该|是使用最少的，因此将其作为保留字符，同时在实例中标识保留 : 。
load|=
cpu|id:0|idle=10
inode|path:/opt|usage=100
curl|url:http://example.com/status|code=200

通过 PluginName+TypeName 确定 DataSets 的类型；指标上报的单位事先约定好。

#######################
## 权限管理
#######################

Representational State Transfer, REST 是 Roy Fielding 博士在 2000 年的博士论文中提出来的一种软件架构风格，其中比较重要的是资源和状态转换。

资源基本可以分为三类，分别是私有 (Private) 资源、角色 (Role) 资源、公共 (Public) 资源。

**私人资源** 是属于某一用户的资源，只有用户本人才能操作，其它用户不能操作，例如用户的个人信息、收货地址等等。
**角色资源** 一般是多种权限的集合，对应了一群人，而且只有身为该角色的用户才能拥有这些权限，例如系统资源只能够管理员操作。
**公共资源** 所有人无论角色都能够访问并操作的资源。

权限就是资源和操作的组合，例如对于用户的管理，包括了增删改查用户。

那么用户、角色、权限三者之间的关系如下：

角色 VS. 用户：一个角色对应一群用户，一个用户也可以扮演多个角色，多对多的关系。
角色 VS. 权限：一个角色拥有一堆权限，一个权限却只能属于一个角色，一(角色)对多(权限)的关系
权限 VS. 用户：一个用户可以扮演多个角色，一个角色拥有多个权限，用户与权限是间接的多对多关系。

需要注意两种特别情况：

1. 私人资源 VS. 用户，私人资源的权限只能属于一个用户，此时用户和权限是一(用户)对多(权限)的关系。
2. 超级管理员直接忽略鉴权，可以操作一切资源。

http://www.jianshu.com/p/db65cf48c111
https://flask-rbac.readthedocs.io/en/latest/


#######################
## 常见概念
#######################

## 模板

模板存在主要是为了方便管理，而且相比来说规则、视图等信息是有限的，可以直接通过关联 ID 统计，减少表大小。

模板通常保存了通用的配置信息，例如报警规则、视图等信息，每台主机同时需要保存关联的模板，同时增加模板修改后是否关联的主机同时继承修改项。

http://john88wang.blog.51cto.com/2165294/1833636

## 主机标示方式

通常有三种标示一台主机 (虚拟机、物理机、容器)：

1. 主机 IP 地址，包括了 IPv4 和 IPv6 ，数据库可以用网络字节序的 int 存储，IPv4:32bit IPv6:128bit；
2. 主机名，通常为字符串，而且一般按照规范定义主机类型，不过如果不标准，那么可能导致大部分是 localhost ；
3. UUID，实际上就是逻辑的 ID 了。

那么对于这三种方式如何选择：

1. 通常主机名存储是最好的，对于用户体验好，但前提是在部署前已经制定好了命名规则；
2. 不建议使用 IP 地址，虽然直接用 INT 类型做主键时效率更高一些；
3. UUID 实际上也可以转换为 INT 类型，但是这个通常作为字符串处理。

对于字符串的，主键建议使用自增 ID ，将唯一字符串作为二级索引。


对于 IP 的查找，可以参考 Linux 内核中的 Netfilter 实现。Netfilter 是一款强大的基于状态的防火墙，其核心具有连接跟踪 (conntrack) 的实现，基于此有很多增强功能的实现，例如 NAT、基于内容的业务识别。

[linux内核netfilter连接跟踪的hash算法](http://www.cnblogs.com/wuchanming/p/3801650.html) 。
[Fast IP Routing with LC-Tries](http://www.drdobbs.com/cpp/fast-ip-routing-with-lc-tries/184410638)
https://www.kernel.org/doc/Documentation/networking/fib_trie.txt
https://github.com/torvalds/linux/blob/master/net/ipv4/fib_trie.c






通过Python实现的一个评论系统
https://posativ.org/isso/
https://zhangnew.com/isso-open-source-comment-system.html
通过Issue提供的评论框
https://github.com/imsun/gitment
一些乱七八糟的评论
https://github.com/Blankj/awesome-comment
一个简单的评论系统
https://github.com/adtac/commento



## 任务管理

### 需求

其中 TAG 一般不会超过 1000，而主机数可以是 100W+ 。

1. 服务端任务保存；分为两个纬度，主机以及Tag，均保存在hosts表中
1. 服务端任务下发流程；可以针对单个主机下发，或者对于 TAG 下发
1. Agent 和 Server 的任务一致性校验；
1. 主机的自动伸缩、扩容支持。当安装并挂载到产品数之后

单纯对于一台主机来说


Python 的双因子验证
http://blog.51cto.com/cwtea/2068137
https://www.secpulse.com/archives/4606.html
http://www.freebuf.com/articles/network/150071.html
http://www.freebuf.com/articles/web/165139.html



* 支持脚本调度采集(主进程中完成)，也就是简单调度执行。
* 通过多线程支持.so的动态库采集，通过线程数控制并发度。
* 特性。注意，包括了动态库和脚本。
  - 参数、任务的动态修改。
  - 动态起停、加载。
* 自监控能力。自身关键指标的监控。
* 事件上报。与指标不同，事件需要持久化并保证不丢失；而指标可能会因为网络、缓存等原因导致丢失。
* 多语言支持。将C提供的基本能力通过 Python、Lua、Java接口暴露出去。
* 通用上报接口。包括了HTTP/1.0、StatsD 。
* 支持指标、日志、Tracing数据的上报。日志应该也可以转换为指标，关键是tracing的数据格式如何兼容。

指标上报信息：

1. 指标名称、数值、类型、tags(例如磁盘有挂载点、类型)。
2. 时间戳。
3. 主机tags，一般有region、云服务、服务、组件信息。UUID可以压缩 https://www.jianshu.com/p/4ebbb1ce94f4

包括指标、tag等，可以提取出一些标准来，作为硬编码添加进去。

指标名称通过三层进行区分：


本来想通过 namespace 标示类似 system、bussiness 的逻辑划分，不过感觉这个是与分类绑定的，可能会在展示的时候使用，而用户在实际查询时基本上不会用到，所以直接忽略。

category(16B) 分类，一般是整体的分类，例如cpu、memory、mysql、haproxy等。
indicator(16B) 指标名称，类似于分类下的子类，例如 cpu 中的 usage，disk 中得 inode_usage 等。
instance(128B) 实例信息，例如 cpu 中监控的 0 核，某个 URL 的成功率等。
tags 一个 KV 对，用于标示指标相关的额外信息。

在解析时 tags、instance、indicator 是与 category 绑定的，也就是说如果 category 被修改了，那么缓存的上述三个指标都会被清除。

message Tag {
	required string Key;
	required string Value;
}
message Data {
	optional string UUID;
}

FAQ

1. 动态库为什么采用多线程？

目前很多组件的 C 接口仍然采用的是阻塞编程方式，通过多线程即使一个线程被阻塞了也不会影响到所有线程。当然，如果所有配置的线程都被阻塞了，那也就意味着将无法进行调度了，此时应该上报告警信息。

而脚本实际上就是 fork 进程之后执行，然后收集标准输出，本身不会占用过多的资源，那么就直接在主进程中进行处理就可以了。

2. 一些主机相关的数据是否需要随指标同时上传？

每次启动Agent会与服务端进行握手，其中包含了是否将一些主机相关的信息上传，主要包括了 Region、服务、微服务等信息。

如果监控数据是通过服务端在传输到流处理，实际上这些与主机绑定的信息在服务端进行更新会更加合适，这样就不再需要将相关的信息与主机侧进行同步了。

但是如果监控数据是直接通过类似 Kafka 再上报到流处理，那么就需要在上报的数据中直接携带上了。

3. 关于混合部署问题如何处理？

混合部署一般就是组件信息不同，实际上只要通过某种方式将主机相关的元数据信息上报即可，而采集的指标无需关心是否合部。


数据库管理很不错的页面
https://severalnines.com/blog/how-cluster-your-proxysql-load-balancers

-->




{% highlight text %}
{% endhighlight %}
