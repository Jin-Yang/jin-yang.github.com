---
Date: October 19, 2013
title: DNSmasq 使用
layout: post
comments: true
language: chinese
category: [linux]
---

通常上网使用的 DNS 服务器都是由电信、联通等运营商提供的，或者是公用的 DNS 解析服务器，这通常会带来些问题，如有时解析不正常、经常弹出广告等。

有时我们需要管理 DNS 解析，如加速 DNS 的解析过程、纠正错误的 DNS 解析记录、配置局域网的 DNS 解析等等。

DNSmasq 就是其中的一个解决方案，相比 BIND 来说，其更加小巧，配置更加方便，其源码仅有 2 万多行，但足以满足常见的需求。

<!-- more -->

## 简介

DNSmasq 是一个小巧且方便地用于配置 DNS 和 DHCP 的工具，它提供了 DNS 功能和可选择的 DHCP 功能，它服务那些只在本机和局域网可用的域名，这些域名是不会在全球的 DNS 服务器中出现的，既不能通过外网访问。

一般情况下，可以用 `bind` 解决 dns 的问题；`dhcpd` 解决 dhcp 的问题；用 `ypbind` 解决自定义 hostname 解析的 ip，而 DNSmasq 已经解决了所有的这些功能，而且配置简单，更适用于虚拟化和大数据环境的部署。

可以使用 dnsmasq 通过缓存来加速 dns 解析，提高上网速度；比 hosts 更强大的域名重定向功能，为在本机测试二级域名提供了很大的方便。

详细的内容可以参考官方网站 [www.thekelleys.org.uk](http://www.thekelleys.org.uk/dnsmasq/doc.html) 。

### 安装

在 CentOS 中，可以通过如下方式安装 dnsmasq 。

{% highlight text %}
# yum install dnsmasq                  // CentOS
# netstat -atunp | grep dnsmasq        // 查看系统是否启动
# systemctl [start|restart] dnsmasq    // 重启等操作
{% endhighlight %}

dnsmasq 的配置文件为 `/etc/dnsmasq.conf` 。

## DHCP 服务器配置

DHCP 的工作过程主要分为以下六个阶段：

1. 发现阶段，DHCP 客户端通过广播 DHCP DISCOVER 消息，寻找 DHCP 服务器。
2. 提供阶段，所有收到 Discover 消息的 DHCP 服务器都会作出响应，包括了出租的 IP 地址在内的 DHCP OFFER 消息。
3. 选择阶段，客户端会选择第一个收到的 Offer 消息，并以广播方式应答一个 DHCP REQUEST 消息，那么没有被选中的服务器将回收之前发出的 IP 地址。
4. 确认阶段，服务器收到客户端应答的 Request 消息后，便向客户端再次发送一个包含 IP 地址的 DHCP ACK 确认消息。此时，客户端可以使用服务器提供的 IP 地址与网卡绑定。
5. 重新登录阶段，以后客户端重新设置网络时，直接发送包括之前分配 IP 地址的 DHCP REQUEST 信息；服务器收到后，如果可以继续使用该 IP 则返回 ACK 确认消息。否则，返回 NACK 消息，客户端将重新发起一次 Discover 流程。
6. 更新租约阶段，在客户端启动以及 IP 租约期限过一半时，客户端都会自动向服务器发送更新其 IP 租约的消息。

接下来，我们看下 DNSmasq 是如何配置 DHCP 服务器的，首先查看本机的 DHCP 服务是否已经启动。

{% highlight text %}
----- DHCP服务器默认使用53号端口
# grep -E '^domain\>' /etc/services
domain          53/tcp                          # name-domain server
domain          53/udp

----- 查看服务器是否启动
# netstat -tnlp | grep dnsmasq
Proto Recv-Q Send-Q Local Address       Foreign Address    State    PID/Program name
tcp        0      0 192.168.122.1:53    0.0.0.0:*          LISTEN   2623/dnsmasq
{% endhighlight %}

如下是其配置文件。

{% highlight text %}
# 服务监听的网络接口地址
listen-address=192.168.1.132,127.0.0.1
# DNS监听端口，如果设置为0则取消DNS功能
port=5353

# 设置默认租期
dhcp-lease-max=150

# dhcp动态分配的地址范围，包括了地址范围，以及租约时间
dhcp-range=192.168.1.50,192.168.1.150,48h

# dhcp服务的静态绑定，包括 MAC 地址、IPv4、主机名
dhcp-host=00:0C:29:5E:F2:6F,192.168.1.201,os02
dhcp-host=00:0C:29:15:63:CF,192.168.1.202,os03

# 租期保存在下面文件
# The DHCP server needs somewhere on disk to keep its lease database.
# This defaults to a sane location, but if you want to change it, use
# the line below.
#dhcp-leasefile=/var/lib/dnsmasq/dnsmasq.leases

# 忽略下面MAC地址的DHCP请求
#dhcp-host=11:22:33:44:55:66,ignore
{% endhighlight %}

<!--
### Rogue DHCP

有时会发现 IP 获取有问题，那么就有可能是在该网络中存在多个 DHCP 服务器导致的。

在 Windows 中可以通过
-->

## 配置DNS服务

dnsmasq 能够缓存外部 DNS 记录，同时也可以提供本地 DNS 解析或者作为外部 DNS 的代理，也就是说实际上 dnsmasq 支持首先查找 `/etc/hosts` 等本地解析文件，然后再查找 `/etc/resolv.conf` 等外部 nameserver 配置文件中定义的外部 DNS。

所以说 dnsmasq 是一个很不错的 DNS 中继，DNS 配置同样写入 `dnsmasq.conf` 配置文件里。

{% highlight text %}
# 服务监听的网络接口地址，可以指定设备端口、排除一些设备端口或者指定IP地址
listen-address=192.168.1.132,127.0.0.1

# 本地解析文件，默认为 /etc/hosts，可以通过 no-hosts 不使用本地，也可以通过如下指定文件
#addn-hosts=/etc/banner_add_hosts

# 添加额外的上级DNS主机配置文件，默认使用 /etc/resolv.conf
#resolv-file=

# 设置DNS缓存大小，单位是DNS解析的条数，为0时将会禁用缓存，默认为150
cache-size=500
{% endhighlight %}

<!--
### 使用DNS缓存

要在本机上以守护进程方式启动 dnsmasq 做 DNS 缓存服务器，编辑 `/etc/dnsmasq.conf`，添加监听地址；如果作为其它主机的默认 DNS，则需要使用固定 IP 地址。

{% highlight text %}
listen-address=127.0.0.1
{% endhighlight %}

其它主机通过 /etc/resolv.conf 配置 DNS 的 IP 地址，然后通过如下步骤设置 DNS 缓存。

配置 resolv-file=/etc/resolv.dnsmasq.conf，表示从这个指定的文件中寻找上级 dns 服务器列表，而不是从本机默认的 /etc/resolv.conf 中读取 DNS 服务器列表。如果机器的地址是通过 dhcp 取得的话，/etc/resolv.conf 容易受到影响从而影响 dnsmasq。

系统首先寻找本地的 dnsmasq 服务器，如果没有找到则会向上查找。如果添加 strict-order，则表示将严格安装 resolv-file 文件中的顺序从上到下进行 DNS 解析，直到第一个成功解析成功为止。

no-hosts 默认情况下 dnsmasq 会首先寻找本地的 hosts 文件，再去寻找缓存下来的域名, 最后去上级 DNS 服务器中寻找；而 addn-hosts 可以使用额外的 hosts 文件，所以说 dnsmasq 是一个很不错的外部 DNS 中继。

设置 listen-address=127.0.0.1,192.168.0.1 表示该 dnsmasq 服务可以在哪些地址上侦听，对外提供服务的话要写上对应的网口所有的地址。

其他配置项。

cache-size=1024               # 设置缓存大小
log-queries                   # 开启debug模式，记录客户端查询记录到/var/log/debug中

* 客户端机器配置，编辑 /etc/resolv.conf ，调整内容为 'nameserver 192.168.0.1' ，其中该 IP 是内部 DNS 的 IP，也即 dnsmasq 的地址。

# libvirt 配置文件解析

包括 lxc 在内的一些容器，使用的是该配置，可以通过 ps aux \| grep dnsmasq 直接查看相关的配置。

{% highlight text %}
strict-order
pid-file=/var/run/libvirt/network/default.pid
except-interface=lo
bind-dynamic
interface=virbr0


# 设置DHCP服务器分配IP地址的范围
dhcp-range=192.168.122.2,192.168.122.254

dhcp-no-override

# 设置默认的最大租期
dhcp-lease-max=253



dhcp-hostsfile=/var/lib/libvirt/dnsmasq/default.hostsfile
addn-hosts=/var/lib/libvirt/dnsmasq/default.addnhosts
{% endhighlight %}
-->


## 参考

* [DNSmasq 官方网站](http://www.thekelleys.org.uk/dnsmasq/doc.html)，关于 DHCP 和 DNS 的配置可以参考 [DNSmasq – 配置DNS和DHCP](http://debugo.com/dnsmasq/) 。

{% highlight text %}
{% endhighlight %}
