---
title: Docker Network 简介
layout: post
comments: true
language: chinese
category: [linux,misc]
keywords: docker,dockerfile
description:
---


<!-- more -->

## 简介

在启动时可以通过 `--network` 参数指定。

### None

在容器中不会创建任何的外部路由，不过仍然会存在一个回环地址。

{% highlight text %}
# docker run -it --name foobar --network none alpine sh
{% endhighlight %}

此时只有一个 lo 设备。

### Host

与主机共享所有的网络设备，不会虚拟自己的网络设备。

{% highlight text %}
# docker run -it --name foobar --network host alpine sh
{% endhighlight %}

当通过 `ifconfig` 查看网络设备信息时，会发现，根主机上的信息完全一样。

实际上，Docker 通过内核提供的 Network Namespace 隔离网卡、路由、iptable 等，而所谓的 Host 模式，就是没有新建网络的命名空间。

### Container

指定一个容器的名称或者 ID ，会与某个容器共享网络，包括 IP、端口范围等。

{% highlight text %}
# docker run -it --name foobar alpine sh
# docker run -it --name test --net=container:foobar alpine sh
{% endhighlight %}

可以看到，后来启动的 `test` 容器与 `foobar` 容器的网络配置是一样的。

与上述的 Host 模式有些类似，此时，两个不同的容器之间共享 Network Namespace 空间。

### Bridge

这也是默认的，通过 veth 接口连接到虚拟 Bridge 上，此时，在主机以及容器侧会分别存在一个 veth 设备，通过网桥以及 iptables nat 表进行通讯。

<!--
NETWORK		Connects the container to a user created network (using docker network create command)))
-->

## Bridge 模式

在 Docker Server 启动时，默认会在主机上创建一个名为 docker0 的虚拟网桥，该主机上所有启动的 Docker 容器会连接到这个虚拟网桥上，其工作方式类似于物理交换机，主机上的所有容器就通过交换机连在了一个二层网络中。

接着要为容器分配 IP 了，默认会从私有 IP 网段中，选择一个和宿主机不同的 IP 地址和子网分配给 docker0 ，所有连接到 docker0 的容器就从这个子网中分配一个未占用的 IP 。

![docker bridge]({{ site.url }}/images/docker/docker-network-bridge.jpg "docker bridge"){: .pull-center width="35%" }

如上容器中，使用的是 `172.17.0.0/16` 这个网段，并将 `172.17.0.1` 分配给 docker0 网桥。

### 示例

如下，启动两个容器，然后观察内部的网络配置。

{% highlight text %}
----- 创建两个容器
# docker run -itd --name foo alpine sh
# docker run -itd --name bar alpine sh

----- 连接到容器，并查看网络配置
# docker exec -it foo sh
/ # ifconfig eth0
eth0      Link encap:Ethernet  HWaddr 02:42:AC:11:00:03
          inet addr:172.17.0.3  Bcast:172.17.255.255  Mask:255.255.0.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:32 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:4306 (4.2 KiB)  TX bytes:0 (0.0 B)
# docker exec -it bar sh
/ # ifconfig eth0
eth0      Link encap:Ethernet  HWaddr 02:42:AC:11:00:04  
          inet addr:172.17.0.4  Bcast:172.17.255.255  Mask:255.255.0.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:32 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:4306 (4.2 KiB)  TX bytes:0 (0.0 B)

----- 在主机上查看Bridge信息
# ifconfig docker0
docker0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        inet6 fe80::42:42ff:fe88:75c5  prefixlen 64  scopeid 0x20<link>
        ether 02:42:42:88:75:c5  txqueuelen 0  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 68  bytes 9984 (9.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
# ip link show docker0
7: docker0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default 
    link/ether 02:42:42:88:75:c5 brd ff:ff:ff:ff:ff:ff

----- 查看网络
# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
ff951e99dbc2        bridge              bridge              local
a4b2bea17aff        host                host                local
94484d413a30        none                null                local
{% endhighlight %}

## 自定义模式

与默认的 bridge 原理一样，但自定义网络具备内部 DNS 发现，可以通过容器名或者主机名容器之间网络通信。

{% highlight text %}
----- 创建一个test网络
# docker network create test

----- 查看当前网络
# docker network ls

----- 启动容器
# docker run -it --name foo --net=test alpine
/ # ping bar -c 3
PING bar (172.19.0.3): 56 data bytes
64 bytes from 172.19.0.3: seq=0 ttl=64 time=0.110 ms
64 bytes from 172.19.0.3: seq=1 ttl=64 time=0.082 ms
64 bytes from 172.19.0.3: seq=2 ttl=64 time=0.180 ms

--- bar ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max = 0.082/0.124/0.180 ms

# docker run -it --name bar --net=test alpine
/ # ping foo -c 3
PING foo (172.19.0.2): 56 data bytes
64 bytes from 172.19.0.2: seq=0 ttl=64 time=0.115 ms
64 bytes from 172.19.0.2: seq=1 ttl=64 time=0.102 ms
64 bytes from 172.19.0.2: seq=2 ttl=64 time=0.069 ms

--- foo ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max = 0.069/0.095/0.115 ms
{% endhighlight %}

可以看到，直接通过容器名可以 ping 通，实际上是在 `/etc/hosts` 中添加了一条记录。

## 常用命令

<!--
docker network connect	Connect a container to a network
docker network create	Create a network
docker network disconnect	Disconnect a container from a network
docker network inspect	Display detailed information on one or more networks
docker network ls	List networks
docker network prune	Remove all unused networks
docker network rm	Remove one or more networks


https://www.jianshu.com/p/ceac9d166bc3
https://www.cnblogs.com/jsonhc/p/7823286.html
https://www.cnblogs.com/readygood/p/10294633.html
https://www.cnblogs.com/zuxing/articles/8780661.html
https://blog.csdn.net/qxxhjy/article/details/82314128



-->

{% highlight text %}
{% endhighlight %}
