---
title: MySQL 中 localhost 127.0.0.1 区别
layout: post
comments: true
language: chinese
category: [mysql,linux,database]
keywords: mysql,linux,localhost,127.0.0.1
description: 最近在 MySQL 中新建了一个数据库帐号，然后配置成允许所有 IP 都可以登陆，但是当尝试从本地登陆时竟然报错 ... ... 用户名和密码都是正确的 WTF 然后仔细研究了下，发现别有天地。
---

最近在 MySQL 中新建了一个数据库帐号，然后配置成允许所有 IP 都可以登陆，但是当尝试从本地登陆时竟然报错 ... ... 用户名和密码都是正确的 WTF

然后仔细研究了下，发现别有天地。

<!-- more -->

## 简介

通过如下方式新建了一个帐号，然后登陆却失败了。

{% highlight text %}
mysql> GRANT ALL ON *.* TO 'foobar'@'%' IDENTIFIED BY '123456';
Query OK, 0 rows affected (0.01 sec)
mysql> SELECT user, host, password FROM mysql.user WHERE user like 'foobar%';
+--------+------+-------------------------------------------+
| user   | host | password                                  |
+--------+------+-------------------------------------------+
| foobar | %    | *6BB4837EB74329105EE4568DDA7DC67ED2CA2AD9 |
+--------+------+-------------------------------------------+
1 row in set (0.00 sec)

$ mysql -ufoobar -h127.0.0.1 -P3307 -p'123456'
ERROR 1045 (28000): Access denied for user 'foobar'@'localhost' (using password: YES)
{% endhighlight %}

需要注意 `%` 和 `localhost` 的配置，简单来说，`%` 是一个通配符，用以匹配所有的 IP 地址，但是不能匹配到 `locahost` 这个特殊的域名。

也就是说，如果要允许本地登录，单纯只配置一个 `%` 是不够的 (应该是说对这种方式是不够的)，需要同时配置一个 `locahost` 的账号。

### Why

在初始化完数据库之后，登录后，通过如下命令查看会发现默认会存在 4 个账号，其对应的用户名都是 `root` ，而 `host` 的配置包括了 `locahost` `主机名` `127.0.0.1` `::1` 。

{% highlight text %}
mysql> SELECT user, host, password FROM mysql.user;
+------+-------------+----------+
| user | host        | password |
+------+-------------+----------+
| root | localhost   |          |
| root | linux.local |          |
| root | 127.0.0.1   |          |
| root | ::1         |          |
+------+-------------+----------+
4 rows in set (0.00 sec)
{% endhighlight %}

在本地登录 MySQL 数据库时，可以通过 `-h` 指定参数，有 `-hlocalhost` 和 `-h127.0.0.1` 两种方式，如果开启了 Unix Domain Socket ，那么还可以通过 `-S` 参数指定。

简单来说，这四个账号都是用于本地登录的，但是有啥区别呢？

## locahost VS. 127.0.0.1

这个概念需要从两个角度看，一个是从操作系统角度，一个是从 MySQL 用户管理的角度。

### 操作系统

首先 `localhost` 是一个域名，而 `127.0.0.1` 是一个 IPv4 的地址；前者可以调用 `gethostname()` 解析出对应的 IP 地址，而后者调用该函数时，因为本身就是个 IP 地址，会直接返回。

<!--
本地地址还有一种简单的写法 `127.1` ，在调用 `gethostname()` 的时候会返回 `127.0.0.1` 这个 IP 地址。
-->

一般来说，`localhost` 指向的是 `127.0.0.1` 这个 IPv4 地址；如果支持 IPv6 ，那么实际上还可以指向 `[::1]` 。

大多数的操作系统通过 `hosts` 文件制定，其中 Linux 在 `/etc/hosts` 文件中指定，一般内容如下。

{% highlight text %}
127.0.0.1 localhost localhost.localdomain
::1       localhost localhost.localdomain
{% endhighlight %}

此时执行 `ping localhost` 域名会被解析成 `127.0.0.1` ；也可以将上述文件中的 `127.0.0.1` 修改为 `8.8.8.8` ，那么再执行 `ping localhost` ，那么会被解析成 `8.8.8.8` 。

当然，正常人一般不会这么办。

#### 127.0.0.1

这个地址通常分配给 `loopback` 虚拟网口，用于本机中各个应用之间的网络交互，只要操作系统的协议栈正常，那么 `loopback` 就能工作。

在 Linux 中可以通过 `ifconfig lo` 命令查看，可以看到其对应的掩码是 `255.0.0.0` ，也就意味着 `127.*` 整个网段是都可以使用的，执行 `ping 127.1.1.1` 也是可以的。

{% highlight text %}
$ ifconfig lo
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 126182  bytes 522268651 (498.0 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 126182  bytes 522268651 (498.0 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
{% endhighlight %}

其中 `lo` 设备与其它的物理设备相同，也可以通过 `ifconfig lo 192.168.128.1` 命令进行修改。当然，正常人一般不会这么办。

### 数据库

这个比较复杂，简单来说，`locahost` 一般意味着使用的是 `Unix Domain Socket` ，此时是不会经过网络防火墙的。

如果 `skip_name_resolve` 配置关闭，那么 `127.0.0.1` 这类地址也会被解析成 `locahost` 。

如下详细介绍。

## 连接方式

如果不指定主机或者使用 `-hlocalhost` ，实际上会有限尝试使用 Unix Domain Socket 连接 (实际还需要保证没有使用 `--protocol=TCP` 参数)，默认的是 `/var/lib/mysql/mysql.sock` ，如果 Sock 地址修改了，也可以通过 `-S PATH` 参数指定。

也就是说，在通过 mysql 客户端访问数据库时，如果指定了 `-h<IP>` 参数，那么会通过 TCP/IP 方式连接数据库。

最简单的，如果要要强制使用 TCP/IP 连接到本地服务器，那就使用 IP 地址 `127.0.0.1` 而不是主机名 `localhost` 。

### 区别

通过 TCP/IP 方式进来的连接，MySQL 服务器接收到的来源主机是 `127.0.0.1`；如果采用的是 UNIX Domain Socket 方式，那么 MySQL 服务器接收到的来源主机是 `localhost` 。

另外，对于 TCP/IP 方式来说，如果关闭了 `skip_name_resolve` 选项，那么会尝试将获取到的 IP 地址解析成域名。

> `skip_name_resolve` 参数在调优时，一般建议开启，也就是说禁止域名解析，可以通过如下命令查看是否开启。
>
>  `SHOW VARIABLES LIKE '%skip_name_resolve%';`
>
> 修改时，可以直接在 `my.cnf` 配置文件的 `[mysqld]` 字段中添加 `skip-name-resolve` 即可。

<!--
关于skip_name_resolve配置项详细可以查看
https://www.cnblogs.com/ivictor/p/5311607.html
-->

## 总结

在 MySQL 中 `localhost` 一般是用来标示 Unix Domain Socket ，如果将 `skip_name_resolve` 关闭，那么 `127.0.0.1` 也可能会被反解析成 `localhost` 。

{% highlight text %}
{% endhighlight %}
