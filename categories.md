---
layout: default
title: Categories
---

<style type="text/css"><!-- p {text-indent: 2em;} --></style>

## 分类

该页面开头是主要分类及其介绍，主要是可以根据分类进行查询，后面会有根据标签（tags）的全部文章的分类，和侧边相似。

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

![Program Logo]({{ site.url }}/images/program-logo.jpg "Program Logo"){: .pull-center width="250"}

码农的工具，简单来说就是用来解释给机器如何执行，有问题还得调试，一耍脾气就要再研究几天，类似于猫奴，完全就是机器奴隶。

即使如此，仍要注意：

Programs must be written for people to read, and only incidentally for machines to execute.

* [版本管理](/post/theme-version-control.html) 主要是通过 git 来进行版本管理。
* [C/C++ 语言专题](/post/theme-language-ccpp.html)
* [Shell 语言专题](/post/theme-language-bash.html) 包括了 Bash 的使用方法，以及相关的编程语言。
* [GoLang 语言专题](/post/theme-language-golang.html)
* [Python 语言专题](/post/theme-language-python.html)
* [Java 语言专题](/post/theme-language-java.html)

#### 其它

一些常用的网站参考。

* [www.tiobe.com](https://www.tiobe.com/tiobe-index/) 各个语言的排名。
* [turnoff.us](http://turnoff.us/) 编程语言、Web、云计算、Linux 相关的漫画。

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

![Database Logo]({{ site.url }}/images/databases/database-logo.jpg "Database Logo"){: .pull-center width="180"}

数据库管理系统，Database Management System 估计应该是除了操作系统之外的最为复杂的系统了，按照不同的方式可以分成不同的类型，例如关系型和非关系型。

包括了商业数据库 Oracle，开源的 MySQL、PostgreSQL等关系型数据库，小型的嵌入式 SQLite，还有最新的 OceanBase、TiDB 等分布式数据库。

详细可以参考：

* [MySQL 数据库专题](/post/theme-database-mysql.html)
* [PostgreSQL 数据库专题](/post/theme-database-postgresql.html)

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

![Linux Logo]({{ site.url }}/images/linux/linux-logo.jpg "Linux Logo"){: .pull-center width="340"}

从 1994.3 Linux1.0 发布到现在，几乎可以说 Linux 已经成为最流行的操作系统，涉及到了服务器、桌面、嵌入式等多种场景，而且支持绝大多数平台。

<!--
鄙人从大三开始用 Linux，开始就是看着 Ubuntu 的 3D 桌面比较酷，然后开始零零散散地使用，一直到现在，几乎所有的日常操作都在使用 Linux 的桌面发行版；目前使用的是 CentOS 版本。
-->

在此，仅介绍 Linux 相关内容，包括了常用的方法，以及相应的内核介绍。

* [Linux 环境搭建](/post/theme-linux-environment.html) 包括了如何搭建 Linux 环境、相关工具、基本概念等。
* [Linux 安全专题](/post/theme-linux-security.html)
* [Linux 内核专题](/post/theme-linux-kenerl-stuff.html)
* [Linux 内存专题](/post/theme-linux-kenerl-memory.html)
* [Linux 网络专题](/post/theme-linux-kenerl-network.html)
* [Linux 容器专题](/post/theme-linux-kenerl-container.html) 不只是 cgroup 容器，还有 ulimit 机制、chroot 相关的内容。
* [Linux 监控专题](/post/theme-linux-kenerl-monitor.html)
* [Linux 时间专题](/post/theme-linux-time-stuff.html) 这真心是个很复杂的问题，包括了基本的概念、系统中的使用方式等等。

#### 其它

比较经典的常用网站。

* [Linux Inside](https://github.com/0xAX/linux-insides) 一个电子书，详细介绍了 Linux 相关的基本概念。

<!--
 post/encryption-introduce.html linux-aio.html

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

* [JavaScript 环境](/post/javascript-environment-introduce.html)，介绍 JavaScript 常用工具，如 node、npm、WebPack 等。
* [JS React 语法简介](/post/javascript-react-syntax-introduce.html)，介绍 JS 和 React 的一些常见语法规则，以及调试工具。
* [React 简明教程](/post/react-practice-examples.html)，通过一些示例简单介绍 React 的使用方法。

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
-->

----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

一些杂七杂八的东西。

* [人工智能](/post/theme-artificial-intelligence.html)

![Nginx Logo]({{ site.url }}/images/nginx-logo.png "Nginx Logo"){: .pull-center width="200" }

Nginx 一款轻量级且高性能的 Web 服务器、反向代理服务器，通过 C 语言编写，通过模块化实现，很多功能都可以通过三方模块支持。

* [Nginx 入门](/post/nginx-introduce.html)，介绍一些常见的操作，例如安装、启动、设置等。
* [Nginx 监控](/post/nginx-monitor.html)，关于 Nginx 的简单监控内容。
* [Nginx 源码解析](/post/nginx-sourecode-analyze.html)，介绍主要的代码实现过程。
* [Nginx 通用网关](/post/nginx-cgi-introduce.html)，与 CGI 相关的内容，以及部分的实现。
* [Nginx 日志解析](/post/nginx-logs-introduce.html)，简单介绍 Nginx 中的日志，以及原子写入的简介。
* [Nginx HTTPS 配置](/post/nginx-https.html)，简单介绍如何使用 Nginx 搭建 https 服务。
* [HTTP 协议简介](/post/network-http-introduce.html)，简单介绍下 HTTP 内容以及其演变过程。
* [HTTPS 协议详解](/post/https-introduce.html)，简单介绍下 HTTPS 协议是如何实现的

![Lua Logo]({{ site.url }}/images/programs/lua-logo.png "Lua Logo"){: .pull-center width="125" }

Lua 在葡萄牙语中是 “月亮” 的意思，是一个小巧的脚本语言，官方版本只包括一个精简的核心和最基本的库，使得其体积小、启动速度快，从而特别适合嵌入到其它程序里。

这里简单介绍其使用方法。

* [Lua 简介](/post/lua-introduce.html)，简单介绍常见概念，包括安装、语法规则、常用模块等。
* [Lua 协程](/post/lua-coroutine.html)，作为一种简单的语言，仍支持闭包、协程等较新的特性，简单介绍协程使用。
* [Lua 源码解析](/post/lua-sourcecode.html)，其核心代码总共才 2W 行左右，但是却实现了很多不错的特性。
* [Lua CAPI 使用](/post/lua-how-capi-works.html)，简单介绍 Lua 和 C 之间的调用，常见的概念如栈、CAPI等概念。

![RAFT Logo]({{ site.url }}/images/databases/raft/raft-logo.png "RAFT Logo"){: .pull-center width="210" }

PAXOS 算法从 90 年提出到现在已经有二十几年了，不过其流程过于复杂，目前较多的有 Chubby、libpaxos ，以及 Zookeeper 修改后的 Zookeeper Atomic Broadcase, ZAB 。

RAFT 是斯坦福的 Diego Ongaro、John Ousterhout 两人设计的一致性算法，在 2013 年发布了论文 《In Search of an Understandable Consensus Algorithm》，目前已经有近十多种语言的实现，其中使用较多的是 ETCD 。

* [RAFT 协议简介](/post/raft-consensus-algorithms-introduce.html) 一个为真实世界应用建立的协议，注重落地性和可理解性。
* [ETCD 基本简介](/post/golang-raft-etcd-introduce.html) 主要介绍 ETCD 如何使用，包括安装、部署、使用以及常见的介绍。
* [ETCD 示例源码](/post/golang-raft-etcd-example-sourcode-details.html) 源码中关于如何 RAFT 协议的示例代码，直接使用的是内存数据库。
* [ETCD 源码解析](/post/golang-raft-etcd-sourcode-details.html) 除了上述的示例代码，这里简单介绍其代码的实现。

<!--
* [ETCD 源码解析](/post/golang-raft-etcd-sourcode-network.html)
* [ETCD 示例源码](/post/golang-raft-etcd-sourcode-storage.html)
* [ETCD 示例源码](/post/golang-raft-etcd-sourcode-consistent-reading.html)
* [ETCD 示例源码](/post/golang-raft-etcd-backend-boltdb.html)
-->

## Tags

{% for category in site.categories %}
<h3 id="{{ category | first }}">{{ category | first }}</h3>
<ul>{% for post in category[1] %}<li>{{ post.date | date: "%Y-%m-%d" }} <a href="{{post.url}}">{{ post.title }}</a></li>{% endfor %}</ul>
{% endfor %}

<!--
一个不错的网站，包含了各种书籍。
http://apprize.info/

当浏览器输入地址时发生了什么
https://github.com/alex/what-happens-when

内存的战争，不错的文章

一个web tty共享
https://tsl0922.github.io/ttyd/

1. Hesitate 犹豫不决
2. Procastination 拖延，逃避问题和懒惰
3. Never last long 三分钟热度
4. Afraid of rejection 害怕拒绝
5. Limit yourself 自我设限
6. Runaway from reality 逃避现实
7. Always find execuess 总是寻找接口
8. Fearness 恐惧
9. Refuse to learn 拒绝学习

Python 资源大全中文版
ttps://github.com/jobbole/awesome-python-cn

SQLite源码解析
http://huili.github.io/srcAnaly/selectExec.html

CVE库
https://www.cvedetails.com/
WebSockets库
https://github.com/uNetworking/uWebSockets
C++使用mysql,断线重连问题
http://www.paobuke.com/zh-cn/develop/c/pbk1821.html
蛋疼的mysql_ping()以及MYSQL_OPT_RECONNECT
https://www.felix021.com/blog/read.php?2102
CA
http://www.barretlee.com/blog/2016/04/24/detail-about-ca-and-certs/
StatsD Python上报示例
https://github.com/etsy/statsd/blob/master/examples/python_example.py
使用C写的editline库，用于替换readline()函数
https://github.com/troglobit/editline
MYSQL C使用
http://zetcode.com/db/mysqlc/
PG用户管理
http://www.davidpashley.com/articles/postgresql-user-administration/
ZeroMQ
https://github.com/anjuke/zguide-cn

索罗斯，三大原则

* 客户第一
* 对发明创造的渴望
* 长远的眼光和想法

Semantic Versioning 语义化版本规范
-->
