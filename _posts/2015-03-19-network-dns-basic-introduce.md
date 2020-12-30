---
title: 详细介绍 DNS 的基本概念
layout: post
comments: true
language: chinese
tag: [Protocol, Network, DevOps]
keywords: dns
description: 在通过浏览器访问某个网站时，或者说访问网络上的服务器时，可以直接使用 IP 地址，但是对于人类来说很难记忆，为此引入了域名，而为了可以做到自动解析，于时就有了 DNS ，这里详细介绍其基本概念。
---

在通过浏览器访问某个网站时，或者说访问网络上的服务器时，可以直接使用 IP 地址，但是对于人类来说很难记忆，为此引入了域名，而为了可以做到自动解析，于时就有了 DNS 。

这里详细介绍 DNS 的基本概念。

<!-- more -->

## 简介

域名最初是通过手动配置的，在 Linux 中就是通过 `/etc/hosts` 文件进行配置，但是这种方法的缺陷是无法自动更新，尤其是当有大量服务器需要配置时，为了解决这一问题，于是就有了 DNS 。

最早是伯克利开发了一套分层的管理系统，称为 `Berkeley Internet Name Domain, BIND`，后来就是全世界使用最广泛的域名系统 `Domain Name System, DNS`。

首先介绍下常用的概念。

### 常用概念

`Fully Qualified Domain Name, FQDN` 全限定域名包括了主机名 `Hostname` 和域名 `Domain Name`，通过符号 `.` 进行分割，主机名和域名是相对而言，如下图所示，不同的层级，主机名和域名也不同。

例如，假设提供 Web 服务的主机名为 `www` 而域名为 `foobar.com` ，那么对应的 FQDN 就是 `www.foobar.com` ，也就是我们最常见的地址。

当然，所谓的主机名并非只是一台主机。

![dns examples](/{{ site.imgdir }}/network/dns-host-domain-name.gif "dns examples")

需要注意的是，并不是以小数点 `.` 区分主机名和域名，主机名可以为 `www.dic` ，而域名还是 `ksu.edu.tw` ，因此此时全名就为 `www.dic.ksu.edu.tw` 。

### DNS 的阶层架构与 TLD

仍以 `ksu.edu.tw` 域名为例，如下图所示。

![dns examples](/{{ site.imgdir }}/network/dns-dot-gtld-cctld.gif "dns examples")

在整个 DNS 系统的最上方一定是根服务器，管理了最上层的域名 (`Generic TLDs, gTLD` 如 `.com` `.org` `.gov` 等等) 以及国家最上层领域名 (`Country code TLDs, ccTLD` 如 `.tw` `.uk` `.jp` `.cn` 等) ，这两者称为 `Top Level Domains, TLDs` 。

每一层需要向上层 ISP 申请，如 `.tw`，管理这个领域名的机器 IP 是在台湾，但是 `.tw` 这部服务器必须向 root `.` 注册领域名查询授权才行。各层 DNS 都能管理自己辖下的主机名或子领域，因此，`.tw` 可以自行规划自己的子域名。

每层 DNS 服务器所记录的信息，其实只有其下层的主机名而已；至于再下一层，则直接授权给下层的主机来管理。

### 顶层服务器

目前，世界上只有 13 个顶级根域名服务器，可以通过 [www.internic.net](http://www.internic.net/domain/named.root) 查看。需要注意的是，这里所说 13 个根 DNS 服务器，指的是逻辑上有 13 个，而不是物理上 13 个服务器。

实际上，目前物理上有几百个根服务器，大部分通过任播技术与逻辑服务器对应，当前的根服务器可以通过 [www.root-servers.org](http://www.root-servers.org/) 查看，那么，为什么全球 DNS 根服务器只有 13 组。

DNS 协议最初从 20 世纪 80 年代未期开始算起，使用了端口上的 UDP 和 TCP 协议；UDP 用于查询和响应，TCP 用于主从服务器之间的区传送。

在 Internet 数据传输中，UDP 数据长度控制在 576 字节 (Internet 标准 MTU 值)，而在许多 UDP 应用程序设计中数据包被限制成 512 字节或更小，这样可以防止数据包的丢失。

而 512 字节对于在每个包中必须含有数字签名的一些 DNS 新特性（如DNSSEC）来说实在是太小了。如果要让所有的根服务器数据能包含在一个 512 字节的 UDP 包中，根服务器只能限制在 13 个，而每个服务器要使用字母表中的单个字母命名。

注意，大部分的客户端会先通过 UDP 查询，如果发现被截断，那么会重新使用 TCP 再次查询，这样就可以绕过 512 字节的限制，但是会降低查询效率。

## 解析过程详解

接下来详细查看下 DNS 解析的具体执行过程。

#### 1. 查询 DNS 服务器 IP

我们的电脑通过 ISP 接入互联网，ISP 会分配一个 DNS 服务器，这个 DNS 服务器不是权威服务器，而是一个代理 DNS 服务器，用来代理查询 IP，可以查看 `/etc/resolv.conf` 文件。

注意，老的 Linux 发行版本，该文件是手动编辑的，现在很多 (如 CentOS) 是自动生成的，详见 [resolv.conf 简介](/post/network-dns-resolv-conf-usage-introduce.html)。

#### 2. 发起请求

向上述的 DNS 服务发起请求查询 `www.baidu.com` 这个域名了，当然详细信息可以通过 `dig +trace www.baidu.com` 查看，不过在国内的环境经常出错，那我们就一步一步来查看。

#### 3. 如果有缓存直接返回

上述服务器收到请求后，如果自己缓存中有这个地址，则直接返回，且标记为非权威服务器应答。

#### 4. 没有命中，则从根开始查询

如果缓存中没有，则会开始查询 13 个[根域名服务器的地址](https://www.internic.net/domain/named.root)，这些地址是不变的，可以直接保存在 BIND 的配置文件中。

可以直接通过如下方式查询。

{% highlight text %}
$ dig @a.root-servers.net root-servers.net ns
ROOT-SERVERS.NET.       141973  IN      NS      h.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      i.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      k.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      b.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      j.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      g.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      e.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      l.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      m.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      c.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      d.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      f.ROOT-SERVERS.NET.
ROOT-SERVERS.NET.       141973  IN      NS      a.ROOT-SERVERS.NET.
{% endhighlight %}

#### 5. 向其中一台根服务器发起请求

根服务器拿到这个请求后，知道他是 `com.` 这个顶级域名下的，所以就会返回 `com` 域中的 NS 记录，通常是 13 台主机名和 IP 。

{% highlight text %}
$ dig com. ns                # 同下
$ dig @a.root-servers.net com ns
com.                    17051   IN      NS      i.gtld-servers.net.
com.                    17051   IN      NS      m.gtld-servers.net.
com.                    17051   IN      NS      l.gtld-servers.net.
com.                    17051   IN      NS      j.gtld-servers.net.
com.                    17051   IN      NS      b.gtld-servers.net.
com.                    17051   IN      NS      a.gtld-servers.net.
com.                    17051   IN      NS      k.gtld-servers.net.
com.                    17051   IN      NS      h.gtld-servers.net.
com.                    17051   IN      NS      c.gtld-servers.net.
com.                    17051   IN      NS      d.gtld-servers.net.
com.                    17051   IN      NS      g.gtld-servers.net.
com.                    17051   IN      NS      f.gtld-servers.net.
com.                    17051   IN      NS      e.gtld-servers.net.
{% endhighlight %}

#### 6. 向顶级域名服务器发送请求

同样是向其中一台发起请求，请求 `baidu.com` 这个域的域名服务器。

{% highlight text %}
$ dig @e.gtld-servers.net baidu.com ns
baidu.com.              9141    IN      NS      dns.baidu.com.
baidu.com.              9141    IN      NS      ns7.baidu.com.
baidu.com.              9141    IN      NS      ns2.baidu.com.
baidu.com.              9141    IN      NS      ns3.baidu.com.
baidu.com.              9141    IN      NS      ns4.baidu.com.
{% endhighlight %}

#### 7. 再次发送请求

再次向 `baidu.com` 这个域的权威服务器发起请求，`baidu.com` 收到之后，查了下有 `www` 的这台主机，就把这个 IP 返回给你了。

{% highlight text %}
$ dig @dns.baidu.com www.baidu.com
www.baidu.com.          547     IN      CNAME   www.a.shifen.com.
www.a.shifen.com.       144     IN      A       220.181.111.188
www.a.shifen.com.       144     IN      A       220.181.112.244
{% endhighlight %}

#### 8. 返回给客户

拿到了 IP 之后将其返回给了客户端，并且把这个保存在高速缓存中。

## 服务器分类

在如上介绍解析流程的时候，会有关于所谓权威服务器的介绍，这里介绍常见概念。

### 权威 DNS

权威 DNS 是经过上一级授权对域名进行解析的服务器，每个域都会有域名服务器，也叫权威域名服务器，例如 `baidu.com` 是一个域名，而 `www.baidu.com` 主机就是这个域管理的主机。

<!--
同时它可以把解析授权转授给其他人，如COM顶级服务器可以授权xxorg.com这个域名的的权威服务器为NS.ABC.COM，同时NS.ABC.COM还可以把授权转授给NS.DDD.COM，这样NS.DDD.COM就成了ABC.COM实际上的权威服务器了。平时我们解析域名的结果都源自权威DNS。比如xxorg.com的权威DNS服务器就是dnspod的F1G1NS1.DNSPOD.NET和F1G1NS2.DNSPOD.NET。
-->

### 递归 DNS

会接受用户对任意域名查询，并返回结果给用户，一般会缓存查询结果以避免重复向上查询。

平时使用最多的就是这类 DNS 服务器，会对公众开放服务，一般由网络运营商或者互联网厂商提供，比如谷歌的 `8.8.8.8` 就属于这一类 DNS 。

### 转发 DNS

同样负责接受用户查询，并返回结果给用户，这个结果不是按标准的域名解析过程得到的，而是直接把递归 DNS 的结果转发给用户，同样具备缓存功能。

<!--
他主要使用在没有直接的互联网连接，但可以连接到一个递归DNS那里，这时使用转发DNS就比较合适。其缺陷是：直接受递归DNS的影响，服务品质较差。
-->

比如路由器里面的 DNS 就是这一类。


## 记录类型

在 DNS 中最常见的是 Internet 资源，包括了 4 个元组：`Name`、`Value`、`Type`、`TTL`。其中 `TTL` 表示生存时间，决定了多长时间会从缓存中删除；`Type` 决定了 `Name` 和 `Value`，常见有如下几种：

* A 记录，Address。用于描述域名到 IP 地址的映射，同一个域名可以对应多个记录，也就是一次 DNS 查询可以返回多个 IP，从而可以实现基本的流量均衡。
* NS 记录，Name Server。域名服务器记录，用于说明这个区域有那些 DNS 服务器来负责解析，每个域可以有多个域名服务器。
* SOA 记录，Start Of Authority。用于指定该域的权威域名服务器，负责说明负责解析的 DNS 服务器中那个是主服务器，只允许有一个 SOA 记录，它是资源记录的第一个条目。
* CNAME 记录。描述别名和域名的关系，从而可以允许将多个名字映射到同一台服务器。例如，一台服务器同时提供 WWW 和 MAIL 服务，可以提供两个 www.foobar.com mail.foobar.com 别名同时指向 host.foobar.com 。
* MX 记录。全称是邮件交换记录，在使用邮件服务器的时候，MX 记录是无可或缺。比如 A 用户向 B 用户发送一封邮件，那么他需要向 DNS 查询 Ｂ 的 MX 记录。

<!--
SRV记录
SRV记录是服务器资源记录的缩写，SRV记录是DNS记录中的新鲜面孔，在RFC2052中才对SRV记录进行了定义，因此很多老版本的DNS服务器并不支持SRV记录。那么SRV记录有什么用呢？SRV记录的作用是说明一个服务器能够提供什么样的服务！SRV记录在微软的Active Directory中有着重要地位，大家知道在NT4时代域和DNS并没有太多关系。但从Win2000开始，域就离不开DNS的帮助了，为什么呢？因为域内的计算机要依赖DNS的SRV记录来定位域控制器！表现形式为：—ldap._tcp.contoso.com 600 IN SRV 0 100 389 NS.contoso.com
ladp: 是一个服务，该标识说明把这台服务器当做响应LDAP请求的服务器
tcp：本服务使用的协议，可以是tcp，也可以是用户数据包协议《udp》
contoso.com：此记录所值的域名
600： 此记录默认生存时间（秒）
IN： 标准DNS Internet类
SRV：将这条记录标识为SRV记录
0： 优先级，如果相同的服务有多条SRV记录，用户会尝试先连接优先级最低的记录
100：负载平衡机制，多条SRV并且优先级也相同，那么用户会先尝试连接权重高的记录
389：此服务使用的端口
NS.contoso.com:提供此服务的主机

PTR记录
PTR记录也被称为指针记录，PTR记录是A记录的逆向记录，作用是把IP地址解析为域名。由于我们在前面提到过，DNS的反向区域负责从IP到域名的解析，因此如果要创建PTR记录，必须在反向区域中创建。

http://www.laojia1987.cn/show.asp?id=756
NS、SOA 记录是任何一个 DNS 区域都不可或缺的两条记录。

## 主机名查询流程

如上所述，DNS 利用类似树状目录的架构，将主机名的管理分配在不同层级的 DNS 服务器当中，经由分层管理， 所以每一部 DNS 服务器记忆的信息就不会很多，而且若有 IP 异动时也相当容易修改！

因为你如果已经申请到主机名解析的授权， 那么在你自己的 DNS 服务器中，就能够修改全世界都可以查询到的主机名了！而不用透过上层 ISP 的维护！

![dns examples]({{ site.url }}/images/network/dns-search-process.gif "dns examples"){: .pull-center}

当在浏览器中输入 www.ksu.edu.tw 时，会根据 /etc/nsswitch.conf 中的配置，通常是先查询 /etc/hosts，如果没有命中开始查看 DNS 系统。

一般来说会依据 /etc/resolv.conf 所提供的 DNS 的 IP 去查询，上述域名指定的 IP 地址。

1. 服务器收到用户的查询要求后，先查看本身有没有纪录，若无则向 . 查询。由于 DNS 是阶层式的架构，每部主机都会管理自己辖下的主机名解译而已，当发现没有相应的域名时，就直接向 .(root) 的服务器查询相关 IP 信息。

2. 向最顶层的 .(root) 查询。当向 root 查询时，但是由于 . 只记录了 .tw 的信息，此时 root 会返回消息说 "我不知道这部主机的 IP，应该向 .tw 去询问才对，我告诉你 .tw 在哪里"。

3. 向第二层的 .tw 服务器查询。于是，又向 .tw 去查询，而该部机器管理的又仅有 .edu.tw, .com.tw, gov.tw... 那几部主机，经过比对后发现我们要的是 .edu.tw 的网域，所以这个时候 .tw 又说："你要去管理 .edu.tw 这个网域的主机那里查询，我有他的 IP"。

4. 向第三层的 .edu.tw 服务器查询。同理，.edu.tw 只会说应该要去 .ksu.edu.tw 进行查询，这里只能告知 .ksu.edu.tw 的 IP 而已。

5. 向第四层的 .ksu.edu.tw 服务器查询。当找到 .ksu.edu.tw 之后，Bingo！.ksu.edu.tw 说："没错！这部主机名是我管理的～ 我跟你说他的 IP 是...所以此时就能够查到 www.ksu.edu.tw 的 IP 了"。

6. 记录暂存内存并回报用户。我们发现，如果没有记录，那么查询的过程非常耗时。因此，查到了正确的 IP 后，DNS 服务器会缓存该地址，通常会保存一段时间，当然这个时间可配，通常可能是 24 小时。

整个分层查询的流程大致就是这样，如果没有记录，则要经过 root 来向下一层进行查询，最终总是能得到答案的。这样分层的好处是：

* 主机名修改的仅需自己的 DNS 更动即可，不需通知其他人。

* DNS 服务器对主机名解析结果通常会缓存 10min~3day 不等，也就是说当修改了域名后，可能要 2 ~ 3 天后才能生效。

* 可持续向下授权 (子领域名授权)，类似于第一条，如果要添加 idb.tw 这个域名，只需要向 .tw 申请。
-->





## FAQ

#### 根域

其实就是所谓的 `.` 或者根域，是指网址 `www.baidu.com` 在配置中应该是 `www.baidu.com.` 只是一般在浏览器输入时会省略后面的点。

<!--
由于根域名服务器在这个系统过的最顶层，无法按照常规方法对其授权，到目前为止其授权方法是把所有根服务器列表存放在一个文本文件内，通常为 root.hist 。
-->

#### A 记录与 CNAME 记录的区别？

A 记录是把一个域名解析到一个 IP 地址，而 CNAME 记录是把域名解析到另外一个域名，而这个域名最终会指向一个 A 记录。

当一台机器提供了多个服务时，这种方式比较容易修改。将多个域名映射到一个相同域名，然后只需要修改被映射的域名即可。

#### CNAME 的使用

比较常见的一个是 Github 提供的项目 Pages 的支持，一般是类似 `PROJECT.github.io` 的域名，实际上指向的是一个 CNAME 配置，最终为同一个域名。

根据解析之后的 A 地址，对应的 Web 服务器就可以进行解析处理了。因为基本上就是个静态页面，所以很容易做一些缓存的策略。

#### PTR 用途

PTR 用于从 IP 地址反查域名，可以使用 `dit -x IP` 进行反查。

其中的一个场景是防止垃圾邮件，即验证发送邮件的 IP 地址，是否真的有它所声称的域名。

#### TTL 值

Time To Live, TTL 生存时间，表示解析记录在 DNS 服务器中的缓存时间，默认是 3600 秒。

例如在访问 `www.foobar.com` 时，如果在 DNS 服务器的缓存中没有该记录，就会向 NS 服务器发出请求，获得该记录后，该记录会在 DNS 服务器上缓存 TTL 的时间长度，在 TTL 有效期内访问 `www.foobar.com` 时，会直接返回。

#### DNS 服务器地址

通过 `dig` 命令查询时，默认会依次查询 `/etc/resolv.conf` 文件中的地址，至于是从哪个 DNS 服务器地址查询到，可以查看返回结果中的 `SERVER` 字段，例如：

{% highlight text %}
;; Query time: 1 msec
;; SERVER: 10.0.52.180#53(10.0.52.180)
;; WHEN: Thu May 09 15:38:04 CST 2017
;; MSG SIZE  rcvd: 101
{% endhighlight %}

当然，可以在执行时通过 `@` 指定，例如如下的方式。

{% highlight text %}
$ dig @8.8.8.8 www.baidu.com
{% endhighlight %}

## 参考

与 DNS 协议的相关 RFC 文档有，RFC1034-(DOMAIN NAMES - CONCEPTS AND FACILITIES)、RFC1035-(DOMAIN NAMES - IMPLEMENTATION AND SPECIFICATION)。

关于 DNS 介绍可以参考 [鸟哥的私房菜 - DNS 服务器](http://vbird.dic.ksu.edu.tw/linux_server/0350dns.php)，或者 [DNS HowTo](http://www.tldp.org/HOWTO/DNS-HOWTO.html)，一个简单的搭建 BIND 服务器的介绍，或者参考 [BIND for the Small LAN](https://www.madboa.com/geek/soho-bind/) 。

关于 dig 命令的帮助可以查看 [DiG HOWTO](https://www.madboa.com/geek/dig/)，其中包括了一个用于检测本地 DNS 配置是否正确的脚本。

其他的一些常见工具包括 [阿里昆仑用户诊断工具](http://tool.alikunlun.com/doc.html)、[腾讯华佗诊断工具](http://utp.qq.com/)；[阿里公共DNS解析服务](http://www.alidns.com/)，及其帮助文档 [FAQs](http://www.alidns.com/faqs/)，其他公共 DNS 解析服务还有 114；[根服务器分布](http://www.root-servers.org/)；[淘宝IP地址库](http://ip.taobao.com) 。

