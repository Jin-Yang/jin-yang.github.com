---
title: DNS 配置文件 resolv.conf 简介
layout: post
comments: true
language: chinese
tag: [Protocol, Network, DevOps]
keywords: DNS, resolv.conf, 配置文件
description: Linux 中可以通过 `/etc/resolv.conf` 文件配置 DNS 服务器的地址，不过目前该文件大部分是通过一些工具自动配置的，例如 NetworkManager ，这里详细介绍其使用方式，以及常用的配置参数。
---

Linux 中可以通过 `/etc/resolv.conf` 文件配置 DNS 服务器的地址，不过目前该文件大部分是通过一些工具自动配置的，例如 NetworkManager ，那么这就可能会导致在重启网络服务后该配置文件被刷新。

这里简单介绍其使用方式。

<!-- more -->

## 简介

Linux 系统下域名解析的配置文件是 `/etc/resolv.conf`，一般会配置上两个或更多的 `nameserver`，这样在一个服务器挂掉后还能正常解析域名。

可以在本机起 dnsmasq 并监听本地的 UDP 53 端口，用来监听来自于本地的解析请求，该进程会维护上游服务器的健康状况，不会把解析请求发到挂掉的上游服务器上。

注意，目前大部分的网络配置工具，会将 DHCP、DNS 相关的配置统一到一个配置文件中，所以，每次启动的时候对应的 `/etc/resolv.conf` 会被更新。

## 配置项

其中 `;` 或者 `#` 开头的为注释，对应的行可以直接忽略。

{% highlight text %}
options timeout:2 attempts:3 rotate single-request-reopen
{% endhighlight %}

但是失败后重试的场景和策略是什么呢？

{% highlight text %}
options timeout:1 attempts:1 rotate
nameserver 10.0.0.1
nameserver 10.0.0.2
nameserver 10.0.0.3
{% endhighlight %}

这里大概讲下几个选项的含义，详细可以通过 `man 5 resolv.conf` 查看：

* `nameserver` DNS服务器的IP地址，最多能设三个
* `timeout` 查询一个NS的超时时间，单位是秒，默认是5，最大为30；
* `attempts` 所有服务器查询的整个都尝试一遍的次数，默认是2；
* `rotate` 随机选取一个作为首选DNS服务器，默认是从上到下；

### ndots

会根据 ndots 的值来判断 Domain Name(dname) 是否是一个完全限定域名，如果 dname 中的 `.` 个数小于 `ndots` 则认为不是一个完全限定域名，此时会依次在 `search` 关键词中指定的域表下搜索。

如果 dname 中的 `.` 个数大于等于 `ndots`，会被认为是一个完全限定域名，直接查询 dname 。

{% highlight text %}
nameserver 10.0.12.210
search foobar.com foo.com
options ndots:5
{% endhighlight %}

如果配置如上，那么当查询 `docs.api` 时，实际上会依次查询 `docs.api.foobar.com` `docs.api.foo.com` 。

## 其它

注意，dig、nslook 只会解析 `/etc/resolv.conf` 的内容，而不会解析 `/etc/hosts` 里面内容，可以使用 `getent hosts baidu.com` 查看。

{% highlight text %}
{% endhighlight %}
