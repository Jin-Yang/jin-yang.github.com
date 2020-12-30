---
title: 常用 DNS 工具介绍
layout: post
comments: true
language: chinese
tag: [Protocol, Network, DevOps]
keywords: dns, getent, nslookup, dig
description: 之前已经介绍了关于 DNS 常见的基本概念，这里会通过一些常见的命令行工具进行验证，例如 getent、nslookup、dig 等等。
---

之前已经介绍了关于 DNS 常见的基本概念，这里会通过一些常见的命令行工具进行验证，例如 getent、nslookup、dig 等等。

<!-- more -->

## 简介

现在使用的 DNS 服务器地址可以通过 `cat /etc/resolv.conf` 查看。

## getent

实际上 dig 会直接使用 DNS 协议查询，如果要考虑 `/etc/hosts` 可以使用 `getent` 名利，示例如下。

{% highlight text %}
$ getent hosts 127.0.0.1
$ getent hosts www.baidu.com
{% endhighlight %}

这里实际上使用的是 Name Service Switch 机制，将会使用 `nsswitch.conf` 中的配置项。

## nslookup

用来查 DNS 信息的，可以使用交互以及非交互模式。

{% highlight text %}
$ nslookup
> www.baidu.com
Server:         61.139.2.69               // 上连的DNS服务器
Address:        61.139.2.69#53            // 上连的DNS服务器的IP地址与端口号

Non-authoritative answer:                 // 从上连DNS服务器的本地缓存中读取出的值，而非实际去查询到的值
www.baidu.com   canonical name = www.a.shifen.com.    // 说明还有个别名叫www.a.shifen.com
Name:   www.a.shifen.com                  // 域名www.a.shifen.com
Address: 119.75.217.56                    // 对应的IP地址之一
Name:   www.a.shifen.com
Address: 119.75.218.77                    // 对应的IP地址之二


> server IP                               // 设置上连的DNS服务器，可以google国内的其它DNS服务器
> set all                                 // 查看当前所有的配置选项
> set nodebug | debug                     // 进入非调试或者调试模式
> set nod2 | d2                           // 高级调试，会输出nslookup内部工作的信息，包括了许多函数调用信息
> set domain=baidu.com                    // 设置默认的域，对于无"."的请求，会自动在尾部追查该域
{% endhighlight %}

一般来说，对于不同的省的 DNS 服务器，通常会解析到不同的 Server 。对于 class 参数来说，常用的是 IN，而 Chaos、Hesiod 已经几乎不再使用。

## dig

比 nslookup 更高端的命令。

{% highlight text %}
$ dig [IP|ADDR]                           // 使用系统默认域名解析，默认查询A记录
$ dig @NS-ADDR [IP|ADDR] [TYPE]           // 指定域名解析服务器
$ dig +trace [IP|ADDR]                    // 查看整个DNS查询过程
{% endhighlight %}

`8.8.8.8` 和 `8.8.4.4` 是 google 对外开放的 DNS 服务器，但是国内不太稳定，建议使用其它开放 DNS 服务器，如 `114.114.114.114`、`1.2.4.8` 等 。

查询后的各个字段介绍如下。

{% highlight text %}
$ dig www.foobar.com
... ...
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 49814  # 是否有错误
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 2, ADDITIONAL: 17

;; QUESTION SECTION:  # 标示查询内容，这里查询的是域名的A记录
;rss.newyingyong.cn.            IN      A

;; ANSWER SECTION:    # 查询结果，A记录的IP地址，缓存600秒
rss.newyingyong.cn.     600     IN      A       139.129.23.162

;; AUTHORITY SECTION: # 从那台服务器获取到的A地址信息，对应了权威DNS服务器信息
newyingyong.cn.         86398   IN      NS      dns10.hichina.com.
newyingyong.cn.         86398   IN      NS      dns9.hichina.com.

;; ADDITIONAL SECTION: # NS服务器对应的IP地址，一般安装了DNS服务，例如BIND软件
dns9.hichina.com.       3490    IN      A       140.205.81.15
dns9.hichina.com.       3490    IN      A       140.205.81.25

;; Query time: 5 msec      # 完成查询的时间
;; SERVER: 10.202.72.116#53(10.202.72.116)  # 本地DNS解析的服务器地址
;; WHEN: Sat Jul 01 11:00:38 CST 2017
;; MSG SIZE  rcvd: 369
{% endhighlight %}

### 简单查询

{% highlight text %}
$ dig -t a www.baidu.com +noall +answer
www.baidu.com.          969     IN      CNAME   www.a.shifen.com.
www.a.shifen.com.       189     IN      A       14.215.177.39
www.a.shifen.com.       189     IN      A       14.215.177.38
{% endhighlight %}

首先 `www.baidu.com` 指向的是另外一个域名 (估计是做负载均衡用的) ，然后才是两条 A 记录。

#### Tips

可以看到，输出的结果后面多了一个 `.` ，实际上这里的真正域名是 `www.baidu.com.root` ，只是所有的根域名 `.root` 都相同，一般直接省略。

根域名下一级是 "顶级域名" (Top Level Domain, TLD)，例如 `.com`、`.net`；再下一级叫做 "次级域名" (Second Level Domain, SLD)，例如上述的 `.baidu`，这一级域名是可以注册的。

再下一级是主机名 (Host) 由用户统一管理，可以任意分配，例如上面的 `www` 。

### 查询 NS 域名

如果想知道 `www.baidu.com` 的记录是由那个 DNS 服务器提供的，那就需要使用 NS (NameServer) 的 RR 类型标志来查询。

由于 NS 是管理整个领域的，因此，需要查询的目标应该是对应的 domain，也就是 `baidu.com` 。

{% highlight text %}
$ dig -t ns baidu.com +noall +answer
baidu.com.              86314   IN      NS      ns4.baidu.com.
baidu.com.              86314   IN      NS      ns3.baidu.com.
baidu.com.              86314   IN      NS      ns2.baidu.com.
baidu.com.              86314   IN      NS      dns.baidu.com.
baidu.com.              86314   IN      NS      ns7.baidu.com.
{% endhighlight %}

每一级域名都有自己的 NS 记录，NS 记录指向该级域名的域名服务器，这些域名服务器中记录了下一级域名的相关信息。

可以通过 `+trace` 参数查看详细的查询过程，也可以通过 `dig -t ns com` `dig -t ns baidu.com` 依次查询。

### SOA 信息查询

也就是查询管理域名的服务器管理信息。

为了提供高可用，一般在管理同一个领域名时会使用 Master Slave 方式来进行管理，此时就需要告知如何管理 Zone File 配置了，此时就得要 Start Of Authority, SOA 的标志了。

{% highlight text %}
$ dig -t soa baidu.com +noall +answer
baidu.com.              6810    IN      SOA     dns.baidu.com. sa.baidu.com. 2012139979 300 300 2592000 7200
{% endhighlight %}

上述查询的结果包含了七个参数，这七个参数的意义依次为：

* Master DNS 服务器主机名，也就是那个 DNS 服务器作为主；
* 管理员 email 地址，也就是发生问题后如何联系管理员，由于 `@` 有特殊含义，所以这里就写成了 `sa.baidu.com`；
* 序号 (Serial) 代表这个数据库档案的新旧，序号越大代表越新，Slave 会根据这个字段判断是否要更新配置；
* 更新频率 (Refresh) Slave 判断是否需要更新的时间间隔；
* 失败后的重新尝试时间 (Retry)；
* 失效时间 (Expire) 如果一直失败尝试时间，当重试到达这个设定值时限后，Slave 将不再继续尝试联机，并且尝试删除这份下载的 Zone file 信息；
* 默认的 TTL，如果这个数据库 Zone file 中，对应的 RR 记录没有设置 TTL 的话，那么就以这个 SOA 的设定值为准；

### 逆向查询

{% highlight text %}
$ dig -x 202.106.196.115 @8.8.8.8                     // 阅读性比较强
$ dig -t ptr 115.196.106.202.in-addr.arpa @8.8.8.8    // 实际最终转换为该查询
{% endhighlight %}

根据域名可以查到对应的 IP，但是再根据 IP 不一定能查到其域名，逆向查询需要配置 ptr 记录。

<!--
查询返回SOA记录

1. 当查询的类型不存在时，会在“AUTHORITY SECTION”返回SOA记录。

2.当查询的域名不存在时，会在“AUTHORITY SECTION”返回其上一层（有可能更上层，直到根）的zone的SOA记录。

也可能是二者的结合，例如上图中的dig -x 202.85.220.123，没有PTR类型，没有123.220.85.202.in-addr.arpa，返回了其上一级220.85.202.in-addr.arpa的SOA记录。
-->

## BIND 服务搭建

BIND 9 使用 C 语言实现，其设计实现几乎涵盖了服务器编程的所有细节，相关的信息可以参考官方网站[Bind The most widely used Name Server Software](http://www.isc.org/downloads/bind/) 。

可以直接安装，或者通过源码进行编译。

{% highlight text %}
$ ./configure --prefix=/data/sdns/named --enable-threads --with-openssl=no        // 源码编译
$ make
# make install

# yum intsll bind             # 直接安装
{% endhighlight %}

<!--
http://www.cnblogs.com/cobbliu/archive/2013/03/19/2970311.html
http://blog.chinaunix.net/uid-11121450-id-342114.html                     编译安装和配置bind(超级详细)
http://yuanbin.blog.51cto.com/363003/108572/                               BIND配置文件详解（一）
http://abbypan.github.io/2013/08/06/recursive-dns/                        递归DNS (recursive dns)
http://blog.51yip.com/server/1569.html                                    linux bind dns 正向解析 详解
http://blog.51yip.com/server/1348.html                                    linux dns服务器 安装配置详解


## 简介

### 解析结果类型

DNS 不只是提供了 IP 地址的解析，也可以是 CNAME 或者邮箱等等。

* A/AAAA (Address) 将域名指向的一个 IPv4/IPv6 地址；
* CNAME (Canonical Name) 将域名指向另外一个域名；
* NS (Name Server) 域名解析服务器记录，如果要将子域名指定某个域名服务器来解析，需要设置NS记录；
* MX (Mail eXchange) 指向邮件服务器地址；
* PTR (Pointer Record) 是 A 记录的逆向记录，又称做 IP 反查记录或指针记录，负责将 IP 反向解析为域名；
* SOA (Start Of Authority) SOA 标示多台 NS 记录中那一台是主服务器；

### 示例

----- 显示13个根域服务器
$ dig

----- 只返回结果的简短显示
$ dig -t a www.foobar.com +noall +answer
;; global options: +cmd
www.foobar.com.          21      IN      A       192.168.9.142
www.foobar.com.          21      IN      A       192.168.9.141

----- 指定记录类型，默认是 A 还可以是 MX、NS、SOA 等
$ dig -t a www.foobar.com


http://coolnull.com/3820.html
-->

{% highlight text %}
{% endhighlight %}
