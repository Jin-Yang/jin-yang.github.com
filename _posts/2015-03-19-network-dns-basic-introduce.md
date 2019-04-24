---
title: DNS 简介
layout: post
comments: true
language: chinese
category: [misc]
keywords: dns
description: 在通过浏览器访问某个网站时，或者说访问网络上的服务器时，可以直接使用 IP 地址，但是对于人类来说很难记忆，为此引入了域名。而为了可以做到自动解析，于时就有了 DNS 。
---

在通过浏览器访问某个网站时，或者说访问网络上的服务器时，可以直接使用 IP 地址，但是对于人类来说很难记忆，为此引入了域名。

而为了可以做到自动解析，于时就有了 DNS 。

<!-- more -->

## 简介

域名最初是通过手动配置的，也就是通过 `/etc/hosts` 进行配置，但是这种方法的缺陷是无法自动更新，尤其是当有大量服务器需要配置时，为了解决这一问题，于是就有了 DNS 。

最早是伯克利开发了一套分层的管理系统，称为 `Berkeley Internet Name Domain, BIND`，后来就是全世界使用最广泛的域名系统 `Domain Name System, DNS`。

首先介绍下常用的概念。

<!--
解析域名时，可以首先采用静态解析的方法，如果静态解析不成功，再采用动态解析的方法。
-->

### 常用概念

`Fully Qualified Domain Name, FQDN` 全限定域名包括了主机名 `Hostname` 和域名 `Domain Name`，通过符号 `.` 进行分割，主机名和域名是相对而言，如下图所示，不同的层级，主机名和域名也不同。

例如，假设提供 Web 服务的主机名为 `www` 而域名为 `foobar.com` ，那么对应的 FQDN 就是 `www.foobar.com` ，也就是我们最常见的地址。

当然，所谓的主机名并非只是一台主机。

![dns examples]({{ site.url }}/images/network/dns-host-domain-name.gif "dns examples"){: .pull-center}

需要注意的是，并不是以小数点 `.` 区分主机名和域名，主机名可以为 `www.dic` ，而域名还是 `ksu.edu.tw` ，因此此时全名就为 `www.dic.ksu.edu.tw` 。

### DNS 的阶层架构与 TLD

仍以 `ksu.edu.tw` 域名为例，如下图所示。

![dns examples]({{ site.url }}/images/network/dns-dot-gtld-cctld.gif "dns examples"){: .pull-center}

在整个 DNS 系统的最上方一定是根服务器，根服务器管理了最上层的域名 (`Generic TLDs, gTLD` 如 `.com` `.org` `.gov` 等等) 以及国家最上层领域名 (`Country code TLDs, ccTLD` 如 `.tw` `.uk` `.jp` `.cn` 等) ，这两者称为 `Top Level Domains, TLDs` 。

每一层需要向上层 ISP 申请，如 `.tw`，管理这个领域名的机器 IP 是在台湾，但是 `.tw` 这部服务器必须向 root `.` 注册领域名查询授权才行。各层 DNS 都能管理自己辖下的主机名或子领域，因此，`.tw` 可以自行规划自己的子域名。

<!--
如上所属，DNS 系统采用的是分层式的管理，例如，.tw 只记录了底下那一层的数个主要的 domain 的主机，例如 edu.tw 底下有个 ksu.edu.tw 这部机器，那就直接授权交给 edu.tw 那部机器去管理了。
-->

每个上一层的 DNS 服务器所记录的信息，其实只有其下一层的主机名而已；至于再下一层，则直接授权给下层的某部主机来管理。

### 顶层服务器

目前，世界上只有 13 个顶级根域名服务器，可以通过 [www.internic.net](http://www.internic.net/domain/named.root) 查看。需要注意的是，我们所说 13 个根 DNS 服务器，指的是逻辑上有 13 个，而不是物理上 13 个服务器。

实际上，目前物理上有几百个根服务器，大部分通过任播技术与逻辑服务器对应，当前的根服务器可以通过 [www.root-servers.org](http://www.root-servers.org/) 查看，那么，为什么全球 DNS 根服务器只有 13 组。

DNS 协议最初从 20 世纪 80 年代未期开始算起，使用了端口上的 UDP 和 TCP 协议；UDP 用于查询和响应，TCP 用于主从服务器之间的区传送。

在 Internet 数据传输中，UDP 数据长度控制在 576 字节 (Internet 标准 MTU 值)，而在许多 UDP 应用程序设计中数据包被限制成 512 字节或更小，这样可以防止数据包的丢失。

而 512 字节对于在每个包中必须含有数字签名的一些 DNS 新特性（如DNSSEC）来说实在是太小了。如果要让所有的根服务器数据能包含在一个 512 字节的 UDP 包中，根服务器只能限制在 13 个，而每个服务器要使用字母表中的单个字母命名。

<!--
采用 IP Anycast+BGP 进行部署，采用 UDP 协议，一般小于 512-bytes 。
一条记录大概几十个字节，为了能通过一个包传输完，顶级通常为 13 组。

以太网数据的长度必须在46-1500字节之间，这是由以太网的物理特性决定的。

事实上，这个1500字节就是网络层IP数据包的长度限制，理论上，IP数据包最大长度是65535字节。

这是由IP首部16比特总长度所限制的，去除20字节IP首部和8个字节UDP首部，UDP数据包中数据最大长度为65507字节。

许多解析器首先发送一条UDP查询，如果它们接收到一条被截断的响应，则会用TCP重新发送该查询。

这个过程绕过了512字节的限制，但是效率不高。您或许认为DNS应该避开UDP，总是使用TCP，但是TCP连接的开销大得多。

一次UDP名字服务器交换可以短到两个包：一个查询包、一个响应包。一次TCP交换则至少包含7个包：三次握手初始化TCP会话、一个查询包、一个响应包以及最后一次握手来关闭连接。
-->

<!--
## DNS 分类

DNS 按功能或者角色，可以分为如下的几类：

1. 权威DNS：

权威DNS是经过上一级授权对域名进行解析的服务器，同时它可以把解析授权转授给其他人，如COM顶级服务器可以授权xxorg.com这个域名的的权威服务器为NS.ABC.COM，同时NS.ABC.COM还可以把授权转授给NS.DDD.COM，这样NS.DDD.COM就成了ABC.COM实际上的权威服务器了。平时我们解析域名的结果都源自权威DNS。比如xxorg.com的权威DNS服务器就是dnspod的F1G1NS1.DNSPOD.NET和F1G1NS2.DNSPOD.NET。

2.递归DNS:

负责接受用户对任意域名查询，并返回结果给用户。递归DNS的工作过程参见本文第二节。递归DNS可以缓存结果以避免重复向上查询。我们平时使用最多的就是这类DNS，他对公众开放服务，一般由网络运营商提供，大家都自己可以架递归DNS提供服务。递归DNS一定要有可靠的互联网连接方可使用。比如谷歌的8.8.8.8和8.8.4.4以及114的114.114.114.114和114.114.115.115都属于这一类DNS。你本地电脑上设置的DNS就是这类DNS。醒醒的习惯是设置114.114.114.114和8.8.8.8这两个，更可靠一点。

3.转发DNS:

负责接受用户查询，并返回结果给用户。但这个结果不是按标准的域名解析过程得到的，而是直接把递归DNS的结果转发给用户。它也具备缓存功能。他主要使用在没有直接的互联网连接，但可以连接到一个递归DNS那里，这时使用转发DNS就比较合适。其缺陷是：直接受递归DNS的影响，服务品质较差。比如我们用的路由器里面的DNS就是这一类，用路由器的朋友可以看下本地电脑的DNS一般都是192.168.1.1。
-->


## 记录类型

在 DNS 中最常见的是 Internet 资源，包括了 4 个元组：Name、Value、Type、TTL。其中 TTL 表示生存时间，决定了多长时间会从缓存中删除；Type 决定了 Name 和 Value，常见有如下几种：

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






## 相关介绍

其实就是所谓的 "." 或者根域，是指网址 www.baidu.com 在配置中应该是 www.baidu.com. 只是一般在浏览器输入时会省略后面的点。

如上所述，根域服务器只是具有 13 个 IP 地址，但机器数量却不是 13 台，这些 IP 地址借助了任播的技术，所以我们可以在全球设立这些 IP 的镜像站点，你访问到的这个 IP 并不是唯一的那台主机。<br><br>

每个域都会有域名服务器，也叫权威域名服务器，例如，baidu.com 是一个顶级域名，而 www.baidu.com 却不是顶级域名，他是在 baidu.com 这个域里的一叫做 www 的主机。

一级域之后还有二级域，三级域，只要我买了一个顶级域，并且我搭建了自己 BIND 服务器注册到互联网中，那么我就可以随意在前面多加几个域了，只是长度是有限制的。

比如 a.www.baidu.com 这个网址中，www.baidu.com 变成了一个二级域而不是一台主机，主机名是 a。<br><br>


由于根域名服务器在这个系统过的最顶层，无法按照常规方法对其授权，到目前为止其授权方法是把所有根服务器列表存放在一个文本文件内，通常为 root.hist 。
-->


## 解析实战

接下来我们看看具体查询。

#### 1. 查询 DNS 服务器 IP

我们的电脑通过 ISP 接入互联网，ISP 会分配一个 DNS 服务器，这个 DNS 服务器不是权威服务器，而是一个代理 DNS 服务器，用来代理查询 IP，可以查看 `/etc/resolv.conf` 文件。

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

<!--
假设目前在互联网上域名解析授权大体上是谁出售域名就把域名的权威DNS授权给谁并由其提供域名的权威DNS来完成域名解析工作，如购买了新网域名默认就是由新网的权威DNS（nsx.xinnetdns.com、nsx.xinnet.cn）负责所售域名解析:

 附加部分1：慎用WHOIS来查看域名权威DNS。
对于域名的Whois数据库是由域名销售商控制的，即每个域名销售商都有自己的WHOIS服务器，这些服务器用来存储自身出售的域名信息，如域名所有人，联系方法，到期时间等内容。WHOIS信息中显示的域名当前权威DNS信息很可能没有及时与域名实际的权威DNS信息同步而导致错误的判断。
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
-->


## 常用操作

现在使用的 DNS 服务器地址可以通过 `cat /etc/resolv.conf` 查看。

### nslookup

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

### dig

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

#### 简单查询

{% highlight text %}
$ dig -t a www.baidu.com +noall +answer
www.baidu.com.          969     IN      CNAME   www.a.shifen.com.
www.a.shifen.com.       189     IN      A       14.215.177.39
www.a.shifen.com.       189     IN      A       14.215.177.38
{% endhighlight %}

首先 `www.baidu.com` 指向的是另外一个域名 (估计是做负载均衡用的) ，然后才是两条 A 记录。

##### Tips

可以看到，输出的结果后面多了一个 `.` ，实际上这里的真正域名是 `www.baidu.com.root` ，只是所有的根域名 `.root` 都相同，一般直接省略。

根域名下一级是 "顶级域名" (Top Level Domain, TLD)，例如 `.com`、`.net`；再下一级叫做 "次级域名" (Second Level Domain, SLD)，例如上述的 `.baidu`，这一级域名是可以注册的。

再下一级是主机名 (Host) 由用户统一管理，可以任意分配，例如上面的 `www` 。

#### 查询 NS 域名

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

#### SOA 信息查询

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

#### 逆向查询

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


## FAQ

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

#### resolv.conf 配置

注意，dig、nslook 只会解析 `/etc/resolv.conf` 的内容，而不会解析 `/etc/hosts` 里面内容，可以使用 `getent hosts baidu.com` 查看。

{% highlight text %}
options timeout:2 attempts:3 rotate single-request-reopen
{% endhighlight %}

Linux 系统下域名解析的配置文件是 `/etc/resolv.conf`，一般会配置上两个或更多的 `nameserver`，这样在一个服务器挂掉后还能正常解析域名。

但是失败后重试的场景和策略是什么呢？

{% highlight text %}
options timeout:1 attempts:1 rotate
nameserver 10.0.0.1
nameserver 10.0.0.2
nameserver 10.0.0.3
{% endhighlight %}

这里大概讲下几个选项的含义，详细可以通过 `man 5 resolv.conf` 查看：

* nameserver DNS服务器的IP地址，最多能设三个
* timeout 查询一个NS的超时时间，单位是秒，默认是5，最大为30；
* attempts 所有服务器查询的整个都尝试一遍的次数，默认是2；
* rotate 随机选取一个作为首选DNS服务器，默认是从上到下；

另外，也可以在本机起 dnsmasq 并监听本地的 UDP 53 端口，用来监听来自于本地的解析请求，该进程会维护上游服务器的健康状况，不会把解析请求发到挂掉的上游服务器上。

## 参考

与 DNS 协议的相关 RFC 文档有，RFC1034-(DOMAIN NAMES - CONCEPTS AND FACILITIES)、RFC1035-(DOMAIN NAMES - IMPLEMENTATION AND SPECIFICATION)。

关于 DNS 介绍可以参考 [鸟哥的私房菜 - DNS 服务器](http://vbird.dic.ksu.edu.tw/linux_server/0350dns.php)，或者 [DNS HowTo](http://www.tldp.org/HOWTO/DNS-HOWTO.html)，一个简单的搭建 BIND 服务器的介绍，或者参考 [BIND for the Small LAN](https://www.madboa.com/geek/soho-bind/) 。

关于 dig 命令的帮助可以查看 [DiG HOWTO](https://www.madboa.com/geek/dig/)，其中包括了一个用于检测本地 DNS 配置是否正确的脚本。

其他的一些常见工具包括 [阿里昆仑用户诊断工具](http://tool.alikunlun.com/doc.html)、[腾讯华佗诊断工具](http://utp.qq.com/)；[阿里公共DNS解析服务](http://www.alidns.com/)，及其帮助文档 [FAQs](http://www.alidns.com/faqs/)，其他公共 DNS 解析服务还有 114；[根服务器分布](http://www.root-servers.org/)；[淘宝IP地址库](http://ip.taobao.com) 。

<!-- http://jwx.zgz.cn/cl/7.41.htm -->

<!--


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

https://klionsec.github.io/2017/12/11/Dns-tips/


<a href="http://me2xp.blog.51cto.com/6716920/1538163">DNS服务及基于BIND的实现</a> 一个不错的与 BIND 相关的介绍，也可以参考 <a href="reference/dns/DNS服务及基于BIND的实现.html">本地文档</a> 。<br><br>
BIND Response Policy Zone (RPZ)    BIND9 的新特性
范域名

http://www.webhosting.info/


BGP   EBGP/IBGP/IGP有何区别  自治系统

http://www.educity.cn/linux/1582091.html


http://coolnull.com/3820.html
http://blog.51cto.com/122269875/1713947
http://roclinux.cn/?p=2449
-->

{% highlight text %}
{% endhighlight %}
